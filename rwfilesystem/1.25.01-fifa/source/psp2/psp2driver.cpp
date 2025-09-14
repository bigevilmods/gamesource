/*

Current issues/bugs:

 ** Relative Paths ** 

Rwfilesystem depends on getting the current working directory in order to open relative paths.
So far, I can’t find a way to get the current working directory. Therefore opening relative paths
will fail. There is one way to go around this (which I have used to get the tests working).
You can hard code the absolute path to the working directory into rwfilesysmanager.cpp: line 856.

Update: the issue is that the rwfilesystem, when you pass it a bare filename, will ALWAYS splice in a '/'
between the current working directory root name and the filename, because all of the code assumes that 
the root path specified will be the directory name (i.e. without the trailing '/').  This is done in 
multiple places throughout the rwfilesystem code.  However, the solution is to specify the defualt 
location to be host0:. , as this will map to the actual CWD

Update:  'app0:' is the working directory - by default it is location of the self executed.

	PSP2SDK documentation:
	https://vita.scedev.net/projects/sdk_docs/docs/vita-en,File_Path_Setting_Guide-vita,Usage/1
		== Working Directory ==
		"The working directory is the root directory accessed by the program with "app0:" during program execution. 
		The working directory can be specified with PSP2 Neighborhood, psp2run or a debugger, but if the working 
		directory is specified in the file path setting file, the path that is specified in the setting file is 
		mounted as the working directory. With either of these methods, if the working directory is not set, the 
		location of the executable file is mounted as the working directory."

 ** File Status Bug ** 
On Vita, if you write to a file, its status won’t get updated immediately. For example, if you
write 4 bytes to a 0-byte file, its “filesize” status won’t get updated until after a 
few milliseconds. This is probably a non-issue for now.
An issue has been started on devnet. Please delete this comment if the issue has been solved in 
future PSP2 SDKs

*/

#include "rw/core/filesys/driverimpl.h"
#include "rw/core/filesys/device.h"
#include "rw/core/filesys/directory.h"
#include "rw/core/filesys/path.h"
#include <EAAssert/eaassert.h>

#include <kernel.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

namespace
{

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
    if (q==n) return;
    *q = 0;
    patternIndex = q-n+1;
}

static uint64_t ConvertToUnixTime(SceDateTime &time)
{
    // Helper consts
    const uint64_t secondsInMinute = 60;
    const uint64_t secondsInHour = secondsInMinute * 60;
    const uint64_t secondsInDay = secondsInHour * 24;
    const uint64_t secondsInMonth = secondsInDay * 30; // Approximate
    const uint64_t secondsInYear = secondsInMonth * 12;

    uint64_t yearsSince1970 = time.year - 1970;

    uint64_t finalValue = yearsSince1970 * secondsInYear;
    finalValue += time.month * secondsInMonth;
    finalValue += time.day * secondsInDay;
    finalValue += time.hour * secondsInHour;
    finalValue += time.minute * secondsInMinute;
    finalValue += time.second;
    finalValue += time.microsecond / 1000;

    return finalValue;
}

static void CopyFindInfo(rw::core::filesys::DeviceDriver::FindData *findData, SceIoDirent &entry)
{
    snprintf(findData->mName, rw::core::filesys::FILESYS_MAX_PATH, "%s", entry.d_name);
    findData->mCreationTime = ConvertToUnixTime(entry.d_stat.st_ctime);
    findData->mAccessTime = ConvertToUnixTime(entry.d_stat.st_atime);
    findData->mModificationTime = ConvertToUnixTime(entry.d_stat.st_mtime);
    findData->mSize = entry.d_stat.st_size;
    findData->mFlags = 0;

    if (entry.d_stat.st_mode & SCE_STM_FDIR)
    {
        findData->mFlags |= rw::core::filesys::DirectoryEntry::FLAG_DIRECTORY;
    }
    if (entry.d_stat.st_mode & SCE_STM_RU)
    {
        findData->mFlags |= rw::core::filesys::DirectoryEntry::FLAG_READONLY;
    }
}

}

namespace rw {
namespace core {
namespace filesys {

// class DeviceFileHandle contains everything that a device driver
// needs to keep track of for the open file.  Each platform will have
// different requirements here.
class DeviceFileHandle
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

    DeviceFileHandle(SceUID handle, const char *name, int32_t systemFlags)
        : mHandle(handle), mCurrPos(0), mSystemFlags(systemFlags)
    {
        mNativeFileName[0] = '\0';

        if (name)
        {
            strncpy(mNativeFileName, name, FILESYS_MAX_PATH - 1);
            mNativeFileName[FILESYS_MAX_PATH - 1] = 0;
        }
    }

    SceUID GetHandle() const
    {
        return mHandle;
    }

