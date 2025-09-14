// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implementation of the ScatterLoad class
*/

#include "EABase/eabase.h"
#include "EAAssert/eaassert.h"
#include <EAStdC/EAString.h>
#include "rw/core/filesys/scatterload.h"

#include "platform_sdk.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

namespace rw {
namespace core {
namespace filesys {

/*!
Timeout for Scatterload memory allocation.  If the initial allocation fails,
the thread will continue trying to allocate the block for up to this many
milliseconds.  If the allocation still fails, it throws an assert, or leads
to a write to NULL in a release build.  Increase or decrease this value as
you see fit.
*/
const EA::Thread::ThreadTime SCATTERLOAD_MEMALLOC_TIMEOUT = 1000;

// Information about an asset.
struct ScatterLoadFilePrivate
{
    // Pointer to output AssetInfo struct passed in by the user
    ScatterLoadFile *pAssetInfo;

    // Dynamically-allocated copy of the asset name so
    // the caller doesn't have to keep it around.
    char *name;

    // Memory address where file is loaded.
    void *mpAddr;

    // File offset (for sorting purposes, caller does not care about this)
    uint64_t offset;

    // File size, to be passed back to the caller
    uint64_t filesize;

    // memory block size (may be larger than file size, for
    // optimal DMA usage.
    size_t blocksize;

    // alignment offset of the file buffer
    unsigned int bufalign;
};


