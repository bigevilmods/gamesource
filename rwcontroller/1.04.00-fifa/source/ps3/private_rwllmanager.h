// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved

/**
\file
PS3 specific controller manager class definition 
*/

#ifndef RW_CORE_CONTROLLER_LLMANAGER_H
#define RW_CORE_CONTROLLER_LLMANAGER_H

#include <sdk_version.h>

#include <cell/pad/libpad.h>
#include <cell/pad/pad_codes.h>

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
                DeviceEffect* GetDeviceEffect(unsigned int) const;
                unsigned int GetNumControllers() const { return Manager::MAX_DEVICES; }
                uint32_t TMPGetButtonData() const { return buttons; }
                bool GetInterceptState() const { return (PadInfo.system_info & CELL_PAD_INFO_INTERCEPTED) == CELL_PAD_INFO_INTERCEPTED; }
				uint32_t TMPGetCapabilityData(unsigned int deviceNumber) const { return (PadInfo.device_capability[deviceNumber]); }
				uint32_t TMPGetClassTypeData(unsigned int deviceNumber) const { return (PadInfo.pclass_type[deviceNumber]); }
				bool TMPGetPadPeriphInfo(CellPadPeriphInfo& cellPadPeriphInfo) const {cellPadPeriphInfo = PadInfo; return true;}

            private:
                // current timestamp
                unsigned int mTimestamp;

                // List of devices and states
                DeviceInfo    *mDeviceInfoList[Manager::MAX_DEVICES];
                DeviceState   *mDeviceStateList[Manager::MAX_DEVICES];

                // list of pointers to the device effect objects
                DeviceEffect *mDeviceEffectList[Manager::MAX_DEVICES];

                uint32_t buttons;

                CellPadPeriphInfo PadInfo;  // Gamepad information buffer                
            };

        } // namespace controller
    } // namespace core
} // namespace rw

#endif