    void SetHandle(SceUID newHandle)
    {
        mHandle = newHandle;
    }

    void MovePointer(int64_t amount)
    {
        EA_ASSERT(amount >= 0);
        mCurrPos += amount;
    }

    void SetPos(uint64_t pos)
    {
        mCurrPos = pos;
    }

    uint64_t GetPos() const
    {
        return mCurrPos;
    }

    const char* GetFilePath() const
    {
        return mNativeFileName;
    }

    int32_t GetSystemFlags() const 
    {
        return mSystemFlags;
    }
   
private:
    SceUID mHandle;
    uint64_t mCurrPos;
    int32_t mSystemFlags;
    char mNativeFileName[FILESYS_MAX_PATH];
};

// Used within FindBegin/Next/End. Same as DeviceFileHandle
// but with extra information relating to directory iteration
class DevicePatternFileHandle : public DeviceFileHandle
{
public:
    DevicePatternFileHandle(SceUID handle, const char *name, int32_t systemFlags)
        : DeviceFileHandle(handle, name, systemFlags), dotProcessed(false),
        dotDotProcessed(false)
    {
    }

    const char* GetPattern() const 
    {
        return mPattern;
    }

    void SetPattern(const char *pattern)
    {
        strcpy(mPattern, pattern);
    }

    bool& DotProcessed() 
    {
        return dotProcessed;
    }

    bool& DotDotProcessed()
    {
        return dotDotProcessed;
    }

private:
    char mPattern[FILESYS_MAX_PATH];

