// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved

/**
\file
Header file of rw::core::controller::DeviceInfo
*/

#ifndef RW_CORE_CONTROLLER_DEVICEINFO_H
#define RW_CORE_CONTROLLER_DEVICEINFO_H

#if defined _MSC_VER
#pragma once
#endif

#if !defined RWCORELIB_API
 #if 0 // defined EA_DLL && !defined RWCROSS
  #define RWCORELIB_API __declspec(dllimport)
 #else
  #define RWCORELIB_API
 #endif
#endif

#include "EABase/eabase.h"

namespace rw
{
    namespace core
    {
        namespace controller
        {
            /**
            \brief Class that holds info and methods to describe a controller device
            */
            class RWCORELIB_API DeviceInfo
            {
            public:
                /**
                \brief The different type of controllers possible
                */
                enum Type
                {
                    PAD,            ///< Standard controller
                    WHEEL,          ///< Driving wheel
                    MOUSE,          ///< Mouse
                    KEYBOARD,       ///< Keyboard
                    UNKNOWN         ///< Unknown/unsupported device
                };

                /**
                \brief Bitwise combinations of flags for the capabilities of the controller
                buttons. All buttons are assumed to be digital.
                */
                enum ButtonCapability
                {
                    ANALOG  = 0x1   ///< Analog ability
                };

                /**
                \brief Returns the ID for the device. The device ID will
                be from 0-n based on the slot the device is plugged into on the system.
                \return the device ID
                */
                unsigned int GetID() const { return mDeviceID; }

                /**
                \brief Returns the type of the device that is connected.
                \return the type of the device
                */
                Type         GetType() const { return mDeviceType; }

                /**
                \brief Returns the number of buttons on the device.
                \return the number of buttons on the device
                */
                unsigned int GetNumberButtons() const { return mNumberButtons; }

                /**
                \brief Returns the number of axes on the device.
                \return the number of axes
                */
                unsigned int GetNumberAxis() const { return mNumberAxis; }

                /**
                \brief Returns the capability flags for a button
                Returns the capability flags for a button on the device.
                \param ButtonIndex the index of the button
                \return the capabilities of the button
                \importlib rwcontroller
                */
                ButtonCapability GetButtonCapability(unsigned int ButtonIndex) const;

#if defined(EA_PLATFORM_WINDOWS)
				const char *GetInstanceName() const{ return tszInstanceName;};
				const char *GetProductName() const{ return tszProductName;};
#endif

            private:
                explicit DeviceInfo(unsigned int id);
                ~DeviceInfo();
                static void *operator new(size_t size);
                static void operator delete(void *p);

                // this class is not copyable
                DeviceInfo(const DeviceInfo &);
                DeviceInfo& operator=(const DeviceInfo &);

                void SetButtonCapability(unsigned int ButtonIndex, ButtonCapability newCapability);
                void Reset(Type deviceType, unsigned int numButtons, unsigned int numAxis);

                unsigned int        mDeviceID;
                Type                mDeviceType;
                unsigned int        mNumberButtons;
                unsigned int        mNumberAxis;
                ButtonCapability   *mButtonCapabilities;

#if defined(EA_PLATFORM_PS3)
                unsigned short      vendorID;
                unsigned short      productID;
#elif defined(EA_PLATFORM_WINDOWS)
				char tszInstanceName[260];
				char tszProductName[260];
#endif
                // grant friendship to the Manager class.  The Manager is the
                // only place this class is constructed and modified.
                friend class LLManager;
                friend class XInputManager;
            };
        } // namespace controller
    } // namespace core
} // namespace rw

#endif  // RW_CORE_CONTROLLER_DEVICEINFO_H
