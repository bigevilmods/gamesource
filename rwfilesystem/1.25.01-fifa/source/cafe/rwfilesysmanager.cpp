// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::filesys::Manager platform specific componentry.
*/

#include <EAAssert/eaassert.h>
#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/driverimpl.h"

namespace rw {
namespace core {
namespace filesys {


    void Manager::PlatformInit()
    {
        FSInit();

        FSClient *clientHandle = static_cast<FSClient *>(RWFILESYSTEM_ALLOCATE(sizeof(FSClient)));
        FSStatus status = FSAddClient(clientHandle, FS_RET_ALL_ERROR);
        EA_ASSERT(status == FS_STATUS_OK);

        gDeviceDriverImpl.SetClientHandle(clientHandle);

        const char *PLATFORM_DEFAULT_CWD = FS_CONTENT_DIR;

        strcpy(mCurrDir, PLATFORM_DEFAULT_CWD);
        gDeviceDriverImpl.AddRoot(mCurrDir);

        // add a root for save files - is this required?
        // In SDK 1.5, /vol/content is writable
        // gDeviceDriverImpl.AddRoot(FS_SAVE_DIR);
        sInstance->RegisterDevice(&gDeviceDriverImpl);
    }

    void Manager::PlatformTeardown()
    {
        FSClient *clientHandle = gDeviceDriverImpl.GetClientHandle();
        gDeviceDriverImpl.Restore();
        gDeviceDriverImpl.SetClientHandle(NULL);
        FSStatus status = FSDelClient(clientHandle, FS_RET_ALL_ERROR);

        if (status != FS_STATUS_OK)
        {
            EA_ASSERT(status == FS_STATUS_OK);
        }
        Free(clientHandle);
        FSShutdown();
    }

}}}
