// (c) Electronic Arts. All Rights Reserved.

#include "EAAssert/eaassert.h"
#include "EAStdC/EAMemory.h"
#include "EAStdC/EASprintf.h"

#include "rw/core/filesys/incrementalinstaller.h"
#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/detail/cachedriver.h"

#ifdef EA_PLATFORM_PS3
    #include "rw/core/filesys/detail/ps3installdriver.h"
#endif

using namespace EA::StdC;

namespace rw {
namespace core {
namespace filesys {

IncrementalInstaller* IncrementalInstaller::sInstance = NULL;


IncrementalInstallerOptions::IncrementalInstallerOptions()
    // Cache
    : cacheSrcPath("."),
      cacheCopyPriority(-256),
      cacheThreadPriority( EA::Thread::kThreadPriorityMin ),
      cacheVersion(0),
      cacheClear(false),
      cacheFiles(NULL),
      cacheTitleID(NULL),
      cacheIdleTime(250),
      cacheUpdateCallback(NULL),
      cacheErrorCallback(NULL),
      cacheFileCallback(NULL),

    // Install
#if defined EA_PLATFORM_PS3
      installSrcPath(NULL),
      installDestPath(NULL),
      installCopyPriority(-256),
      installThreadPriority( EA::Thread::kThreadPriorityMin ),
      installVersion(0),
      contentInstallSrcPath(NULL),
      contentInstallFiles(NULL),
      preInstallFiles(NULL),
      postInstallFiles(NULL),
      postInstallIdleTime(250),
      preInstallHandleErrorCallback(NULL),
      preInstallCallback(NULL),
      preInstallBeginCallback(NULL),
      preInstallUpdateCallback(NULL),
      preInstallEndCallback(NULL),
      installUpdateCallback(NULL),
      installErrorCallback(NULL),
      installFileCallback(NULL),
      installWillInstallString(NULL),
      installMakeSpaceString(NULL),
      installMakeSpaceForTrophies(NULL),
      installCorruptGameDataString(NULL),
      mbFakeFreeSpace(false),
      mbIsBootedFromDvd(false),
      mbShowDialogMessages(true),
      miFakeFreeSpaceKB(0),
      miCareerDataSizeKB(0),
      miTropheySizeKB(0),
#endif

