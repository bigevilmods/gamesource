// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved

#include "rw/core/controller/manager.h"
#include "rw/core/controller/deviceeffect.h"

#include <EABase/EABase.h>
#include <eaassert/eaassert.h>

#if defined(EA_PLATFORM_WINDOWS)
#include <windows.h>
#include <xinput.h>
#endif

#if defined(EA_PLATFORM_XBOXONE)
#include <windows.h>
#include <xdk.h>
#if _XDK_VER < 1224
#include <xinput.h>
#endif
#endif

#if defined(EA_PLATFORM_XENON)
#include <xtl.h>
#endif

#if defined(EA_PLATFORM_PS4)
#include <pad.h>
#endif

#if defined(EA_PLATFORM_PS3)
#include <cell/pad/libpad.h>
#include <cell/pad/error.h>
#include <sdk_version.h>
#endif

#if defined(EA_PLATFORM_XBOXONE)
using namespace Windows::Foundation::Collections;
using namespace Windows::Xbox::Input;
#endif

namespace rw {
namespace core {
namespace controller {

DeviceEffect::DeviceEffect(unsigned int DeviceId)
: mDeviceID(DeviceId),
  mPlaying(false),
  mMagnitudeLeft(0),
  mMagnitudeRight(0)
{
    // empty
}

DeviceEffect::~DeviceEffect()
{
    // stop the effect if it is playing
    if (mPlaying)
    {
        DeviceEffect::Stop();
    }
}

void DeviceEffect::Start()
{
#if defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_XENON) 
    XINPUT_VIBRATION xiVibe = {
        static_cast<WORD>(mMagnitudeLeft*Info::MAX_MAGNITUDE),
        static_cast<WORD>(mMagnitudeRight*Info::MAX_MAGNITUDE)
    };

    XInputSetState(mDeviceID, &xiVibe);

    mPlaying = true;

#elif defined(EA_PLATFORM_XBOXONE)

#if _XDK_VER < 9403
	GamepadVibration vibration;
	vibration.LeftMotorSpeed = mMagnitudeLeft;
	vibration.RightMotorSpeed = mMagnitudeRight;

	if (vibration.LeftMotorSpeed < 0.0f || vibration.RightMotorSpeed < 0.0f)
	{
		vibration.LeftMotorSpeed = 0.0f;
		vibration.RightMotorSpeed = 0.0f;
	}
#else
	GamepadVibration vibration;
	vibration.LeftMotorLevel = mMagnitudeLeft;
	vibration.RightMotorLevel = mMagnitudeRight;
	vibration.LeftTriggerLevel = 0.0f;
	vibration.RightTriggerLevel = 0.0f;

	if (vibration.LeftMotorLevel < 0.0f || vibration.RightMotorLevel < 0.0f)
	{
		vibration.LeftMotorLevel = 0.0f;
		vibration.RightMotorLevel = 0.0f;
	}
#endif

	try
	{
		uint64_t xboxCtrlID = rw::core::controller::Manager::GetInstance()->GetGamePadID(mDeviceID);
		IVectorView<IGamepad^>^ gamepads = Gamepad::Gamepads;
		unsigned int numPads = gamepads->Size;
		for ( unsigned int i = 0; i < numPads; ++i )
		{
			IGamepad^ gamepad = gamepads->GetAt(i);
			if(gamepad->Id == xboxCtrlID)
			{
				gamepad->SetVibration(vibration);
				break;
			}
		}

		// This was the old code. With the new code, *IF* holding on to the list guarantees that it
		// is not modified, then we shouldn't need the exception handling and that is a better fix.
		// Either way, probably all related to the controller flashing bug...
		
		/*for( int i=0; i < numGamepads; i++ )
		{
			IGamepad^ gamepad = Gamepad::Gamepads->GetAt(i);
			if(gamepad->Id == xboxCtrlID)
			{
				gamepad->SetVibration(vibration);
				break;
			}
		}*/
	}
	catch (Platform::Exception ^ e)
	{
		// We are very rarely getting an OutOfBoundsException thrown here. Last crash dump showed Gamepads->Size
		// being 1 and i being 0... so it's as if the list of gamepads got emptied between the time we queried
		// its size and a few lines of code later when we iterated through it. This could possibly be due to
		// an improperly flashed controller, and during 11v11 games it's hard to know if all of QA have flashed.

		// Just fall through, do nothing.
	}

	mPlaying = true;

#elif defined(EA_PLATFORM_PS3)
	// FIFA fix for the controller port swap: we must make sure cellPadGetInfo2 is called only once per frame
	CellPadPeriphInfo PadInfo;
	bool retVal = rw::core::controller::Manager::GetInstance()->TMPGetPadPeriphInfo(PadInfo);

    if (retVal && (PadInfo.device_capability[mDeviceID] & CELL_PAD_CAPABILITY_ACTUATOR))
    {
        // Right motor is either on or off
        if (mMagnitudeRight != 0)
            mMagnitudeRight = 1;

        // scale the left motor down from the portable input range
        // of 0-65535 to the platform-specific range of 0-255
        mMagnitudeLeft /= ((Info::MAX_MAGNITUDE+1)/256);

		CellPadActParam padAct = 
		{
			{ mMagnitudeRight, mMagnitudeLeft }, 
			{ 0 }
		};

        int err = cellPadSetActDirect(mDeviceID, &padAct);
		// Removing this assert... I am getting a error 0x80010005. 
		// for some reason mPlaying is true after pulling the pad. 
		// I am guessing this means I am trying to stop a pad with
		// out actuator but I can't find the error code in cell\pad.h
		// But in sys\return_code.h maybe they are using this. 
		// The resource (process, thread, synchronous object, etc) with the
		// specified identifier does not exist.
		// #define ESRCH       -2147418107	/* 0x80010005 */
        //EA_ASSERT(err == CELL_OK); 
        (void)err;

        mPlaying = true;
    }
    else
    {
        mMagnitudeLeft = mMagnitudeRight = 0;
        mPlaying = false;
    }

#elif defined(EA_PLATFORM_PS4)
	// Actuator setting value range.
    //     +-----------+----------------------+
    //     |Small motor 0=stop, 1-255=vibrate |
    //     |Big motor   0=stop, 1-255=vibrate |
    //     +-----------+----------------------+

	ScePadVibrationParam param;
	param.smallMotor = (uint8_t)(mMagnitudeRight*255); // A user value of 0 means no vibration.
	param.largeMotor = (uint8_t)(mMagnitudeRight*255);

	const EA::Controller::IEAController* ctrl = 
		rw::core::controller::Manager::GetInstance()->GetController(mDeviceID);

	if (ctrl != NULL)
	{
		scePadSetVibration(ctrl->GetControllerID(), &param);
	}

	mPlaying = true;
#endif
}

void DeviceEffect::Stop()
{
    mMagnitudeLeft = mMagnitudeRight = 0;

    Start();

    mPlaying = false;
}

void DeviceEffect::SetInfo(const Info *pInfo)
{
    // set new values
    mMagnitudeLeft = pInfo->MagnitudeLeft;
    mMagnitudeRight = pInfo->MagnitudeRight;

    // start playing if needed
    if ((pInfo->StartPlaying == Info::START) ||
        ((pInfo->StartPlaying == Info::START) && mPlaying))
    {
        Start();
    }
}

void *DeviceEffect::operator new(size_t size)
{
    return Manager::GetAllocator()->Alloc(size,
        "rw::core::controller::DeviceEffect", EA::Allocator::MEM_PERM);
}

void DeviceEffect::operator delete(void *p)
{
    Manager::GetAllocator()->Free(p);
}

}}}
