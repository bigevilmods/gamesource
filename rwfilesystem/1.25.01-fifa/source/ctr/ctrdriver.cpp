// (c) Electronic Arts. All Rights Reserved.

#include "EAAssert/eaassert.h"

#include "private_rwhandle.h"
#include "private_rwdevice.h"
#include "rw/core/filesys/device.h"
#include "rw/core/filesys/driverimpl.h"
#include "rw/core/filesys/directory.h"
#include "rw/core/filesys/path.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <wchar.h>

#include <nn/os.h>
#include <nn/fs.h>

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

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


static void CopyFindInfo(rw::core::filesys::DeviceDriver::FindData *findData, nn::fs::DirectoryEntry &entry,
						 char* fileName)
{
    strcpy(findData->mName, fileName);
    findData->mCreationTime = 0;
    findData->mAccessTime = 0;
    findData->mModificationTime = 0;
    findData->mSize = 0;
    findData->mFlags = 0;

    if (entry.attributes.isDirectory)
    {
        findData->mFlags |= rw::core::filesys::DirectoryEntry::FLAG_DIRECTORY;
    }
}


}

namespace rw {
namespace core {
namespace filesys {

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

	DeviceFileHandle(const char *name)
        : mCurrPos(0)
    {
        if (name)
        {
            strncpy(mNativeFileName, name, FILESYS_MAX_PATH - 1);
            mNativeFileName[FILESYS_MAX_PATH - 1] = 0;
        }
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

	uint64_t mCurrPos;
    int         fileInfo;
    int         offset;
    int         size;

    char        mNativeFileName[FILESYS_MAX_PATH];

	nn::fs::FileStream* mpFilehandle;

};


class DevicePatternFileHandle 
{
	
public:
    DevicePatternFileHandle(nn::fs::Directory* handle)
        : dotProcessed(false),dotDotProcessed(false), mpDirectoryhandle(handle)
    {
    }

