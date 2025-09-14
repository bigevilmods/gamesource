// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved

#include "rw/core/controller/deviceinfo.h"
#include "rw/core/controller/devicestate.h"
#include "rw/core/controller/manager.h"
#include "rw/core/controller/deviceeffect.h"

#include "private_rwllmanager.h"

#include "EAStdC/EAMemory.h"

#include "EAAssert/eaassert.h"
#include "EAMessage/EAMessage.h"
#include "EAControllerUserPairing/Messages.h"
#include "IEAController/IEAController.h"
#include "gameframework/input.h"
// FIFA_BEGIN | DIVERGENCE | ngmansun | 2015-4-28 | Replace CUP messages with CUPPL messages
#include "gameframework/cupplproxy.h"
// FIFA_END

#include <sceerror.h>
#include <memory.h>
#include <pad.h>
#include <kernel.h>


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

#if !defined(EA_FINAL)
#define RWLLM_DEBUGOUT( s ) printf s
#else
	#define RWLLM_DEBUGOUT( s )
#endif

bool LLManager::HandleMessage(EA::Messaging::MessageId messageId, void* pMessage)
{
	EA::Pairing::ControllerMessage* ctrlMessage = reinterpret_cast<EA::Pairing::ControllerMessage*>(pMessage);

	if (messageId == EA::Pairing::E_CONTROLLER_ADDED)
	{
		printf("RW handling new controller add message\n");
		for( int i = 0; i < Manager::MAX_DEVICES; ++i )
		{
			if (mCtrlPtrs[i] == NULL)
			{
				printf("RW adding controller at index %d\n", i);
				mCtrlPtrs[i] = const_cast <EA::Controller::IEAController*> (ctrlMessage->GetController());
				break;
			}
		}
	}
	else if (messageId == EA::Pairing::E_CONTROLLER_REMOVED)
	{
		for( int i = 0; i < Manager::MAX_DEVICES; ++i )
		{
			if (mCtrlPtrs[i] == ctrlMessage->GetController())
			{
				printf("RW removing controller at index %d\n", i);
				mCtrlPtrs[i] = NULL;
				break;
			}
		}
	}

	return true;
}


