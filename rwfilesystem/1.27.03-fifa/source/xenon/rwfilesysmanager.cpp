// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::filesys::Manager platform specific componentry.
*/

#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/driverimpl.h"
#include "EAStdC/EAString.h"
#include "EAAssert/EAAssert.h"
#include "platform_sdk.h"

namespace rw {
namespace core {
namespace filesys {


    void Manager::PlatformInit()
    {
        const char *PLATFORM_DEFAULT_CWD = "D:";

        EA_STRLCPY_VERIFY(mCurrDir, PLATFORM_DEFAULT_CWD, EAArrayCount(mCurrDir));
        gDeviceDriverImpl.AddRoot(mCurrDir);
        sInstance->RegisterDevice(&gDeviceDriverImpl);

        DWORD dwLicenseMask;
        if (XContentGetLicenseMask(&dwLicenseMask, NULL)== ERROR_SUCCESS)
        {
            // Booting from hard disk, do not create a second physical device for it.
            gDeviceDriverImpl.AddRoot("cache:");
        }
        else
        {
            // Booting from optical disc, add a second physical device for the hard disk
            // and enable location queries for load time optimization
            // Add the cache drive on the XENON.
            gCacheDriverImpl.AddRoot("cache:");
            sInstance->RegisterDevice(&gCacheDriverImpl);
            gDeviceDriverImpl.SetQueryLocationEnabled(true);
        }
    }


    void Manager::PlatformTeardown()
    {
        // Release the root lists for the other devices
        gDeviceDriverImpl.Restore();
        gCacheDriverImpl.Restore();
    }

}}}