      doneCallback(NULL)
{
#if defined EA_PLATFORM_PS3
    MemFill32( &gameDataSystemFileParam, 0, sizeof( gameDataSystemFileParam ) );
#endif
}


void IncrementalInstaller::InstallDriverDone()
{
    EA_ASSERT(sInstance != NULL);

#if defined(EA_PLATFORM_PS3)
    if( sInstance->mState == INSTALL_DRIVER )
    {
        // restore the original done callback.
        sInstance->mOptions.doneCallback = sInstance->mUserDoneCallback;
        sInstance->mUserDoneCallback = NULL;
        sInstance->mState = CACHE_DRIVER;
        sInstance->UpdateState();
        return;
    }
#endif

    if( sInstance->mState == CACHE_DRIVER )
    {
        sInstance->mState = END;
        sInstance->UpdateState();
    }
}

void *IncrementalInstaller::operator new(size_t size)
{
    EA_ASSERT(sizeof(IncrementalInstaller) == size);
    return RWFILESYSTEM_ALLOCATE(size);
}


void IncrementalInstaller::operator delete(void *p)
{
    if( p ) Manager::GetInstance()->Free( p );
}


IncrementalInstaller *IncrementalInstaller::CreateInstance(const IncrementalInstallerOptions &options)
{
    EA_ASSERT_MSG(sInstance == NULL, "instance already exists.");
    sInstance = new IncrementalInstaller(options);
    return sInstance;
}


IncrementalInstaller *IncrementalInstaller::GetInstance()
{
    EA_ASSERT_MSG(sInstance != NULL, "instance does not exist, call IncrementalInstaller::CreateInstance() first." );
    return sInstance;
}


void IncrementalInstaller::DestroyInstance()
{
    EA_ASSERT_MSG(sInstance != NULL, "instance does not exist, call IncrementalInstaller::CreateInstance() first.");
    delete sInstance;
    sInstance = NULL;
}



IncrementalInstaller::IncrementalInstaller( const IncrementalInstallerOptions& options )
    : mOptions( options )
#ifdef EA_PLATFORM_PS3
    , mUserDoneCallback(NULL)
#endif
    , mCacheDriver(NULL)
    , mInstallDriver( NULL )
    , mInstallDriverIsStarted( false )
    , mState( BEGIN )
{
    #ifdef EA_PLATFORM_PS3
        mInstallPath[0] = 0;
    #endif

    CacheDriverOptions cdOptions;
    cdOptions.srcPath = mOptions.cacheSrcPath;
    cdOptions.copyPriority = mOptions.cacheCopyPriority;
    cdOptions.threadPriority = mOptions.cacheThreadPriority;
    cdOptions.clear = mOptions.cacheClear;
    cdOptions.version = mOptions.cacheVersion;
    cdOptions.titleID = mOptions.cacheTitleID;
    cdOptions.files = mOptions.cacheFiles;
    cdOptions.doneCallback = IncrementalInstaller::InstallDriverDone;
    cdOptions.updateCallback = mOptions.cacheUpdateCallback;
    cdOptions.errorCallback = mOptions.cacheErrorCallback;
    cdOptions.fileCallback = mOptions.cacheFileCallback;
    cdOptions.idleTime = mOptions.cacheIdleTime;
    mCacheDriver = new CacheDriver( cdOptions );
}


IncrementalInstaller::~IncrementalInstaller()
{
    if(mCacheDriver)
    {
        delete mCacheDriver;
        mCacheDriver = NULL;
    }

    if(mInstallDriver)
    {
        delete mInstallDriver;
        mInstallDriver = NULL;
    }
}


void IncrementalInstaller::Start()
{
    mInstallDriverIsStarted = true;

    if(mState == BEGIN)
    {
        UpdateState();
    }

    if( mInstallDriver )
    {
        mInstallDriver->Start();
    }
    else if( mCacheDriver && (mOptions.cacheFiles != NULL) )
    {
        mCacheDriver->Start();
    }
}


void IncrementalInstaller::Stop()
{
    if( mInstallDriver )
    {
        mInstallDriver->Stop();
    }
    else if( mCacheDriver )
    {
        mCacheDriver->Stop();
    }

    mInstallDriverIsStarted = false;
}


void IncrementalInstaller::Terminate(bool waitForEnd)
{
    if( mInstallDriver )
    {
        mInstallDriver->Terminate(waitForEnd);
    }
    else if( mCacheDriver )
    {
        mCacheDriver->Terminate(waitForEnd);
    }
}


bool IncrementalInstaller::IsTerminating() const
{
    if( mInstallDriver )
    {
        return mInstallDriver->IsTerminating();
    }
    else if( mCacheDriver )
    {
        return mCacheDriver->IsTerminating();
    }

    return false;
}


void IncrementalInstaller::OnDiskEject()
{
    if( mInstallDriver )
    {
        if( mInstallDriverIsStarted )
        {
            mInstallDriver->Stop();
            mInstallDriverIsStarted = true; // because Stop() sets it to false
        }
    }
    else if( mCacheDriver )
    {
        if( mInstallDriverIsStarted )
        {
            mCacheDriver->Stop();
            mInstallDriverIsStarted = true; // because Stop() sets it to false
        }
    }
}


void IncrementalInstaller::OnDiskRecover()
{
    if( mInstallDriver )
    {
        if( mInstallDriverIsStarted )
        {
            mInstallDriver->Start();
        }
    }
    else if( mCacheDriver )
    {
        if( mInstallDriverIsStarted )
        {
            mCacheDriver->Start();
        }
    }

}


void IncrementalInstaller::UpdateState()
{
    bool updateStateAgain;

    do
    {
        updateStateAgain = false;

        switch( mState )
        {
        case BEGIN:
            #if defined EA_PLATFORM_PS3
                mState = INSTALL_DRIVER;
            #elif defined EA_PLATFORM_XENON
                mState = CACHE_DRIVER;
            #else
                mState = END;
            #endif
                updateStateAgain = true;
            break;

#if defined(EA_PLATFORM_PS3)
        case INSTALL_DRIVER:
            if( mOptions.contentInstallFiles != NULL ||
                mOptions.preInstallFiles != NULL ||
                mOptions.postInstallFiles != NULL )
            {
                // Install some content/files to the PS3 hard disk.

                // a bit of hackery to allow us to use the caller's options
                // while overriding a single callback (the done callback,
                // which we want redirected to
                // IncrementalInstaller::InstallDriverDone.
                // InstallDriverDone will restore the original callback so
                // it can be called after both the install & the cache
                // content are copied.
                mUserDoneCallback = mOptions.doneCallback;
                mOptions.doneCallback = InstallDriverDone;

                mInstallDriver = new PS3InstallDriver( mOptions );
                ((PS3InstallDriver*)mInstallDriver)->Init();
                if(mInstallDriver != 0)
                {
                    const char *installPath = ((PS3InstallDriver*)mInstallDriver)->GetInstallPath();
                    Snprintf(mInstallPath, FILESYS_MAX_PATH, "%s", installPath);
                }
            }
            else
            {
                // Nothing to install, move on to the cache files.
                mState = CACHE_DRIVER;
                updateStateAgain = true;
            }
            break;
#endif

        case CACHE_DRIVER:

            if( mInstallDriver )
            {
#if defined(EA_PLATFORM_PS3)
                const char *installPath = ((PS3InstallDriver*)mInstallDriver)->GetInstallPath();
                Snprintf(mInstallPath, FILESYS_MAX_PATH, "%s", installPath);
#endif
                delete mInstallDriver;
                mInstallDriver = NULL;
            }

            if( mOptions.cacheFiles != NULL )
            {
               if( mInstallDriverIsStarted )
               {
                   mCacheDriver->Start();
               }
            }
            else
            {
                mState = END;
                updateStateAgain = true;
            }
            break;

        case END:

            if( mOptions.doneCallback )
            {
                mOptions.doneCallback();
            }

            break;
        }

    } while( updateStateAgain );
}


void IncrementalInstaller::MountCache()
{
    CacheDriver *cacheDriver = static_cast<CacheDriver*>(mCacheDriver);
    if(cacheDriver)
        cacheDriver->Mount();
}

const char *IncrementalInstaller::GetCachePath() const
{
    CacheDriver *cacheDriver = static_cast<CacheDriver*>(mCacheDriver);
    if(cacheDriver)
        return cacheDriver->GetPath();
    else
        return NULL;
}

const char *IncrementalInstaller::GetInstallPath() const
{
    #ifdef EA_PLATFORM_PS3
        return mInstallPath;
    #else
        return NULL;
    #endif
}

bool IncrementalInstaller::CacheIsMounted() const
{
    CacheDriver *cacheDriver = static_cast<CacheDriver *>(mCacheDriver);
    if (cacheDriver != NULL)
    {
        return cacheDriver->Mounted();
    }

    return false;
}

}}}
