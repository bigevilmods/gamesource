// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved

/**
\file
PC specific controller manager class definition
*/

#ifndef RW_CORE_CONTROLLER_LLMANAGER_H
#define RW_CORE_CONTROLLER_LLMANAGER_H

#if defined _MSC_VER
#pragma once
#endif

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#include "private_rwxinput.h"
#include "EAAssert/eaassert.h"

namespace EA
{
	namespace Thread
	{
		class Thread;
		class Semaphore;
	}
}

namespace rw
{
    namespace core
    {
        namespace controller
        {
			class AtomicAcquireQueue;

			// PC specific data structure needed for each device we are tracking
            struct LLDeviceData
            {
                LLDeviceData();
                ~LLDeviceData();

                LPDIRECTINPUTDEVICE8 mDiDevice;
                DIDEVCAPS            mDiDevCaps;

                static void *operator new(size_t size);
                static void operator delete(void *p);
            };

            // PC specific manager class to handle low level functionality.
            class LLManager
            {
            public:
                explicit LLManager(const Options &options);
                ~LLManager();

                static void *operator new(size_t size);
                static void operator delete(void *p);

                void Update();
                const DeviceInfo* GetDeviceInfo(unsigned int) const;
                const DeviceState* GetDeviceState(unsigned int) const;
                DeviceEffect* GetDeviceEffect(unsigned int) const;
                unsigned int GetNumControllers() const { return mCurDeviceId; }

            private:
                // returns a string for an error code
                const char *GetErrorString(HRESULT errorCode) const;

                void CreateDirectInputDevice(uint32_t deviceId, const DIDEVICEINSTANCE *pDidInstance,
                    const DIDATAFORMAT *pFormat);
                void Acquire(uint32_t deviceId) const;
                void Release(uint32_t deviceId) const;
                void InitPad(const DIDEVICEINSTANCE *pDidInstance);
                void InitKeyboard(const DIDEVICEINSTANCE *pDidInstance);
                void InitMouse(const DIDEVICEINSTANCE *pDidInstance);

                // callback when a device is found
                static BOOL CALLBACK MyEnumDeviceCallback(const DIDEVICEINSTANCE *pDidInstance, void *pContext);

				static intptr_t AcquireDeviceThread( void *lpContext );

                LLManager &operator=(const LLManager&);

                // current timestamp
                unsigned int mTimestamp;

                // the direct input interface
                LPDIRECTINPUT8  mDirectInput;

                bool            mUseXInput;
				bool			mUseForeground;
				HWND			mWnd;

                XInputManager   mXiManager;

                // id number for generating ids for Direct Input devices
                uint32_t mCurDeviceId;

                // List of devices and states
                DeviceInfo    *mDeviceInfoList[Manager::MAX_DEVICES];
                DeviceState   *mDeviceStateList[Manager::MAX_DEVICES];

                // list of pointers to data for managing the connected devices
                LLDeviceData   *mDeviceDataList[Manager::MAX_DEVICES];
				uint32_t	   mDeviceQueuedList[Manager::MAX_DEVICES];

				volatile uint32_t mTerminateAsyncThread;
				EA::Thread::Semaphore	*mAcquireSemaphore;
				AtomicAcquireQueue		*mAcquireQueue;
				EA::Thread::Thread		*mAcquireDeviceThread;
            };
        } // namespace controller
    } // namespace core
} // namespace rw

#endif
