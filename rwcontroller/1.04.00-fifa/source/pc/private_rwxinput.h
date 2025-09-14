// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved

/*      PC specific controller manager class definition */

#ifndef RW_CORE_CONTROLLER_XINPUT_H
#define RW_CORE_CONTROLLER_XINPUT_H

#if defined _MSC_VER
#pragma once
#endif

#include "rw/core/controller/deviceeffect.h"

#include <windows.h>
#include <xinput.h>

namespace rw {
namespace core {
namespace controller {

// There are always four controllers
const int NUM_XINPUT_CONTROLLERS = 4;

class XInputManager
{
public:
    XInputManager(bool active, DeviceInfo **, DeviceState **);
    ~XInputManager();

    int GetNumXInputControllers() const
    {
        return mActive ? NUM_XINPUT_CONTROLLERS : 0;
    }

    DeviceEffect *GetXInputDeviceEffect(unsigned int deviceNumber) const
    {
        return mDeviceEffectList[deviceNumber];
    }

    static bool IsXInputDevice(const GUID* pGuidProductFromDirectInput);
    void Update(uint32_t timeStamp);

    // new/delete operators
    static void *operator new(size_t size);
    static void operator delete(void *p);

private:

    DeviceInfo    **mDeviceInfoList;
    DeviceState   **mDeviceStateList;
    DeviceEffect   *mDeviceEffectList[NUM_XINPUT_CONTROLLERS];

    bool mActive;
};
}}}

#endif
