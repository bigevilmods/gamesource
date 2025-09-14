// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::filesys::Manager platform specific componentry.
*/

#include "EAAssert/eaassert.h"
#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/driverimpl.h"

#if defined(EA_PLATFORM_LINUX) || defined(EA_PLATFORM_BSD)
#include <unistd.h>
#endif

namespace rw {
namespace core {
namespace filesys {


    void Manager::PlatformInit()
    {
        char *cwd = getcwd(mCurrDir,sizeof(mCurrDir));
        (void)cwd;
        EA_ASSERT(cwd == mCurrDir);
        gDeviceDriverImpl.AddRoot("/");
        sInstance->RegisterDevice(&gDeviceDriverImpl);
    }


    void Manager::PlatformTeardown()
    {
        gDeviceDriverImpl.Restore();
    }

}}}
