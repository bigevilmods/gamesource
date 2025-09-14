// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved

#include "rw/core/controller/deviceinfo.h"
#include "rw/core/controller/devicestate.h"
#include "rw/core/controller/manager.h"
#include "rw/core/controller/deviceeffect.h"

#include "private_rwllmanager.h"

#include "EAStdC/EAMemory.h"
#include "EAStdC/EASprintf.h"

#include "EAAssert/eaassert.h"

// get _XDK_VER
#include <xdk.h>


#if _XDK_VER >= 9299
	using namespace Windows::Xbox;
#else
	using namespace Microsoft::Xbox;
#endif

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

uint64			LLManager::sGamepadIDs[Manager::MAX_DEVICES];
static uint8	sSimulatedButtonPushes[Manager::MAX_DEVICES][PAD_BUTTON_ID_MAX];


EA::Thread::Futex	LLManager::sMutex;
bool LLManager::sFakeNullPads = false;

void LLManager::SetNullPads(bool enable)
{
	sFakeNullPads = enable;
}

static inline void SetDigitalButtonValue(DeviceState* deviceState, PadButtonId buttonID, bool controllerReading, uint8 simulatedValue)
{
	deviceState->SetButtonValue(buttonID,
			controllerReading || simulatedValue != 0 ?
			static_cast<uint8_t>(DeviceState::MAX_BUTTON_VALUE) : static_cast<uint8_t>(DeviceState::MIN_BUTTON_VALUE)) ;
}

