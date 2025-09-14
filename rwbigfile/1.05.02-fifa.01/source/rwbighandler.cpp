// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::BigHandler.
*/

#include "EAAssert/eaassert.h"
#include "EAStdC/EAString.h"
#include "EAStdC/EAAlignment.h"
#include "EAStdC/EASprintf.h"
#include "EAStdC/EAMemory.h"
#include "Endian/endian.h"
#include "rw/core/filesys/scatterload.h"    // DECOMPRESS_IN_PLACE_OVERLAP_BYTES constant
#include "rw/core/filesys/version.h"
#include "rw/core/bigfile/handler.h"
#include "rw/core/bigfile/bigfile.h"
#include "rw/core/bigfile/fileformat.h"

#ifdef RWBIGFILE_CHUNKED_DECOMPRESSION_SUPPORTED
#include "rw/core/codec/chunkcodec.h"
#endif

#include "rw/core/codec/refpack.h"
#include "rw/core/codec/version.h"

#include <new>  // placement new

#ifdef EA_DEBUG
#include <stdlib.h> // qsort
#endif

#if defined(_MSC_VER)
    #pragma warning(disable: 4291)  // no matching operator delete (MemberFileStruct class)
#endif

#if !defined(MIN)
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


using namespace EA::Allocator;
using namespace EA::StdC;

namespace rw {
namespace core {

class DecompressDriver : public rw::core::filesys::DeviceDriver
{
public:
    DecompressDriver() {}
    virtual ~DecompressDriver() {}

    virtual void* Open(const char *, unsigned int, rw::core::filesys::Handle **) { return 0; }
    virtual void Close(void *) {}

    virtual unsigned int Read(void *, void *, unsigned int, rw::core::filesys::DeviceDriver *, void *) { return 0; }

    //[FIFA][YC][ACCEPT TARGET][REASON: API added by DICE]
    virtual bool BeginRead(void *h, void *buf, unsigned int bufsize, DeviceDriver *ddParent, void * ddFileHandle) { return 0; };
    virtual bool IsReadDone(void *h, DeviceDriver *ddParent, void * ddFileHandle) { return 0; };
    virtual unsigned int EndRead(void *h, DeviceDriver *ddParent, void * ddFileHandle) { return 0; };
    //[FIFA][YC][ACCEPT TARGET][REASON: API added by DICE]

