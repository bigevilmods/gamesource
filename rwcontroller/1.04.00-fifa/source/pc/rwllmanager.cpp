// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved

/*      controller PC manager class implementation */
#include "rw/core/controller/deviceinfo.h"
#include "rw/core/controller/devicestate.h"
#include "rw/core/controller/manager.h"
#include "private_rwllmanager.h"
#include "private_rwxinput.h"

#include "EAStdC/EAMemory.h"
#include "EAStdC/EASprintf.h"

#include "eathread/eathread_thread.h"
#include "eathread/eathread_futex.h"
#include "eathread/eathread_semaphore.h"

#include "coreallocator/icoreallocatormacros.h"

#pragma warning(disable: 4291)

namespace rw
{
namespace core
{
namespace controller
{

#ifndef RW_CORE_CONTROLLER_NORMALIZE_AXIS_DEFINED
#define RW_CORE_CONTROLLER_NORMALIZE_AXIS_DEFINED 1

struct AcquireEntry
{
	const DeviceInfo   *mDeviceInfo;
	const LLDeviceData *mDeviceData;
	DeviceState        *mDeviceState;
	uint32_t		   *mDeviceQueued;
};

class AtomicAcquireQueue
{
public:
	AtomicAcquireQueue() : mListHead(0), mListTail(0)
	{
		memset(mAcquireEntryList, 0, sizeof(mAcquireEntryList[0]) * MAX_LIST_ENTRIES);
	}

	bool Push(const DeviceInfo *deviceInfo, const LLDeviceData *deviceData, DeviceState *deviceState, uint32_t *deviceQueued)
	{
		EA::Thread::AutoFutex autoLock(mFutex);

		EA_ASSERT(mListHead <= mListTail);
		EA_ASSERT((int)mListHead > (int)mListTail - MAX_LIST_ENTRIES);

		if(*deviceQueued == 0)
		{
			uint32_t index = mListTail++ %  MAX_LIST_ENTRIES;
			AcquireEntry *entry  = &mAcquireEntryList[index];

			// make sure we haven't looped on ourselves
			EA_ASSERT(entry->mDeviceInfo   == NULL	&& 
					  entry->mDeviceData   == NULL	&&
					  entry->mDeviceState  == NULL	&&
					  entry->mDeviceQueued == NULL);

			entry->mDeviceInfo		= deviceInfo;
			entry->mDeviceData		= deviceData;
			entry->mDeviceState		= deviceState;
			entry->mDeviceQueued	= deviceQueued;

			*deviceQueued = 1;
			return true;
		}

		return false;
	}

	AcquireEntry *Pop()
	{
		EA::Thread::AutoFutex autoLock(mFutex);

		EA_ASSERT(mListHead < mListTail);
		
		uint32_t index = mListHead++ %  MAX_LIST_ENTRIES;
		AcquireEntry *entry = &mAcquireEntryList[index];
		return entry;
	}

	void MarkDeviceNotQueued(AcquireEntry *entry)
	{
		EA::Thread::AutoFutex autoLock(mFutex);

		uint32_t *deviceQueued = entry->mDeviceQueued;
		*deviceQueued = 0;

		entry->mDeviceInfo		= NULL;
		entry->mDeviceData		= NULL;
		entry->mDeviceState		= NULL;
		entry->mDeviceQueued	= NULL;
	}

private:
	enum { MAX_LIST_ENTRIES = 64 };