void LLManager::Update()
{
	mTimestamp++;

	//Get the controller and check if it's in use.
	EA::Controller::ControllerList controllers = EACUPS->GetControllers();
	EA::Controller::ControllerList::const_iterator controllersEnd = controllers.end();

	for(EA::Controller::ControllerList::const_iterator controllersIt = controllers.begin(); controllersIt != controllersEnd; ++controllersIt)
	{
		int64_t controllerId = (*controllersIt)->GetControllerID();
		if (controllerId >= 0)
		{
			/// temp var and copy only on success
			ScePadData padData;
			int32_t ret = scePadReadState( controllerId, &padData );

			if (ret != SCE_OK)
			{
				RWLLM_DEBUGOUT( ("Unable to read pad state [0x%08X]\n", ret) );
			}
			else
			{
				//Find index
				bool foundIt = false;
				for (int j = 0; j < GetNumControllers(); j++)
				{
					const EA::Controller::IEAController* pad = GetController(j);

					if ((pad != NULL) && (pad->GetControllerID() == controllerId))
					{
						memcpy(&mData[j], &padData, sizeof(padData));	
						foundIt = true;
						break;
					}
				}
			}
		}
	}

	for (uint32_t ctr = 0; ctr < GetNumControllers(); ctr++)
	{
			DeviceState *curDeviceState = mDeviceStateList[ctr];
			DeviceInfo  *curDeviceInfo  = mDeviceInfoList[ctr];

			// if there was no device connected before but is now, set up the info & state
			if (mData[ctr].connected && !curDeviceState->IsConnected())
			{
				curDeviceInfo->Reset(DeviceInfo::PAD, 16, 8);
				// setup device state.  4 digital buttons, 12 analog buttons, 8 axis
				curDeviceState->Reset(DeviceState::CONNECTED, 4, 12, 8);

				for (int j=4; j<16; ++j)
				{
					curDeviceInfo->SetButtonCapability(j,(DeviceInfo::ButtonCapability)0);
				}

				curDeviceInfo->SetButtonCapability(PAD_BUTTON_L2,DeviceInfo::ANALOG);
				curDeviceInfo->SetButtonCapability(PAD_BUTTON_R2,DeviceInfo::ANALOG);
			}

			// see if a device used to be connected
			if (!mData[ctr].connected && curDeviceState->IsConnected())
			{
				// device was removed
				curDeviceInfo->Reset(DeviceInfo::UNKNOWN, 0, 0);
				curDeviceState->Reset(DeviceState::DISCONNECTED, 0, 0, 0);
			}

	}

	// for each connected controller, update the state
	for (uint32_t ctr = 0; ctr < GetNumControllers(); ctr++)
	{
		// don't update controller that are not connected. 
		if(!mData[ctr].connected)
		{
			continue;
		}
		DeviceState *curDeviceState = mDeviceStateList[ctr];
		DeviceInfo  *curDeviceInfo  = mDeviceInfoList[ctr];

		// update data for TMPGetButtonData() function
		uint32_t buttons = mData[ctr].buttons;

		// 4 digital buttons
		// NOTE - Sony has removed the select button but the new controllers and have added the 
		//        touch pad which can be clicked

		curDeviceState->SetButtonValue( PAD_BUTTON_TOUCH_PAD,(buttons & SCE_PAD_BUTTON_TOUCH_PAD) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
		curDeviceState->SetButtonValue( PAD_BUTTON_START,    (buttons & SCE_PAD_BUTTON_START)	 ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
		curDeviceState->SetButtonValue( PAD_BUTTON_LTHUMB,   (buttons & SCE_PAD_BUTTON_L3)		 ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
		curDeviceState->SetButtonValue( PAD_BUTTON_RTHUMB,   (buttons & SCE_PAD_BUTTON_R3)		 ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );

		// set digital buttons
		curDeviceState->SetButtonValue( PAD_BUTTON_UP,       (buttons & SCE_PAD_BUTTON_UP)    ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
		curDeviceState->SetButtonValue( PAD_BUTTON_RIGHT,    (buttons & SCE_PAD_BUTTON_RIGHT) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
		curDeviceState->SetButtonValue( PAD_BUTTON_DOWN,     (buttons & SCE_PAD_BUTTON_DOWN)  ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
		curDeviceState->SetButtonValue( PAD_BUTTON_LEFT,     (buttons & SCE_PAD_BUTTON_LEFT)  ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );

		curDeviceState->SetButtonValue( PAD_BUTTON_SQUARE,   (buttons & SCE_PAD_BUTTON_SQUARE)   ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
		curDeviceState->SetButtonValue( PAD_BUTTON_X,        (buttons & SCE_PAD_BUTTON_CROSS)    ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
		curDeviceState->SetButtonValue( PAD_BUTTON_CIRCLE,   (buttons & SCE_PAD_BUTTON_CIRCLE)   ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
		curDeviceState->SetButtonValue( PAD_BUTTON_TRIANGLE, (buttons & SCE_PAD_BUTTON_TRIANGLE) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );

		curDeviceState->SetButtonValue( PAD_BUTTON_L1,       (buttons & SCE_PAD_BUTTON_L1) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
		curDeviceState->SetButtonValue( PAD_BUTTON_R1,       (buttons & SCE_PAD_BUTTON_R1) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
		// curDeviceState->SetButtonValue( PAD_BUTTON_L2,      (buttons & SCE_PAD_BUTTON_L2) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
		// curDeviceState->SetButtonValue( PAD_BUTTON_R2,      (buttons & SCE_PAD_BUTTON_R2) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE );
		
		curDeviceState->SetButtonValue( PAD_BUTTON_L2,      mData[ctr].analogButtons.l2);
		curDeviceState->SetButtonValue( PAD_BUTTON_R2,      mData[ctr].analogButtons.r2);

		// set axis
		curDeviceState->SetAxisValue(PAD_AXIS_LEFTX,  NormalizeAxis((int32_t)mData[ctr].leftStick.x,  (int32_t)0, (int32_t)255, (int32_t) DeviceState::MIN_AXIS_VALUE, (int32_t) DeviceState::MAX_AXIS_VALUE));
		curDeviceState->SetAxisValue(PAD_AXIS_RIGHTX, NormalizeAxis((int32_t)mData[ctr].rightStick.x, (int32_t)0, (int32_t)255, (int32_t) DeviceState::MIN_AXIS_VALUE, (int32_t) DeviceState::MAX_AXIS_VALUE));

		// min and max ranges have been inverted for the Y-axis so that the return value from GetAxisValue()
		// is consistent for all platforms (that is pushing up will return a positive value).
		curDeviceState->SetAxisValue(PAD_AXIS_LEFTY,  NormalizeAxis((int32_t)mData[ctr].leftStick.y,  (int32_t)0, (int32_t)255, (int32_t) DeviceState::MAX_AXIS_VALUE, (int32_t) DeviceState::MIN_AXIS_VALUE));
		curDeviceState->SetAxisValue(PAD_AXIS_RIGHTY, NormalizeAxis((int32_t)mData[ctr].rightStick.y, (int32_t)0, (int32_t)255, (int32_t) DeviceState::MAX_AXIS_VALUE, (int32_t) DeviceState::MIN_AXIS_VALUE));
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
	EA::StdC::Memclear(mData, sizeof(mData));
	for (uint32_t ctr = 0; ctr < GetNumControllers(); ctr++)
	{
		// alloc space for device info & state
		mDeviceInfoList[ctr] = new DeviceInfo(ctr);
		mDeviceStateList[ctr] = new DeviceState(ctr, DeviceState::LOCAL);

		// alloc space for the effect
		mDeviceEffectList[ctr] = new DeviceEffect(ctr);

		mCtrlPtrs[ctr] = NULL;
	}
	
	SceUserServiceInitializeParams params;
	EA::StdC::Memclear(&params, sizeof(params));
	params.priority = SCE_KERNEL_PRIO_FIFO_DEFAULT;
	int32_t ret = sceUserServiceInitialize(&params);
	if (ret != SCE_OK && ret != SCE_USER_SERVICE_ERROR_ALREADY_INITIALIZED)
	{
		EA_ASSERT_FORMATTED(false, ("Unable to initialize user service [0x%08X]", ret));
		return;
	}
	ret = scePadInit();
	if (ret != SCE_OK)
	{
		EA_ASSERT_FORMATTED(false, ("Unable to initialize pad library [0x%08X]", ret));
	}

	EACUPS->AddMessageHandler(this, EA::Pairing::E_CONTROLLER_ADDED, false);
	EACUPS->AddMessageHandler(this, EA::Pairing::E_CONTROLLER_REMOVED, false);
}

LLManager::~LLManager()
{
    for (uint32_t ctr = 0; ctr < LLManager::GetNumControllers(); ctr++)
    {
        delete mDeviceEffectList[ctr];
        delete mDeviceStateList[ctr];
        delete mDeviceInfoList[ctr];
    }

	sceUserServiceTerminate();
}

void *LLManager::operator new(size_t size)
{
    return Manager::GetAllocator()->Alloc(size, "rw::core::controller::LLManager", EA::Allocator::MEM_PERM);
}

void LLManager::operator delete(void *p)
{
    Manager::GetAllocator()->Free(p);
}

const EA::Controller::IEAController* LLManager::GetController(int index)
{
	return mCtrlPtrs[index];
}
}}}