    virtual uint64_t Seek(void *, uint64_t, int, rw::core::filesys::DeviceDriver *, void *) { return INT64_C(0); }
    virtual uint64_t GetSize(void *) { return INT64_C(0); }
};

DecompressDriver gDecompressDriver;
rw::core::filesys::Device *gpBigHandlerDecompressDevice = NULL;

// signature check for open handles
const int OPENFILESIGNATURE = static_cast<int>(EA::Endian::make_id32('O', 'P', 'E', 'N'));

BigHandler::BigFileStruct::BigFileStruct(filesys::Handle* handle,
    void *_header, char *_name, uint64_t loc, unsigned int _flags, char *_bighead, char *_bigdata) :
next(NULL), hFile(handle), header(_header), mBig(_header), name(_name),
    location(loc), flags(_flags), bighead(_bighead), bigdata(_bigdata)
{
    headerformat = Big::GetHeaderType(_header);

    if (headerformat == Big::BFTYPE_FAST)
    {
        if ((flags & ADDFLAG_LOAD_NAME_INDEX) == ADDFLAG_LOAD_NAME_INDEX)
        {
            mBig.SetNameIndex(static_cast<const char *>(header) + Big::GetHeaderSize(header));
        }
    }
}

BigHandler::MemberFileStruct::MemberFileStruct(BigFileStruct *bf,
    unsigned int _modeflags, unsigned int _fileflags, uint64_t _baseoffset, uint64_t _length, uint64_t _zLength) 
    : pBigfile(bf), 
      modeflags(_modeflags), 
      fileflags(_fileflags), 
      inf(NULL),
      tmpZbuf(NULL), 
      baseoffset(_baseoffset), 
      curroffset(0), 
      length(_length), 
      zLength(_zLength),
      chunkDecompressionClosure(NULL)
{
    signature = OPENFILESIGNATURE;
}

// List helper routines
BigHandler::BigFileStruct* BigHandler::BigFileList::GetHead() const
{
    if (permanenthead)
    {
        permanenthead->next = head;
        return permanenthead;
    }
    else
   {
       return head;
   }
}

void BigHandler::BigFileList::PushHead(BigFileStruct *node)
{
    BigFileStruct *ls = head;
    node->next = ls;
    head = node;
    ++nNodes;

    if(ls == 0)
    {
        tail = node;
    }
}

void BigHandler::BigFileList::PushPermanentHead(BigFileStruct *node)
{
    EA_ASSERT_MSG(permanenthead == NULL, "Only one permanent head allowed.");
    permanenthead = node;
    ++nNodes;
}

void BigHandler::BigFileList::PushTail(BigFileStruct *node)
{
    BigFileStruct *ls = tail;
    node->next = 0;
    tail = node;
    ++nNodes;

    if (ls == 0)
        head = node;
    else
        ls->next = node;
}

bool BigHandler::BigFileList::Remove(BigFileStruct *node)
{
    bool foundit = false;

    if (node == permanenthead)
    {
        permanenthead = NULL;
        --nNodes;
        return true;
    }

    if (node == head)
    {
        foundit = true;
        --nNodes;
        if (node == tail)
            tail = head = NULL;
        else
            head = static_cast<BigFileStruct *>(node->next);
    }
    else if (head != 0) // ensure there's at least one member in the list
    {
        BigFileStruct *prev = head;
        while (prev->next && prev->next != node)
            prev = static_cast<BigFileStruct *>(prev->next);

        if (prev->next && prev->next == node)
        {
            foundit = true;
            --nNodes;
            prev->next = node->next;
            if (node == tail)
                tail = prev;
        }
    }

    if (foundit)
    {
        node->next = 0;
    }

    return foundit;
}

/// \brief Construct a BigHandler object
///
/// \importlib rwbigfile
///
/// After the object is created, register it with the library so it can be
/// used just any other physical devices with
/// rw::core::filesys::Manager::RegisterDevice.  The device's name is "big:".
///
/// Before any files can be opened with device, any files on this device,
/// use the AddArchiveFromFile method to add one or more bigfiles to the
/// device.
///
/// \param allocator Allocator object to be used for this class' allocations
/// and deallocations.
/// \sa ~BigHandler, AddArchiveFromFile, RemoveArchive
BigHandler::BigHandler(ICoreAllocator* allocator) 
    : filesys::DeviceDriver("big:"),
      mAllocator(allocator),
      mChunkDecompressor(NULL)
{
    if (gpBigHandlerDecompressDevice == NULL)
    {
        gpBigHandlerDecompressDevice =
            rw::core::filesys::Manager::GetInstance()->RegisterDevice(&gDecompressDriver);
    }
}

/// \brief Construct a BigHandler object with a user-specified name
///
/// \importlib rwbigfile
///
/// This constructor allows the caller to specify an alternate name
/// for the device.  This can be useful if you want to have more than
/// a single instance in your game.
///
/// \param allocator Allocator object to be used for this class' allocations
/// and deallocations.
/// \param deviceName Name to give the new device.  The default is "big:".
/// \sa ~BigHandler, AddArchiveFromFile, RemoveArchive
BigHandler::BigHandler(ICoreAllocator* allocator, const char *deviceName)
    : filesys::DeviceDriver(deviceName),
      mAllocator(allocator),
      mChunkDecompressor(NULL)
{
    if (gpBigHandlerDecompressDevice == NULL)
    {
        gpBigHandlerDecompressDevice =
            rw::core::filesys::Manager::GetInstance()->RegisterDevice(&gDecompressDriver);
    }
}

#ifdef RWBIGFILE_CHUNKED_DECOMPRESSION_SUPPORTED
/// \brief Set parameters for decompression jobs.
///
/// \importlib rwbigfile
///
/// This function allows the caller set parameters for the rwbigfile library
/// to decompress the loaded with a job.  These values are passed to the codec
/// chunk decompressor if data of that type is loaded.
/// \param pJobScheduler Pass your application job scheduler here.
/// \param cJobs The maximum number of parallel jobs to run.
/// \param jobAffinity The job affinity.
/// \param jobPriority The job priority.
BigHandler::BigHandler(ICoreAllocator* allocator, 
        const char *deviceName, 
        EA::Jobs::JobScheduler *jobScheduler, 
        uint32_t unused, 
        EA::Jobs::JobAffinity jobAffinity,
        EA::Jobs::JobPriority jobPriority) 
    : filesys::DeviceDriver(deviceName), 
      mAllocator(allocator), 
      mChunkDecompressor(NULL)
{
    if (gpBigHandlerDecompressDevice == NULL)
    {
        gpBigHandlerDecompressDevice =
            rw::core::filesys::Manager::GetInstance()->RegisterDevice(&gDecompressDriver);
    }

    mChunkDecompressor = static_cast<rw::core::codec::ChunkDecompressor *>(
        mAllocator->Alloc(sizeof(rw::core::codec::ChunkDecompressor), NULL, 0,
        EA_ALIGN_OF(rw::core::codec::ChunkDecompressor)));

    new (mChunkDecompressor) rw::core::codec::ChunkDecompressor(
        mAllocator, jobScheduler, unused, jobAffinity, jobPriority);
}
#endif

/// \brief Destroy a BigHandler object.
///
/// \importlib rwbigfile
///
/// This function releases any resources allocated by the object.
///    
/// Before destroying the object, all open subfiles should be closed
/// and all added bigfiles should be removed with RemoveArchive.  Also,
/// the bigfile device should be unregistered with the library so the
/// library can throw a helpful error message if the big: device is
/// referenced after it's been destroyed.
///
/// \sa BigHandler, AddArchiveFromFile, RemoveArchive
BigHandler::~BigHandler()
{
    EA_ASSERT_MSG(mBigfileList.GetCount() == 0,
        "Remove all open archive files first.");

    if (gpBigHandlerDecompressDevice != NULL)
    {
        rw::core::filesys::Manager::GetInstance()->UnregisterDevice(gpBigHandlerDecompressDevice);
        gpBigHandlerDecompressDevice = NULL;
    }

#ifdef RWBIGFILE_CHUNKED_DECOMPRESSION_SUPPORTED
    if (mChunkDecompressor != NULL)
    {
        mChunkDecompressor->~ChunkDecompressor();
        mAllocator->Free(mChunkDecompressor);
        mChunkDecompressor = NULL;
    }
#endif
}

/// \brief Adds a bigfile archive to the search list
///
/// \importlib rwbigfile
///
/// Adds the given bigfile to the list of searchable bigfiles.  The new bigfile
/// is added to the tail of the search list.
///
/// \param filename name of bigfile to add to the list
/// \param flags 0 or a combination of some of the AddOption flags
/// \return On success, a handle to the new archive is returned.  Use this
/// handle to remove the archive from the list when it is no longer needed.
/// If there was an error (such as file not found, unrecognized bigfile type),
/// NULL is returned.
///
/// \sa RemoveArchive
void *BigHandler::AddArchiveFromFile(const char *filename, unsigned int flags)
{
    const unsigned int DEFAULT_MAX_BIGFILE_HEADERSIZE = sizeof(BigFileFormatHeader);
    char stackHeaderBuffer[DEFAULT_MAX_BIGFILE_HEADERSIZE + RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT];
    filesys::AsyncOp fileOp;

    char headerName[filesys::FILESYS_MAX_PATH];
    Strcpy(headerName, filename);

    // substitute the extention with ".bh" if the caller is using an
    // alternate header.
    if ((flags & ADDFLAG_USE_ALTERNATE_HEADER) == ADDFLAG_USE_ALTERNATE_HEADER)
    {
        Strcpy(Strrchr(headerName, '.'), ".bh");
    }

    unsigned int openFlags = filesys::AsyncOp::FOPEN_READONLY;

    if ((flags & ADDFLAG_DRM) == ADDFLAG_DRM)
    {
        Strcat(headerName, ".edat");
        openFlags |= filesys::AsyncOp::FOPEN_DRM;
    }

    if ((flags & ADDFLAG_FILE_USES_SDATA) == ADDFLAG_FILE_USES_SDATA)
    {
        // name gets changed in the driver, just the flag is needed
        openFlags |= filesys::AsyncOp::FOPEN_SDATA;
    }
    
    fileOp.Open(headerName, openFlags);
    filesys::Handle *handle = fileOp.GetResultHandle();

    if ((handle == NULL) &&
        ((flags & ADDFLAG_USE_ALTERNATE_HEADER) == ADDFLAG_USE_ALTERNATE_HEADER))
    {
        // the alternate header file was not found.  Fall back on the
        // actual file
        Strcpy(headerName, filename);

        char filename_edat[filesys::FILESYS_MAX_PATH];
        Strcpy(filename_edat, filename);

        if ((flags & ADDFLAG_DRM) == ADDFLAG_DRM)
        {
            Strcat(filename_edat, ".edat");
        }

        fileOp.Open(filename_edat, openFlags);
        handle = fileOp.GetResultHandle();
    }

    if (handle == NULL)
    {
        return NULL;
    }

    void *alignedStackHeaderBuffer = AlignUp(
        stackHeaderBuffer, RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT);

    // read the first 64 bytes to get the size of the header.  Sizeofheader
    // will abort if this is not a recognized bigfile format...
    fileOp.Read(handle, alignedStackHeaderBuffer, 0, DEFAULT_MAX_BIGFILE_HEADERSIZE);
    unsigned int nread = static_cast<uint32_t>(fileOp.GetResultSize());
    EA_ASSERT_FORMATTED(nread == DEFAULT_MAX_BIGFILE_HEADERSIZE,
        ("Bigfile header must be at least 16 bytes"));

    unsigned int headerSize = Big::GetHeaderSize(alignedStackHeaderBuffer);

    // ... but we'll check anyway and return failure
    // if the file type is not recognized.
    if (headerSize == 0)
    {
        fileOp.Close(handle);
        fileOp.Wait();
        return NULL;
    }

    if (flags & ADDFLAG_LOAD_NAME_INDEX)
    {
        headerSize += Big::GetNameIndexSize(alignedStackHeaderBuffer);
    }

    if (Big::HasFAT(alignedStackHeaderBuffer))
    {
        headerSize = AlignUp(headerSize, 16);
        headerSize += Big::GetFATSize(alignedStackHeaderBuffer);
    }

    // allocate memory for the header
    char allocNameString[512];
    Sprintf(allocNameString, "rw::core::BigHandler::AddArchiveFromFile (%s)", filename);

    void *header = mAllocator->Alloc(headerSize, allocNameString,
        MEM_TEMP, RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT);
    EA_ASSERT(header != NULL);

    // copy the previously-read data into the header buffer
    Memcpy(header, alignedStackHeaderBuffer, DEFAULT_MAX_BIGFILE_HEADERSIZE);

    if (headerSize > DEFAULT_MAX_BIGFILE_HEADERSIZE)
    {
        // read the rest of the header into the allocated buffer
        fileOp.Read(handle, (char *)header + DEFAULT_MAX_BIGFILE_HEADERSIZE,
            DEFAULT_MAX_BIGFILE_HEADERSIZE, headerSize - DEFAULT_MAX_BIGFILE_HEADERSIZE);
        nread += static_cast<uint32_t>(fileOp.GetResultSize());
        EA_ASSERT_FORMATTED(nread == headerSize, ("incorrect # bytes read"));
    }

    // Copy the name of the bigfile.  We need this for searching,
    // if the user ever wants to look in a specific bigfile for a
    // subfile.
    char *name = static_cast<char *>(mAllocator->Alloc(Strlen(filename)+1,
        allocNameString, MEM_TEMP));
    EA_ASSERT(name != NULL);

    Strcpy(name, filename);

    if (Strcmp(filename, headerName) != 0)
    {
        // if the header came out of an alternate file, close that
        // handle and open the actual bigfile now.
        fileOp.Close(handle);
        fileOp.Wait();
        char filename_edat[filesys::FILESYS_MAX_PATH];
        Strcpy(filename_edat, filename);

        if ((flags & ADDFLAG_DRM) == ADDFLAG_DRM)
        {
            Strcat(filename_edat, ".edat");
        }
        fileOp.Open(filename_edat, openFlags);
        handle = fileOp.GetResultHandle();
        nread = 0;
    }

    BigFileStruct *pBigHandle = new (mAllocator->Alloc(
        sizeof(BigFileStruct), "BigFileHandler", MEM_TEMP))
        BigFileStruct(handle, header, name, filesys::GetLocation(handle), flags, NULL, NULL);

    {
        EA::Thread::AutoFutex autoFutex( mBigFileListMutex );

        // put this newly-opened bigfile on the list of bigfiles to search when
        // this virtual device is asked to open a file.
        if (ADDFLAG_ADD_PERMANENT_HEAD == (flags & ADDFLAG_ADD_PERMANENT_HEAD))
        {
            mBigfileList.PushPermanentHead(pBigHandle);
        }
        else if (ADDFLAG_ADD_TO_HEAD == (flags & ADDFLAG_ADD_TO_HEAD))
        {
            mBigfileList.PushHead(pBigHandle);
        }
        else
        {
            mBigfileList.PushTail(pBigHandle);
        }
    }

    // return the index into the newly-added bigfile (0 is a valid index)
    return static_cast<void *>(pBigHandle);
}

/// \brief Adds a bigfile archive from memory to the search list
///
/// \importlib rwbigfile
///
/// Adds the given bigfile to the list of searchable bigfiles.  The new bigfile
/// is added to the tail of the search list.
///
/// \param filename name of bigfile to add to the list
/// \param bigdata pointer to the big file (.big) in memory
/// \param bighead pointer to the big file header (.bh) in memory (optional)
/// \param flags 0 or a combination of some of the AddOption flags
/// \return On success, a handle to the new archive is returned.  Use this
/// handle to remove the archive from the list when it is no longer needed.
/// If there was an error (such as file not found, unrecognized bigfile type),
/// NULL is returned.
///
/// \sa RemoveArchive
void *BigHandler::AddArchiveFromMemory(const char *filename, char* bigdata, char* bighead/* = NULL*/, unsigned int flags/* = 0*/)
{
    //BigHandler is a virtual device and must maintain a parent handle, so despite never using this handle, we'll keep the file open as normal.
    filesys::AsyncOp fileOp;
    fileOp.Open(filename, filesys::AsyncOp::FOPEN_READONLY);
    filesys::Handle *handle = fileOp.GetResultHandle();
    if (handle == NULL)
    {
        return NULL;
    }

    char* header = bighead ? bighead : bigdata;

    unsigned int headerSize = Big::GetHeaderSize(header);
    if (headerSize == 0)
    {
        return NULL;
    }

    // Copy the name of the bigfile.  We need this for searching,
    // if the user ever wants to look in a specific bigfile for a
    // subfile.
    char allocNameString[512];
    Sprintf(allocNameString, "rw::core::BigHandler::AddArchiveFromMemory (%s)", filename);
    char *name = static_cast<char *>(mAllocator->Alloc(Strlen(filename)+1, allocNameString, MEM_TEMP));
    EA_ASSERT(name != NULL);
    Strcpy(name, filename);

    BigFileStruct *pBigHandle = new (mAllocator->Alloc(sizeof(BigFileStruct), "BigFileHandler", MEM_TEMP))
                                BigFileStruct(handle, header, name, 0, flags, bighead, bigdata);

    {
        EA::Thread::AutoFutex autoFutex( mBigFileListMutex );

        // put this newly-opened bigfile on the list of bigfiles to search when
        // this virtual device is asked to open a file.
        if (ADDFLAG_ADD_PERMANENT_HEAD == (flags & ADDFLAG_ADD_PERMANENT_HEAD))
        {
            mBigfileList.PushPermanentHead(pBigHandle);
        }
        else if (ADDFLAG_ADD_TO_HEAD == (flags & ADDFLAG_ADD_TO_HEAD))
        {
            mBigfileList.PushHead(pBigHandle);
        }
        else
        {
            mBigfileList.PushTail(pBigHandle);
        }
    }

    return static_cast<void *>(pBigHandle);
}

/// \brief Remove an archive from the bigfile search list.
///
/// \importlib rwbigfile
///
/// This function removes an archive from the search list.  Before
/// removing the archive, all of its subfiles should be closed.
/// In debug configs, an assert will be thrown if this is not the
/// case.
///
/// \sa AddArchiveFromFile

void BigHandler::RemoveArchive(void *bighandle)
{
    BigFileStruct *pBigHandle = static_cast<BigFileStruct *>(bighandle);
    filesys::Handle *hFile = pBigHandle->hFile;

    {
        EA::Thread::AutoFutex autoFutex( mBigFileListMutex );

#ifdef EA_DEBUG
        int ret = mBigfileList.Remove(pBigHandle);
        EA_ASSERT_FORMATTED(ret == 1, ("This bigfile was not found in the active list"));
#else
        mBigfileList.Remove(pBigHandle);
#endif

        // release memory
        if (pBigHandle->bigdata)
        {
            mAllocator->Free(pBigHandle->bigdata);
            if (pBigHandle->bighead)
            {
                mAllocator->Free(pBigHandle->bighead);
            }
        }
        else //if not a memory-big, header will have been allocated
        {
            mAllocator->Free(pBigHandle->header);
        }
        mAllocator->Free(pBigHandle->name);

        // clear out the structure data
        MemFill32(pBigHandle, 0, sizeof(BigFileStruct));

        delete pBigHandle;
        mAllocator->Free(pBigHandle);
    }

    // close the bigfile
    filesys::AsyncOp fileOp;
    fileOp.Close(hFile);
    fileOp.GetStatus(EA::Thread::kTimeoutNone);
}

/// \brief Get the bigfile header from a given archive handle.
///
/// \importlib rwbigfile
///
/// This function is really only useful for debugging purposes
/// (say you want to iterate over the bigfile index and print
/// out each member's information).
void *  BigHandler::GetHeader(void *archive     ///< archive handle you want to get the header for
    )
{
    return static_cast<BigFileStruct *>(archive)->header;
}

#ifdef EA_DEBUG

bool    BigHandler::Hash16CollisionCheck(unsigned int flags)
{
    EA::Thread::AutoFutex autoFutex( mBigFileListMutex );

    bool ret = true;

    BigFileStruct *Itr;

    int totalEntries = 0;

    for ( Itr = mBigfileList.GetHead(); Itr != NULL; Itr = Itr->next)
    {
        if (Itr->headerformat == Big::BFTYPE_HASH16 ||
            Itr->headerformat == Big::BFTYPE_HASH20 ||
            Itr->headerformat == Big::BFTYPE_FAST)
        {
            totalEntries += Itr->mBig.GetCount();
        }
    }

    uint32_t *entryArray = static_cast<uint32_t *>(mAllocator->Alloc(
        totalEntries * sizeof(uint32_t),
        "rw::core::BigHandler::Hash16CollisionCheck", MEM_TEMP));

    uint32_t *currEntry = entryArray;

    for ( Itr = mBigfileList.GetHead(); Itr != NULL; Itr = Itr->next)
    {
        if (Itr->headerformat == Big::BFTYPE_HASH16 ||
            Itr->headerformat == Big::BFTYPE_HASH20 ||
            Itr->headerformat == Big::BFTYPE_FAST)
        {
            for (int i = 0; i < Itr->mBig.GetCount(); ++i)
            {
                *currEntry++ = Itr->mBig.GetHashValueByIndex(i);
            }
        }
    }

    ::qsort(entryArray, static_cast<size_t>(totalEntries),
        sizeof(uint32_t), uint_cmp);

    uint32_t collisionValue(0);

    for (int i = 1; (i < totalEntries) && ret; ++i)
    {
        if (entryArray[i] == entryArray[i-1])
        {
            // a collision was found.
            collisionValue = entryArray[i];
            ret = false;
        }
    }

    mAllocator->Free(entryArray);

    if (!ret && ((flags & HASH_COLLISION_CHECK_ASSERT) == HASH_COLLISION_CHECK_ASSERT))
    {
        // find the two bigfiles that had a collision
        char *bigfile1 = NULL, *bigfile2 = NULL;

        for ( Itr = mBigfileList.GetHead();
            (Itr != NULL) &&
            (bigfile1 == NULL) &&
            (bigfile2 == NULL);
            Itr = Itr->next)
        {
            if (Itr->headerformat == Big::BFTYPE_HASH16 ||
                Itr->headerformat == Big::BFTYPE_HASH20 ||
                Itr->headerformat == Big::BFTYPE_FAST)
            {
                for (int i = 0; 
                    (i < Itr->mBig.GetCount()) &&
                    (bigfile1 == NULL) &&
                    (bigfile2 == NULL);
                    ++i)
                {
                    if (Itr->mBig.GetHashValueByIndex(i) == collisionValue)
                    {
                        if (bigfile1 == NULL)
                        {
                            bigfile1 = Itr->name;
                        }
                        else
                        {
                            bigfile2 = Itr->name;
                        }
                    }
                }
            }
        }

        EA_FAIL_FORMATTED(("Collision detected between bigfiles %s and %s (Name hash value 0x%08x)", 
            bigfile1, bigfile2, collisionValue));
    }

    return ret;
}

int BigHandler::uint_cmp(const void *pa, const void *pb)
{
    const uint64_t a = *static_cast<const uint32_t *>(pa);
    const uint64_t b = *static_cast<const uint32_t *>(pb);

    if (a > b)
        return 1;
    else if (a < b)
        return -1;
    else
        return 0;
}

#endif

// ---------------------------------------------------------------------------
// Non-public functions.  These are defined by the abstract DeviceDriver
// base class - see the docs on rw::core::filesys::DeviceDriver for
// information on how that class works.

// Open will search the list of added archives.  If a match is found, the
// Parent file handle is returned via pParentFileHandle.
void * BigHandler::Open(const char *name, unsigned int modeflags, filesys::Handle **pParentFileHandle)
{
    uint64_t offset = 0, size = 0, zSize = 0;
    unsigned int entryFlags = BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_NONE;
    bool searchall;
    bool findAlternateMatch = (modeflags & FOPEN_SEARCH_OTHER_BIGFILES) != 0;
    MemberFileStruct alternateFileInfo;
    char bigfn[filesys::FILESYS_MAX_PATH];
    bool openByIndex = false;
    char openByIndexFileName[filesys::FILESYS_MAX_PATH] = {}; // The name of the bigfile to open by index in
    int openByIndexFileIndex = 0; // The index of the file to open

    EA::Thread::AutoFutex autoFutex( mBigFileListMutex );

    if (mBigfileList.GetHead() == NULL)
        return NULL;

    // skip over the device name, if present.
    const char *filename = Strchr(name, ':');
    if (filename != NULL)
        name = filename + 1;

    // skip over the first / character.
    if (name[0] == '/' || name[0] == '\\')
        ++name;

    const char *fn = Strchr(name, '|');   // locate unqualified portion

    // set up the bigfile search
    // BIG_name is the name of container file to search in
    // (if blank, all active bigfiles are searched)
    char *BIG_name = bigfn;

    // filename is the name of the subfile to search for
    filename = name;

    // if file has a big file qualifier then extract it
    if (fn)
    {
        Strncpy(BIG_name, name, (unsigned long)(fn - name));  // copy target big file's name
        BIG_name[fn - name] = '\0';          // null-terminate it
        filename = fn + 1;                   // select unqualified filename
        searchall = false;
    }
    else
    {
        BIG_name[0] = 0;
        searchall = true;

        // Is this an open by file index?
        // Example: big:someBigFile.big,2
        const char* commaLoc = Strrchr(name, ',');
        if (commaLoc != NULL)
        {
            size_t locationOfComma = static_cast<size_t>(commaLoc - name);

            memcpy(openByIndexFileName, name, locationOfComma);
            // Now, get the index.
            commaLoc++;
            openByIndexFileIndex = strtol(commaLoc, NULL, 10);

            // Change name to the true name.
            filename = openByIndexFileName;

            EA_ASSERT(openByIndexFileIndex >= 0);

            openByIndex = true;
        }
    }

    BigFileStruct *bf = NULL;
    BigFileStruct *alternateMatch = NULL;
    BigFileStruct *Itr = mBigfileList.GetHead();

    while (Itr != NULL)
    {
        entryFlags = 0;

        // Skip over any device name on the loaded bigfile
        char * filenameNoDevice = Strchr(Itr->name, ':');
        if(filenameNoDevice == 0)
            filenameNoDevice = Itr->name;
        else
            filenameNoDevice++; // skip over ':'

        bool containerNameMatch = searchall ? true : !Strcmp(filenameNoDevice, BIG_name);

        if (containerNameMatch || (findAlternateMatch && (alternateMatch == NULL)))
        {
            unsigned int locateFlags = Big::LOCATE_OPT_NONE;

            if ((Itr->flags & ADDFLAG_SLASH_CONVERT_FORWARD) != 0)
            {
                locateFlags |= Big::LOCATE_OPT_SLASH_CONVERT_FORWARD;
            }
            else if ((Itr->flags & ADDFLAG_SLASH_CONVERT_BACKWARD) != 0)
            {
                locateFlags |= Big::LOCATE_OPT_SLASH_CONVERT_BACK;
            }

            if ((Itr->flags & ADDFLAG_NAME_CONVERT_LOWERCASE) != 0)
            {
                locateFlags |= Big::LOCATE_OPT_NAME_CONVERT_LOWERCASE;
            }
            else if ((Itr->flags & ADDFLAG_NAME_CONVERT_UPPERCASE) != 0)
            {
                locateFlags |= Big::LOCATE_OPT_NAME_CONVERT_UPPERCASE;
            }

            if (openByIndex)
            {
                // Is this the correct bigfile?
                if (Stricmp(filename, Itr->name) == 0)
                {
                    Itr->mBig.FindFileByIndex(openByIndexFileIndex, &offset, &size, &zSize, &entryFlags);
                    bf = Itr;
                    break;
                }
            }
            // either we are searching all big files, or this is the target big file
            else if (Itr->headerformat == Big::BFTYPE_HASH16 ||
                Itr->headerformat == Big::BFTYPE_HASH20 ||
                Itr->headerformat == Big::BFTYPE_FAST)
            {
                if (Itr->mBig.FindFileByName(filename, &offset, &size, &zSize, NULL, locateFlags, &entryFlags))
                {
                    if (containerNameMatch)
                    {
                        bf = Itr;
                        break;
                    }
                    else if (alternateMatch == NULL)
                    {
                        alternateMatch = Itr;
                        alternateFileInfo.baseoffset = offset;
                        alternateFileInfo.length = size;
                        alternateFileInfo.zLength = zSize;
                        alternateFileInfo.fileflags = entryFlags;
                    }
                }
            }
            else if (Itr->mBig.LocateEntry(filename, 0, &offset, &size, NULL, NULL, locateFlags))
            {
                if (containerNameMatch)
                {
                    bf = Itr;
                    break;
                }
                else if (alternateMatch == NULL)
                {
                    alternateMatch = Itr;
                    alternateFileInfo.baseoffset = offset;
                    alternateFileInfo.length = size;
                    alternateFileInfo.zLength = 0;
                    alternateFileInfo.fileflags = BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_NONE;
                }
            }
        }

        Itr = Itr->next;
    }

    if ((bf == NULL) && 
        (findAlternateMatch && (alternateMatch != NULL)))
    {
        // we didn't find a match in the bigfile specified, but we did find
        // a match in another bigfile; so use that one.
        bf = alternateMatch;
        offset = alternateFileInfo.baseoffset;
        size = alternateFileInfo.length;
        zSize = alternateFileInfo.zLength;
        entryFlags = alternateFileInfo.fileflags;
    }

    if (bf != 0)
    {
        //if we've got a compressed file and we're decompressing externally, override entryFlags, size and zSize.
        if (zSize != 0 && (bf->flags & ADDFLAG_EXTERNAL_DECOMPRESS) != 0)
        {
            size = zSize;
            zSize = 0;
            entryFlags = BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_NONE;
        }
        else
        {
            if (!(bf->headerformat == Big::BFTYPE_HASH16 || 
                bf->headerformat == Big::BFTYPE_HASH20 ||
                bf->headerformat == Big::BFTYPE_FAST) && size != 0)
            {
                // read in the first 16 bytes of the target file, so we can
                // tell whether the file is compressed or not.
                char tmpHeaderBuffer[64];
                char *tmpHeaderBuf = static_cast<char *>(
                    (AlignUp(tmpHeaderBuffer, RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT)));
                filesys::AsyncOp readOp;

                if (bf->bigdata)
                {
                    Memcpy(tmpHeaderBuf, bf->bigdata + offset, 16);
                }
                else
                {
                    readOp.Read(bf->hFile, tmpHeaderBuf, (uint64_t)offset, 16);
                    readOp.Wait();
                }

                // Check whether the file is compressed
                codec::RefpackBuf refpack;
                zSize = refpack.GetDecompressedSize(tmpHeaderBuf);

                // if not, the size value returned by LocateEntry above is correct.
                // if so, update the size to be what is reported by GetOriginalSize.
                if (zSize != 0)
                {
                    uint64_t smallSize = 0;

                    smallSize = size;
                    size = zSize;
                    zSize = smallSize;

                    entryFlags = BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_REFPACK;
                }
            }
            else
            {
                if (zSize == size)
                {
                    // if the file is uncompressed, set zSize back to 0.
                    zSize = 0;
                }
            }

            if ((zSize != 0) && (zSize > size))
            {
                EA_ASSERT_MSG((modeflags & FOPEN_DECOMPRESS_IN_PLACE) != FOPEN_DECOMPRESS_IN_PLACE,
                    ("In-place decompression cannot be used with files that compress to a larger size than the orginal data.\n"));
            }
        }

        // we found a bigfile with the file name in it.
        MemberFileStruct *hSubF = new (mAllocator->Alloc(sizeof(MemberFileStruct),
            "BigFileHandler", MEM_TEMP))
            MemberFileStruct(bf, modeflags, entryFlags, offset, size, zSize);

        // This is a virtual device, so tell the library that it has a parent file
        *pParentFileHandle = bf->hFile;

        // ProfilerEventBigFileOpen
        #ifdef EA_FILESYSTEM_PROFILING
        {
            rw::core::filesys::Manager *pMgr = rw::core::filesys::Manager::GetInstance();
            rw::core::filesys::FileSysProfiler * pProfiler = pMgr->GetEventProfiler();

            if (pProfiler != NULL)
            {
                rw::core::filesys::ProfilerEventBigFileOpenStruct
                    pebfos( bf->name, filename, hSubF );

                pProfiler->Callback(
                    rw::core::filesys::ProfilerEventBigFileOpen,
                    NULL,
                    &pebfos,
                    pMgr->GetEventProfilerContext() );
            }
        }
        #endif

        return reinterpret_cast<void *>(hSubF);
    }

    return NULL;
}

void        BigHandler::Close(void * vfh)
{
    // ProfilerEventBigFileClose
    #ifdef EA_FILESYSTEM_PROFILING
    {
        rw::core::filesys::Manager *pMgr = rw::core::filesys::Manager::GetInstance();
        rw::core::filesys::FileSysProfiler * pProfiler = pMgr->GetEventProfiler();

        if (pProfiler != NULL)
        {
            pProfiler->Callback( 
                rw::core::filesys::ProfilerEventBigFileClose,
                NULL,
                vfh,
                pMgr->GetEventProfilerContext() );
        }
    }
    #endif

    MemberFileStruct *sfh = reinterpret_cast<MemberFileStruct *>(vfh);

    EA_ASSERT_FORMATTED(sfh->signature == OPENFILESIGNATURE, ("invalid signature"));

    if (sfh->inf != NULL)
    {
        sfh->inf->~ICodec();
        mAllocator->Free(sfh->inf); sfh->inf = NULL;
        mAllocator->Free(sfh->tmpZbuf); sfh->tmpZbuf = NULL;
    }

    sfh->~MemberFileStruct();
    mAllocator->Free(sfh);
}

uint64_t    BigHandler::Seek(void * vfh, uint64_t offset, int whence,
    DeviceDriver *ddParent, void * ddFileHandle)
{
    MemberFileStruct *sfh = reinterpret_cast<MemberFileStruct *>(vfh);
    EA_ASSERT_FORMATTED(sfh->signature == OPENFILESIGNATURE, ("invalid signature"));

    // parent file device driver will have code to
    // detect any seeks that are redundant and ignore them.
    // If we're seeking to the start of the file, or the file isn't compressed, this
    // is OK.
    if ((offset == 0) || (sfh->zLength == 0))
    {
        if (!sfh->pBigfile->bigdata) //avoid seek if this is a memory big
        {
            ddParent->Seek(ddFileHandle, sfh->baseoffset + offset, whence, NULL, NULL);
        }

        // if the inflator object is already created, delete it as
        // we'll be starting a new stream that could have different
        // parameters for the read
        if (sfh->inf != NULL)
        {
            sfh->inf->~ICodec();
            mAllocator->Free(sfh->inf); sfh->inf = NULL;
            mAllocator->Free(sfh->tmpZbuf); sfh->tmpZbuf = NULL;
        }

        sfh->curroffset = offset;
    }
    else
    {
        // If we are in the middle of a chunked decompression we 
        // just pretend that we've performed the seek. This happens
        // in the case where the filesystem quanta is less than 
        // the size of the files we want to decompress.
        if (sfh->chunkDecompressionClosure != NULL)
            return offset;

        // We aren't going to support seeking on compressed files.
        // Assert that this is a redundant seek.  If not, the user
        // has screwed up, or perhaps wants to be able to perform
        // random seeks on compressed streams, which we are going
        // to disallow, because the performance would be horribly
        // slow
        EA_ASSERT_MSG(sfh->inf != NULL,
            "Bigfile streamed decompressor has gotten into a bad state.  Contact support");
        EA_ASSERT_MSG(offset == sfh->inf->GetTotalOut(),
            "This implementation does not support random seeks on compressed files.");

        // do the seek anyway, in case the read pointer got moved somewhere else.
        // In all other cases, this is redundant
        if (!sfh->pBigfile->bigdata) //avoid seek if this is a memory big
        {
            ddParent->Seek(ddFileHandle, sfh->baseoffset + sfh->curroffset,
                whence, NULL, NULL);
        }
    }

    // return offset to the subfile, not the parent file
    return offset;
}

#ifdef RWBIGFILE_CHUNKED_DECOMPRESSION_SUPPORTED
// If the rwfilesystem quanta is smaller than the chunk size we need to iteratively decompress
// the file as the main filesystem scaffolding will call into our Read() function repeatedly
// giving us the quantum size and do its own pointer math on the destination buffer. This 
// throws a wrench into the works so we need to make the chunk decompression code re-entrant,
// which is what this class takes care of.
class ChunkDecompressionClosure
{
    ICoreAllocator *mAllocator;
    void *mCompressedDataBuffer;
    size_t mCompressedBufferSize;
    codec::ChunkReader mChunkReader;
    codec::DecompressionGroupHandle mDecompressionGroup;
    void *mLastBufferSlice;
    size_t mLastBufferSliceSize;
    size_t mAccumulatedBufferSliceSize;
    const char *mInputBufferCursor;
    char *mOutputBufferCursor;
    size_t mCurrentChunk;

