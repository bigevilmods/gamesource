// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::filesys::Manager platform specific componentry.
*/

#include "EAAssert/eaassert.h"
#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/driverimpl.h"
#include <unistd.h>
#include <jni.h>

namespace rw {
namespace core {
namespace filesys {

#ifndef ASSETMANAGER_INIT_FAILURE_MESSAGE
    #define ASSETMANAGER_INIT_FAILURE_MESSAGE "Note: rwfilesystem.Startup(activityInstance) must be called from Java."
#endif
#include "assetmanagerjni.h"

    void Manager::PlatformInit()
    {
        strcpy(mCurrDir, AssetManagerJNI::GetExternalRoot());
        gDeviceDriverImpl.AddRoot("/");
        chdir(mCurrDir);
        sInstance->RegisterDevice(&gDeviceDriverImpl);
        gAppBundleDeviceDriverImpl.AddRoot("appbundle:");
        sInstance->RegisterDevice(&gAppBundleDeviceDriverImpl);
    }


    void Manager::PlatformTeardown()
    {
        gDeviceDriverImpl.Restore();
        gAppBundleDeviceDriverImpl.Restore();
    }

}}}
