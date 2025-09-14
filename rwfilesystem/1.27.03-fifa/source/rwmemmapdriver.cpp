// (c) Electronic Arts. All Rights Reserved.

#include <EABase/eabase.h>

EA_DISABLE_VC_WARNING(4574)
#include <new>
EA_RESTORE_VC_WARNING()

#include "EAAssert/eaassert.h"
#include "EASTL/intrusive_list.h"
#include "EAStdC/EACType.h"
#include "rw/core/filesys/memmapdriver.h"

using namespace EA::Allocator;
using namespace eastl;

#define MEMMAP_DEFINE_CLASS_NEW_AND_DELETE(name, flags)                                                 \
    void* operator new    (size_t size) { return Manager::GetAllocator()->Alloc(size, name, flags); }   \
    void* operator new [] (size_t size) { return Manager::GetAllocator()->Alloc(size, name, flags); }   \
    void* operator new    (size_t, void* ptr) { return ptr; }                                           \
    void operator delete    (void* ptr) { if (ptr) Manager::GetAllocator()->Free(ptr); }                \
    void operator delete [] (void* ptr) { if (ptr) Manager::GetAllocator()->Free(ptr); }

#ifndef MIN
    #define MIN(a, b) ((a)<(b)?(a):(b))
#endif

#if defined(EA_COMPILER_SN)
// ignore unused variables in release mode
#pragma diag_suppress=552
#endif

namespace rw {
namespace core {
namespace filesys {

namespace {
    const size_t HASH_CODE_UNUSED = 0;

    const unsigned int FILE_CLOSED = ~0u;

    //----------------------------------------------------------------------
    static size_t StringHash(const char* string)
    {
        // FNV1 string hash, courtesy of EASTL, modified for case insensitivity
        // and consistent slashes
        size_t c, result = 2166136261U;

        while ((c = (uint8_t)(*string == '/' ? '\\' : EA::StdC::Tolower(*string))) != 0)
        {
            ++string;
            result = (result * 16777619) ^ c;
        }

        return result;
    }

    //----------------------------------------------------------------------
    #define IS_POWER_OF_TWO(a) (((a) & ((a) - 1)) == 0)
    static int RoundUpToPowerOfTwo(int value, int boundary)
    {
        EA_ASSERT(IS_POWER_OF_TWO(boundary));
        return ((value + (boundary - 1)) & ~(boundary - 1));
    }
}

//----------------------------------------------------------------------
template<typename T>
inline void insert_before(T& pos, T& value)
{
    value.mpNext->mpPrev = value.mpPrev;
    value.mpPrev->mpNext = value.mpNext;

    value.mpNext = &pos;
    value.mpPrev = pos.mpPrev;
    pos.mpPrev->mpNext = &value;
    pos.mpPrev = &value;
}

//----------------------------------------------------------------------
// Pages represent the minimum granularity of allocation within the memmap
// driver.
struct Page : public intrusive_list_node
{
    explicit Page(void* addr, int allocID)
        : mPageAddress(addr),
          mAllocationID(allocID),
          mIsFree(true)
    {
        mpNext = NULL;
        mpPrev = NULL;
    }

    ~Page()
    {
        intrusive_list<Page>::remove(*this);
        mPageAddress = NULL;
    }

    void* mPageAddress;
    const int mAllocationID;
    bool mIsFree;

private:
    Page& operator=(const Page&);
    Page(const Page&);
};

//----------------------------------------------------------------------
struct File : public intrusive_list_node
{
    File()
        : mHashCode(HASH_CODE_UNUSED),
          mFileSize(0),
          mRefCount(0)
    {
        mpNext = NULL;
        mpPrev = NULL;
    }

    ~File()
    {
        EA_ASSERT(mPageList.size() == 0);
        EA_ASSERT_MSG(mRefCount == 0, "File still has open handles!");
        intrusive_list<File>::remove(*this);
    }

	MEMMAP_DEFINE_CLASS_NEW_AND_DELETE("File", EA::Allocator::MEM_TEMP);

    void AddRef() { ++mRefCount; }
    void Release() { --mRefCount; }
    int RefCount() { return mRefCount; }

    size_t mHashCode;
    int mFileSize;
    intrusive_list<Page> mPageList;

private:
    int mRefCount;
};

//----------------------------------------------------------------------
// This is the object that is returned to the user whenever a file handle
// is requested.
struct MemMapHandle
{
    explicit MemMapHandle(unsigned int flags, File& file)
        : mOpenFlags(flags),
          mCurrentPosition(0),
          mFile(file)
    {
        mFile.AddRef();
    }

