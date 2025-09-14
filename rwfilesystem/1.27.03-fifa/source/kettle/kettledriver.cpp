// (c) Electronic Arts. All Rights Reserved.


#include "private_rwprofiler.h"
#include "rw/core/filesys/driverimpl.h"
#include "rw/core/filesys/directory.h"
#include "rw/core/filesys/path.h"

#include "EAAssert/eaassert.h"
#include "EAStdC/EAString.h"
#include "EAStdC/EASprintf.h"

#include <scebase_common.h>
#include <kernel.h>

#define INVALID_FILE_DESCR -1
#define CURRENT_DIR_ENTRY "."

namespace rw {
namespace core {
namespace filesys {

// class DeviceFileInfo contains everything that a device driver
// needs to keep track of for the open file.  Each platform will have
// different requirements here.
class DeviceFileInfo
{
public:
    static void *operator new(size_t size)
    {
        return RWFILESYSTEM_ALLOCATE(size);
    }

    static void operator delete(void *p)
    {
        return Manager::Free(p);
    }

    DeviceFileInfo(const char *nativeFileName, int fileDescr, int flags, int mode)
        : mFileDescriptor(fileDescr),
          mFlags(flags),
          mMode(mode),
          mCurrPos(0)
    {
        SetFileName(nativeFileName);
    }

    int GetDescriptor() const { return mFileDescriptor; }
    bool IsValidDescriptor() const { return mFileDescriptor >= 0; }
    void SetDescriptor(int fileDescr) { mFileDescriptor = fileDescr; }

    const char *GetName() const { return mNativeFileName; }
    
    int GetFlags() const { return mFlags; }
    int GetMode() const { return mMode; }
    
    void MovePointer(int amount) { mCurrPos += amount; }
    void SetPos(off_t pos) { mCurrPos = pos; }
    off_t GetPos() const { return mCurrPos; }

private:

    void SetFileName(const char *nativeFileName)
    {
        memset(mNativeFileName, 0, FILESYS_MAX_PATH);
        if (nativeFileName != NULL)
            EA_STRLCPY_VERIFY(mNativeFileName, nativeFileName, FILESYS_MAX_PATH);
    }

    int mFileDescriptor;
    char mNativeFileName[FILESYS_MAX_PATH];
    int mFlags;
    int mMode;
    off_t mCurrPos;
};

class DeviceDirectoryInfo
{
public:
    DeviceDirectoryInfo(int dirDescr, const char *dirName, int bufferSize)
        : mDirDescriptor(dirDescr),
          mBufferSize(bufferSize),
          mBytesRead(0),
          mCurrPos(0),
          mBuffer(NULL),
          mPattern(NULL)
    {
        memset(mNativeDirectoryName, 0, FILESYS_MAX_PATH);
        if (dirName != NULL)
            EA_STRLCPY_VERIFY(mNativeDirectoryName, dirName, FILESYS_MAX_PATH);

        mBuffer = static_cast<char*>(RWFILESYSTEM_ALLOCATE(mBufferSize));
    }

    ~DeviceDirectoryInfo()
    {
        Manager::Free(mBuffer);
        mBuffer = NULL;
    }

    static void *operator new(size_t size)
    {
        return RWFILESYSTEM_ALLOCATE(size);
    }

    static void operator delete(void *p)
    {
        return Manager::Free(p);
    }

    bool ReadMultipleEntries()
    {
        mBytesRead = sceKernelGetdents(mDirDescriptor, mBuffer, mBufferSize);
        mCurrPos = 0;
        return mBytesRead > 0;
    }

    SceKernelDirent *GetNextEntry()
    {
        while(mCurrPos < mBytesRead)
        {
            SceKernelDirent* dirEnt = reinterpret_cast<SceKernelDirent*>(mBuffer + mCurrPos);
            mCurrPos += dirEnt->d_reclen;

            if(dirEnt->d_fileno != 0) // If not deleted...
                return dirEnt;
        } 

        return NULL;
    }

    void SetPattern(char const * x) 
    { 
        size_t mNativeDirectoryNameLength = EA::StdC::Strlen(mNativeDirectoryName) + 1; //+1 to go *past* null terminator
        mPattern = mNativeDirectoryName + mNativeDirectoryNameLength;
        EA_STRLCPY_VERIFY(mPattern, x, FILESYS_MAX_PATH - mNativeDirectoryNameLength);
    }
    
