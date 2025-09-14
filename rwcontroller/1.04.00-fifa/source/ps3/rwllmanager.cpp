// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved

#include "rw/core/controller/deviceinfo.h"
#include "rw/core/controller/devicestate.h"
#include "rw/core/controller/manager.h"
#include "rw/core/controller/deviceeffect.h"
#include "private_rwllmanager.h"
#include "EAStdC/EAMemory.h"
#include "EAAssert/eaassert.h"

#include <cellstatus.h>
#include <cell/pad/error.h>

namespace rw {
namespace core {
namespace controller {

// Macro to normalize the axis values to a range
inline int32_t NormalizeAxis(int32_t value, int32_t minValue, int32_t maxValue, int32_t minRange, int32_t maxRange)
{
    return ((((maxRange - minRange) * (value - minValue)) / (maxValue - minValue)) - ((maxRange - minRange) / 2));
}

void LLManager::Update()
{
    // increment the timestamp
    mTimestamp++;

    int ret = cellPadPeriphGetInfo(&PadInfo);

    if (0 != ret)
    {
        // throw a helpful error message if the user forgot to
        // initialize the cell pad library before rwcontroller.
        EA_ASSERT_MSG(ret != CELL_PAD_ERROR_UNINITIALIZED,
            "The cell pad library is not initialized.  Call Sony's cellPadInit() first.");

        return;
    }

    if (PadInfo.system_info & CELL_PAD_INFO_INTERCEPTED)
    {
        // special case, zero out all connected controller inputs
        for (uint32_t ctr = 0; ctr < GetNumControllers(); ctr++)
        {
            DeviceState *curDeviceState = mDeviceStateList[ctr];
            if (curDeviceState->IsConnected())
            {
                // set the timestamp
                curDeviceState->SetTimestamp(mTimestamp);

                // 4 digital buttons
                curDeviceState->SetButtonValue( PAD_BUTTON_SELECT,  0);
                curDeviceState->SetButtonValue( PAD_BUTTON_START,   0);
                curDeviceState->SetButtonValue( PAD_BUTTON_LTHUMB,  0);
                curDeviceState->SetButtonValue( PAD_BUTTON_RTHUMB,  0);

                // analog buttons
                curDeviceState->SetButtonValue( PAD_BUTTON_UP,      0);
                curDeviceState->SetButtonValue( PAD_BUTTON_RIGHT,   0);
                curDeviceState->SetButtonValue( PAD_BUTTON_DOWN,    0);
                curDeviceState->SetButtonValue( PAD_BUTTON_LEFT,    0);
                curDeviceState->SetButtonValue( PAD_BUTTON_SQUARE,  0);
                curDeviceState->SetButtonValue( PAD_BUTTON_X,       0);
                curDeviceState->SetButtonValue( PAD_BUTTON_CIRCLE,  0);
                curDeviceState->SetButtonValue( PAD_BUTTON_TRIANGLE,0);
                curDeviceState->SetButtonValue( PAD_BUTTON_L1,      0);
                curDeviceState->SetButtonValue( PAD_BUTTON_R1,      0);
                curDeviceState->SetButtonValue( PAD_BUTTON_L2,      0);
                curDeviceState->SetButtonValue( PAD_BUTTON_R2,      0);

                // set axis
                curDeviceState->SetAxisValue(PAD_AXIS_LEFTX,        0);
                curDeviceState->SetAxisValue(PAD_AXIS_RIGHTX,       0);

                curDeviceState->SetAxisValue(PAD_AXIS_LEFTY,        0);
                curDeviceState->SetAxisValue(PAD_AXIS_RIGHTY,       0);

                if (curDeviceState->HasSensorData())
                {
                    curDeviceState->SetAxisValue(PAD_AXIS_SENSORX, 0);
                    curDeviceState->SetAxisValue(PAD_AXIS_SENSORY, 0);
                    curDeviceState->SetAxisValue(PAD_AXIS_SENSORZ, 0);
                    curDeviceState->SetAxisValue(PAD_AXIS_SENSORG, 0);
                }
            }
        }
    }

    // for each connected controller, update the state
    for (uint32_t ctr = 0; ctr < GetNumControllers(); ctr++)
    {
        DeviceState *curDeviceState = mDeviceStateList[ctr];
        DeviceInfo  *curDeviceInfo  = mDeviceInfoList[ctr];

        // if a device is connected, update it
		// FIFA fix for the controller port swap scenario
		// The controller swap is considered to be a one-frame disconnect by the PS3 libraries
		// therefore we need to ensure that the device attached state is set to false for one
		// frame as well
		const uint32_t kDeviceMask = CELL_PAD_STATUS_CONNECTED | CELL_PAD_STATUS_ASSIGN_CHANGES;
		const uint32_t kDeviceValue = CELL_PAD_STATUS_CONNECTED;

		if((PadInfo.port_status[ctr] & kDeviceMask) == kDeviceValue)
        {
            // if there was no device connected before, set up the info & state
            if (!curDeviceState->IsConnected())
            {
                // setup device info
                switch (PadInfo.device_type[ctr])
                {
                case CELL_PAD_DEV_TYPE_STANDARD:
                    curDeviceInfo->Reset(DeviceInfo::PAD, 16, 8);
                    break;

                case CELL_PAD_DEV_TYPE_LDD:
                    curDeviceInfo->Reset(DeviceInfo::WHEEL, 16, 8);
                    break;

                default:
                    EA_FAIL_MSG("Unknown pad type");

                    // assume it's a PAD
                    curDeviceInfo->Reset(DeviceInfo::PAD, 16, 8);
                    break;
                }

                // setup device state.  4 digital buttons, 12 analog buttons, 8 axis
                curDeviceState->Reset(DeviceState::CONNECTED, 4, 12, 8);

                // check if sensor mode is supported
                if (PadInfo.device_capability[ctr] & CELL_PAD_CAPABILITY_SENSOR_MODE)
                {
                    curDeviceState->HasSensorData(true);
                    cellPadSetPortSetting(ctr, CELL_PAD_SETTING_SENSOR_ON);
                }
                else
                {
                    curDeviceState->HasSensorData(false);
                }

                curDeviceState->HasPressData(false);
                
                for (int j=4; j<16; ++j)
                {
                    curDeviceInfo->SetButtonCapability(j,(DeviceInfo::ButtonCapability)0);
                }

                // check if press mode is supported
                if (PadInfo.device_capability[ctr] & CELL_PAD_CAPABILITY_PRESS_MODE)
                {
                    ret = cellPadSetPortSetting(ctr, CELL_PAD_SETTING_PRESS_ON);
                    if(ret == CELL_OK)
                    {
                        curDeviceState->HasPressData(true);
                        for (int j=4; j<16; ++j)
                        {
                            curDeviceInfo->SetButtonCapability(j,DeviceInfo::ANALOG);
                        }
                    }
                }
            }

            // set the timestamp
            curDeviceState->SetTimestamp(mTimestamp);

            CellPadData PadData;
            ret = cellPadGetData(ctr, &PadData);

            // Ignore packets of zero length
            if (PadData.len == 0)
            {
                continue;
            }

            // update data for TMPGetButtonData() function
            buttons = *reinterpret_cast<uint32_t *>(&PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL1]);

            // 4 digital buttons
            curDeviceState->SetButtonValue( PAD_BUTTON_SELECT,  (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_SELECT) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
            curDeviceState->SetButtonValue( PAD_BUTTON_START,   (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_START)  ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
            curDeviceState->SetButtonValue( PAD_BUTTON_LTHUMB,  (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_L3)     ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
            curDeviceState->SetButtonValue( PAD_BUTTON_RTHUMB,  (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_R3)     ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );

            if (curDeviceState->HasPressData())
            {
                // set analogue buttons
                curDeviceState->SetButtonValue( PAD_BUTTON_UP,      PadData.button[CELL_PAD_BTN_OFFSET_PRESS_UP]);
                curDeviceState->SetButtonValue( PAD_BUTTON_RIGHT,   PadData.button[CELL_PAD_BTN_OFFSET_PRESS_RIGHT]);
                curDeviceState->SetButtonValue( PAD_BUTTON_DOWN,    PadData.button[CELL_PAD_BTN_OFFSET_PRESS_DOWN]);
                curDeviceState->SetButtonValue( PAD_BUTTON_LEFT,    PadData.button[CELL_PAD_BTN_OFFSET_PRESS_LEFT]);

                curDeviceState->SetButtonValue( PAD_BUTTON_SQUARE,  PadData.button[CELL_PAD_BTN_OFFSET_PRESS_SQUARE]);
                curDeviceState->SetButtonValue( PAD_BUTTON_X,       PadData.button[CELL_PAD_BTN_OFFSET_PRESS_CROSS]);
                curDeviceState->SetButtonValue( PAD_BUTTON_CIRCLE,  PadData.button[CELL_PAD_BTN_OFFSET_PRESS_CIRCLE]);
                curDeviceState->SetButtonValue( PAD_BUTTON_TRIANGLE,PadData.button[CELL_PAD_BTN_OFFSET_PRESS_TRIANGLE]);

                curDeviceState->SetButtonValue( PAD_BUTTON_L1,      PadData.button[CELL_PAD_BTN_OFFSET_PRESS_L1]);
                curDeviceState->SetButtonValue( PAD_BUTTON_R1,      PadData.button[CELL_PAD_BTN_OFFSET_PRESS_R1]);
                curDeviceState->SetButtonValue( PAD_BUTTON_L2,      PadData.button[CELL_PAD_BTN_OFFSET_PRESS_L2]);
                curDeviceState->SetButtonValue( PAD_BUTTON_R2,      PadData.button[CELL_PAD_BTN_OFFSET_PRESS_R2]);

				// System software 4.2.1 issue with motion controller.
				// Motion controllers register as having CELL_PAD_CAPABILITY_PRESS_MODE, but never registers pressure data
				// from square, cross, circle or triangle.
				// Result is motion controller face buttons are not forwarded to device state.
				
				// If analog is zero, check digital inputs and push a fake analogue value.
				if( PadData.button[CELL_PAD_BTN_OFFSET_PRESS_SQUARE]	== 0 )
				{
					curDeviceState->SetButtonValue(PAD_BUTTON_SQUARE,	(PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_SQUARE)	? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
				}
				if( PadData.button[CELL_PAD_BTN_OFFSET_PRESS_CROSS]		== 0 )
				{
					curDeviceState->SetButtonValue(PAD_BUTTON_X,		(PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CROSS)	? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
				}
				if( PadData.button[CELL_PAD_BTN_OFFSET_PRESS_CIRCLE]	== 0 )
				{
					curDeviceState->SetButtonValue(PAD_BUTTON_CIRCLE,	(PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CIRCLE)	? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
				}
				if( PadData.button[CELL_PAD_BTN_OFFSET_PRESS_TRIANGLE]	== 0 )
				{
					curDeviceState->SetButtonValue(PAD_BUTTON_TRIANGLE, (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_TRIANGLE)	? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
				}
            }
            else
            {
                // set digital buttons
                curDeviceState->SetButtonValue( PAD_BUTTON_UP,      (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_UP)    ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
                curDeviceState->SetButtonValue( PAD_BUTTON_RIGHT,   (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_RIGHT) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
                curDeviceState->SetButtonValue( PAD_BUTTON_DOWN,    (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_DOWN)  ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
                curDeviceState->SetButtonValue( PAD_BUTTON_LEFT,    (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_LEFT)  ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );

                curDeviceState->SetButtonValue( PAD_BUTTON_SQUARE,  (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_SQUARE)   ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
                curDeviceState->SetButtonValue( PAD_BUTTON_X,       (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CROSS)    ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
                curDeviceState->SetButtonValue( PAD_BUTTON_CIRCLE,  (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CIRCLE)   ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
                curDeviceState->SetButtonValue( PAD_BUTTON_TRIANGLE,(PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_TRIANGLE) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );

                curDeviceState->SetButtonValue( PAD_BUTTON_L1,      (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L1) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
                curDeviceState->SetButtonValue( PAD_BUTTON_R1,      (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R1) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
                curDeviceState->SetButtonValue( PAD_BUTTON_L2,      (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L2) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
                curDeviceState->SetButtonValue( PAD_BUTTON_R2,      (PadData.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R2) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
            }

            // set axis
            curDeviceState->SetAxisValue(PAD_AXIS_LEFTX,  NormalizeAxis((int32_t)PadData.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_X],  (int32_t)0, (int32_t)255, (int32_t) DeviceState::MIN_AXIS_VALUE, (int32_t) DeviceState::MAX_AXIS_VALUE));
            curDeviceState->SetAxisValue(PAD_AXIS_RIGHTX, NormalizeAxis((int32_t)PadData.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_X], (int32_t)0, (int32_t)255, (int32_t) DeviceState::MIN_AXIS_VALUE, (int32_t) DeviceState::MAX_AXIS_VALUE));

            // min and max ranges have been inverted for the Y-axis so that the return value from GetAxisValue()
            // is consistent for all platforms (that is pushing up will return a positive value).
            curDeviceState->SetAxisValue(PAD_AXIS_LEFTY,  NormalizeAxis((int32_t)PadData.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_Y],  (int32_t)0, (int32_t)255, (int32_t) DeviceState::MAX_AXIS_VALUE, (int32_t) DeviceState::MIN_AXIS_VALUE));
            curDeviceState->SetAxisValue(PAD_AXIS_RIGHTY, NormalizeAxis((int32_t)PadData.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_Y], (int32_t)0, (int32_t)255, (int32_t) DeviceState::MAX_AXIS_VALUE, (int32_t) DeviceState::MIN_AXIS_VALUE));

            if (curDeviceState->HasSensorData())
            {
                curDeviceState->SetAxisValue(PAD_AXIS_SENSORX, NormalizeAxis((int32_t)PadData.button[CELL_PAD_BTN_OFFSET_SENSOR_X], (int32_t)0, (int32_t)0x3FF, (int32_t) DeviceState::MIN_AXIS_VALUE, (int32_t) DeviceState::MAX_AXIS_VALUE));
                curDeviceState->SetAxisValue(PAD_AXIS_SENSORY, NormalizeAxis((int32_t)PadData.button[CELL_PAD_BTN_OFFSET_SENSOR_Y], (int32_t)0, (int32_t)0x3FF, (int32_t) DeviceState::MIN_AXIS_VALUE, (int32_t) DeviceState::MAX_AXIS_VALUE));
                curDeviceState->SetAxisValue(PAD_AXIS_SENSORZ, NormalizeAxis((int32_t)PadData.button[CELL_PAD_BTN_OFFSET_SENSOR_Z], (int32_t)0, (int32_t)0x3FF, (int32_t) DeviceState::MIN_AXIS_VALUE, (int32_t) DeviceState::MAX_AXIS_VALUE));
                curDeviceState->SetAxisValue(PAD_AXIS_SENSORG, NormalizeAxis((int32_t)PadData.button[CELL_PAD_BTN_OFFSET_SENSOR_G], (int32_t)0, (int32_t)0x3FF, (int32_t) DeviceState::MIN_AXIS_VALUE, (int32_t) DeviceState::MAX_AXIS_VALUE));
            }
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
: mTimestamp(0), buttons(0)
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
    return Manager::GetAllocator()->Alloc(size,
        "rw::core::controller::LLManager", EA::Allocator::MEM_PERM);
}

void LLManager::operator delete(void *p)
{
    Manager::GetAllocator()->Free(p);
}

}}}