    ~MemMapHandle()
    {
        mFile.Release();
    }

	MEMMAP_DEFINE_CLASS_NEW_AND_DELETE("MemMapHandle", EA::Allocator::MEM_TEMP);

    unsigned int mOpenFlags;
    int mCurrentPosition;

    File& mFile;

private:
    MemMapHandle& operator=(const MemMapHandle& h);
    MemMapHandle(const MemMapHandle& h);
};

//----------------------------------------------------------------------
// In order to facilitate minimal numbers of allocations, all allocations
// are tracked. This also lets us move data around so that free pages can
// be deallocated on demand.
struct AllocationRecord : public intrusive_list_node
{
    AllocationRecord()
        : mBaseAddress(NULL),
          mBasePage(NULL),
          mAllocationID(++sAllocationIDCounter),
          mSize(0),
          mNumPages(0)
    {
        mpNext = NULL;
        mpPrev = NULL;
    }

    ~AllocationRecord()
    {
        intrusive_list<AllocationRecord>::remove(*this);
        mBaseAddress = NULL;
        mBasePage = NULL;
    }

	MEMMAP_DEFINE_CLASS_NEW_AND_DELETE("AllocationRecord", EA::Allocator::MEM_TEMP);

    void* mBaseAddress;
    Page* mBasePage;
    int mAllocationID;
    int mSize;
    int mNumPages;

private:
    static int sAllocationIDCounter;
};
int AllocationRecord::sAllocationIDCounter = 0;

//----------------------------------------------------------------------
struct AllocationRecordSizeComparer
{
    bool operator()(AllocationRecord& ar1, AllocationRecord& ar2)
    {
        return ar1.mSize > ar2.mSize;
    }
};

//----------------------------------------------------------------------
class MemMapDeviceDriverImpl
{
public:
    MemMapDeviceDriverImpl(const MemMapDeviceDriver::Options& ops)
        : mScratchBuffer(NULL),
          mPoolAllocator(ops.poolAllocator),
          mStorageTransfer(ops.storageTransfer),
          mPageSize(ops.pageSize),
          mNumPages(0),
          mNumFreePages(0)
    {
        EA_ASSERT(IS_POWER_OF_TWO(mPageSize));

        mScratchBuffer = static_cast<char*>(
                Manager::GetAllocator()->Alloc(
                    static_cast<size_t>(mPageSize), "[MemMap] Scratch buffer", EA::Allocator::MEM_PERM, static_cast<unsigned int>(mPageSize)));
    }

    ~MemMapDeviceDriverImpl()
    {
        Format();
        Manager::GetAllocator()->Free(mScratchBuffer);
        mScratchBuffer = NULL;
    }

    MEMMAP_DEFINE_CLASS_NEW_AND_DELETE("MemMapDeviceDriverImpl", EA::Allocator::MEM_PERM)

    File* FindFile(size_t hash);
    bool Delete(size_t hash);
    void AddFile(File* file);

    int PerformRead(MemMapHandle* handle, void* buf, int bufSize);
    int PerformWrite(MemMapHandle* handle, const void* buf, int bufSize);
    int ReleaseMemory();
    bool Format();

private:
    typedef intrusive_list<File>::iterator FileIterator;
    typedef intrusive_list<Page>::iterator PageIterator;

    bool Delete(FileIterator& i);
    PageIterator FindPage(MemMapHandle* handle, int position);
    PageIterator FindCurrentPage(MemMapHandle* handle);
    void ReclaimPages(intrusive_list<Page>& list, PageIterator begin, PageIterator end);
    void FreePage(Page& p);
    void AllocatePages(intrusive_list<Page>& mPageList, int size);
    void FetchPage(PageIterator& page);
    void PushPage(PageIterator& page);

    char* mScratchBuffer;
    PageIterator mPageInScratchBuffer;

    ICoreAllocator& mPoolAllocator;
    MemMapDeviceDriver::IStorageTransfer& mStorageTransfer;

    const int mPageSize;

    int mNumPages;

    intrusive_list<AllocationRecord> mAllocationRecords;
    intrusive_list<File> mFiles;
    intrusive_list<Page> mFreePages;

