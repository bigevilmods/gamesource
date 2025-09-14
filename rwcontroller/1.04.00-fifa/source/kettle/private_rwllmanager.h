// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved

/**
\file Xenon specific controller manager class definition 
*/

#ifndef RW_CORE_CONTROLLER_LLMANAGER_H
#define RW_CORE_CONTROLLER_LLMANAGER_H

#include <pad.h>

#include "eathread/eathread_futex.h"
#include "EAMessage/EAMessage.h"
#include "IEAController/IEAController.h"

namespace rw
{
    namespace core
    {
        namespace controller
        {
            class LLManager : public EA::Messaging::IHandler
            {
            public:
                explicit LLManager(const Options &options);
                ~LLManager();

                static void *operator new(size_t size);
                static void operator delete(void *p);

                void Update();
				virtual bool HandleMessage(EA::Messaging::MessageId messageId, void* pMessage);
                const DeviceInfo* GetDeviceInfo(unsigned int) const;
				const EA::Controller::IEAController* GetController(int index);
                const DeviceState* GetDeviceState(unsigned int) const;
                DeviceEffect* GetDeviceEffect(unsigned int deviceNumber) const;
                unsigned int GetNumControllers() const { return Manager::MAX_DEVICES; }

            private:
                // current timestamp
                unsigned int mTimestamp;

                // List of devices and states
                DeviceInfo    *mDeviceInfoList[Manager::MAX_DEVICES];
                DeviceState   *mDeviceStateList[Manager::MAX_DEVICES];

                // list of pointers to the device effect objects
                DeviceEffect *mDeviceEffectList[Manager::MAX_DEVICES];

				ScePadData mData[Manager::MAX_DEVICES];
				EA::Controller::IEAController* mCtrlPtrs[Manager::MAX_DEVICES];

				EA::Thread::Futex			mMutex;
            };
        } // namespace controller
    } // namespace core
} // namespace rw

#endif
