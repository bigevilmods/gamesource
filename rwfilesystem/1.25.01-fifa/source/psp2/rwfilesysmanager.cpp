// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::filesys::Manager platform specific componentry.
*/

#include "EAAssert/eaassert.h"
#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/driverimpl.h"

namespace rw {
namespace core {
namespace filesys {

    void Manager::PlatformInit()
    {
        // add the app_home device
		strcpy(mCurrDir, "app0:.");
        gDeviceDriverImpl.AddRoot("app0:");
		
		// add the main device
		gDeviceDriverImpl.AddRoot("host0:");
        sInstance->RegisterDevice(&gDeviceDriverImpl);
    }

    void Manager::PlatformTeardown()
    {
        gDeviceDriverImpl.Restore();
    }
}}}