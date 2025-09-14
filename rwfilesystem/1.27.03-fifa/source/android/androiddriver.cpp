// (c) Electronic Arts. All Rights Reserved.

#include <jni.h>
#include <EAStdC/EAString.h>

#include "unix/unixdriver.cpp"

namespace rw {
namespace core {
namespace filesys {

#ifndef ASSETMANAGER_INIT_FAILURE_MESSAGE
    #define ASSETMANAGER_INIT_FAILURE_MESSAGE "Note: rwfilesystem.Startup(activityInstance) must be called from Java."
#endif
#include "assetmanagerjni.inc"

namespace
{

class AssetHandle
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

    AssetHandle(const char *NativeFileName, jobject inputStream, uint64_t fileSize)
        : mInputStream(inputStream)
        , mFileSize(fileSize)
        , mCurrPos(0)
    {
        EA_STRLCPY_VERIFY(mNativeFileName, NativeFileName, FILESYS_MAX_PATH);
    }

    jobject GetInputStream() const
    {
        return mInputStream;
    }

    void SetInputStream(jobject inputStream)
    {
        mInputStream = inputStream;
    }

    uint64_t GetFileSize() const
    {
        return mFileSize;
    }

    const char *GetFileName() const
    {
        return mNativeFileName;
    }

    void SetPos(off_t pos)
    {
        mCurrPos = pos;
    }

    off_t GetPos() const
    {
        return mCurrPos;
    }

    void AdvancePos(unsigned int amount)
    {
        mCurrPos += amount;
    }

private:
    char mNativeFileName[FILESYS_MAX_PATH];
    jobject  mInputStream;
    uint64_t mFileSize;
    off_t mCurrPos;
};

struct AndroidEntryFindData
{
    static void *operator new(size_t size)
    {
        return RWFILESYSTEM_ALLOCATE(size);
    }

    static void operator delete(void *p)
    {
        return Manager::Free(p);
    }

