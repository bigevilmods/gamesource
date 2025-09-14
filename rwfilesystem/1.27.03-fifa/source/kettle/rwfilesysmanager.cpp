// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::filesys::Manager platform specific componentry.
*/

#include "EAAssert/eaassert.h"
#include "EAStdC/EAString.h"
#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/driverimpl.h"

namespace rw {
namespace core {
namespace filesys {


    void Manager::PlatformInit()
    {
        const char *PLATFORM_DEFAULT_CWD = "/app0";
        EA_STRLCPY_VERIFY(mCurrDir, PLATFORM_DEFAULT_CWD, EAArrayCount(mCurrDir));
        gDeviceDriverImpl.AddRoot(mCurrDir);
        sInstance->RegisterDevice(&gDeviceDriverImpl);
    }


    void Manager::PlatformTeardown()
    {
        gDeviceDriverImpl.Restore();
    }

}}}
