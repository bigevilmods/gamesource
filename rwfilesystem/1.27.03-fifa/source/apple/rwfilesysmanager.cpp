// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::filesys::Manager platform specific componentry.
*/

#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/driverimpl.h"
#include "EAStdC/EAString.h"
#include "EAStdC/EASprintf.h"
#include "EAAssert/EAAssert.h"

#if defined EA_PLATFORM_IPHONE
#include <apple/sysdirs.h>
#endif
#include <unistd.h>

namespace rw {
namespace core {
namespace filesys {


    void Manager::PlatformInit()
    {
#if defined EA_PLATFORM_IPHONE
        char appName[FILESYS_MAX_PATH];
        char homeDir[FILESYS_MAX_PATH];
        appleGetAppName(appName, sizeof(appName));
        appleGetHomeDirectory(homeDir, sizeof(homeDir));
        EA::StdC::Snprintf(mAppDir,sizeof(mAppDir),"%s/%s.app",homeDir,appName);
        EA_STRLCPY_VERIFY(mCurrDir, mAppDir, EAArrayCount(mCurrDir));

        chdir(mCurrDir);

        gDeviceDriverImpl.AddRoot("/");
        sInstance->RegisterDevice(&gDeviceDriverImpl);
#elif defined EA_PLATFORM_OSX
        char cwd[FILESYS_MAX_PATH];
        getcwd(cwd, FILESYS_MAX_PATH);
        EA_STRLCPY_VERIFY(mCurrDir, cwd, EAArrayCount(mCurrDir));

        gDeviceDriverImpl.AddRoot("/");
        sInstance->RegisterDevice(&gDeviceDriverImpl);
#else
        const char *PLATFORM_DEFAULT_CWD = "";
        EA_STRLCPY_VERIFY(mCurrDir, PLATFORM_DEFAULT_CWD, EAArrayCount(mCurrDir));
        gDeviceDriverImpl.AddRoot(mCurrDir);
        sInstance->RegisterDevice(&gDeviceDriverImpl);
#endif
    }


    void Manager::PlatformTeardown()
    {
        gDeviceDriverImpl.Restore();
    }

}}}
