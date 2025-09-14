// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved

#include <xtl.h>

#include "rw/core/controller/deviceinfo.h"
#include "rw/core/controller/devicestate.h"
#include "rw/core/controller/manager.h"
#include "rw/core/controller/deviceeffect.h"

#include "private_rwllmanager.h"

#include "EAStdC/EAMemory.h"

#include "EAAssert/eaassert.h"

namespace rw {
namespace core {
namespace controller {

// Macro to normalize the axis values to a range
inline int32_t NormalizeAxis(int32_t value, int32_t minValue, int32_t maxValue, int32_t minRange, int32_t maxRange)
{
    return ((((maxRange - minRange) * (value - minValue)) / (maxValue - minValue)) - ((maxRange - minRange) / 2));
}

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

void LLManager::Update()
{
    // increment the timestamp
    mTimestamp++;

    // for each connected controller, update the state
    for (uint32_t ctr = 0; ctr < GetNumControllers(); ctr++)
    {
        DeviceState *curDeviceState = mDeviceStateList[ctr];
        DeviceInfo  *curDeviceInfo  = mDeviceInfoList[ctr];

        // get pad state
        XINPUT_STATE xInputState;
        bool isConnected = (XInputGetState(ctr, &xInputState) == ERROR_SUCCESS);

        // if a device is connected, update it
        if (isConnected)
        {
            // if there was no device connected before, set up the info & state
            if (!curDeviceState->IsConnected())
            {
                XINPUT_CAPABILITIES deviceCapabilities;

                XInputGetCapabilities(ctr, XINPUT_FLAG_GAMEPAD, &deviceCapabilities);

                switch(deviceCapabilities.SubType)
                {
                case XINPUT_DEVSUBTYPE_GAMEPAD:

                    // This library doesn't offer specific support for
                    // any controller types other than the wheel at this 
                    // time.
                default:
                    // setup device info
                    curDeviceInfo->Reset(DeviceInfo::PAD, 16, 4);
                    break;

                case XINPUT_DEVSUBTYPE_WHEEL:

                    // TODO: check this against an actual device.
                    curDeviceInfo->Reset(DeviceInfo::WHEEL, 16, 4);
                    break;
                }

                // first 14 buttons are digital, rest are analog
                for (unsigned int i = 14; i < curDeviceInfo->GetNumberButtons(); ++i)
                {
                    curDeviceInfo->SetButtonCapability(i, DeviceInfo::ANALOG);
                }

                // setup device state
                curDeviceState->Reset(DeviceState::CONNECTED, 14, 2, 4);
            }

            // set the timestamp
            curDeviceState->SetTimestamp(mTimestamp);

            // set digital buttons
            for (uint32_t i = 0; i < 10; ++i)
            {
                curDeviceState->SetButtonValue(i,
                    static_cast<uint8_t>(((xInputState.Gamepad.wButtons & (1 << i)) == (1 << i)) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE));
            }

            for (uint32_t i = 10; i < 14; ++i)
            {
                curDeviceState->SetButtonValue(i,
                    static_cast<uint8_t>(((xInputState.Gamepad.wButtons & (1 << (i+2))) == (1 << (i+2))) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE));
            }

            // set analog buttons
            curDeviceState->SetButtonValue(
                curDeviceState->GetNumberDigitalButtons() + 0,
                xInputState.Gamepad.bLeftTrigger);

            curDeviceState->SetButtonValue(
                curDeviceState->GetNumberDigitalButtons() + 1,
                xInputState.Gamepad.bRightTrigger);

            if (Manager::IsDeadZoneFilteringEnabled() &&
                (curDeviceInfo->GetType() == DeviceInfo::PAD))
            {
                const float XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE_FLOAT = static_cast<const float>(
                    XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                const float XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE_FLOAT = static_cast<const float>(
                    XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

                D3DXVECTOR2 LeftThumb(xInputState.Gamepad.sThumbLX, xInputState.Gamepad.sThumbLY);
                float thumbLen = MIN(D3DXVec2Length(&LeftThumb), 32767.0f);

                if (thumbLen < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE_FLOAT)
                {
                    // in dead zone
                    xInputState.Gamepad.sThumbLX = 0;
                    xInputState.Gamepad.sThumbLY = 0;
                }
                else
                {
                    // not in dead zone, so let's scale both axes at once
                    const float scale =
                        (thumbLen - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE_FLOAT) /
                        (32767.0f - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE_FLOAT);

                    xInputState.Gamepad.sThumbLX =
                        static_cast<SHORT>(scale * xInputState.Gamepad.sThumbLX);

                    xInputState.Gamepad.sThumbLY =
                        static_cast<SHORT>(scale * xInputState.Gamepad.sThumbLY);
                }

                D3DXVECTOR2 RightThumb(xInputState.Gamepad.sThumbRX, xInputState.Gamepad.sThumbRY);
                thumbLen = MIN(D3DXVec2Length(&RightThumb), 32767.0f);

                if (thumbLen < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE_FLOAT)
                {
                    // in dead zone
                    xInputState.Gamepad.sThumbRX = 0;
                    xInputState.Gamepad.sThumbRY = 0;
                }
                else
                {
                    // not in dead zone, so let's scale both axes at once
                    const float scale =
                        (thumbLen - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE_FLOAT) /
                        (32767.0f - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE_FLOAT);

                    xInputState.Gamepad.sThumbRX =
                        static_cast<SHORT>(scale * xInputState.Gamepad.sThumbRX);

                    xInputState.Gamepad.sThumbRY =
                        static_cast<SHORT>(scale * xInputState.Gamepad.sThumbRY);
                }
            }

            // set axis
            curDeviceState->SetAxisValue(PAD_AXIS_LEFTX,  NormalizeAxis(xInputState.Gamepad.sThumbLX, -32768, 32767, DeviceState::MIN_AXIS_VALUE, DeviceState::MAX_AXIS_VALUE));
            curDeviceState->SetAxisValue(PAD_AXIS_LEFTY,  NormalizeAxis(xInputState.Gamepad.sThumbLY, -32768, 32767, DeviceState::MIN_AXIS_VALUE, DeviceState::MAX_AXIS_VALUE));
            curDeviceState->SetAxisValue(PAD_AXIS_RIGHTX, NormalizeAxis(xInputState.Gamepad.sThumbRX, -32768, 32767, DeviceState::MIN_AXIS_VALUE, DeviceState::MAX_AXIS_VALUE));
            curDeviceState->SetAxisValue(PAD_AXIS_RIGHTY, NormalizeAxis(xInputState.Gamepad.sThumbRY, -32768, 32767, DeviceState::MIN_AXIS_VALUE, DeviceState::MAX_AXIS_VALUE));
        }
        else
        {
            // see if a device used to be connected
            if (curDeviceState->IsConnected())
            {
                // device was removed
                curDeviceInfo->Reset(DeviceInfo::UNKNOWN, 0, 0);
                curDeviceState->Reset(DeviceState::DISCONNECTED, 0, 0, 0);
            }
        }
    }
}

const DeviceInfo* LLManager::GetDeviceInfo(unsigned int deviceNumber) const
{
    EA_ASSERT_MSG(deviceNumber < GetNumControllers(), "DeviceNumber too large");

    if (mDeviceInfoList[deviceNumber]->GetType() != DeviceInfo::UNKNOWN)
    {
        return mDeviceInfoList[deviceNumber];
    }

    // no device attached
    return NULL;
}

const DeviceState* LLManager::GetDeviceState(unsigned int deviceNumber) const
{
    EA_ASSERT_MSG(deviceNumber < GetNumControllers(), "DeviceNumber too large");

    return mDeviceStateList[deviceNumber];
}

DeviceEffect* LLManager::GetDeviceEffect(unsigned int deviceNumber) const
{
    EA_ASSERT_MSG(deviceNumber < GetNumControllers(), "DeviceNumber too large");

    return mDeviceEffectList[deviceNumber];
}

LLManager::LLManager(const Options &options)
: mTimestamp(0)
{
    EA::StdC::Memclear(mDeviceInfoList, sizeof(mDeviceInfoList));
    EA::StdC::Memclear(mDeviceStateList, sizeof(mDeviceStateList));
    EA::StdC::Memclear(mDeviceEffectList, sizeof(mDeviceEffectList));

    for (uint32_t ctr = 0; ctr < LLManager::GetNumControllers(); ctr++)
    {
        // alloc space for device info & state
        mDeviceInfoList[ctr] = new DeviceInfo(ctr);
        mDeviceStateList[ctr] = new DeviceState(ctr, DeviceState::LOCAL);

        // alloc space for the effect
        mDeviceEffectList[ctr] = new DeviceEffect(ctr);
    }
}

LLManager::~LLManager()
{
    for (uint32_t ctr = 0; ctr < LLManager::GetNumControllers(); ctr++)
    {
        delete mDeviceEffectList[ctr];
        delete mDeviceStateList[ctr];
        delete mDeviceInfoList[ctr];
    }
}

void *LLManager::operator new(size_t size)
{
    return Manager::GetAllocator()->Alloc(size, "rw::core::controller::LLManager", EA::Allocator::MEM_PERM);
}

void LLManager::operator delete(void *p)
{
    Manager::GetAllocator()->Free(p);
}

}}}