    // Used to simulate the "." and ".." directories since sceIoDread does not return them.
    bool dotProcessed;
    bool dotDotProcessed;
};

PSP2FileDeviceDriver::PSP2FileDeviceDriver()
{
}

PSP2FileDeviceDriver::~PSP2FileDeviceDriver()
{
}

void* PSP2FileDeviceDriver::Open(const char *name, unsigned int oflags, Handle **)
{
    int flags = 0;

    if ((oflags & AsyncOp::FOPEN_READWRITE) == AsyncOp::FOPEN_READWRITE)
        flags |= SCE_O_RDWR;
    else
        flags |= SCE_O_RDONLY;

    
    switch (oflags & (AsyncOp::FOPEN_CREATE | AsyncOp::FOPEN_OVERWRITE))
    {
    case 0:
        /* createmode is already correct */ break;

    case AsyncOp::FOPEN_CREATE:
        {
            // Make sure that the file does not exist
            SceIoStat tempStat;
            if (sceIoGetstat(name, &tempStat) == 0)
            {
                Manager::GetInstance()->GetErrorHandler()(
                    static_cast<int>(SCE_ERROR_ERRNO_EEXIST), Manager::FILEERROR_OPEN, this, name);
                return NULL;
            }
        }
        flags |= SCE_O_CREAT;
        break;

    case AsyncOp::FOPEN_OVERWRITE:
        flags |= SCE_O_TRUNC;
        break;

    case AsyncOp::FOPEN_CREATE | AsyncOp::FOPEN_OVERWRITE:
        flags |= SCE_O_CREAT | SCE_O_TRUNC;
        break;
    }

    // Add the CWD to file names starting with ./ since rwfilesystem fails to do so.
    char localName[FILESYS_MAX_PATH] = {};
    const char *fullName = name;
    size_t nameLength = strlen(fullName);

    if (nameLength >= 2 && fullName[0] == '.' && fullName[1] == '/') 
    {
        fullName += 2;
    }

    if (nameLength >= 2 && fullName[0] != '/' && !strchr(fullName,':'))
    {
        strncpy(localName, Manager::GetInstance()->GetCWD(), sizeof(localName));
        strcat(localName, "/");
        strcat(localName, fullName);
        fullName = localName;
    }

    SceUID result = sceIoOpen(fullName, flags, SCE_STM_RWU);

    if (result >= 0)
    {
        return new DeviceFileHandle(result, name, flags);
    }

    Manager::GetInstance()->GetErrorHandler()(
        static_cast<int>(result), Manager::FILEERROR_OPEN, this, fullName);

    return NULL;
}

void PSP2FileDeviceDriver::Close(void *h)
{
    DeviceFileHandle *handle = static_cast<DeviceFileHandle*>(h);

    int32_t result = sceIoClose(handle->GetHandle());
    EA_ASSERT(result == SCE_OK);

    delete handle;
}

unsigned int PSP2FileDeviceDriver::Read(void *h, void *buf, unsigned int bufsize, DeviceDriver *, void *)
{
    DeviceFileHandle *handle = static_cast<DeviceFileHandle *>(h);
    ssize_t numberBytesRead = 0;

    // touch the first & last bytes of the buffer before calling
    // ReadFile().  This allows virtual memory to be committed via
    // a structured exception handler.  The performance impact
    // should be negligible, so we won't have a configuration
    // option to enable/disable it.
    *((volatile char *)buf+0) = *((volatile char *)buf+0);
    *((volatile char *)buf+bufsize-1) = *((volatile char *)buf+bufsize-1);

    if (Manager::GetInstance()->GetErrorInjectionFunction()(this, handle->GetFilePath()))
    {
        numberBytesRead = sceIoRead(handle->GetHandle(), buf, static_cast<size_t>(bufsize));

        EA_ASSERT_FORMATTED(numberBytesRead >= 0,
            ("read failed with error %d", numberBytesRead));
    }

    if (numberBytesRead >= 0)
    {
        handle->MovePointer(numberBytesRead);
        return static_cast<unsigned int>(numberBytesRead);
    }
    else
    {
        return 0;
    }
}

unsigned int PSP2FileDeviceDriver::Write(void *h, const void *buf, unsigned int bufsize, DeviceDriver *, void *)
{
    DeviceFileHandle *handle = static_cast<DeviceFileHandle *>(h);
    
    ssize_t numberBytesWritten = sceIoWrite(handle->GetHandle(), buf, static_cast<size_t>(bufsize));

    if (numberBytesWritten < 0)
    {
        // If the write fails, let the user know.  This is not necessarily
        // fatal because either the file was opened with the wrong permissions
        // or the disk is full.  We could throw an assert if the error is
        // anything other than disk full, but I think it's better to be
        // consistent and let the user halt the system if the error is not
        // recoverable.
        Manager::GetInstance()->GetErrorHandler()(
            static_cast<int>(numberBytesWritten), Manager::FILEERROR_WRITE, this, NULL);
        return 0;
    }
    
    handle->MovePointer(numberBytesWritten);
    return static_cast<unsigned int>(numberBytesWritten);
}

uint64_t PSP2FileDeviceDriver::Seek(void *h, uint64_t offset, int whence, DeviceDriver *, void *)
{
    DeviceFileHandle *handle = static_cast<DeviceFileHandle *>(h);

    // the driver only needs to handle seeks from the origin because the caller
    // (the Async module) keeps track of the file position
    EA_ASSERT(whence == static_cast<int>(FSEEK_BEGIN));

    if(offset == handle->GetPos())
	{
#if defined(EA_ASSERT_ENABLED)
		uint64_t ret = sceIoLseek(handle->GetHandle(), 0, SCE_SEEK_CUR);
		EA_ASSERT_FORMATTED(ret == offset, ("file %s, offset %lld, ret %lld, handle %u", handle->GetFilePath(), offset, ret, (unsigned int)(handle->GetHandle())));
#endif
		return offset;
	}

    int64_t newOffset = sceIoLseek(handle->GetHandle(), static_cast<int64_t>(offset), SCE_SEEK_SET);

    EA_ASSERT_FORMATTED(newOffset >= 0,
        ("seek failed with error %ld", newOffset));

#ifdef EA_FILESYSTEM_PROFILING
    {
        Manager *pMgr = Manager::GetInstance();
        FileSysProfiler * pProfiler = pMgr->GetEventProfiler();
        if (pProfiler != NULL)
        {
            ProfilerEventSeekStruct seek_info(
                handle->GetHandle(), whence, (uint64_t)handle->GetPos(),
                offset);

            pProfiler->Callback(ProfilerEventSeek,
                GetName(),
                &seek_info, pMgr->GetEventProfilerContext());
        }
    }
#endif

    if (newOffset >= 0)
    {
        handle->SetPos(static_cast<uint64_t>(newOffset));
        return static_cast<uint64_t>(newOffset);
    }
    else
    {
        // The seek did not occur. Return the old position
        return handle->GetPos();
    }
}

uint64_t PSP2FileDeviceDriver::GetSize(void *h)
{
    DeviceFileHandle *handle = static_cast<DeviceFileHandle *>(h);
    SceIoStat stat;

    int32_t result = sceIoGetstat(handle->GetFilePath(), &stat);
    EA_ASSERT(result == SCE_OK);

    return result == SCE_OK? static_cast<uint64_t>(stat.st_size) : 0;
}

bool PSP2FileDeviceDriver::Resize(void *h, uint64_t newsize)
{
    DeviceFileHandle *handle = static_cast<DeviceFileHandle *>(h);

    // The file must be closed first, however, we do not want to delete
    // the original handle. Therefore we call sceIoClose manually.
    int32_t result = sceIoClose(handle->GetHandle());
    (void)result;
    EA_ASSERT(result == SCE_OK);

    SceIoStat newStat;
    memset(static_cast<void*>(&newStat), 0, sizeof(SceIoStat));

    newStat.st_size = static_cast<int64_t>(newsize);

    result = sceIoChstat(handle->GetFilePath(), &newStat, SCE_CST_SIZE);
    EA_ASSERT(result == SCE_OK);

    // The file must be reopened whether or not the resize succeeded
    int newFlags = handle->GetSystemFlags() & SCE_O_RDWR? SCE_O_RDWR : SCE_O_RDONLY;
    SceUID newHandle = sceIoOpen(handle->GetFilePath(), newFlags, SCE_STM_RWU);
    EA_ASSERT(newHandle >= 0);

    handle->SetHandle(newHandle);

    return result == SCE_OK;


    // IMPORTANT TODO: Check the new file position after the resizing has occured. Perhaps use seek with 0 offset
}

bool PSP2FileDeviceDriver::Delete(const char *filePath)
{
    int32_t result = sceIoRemove(filePath);
    return result == SCE_OK;
}

bool PSP2FileDeviceDriver::Move(const char *currentName, const char *newName)
{
    int32_t result = sceIoRename(currentName, newName);
    return result == SCE_OK;
}

void* PSP2FileDeviceDriver::FindBegin(const char *filePattern, FindData *findData)
{
    DevicePatternFileHandle *handle = NULL;
    char directoryName[FILESYS_MAX_PATH] = {};
    strncpy(directoryName, filePattern, FILESYS_MAX_PATH - 1);
    directoryName[FILESYS_MAX_PATH - 1] = 0;

    size_t patternIndex;
    SplitName(directoryName, patternIndex);

    SceUID result = sceIoDopen(directoryName);

    if (result >= 0)
    {
        handle = new DevicePatternFileHandle(result, directoryName, 0);
        handle->SetPattern(directoryName + patternIndex);

        if (FindNext(handle, findData))
            return handle;

        Close(handle);
    }

    return NULL;
}

bool PSP2FileDeviceDriver::FindNext(void *h, FindData *findData)
{
    EA_ASSERT(findData != NULL);
    EA_ASSERT(h != NULL);

    DevicePatternFileHandle *handle = static_cast<DevicePatternFileHandle *>(h);
    
    const char *pattern = handle->GetPattern();

    for (;;)
    {
        SceIoDirent entry;
        memset(static_cast<void*>(&entry), 0, sizeof(SceIoDirent));

        int32_t result = sceIoDread(handle->GetHandle(), &entry);

        if (result > 0)
        {
            // An entry was found
            if (NoCaseMatchesWildcardPattern(entry.d_name, pattern))
            {
                CopyFindInfo(findData, entry);
                return true;
            }
        }
        else if (result == 0)
        {
            // Simulate the "." and ".." directories
            // WARNING TODO: Only the name is copied. Perhaps we can call sceIoGetstat on the current
            // directory for "." and the parent directory for ".."
            if ((!handle->DotProcessed()) && NoCaseMatchesWildcardPattern(".", pattern))
            {
                findData->mFlags = DirectoryEntry::FLAG_DIRECTORY;
                strcpy(findData->mName, ".");
                handle->DotProcessed() = true;
                return true;
            }
            else if ((!handle->DotDotProcessed()) && NoCaseMatchesWildcardPattern("..", pattern))
            {
                findData->mFlags = DirectoryEntry::FLAG_DIRECTORY;
                strcpy(findData->mName, "..");
                handle->DotDotProcessed() = true;
                return true;
            }

            // No more entries are available
            return false;
        }
        else
        {
            EA_FAIL_MSG("FindNext: sceIoDread failure.");
            return false;
        }
    }
}

bool PSP2FileDeviceDriver::FindEnd(void *h)
{
    EA_ASSERT(h != NULL);
    DevicePatternFileHandle *handle = static_cast<DevicePatternFileHandle *>(h);

    int32_t result = sceIoDclose(handle->GetHandle());
    EA_ASSERT(result == SCE_OK);

    delete handle;

    return result == SCE_OK;
}

bool PSP2FileDeviceDriver::DirectoryCreate(const char *directory)
{
    int32_t result = sceIoMkdir(directory, SCE_STM_RWU);
    return result == SCE_OK;
}

bool PSP2FileDeviceDriver::DirectoryRemove(const char *directory)
{
    int32_t result = sceIoRmdir(directory);
    return result == SCE_OK;
}

bool PSP2FileDeviceDriver::GetNameFromHandle(void *h, char *name, size_t nameBufferSize)
{
    DeviceFileHandle *handle = static_cast<DeviceFileHandle *>(h);
    EA_ASSERT(handle != NULL);

    const char* filePath = handle->GetFilePath();

    strncpy(name, filePath, nameBufferSize);
    name[nameBufferSize - 1] = 0;

    return true;
}

PSP2FileDeviceDriver gDeviceDriverImpl;

}}}