    jobject list;
    int     curListIndex;
    char    pattern[FILESYS_MAX_PATH];      // Note: rwfilesystem doesn't appear to have a maximum pattern length, so max path is used instead.
};

} // anonymous namespace


AppBundleDeviceDriver::AppBundleDeviceDriver()
{
}

static const char APPBUNDLE_PATH_PREFIX[] = "appbundle:/";
static const size_t APPBUNDLE_PATH_PREFIX_LENGTH = sizeof(APPBUNDLE_PATH_PREFIX) - 1;

void*  AppBundleDeviceDriver::Open(const char *name, unsigned int rwflags, Handle **)
{
    char nativeFileName[FILESYS_MAX_PATH];
    ReplaceSlashes(nativeFileName, name);
    
    if (!BQ(rwflags, AsyncOp::FOPEN_READONLY))
    {
        return NULL;
    }

    const char* fileName = name + APPBUNDLE_PATH_PREFIX_LENGTH;

    AutoJNIEnv autoEnv;
    uint64_t fileSize;
    jobject inputStream = AssetManagerJNI::Open(autoEnv, fileName, &fileSize);
    if(inputStream)
    {
        return new AssetHandle(fileName, inputStream, fileSize);
    }
    return NULL;
}

void AppBundleDeviceDriver::Close(void *fh)
{
    const AssetHandle *assetHandle = static_cast<AssetHandle *>(fh);
    AutoJNIEnv autoEnv;
    AssetManagerJNI::Close(autoEnv, assetHandle->GetInputStream());
    delete assetHandle;
}

unsigned int AppBundleDeviceDriver::Read(void *fh, void *buf, unsigned int bufSize, DeviceDriver *, void *)
{
    AssetHandle *assetHandle = static_cast<AssetHandle *>(fh);
    char *ptr = reinterpret_cast<char*>(buf);
    unsigned int totalAmountRead = 0;
    int amountRead;
    AutoJNIEnv autoEnv;
    do
    {
        amountRead = AssetManagerJNI::Read(autoEnv, assetHandle->GetInputStream(), ptr, bufSize-totalAmountRead);
        if(amountRead == -1)
        {
            // End of file has been reached
            break;
        }
        ptr += amountRead;
        totalAmountRead += static_cast<size_t>(amountRead);
    } while (amountRead != 0 && totalAmountRead < bufSize);

    assetHandle->AdvancePos(totalAmountRead);

    return totalAmountRead;
}

unsigned int AppBundleDeviceDriver::Write(void *fh, const void *buf, unsigned int bufsize, DeviceDriver *, void *)
{
    EA_FAIL_MSG("Write() not implemented for app bundle files.");
    return 0;
}

uint64_t AppBundleDeviceDriver::Seek(void *fh, uint64_t offset, int whence, DeviceDriver *, void *)
{
    AssetHandle *assetHandle = static_cast<AssetHandle *>(fh);

    // the driver only needs to handle seeks from the origin becuase the caller
    // (the Async module) keeps track of the file position
    EA_ASSERT(whence == FSEEK_BEGIN);

    if(offset == static_cast<uint64_t>(assetHandle->GetPos()))
    {
        // The seek position matches the current position, so no operation is necessary.
        return offset;
    }
    else if(offset == 0)
    {
        // Close and reopen the file to seek to the beginning.
        {
            AutoJNIEnv autoEnv;
            AssetManagerJNI::Close(autoEnv, assetHandle->GetInputStream());
            uint64_t fileSize;
            jobject inputStream = AssetManagerJNI::Open(autoEnv, assetHandle->GetFileName(), &fileSize);
            assetHandle->SetInputStream(inputStream);
            assetHandle->SetPos(0);
        }
        return 0;
    }
    else
    {
        EA_FAIL_MSG("Seek() for android asset bundles currently only supports seeking to the beginning of the file or the current stream location.");
        return 0xffffffffffffffffull;
    }
}

uint64_t AppBundleDeviceDriver::GetSize(void *fh)
{
    const AssetHandle *assetHandle = static_cast<AssetHandle *>(fh);
    uint64_t result = assetHandle->GetFileSize();
    EA_ASSERT_MSG(result != 0xffffffffffffffffull, "The file size could not be determined.  Make sure the file does not have any compression applied when it is stored in the .apk file.");
    return result;
}

bool AppBundleDeviceDriver::Resize(void *, uint64_t)
{
    EA_FAIL_MSG("Resize() not implemented for app bundle files.");
    return false;
}

bool AppBundleDeviceDriver::Delete(const char *filePath)
{
    EA_FAIL_MSG("Delete() not implemented for app bundle files.");
    return false;
}

bool AppBundleDeviceDriver::Move(const char *currentName, const char *newName)
{
    EA_FAIL_MSG("Move() not implemented for app bundle files.");
    return false;
}

void * AppBundleDeviceDriver::FindBegin(const char *name, FindData *findData)
{
    AutoJNIEnv autoEnv;

    // Find the pattern which should be located immediately after the last '/'.
    const char* pattern = EA::StdC::Strrchr(name, '/') + 1;
    EA_ASSERT(pattern);
    const char* pathNoPrefix = name + APPBUNDLE_PATH_PREFIX_LENGTH; // The path without the "appbundle:/" prefix.

    size_t pathLen = pattern - pathNoPrefix;
    if (pathLen >= FILESYS_MAX_PATH)
    {
        return NULL;
    }

    char16_t path[FILESYS_MAX_PATH];
    // We don't use EA_STRLCPY_VERIFY here because we are intentionally truncating the source string.
    EA::StdC::Strlcpy(path, pathNoPrefix, pathLen + 1);

    AndroidEntryFindData *androidFindData = new AndroidEntryFindData;
    androidFindData->list = AssetManagerJNI::List(autoEnv, path, pathLen);
    androidFindData->curListIndex = 0;
    EA_STRLCPY_VERIFY(androidFindData->pattern, pattern, EAArrayCount(androidFindData->pattern));

    if(FindNext(androidFindData, findData))
    {
        return androidFindData;
    }
    else
    {
        // Otherwise clean up the list that was created.
        AssetManagerJNI::FreeList(autoEnv, androidFindData->list);
        delete androidFindData;
        return NULL;
    }
}

bool AppBundleDeviceDriver::FindNext(void *h, FindData *findData)
{
    AndroidEntryFindData *androidFindData = reinterpret_cast<AndroidEntryFindData *>(h);

    char16_t entryName[FILESYS_MAX_PATH];
    AutoJNIEnv autoEnv;
    while(AssetManagerJNI::GetListElement(autoEnv, androidFindData->list, androidFindData->curListIndex, entryName, EAArrayCount(entryName)))
    {
        androidFindData->curListIndex++;

        EA_STRLCPY_VERIFY(findData->mName, entryName, EAArrayCount(findData->mName));

        if (NoCaseMatchesWildcardPattern(findData->mName, androidFindData->pattern))
        {
            // All appbundle files/directories are read only.
            findData->mFlags = filesys::DirectoryEntry::FLAG_READONLY;

            findData->mSize = -1;
            // HACK
            // There doesn't seem to currently be a nice efficient way to determine if an entry is a directory
            // or not. So we make the assumption that any entry name without an extension (e.g. .jpg) is a directory.
            if( EA::StdC::Strrchr(findData->mName, '.') == NULL )
            {

                findData->mFlags |= filesys::DirectoryEntry::FLAG_DIRECTORY;
            }
            else
            {
                jobject inputStream = AssetManagerJNI::Open(autoEnv, findData->mName, &findData->mSize);
                if(inputStream)
                {
                    AssetManagerJNI::Close(autoEnv, inputStream);
                }
            }

            // None of these times are available on Android, so initialize them to 0 so they are at least in a well defined state.
            findData->mCreationTime = 0;
            findData->mAccessTime = 0;
            findData->mModificationTime = 0;
    
            return true;
        }
    }
    return false;
}

bool AppBundleDeviceDriver::FindEnd(void *h)
{
    AndroidEntryFindData *androidFindData = reinterpret_cast<AndroidEntryFindData *>(h);
    AutoJNIEnv autoEnv;
    AssetManagerJNI::FreeList(autoEnv, androidFindData->list);
    delete androidFindData;
    return true;
}

bool AppBundleDeviceDriver::DirectoryCreate(const char *directory)
{
    EA_FAIL_MSG("DirectoryCreate() not implemented for app bundle files.");
    return false;
}

bool AppBundleDeviceDriver::DirectoryRemove(const char *directory)
{
    EA_FAIL_MSG("DirectoryRemove() not implemented for app bundle files.");
    return false;
}

bool AppBundleDeviceDriver::GetNameFromHandle(void *fh, char *name, size_t nameBufferSize)
{
    const AssetHandle *assetHandle = static_cast<const AssetHandle *>(fh);
    EA_STRLCPY_VERIFY(name, APPBUNDLE_PATH_PREFIX, nameBufferSize);
    if(nameBufferSize > APPBUNDLE_PATH_PREFIX_LENGTH)
    {
        const char *fileName = assetHandle->GetFileName();
        EA_STRLCPY_VERIFY(name + APPBUNDLE_PATH_PREFIX_LENGTH, fileName, nameBufferSize - APPBUNDLE_PATH_PREFIX_LENGTH);
    }
    return true;
}

AppBundleDeviceDriver gAppBundleDeviceDriverImpl;

}}}

// JNI code

extern "C" __attribute__ ((visibility("default"))) void JNICALL Java_com_ea_rwfilesystem_rwfilesystem_StartupNativeImpl 
  (JNIEnv *env, jclass, jobject assetManager, jstring dataDir, jstring filesDir, jstring externalDir)
{
    rw::core::filesys::AssetManagerJNI::Startup(env, assetManager, dataDir, filesDir, externalDir);
}

extern "C" __attribute__ ((visibility("default"))) void JNICALL Java_com_ea_rwfilesystem_rwfilesystem_Shutdown
  (JNIEnv *env, jclass)
{
    rw::core::filesys::AssetManagerJNI::Shutdown(env);
}