    int mNumFreePages;

private:
    MemMapDeviceDriverImpl& operator=(const MemMapDeviceDriverImpl&);
    MemMapDeviceDriverImpl(const MemMapDeviceDriverImpl&);
};

//----------------------------------------------------------------------
// Given a hash, return the File record associated with it, if it exists
File* MemMapDeviceDriverImpl::FindFile(size_t hash)
{
    FileIterator end = mFiles.end();

    for(FileIterator i = mFiles.begin(); i != end; ++i)
    {
        if(i->mHashCode == hash)
            return &*i;
    }

    return NULL;
}

//----------------------------------------------------------------------
// Given an iterator pointing to the location in the file list where a
// file lives, delete it and add its pages to the free list
bool MemMapDeviceDriverImpl::Delete(FileIterator& i)
{
    EA_ASSERT(i->RefCount() == 0);

    ReclaimPages(i->mPageList, i->mPageList.begin(), i->mPageList.end());
    EA_ASSERT(i->mPageList.empty());

    delete &(*i);
    return true;
}

//----------------------------------------------------------------------
// Delete a file identified by the given hash
bool MemMapDeviceDriverImpl::Delete(size_t hash)
{
    FileIterator end = mFiles.end();

    for(FileIterator i = mFiles.begin(); i != end; ++i)
        if(i->mHashCode == hash)
            return Delete(i);

    return false;
}

//----------------------------------------------------------------------
// Add a file to the list of files we maintain. This checks in debug to see
// if a file with an existing hash already exists and asserts if it does.
void MemMapDeviceDriverImpl::AddFile(File* file)
{
    #ifdef EA_DEBUG
        for(FileIterator i = mFiles.begin(); i != mFiles.end(); ++i)
            EA_ASSERT(i->mHashCode != file->mHashCode);
    #endif

    mFiles.push_front(*file);
}

//----------------------------------------------------------------------
// Given an offset into a file, find the associated page. This is for
// performing operations like appending or clobbering
MemMapDeviceDriverImpl::PageIterator MemMapDeviceDriverImpl::FindPage(MemMapHandle* handle, int position)
{
    EA_ASSERT(handle != NULL);

    int pageNum = position / mPageSize;
    PageIterator page = handle->mFile.mPageList.begin();

    for(int i = 0; i < pageNum; ++i, ++page)
        ;

    return page;
}

//----------------------------------------------------------------------
// Return the page associated with the current file pointer with a given file
MemMapDeviceDriverImpl::PageIterator MemMapDeviceDriverImpl::FindCurrentPage(MemMapHandle* handle)
{
    return FindPage(handle, handle->mCurrentPosition);
}

//----------------------------------------------------------------------
// Adds any free pages to the free page list to be used later
void MemMapDeviceDriverImpl::ReclaimPages(intrusive_list<Page>& l, PageIterator begin, PageIterator end)
{
    for(PageIterator i = begin; i != end; ++i, ++mNumFreePages)
        i->mIsFree = true;

    mFreePages.splice(mFreePages.begin(), l, begin, end);
}

//----------------------------------------------------------------------
// Ensures that the free page list has enough space to satisfy the given
// allocation and then populates the specified page list with the free
// pages
void MemMapDeviceDriverImpl::AllocatePages(intrusive_list<Page>& pageList, int size)
{
    int sizeDifference = size - (mNumFreePages * mPageSize);

    // If there aren't enough free pages in the free page list already,
    // allocate the difference and create a new allocation record entry
    if(sizeDifference > 0)
    {
        AllocationRecord* record = new AllocationRecord();
        record->mSize = RoundUpToPowerOfTwo(sizeDifference, mPageSize);
        record->mBaseAddress = mPoolAllocator.Alloc(
			static_cast<size_t>(record->mSize), "[MemMap] Foreign storage pool", EA::Allocator::MEM_TEMP, static_cast<unsigned int>(mPageSize));
        record->mNumPages = record->mSize / mPageSize;

        mAllocationRecords.push_front(*record);

		Page* pages = static_cast<Page*>(Manager::GetAllocator()->Alloc(sizeof(Page) * record->mNumPages, "Pages", EA::Allocator::MEM_TEMP));
        record->mBasePage = pages;

        intrusive_list<Page> newPageList;

        for(int i = 0; i < record->mNumPages; ++i)
        {
            new(pages + i) Page(static_cast<char*>(record->mBaseAddress) + i * mPageSize, record->mAllocationID);
            newPageList.push_back(pages[i]);
        }

        // After the pages have been allocated we can just use ReclaimPages
        // to stuff them onto the end of the free page list. This simplifies
        // the code below that actually populates the provided page list
        ReclaimPages(newPageList, newPageList.begin(), newPageList.end());
        EA_ASSERT(newPageList.empty());
        EA_ASSERT(!mFreePages.empty());
    }

    int requiredPages = (size + mPageSize - 1) / (mPageSize);
    PageIterator begin = mFreePages.begin();
    PageIterator end = begin;

    // All required pages are marked as being used so that we
    // don't accidently reclaim them later
    for(int i = 0; i < requiredPages; ++i, ++end)
        end->mIsFree = false;

    // Add the pages we allocated (or not, depending on the number that
    // were in the free list before) to the specified page list.
    pageList.splice(pageList.end(), mFreePages, begin, end);
    mNumFreePages -= requiredPages;
}

//----------------------------------------------------------------------
// Given a page, move the data on this page to a page belonging to a different
// allocation block. This permits us to discard all pages belonging to a certain
// allocation block so that memory may be then freed.
void MemMapDeviceDriverImpl::FreePage(Page& p)
{
    PageIterator end = mFreePages.end();
    for(PageIterator iter = mFreePages.begin(); iter != end; ++iter)
    {
        if(iter->mAllocationID != p.mAllocationID)
        {
            mStorageTransfer.Transfer(iter->mPageAddress, p.mPageAddress, static_cast<size_t>(mPageSize));
            insert_before(p, *iter);
            intrusive_list<Page>::remove(p);

            break;
        }
    }
}

//----------------------------------------------------------------------
// Checks to see if we can shuffle the data around so that we can perform
// any free operations. We use a greedy algorithm to identify the largest
// possible allocation we can free given the amount of free space we have,
// move the data from the pages associated with this allocation, then
// perform the free. This continues as long as the amount of space in the
// free pages is greater than or equal to the amount of space in the
// smallest allocation block
int MemMapDeviceDriverImpl::ReleaseMemory()
{
    if(mNumFreePages == 0)
        return 0;

    int pagesFreed = 0;
    mAllocationRecords.sort(AllocationRecordSizeComparer());

    // The fewest number of pages we can free is the number of pages in the smallest allocation record
    while(!mAllocationRecords.empty() && mNumFreePages >= mAllocationRecords.rbegin()->mNumPages)
    {
        intrusive_list<AllocationRecord>::iterator record = mAllocationRecords.begin();

        while(record->mNumPages > mNumFreePages)
            ++record;

        intrusive_list<Page> freePageList;
        Page* p = record->mBasePage;

        // Iterate through all the pages beloging to this allocation record
        // and move the data to other free pages.
        for(int i = 0; i < record->mNumPages; ++i)
        {
            if(!p[i].mIsFree)
                FreePage(p[i]);
            else
               intrusive_list<Page>::remove(p[i]);

            freePageList.push_front(p[i]);
        }

        EA_ASSERT(static_cast<int>(freePageList.size()) == record->mNumPages);
        pagesFreed += record->mNumPages;
        mNumFreePages -= record->mNumPages;

        for(int i = 0; i < record->mNumPages; ++i)
            record->mBasePage[i].~Page();
        mPoolAllocator.Free(record->mBaseAddress);
        Manager::GetAllocator()->Free(record->mBasePage);
        delete &(*record);
    }

    return pagesFreed * mPageSize;
}

//----------------------------------------------------------------------
// Format will delete all files and release all allocated memory save
// the device driver overhead (such as the scratch buffer).
bool MemMapDeviceDriverImpl::Format()
{
    while(!mFiles.empty())
    {
        FileIterator firstFile = mFiles.begin();
        Delete(firstFile);
    }

    ReleaseMemory();

    EA_ASSERT(mAllocationRecords.size() == 0);
    mAllocationRecords.clear();

    EA_ASSERT(mFreePages.size() == 0);
    mFreePages.clear();

    EA_ASSERT(mFiles.size() == 0);
    mFiles.clear();

    return true;
}

//----------------------------------------------------------------------
// Given a page, this copies the data from that page into the scratch
// buffer.
void MemMapDeviceDriverImpl::FetchPage(PageIterator& page)
{
    if(page != mPageInScratchBuffer)
    {
        mStorageTransfer.Transfer(mScratchBuffer, page->mPageAddress, static_cast<size_t>(mPageSize));
        mPageInScratchBuffer = page;
    }
}

//----------------------------------------------------------------------
// Pushes a page from the scratch buffer to its location in memory
void MemMapDeviceDriverImpl::PushPage(PageIterator& page)
{
    mStorageTransfer.Transfer( page->mPageAddress, mScratchBuffer, static_cast<size_t>( mPageSize ) );
    	mPageInScratchBuffer = page;
}

//----------------------------------------------------------------------
// Performs a read from the current file pointer to the end of the file
// or to the size of the buffer, whichever comes first.
int MemMapDeviceDriverImpl::PerformRead(MemMapHandle* handle, void* buf, int bufSize)
{
    EA_ASSERT(handle->mOpenFlags != FILE_CLOSED);
    EA_UNUSED(FILE_CLOSED);

    if(handle->mCurrentPosition >= handle->mFile.mFileSize)
        return 0;

    char* bufPtr = static_cast<char*>(buf);

    int bytesToCopy = MIN(bufSize, handle->mFile.mFileSize - handle->mCurrentPosition);
    int bytesCopied = 0;
    int roundedUpFileSize = RoundUpToPowerOfTwo(handle->mFile.mFileSize, mPageSize);

    int position = handle->mCurrentPosition;
    PageIterator page = FindCurrentPage(handle);
    PageIterator end = handle->mFile.mPageList.end();

    // As memcpy for the platforms is generally faster than doing a byte-by-byte
    // copy of the data we do some fancy shmancy math to figure out how much we
    // need to copy and where to copy from to do it in one fell swoop.
    while(bytesToCopy != bytesCopied && page != end)
    {
        FetchPage(page);

        // There are two cases for the amount of data we copy. We either copy
        // from the current pointer position to the end of a page boundary, or
        // we copy that last little bit remaining from our destination buffer.
        int positionOffsetInPage = (position & (mPageSize - 1));
        int bytesToCopyFromPage = mPageSize - positionOffsetInPage;

        if(roundedUpFileSize - position <= mPageSize)
            bytesToCopyFromPage -= roundedUpFileSize - handle->mFile.mFileSize;

        bytesToCopyFromPage = MIN(bytesToCopyFromPage, bufSize - bytesCopied);

        memcpy(bufPtr + bytesCopied, mScratchBuffer + positionOffsetInPage, static_cast<size_t>(bytesToCopyFromPage));

        position += bytesToCopyFromPage;
        bytesCopied += bytesToCopyFromPage;

        ++page;
    }

    handle->mCurrentPosition += bytesCopied;
    return bytesCopied;
}

//----------------------------------------------------------------------
// Performs a write of bufSize bytes from the specified buffer to the file.
// If the write goes beyond the end of the file this will allocate any
// necessary extra pages. If the write stops short of the existing end of
// the file (truncation), it will add any unused pages to the free list.
int MemMapDeviceDriverImpl::PerformWrite(MemMapHandle* handle, const void* buf, int bufSize)
{
    EA_ASSERT(handle->mOpenFlags != FILE_CLOSED);

    const char* bufPtr = static_cast<const char*>(buf);
    int totalBytesWritten = 0;
    int positionOffsetInPage = handle->mCurrentPosition & (mPageSize - 1);

    // Determine if we need to allocate any space in order for this write to
    // successfully finish and if so, perform the allocation.
    int spaceToAllocate = 0;
    if(handle->mFile.mFileSize == 0)
    {
        EA_ASSERT(handle->mFile.mPageList.empty());
        spaceToAllocate = bufSize;
    }
    else
    {
        int totalCurrentSpaceAllocated = RoundUpToPowerOfTwo(handle->mFile.mFileSize, mPageSize);
        int spaceRemaining = totalCurrentSpaceAllocated - handle->mCurrentPosition;
        spaceToAllocate = bufSize - spaceRemaining;
    }

    if(spaceToAllocate > 0)
        AllocatePages(handle->mFile.mPageList, spaceToAllocate);

    // Determine where we are in the file currently. If our write starts
    // at the beginning of the page we don't have to fetch the current page
    // from its location in memory as we will just overwrite the page when
    // we push the scratch buffer to memory.
    PageIterator page = FindCurrentPage(handle);

    if(positionOffsetInPage != 0)
        FetchPage(page);

    // Perform the write.
    while(totalBytesWritten != bufSize)
    {
        // Like with the implementation of Read(), we use memcpy in order to
        // move the data around for us, with similar reasoning on what we use
        // to calculate the amount of data we copy
        int bytesWritten = MIN(mPageSize - positionOffsetInPage, bufSize - totalBytesWritten);
        memcpy(mScratchBuffer + positionOffsetInPage, bufPtr, static_cast<size_t>(bytesWritten));

        // If the write doesn't go to the end of the page then we fill it
        // with zeros.
        if(bytesWritten < mPageSize && bufSize - totalBytesWritten < mPageSize)
        {
            int zeroSpaceSize = mPageSize - (positionOffsetInPage + bytesWritten);
            char* zeroSpaceBegin = mScratchBuffer + positionOffsetInPage + bytesWritten;
            memset(zeroSpaceBegin, 0, static_cast<size_t>(zeroSpaceSize));
        }

        if(positionOffsetInPage != 0)
            positionOffsetInPage = 0;

        PushPage(page);
        bufPtr += bytesWritten;
        totalBytesWritten += bytesWritten;

        ++page;
    }

    // Reclaim any unused pages, if they exist
    if(page != handle->mFile.mPageList.end())
        ReclaimPages(handle->mFile.mPageList, page, handle->mFile.mPageList.end());

    // Update the file pointer
    handle->mCurrentPosition += totalBytesWritten;
    handle->mFile.mFileSize = handle->mCurrentPosition;

    return totalBytesWritten;
}

//----------------------------------------------------------------------
MemMapDeviceDriver::MemMapDeviceDriver(const Options& ops)
    : DeviceDriver("memmap:")
{
    mImpl = new MemMapDeviceDriverImpl(ops);
}

//----------------------------------------------------------------------
MemMapDeviceDriver::~MemMapDeviceDriver()
{
    delete mImpl;
    mImpl = NULL;
}

//----------------------------------------------------------------------
void* MemMapDeviceDriver::Open(const char *name, unsigned int openFlags, rw::core::filesys::Handle**)
{
    size_t hash = StringHash(name);

    // see if the file already exists
    File* file = mImpl->FindFile(hash);

    // Do we need to create a new file?
    bool createNewFile(false);

    if ((openFlags & AsyncOp::FOPEN_CREATE) == AsyncOp::FOPEN_CREATE)
    {
        if(file == NULL)
        {
            createNewFile = true;
        }
        else
        {
            file = NULL;

            if ((openFlags & AsyncOp::FOPEN_OVERWRITE) == AsyncOp::FOPEN_OVERWRITE)
            {
                // blow the existing file away and creat a new one
                Delete(name);
                createNewFile = true;
            }
            else    // FOPEN_OVERWRITE flag is not set
            {
                // an existing file cannot be opened with just the FOPEN_CREATE flag
            }
        }
    }
    else        // FOPEN_CREATE flag is not set
    {
        if ((openFlags & AsyncOp::FOPEN_OVERWRITE) == AsyncOp::FOPEN_OVERWRITE)
        {
            if (file != NULL)
            {
                if ((openFlags & AsyncOp::FOPEN_READWRITE) == AsyncOp::FOPEN_READWRITE)
                {
                    // clear the contents of this file
                    Delete(name);
                    createNewFile = true;
                }
                else
                {
                    // this file cannot be opened
                    file = NULL;
                }
            }
        }
        else    // FOPEN_OVERWRITE flag is not set
        {
                // do nothing.  If the file already existed, we return it,
                // otherwise we return NULL.
        }
    }

    if (createNewFile)
    {
        file = new File();
        file->mHashCode = hash;
        file->mFileSize = 0;
        mImpl->AddFile(file);
    }

    if (file != NULL)
    {
        return new MemMapHandle( openFlags, *file );
    }
    else
    {
         // Reporting errors on file open can be tricky because people often use 
         // Open to check if a file exists.
         Manager::GetInstance()->GetErrorHandler()(
            static_cast<int>(Manager::FILEERROR_OPEN), Manager::FILEERROR_OPEN, this, NULL);
    }

    return NULL;
}

//----------------------------------------------------------------------
void MemMapDeviceDriver::Close(void* handle, DeviceDriver*, void*)
{
    EA_ASSERT(handle != NULL);

    MemMapHandle* memMapHandle = static_cast<MemMapHandle*>(handle);

    EA_ASSERT_MSG(memMapHandle->mOpenFlags != FILE_CLOSED, "File has already been closed!");
    delete memMapHandle;
}

//----------------------------------------------------------------------
unsigned int MemMapDeviceDriver::Read(void *handle, void *buf, unsigned int bufSize, DeviceDriver *, void *)
{
    EA_ASSERT(handle != NULL);
    EA_ASSERT(bufSize < 0x80000000UL);

    MemMapHandle* memMapHandle = static_cast<MemMapHandle*>(handle);
    return static_cast<unsigned int>(mImpl->PerformRead(memMapHandle, buf, static_cast<int>(bufSize)));
}

//----------------------------------------------------------------------
unsigned int MemMapDeviceDriver::Write(void *handle, const void * buf, unsigned int bufSize, DeviceDriver *, void *)
{
    EA_ASSERT(handle != NULL);
    EA_ASSERT(bufSize < 0x80000000UL);

    MemMapHandle* memMapHandle = static_cast<MemMapHandle*>(handle);

    if ((memMapHandle->mOpenFlags & AsyncOp::FOPEN_READWRITE) == 0)
    {
        // TODO: the error handler passes back a platform-specific error
        // message here, but in this case, there is no platform-specific
        // error, just a generic failed write.  It would be better to have
        // a better error message.
        Manager::GetInstance()->GetErrorHandler()(0, Manager::FILEERROR_WRITE, this, "MemMapFile");

        return 0;
    }

    // if the seek position is beyond the end of the file, grow it by adding 0's
    // until it's the same size as the seek offset
    int padBytes = memMapHandle->mCurrentPosition - memMapHandle->mFile.mFileSize;

    if (padBytes > 0)
    {
        memMapHandle->mCurrentPosition = memMapHandle->mFile.mFileSize;

        const char padding = 0x00;

        while (padBytes != 0)
        {
            mImpl->PerformWrite(memMapHandle, &padding, 1);
            --padBytes;
        }
    }

    EA_ASSERT(memMapHandle->mCurrentPosition <= memMapHandle->mFile.mFileSize);

    return static_cast<unsigned int>(mImpl->PerformWrite(memMapHandle, buf, static_cast<int>(bufSize)));
}

//----------------------------------------------------------------------
uint64_t MemMapDeviceDriver::Seek(void *handle, uint64_t offset, int whence, DeviceDriver *, void *)
{
    EA_ASSERT(handle != NULL);

    MemMapHandle* memMapHandle = static_cast<MemMapHandle*>(handle);
    EA_ASSERT(memMapHandle->mOpenFlags != FILE_CLOSED);

    EA_ASSERT(whence == FSEEK_BEGIN);
    (void)whence;

    memMapHandle->mCurrentPosition = static_cast<int>(offset);

    return static_cast<uint64_t>(memMapHandle->mCurrentPosition);
}

//----------------------------------------------------------------------
uint64_t MemMapDeviceDriver::GetSize(void* handle, DeviceDriver*, void*)
{
    EA_ASSERT(handle != NULL);

    MemMapHandle* memMapHandle = static_cast<MemMapHandle*>(handle);
    EA_ASSERT(memMapHandle->mOpenFlags != FILE_CLOSED);

    return static_cast<uint64_t>(memMapHandle->mFile.mFileSize);
}

//----------------------------------------------------------------------
// Move a file from one location/name to another. This is a simple operation
// that just involves updating the hash to the new value.
bool MemMapDeviceDriver::Move(const char* from, const char* to)
{
    const size_t fromHash = StringHash(from);

    File* file = mImpl->FindFile(fromHash);

    if(file != NULL)
    {
        const size_t toHash = StringHash(to);

        file->mHashCode = toHash;
        return true;
    }

    return false;
}

//----------------------------------------------------------------------
bool MemMapDeviceDriver::Delete(const char* name)
{
    size_t hash = StringHash(name);

    return mImpl->Delete(hash);
}

//----------------------------------------------------------------------
unsigned int MemMapDeviceDriver::ReleaseMemory()
{
    return static_cast<unsigned int>(mImpl->ReleaseMemory());
}

//----------------------------------------------------------------------
bool MemMapDeviceDriver::Format()
{
    return mImpl->Format();
}

}}}

