// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::filesys::Manager platform specific componentry.
*/

#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/driverimpl.h"
#include "EAStdC/EAString.h"
#include "EAAssert/EAAssert.h"

namespace rw {
namespace core {
namespace filesys {


    void Manager::PlatformInit()
    {
        const char *PLATFORM_DEFAULT_CWD = "/app_home";
        EA_STRLCPY_VERIFY(mCurrDir, PLATFORM_DEFAULT_CWD, EAArrayCount(mCurrDir));
        gDeviceDriverImpl.AddRoot(mCurrDir);
        sInstance->RegisterDevice(&gDeviceDriverImpl);

        // Add the Hard drive on the PS3.
        gHddDriverImpl.AddRoot("/dev_hdd0");
        gHddDriverImpl.AddRoot("/dev_hdd1");
        sInstance->RegisterDevice(&gHddDriverImpl);

        // Add the app_home device
        gDeviceDriverImpl.AddRoot("/dev_bdvd");

        // legacy device names for HDD
        gHddDriverImpl.AddRoot("hdd0:");
        gHddDriverImpl.AddRoot("hdd1:");

        // legacy device names for main device
        gDeviceDriverImpl.AddRoot("host0:");
        gDeviceDriverImpl.AddRoot("host1:");
        gDeviceDriverImpl.AddRoot("bdvd:");
    }


    void Manager::PlatformTeardown()
    {
        gDeviceDriverImpl.Restore();
        gHddDriverImpl.Restore();
    }

}}}
