// (c) Electronic Arts. All Rights Reserved.

#include "EAAssert/eaassert.h"
#include "EAStdC/EAString.h"

#include "rw/core/filesys/detail/cachedriver.h"
#include "rw/core/filesys/directory.h"
#include "rw/core/filesys/loadsave.h"

#include "platform_sdk.h"

#ifdef EA_PLATFORM_PS3
#   include <sysutil/sysutil_syscache.h>
#elif defined EA_PLATFORM_XENON
#   pragma warning(push,0)
#   include <Xfilecache.h>
#   pragma warning(pop)
#endif

using namespace EA::StdC;

namespace rw {
namespace core {
namespace filesys {

CacheDriver* CacheDriver::sInstance = NULL;


bool CacheDriver::TestCallback(const char *srcFilename, const char *dstFilename )
{
    uint64_t srcSize = GetSize( srcFilename );
    uint64_t dstSize = GetSize( dstFilename );
    bool result = srcSize != dstSize || srcSize == 0 || dstSize == 0;
    VERBOSE( ("CacheDriver::TestCallback -> srcFilename = %s, dstFilename = %s, result = %d\n", srcFilename, dstFilename, result ? 1 : 0 ) );
    return result;
}


void CacheDriver::DoneCallback()
{
    VERBOSE( ("CacheDriver::DoneCallback\n" ) );

    sInstance->mDone = true;
    if( sInstance->mOptions.doneCallback )
    {
        sInstance->mOptions.doneCallback();
    }
}


void *CacheDriver::operator new(size_t size)
{
    EA_ASSERT(sizeof(CacheDriver) == size);
    return RWFILESYSTEM_ALLOCATE(size);
}


void CacheDriver::operator delete(void *p)
{
    if( p ) Manager::GetInstance()->Free( p );
}


CacheDriver::CacheDriver( const CacheDriverOptions &options )
    : detail::InstallDriver()
    , mOptions( options )
    , mFileCopier( NULL )
    , mMounted(false)
    , mValidated(false)
    , mDone(false)
{
    EA_ASSERT(mOptions.titleID != NULL);
    EA_ASSERT( sInstance == NULL );
    sInstance = this;

    mPath[0] = 0;
}


CacheDriver::~CacheDriver()
{
    delete mFileCopier;
    sInstance = NULL;
}


void CacheDriver::Start()
{
    Mount();
    ValidateCache();

    if( mFileCopier == NULL && !mDone )
    {
        if( IsCacheAvailable() )
        {
            FileCopierOptions options;
            options.copyPriority = mOptions.copyPriority;
            options.threadPriority = mOptions.threadPriority;
            options.errorCallback = mOptions.errorCallback;
            options.updateCallback = mOptions.updateCallback;
            options.fileCallback = mOptions.fileCallback;
            options.srcIdleTime = mOptions.idleTime;
            options.testCallback = CacheDriver::TestCallback;
            options.doneCallback = CacheDriver::DoneCallback;
            mFileCopier = new FileCopier( options );
            mFileCopier->Copy( mOptions.files, mOptions.srcPath, mPath );
        }
    }

    if( mFileCopier )
    {
        mFileCopier->Start();
    }
}


void CacheDriver::Stop()
{
    if( mFileCopier )
    {
        mFileCopier->Stop();
    }
}


void CacheDriver::Terminate(bool waitForEnd)
{
    if( mFileCopier )
    {
        mFileCopier->Terminate(waitForEnd);
    }
}


bool CacheDriver::IsTerminating() const
{
    if( mFileCopier )
    {
        return mFileCopier->IsTerminating();
    }

    return false;
}


void CacheDriver::Clear()
{
    VERBOSE(("CacheDriver::Clear\n"));

    #if defined EA_PLATFORM_PS3
        int ret = cellSysCacheClear();
        VERBOSE(("CacheDriver::Clear -> cellSysCacheClear() : 0x%x \n", ret ));
        EA_ASSERT(ret != (int)CELL_SYSCACHE_ERROR_ACCESS_ERROR);
        EA_ASSERT(ret != (int)CELL_SYSCACHE_ERROR_INTERNAL);
        EA_ASSERT(ret != (int)CELL_SYSCACHE_ERROR_NOTMOUNTED);
        (void)ret;
    #elif defined EA_PLATFORM_XENON
        DWORD result = XUnmountUtilityDrive();
        EA_ASSERT(result == ERROR_SUCCESS);
        XFlushUtilityDrive();

        if(result == ERROR_SUCCESS)
        {
            mMounted = false;

            result = XMountUtilityDrive(TRUE, 8 * 1024, 512 * 1024);
            if(result == ERROR_SUCCESS)
            {
                mMounted = true;
                XFlushUtilityDrive();
            }
        }
    #endif
}


bool CacheDriver::IsCacheAvailable() const
{
    return mPath[ 0 ] != NULL;
}


void CacheDriver::Mount()
{
    if(mMounted)
        return;

    VERBOSE(("CacheDriver::MountCache\n"));

    mPath[ 0 ] = NULL;

    #if defined EA_PLATFORM_PS3
        CellSysCacheParam param;
        memset(&param, 0x00 , sizeof(CellSysCacheParam)) ;
        Strncpy(param.cacheId, mOptions.titleID, sizeof(param.cacheId)) ;
        int ret = cellSysCacheMount(&param) ;

        VERBOSE(("CacheDriver::MountCache -> cellSysCacheMount() : 0x%x  sysCachePath:[%s]\n", ret, param.getCachePath));
        EA_ASSERT(ret != (int)CELL_SYSCACHE_ERROR_ACCESS_ERROR);
        EA_ASSERT(ret != (int)CELL_SYSCACHE_ERROR_INTERNAL);
        EA_ASSERT(ret != (int)CELL_SYSCACHE_ERROR_PARAM);

        if(mOptions.clear)
        {
            Clear();
        }

        mMounted = (ret == CELL_SYSCACHE_RET_OK_CLEARED) || (ret == CELL_SYSCACHE_RET_OK_RELAYED);
    #elif defined EA_PLATFORM_XENON
        if(XenonCacheMounted())
        {
            mMounted = true;
            XFlushUtilityDrive();
        }
        else
        {
            DWORD result = XMountUtilityDrive( mOptions.clear , 8 * 1024, 512 * 1024);
            VERBOSE( ("CacheDriver::MountCache -> XMountUtilityDriveEx\n" ) );
            VERBOSE( ("CacheDriver::MountCache -> XMountUtilityDrive : %d\n", result ) );

            if (result == ERROR_SUCCESS)
            {
                mMounted = true;
                XFlushUtilityDrive();
            }
        }
    #endif

    if(mMounted)
    {
        #if defined EA_PLATFORM_PS3
            Strncpy( mPath, param.getCachePath, FILESYS_MAX_PATH );
        #elif defined EA_PLATFORM_XENON
            Strncpy( mPath, "cache:", FILESYS_MAX_PATH );
        #endif
    }
}

void CacheDriver::ValidateCache()
{
    if(mValidated)
        return;

    VERBOSE(("CacheDriver::ValidateCache\n"));

    if(mMounted && !mValidated)
    {
        VERBOSE( ("CacheDriver::ValidateCache -> SUCCESS!\n" ) );

        bool validVersion = false;

        if(mOptions.clear)
        {
            VERBOSE( ("CacheDriver::ValidateCache -> Forced clear!\n" ) );
        }
        else
        {
            validVersion = InstallDriver::ValidateVersion( mPath, mOptions.version );
        }

        if (mOptions.clear || !validVersion)
            Clear();

        InstallDriver::CreateVersionFile(mPath, mOptions.version);
        rw::core::filesys::Manager::GetInstance()->AddSearchLocation( mPath, rw::core::filesys::Manager::SEARCHPATH_ADD_TO_HEAD );
    }
    else
    {
        VERBOSE( ("CacheDriver::ValidateCache -> FAILED!\n" ) );
    }

    mValidated = true;
}

bool CacheDriver::XenonCacheMounted() const
{
    DirectoryEntry de;
    de.OpenDirectory("cache:\\");
    return de.IsValid();
}

}}}