	~DevicePatternFileHandle()
	{
		if (mpDirectoryhandle)
			mpDirectoryhandle->Finalize();
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

    char mPattern[FILESYS_MAX_PATH];

    // Used to simulate the "." and ".." directories since sceIoDread does not return them.
    bool dotProcessed;
    bool dotDotProcessed;

	nn::fs::Directory* mpDirectoryhandle;
	
};


static nn::os::CriticalSection sCtrSdkIsAccessingFileSystemCriticalSection;
static void* sFileSysWorkingBuffer = NULL;
static volatile bool sCtrSdkIsAccessingFileSystem = false;

//////////////////////////////////////////////////////////////////////////////
// Driver functions                                                         //
//////////////////////////////////////////////////////////////////////////////

bool CtrFileDeviceDriver::Init()
{
	nn::fs::Initialize();
    sCtrSdkIsAccessingFileSystemCriticalSection.Initialize();

    // Todo : Parameterize this.
    const int maxDirs = 4;
	const int maxOpen = 10;

    const int workingMemorySize = nn::fs::GetRomRequiredMemorySize( maxOpen, maxDirs );
	sFileSysWorkingBuffer = RWFILESYSTEM_ALLOCATE(workingMemorySize);
    nn::fs::MountRom(maxOpen, maxDirs, sFileSysWorkingBuffer, workingMemorySize);

    return true;
}

void CtrFileDeviceDriver::Restore()
{
	DeviceDriver::Restore();
	Manager::Free(sFileSysWorkingBuffer);
	  
    sFileSysWorkingBuffer = NULL;

    sCtrSdkIsAccessingFileSystemCriticalSection.Finalize();
	nn::fs::Unmount("rom:");
}

void *CtrFileDeviceDriver::Open(const char *filename, unsigned int oflags, Handle **)
{
	//for now we only allow access to the ROMFS (read only)
    if((oflags & AsyncOp::FOPEN_READONLY) == AsyncOp::FOPEN_READONLY)
    {
        char TempName[256];//8.3 file name plus nul byte
        TempName[0] = '\0';

        strcpy(TempName, filename);
         

        // change \ to /
        for(int i = 0; TempName[i]; ++i)
        {
            if(TempName[i] == '\\')
            {
                TempName[i] = '/';
            }
        }
        DeviceFileHandle* handle = new DeviceFileHandle(TempName);
        CreateNintendoFileReader(handle);
		
        nn::Result result;
        sCtrSdkIsAccessingFileSystemCriticalSection.Enter();
        {
            sCtrSdkIsAccessingFileSystem = true;
			nn::fs::FileInputStream* fileHandle = (nn::fs::FileInputStream*)handle->mpFilehandle;
            result = handle->mpFilehandle->TryInitialize(TempName, nn::fs::OPEN_MODE_READ);
            sCtrSdkIsAccessingFileSystem = false;
        }
		
        sCtrSdkIsAccessingFileSystemCriticalSection.Leave();

        if (result.IsSuccess())
        {
            return handle;
        }
        else
        {
            DestroyNintendoFileReader(handle);
        }
    }
    return NULL;
}


void CtrFileDeviceDriver::CreateNintendoFileReader(DeviceFileHandle * handle)
{
    sCtrSdkIsAccessingFileSystemCriticalSection.Enter();
    {
        sCtrSdkIsAccessingFileSystem = true;
        void* fileInfoPtr = RWFILESYSTEM_ALLOCATE(sizeof(nn::fs::FileStream));
	        
        handle->mpFilehandle = new (fileInfoPtr) nn::fs::FileStream();
    
        sCtrSdkIsAccessingFileSystem = false;
    }
    sCtrSdkIsAccessingFileSystemCriticalSection.Leave();
}

void CtrFileDeviceDriver::DestroyNintendoFileReader(DeviceFileHandle * handle)
{
    sCtrSdkIsAccessingFileSystemCriticalSection.Enter();
    {
        sCtrSdkIsAccessingFileSystem = true;
        handle->mpFilehandle->~FileStream();
        Manager::Free(handle->mpFilehandle);
        handle->mpFilehandle = NULL;
        sCtrSdkIsAccessingFileSystem = false;
    }
    sCtrSdkIsAccessingFileSystemCriticalSection.Leave();
}

void CtrFileDeviceDriver::Close(void *h)
{
	DeviceFileHandle * handle = (DeviceFileHandle*)h;
	sCtrSdkIsAccessingFileSystemCriticalSection.Enter();
    {
        handle->mpFilehandle->Finalize();
    }
    sCtrSdkIsAccessingFileSystemCriticalSection.Leave();
    DestroyNintendoFileReader(handle);
}

unsigned int CtrFileDeviceDriver::Read(void *h, void *buf, unsigned int bufsize,
                                          DeviceDriver *, void *)
{
	DeviceFileHandle *handle = static_cast<DeviceFileHandle *>(h);
    int numberBytesRead = 0;

    // touch the first & last bytes of the buffer before calling
    // ReadFile().  This allows virtual memory to be committed via
    // a structured exception handler.  The performance impact
    // should be negligible, so we won't have a configuration
    // option to enable/disable it.
    *((volatile char *)buf+0) = *((volatile char *)buf+0);
    *((volatile char *)buf+bufsize-1) = *((volatile char *)buf+bufsize-1);

	nn::Result result;
    if (Manager::GetInstance()->GetErrorInjectionFunction()(this, handle->GetFilePath()))
    {
		result = handle->mpFilehandle->TryRead(&numberBytesRead, buf, bufsize);

        EA_ASSERT_FORMATTED(result.IsSuccess(), ("read failed with error %d", numberBytesRead));

		if (result.IsSuccess())
		{
        handle->MovePointer(numberBytesRead);
        return static_cast<unsigned int>(numberBytesRead);
		}
    }
     
	return 0;
    

}

uint64_t CtrFileDeviceDriver::Seek(void *h, uint64_t offset, int whence,
                                      DeviceDriver *, void *)
{
    DeviceFileHandle *handle = static_cast<DeviceFileHandle *>(h);

    // the driver only needs to handle seeks from the origin because the caller
    // (the Async module) keeps track of the file position
    EA_ASSERT(whence == static_cast<int>(FSEEK_BEGIN));

	nn::Result result = handle->mpFilehandle->TrySeek(offset, nn::fs::POSITION_BASE_BEGIN);

    EA_ASSERT_FORMATTED(result.IsSuccess(), ("seek failed with error"));

    if (result.IsSuccess())
    {
        handle->SetPos(static_cast<uint64_t>(offset));
	}

    return handle->GetPos();
}

uint64_t CtrFileDeviceDriver::GetSize(void *h)
{
	DeviceFileHandle *handle = static_cast<DeviceFileHandle *>(h);
	int64_t size = 0;
	nn::Result result = handle->mpFilehandle->TryGetSize(&size);
  
    EA_ASSERT(result.IsSuccess());

    return result.IsSuccess()? (uint64_t)size: 0;
	
}

bool CtrFileDeviceDriver::Resize(void * h, uint64_t newsize)
{
	DeviceFileHandle *handle = static_cast<DeviceFileHandle *>(h);
	nn::Result result = handle->mpFilehandle->TrySetSize(newsize);
	EA_ASSERT(result.IsSuccess());
	return result.IsSuccess();
}

bool CtrFileDeviceDriver::Delete(const char *filePath)
{
	nn::Result result = nn::fs::TryDeleteFile(filePath);	
	return result.IsSuccess();
}

bool CtrFileDeviceDriver::Move(const char *currentName, const char *newName)
{
	
	//attempt to rename it as a directory
	nn::Result result = nn::fs::TryRenameDirectory(currentName, newName);

	if (!result.IsSuccess())
	{
		result = nn::fs::TryRenameFile(currentName, newName);
	}

	return result.IsSuccess();
}

void* CtrFileDeviceDriver::CtrFileDeviceDriver::FindBegin(const char *filePattern, FindData *findData)
{
	
    DevicePatternFileHandle *handle = NULL;
    char directoryName[FILESYS_MAX_PATH] = {};
    strncpy(directoryName, filePattern, FILESYS_MAX_PATH - 1);
    directoryName[FILESYS_MAX_PATH - 1] = 0;

    size_t patternIndex;
    SplitName(directoryName, patternIndex);


    strcat(directoryName, "/");
    
	nn::fs::Directory* directory = new nn::fs::Directory;

	nn::Result result = directory->TryInitialize(directoryName);

    if (result.IsSuccess())
    {
        handle = new DevicePatternFileHandle(directory);
        handle->SetPattern(filePattern + patternIndex);

        if (FindNext(handle, findData))
            return handle;

        delete handle;
    }

    return NULL;

}

bool    CtrFileDeviceDriver::CtrFileDeviceDriver::FindNext(void *h, FindData *findData)
{
    EA_ASSERT(findData != NULL);
    EA_ASSERT(h != NULL);

    DevicePatternFileHandle *handle = static_cast<DevicePatternFileHandle *>(h);
    
    const char *pattern = handle->GetPattern();

    for (;;)
    {
 
        nn::fs::DirectoryEntry entryArray[1];
        s32 entryNum = 0;
		nn::Result result = handle->mpDirectoryhandle->TryRead(&entryNum, entryArray,
                                      sizeof(entryArray) / sizeof(*entryArray) );

        if (entryNum > 0)
        {
            // An entry was found
		    char nameBuffer[ FILESYS_MAX_PATH ];
			nn::fs::DirectoryEntry& entry = entryArray[0];        
			WCharBufferToCharBuffer( entry.entryName, nameBuffer );

            if (NoCaseMatchesWildcardPattern(nameBuffer, pattern))
            {
                CopyFindInfo(findData, entryArray[0], nameBuffer);
                return true;
            }
        }
        else if (entryNum == 0)
        {
            // Simulate the "." and ".." directories
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
     
            return false;
        }
    }	
}

bool CtrFileDeviceDriver::CtrFileDeviceDriver::FindEnd(void *h)
{
    EA_ASSERT(h != NULL);
    DevicePatternFileHandle *handle = static_cast<DevicePatternFileHandle *>(h);

    handle->mpDirectoryhandle->Finalize();

    delete handle;

    return true;
}

bool CtrFileDeviceDriver::DirectoryCreate(const char *directory)
{
	nn::Result result = nn::fs::TryCreateDirectory(directory);
	EA_ASSERT(result.IsSuccess());
	return result.IsSuccess();
}

bool CtrFileDeviceDriver::DirectoryRemove(const char *directory)
{
	nn::Result result = nn::fs::TryDeleteDirectory(directory);
	EA_ASSERT(result.IsSuccess());
	return result.IsSuccess();
}

bool CtrFileDeviceDriver::GetNameFromHandle(void *h, char *name, size_t nameBufferSize)
{
    DeviceFileHandle *handle = static_cast<DeviceFileHandle *>(h);
    EA_ASSERT(handle != NULL);

    const char* filePath = handle->GetFilePath();

    strncpy(name, filePath, nameBufferSize);
    name[nameBufferSize - 1] = 0;

    return true;
}

void CtrFileDeviceDriver::FixSlash(char *Dest,const char *Src)
{
    int len;
    // change \\ to /
    for( len = 0 ; Src[len]!='\0' ; len++)
    {
        if(Src[len] == '\\')
        {
            Dest[len] = '/';
        }
        else
        {
            Dest[len] = Src[len];
        }
    }
    Dest[len] = Src[len]; // copy nul
}

void CtrFileDeviceDriver::WCharBufferToCharBuffer(const wchar_t* src_wchar_buffer, char* dst_char_buffer )
{
    u32 wcs_length = wcslen( src_wchar_buffer );
    const wchar_t* tmp_wchar_ptr = src_wchar_buffer;
	wcsrtombs( dst_char_buffer, &(tmp_wchar_ptr),
	  		   wcs_length + 1,
			   NULL );
}


CtrFileDeviceDriver gDeviceDriverImpl;

}
}
}