    void ReadNextChunkInfo()
    {
        mInputBufferCursor += mChunkReader.GetExtraOffsetBeforeChunkInfo(mInputBufferCursor);
        mChunkReader.ReadChunkInfo(mInputBufferCursor);
        mInputBufferCursor += mChunkReader.GetChunkInfoSize();
    }

    void Initialize(void *buffer)
    {
        EA_ASSERT(mLastBufferSliceSize == 0);

        mInputBufferCursor = static_cast<char *>(mCompressedDataBuffer);
        mOutputBufferCursor = static_cast<char *>(buffer);
        mChunkReader.ReadFileHeader(mCompressedDataBuffer);
        mInputBufferCursor += mChunkReader.GetFileHeaderSize();

        mCurrentChunk = 0;

        ReadNextChunkInfo();
    }

public:
    ChunkDecompressionClosure(BigHandler::MemberFileStruct *sfh, 
            codec::ChunkDecompressor *decompressor,
            ICoreAllocator *allocator,
            filesys::DeviceDriver *parentDeviceDriver,
            void *deviceDriverFileHandle)
        : mAllocator(allocator),
          mCompressedBufferSize(static_cast<size_t>(sfh->zLength)),
          mChunkReader(decompressor->GetHandle()),
          mLastBufferSlice(NULL),
          mLastBufferSliceSize(0),
          mAccumulatedBufferSliceSize(0),
          mInputBufferCursor(NULL),
          mOutputBufferCursor(NULL),
          mCurrentChunk(0)
    {
        mCompressedDataBuffer = mAllocator->Alloc(AlignUp(mCompressedBufferSize, 16), 
                "rw::core::BigHandler::temp compressed data", 
                MEM_TEMP, 
                16);
        mDecompressionGroup = mChunkReader.GetNewDecompressionGroupHandle();
        parentDeviceDriver->Read(deviceDriverFileHandle, 
                mCompressedDataBuffer, 
                static_cast<unsigned int>(mCompressedBufferSize), 
                NULL, 
                NULL);
    }

