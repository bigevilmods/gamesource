// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::filesys::Manager platform specific componentry.
*/

#include "EAAssert/eaassert.h"
#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/driverimpl.h"

#if defined(EA_PLATFORM_PLAYBOOK) || defined(EA_PLATFORM_LINUX) || defined(EA_PLATFORM_BSD)
#include <unistd.h>
#endif

namespace rw {
namespace core {
namespace filesys {


    void Manager::PlatformInit()
    {
#if defined(EA_PLATFORM_AIRPLAY)
        strcpy(mCurrDir, ".");
        gDeviceDriverImpl.AddRoot(mCurrDir);
#else
        char *cwd = getcwd(mCurrDir,sizeof(mCurrDir));
        (void)cwd;
        EA_ASSERT(cwd == mCurrDir);
        gDeviceDriverImpl.AddRoot("/");
#endif
        sInstance->RegisterDevice(&gDeviceDriverImpl);
    }


    void Manager::PlatformTeardown()
    {
        gDeviceDriverImpl.Restore();
    }

}}}