    char const *GetPattern() const { return mPattern; }
    const char *GetName() const { return mNativeDirectoryName; }
    int GetDescriptor() const { return mDirDescriptor; }

private:
    int mDirDescriptor;
    char mNativeDirectoryName[FILESYS_MAX_PATH];
    int mBufferSize;
    int mBytesRead;
    int mCurrPos;
    char *mBuffer;
    char *mPattern;
};

KettleFileDeviceDriver::KettleFileDeviceDriver()
{
    // empty
}

// Bit ( Query, Set, Clr )
#define BQ(x, y) (((x)&(y))==(y))
#define BS(x, y) ((x)|(y))
#define BC(x, y) ((x)&(~y))

void*  KettleFileDeviceDriver::Open(const char *name, unsigned int rwflags, Handle **)
{
    int oflag = 0;
    SceKernelMode accessMode = SCE_KERNEL_S_IRWXU;

    char nativeFileName[FILESYS_MAX_PATH];
    ReplaceSlashes(nativeFileName, name);

    // choose access, sharing, creation, and attributes
    if (BQ(rwflags, AsyncOp::FOPEN_READWRITE))
    {
        oflag = SCE_KERNEL_O_RDWR;
    }
    else
    {
        oflag = SCE_KERNEL_O_RDONLY;
    }
    
    switch(rwflags & (AsyncOp::FOPEN_CREATE | AsyncOp::FOPEN_OVERWRITE))
    {
    case 0:
        /* createmode is already correct */ break;

    case AsyncOp::FOPEN_CREATE:
        // Create flag and make sure the file doesn't already exists
        oflag |= SCE_KERNEL_O_CREAT | SCE_KERNEL_O_EXCL;
        break;

    case AsyncOp::FOPEN_OVERWRITE:
        {
            // File must be opened for write operations
            if (!BQ(oflag, SCE_KERNEL_O_RDWR) && !BQ(oflag, SCE_KERNEL_O_WRONLY))
            {
                Manager::GetInstance()->GetErrorHandler()(SCE_KERNEL_ERROR_EACCES, Manager::FILEERROR_OPEN, this, nativeFileName);
                return NULL;
            }

            // File must exists
            SceKernelStat st;
            if (sceKernelStat(nativeFileName, &st) != SCE_OK)
            {
                Manager::GetInstance()->GetErrorHandler()(SCE_KERNEL_ERROR_ENOENT, Manager::FILEERROR_OPEN, this, nativeFileName);
                return NULL;
            }
        }

        oflag |= SCE_KERNEL_O_TRUNC;
        break;

    case AsyncOp::FOPEN_CREATE | AsyncOp::FOPEN_OVERWRITE:
		{
            // Create or truncate if exists
			oflag |= SCE_KERNEL_O_CREAT | SCE_KERNEL_O_TRUNC;
		}
        break;
    }

    // Data will be immediately written to storage without being cached. The write
    // operation will not return untile the writting has completed
    if (rwflags & AsyncOp::FOPEN_NOBUFFERING)
    {
        oflag |= SCE_KERNEL_O_FSYNC;
    }

    int descr = sceKernelOpen(nativeFileName, oflag, accessMode);
    
    if (descr >= 0) 
    {
        return new DeviceFileInfo(nativeFileName, descr, oflag, accessMode);
    }

    Manager::GetInstance()->GetErrorHandler()(descr, Manager::FILEERROR_OPEN, this, nativeFileName);

    return NULL;
}

#undef BQ
#undef BS
#undef BC


void KettleFileDeviceDriver::Close(void *fh)
{
    DeviceFileInfo *h = static_cast<DeviceFileInfo *>(fh);
    
    if (h->IsValidDescriptor())
    {
        // Kettle documentation: When a file is closed the cached data will not always be written to
        // storage. To explicitly write data left in the cache to storage use sceKernelSync() or
        // sceKernelFSync() ...
        int flags = h->GetFlags();
        int res =  0;
        if (flags & SCE_KERNEL_O_RDWR)
        {
            res = sceKernelFsync(h->GetDescriptor());
            EA_ASSERT(res == SCE_OK);
        }

        res = sceKernelClose(h->GetDescriptor());
        EA_ASSERT(res == SCE_OK);
    }

    delete h;
}

unsigned int KettleFileDeviceDriver::Read(void *fh, void *buf, unsigned int bufsize, DeviceDriver *, void *)
{
    DeviceFileInfo *dfi = static_cast<DeviceFileInfo *>(fh);
    ssize_t nbytes = 0;
    bool hadError = false;
    int fileDescr;

errorRetryRead:

    // return 0 bytes read if the handle was previously closed
    // (because of a read error)
    if (!dfi->IsValidDescriptor())
        return 0;

    fileDescr = dfi->GetDescriptor();
    
    // Touch the first & last bytes of the buffer before reading This allows
    // virtual memory to be committed via a structured exception handler.  The
    // performance impact should be negligible, so we won't have a
    // configuration option to enable/disable it.
    *((volatile char *)buf+0) = *((volatile char *)buf+0);
    *((volatile char *)buf+bufsize-1) = *((volatile char *)buf+bufsize-1);

    bool success = false;

    if (Manager::GetInstance()->GetErrorInjectionFunction()(this, dfi->GetName()))
    {
        nbytes = sceKernelRead(fileDescr, buf, bufsize);
        success = nbytes >= 0;
    }

    if (success == false)
    {
        hadError = true;

errorRetryReadClose:
        sceKernelClose(fileDescr);
        dfi->SetDescriptor(INVALID_FILE_DESCR);

errorRetryReadOpen:
        if (Manager::GetInstance()->GetErrorHandler()(
            static_cast<int>(nbytes), Manager::FILEERROR_READ, this, dfi->GetName()))
        {
            fileDescr = sceKernelOpen(dfi->GetName(), dfi->GetFlags(), dfi->GetMode());

            if (fileDescr < 0)
                goto errorRetryReadOpen;

            dfi->SetDescriptor(fileDescr);

            // seek to correct pos
            if (sceKernelLseek(fileDescr, dfi->GetPos(), SCE_KERNEL_SEEK_SET) < 0)
                goto errorRetryReadClose;

            goto errorRetryRead;
        }

        // The user has chosen not to retry the read.  The library will
        // return any data that was read up to the point at which the
        // failure occurred.  It is up to the application to provide a
        // fallback method of getting the rest of the data.

        // hadError will remain set, so the user will get a final error
        // callback stating success.  I hope that's not too misleading.
        // The "success" is that we are no longer blocking waiting on
        // data - we've given up and are now out of the wait loop.
    }

    if (hadError)
    {
        Manager::GetInstance()->GetErrorHandler()(success ? 0 : -1, Manager::FILEERROR_READ, this, dfi->GetName());
    }

    dfi->MovePointer(nbytes);
    return nbytes;
}

unsigned int KettleFileDeviceDriver::Write(void *fh, const void *buf, unsigned int bufsize, DeviceDriver *, void *)
{
    DeviceFileInfo *dfi = static_cast<DeviceFileInfo *>(fh);
    ssize_t nbytes = 0;

    EA_COMPILETIME_ASSERT(sizeof(int) == 4);

    bool success = false;

    while (success == false)
    {
        nbytes = sceKernelWrite(dfi->GetDescriptor(), buf, bufsize);
        success = nbytes >= 0;

        if (success != true)
        {
            // If the write fails, let the user know and give the game a chance
            // to repeat the operation as many times as they wish.

            if (Manager::GetInstance()->GetErrorHandler()(
                static_cast<int>(nbytes), Manager::FILEERROR_WRITE, this, dfi->GetName()))
            {
                // wait a 10 ms before retrying.  The failure may have occurred
                // due to a heavy load on the HDD and waiting for some time to
                // pass before retrying will allow the write to succeed.
                EA::Thread::ThreadSleep(10);
            }
            else
            {
                // do not retry any more.
                nbytes = 0;
                success = true;
            }
        }
    }

    dfi->MovePointer(nbytes);
    return nbytes;
}

uint64_t KettleFileDeviceDriver::Seek(void *fh, uint64_t offset, int whence, DeviceDriver *, void *)
{
    DeviceFileInfo *dfi = static_cast<DeviceFileInfo *>(fh);

    // the driver only needs to handle seeks from the origin because the caller
    // (the Async module) keeps track of the file position
    EA_ASSERT(whence == static_cast<int>(FSEEK_BEGIN));

    // don't bother doing anything if we're
    // already at the correct position.
    // Also don't do anything if the handle has been closed due to
    // a read error
    if (static_cast<off_t>(offset) == dfi->GetPos() || !dfi->IsValidDescriptor())
    {
        return offset;
    }

    off_t resOffset = sceKernelLseek(dfi->GetDescriptor(), static_cast<off_t>(offset), SCE_KERNEL_SEEK_SET);

    if (resOffset < 0)
    {
        Manager::GetInstance()->GetErrorHandler()(static_cast<int>(resOffset), Manager::FILEERROR_SEEK, this, dfi->GetName());
    }
    EA_ASSERT(resOffset < 0 || resOffset == static_cast<off_t>(offset));
    
    LogSeekEvent(GetName(), dfi->GetDescriptor(), whence, static_cast<uint64_t>(dfi->GetPos()), offset);

    dfi->SetPos(static_cast<off_t>(offset));

    return offset;
}

uint64_t KettleFileDeviceDriver::GetSize(void *fh)
{
    const DeviceFileInfo *dfi = static_cast<DeviceFileInfo *>(fh);
    
    if (!dfi->IsValidDescriptor())
        return 0;

    SceKernelStat st;
    if (sceKernelFstat(dfi->GetDescriptor(), &st) == SCE_OK)
        return (uint64_t)st.st_size;

    return 0;
}

bool KettleFileDeviceDriver::Resize(void *fh, uint64_t newsize)
{
    DeviceFileInfo *dfi = static_cast<DeviceFileInfo *>(fh);
    if (!dfi->IsValidDescriptor())
        return false;

    int descr = dfi->GetDescriptor();
    
    off_t cur = dfi->GetPos();
    off_t size = static_cast<off_t>(newsize);

    if (sceKernelFtruncate(descr, size) != SCE_OK)
        return false;

    // ftruncate doesn't change file pos
    if (cur > size)
    {
        int res = sceKernelLseek(descr, size, SCE_KERNEL_SEEK_SET);
        EA_ASSERT(res == SCE_OK); EA_UNUSED(res);
        dfi->SetPos(size);
    }
    return true;
}

bool KettleFileDeviceDriver::Delete(const char *filePath)
{
    char drvPath[FILESYS_MAX_PATH];
    ReplaceSlashes(drvPath, filePath);

    bool ret = (sceKernelUnlink(drvPath) == SCE_OK);
    
    //printf("\n\nremove \"%s\" returned %d errno=%d\n", drvPath, ret, errno);
    return ret;
}

bool KettleFileDeviceDriver::Move(const char *currentName, const char *newName)
{
    char drvCurrName[FILESYS_MAX_PATH], drvNewName[FILESYS_MAX_PATH];

    return sceKernelRename(
        ReplaceSlashes(drvCurrName, currentName), 
        ReplaceSlashes(drvNewName, newName)) == SCE_OK;
}

static void SplitName(char * n, size_t & patternIndex)
{
    patternIndex = 0;
    char *q = n;
    char *p = n;
    while (*p)
    {
        if (*p == '/')
            q = p;
        ++p;
    }
    if (q == n) return;
    *q = 0;
    patternIndex = q-n+1;
}

#define TS_TO_U64_S(x) (uint64_t(x))

static void SetEntryData(DeviceDriver::FindData *findData, SceKernelStat &st, const char *entryName)
{
    if (findData) 
    {
        EA::StdC::Snprintf(findData->mName, FILESYS_MAX_PATH - 1, "%s", entryName);
        findData->mName[FILESYS_MAX_PATH - 1] = 0;

        findData->mCreationTime = TS_TO_U64_S(st.st_ctime);
        findData->mAccessTime = TS_TO_U64_S(st.st_atime);
        findData->mModificationTime =  TS_TO_U64_S(st.st_mtime);
        findData->mSize = (uint64_t)st.st_size;

        findData->mFlags = 0;

        if (!(st.st_mode & SCE_KERNEL_S_IWUSR))
            findData->mFlags |= DirectoryEntry::FLAG_READONLY;

        if (SCE_KERNEL_S_ISDIR(st.st_mode))
            findData->mFlags |= DirectoryEntry::FLAG_DIRECTORY;
    }
}

#undef TS_TO_U64_S

void * KettleFileDeviceDriver::FindBegin(const char *name, FindData *findData)
{
    EA_ASSERT(findData);

    DeviceDirectoryInfo *ddi = NULL;

    char drvFileName[FILESYS_MAX_PATH];
    ReplaceSlashes(drvFileName, name);

    size_t patternIndex;
    SplitName(drvFileName, patternIndex);

    int flags = SCE_KERNEL_O_DIRECTORY | SCE_KERNEL_O_RDONLY;
    SceKernelMode accessMode = SCE_KERNEL_S_IRWXU;
    int dirDescr = sceKernelOpen(drvFileName, flags, accessMode);

    if (dirDescr >= 0)
    {
        SceKernelStat st;
        if (sceKernelFstat(dirDescr, &st) != SCE_OK)
            return NULL;

        int bufferSize = st.st_blksize;

        ddi = new DeviceDirectoryInfo(dirDescr, drvFileName, bufferSize);
        ddi->SetPattern(drvFileName + patternIndex);

        // By convention, the FindBegin will return the first entry of the directory
        // In normal case, for an empty directory the first entry will be "." or ".."
        // On kettle, at least on the host drive, when reading from an empty directory
        // no entry is returned at all creating an ivalid directory entry in rwfilesystem
        // To work arround this we insert manually an "." entry in this case
        // According to Sony this should happen only on the host driver but it has been observed on the app0 driver too

        // Test for directories that report no entry at all
        if (!ddi->ReadMultipleEntries())
        {
            // If the directory return no entries, add the "current" directory entry manually
            // (if the pattern matches)
            if (NoCaseMatchesWildcardPattern(CURRENT_DIR_ENTRY, ddi->GetPattern()))
            {
                SetEntryData(findData, st, CURRENT_DIR_ENTRY);
                return ddi;
            }
        }
        // Find the first valid entry
        else if (FindNext(ddi, findData))
        {
            return ddi;
        }

        // No valid entry found
        sceKernelClose(dirDescr);
        delete ddi;
    }

    return NULL;
}

bool KettleFileDeviceDriver::FindNext(void *h, FindData *findData)
{
    EA_ASSERT(findData);

    DeviceDirectoryInfo *ddi = static_cast<DeviceDirectoryInfo *>(h);
    char const *pat = ddi->GetPattern();

    for (;;)
    {
        SceKernelDirent *dirEnt = ddi->GetNextEntry();

        if (dirEnt == NULL)
        {
            if (!ddi->ReadMultipleEntries())
            {
                // We reached the end
                return false;
            }

            dirEnt = ddi->GetNextEntry();
            
            EA_ASSERT(dirEnt); // We should have a new batch of entries
            if (dirEnt == NULL)
                return false;
        }
        
        if (NoCaseMatchesWildcardPattern(dirEnt->d_name, pat))
        {
            char buf[FILESYS_MAX_PATH];
            EA::StdC::Snprintf(buf, FILESYS_MAX_PATH - 1, "%s/%s", ddi->GetName(), dirEnt->d_name);
            buf[FILESYS_MAX_PATH - 1] = 0;
            
            SceKernelStat st;
            int ret = sceKernelStat(buf, &st);
            EA_ASSERT(ret == SCE_OK); EA_UNUSED(ret);
            SetEntryData(findData, st, dirEnt->d_name);

            return true;
        }
    }
}

bool KettleFileDeviceDriver::DirectoryCreate(const char *directory)
{
    char localDir[FILESYS_MAX_PATH];
    SceKernelMode mode = SCE_KERNEL_S_IRWXU;
    return sceKernelMkdir(ReplaceSlashes(localDir, directory), mode) == SCE_OK;
}

bool KettleFileDeviceDriver::DirectoryRemove(const char *directory)
{
    char localDir[FILESYS_MAX_PATH];
    return sceKernelRmdir(ReplaceSlashes(localDir, directory)) == SCE_OK;
}

bool KettleFileDeviceDriver::FindEnd(void *h)
{
    DeviceDirectoryInfo *ddi = static_cast<DeviceDirectoryInfo *>(h);
    if (ddi != NULL)
    {
        int ret = sceKernelClose(ddi->GetDescriptor());
        EA_ASSERT(ret == SCE_OK); EA_UNUSED(ret);

        delete ddi;
    }

    return true;
}

bool KettleFileDeviceDriver::GetNameFromHandle(void *fh, char *name, size_t nameBufferSize)
{
    DeviceFileInfo *h = static_cast<DeviceFileInfo *>(fh);

    EA_ASSERT(h != NULL);

    EA_STRLCPY_VERIFY(name, h->GetName(), nameBufferSize);

    return true;
}

KettleFileDeviceDriver gDeviceDriverImpl;

}}}

#undef INVALID_FILE_DESCR
#undef CURRENT_DIR_ENTRY