    static int Compare(const void *lhs, const void *rhs)
    {
        const ScatterLoadFilePrivate *pa = static_cast<const ScatterLoadFilePrivate *>(lhs);
        const ScatterLoadFilePrivate *pb = static_cast<const ScatterLoadFilePrivate *>(rhs);

        if (pa->offset > pb->offset)
            return 1;
        else if (pa->offset < pb->offset)
            return -1;
        else
            return 0;
    }

/*!
    This constructor starts an asynchronous load of a list of
    files.  Files will be loaded in an order that minimizes
    seeking.  The allocator will be used to allocate the
    memory for each file which is found.  The ScatterLoad object
    itself is allocated using the filesystem library Manager's
    allocator, not the allocator passed to this constructor.

    Any file buffers allocated by the Scatterloader must be
    released by the caller.  They are <b>not</b> freed when the
    ScatterLoad is destroyed.

    The object must remain valid until all files are loaded.
    The mFilePath member does not need to be kept valid because a
    copy of the name is made.  However, if you want to refer to
    the file name in the callback, the mFilePath members <b>must</b>
    stay valid until the callback has completed.

    \importlib rwfilesystem

    \param pAssetInfo   array of files to load
    \param listsize     how many entries are in the array.
    \param flags        0 or any combination of the ScatterLoadFlags.
    \param pAllocator   Allocator used for allocating the file buffers.  Must be
    thread-safe.  Releasing these buffers is the responsibility of the caller;
    they are not freed automatically when the ScatterLoad object is destroyed.
    The file buffer may be freed in the ScatterLoadFile callback function, or
    any time thereafter.
    \param maxOps       The maximum number of concurrent operations to use.  More
    operations may allow faster execution as the file system or the O/S is able
    to reorder operations to make the most efficient use of the disk I/O system.
    \sa ScatterLoadFile
*/
ScatterLoad::ScatterLoad(ScatterLoadFile *pAssetInfo, unsigned int listsize,
                         unsigned int flags, EA::Allocator::ICoreAllocator *pAllocator,
                         unsigned int maxOps) :
    mCompleteFlag(false),
    mNumFiles(0),
    mFlags(flags),
    mOpArray(),
    mNumOpsUsed(0),
    mCurr(0),
    mAipList(NULL),
    mComplete(),
    mAllocator(pAllocator)
{
    // Open all the files and sort all the files which were
    // successfully opened by increasing value of their location.

    // Allocate the AssetList object which contains the state
    // of the asset loads.  This object contains enough entries
    // for each asset the user wants to load.
    mAipList = static_cast<ScatterLoadFilePrivate *>(RWFILESYSTEM_ALLOCATE(
        listsize * sizeof(ScatterLoadFilePrivate)));

    memset(mAipList, 0, listsize * sizeof(ScatterLoadFilePrivate));

    ScatterLoadFilePrivate *aip = mAipList;

    for (unsigned int i = 0; i < listsize; ++i, ++pAssetInfo)
    {
        EA_ASSERT(pAssetInfo->GetFilePath()!=NULL);

        if ((flags & FLAG_SORT_FILES_BY_LOCATION) == FLAG_SORT_FILES_BY_LOCATION)
        {
            if (!GetLocationAndSize(pAssetInfo->GetFilePath(), &aip->offset, &aip->filesize))
            {
                continue;
            }
        }

        // keep a pointer to the output buffer
        aip->pAssetInfo = pAssetInfo;

        // keep a copy of the asset name so the user doesn't have to do that.
        // Note that this memory comes out of the default allocation pool.
        size_t aipNameLen = static_cast<size_t>(EA::StdC::Strlen(aip->pAssetInfo->GetFilePath()) + 1);
        aip->name = static_cast<char *>(RWFILESYSTEM_ALLOCATE(aipNameLen));

        EA_STRLCPY_VERIFY(aip->name, aip->pAssetInfo->GetFilePath(), aipNameLen);

        if (pAssetInfo->GetAddress() != NULL)
        {
            // user defined an address
            EA_ASSERT(pAssetInfo->GetFileSize() != 0);

            aip->mpAddr = pAssetInfo->GetAddress();
            aip->blocksize = static_cast<size_t>(pAssetInfo->GetFileSize());
        }
        else
        {
            EA_ASSERT_MSG(pAllocator!=NULL,
                "ScatterLoadFile: Allocator pointer passed to ScatterLoad is NULL.");
            EA_ASSERT_MSG(pAssetInfo->GetFileSize()==0,
                "ScatterLoadFile: File size must be zero when the buffer is not pre-allocated.");

            aip->bufalign = pAssetInfo->GetAlignment();

            // sanity check fileAlignment is a power of 2
            if(!(((aip->bufalign & (aip->bufalign - 1)) == 0) && aip->bufalign))
            {
                aip->bufalign = RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT;
            }

            // buffer is allocated in callback.
        }

        ++mNumFiles;    // increment the number of assets to load
        ++aip;          // increment the assetinfo pointer.
    }

    // Don't use more operations than there are files to be loaded
    unsigned int numOps = MIN(maxOps, mNumFiles);

    if (mNumFiles > 0)
    {
        if ((flags & FLAG_SORT_FILES_BY_LOCATION) == FLAG_SORT_FILES_BY_LOCATION)
        {
            // Sort the remaining files by offset
            qsort(mAipList, mNumFiles, sizeof(ScatterLoadFilePrivate), Compare);
        }

        mOpArray = static_cast<OpInfo *>(RWFILESYSTEM_ALLOCATE(numOps * sizeof(OpInfo)));

        // Set the current asset to the start of the list
        mCurr = 0;
        unsigned int opIndex = 0;

        // Start a load for each available operation
        while(opIndex != numOps)
        {
            // Must respect the fact that while the operations being
            // started here, files could be finished loading and move
            // onto the next one before the second and subsequent
            // operations get started because the background thread
            // schedules more file loads as long as there are still
            // unstarted loads.
            unsigned int currFile = mCurr++;
            if (currFile < mNumFiles)
            {
                // start an operation
                OpInfo *op = &mOpArray[opIndex++];

                // construct the operation in-place
                new (op) OpInfo(mAipList + currFile, this);
                op->mOp.Open(op->mFile->pAssetInfo->GetFilePath(),
                    op->mFile->pAssetInfo->GetFlags(), OpDoneCallback, op);
            }
            else
            {
                // no more operations to start (because some were
                // started by the file system device thread(s) while
                // this loop was still priming the first operation for
                // each OpInfo structure.
                break;
            }
        }

        mNumOpsUsed = opIndex;
    }
}

/*!
    The destructor frees the resources allocated by the
    ScatterLoad constructor.  This must not be called while there
    are still pending loads to be done.  Doing so will certainly
    cause race-condition bugs.  Wait for all the loads to complete
    with GetStatus before releasing the object.
    \sa GetStatus
    \importlib rwfilesystem
*/
ScatterLoad::~ScatterLoad()
{
    EA_ASSERT_MSG(mCompleteFlag,
        "Even if all the files have finished loading. call GetStatus() without blocking one time before destroying the ScatterLoad object");
    Manager::Free(mAipList); mAipList = NULL;

    // must call destructor for each operation to free its resources
    for (unsigned int i = 0; i < mNumOpsUsed; ++i)
    {
        (mOpArray + i)->~OpInfo();
    }
    Manager::Free(mOpArray); mOpArray = NULL;

    mAllocator = NULL;
}

// This function is called by the AsyncOp class each time an
// operation completes.
void ScatterLoad::OpDoneCallback(AsyncOp *op)
{
    OpInfo *currOp = static_cast<OpInfo *>(op->GetContext());
    ScatterLoad *lfl = currOp->mLoader;

    // TODO: support cancellation

    EA::Thread::ThreadTime allocTimeout =
        EA::Thread::GetThreadTime() + SCATTERLOAD_MEMALLOC_TIMEOUT;

    switch(currOp->mState)
    {
    case ALS_OPEN:
        currOp->mFileHandle = op->GetResultHandle();

        if (currOp->mFileHandle == NULL)
        {
            // file was not found; go on to the next one
            Manager::Free(currOp->mFile->name);
            lfl->StartNextFile(currOp);
            return;
        }

        // If the caller supplied a buffer, use that size
        currOp->mFile->filesize = filesys::GetSize(currOp->mFileHandle);

        if (currOp->mFile->blocksize == 0)
        {
            currOp->mFile->blocksize = static_cast<size_t>(currOp->mFile->filesize);

            // The caller wants some extra memory allocated at the end of
            // the buffer so the data may be decompressed in-place.
            if ((lfl->mFlags & FLAG_USE_OVERALLOCATION_SIZE) == FLAG_USE_OVERALLOCATION_SIZE)
            {
                currOp->mFile->blocksize += OVERALLOCATION_SIZE + RW_CORE_FILESYS_READ_BUFFER_ALIGNMENT;
            }

            if ((currOp->mFile->pAssetInfo->GetFlags() & AsyncOp::FOPEN_NOBUFFERING) ==
                AsyncOp::FOPEN_NOBUFFERING)
            {
                // round up the allocated memory buffer to the next
                // sector size
#if EA_WINAPI_FAMILY_PARTITION(EA_WINAPI_PARTITION_DESKTOP)
                DWORD BytesPerSector;
                GetDiskFreeSpace(NULL, NULL, &BytesPerSector, NULL, NULL);
                const size_t ALIGN_MASK = BytesPerSector - 1;
#else
                const size_t ALIGN_MASK = 2*1024 - 1;
#endif
                // round up the buffer size to the next multiple
                // of the sector size.
                currOp->mFile->blocksize = (currOp->mFile->blocksize + ALIGN_MASK) & ~ALIGN_MASK;
            }
        }

        currOp->mState = ALS_READ;

        while ((currOp->mFile->mpAddr == NULL) &&
            (EA::Thread::GetThreadTime() < allocTimeout))
        {
            // The user did not supply a buffer for this file, so allocate
            // one now.  The advantage of doing the allocation here in the
            // callback instead of in the ScatterLoad constructor is that
            // it requires less memory.  Each file can be loaded, processed,
            // and freed, which requires less memory than pre-allocating all
            // the buffers.

            currOp->mFile->mpAddr = lfl->mAllocator->Alloc(currOp->mFile->blocksize,
                currOp->mFile->name, EA::Allocator::MEM_PERM, currOp->mFile->bufalign);

            if (currOp->mFile->mpAddr == NULL)
            {
                // If the allocation failed, sleep for 1 millisecond, and try
                // again.  Maybe somethat some heap space got freed up in
                // another thread, so the next allocation attempt could succeed.
                EA::Thread::ThreadSleep(1);
            }
        }

        // assert that we have a buffer.  The loop above times out
        // after some period of time.
        EA_ASSERT_FORMATTED(currOp->mFile->mpAddr != NULL,
            ("Timeout expired trying to allocate a buffer for file %s",
            currOp->mFile->name));

        currOp->mOp.Read(currOp->mFileHandle, currOp->mFile->mpAddr,
            0, static_cast<uint64_t>(currOp->mFile->blocksize),
            OpDoneCallback, currOp);
        break;

    case ALS_READ:
        currOp->mState = ALS_CLOSE;
        currOp->mOp.Close(currOp->mFileHandle, OpDoneCallback, currOp);
        break;

    case ALS_CLOSE:
        Manager::Free(currOp->mFile->name);
        currOp->mFile->pAssetInfo->SetFileSize(currOp->mFile->filesize);
        currOp->mFile->pAssetInfo->SetAddress(currOp->mFile->mpAddr);
        currOp->mFile->pAssetInfo->SetDone();

        if (currOp->mFile->pAssetInfo->GetCallback() != NULL)
        {
            // execute the callback.
            currOp->mFile->pAssetInfo->GetCallback()(currOp->mFile->pAssetInfo,
                currOp->mFile->pAssetInfo->GetContext());
        }

        lfl->StartNextFile(currOp);
        break;
    }
}

void ScatterLoad::StartNextFile(OpInfo *op)
{
    unsigned int currFile = mCurr++;

    if (currFile < mNumFiles)
    {
        // there is another file in the list, get it started
        op->mState = ALS_OPEN;
        op->mFile = mAipList + currFile;
        op->mOp.Open(op->mFile->pAssetInfo->GetFilePath(),
            op->mFile->pAssetInfo->GetFlags(), OpDoneCallback, op);
    }
    else if (op->mFile == mAipList + mNumFiles - 1)
    {
        // The last one has completed; signal that we're done.
        mComplete.Post();
    }
}

/*!
    \brief Checks to see if all files in the asset list have been loaded.

    After this function returns true, the caller must free the
    ScatterLoadFile buffer using the file library Allocator object,
    not the ScatterLoad allocator.  If multiple threads are waiting
    for load operations to complete, please be sure that only one of
    them actually blocks.  The others must poll because a semaphore
    is used to signal that a set of loads is complete, and only one
    Post() call is made.  If multiple threads are blocking here, only
    one will be signalled when all the operations are complete.

    \param timeoutAbsolute timeout value for unreliable file systems.
    \importlib rwfilesystem
    \return ScatterLoad::OPSTATUS_COMPLETE if the load is complete,
    ScatterLoad::OPSTATUS_PENDING if not.
*/
ScatterLoad::OpStatus ScatterLoad::GetStatus(const EA::Thread::ThreadTime&
                                             timeoutAbsolute)
{
    if (!mCompleteFlag)
    {
        if (mComplete.Wait(timeoutAbsolute) !=
            static_cast<int>(EA::Thread::Semaphore::kResultTimeout))
        {
            mCompleteFlag = true;
        }
    }

    return mCompleteFlag ? OPSTATUS_COMPLETE : OPSTATUS_PENDING;
}

}}}
