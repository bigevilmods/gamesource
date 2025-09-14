// (c) Electronic Arts. All Rights Reserved.

/**
\file
Implements rw::core::filesys::Manager platform specific componentry.
*/

#include "EAAssert/eaassert.h"
#include "EAStdC/EAString.h"
#include "rw/core/filesys/manager.h"
#include "rw/core/filesys/driverimpl.h"

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
        EA_STRLCPY_VERIFY(mCurrDir, AssetManagerJNI::GetExternalRoot(), EAArrayCount(mCurrDir));
        gDeviceDriverImpl.AddRoot("/");
        SetCWD(mCurrDir);
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