void LLManager::Update()
{

    // increment the timestamp
    mTimestamp++;

    using namespace Input;
    using namespace Windows::Foundation::Collections;

	uint64 gamepadIDs[Manager::MAX_DEVICES];
	uint8 simulatedPushes[Manager::MAX_DEVICES][PAD_BUTTON_ID_MAX];
	{
		EA::Thread::AutoFutex mutex(sMutex);
		EA::StdC::Memcpy(gamepadIDs, sGamepadIDs, sizeof(gamepadIDs));
		for( size_t ctr = 0 ; ctr < Manager::MAX_DEVICES ; ++ctr )
		{
			for( size_t button = 0 ; button < PAD_BUTTON_ID_MAX ; ++button )
			{
				simulatedPushes[ctr][button] = sSimulatedButtonPushes[ctr][button];
				if( sSimulatedButtonPushes[ctr][button] > 0 )
				{
					--(sSimulatedButtonPushes[ctr][button]) ;
				}
			}
		}
	}

	for (size_t ctr = 0; ctr < sizeof(gamepadIDs) / sizeof(gamepadIDs[0]); ctr++)
	{
		DeviceInfo *curDeviceInfo = mDeviceInfoList[ctr];
		DeviceState *curDeviceState = mDeviceStateList[ctr];
		
		// if there was no device connected before but is now, set up the info & state
		if (gamepadIDs[ctr] && !curDeviceState->IsConnected())
		{
			// setup device info
			curDeviceInfo->Reset(DeviceInfo::PAD, PAD_BUTTON_ID_MAX, 4);

			// first 15 buttons are digital (binary on/off), rest are analog
			for (unsigned int i = PAD_BUTTON_ID_MAX - 2; i < PAD_BUTTON_ID_MAX; ++i)
			{
				curDeviceInfo->SetButtonCapability(i, DeviceInfo::ANALOG);
			}

			// setup device state
			curDeviceState->Reset(DeviceState::CONNECTED, PAD_BUTTON_ID_MAX - 2, 2, 4);
		}

		// see if a device used to be connected
		if (!gamepadIDs[ctr] && curDeviceState->IsConnected())
		{
			// device was removed
			curDeviceInfo->Reset(DeviceInfo::UNKNOWN, 0, 0);
			curDeviceState->Reset(DeviceState::DISCONNECTED, 0, 0, 0);
		}
	}
	
    // loop through each controller to set device state
    for (size_t ctr = 0; ctr < sizeof(gamepadIDs) / sizeof(gamepadIDs[0]); ctr++)
    {
        DeviceInfo *curDeviceInfo = mDeviceInfoList[ctr];
		DeviceState *curDeviceState = mDeviceStateList[ctr];
        if (gamepadIDs[ctr])
        {
			try
			{
				IVectorView<IGamepad ^> ^allGamepads = Gamepad::Gamepads;
				IGamepadReading ^gamepadReading = nullptr;
				IGamepad ^gamepad = nullptr;
				for (uint32 i = 0; i < allGamepads->Size; i++)
				{
					gamepad = allGamepads->GetAt(i);

					if (gamepad->Id == gamepadIDs[ctr])
					{
						gamepadReading = gamepad->GetCurrentReading();
						break;
					}
				}
				if (gamepadReading == nullptr)
				{
					continue;
				}

				// set the timestamp
				curDeviceState->SetTimestamp(mTimestamp);

				if( sFakeNullPads )
				{
					// set digital buttons
					curDeviceState->SetButtonValue(PAD_BUTTON_UP,		static_cast<uint8_t>(DeviceState::MIN_BUTTON_VALUE) );
					curDeviceState->SetButtonValue(PAD_BUTTON_DOWN,		static_cast<uint8_t>(DeviceState::MIN_BUTTON_VALUE) );
					curDeviceState->SetButtonValue(PAD_BUTTON_LEFT,		static_cast<uint8_t>(DeviceState::MIN_BUTTON_VALUE) );
					curDeviceState->SetButtonValue(PAD_BUTTON_RIGHT,	static_cast<uint8_t>(DeviceState::MIN_BUTTON_VALUE) );
		#if (defined(EA_PLATFORM_XBOXONE) && _XDK_VER >= 9386)		
		// new april SDK
					curDeviceState->SetButtonValue(PAD_BUTTON_START,	static_cast<uint8_t>(DeviceState::MIN_BUTTON_VALUE) );
					curDeviceState->SetButtonValue(PAD_BUTTON_BACK,		static_cast<uint8_t>(DeviceState::MIN_BUTTON_VALUE) );
		#else
		// old pre-april SDK
					curDeviceState->SetButtonValue(PAD_BUTTON_START,		static_cast<uint8_t>(DeviceState::MIN_BUTTON_VALUE) );
					curDeviceState->SetButtonValue(PAD_BUTTON_BACK,		static_cast<uint8_t>(DeviceState::MIN_BUTTON_VALUE) );
		#endif
					curDeviceState->SetButtonValue(PAD_BUTTON_LTHUMB,		static_cast<uint8_t>(DeviceState::MIN_BUTTON_VALUE) );
					curDeviceState->SetButtonValue(PAD_BUTTON_RTHUMB,		static_cast<uint8_t>(DeviceState::MIN_BUTTON_VALUE) );
					curDeviceState->SetButtonValue(PAD_BUTTON_LEFT_SHOULDER,		static_cast<uint8_t>(DeviceState::MIN_BUTTON_VALUE) );
					curDeviceState->SetButtonValue(PAD_BUTTON_RIGHT_SHOULDER,		static_cast<uint8_t>(DeviceState::MIN_BUTTON_VALUE) );
					curDeviceState->SetButtonValue(PAD_BUTTON_A,		static_cast<uint8_t>(DeviceState::MIN_BUTTON_VALUE) );
					curDeviceState->SetButtonValue(PAD_BUTTON_B,		static_cast<uint8_t>(DeviceState::MIN_BUTTON_VALUE) );
					curDeviceState->SetButtonValue(PAD_BUTTON_X,		static_cast<uint8_t>(DeviceState::MIN_BUTTON_VALUE) );
					curDeviceState->SetButtonValue(PAD_BUTTON_Y,		static_cast<uint8_t>(DeviceState::MIN_BUTTON_VALUE) );

					// set analog buttons
					curDeviceState->SetButtonValue(PAD_BUTTON_LTRIGGER, static_cast<uint8_t>(0));
					curDeviceState->SetButtonValue(PAD_BUTTON_RTRIGGER, static_cast<uint8_t>(0));

					// set axis
					curDeviceState->SetAxisValue(PAD_AXIS_LEFTX,  static_cast<int>(0));
					curDeviceState->SetAxisValue(PAD_AXIS_LEFTY,  static_cast<int>(0));
					curDeviceState->SetAxisValue(PAD_AXIS_RIGHTX, static_cast<int>(0));
					curDeviceState->SetAxisValue(PAD_AXIS_RIGHTY, static_cast<int>(0));
				}
				else
				{

					// set digital buttons

					SetDigitalButtonValue(curDeviceState, PAD_BUTTON_UP, gamepadReading->IsDPadUpPressed, simulatedPushes[ctr][PAD_BUTTON_UP]);
					SetDigitalButtonValue(curDeviceState, PAD_BUTTON_DOWN, gamepadReading->IsDPadDownPressed, simulatedPushes[ctr][PAD_BUTTON_DOWN]);
					SetDigitalButtonValue(curDeviceState, PAD_BUTTON_LEFT, gamepadReading->IsDPadLeftPressed, simulatedPushes[ctr][PAD_BUTTON_LEFT]);
					SetDigitalButtonValue(curDeviceState, PAD_BUTTON_RIGHT, gamepadReading->IsDPadRightPressed, simulatedPushes[ctr][PAD_BUTTON_RIGHT]);
		#if (defined(EA_PLATFORM_XBOXONE) && _XDK_VER >= 9386)		
		// new april SDK
					SetDigitalButtonValue(curDeviceState, PAD_BUTTON_START, gamepadReading->IsMenuPressed, simulatedPushes[ctr][PAD_BUTTON_START]);
					SetDigitalButtonValue(curDeviceState, PAD_BUTTON_BACK, gamepadReading->IsViewPressed, simulatedPushes[ctr][PAD_BUTTON_BACK]);
		#else
		// old pre-april SDK
					SetDigitalButtonValue(curDeviceState, PAD_BUTTON_START, gamepadReading->IsStartPressed, simulatedPushes[ctr][PAD_BUTTON_START]);
					SetDigitalButtonValue(curDeviceState, PAD_BUTTON_BACK, gamepadReading->IsBackPressed, simulatedPushes[ctr][PAD_BUTTON_BACK]);
		#endif
					SetDigitalButtonValue(curDeviceState, PAD_BUTTON_LTHUMB, gamepadReading->IsLeftThumbstickPressed, simulatedPushes[ctr][PAD_BUTTON_LTHUMB]);
					SetDigitalButtonValue(curDeviceState, PAD_BUTTON_RTHUMB, gamepadReading->IsRightThumbstickPressed, simulatedPushes[ctr][PAD_BUTTON_RTHUMB]);
					SetDigitalButtonValue(curDeviceState, PAD_BUTTON_LEFT_SHOULDER, gamepadReading->IsLeftShoulderPressed, simulatedPushes[ctr][PAD_BUTTON_LEFT_SHOULDER]);
					SetDigitalButtonValue(curDeviceState, PAD_BUTTON_RIGHT_SHOULDER, gamepadReading->IsRightShoulderPressed, simulatedPushes[ctr][PAD_BUTTON_RIGHT_SHOULDER]);
					SetDigitalButtonValue(curDeviceState, PAD_BUTTON_A, gamepadReading->IsAPressed, simulatedPushes[ctr][PAD_BUTTON_A]);
					SetDigitalButtonValue(curDeviceState, PAD_BUTTON_B, gamepadReading->IsBPressed, simulatedPushes[ctr][PAD_BUTTON_B]);
					SetDigitalButtonValue(curDeviceState, PAD_BUTTON_X, gamepadReading->IsXPressed, simulatedPushes[ctr][PAD_BUTTON_X]);
					SetDigitalButtonValue(curDeviceState, PAD_BUTTON_Y, gamepadReading->IsYPressed, simulatedPushes[ctr][PAD_BUTTON_Y]);

					// set analog buttons
					curDeviceState->SetButtonValue(PAD_BUTTON_LTRIGGER, static_cast<uint8_t>(DeviceState::MAX_BUTTON_VALUE * gamepadReading->LeftTrigger));
					curDeviceState->SetButtonValue(PAD_BUTTON_RTRIGGER, static_cast<uint8_t>(DeviceState::MAX_BUTTON_VALUE * gamepadReading->RightTrigger));

					// set axis
					curDeviceState->SetAxisValue(PAD_AXIS_LEFTX,  static_cast<int>(DeviceState::MAX_AXIS_VALUE * gamepadReading->LeftThumbstickX));
					curDeviceState->SetAxisValue(PAD_AXIS_LEFTY,  static_cast<int>(DeviceState::MAX_AXIS_VALUE * gamepadReading->LeftThumbstickY));
					curDeviceState->SetAxisValue(PAD_AXIS_RIGHTX, static_cast<int>(DeviceState::MAX_AXIS_VALUE * gamepadReading->RightThumbstickX));
					curDeviceState->SetAxisValue(PAD_AXIS_RIGHTY, static_cast<int>(DeviceState::MAX_AXIS_VALUE * gamepadReading->RightThumbstickY));

				}
			} //try
			catch(Platform::COMException^ e)
			{
				return;
			}
			catch(Platform::Exception^ e)
			{
				return;
			}
			catch(...)
			{
				return;
			}
		} //if
    } //for
}