    bool Iterate(void *buffer, size_t size, size_t &amountProcessed)
    {
        EA_ASSERT_MSG(!mLastBufferSlice || static_cast<char *>(mLastBufferSlice) + mLastBufferSliceSize == static_cast<char *>(buffer),
                "Error! Attempting to read data into non-contiguous output buffers!");

        if (!mLastBufferSlice)
            Initialize(buffer);

        mAccumulatedBufferSliceSize += size;

        char * const outputBufferBegin = mOutputBufferCursor;

        while (mAccumulatedBufferSliceSize >= mChunkReader.GetCurrentChunkDataDecompressedSize() 
                && mCurrentChunk < mChunkReader.GetNumberOfSegments())
        {
            const size_t inputBufferSizeRemaining = mCompressedBufferSize - 
                (reinterpret_cast<uintptr_t>(mInputBufferCursor) - reinterpret_cast<uintptr_t>(mCompressedDataBuffer));

            mChunkReader.DecompressChunkDataAsync(mOutputBufferCursor, 
                    mAccumulatedBufferSliceSize, 
                    mInputBufferCursor, 
                    inputBufferSizeRemaining,
                    mDecompressionGroup);

            const size_t currentChunkCompressedSize = mChunkReader.GetCurrentChunkDataCompressedSize();
            const size_t currentChunkDecompressedSize = mChunkReader.GetCurrentChunkDataDecompressedSize();
            mInputBufferCursor += currentChunkCompressedSize;
            mOutputBufferCursor += currentChunkDecompressedSize;
            mAccumulatedBufferSliceSize -= currentChunkDecompressedSize;

            ++mCurrentChunk;

            if (mCurrentChunk < mChunkReader.GetNumberOfSegments())
                ReadNextChunkInfo();
        }

        mLastBufferSlice = buffer;
        mLastBufferSliceSize = size;

        // If we don't actually perform any decompression because, say,
        // the size provided is less than our chunk size, we cannot return 
        // zero otherwise the filesystem will think we've reached EOF. 
        // The "amount decompressed" value here is used to ensure that the 
        // AsyncOp result size is consistent with the file size and not the
        // buffer sizes that are provided.
        const size_t amountDecompressed = static_cast<size_t>(mOutputBufferCursor - outputBufferBegin);
        amountProcessed = amountDecompressed > 0 && amountDecompressed < size ? amountDecompressed : size;

        return mCurrentChunk != mChunkReader.GetNumberOfSegments();
    }