	EA::Thread::Futex mFutex;
	uint32_t mListHead;
	uint32_t mListTail;
	AcquireEntry mAcquireEntryList[MAX_LIST_ENTRIES];
};

intptr_t LLManager::AcquireDeviceThread( void *lpContext )
{
	for(;;)
	{
		LLManager *llManager = reinterpret_cast<LLManager*>(lpContext);
		llManager->mAcquireSemaphore->Wait();
		
		if(llManager->mTerminateAsyncThread)
		{
			break;
		}

		AcquireEntry *entry = llManager->mAcquireQueue->Pop();

        const DeviceInfo   *curDeviceInfo  = entry->mDeviceInfo;
        const LLDeviceData *curDeviceData  = entry->mDeviceData;
        DeviceState        *curDeviceState = entry->mDeviceState;


		// DInput is telling us that the input stream has been
		// interrupted. We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done. We
		// just re-acquire and try again.

		HRESULT hr = curDeviceData->mDiDevice->Acquire();

		// Disconnect the device if it's unplugged
		if (hr == EVENT_E_TOO_MANY_METHODS)
		{
			curDeviceState->Reset(DeviceState::DISCONNECTED, 
				curDeviceState->GetNumberDigitalButtons(), curDeviceState->GetNumberAnalogButtons(), curDeviceState->GetNumberAxis());
		}

		while(hr == DIERR_INPUTLOST)
		{
			hr = curDeviceData->mDiDevice->Acquire();

			// give other threads a chance
			//THREAD_yield(0);
		}

		llManager->mAcquireQueue->MarkDeviceNotQueued(entry);
	}

	return 0;
}


// Macro to normalize the axis values to a range
inline int32_t NormalizeAxis(int32_t value, int32_t minValue, int32_t maxValue, int32_t minRange, int32_t maxRange)
{
    return ((((maxRange - minRange) * (value - minValue)) / (maxValue - minValue)) - ((maxRange - minRange) / 2));
}

#endif

void LLManager::Update()
{
    mTimestamp++;

    mXiManager.Update(mTimestamp);

    // for each connected controller, update the state
    for (int ctr = mXiManager.GetNumXInputControllers();
        ctr < static_cast<int>(GetNumControllers()); ctr++)
    {
        const DeviceInfo   *curDeviceInfo  = mDeviceInfoList[ctr];
        const LLDeviceData *curDeviceData  = mDeviceDataList[ctr];
        DeviceState        *curDeviceState = mDeviceStateList[ctr];

		// If the device is not connected
		// Try to acquire it again
		if (!curDeviceState->IsConnected())
		{
			HRESULT hr;

			// Poll the device to read the current state
			hr = curDeviceData->mDiDevice->Poll();
			if (hr < 0)
			{
				// re-acquire and see if the controller is reconnected again.
				hr = curDeviceData->mDiDevice->Acquire();
				if (hr==S_OK)
				{
					curDeviceState->Reset(DeviceState::CONNECTED, 
						curDeviceState->GetNumberDigitalButtons(), curDeviceState->GetNumberAnalogButtons(), curDeviceState->GetNumberAxis());
				}
			}
		}

        // if a device is connected, update it
        if (curDeviceState->IsConnected())
        {
            HRESULT hr;

            // Poll the device to read the current state
            hr = curDeviceData->mDiDevice->Poll();
            if (hr < 0)
            {
				uint32_t *curDeviceQueued = &mDeviceQueuedList[ctr];
				bool success = mAcquireQueue->Push(curDeviceInfo, curDeviceData, curDeviceState, curDeviceQueued);
				// don't post to the semaphore if the device has already been queued.  We are likely stalled in this case trying to reaquire the device.
				if(success)
				{
					mAcquireSemaphore->Post();
				}

                return;
            }

            curDeviceState->SetTimestamp(mTimestamp);

            switch (curDeviceInfo->GetType())
            {
                case DeviceInfo::PAD:
                {
                    // Get the input's device state
                    DIJOYSTATE2 js;           // DInput joystick state

                    hr = curDeviceData->mDiDevice->GetDeviceState(sizeof(DIJOYSTATE2), &js);

                    if (hr != DI_OK)
                    {
                        return;
                    }

                    // got new states. Now copy it to the state

                    // check the directional axes, normalize to -1000/1000 as well
                    curDeviceState->SetAxisValue(PAD_AXIS_LEFTX, NormalizeAxis(js.lX, 0, 65535, DeviceState::MIN_AXIS_VALUE, DeviceState::MAX_AXIS_VALUE));
                    // min and max ranges have been inverted for the Y-axis so that the return value from GetAxisValue()
                    // is consistent for all platforms (that is pushing up will return a positive value).
                    curDeviceState->SetAxisValue(PAD_AXIS_LEFTY, NormalizeAxis(js.lY, 0, 65535, DeviceState::MAX_AXIS_VALUE, DeviceState::MIN_AXIS_VALUE));
                    curDeviceState->SetAxisValue(PAD_AXIS_LEFTZ, NormalizeAxis(js.lZ, 0, 65535, DeviceState::MIN_AXIS_VALUE, DeviceState::MAX_AXIS_VALUE));
                    
                    curDeviceState->SetAxisValue(PAD_AXIS_RIGHTX, NormalizeAxis(js.lRx, 0, 65535, DeviceState::MIN_AXIS_VALUE, DeviceState::MAX_AXIS_VALUE));
                    // min and max ranges have been inverted for the Y-axis so that the return value from GetAxisValue()
                    // is consistent for all platforms (that is pushing up will return a positive value).
                    curDeviceState->SetAxisValue(PAD_AXIS_RIGHTY, NormalizeAxis(js.lRy, 0, 65535, DeviceState::MAX_AXIS_VALUE, DeviceState::MIN_AXIS_VALUE));
                    curDeviceState->SetAxisValue(PAD_AXIS_RIGHTZ, NormalizeAxis(js.lRz, 0, 65535, DeviceState::MIN_AXIS_VALUE, DeviceState::MAX_AXIS_VALUE));

                    // $$TODO$$ BAH 04/15/04 - make sure these are correct
                    curDeviceState->SetAxisValue(PAD_AXIS_SLIDER0, js.rglSlider[0]);
                    curDeviceState->SetAxisValue(PAD_AXIS_SLIDER1, js.rglSlider[1]);

                    // $$TODO$$ BAH 04/15/04 - these might be supposed to be digital value as well?
                    curDeviceState->SetAxisValue(PAD_AXIS_POV0, static_cast<int32_t>(js.rgdwPOV[0]));
                    curDeviceState->SetAxisValue(PAD_AXIS_POV1, static_cast<int32_t>(js.rgdwPOV[1]));
                    curDeviceState->SetAxisValue(PAD_AXIS_POV2, static_cast<int32_t>(js.rgdwPOV[2]));
                    curDeviceState->SetAxisValue(PAD_AXIS_POV3, static_cast<int32_t>(js.rgdwPOV[3]));

                    // check the buttons
                    for (uint32_t buttonCtr = 0; buttonCtr < curDeviceInfo->GetNumberButtons(); buttonCtr++)
                    {
                        curDeviceState->SetButtonValue(buttonCtr, js.rgbButtons[buttonCtr]);
                    }
                    break;
                }

                case DeviceInfo::KEYBOARD:
                {
                    uint8_t diKeys[256];   // DirectInput keyboard state buffer
                    EA::StdC::Memclear(diKeys, sizeof(diKeys));

                    hr = curDeviceData->mDiDevice->GetDeviceState(sizeof(diKeys), diKeys);

                    if (hr != DI_OK)
                    {
                        return;
                    }

                    // check the buttons
                    for (uint32_t buttonCtr = 0; buttonCtr < curDeviceInfo->GetNumberButtons(); buttonCtr++)
                    {
                        curDeviceState->SetButtonValue(buttonCtr, static_cast<uint8_t>(diKeys[buttonCtr] & 0x80));
                    }

                    break;
                }

                case DeviceInfo::MOUSE:
                {
                    DIMOUSESTATE2 diMouseData;
                    EA::StdC::Memclear(&diMouseData, sizeof(diMouseData));

                    hr = curDeviceData->mDiDevice->GetDeviceState(sizeof(DIMOUSESTATE2), &diMouseData);

                    if(hr == DI_OK)
                    {
                        // update positions
                        curDeviceState->SetAxisValue(MOUSE_AXIS_X, diMouseData.lX);
                        curDeviceState->SetAxisValue(MOUSE_AXIS_Y, diMouseData.lY);
                        curDeviceState->SetAxisValue(MOUSE_AXIS_Z, diMouseData.lZ);

                        // update the buttons
                        for (uint32_t buttonCtr = 0; buttonCtr < curDeviceInfo->GetNumberButtons(); buttonCtr++)
                        {
                            curDeviceState->SetButtonValue(buttonCtr, diMouseData.rgbButtons[buttonCtr]);
                        }
                    }
                    else
                    {
                        // DirectInput may be telling us that the input stream has been
                        // interrupted.  We aren't tracking any state between polls, so
                        // we don't have any special reset that needs to be done.
                        // We just re-acquire and try again.

                        curDeviceData->mDiDevice->Acquire();
                    }
                    break;
                }

                case DeviceInfo::UNKNOWN:
                default:
                {
                    EA_FAIL_MSG("unknown device type");
                    break;
                }
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

    unsigned int ctr = static_cast<unsigned int>(
        mXiManager.GetNumXInputControllers());

    if (mDeviceInfoList[deviceNumber]->GetType() == DeviceInfo::PAD && deviceNumber < ctr)
    {
        // TODO: assume only xinput pads support effect for now
        return mXiManager.GetXInputDeviceEffect(deviceNumber);
    }

    // other devices do not support effects
    return NULL;
}

LLManager::LLManager(const Options &options)
: mTimestamp(0),
  mDirectInput(NULL),
  mUseXInput(options.mUseXInput),
  mUseForeground(options.mUseForeground),
  mWnd(((HWND)options.mWnd)),
  mXiManager(mUseXInput, mDeviceInfoList, mDeviceStateList),
  mCurDeviceId(static_cast<uint32_t>(mXiManager.GetNumXInputControllers())),
  mTerminateAsyncThread(0)
{
    EA::StdC::Memclear(mDeviceInfoList+mXiManager.GetNumXInputControllers(),
        sizeof(mDeviceInfoList)-(mXiManager.GetNumXInputControllers()*sizeof(*mDeviceInfoList)));
    EA::StdC::Memclear(mDeviceStateList+mXiManager.GetNumXInputControllers(),
        sizeof(mDeviceStateList)-(mXiManager.GetNumXInputControllers()*sizeof(*mDeviceStateList)));

    // Create the DirectInput object.
    HRESULT result = DirectInput8Create(GetModuleHandle(NULL),
                                        DIRECTINPUT_VERSION,
                                        IID_IDirectInput8,
                                        (LPVOID *)&mDirectInput,
                                        NULL);
    EA_ASSERT_MSG(result == DI_OK, "Error in DirectInputCreate");

    // enumerate controllers
    result = mDirectInput->EnumDevices(DI8DEVCLASS_ALL,
                                       MyEnumDeviceCallback,
                                       this,
                                       DIEDFL_ATTACHEDONLY);
    EA_ASSERT_MSG(result == DI_OK, "Error in EnumDevices");

	mAcquireSemaphore = CORE_NEW(Manager::GetAllocator(), "LLManager::mAcquireSemaphore", EA::Allocator::MEM_PERM) EA::Thread::Semaphore();
	mAcquireQueue = CORE_NEW(Manager::GetAllocator(), "LLManager::mAcquireQueue", EA::Allocator::MEM_PERM) AtomicAcquireQueue();
	mAcquireDeviceThread = CORE_NEW(Manager::GetAllocator(), "LLManager::mAcquireDeviceThread", EA::Allocator::MEM_PERM) EA::Thread::Thread();

	memset(mDeviceQueuedList, 0, Manager::MAX_DEVICES * sizeof(mDeviceQueuedList[0]));

	EA::Thread::ThreadParameters threadParams;
	threadParams.mpName		= "rwController DeviceAquire";
	threadParams.mnPriority = 4; // copy the input thread's priority

	mAcquireDeviceThread->Begin(AcquireDeviceThread, this, &threadParams);
}

// Destructor for the LLManager class
LLManager::~LLManager()
{
	mTerminateAsyncThread = 1;
	mAcquireSemaphore->Post();
	mAcquireDeviceThread->WaitForEnd();
	EA::Thread::Thread::Status status = EA::Thread::Thread::kStatusNone;
	if(mAcquireDeviceThread->GetStatus() == EA::Thread::Thread::kStatusRunning)
	{
		status = mAcquireDeviceThread->WaitForEnd();
	}
	// make sure the thread was properly shutdown
	EA_ASSERT(status != EA::Thread::Thread::kStatusRunning);

    // release all the devices
    for (uint32_t ctr = static_cast<uint32_t>(mXiManager.GetNumXInputControllers());
        ctr < LLManager::GetNumControllers(); ctr++)
    {
        Release(ctr);
        delete mDeviceDataList[ctr];
        mDeviceDataList[ctr] = NULL;
    }

    if (mDirectInput != NULL)
    {
        mDirectInput->Release();
    }

    for (uint32_t ctr = static_cast<uint32_t>(mXiManager.GetNumXInputControllers()); ctr < LLManager::GetNumControllers(); ctr++)
    {
        delete mDeviceInfoList[ctr];
        delete mDeviceStateList[ctr];
    }

	CORE_DELETE(Manager::GetAllocator(), mAcquireSemaphore);
	CORE_DELETE(Manager::GetAllocator(), mAcquireQueue);
	CORE_DELETE(Manager::GetAllocator(), mAcquireDeviceThread);
}

const char *LLManager::GetErrorString(HRESULT errorCode) const
{
    static char errorStringBuf[256];

    errorStringBuf[0] = '\0';

    switch(errorCode)
    {
    case DIERR_INPUTLOST:
        strcpy(errorStringBuf, "DIERR_INPUTLOST");
        break;

    case DIERR_INVALIDPARAM:
        strcpy(errorStringBuf, "DIERR_INVALIDPARAM");
        break;

    case DIERR_NOTINITIALIZED:
        strcpy(errorStringBuf, "DIERR_NOTINITIALIZED");
        break;

    case DIERR_UNSUPPORTED:
        strcpy(errorStringBuf, "DIERR_UNSUPPORTED");
        break;

    case DIERR_ACQUIRED:
        strcpy(errorStringBuf, "DIERR_ACQUIRED");
        break;

    case DIERR_NOTACQUIRED:
        strcpy(errorStringBuf, "DIERR_NOTACQUIRED");
        break;

    case DI_EFFECTRESTARTED:
       strcpy(errorStringBuf, "effect restarted");
       break;

    case DI_DOWNLOADSKIPPED:
       strcpy(errorStringBuf, "effect not loaded");
       break;

    case DI_TRUNCATED:
       strcpy(errorStringBuf, "effect truncated");
       break;

    case DI_TRUNCATEDANDRESTARTED:
       strcpy(errorStringBuf, "effect truncated and restarted");
       break;

    case S_FALSE:
       strcpy(errorStringBuf, "S_FALSE");
       break;

    case DIERR_OLDDIRECTINPUTVERSION:
        strcpy(errorStringBuf, "DInput Version Mismatch");
        break;

    case DIERR_DEVICENOTREG:
        // The device or device instance is not registered with Microsoft® DirectInput®.
        // This value is equal to the REGDB_E_CLASSNOTREG standard COM return value.
        strcpy(errorStringBuf, "The device is not registered");
        break;

    default:
        EA::StdC::Sprintf(errorStringBuf, "UNKNOWN %#x", errorCode);
        break;
    }

    return errorStringBuf;
}

// deviceId ID of the controller to setup
// pDidInstance The direct input device instance from the enumeration
// pFormat The format to use for the device
void LLManager::CreateDirectInputDevice(uint32_t deviceId, const DIDEVICEINSTANCE *pDidInstance, const DIDATAFORMAT *pFormat)
{
	LLDeviceData *pDeviceData = mDeviceDataList[deviceId];
	DeviceInfo    *pDeviceInfo = mDeviceInfoList[deviceId];
    HRESULT hr;

    // Create a DIRECTINPUTDEVICE based on pDidInstance
    hr = mDirectInput->CreateDevice(pDidInstance->guidInstance, &pDeviceData->mDiDevice, NULL);
    if (hr != DI_OK)
    {
        EA_FAIL_FORMATTED(("Error in CreateDirectInputDevice: %s", GetErrorString(hr)));
        return;
    }

	if( mUseForeground && mWnd != 0)
	{
		hr = pDeviceData->mDiDevice->SetCooperativeLevel(mWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		if ( hr != DI_OK )
		{
			EA_FAIL_FORMATTED(("Error in CreateDirectInputDevice setting CooperativeLevel: %s", GetErrorString(hr)));
			return;
		}
	}

    // Get device info
    DIDEVICEINSTANCE diInfo;
    EA::StdC::Memclear(&diInfo, sizeof(diInfo));
    diInfo.dwSize = sizeof(DIDEVICEINSTANCE);
    hr = pDeviceData->mDiDevice->GetDeviceInfo(&diInfo);

    if (hr == DI_OK)
    {
		memcpy(pDeviceInfo->tszInstanceName, diInfo.tszInstanceName, sizeof(pDeviceInfo->tszInstanceName)-1);
		memcpy(pDeviceInfo->tszProductName, diInfo.tszProductName, sizeof(pDeviceInfo->tszProductName)-1);
    }

    // Set data format:
    // The data format must be set before the device can be acquired by
    // using the IDirectInputDevice8::Acquire method. It is necessary to
    // set the data format only once. The data format cannot be changed
    // while the device is acquired.
    hr = pDeviceData->mDiDevice->SetDataFormat(pFormat);
    if (hr != DI_OK)
    {
        EA_FAIL_FORMATTED(("Error in CreateDirectInputDevice: %s", GetErrorString(hr)));
        return;
    }
}

void LLManager::Acquire(uint32_t deviceId) const
{
    const LLDeviceData *pDeviceData = mDeviceDataList[deviceId];
    HRESULT hr = pDeviceData->mDiDevice->Acquire();

    if (hr == DI_OK)
    {
        return;
    }

    // If input is lost then acquire and keep trying
    while (hr == DIERR_INPUTLOST)
    {
        hr = pDeviceData->mDiDevice->Acquire();
    }
}

void LLManager::Release(uint32_t deviceId) const
{
    LLDeviceData *pDeviceData = mDeviceDataList[deviceId];

    // Unacquire the device one last time just in case
    // the app tried to exit while the device is still acquired.
    if(pDeviceData->mDiDevice != NULL)
    {
        pDeviceData->mDiDevice->Unacquire();
        pDeviceData->mDiDevice->Release();
        pDeviceData->mDiDevice = NULL;
    }
}

void LLManager::InitPad(const DIDEVICEINSTANCE *pDidInstance)
{
    uint32_t deviceId = mCurDeviceId;
    mCurDeviceId++;

    // alloc the data and store it
    DeviceInfo   *pDeviceInfo  = new DeviceInfo(deviceId);
    mDeviceInfoList[deviceId] = pDeviceInfo;

    DeviceState  *pDeviceState = new DeviceState(deviceId, DeviceState::LOCAL);
    mDeviceStateList[deviceId] = pDeviceState;

    LLDeviceData *pDeviceData  = new LLDeviceData();
    mDeviceDataList[deviceId] = pDeviceData;

    // create a DirectInputDevice, set data format and co-operative level
    CreateDirectInputDevice(deviceId, pDidInstance, &c_dfDIJoystick2);

    // Determine capabilities,
    // e.g. how many axis the joystick has (so we don't error out setting
    // properties for unavailable axis)
    pDeviceData->mDiDevCaps.dwSize = sizeof(DIDEVCAPS);
    HRESULT hr = pDeviceData->mDiDevice->GetCapabilities(&pDeviceData->mDiDevCaps);
    if (hr < 0)
    {
        // failed!
        EA_FAIL_FORMATTED(("Error in InitPad GetCapabilities: %s", GetErrorString(hr)));
    }
    else
    {
        // reset the device info to the new controller
        pDeviceInfo->Reset(DeviceInfo::PAD, pDeviceData->mDiDevCaps.dwButtons, 12);

        // reset the device state to the new controller
        if ((pDeviceData->mDiDevCaps.dwFlags & DIDC_ATTACHED) == DIDC_ATTACHED)
        {
            pDeviceState->Reset(DeviceState::CONNECTED, pDeviceData->mDiDevCaps.dwButtons, 0, 12);
        }
        else
        {
            pDeviceState->Reset(DeviceState::DISCONNECTED, pDeviceData->mDiDevCaps.dwButtons, 0, 12);
        }
    }

    pDeviceState->SetTimestamp(mTimestamp);

    // Turn off "autocenter":
    // You might want to turn off the device's autocenter feature.
    // Autocenter is essentially a condition effect that uses the motors to
    // simulate the springs in a standard joystick. It is a good idea to turn
    // it off so that it does not interfere with other effects.
    DIPROPDWORD DIPropAutoCenter;

    DIPropAutoCenter.diph.dwSize       = sizeof(DIPropAutoCenter);
    DIPropAutoCenter.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    DIPropAutoCenter.diph.dwObj        = 0;
    DIPropAutoCenter.diph.dwHow        = DIPH_DEVICE;
    DIPropAutoCenter.dwData            = DIPROPAUTOCENTER_OFF;

    hr = pDeviceData->mDiDevice->SetProperty(DIPROP_AUTOCENTER, &DIPropAutoCenter.diph);

    if (hr < 0 && hr != E_NOTIMPL)
    {
        // Handle the failure as appropriate
        EA_FAIL_FORMATTED(("Error in InitPad SetProperty: %s", GetErrorString(hr)));
    }

    // acquire the device
    Acquire(deviceId);
}

void LLManager::InitKeyboard(const DIDEVICEINSTANCE *pDidInstance)
{
    uint32_t deviceId = mCurDeviceId;
    mCurDeviceId++;

    // alloc the data and store it
    DeviceInfo   *pDeviceInfo  = new DeviceInfo(deviceId);
    mDeviceInfoList[deviceId] = pDeviceInfo;

    DeviceState  *pDeviceState = new DeviceState(deviceId, DeviceState::LOCAL);
    mDeviceStateList[deviceId] = pDeviceState;

    LLDeviceData *pDeviceData  = new LLDeviceData();
    mDeviceDataList[deviceId] = pDeviceData;

    // create a DirectInputDevice, set data format and co-operative level
    CreateDirectInputDevice(deviceId, pDidInstance, &c_dfDIKeyboard);

    // reset the device info to the new controller
    pDeviceInfo->Reset(DeviceInfo::KEYBOARD, 256, 0);

    // reset the device state to the new controller
    pDeviceState->Reset(DeviceState::CONNECTED, 256, 0, 0);

    pDeviceState->SetTimestamp(mTimestamp);

    // acquire the device
    Acquire(deviceId);
}

// pDidInstance The DirectInput device instance from the enumeration
void LLManager::InitMouse(const DIDEVICEINSTANCE *pDidInstance)
{
    uint32_t deviceId = mCurDeviceId;
    mCurDeviceId++;

    // alloc the data and store it
    DeviceInfo   *pDeviceInfo  = new DeviceInfo(deviceId);
    mDeviceInfoList[deviceId] = pDeviceInfo;

    DeviceState  *pDeviceState = new DeviceState(deviceId, DeviceState::LOCAL);
    mDeviceStateList[deviceId] = pDeviceState;

    LLDeviceData *pDeviceData  = new LLDeviceData();
    mDeviceDataList[deviceId] = pDeviceData;

    // create a DirectInputDevice, set data format and co-operative level
    CreateDirectInputDevice(deviceId, pDidInstance, &c_dfDIMouse2);

    // reset the device info to the new controller
    pDeviceInfo->Reset(DeviceInfo::MOUSE, 8, 3);

    // reset the device state to the new controller
    pDeviceState->Reset(DeviceState::CONNECTED, 8, 0, 3);

    pDeviceState->SetTimestamp(mTimestamp);

    // acquire the device
    Acquire(deviceId);
}

// Callback function that will be called for every DirectInput device connected to the PC
// pDidInstance The device instance
// pContext Application specific data passed from the caller (for us a pointer to LLManager)
// return whether to continue the enumeration or not
BOOL CALLBACK LLManager::MyEnumDeviceCallback(const DIDEVICEINSTANCE *pDidInstance, void *pContext)
{
    LLManager *pLLManager = static_cast<LLManager *>(pContext);

    // if this is an XInputDevice, don't register it with this manager.
    if (pLLManager->mUseXInput && XInputManager::IsXInputDevice(
        &pDidInstance->guidProduct ) )
    {
        return DIENUM_CONTINUE;
    }

	// create a device based on pDidInstance
    switch (LOBYTE(LOWORD(pDidInstance->dwDevType)))
    {
    case DI8DEVTYPE_DEVICE:
        break;

    case DI8DEVTYPE_MOUSE:
        pLLManager->InitMouse(pDidInstance);
        break;

    case DI8DEVTYPE_KEYBOARD:
        pLLManager->InitKeyboard(pDidInstance);
        break;

    case DI8DEVTYPE_JOYSTICK:
    case DI8DEVTYPE_GAMEPAD:
    case DI8DEVTYPE_DRIVING:
    case DI8DEVTYPE_FLIGHT:
    case DI8DEVTYPE_1STPERSON:
        pLLManager->InitPad(pDidInstance);
        break;

    case DI8DEVTYPE_DEVICECTRL:
    case DI8DEVTYPE_SCREENPOINTER:
    case DI8DEVTYPE_REMOTE:
    case DI8DEVTYPE_SUPPLEMENTAL:
        // unsupported device !
        break;

    default:
        // unknown device !
        break;
    }

    // next device
    return DIENUM_CONTINUE;
}

void *LLManager::operator new(size_t size)
{
    return Manager::GetAllocator()->Alloc(size, "rw::core::controller::LLManager",
        EA::Allocator::MEM_PERM);
}

void LLManager::operator delete(void *p)
{
    Manager::GetAllocator()->Free(p);
}

void *LLDeviceData::operator new(size_t size)
{
    return Manager::GetAllocator()->Alloc(size, "rw::core::controller::LLDeviceData",
        EA::Allocator::MEM_PERM);
}

void LLDeviceData::operator delete(void *p)
{
    Manager::GetAllocator()->Free(p);
}

LLDeviceData::LLDeviceData()
: mDiDevice(NULL)
{
    EA::StdC::Memclear(&mDiDevCaps, sizeof(mDiDevCaps));
}

LLDeviceData::~LLDeviceData()
{
    mDiDevice = NULL;
}

}
}
}