const DeviceInfo* LLManager::GetDeviceInfo(unsigned int deviceNumber) const
{
	if (deviceNumber < GetNumControllers())
	{
		if (mDeviceInfoList[deviceNumber]->GetType() != DeviceInfo::UNKNOWN)
		{
			return mDeviceInfoList[deviceNumber];
		}
	}
	else
	{
		EA_ASSERT_MSG(deviceNumber < GetNumControllers(), "DeviceNumber too large");
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

	try
	{
		EA::StdC::Memclear(sGamepadIDs, sizeof(sGamepadIDs));
		Windows::Foundation::Collections::IVectorView<Input::IGamepad ^> ^allGamepads = Input::Gamepad::Gamepads;
		for (uint32 ctr = 0, idx = 0; ctr < allGamepads->Size && idx < sizeof(sGamepadIDs) / sizeof(sGamepadIDs[0]); ctr++)
		{
			sGamepadIDs[idx++] = allGamepads->GetAt(ctr)->Id;
		}
	}
	catch(Platform::COMException^ e)
	{
		//If your gamepads are messed up right at init time, you probably have bigger problems.  
		//Consider changing this to an assert to catch on bootflow, but don't mess with smokes right now.
		return;
	}
	catch(Platform::Exception^ e)
	{
		return;
	}
	catch(...)
	{
		return;
	}

	Input::Gamepad::GamepadAdded += ref new Windows::Foundation::EventHandler<Input::GamepadAddedEventArgs ^>(GamepadAdded, Platform::CallbackContext::Any);
	Input::Gamepad::GamepadRemoved += ref new Windows::Foundation::EventHandler<Input::GamepadRemovedEventArgs ^>(GamepadRemoved, Platform::CallbackContext::Any);
	Windows::ApplicationModel::Core::CoreApplication::Resuming += ref new Windows::Foundation::EventHandler<Platform::Object^>(Resuming, Platform::CallbackContext::Any);
	EA::StdC::Memclear(&sSimulatedButtonPushes[0][0], sizeof(sSimulatedButtonPushes));
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

void LLManager::GamepadAdded(_In_ Platform::Object ^sender, _In_ Windows::Xbox::Input::GamepadAddedEventArgs ^args)
{
	EA::Thread::AutoFutex mutex(sMutex);
	for (size_t i = 0; i < sizeof(sGamepadIDs) / sizeof(sGamepadIDs[0]); i++)
	{
		if (sGamepadIDs[i] == args->Gamepad->Id)
		{
			return;
		}
	}
	for (size_t i = 0; i < sizeof(sGamepadIDs) / sizeof(sGamepadIDs[0]); i++)
	{
		if (!sGamepadIDs[i])
		{
			sGamepadIDs[i] = args->Gamepad->Id;
			break;
		}
	}
}

void LLManager::GamepadRemoved(_In_ Platform::Object ^sender, _In_ Windows::Xbox::Input::GamepadRemovedEventArgs ^args)
{
	EA::Thread::AutoFutex mutex(sMutex);
	for (size_t i = 0; i < sizeof(sGamepadIDs) / sizeof(sGamepadIDs[0]); i++)
	{
		if (sGamepadIDs[i] == args->Gamepad->Id)
		{
			sGamepadIDs[i] = 0;
			break;
		}
	}
}

void LLManager::Resuming(_In_ Platform::Object^ sender, _In_ Platform::Object^ args)
{
	try
	{
		EA::Thread::AutoFutex mutex(sMutex);
		EA::StdC::Memclear(sGamepadIDs, sizeof(sGamepadIDs));
		Windows::Foundation::Collections::IVectorView<Input::IGamepad ^> ^allGamepads = Input::Gamepad::Gamepads;
		for (uint32 ctr = 0, idx = 0; ctr < allGamepads->Size && idx < sizeof(sGamepadIDs) / sizeof(sGamepadIDs[0]); ctr++)
		{
			sGamepadIDs[idx++] = allGamepads->GetAt(ctr)->Id;
		}
	}
	catch(Platform::COMException^ e)
	{
		EA_ASSERT_MSG(false, "Failed to re-init controllers!");
		return;
	}
	catch(Platform::Exception^ e)
	{
		return;
	}
	catch(...)
	{
		return;
	}
}

uint64_t LLManager::GetGamePadID(int index)
{	
	EA::Thread::AutoFutex mutex(sMutex);
	if ( index >= 0 && index < Manager::MAX_DEVICES )
	{
		return sGamepadIDs[index];
	}
	else
	{
		return 0; // 0 would be invalid in Durango world
	}
}

void LLManager::SimulateButtonPush(unsigned int deviceNumber, const int* buttonIds, unsigned int numButtons, unsigned int numFrames)
{
	EA::Thread::AutoFutex mutex(sMutex);

	EA_ASSERT(deviceNumber < GetNumControllers());
	EA_ASSERT(numFrames <= 255);

	for( size_t button = 0 ; button < numButtons ; ++button )
	{
		EA_ASSERT(buttonIds[button] < PAD_BUTTON_ID_MAX);
		sSimulatedButtonPushes[deviceNumber][buttonIds[button]] = (uint8)numFrames ;
	}
}

}}}