    size_t GetDecompressedSize() 
    {
        return mChunkReader.GetDecompressedSize();
    }

    ~ChunkDecompressionClosure()
    {
#if defined(EA_PLATFORM_PS3) && (RW_CODEC_VERSION < RW_CODEC_CREATE_VERSION_NUMBER(1,5,0))
        // SINGLEREADER_SleepOnAllDecompressions is deprecated in rwcodec-1.05.00 but 
        // it is required if the user is building with rwcodec-1.04.00.
        mChunkReader.SINGLEREADER_SleepOnAllDecompressions();
#else
        mChunkReader.SleepOnDecompressionGroup(mDecompressionGroup);
#endif
        mChunkReader.ReleaseDecompressionGroupHandle(mDecompressionGroup);
        mAllocator->Free(mCompressedDataBuffer);
        mCompressedDataBuffer = NULL;
    }
};
#endif


//------------------------------------------------------------------------
unsigned int BigHandler::Read(void * vfh, void *pbuf, unsigned int bufsize, DeviceDriver *ddParent, void * ddFileHandle)
{
    MemberFileStruct *sfh = reinterpret_cast<MemberFileStruct *>(vfh);

    EA_ASSERT_FORMATTED(sfh->signature == OPENFILESIGNATURE, ("invalid signature"));
    uint32_t nread = 0;

    // data is not compressed
    if (sfh->fileflags == BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_NONE)
    {
        // limit the number of bytes read to the length of the member file
        bufsize = MIN(bufsize, static_cast<unsigned int>(sfh->length - sfh->curroffset));

        if (sfh->pBigfile->bigdata)
        {
            Memcpy(pbuf, sfh->pBigfile->bigdata + sfh->baseoffset + sfh->curroffset, bufsize);
            nread = bufsize;
        }
        else
        {
            nread = ddParent->Read(ddFileHandle, pbuf, bufsize, NULL, NULL);
        }
        sfh->curroffset += nread;
    }
    else if (sfh->fileflags == BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_REFPACK)
    {
        if (((sfh->modeflags & FOPEN_DECOMPRESS_IN_PLACE) == FOPEN_DECOMPRESS_IN_PLACE) &&
            // The caller wants to use in-place decompression, so lets ensure
            // the buffer is large enough to support that.  The amount of
            // space required is the size of the decompressed file plus 
            // DECOMPRESS_IN_PLACE_OVERLAP_BYTES plus READ_BUFFER_ALIGNMENT
            (bufsize >= sfh->length +
                filesys::ScatterLoad::OVERALLOCATION_SIZE +
                RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT))
        {
            // pZBuf is the location that the physical device will
            // read the compressed data into.  The data should be read to the
            // *end* of the user buffer.  The length of the buffer passed must be
            // at least filesys::DECOMPRESS_IN_PLACE_OVERLAP_BYTES plus 
            // RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT bytes larger than the actual size
            // required to hold the uncompressed data buffer.  This allows us to
            // align the read buffer to RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT byte-offset
            // and still leave room for the minimum number of overlap bytes.
            void *pZBuf = static_cast<uint8_t *>(pbuf) +
                ((bufsize - sfh->zLength) & ~(RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT-1));

            // now read the data from the physical
            // device to the *end* of the user buffer
            if (sfh->pBigfile->bigdata)
            {
                Memcpy(pZBuf, sfh->pBigfile->bigdata + sfh->baseoffset + sfh->curroffset, static_cast<size_t>(sfh->zLength));
                nread = static_cast<uint32_t>(sfh->zLength);
            }
            else
            {
                nread = ddParent->Read(ddFileHandle, pZBuf, static_cast<uint32_t>(sfh->zLength), NULL, NULL);
            }
            sfh->curroffset += nread;

#if (RW_FILESYS_VERSION < 1012000) || defined(EA_ASSERT_ENABLED)
            // Now decompress the data to the start of the buffer
            codec::RefpackBuf refpackBuf;
#endif

#if defined(EA_ASSERT_ENABLED)
            // the return value should equal the size of the decompressed member file
            EA_ASSERT_FORMATTED(nread == static_cast<uint32_t>(sfh->zLength), ("incorrect number of bytes read"));

            // validate that the data will decompress to the same size
            // that the TOC said it would.
            EA_ASSERT_MSG(refpackBuf.GetDecompressedSize(pZBuf) == sfh->length,
                "Possible TOC corruption - TOC uncompressed size doesn't match file's actual size");
#endif

#if RW_FILESYS_VERSION >= 1012000

            // update information on this member file for the decompression
            pContext = static_cast<BigFileAsyncReadProcessContext *>(
                mAllocator->Alloc(sizeof(BigFileAsyncReadProcessContext), NULL, 0));

            pContext->dstBuf = pbuf;
            pContext->srcBuf = pZBuf;

            // This somewhat hackish return value indicates to rwfilesystem
            // that the operation's I/O portion is complete and more ops can
            // be scheduled on this device, but there is further processing
            // to do before the data is returned to the caller.  This will be
            // done on another thread.
            nread = (unsigned int)(-1);
#else
            // decompress in the read thread, which blocks while the
            // decompression happens, thereby delaying the start of the next
            // read.  On systems which have a read-ahead buffer (e.g. PS3 Blu-
            // Ray) this won't matter, but if there is no read-ahead buffer,
            // such as on the Xbox 360's hard disk, it can cause a performance
            // impact.  To avoid that, use rwfilesystem 1.12.00 or later, which
            // supports parallel decompression & I/O.
            nread = refpackBuf.Decompress(pbuf, pZBuf);
#endif
        }
        else
        {
            uint64_t Z_TEMP_BUF_SIZE = (32*1024);

            if (bufsize >= sfh->length)
            {
                // If the user's buffer is big enough to hold all the
                // output data, allocate a temp buffer that's large enough
                // to hold all the compressed input data.  Note that if you
                // don't do this, and you're using the refpack decompressor,
                // the data won't decompress properly because the decompressor
                // will use an invalid optimization path where it assumes all
                // the input data is also available.  By making this buffer
                // large enough to hold all the input data, that problem goes
                // away.  Use refpack's in-place decompression if this blows
                // your heap budget.
                Z_TEMP_BUF_SIZE = (sfh->zLength + 2048-1) & ~(2048-1);
            }

            if (sfh->inf == NULL)
            {
                sfh->inf = static_cast<rw::core::codec::RefpackInflate *>
                    (mAllocator->Alloc(sizeof(rw::core::codec::RefpackInflate), 
                    "RefpackInflate", MEM_TEMP));
                new(sfh->inf) rw::core::codec::RefpackInflate(mAllocator);
                sfh->tmpZbuf = mAllocator->Alloc(static_cast<size_t>(Z_TEMP_BUF_SIZE),
                    "rw::core::BigHandler::temp Z buf", MEM_TEMP,
                    RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT);
            }

            codec::ICodec::Status status = codec::ICodec::STREAM_OK;

            unsigned int prevTotalOut = static_cast<unsigned int>(sfh->inf->GetTotalOut());

            EA_ASSERT_MSG(sfh->inf->GetOutputBytesAvailable() == 0,
                "Internal error in decompressor.  Contact support");

            sfh->inf->SetOutputBuffer(pbuf, bufsize);

            while ((status == codec::ICodec::STREAM_OK) &&
                (sfh->inf->GetOutputBytesAvailable() != 0))
            {
                // check to see whether we need to feed more data into the decompressor
                // We might have some left over from a previous iteration
                if (sfh->inf->GetInputBytesAvailable() == 0)
                {
                    // need some more source data.  limit the number of bytes
                    // read to the length of the member file

#if defined(EA_PLATFORM_REVOLUTION)
                    uint64_t nBytesToRead = Z_TEMP_BUF_SIZE;
                    uint64_t bytesRemaining = sfh->zLength - sfh->curroffset;
                    if (bytesRemaining < nBytesToRead )
                        nBytesToRead = bytesRemaining;
#else
                    uint64_t nBytesToRead = MIN(
                        sfh->zLength - sfh->curroffset, Z_TEMP_BUF_SIZE);
#endif

                    // if no more data could be read, we're done.
                    if (nBytesToRead > 0)
                    {
                        if (sfh->pBigfile->bigdata)
                        {
                            Memcpy(sfh->tmpZbuf, sfh->pBigfile->bigdata + sfh->baseoffset + sfh->curroffset, static_cast<size_t>(nBytesToRead));
                            nread = static_cast<uint32_t>(nBytesToRead);
                        }
                        else
                        {
                            nread = ddParent->Read(ddFileHandle, sfh->tmpZbuf, static_cast<uint32_t>(nBytesToRead), NULL, NULL);
                        }
                        sfh->curroffset += nread;

                        EA_ASSERT_MSG(nread == nBytesToRead, "Incorrect number of bytes were read");

                        sfh->inf->SetInputBuffer(sfh->tmpZbuf, static_cast<uint32_t>(nBytesToRead));
                    }
                }

                status = sfh->inf->Exec(0);
            }

            nread = static_cast<unsigned int>(sfh->inf->GetTotalOut()) - prevTotalOut;
        }
    }
#ifdef RWBIGFILE_CHUNKED_DECOMPRESSION_SUPPORTED
    else if (sfh->fileflags == BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_REFPACK_CHUNKED ||
         sfh->fileflags == BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_ZLIB_CHUNKED ||
         sfh->fileflags == BIGFILE_FORMAT_FLAG_COMPRESSION_TYPE_LZX)
    {
        if (!sfh->chunkDecompressionClosure) 
        {
            sfh->chunkDecompressionClosure = new (mAllocator->Alloc(sizeof(ChunkDecompressionClosure), 
                        "rw::core::BigHandler::chunk decompression state", 
                        MEM_TEMP, 
                        16))
                ChunkDecompressionClosure(sfh, mChunkDecompressor, mAllocator, ddParent, ddFileHandle);
        }
        ChunkDecompressionClosure &CDC = *sfh->chunkDecompressionClosure;

        size_t amountDecompressed = 0;
        if (!CDC.Iterate(pbuf, static_cast<size_t>(bufsize), amountDecompressed))
        {
            sfh->chunkDecompressionClosure->~ChunkDecompressionClosure();
            mAllocator->Free(sfh->chunkDecompressionClosure);
            sfh->chunkDecompressionClosure = NULL;
        }

        nread = static_cast<unsigned int>(amountDecompressed);
    }
#else
    else
    {
        EA_FAIL_MSG("This file has been compressed with a chunked codec which cannot be decompressed due to "
                "platform incompatibility or supporting library (rwcodec) incompatibility.");
    }
#endif

    return nread;
}

uint64_t    BigHandler::GetSize(void * vfh)
{   
    MemberFileStruct *sfh = reinterpret_cast<MemberFileStruct *>(vfh);

    EA_ASSERT_FORMATTED(sfh->signature == OPENFILESIGNATURE, ("invalid signature"));

    return sfh->length;
}

uint64_t    BigHandler::QueryLocation(void * vfh)
{
    MemberFileStruct *sfh = reinterpret_cast<MemberFileStruct *>(vfh);

    EA_ASSERT_FORMATTED(sfh->signature == OPENFILESIGNATURE, ("invalid signature"));

    return sfh->pBigfile->location + sfh->baseoffset;
}

int BigHandler::ExecuteProcessingFunction(void *pContext, unsigned int *numRead)
{
    BigFileAsyncReadProcessContext *pRead =
        static_cast<BigFileAsyncReadProcessContext *>(pContext);

    codec::RefpackBuf refpackBuf;

    *numRead = refpackBuf.Decompress(pRead->dstBuf, pRead->srcBuf);

    mAllocator->Free(pRead);

    return 0;
}

rw::core::filesys::Device *BigHandler::ReadProcessDevice()
{
    return gpBigHandlerDecompressDevice;
}

void* BigHandler::FindBegin(const char *pattern, filesys::DeviceDriver::FindData *findData)
{
    // Process the pattern, we don't want the initial device on it
    const char *patternPtr = Strchr(pattern, ':');

    if( patternPtr == NULL )
    {
        // no device specifier, use whole pattern then
        patternPtr = pattern;
    }
    else
    {
        // Skip the :
        ++patternPtr;

        // Also skip initial \, so that big:\*.txt just
        // resolves to *.txt as our pattern matcher expects
        if( *patternPtr == '\\' || *patternPtr == '/' )
            ++patternPtr;
    }

    EA::Thread::AutoFutex autoFutex( mBigFileListMutex );

    // Allocate a new findhandle structure
    FindStruct *bfh = new (mAllocator->Alloc(sizeof(FindStruct),
        "BigHandler::FindStruct", MEM_TEMP))
        FindStruct(patternPtr, mBigfileList.GetHead());

    bool bResult = FindNext(bfh, findData);

    if( !bResult )
    {
        // Didn't even find a first file, just FindEnd and clean up
        FindEnd(bfh);
        return NULL;
    }

    return bfh;
}

bool BigHandler::FindNextWithFAT(void *hFind, rw::core::filesys::DeviceDriver::FindData *findData)
{
    using namespace EA::Endian;
    using namespace rw::core;

    FindStruct *bfh = reinterpret_cast<FindStruct*>(hFind);
    if (bfh->pBigFile == NULL)
        return false;

    Big &big = bfh->pBigFile->mBig;

    uint32_t currentDirectoryEntry = 0;
    uint32_t currentEntryIndex = 0;
    const BigFileDirectoryEntry *BFD = NULL;
    const uint32_t *constituents = NULL;

    // First time in:
    // - work your way through the directory structure following the path laid out
    //   in the pattern
    // - reset the pattern to remove the path information because at this point it
    //   is moot, we just need the wildcard information.
    // - set the current file to be the first one in the directory
    // 
    // Other times in:
    // - figure out what directory you are in and where you are in that directory
    // - set the current file to the one *past* the last one
    //
    // In all cases:
    // - starting at the current file and for every constituent in the directory,
    //   see if you have a match.
    if (bfh->fatDirectoryEntry == ~0U)
    {
        MemFill32(bfh->TrimmedPattern, 0, sizeof bfh->TrimmedPattern);

        const char *patternBegin = bfh->Pattern;
        const char *slash = patternBegin;

        BFD = big.GetEntryForIndex(0);

        for (; (slash = Strchr(slash, '/')) != 0; patternBegin = ++slash)
        {
            // Are we searching from the root? For example, does our pattern
            // look like "/blat/*"? If so, skip past the first slash.
            if (slash == &bfh->Pattern[0])
                continue;

            EA_ASSERT(big_endian(BFD->EntryType) == BIGFILE_FAT_DIRECTORY);

            const uint32_t *BFDconstituents = big.GetConstituents(*BFD);
            const uint32_t numConstituents = big_endian(BFD->NumConstituents);

            bool found = false;
            for (uint32_t i = 0; i < numConstituents; ++i)
            {
                const BigFileDirectoryEntry *constituentBFD = big.GetEntryForIndex(big_endian(BFDconstituents[i]));
                const char *str = big.GetFATStringForIndex(big_endian(constituentBFD->NameIndex));
                if (Memcmp(str, patternBegin, static_cast<size_t>(slash - patternBegin)) == 0)
                {
                    if (big_endian(constituentBFD->EntryType) == BIGFILE_FAT_DIRECTORY)
                    {
                        BFD = constituentBFD;
                        found = true;
                        break;
                    }
                }
            }

            if (!found)
                goto ADVANCE_TO_NEXT_BIGFILE;
        }

        EA_ASSERT(*patternBegin != '/');
        Strcpy(bfh->TrimmedPattern, patternBegin);
    }
    else
    {
        const BigFileDirectoryEntry *currentFileBFD = big.GetEntryForIndex(bfh->fatDirectoryEntry);
        currentDirectoryEntry = big_endian(currentFileBFD->ContainingNodeIndex);
        BFD = big.GetEntryForIndex(currentDirectoryEntry);

        EA_ASSERT(big_endian(BFD->EntryType) == BIGFILE_FAT_DIRECTORY);

        constituents = big.GetConstituents(*BFD);

        for (uint32_t i = 0; i < big_endian(BFD->NumConstituents); ++i)
        {
            if (big_endian(constituents[i]) == bfh->fatDirectoryEntry)
            {
                currentEntryIndex = i + 1;
                break;
            }
        }
    }

    EA_ASSERT(big_endian(BFD->EntryType) == BIGFILE_FAT_DIRECTORY);

    constituents = big.GetConstituents(*BFD);
    for (uint32_t i = currentEntryIndex; i < big_endian(BFD->NumConstituents); ++i)
    {
        const uint32_t constituentIndex = big_endian(constituents[i]);
        const BigFileDirectoryEntry *entry = big.GetEntryForIndex(constituentIndex);

        const char *str = big.GetFATStringForIndex(big_endian(entry->NameIndex));

        if (SearchStringMatches(str, bfh->TrimmedPattern))
        {
            // TODO: there has to be a better way to do this:
            bfh->fatDirectoryEntry = static_cast<uint32_t>(entry - big.GetEntryForIndex(0));
            findData->mFlags = filesys::DirectoryEntry::FLAG_READONLY;

            Strcpy(findData->mName, str);

            if (big_endian(entry->EntryType) == BIGFILE_FAT_DIRECTORY)
            {
                findData->mFlags |= filesys::DirectoryEntry::FLAG_DIRECTORY;
                findData->mSize = 0;
            }
            else
            {
                uint64_t entrySize;
                int index = static_cast<int>(big_endian(entry->FileIndex));
                big.FindFileByIndex(index, NULL, &entrySize);
                findData->mSize = entrySize;
            }

            return true;
        }
    }

ADVANCE_TO_NEXT_BIGFILE:
    bfh->fatDirectoryEntry = ~0U;
    bfh->pBigFile = bfh->pBigFile->next;
    return FindNext(hFind, findData);
}


bool BigHandler::FindNext(void *hFind, rw::core::filesys::DeviceDriver::FindData *findData)
{
    EA_ASSERT_MSG( hFind != NULL, "Find handle is NULL");
    EA_ASSERT_MSG( findData != NULL, "Internal error - findData is NULL");

    FindStruct *bfh = reinterpret_cast<FindStruct*>(hFind);

    if( bfh->pBigFile == NULL )
    {
        // no next file
        return false;
    }

    for (;;)
    {
        // If we have a FAT then we use it to perform our scan. This has to be checked
        // first because if we do it after the index advancement below we may abort the
        // find process prematurely.
        {
            Big &big = bfh->pBigFile->mBig;
            if (bfh->pBigFile->headerformat == Big::BFTYPE_FAST && big.HasFAT())
                return FindNextWithFAT(bfh, findData);
        }

        // Move the currentfile index on
        while( ++bfh->fileIndex >= bfh->pBigFile->mBig.GetCount() )
        {
            // Move to the next big file in our list
            bfh->pBigFile = bfh->pBigFile->next;
            if( bfh->pBigFile == NULL )
            {
                // No more big files to search
                return false;
            }

            // reset the index values to 
            bfh->fileIndex = -1;
            bfh->tocIdx = 0;
            bfh->tocPtr = NULL;
        }

        uint64_t entrySize = INT64_C(0);
        char filename[filesys::FILESYS_MAX_PATH];

        if (bfh->pBigFile->headerformat == Big::BFTYPE_FAST)
        {
            Big &big = bfh->pBigFile->mBig;
            EA_ASSERT_MSG((bfh->pBigFile->flags & ADDFLAG_LOAD_NAME_INDEX) == ADDFLAG_LOAD_NAME_INDEX, 
                "BigFile searches must have the name indexes loaded.  Pass the ADDFLAG_LOAD_NAME_INDEX to AddArchiveFromFile");

            big.FindNameByIndex(bfh->fileIndex, filename, sizeof(filename));
            big.FindFileByIndex(bfh->fileIndex, NULL, &entrySize);
        }
        else
        {
            Strcpy(filename, bfh->pBigFile->mBig.LocateEntry(
                NULL, bfh->fileIndex, NULL, &entrySize, &bfh->tocPtr, &bfh->tocIdx));
        }

        CleanPath(filename);

        if( SearchStringMatches(filename, bfh->Pattern) )
        {
            // we just need the filename portion
            const char *pFile = filename + Strlen(filename);
            while( pFile > filename && ( *(pFile-1)!='\\' && *(pFile-1)!='/' ) )
            {
                --pFile;
            }

            Strcpy(findData->mName, pFile);

            findData->mFlags = rw::core::filesys::DirectoryEntry::FLAG_READONLY;
            findData->mSize = entrySize;

            return true;
        }
    }
}

bool BigHandler::FindEnd(void *hFind)
{
    if( hFind != NULL )
    {
        FindStruct *bfh = reinterpret_cast<FindStruct*>(hFind);

        // Destruct ( it was inplace newed )
        bfh->~FindStruct();
        mAllocator->Free(bfh);

        return true;
    }

    return false;
}

// - convert all backslashes to forward slashes
// - ensure the path doesn't end with a directory separator
void BigHandler::CleanPath( char *path )
{
    char *c = path;

    // convert any backslashes to forward slashes
    while( *c )
    {
        if( *c == '\\' )
            *c = '/';

        ++c;
    }

    // remove trailing dirsep
    while( (c > path) && (*(c-1)=='/') )
    {
        --c;
    }

    // NULL-terminate the path
    *c = 0;
}

bool BigHandler::SearchStringMatches(const char *s, const char *pattern)
{
    if(Strlen(s) == 0)
    {
        if (Strlen(pattern) == 0) 
        {
            return true;
        }
        return false;
    }
    if (Strlen(pattern) == 0) 
    {
        return false;
    }

    const char *c1 = s;
    const char *c2 = pattern;

    while ((*c1 == *c2) || (*c2 == '?') || (*c2 == '*'))
    {
        if ((*c1 == *c2) || (*c2 == '?')) 
        {
            ++c1;
            ++c2;
        } 
        else if (*c2 == '*')
        {
            ++c2;
            if (*c2 == 0)
            {
                return true;
            }
            while (*c1 != 0)
            {
                if (SearchStringMatches(c1, c2))
                {
                    return true;
                }
                ++c1;
            }
        }
        if (*c1 == 0) 
        {
            if (*c2 == 0) 
            {
                return true;
            }
            return false;
        }
        if (*c2 == 0) 
        {
            return false;
        }
    }
    return false;
}

BigHandler::FindStruct::FindStruct(const char *pattern, BigFileStruct *bigFile)
: pBigFile(bigFile), fileIndex(-1), tocIdx(0), tocPtr(NULL), fatDirectoryEntry(~0U)
{
    Strcpy(Pattern, pattern);
    CleanPath(Pattern);
}

uint64_t BigHandler::FindEntry( const char* name, 
                                void** entryArchive/* = NULL*/,
                                uint64_t* entryOffset/* = NULL*/,
                                uint64_t* entrySize/* = NULL*/,
                                uint64_t* zSize/* = NULL*/,
                                int* tocIndex/* = NULL*/,
                                unsigned int *entryFlags/* = NULL*/)
{
    uint64_t offset = 0;
    bool searchall;
    char bigfn[filesys::FILESYS_MAX_PATH];

    // skip over the device name, if present.
    const char *filename = Strchr(name, ':');
    if (filename != NULL)
        name = filename + 1;

    // skip over the first / character.
    if (name[0] == '/' || name[0] == '\\')
        ++name;

    const char *fn = Strchr(name, '|');   // locate unqualified portion

    // set up the bigfile search
    // BIG_name is the name of container file to search in
    // (if blank, all active bigfiles are searched)
    char *BIG_name = bigfn;

    // filename is the name of the subfile to search for
    filename = name;

    // if file has a big file qualifier then extract it
    if (fn)
    {
        Strncpy(BIG_name, name, (unsigned long)(fn - name));  // copy target big file's name
        BIG_name[fn - name] = '\0';          // null-terminate it
        filename = fn + 1;                   // select unqualified filename
        searchall = false;
    }
    else
    {
        BIG_name[0] = 0;
        searchall = true;
    }

    EA::Thread::AutoFutex autoFutex( mBigFileListMutex );

    BigFileStruct *Itr = mBigfileList.GetHead();

    while (Itr != NULL)
    {
        // Skip over any device name on the loaded bigfile
        char * filenameNoDevice = Strchr(Itr->name, ':');
        if(filenameNoDevice == 0)
            filenameNoDevice = Itr->name;
        else
            filenameNoDevice++; // skip over ':'

        if (searchall || !Strcmp(filenameNoDevice, BIG_name))
        {
            unsigned int locateFlags = Big::LOCATE_OPT_NONE;

            if ((Itr->flags & ADDFLAG_SLASH_CONVERT_FORWARD) != 0)
            {
                locateFlags |= Big::LOCATE_OPT_SLASH_CONVERT_FORWARD;
            }
            else if ((Itr->flags & ADDFLAG_SLASH_CONVERT_BACKWARD) != 0)
            {
                locateFlags |= Big::LOCATE_OPT_SLASH_CONVERT_BACK;
            }

            if ((Itr->flags & ADDFLAG_NAME_CONVERT_LOWERCASE) != 0)
            {
                locateFlags |= Big::LOCATE_OPT_NAME_CONVERT_LOWERCASE;
            }
            else if ((Itr->flags & ADDFLAG_NAME_CONVERT_UPPERCASE) != 0)
            {
                locateFlags |= Big::LOCATE_OPT_NAME_CONVERT_UPPERCASE;
            }

            // either we are searching all big files, or this is the target big file
            if (Itr->headerformat == Big::BFTYPE_HASH16 ||
                Itr->headerformat == Big::BFTYPE_HASH20 ||
                Itr->headerformat == Big::BFTYPE_FAST)
            {
                if (Itr->mBig.FindFileByName(filename, &offset, entrySize, zSize, tocIndex, locateFlags, entryFlags))
                {
                    break;
                }
            }
            else if (Itr->mBig.LocateEntry(filename, 0, &offset, entrySize, NULL, tocIndex, locateFlags))
            {
                break;
            }
        }

        Itr = Itr->next;
    }

    if (entryArchive != 0 && offset != 0)
    {
        *entryArchive = static_cast<void*>(Itr);
    }

    if (entryOffset != 0)
    {
        *entryOffset = offset;
    }

    return offset;
}


}   // namespace core
}   // namespace rw
