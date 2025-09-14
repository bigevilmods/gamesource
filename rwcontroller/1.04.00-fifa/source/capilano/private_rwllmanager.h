// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved

/**
\file Xenon specific controller manager class definition 
*/

#ifndef RW_CORE_CONTROLLER_LLMANAGER_H
#define RW_CORE_CONTROLLER_LLMANAGER_H

#if defined _MSC_VER
#pragma once
#endif

#include "eathread/eathread_futex.h"

namespace rw
{
    namespace core
    {
        namespace controller
        {
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
                DeviceEffect* GetDeviceEffect(unsigned int deviceNumber) const;
                unsigned int GetNumControllers() const { return Manager::MAX_DEVICES; }
				void SetNullPads(bool enable);

				static uint64 GetGamePadID(int index);
				void SimulateButtonPush(unsigned int deviceNumber, const int* buttonIds, unsigned int numButtons, unsigned int numFrames);

            private:
				static void GamepadAdded	(_In_ Platform::Object ^sender, _In_ Windows::Xbox::Input::GamepadAddedEventArgs ^args);
				static void GamepadRemoved	(_In_ Platform::Object ^sender, _In_ Windows::Xbox::Input::GamepadRemovedEventArgs ^args);
                static void Resuming        (_In_ Platform::Object^ sender, _In_ Platform::Object^ args);

                // current timestamp
                unsigned int mTimestamp;

                // List of devices and states
                DeviceInfo    *mDeviceInfoList[Manager::MAX_DEVICES];
                DeviceState   *mDeviceStateList[Manager::MAX_DEVICES];

                // list of pointers to the device effect objects
                DeviceEffect *mDeviceEffectList[Manager::MAX_DEVICES];

				static uint64				sGamepadIDs[Manager::MAX_DEVICES];
				static EA::Thread::Futex	sMutex;
				static bool					sFakeNullPads;
            };
        } // namespace controller
    } // namespace core
} // namespace rw

#endif
