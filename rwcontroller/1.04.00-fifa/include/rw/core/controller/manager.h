// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved

/**
\file
Header file of rw::core::controller::Manager
*/

#ifndef RW_CORE_CONTROLLER_MANAGER_H
#define RW_CORE_CONTROLLER_MANAGER_H

#include "EABase/eabase.h"
#include "coreallocator/icoreallocator_interface.h"
#include "IEAController/IEAController.h"
#if defined(EA_PLATFORM_PS4)
#include <user_service.h>
#endif

#if defined _MSC_VER
    #pragma once
#endif

#ifndef RWCORELIB_API
    #define RWCORELIB_API
#endif

namespace rw {
namespace core {

    // Compatibility with legacy code
    namespace debug { class Channel; }

/** \namespace rw::core::controller
    \brief Contains classes to describe controller devices
    and a Manager class to control them.
*/
namespace controller {

class DeviceInfo;
class DeviceState;
class DeviceEffect;
class LLManager;

/**
    \brief The options structure is used as a set of options for use by the Controller Manager class.

    Putting the initialization options in a separate structure allows options
    to be added without breaking existing code.
*/
struct RWCORELIB_API Options
{
    /** \brief Options constructor with user provided allocator.
        \param coreAllocator The allocator to be used.
        \importlib rwcontroller
    */
    Options(EA::Allocator::ICoreAllocator* coreAllocator);

    // Deprecated constructor.  Use Options(EA::Allocator::ICoreAllocator*)
    // instead of this one.
    Options(rw::core::debug::Channel*, EA::Allocator::ICoreAllocator*);

    /*! Allocator for the controller library to use when allocating dynamic
        memory.  The caller must not destroy this object while the controller
        library is active.
    */
    EA::Allocator::ICoreAllocator *mAllocator;

#if defined EA_PLATFORM_WINDOWS
    /*! [PC only] If you want to enable XInput support, set this to true (the
        default is false).  If you enable XInput support, you should be aware
        of some differences between XInput and DirectInput.  Firstly, if a
        device is detected as supporting XInput, it will be treated as an XInput
        device, not a DirectInput device.  Secondly, XInput controllers are not
        enumerated until the Manager::Update function is called, unlike
        DirectInput devices which are enabled as soon as they are connected,
        without requiring a call to update the connection status.  Thirdly,
        the number of XInput devices is fixed at four, and they occupy the
        first four device IDs.  Even if there are fewer than four XInput
        controllers connected to the system, Manager::GetNumControllers will
        return 4 plus the number of attached DirectInput devices (keyboard,
        mouse, non-XInput controllers).  This is the same as the consoles,
        where the maximum number of controllers is set at a fixed value
        (Manager::MAX_DEVICES).  Finally, this library supports force
        feedback for XInput devices, it does not support force feedback for
        DirectInput devices.
        \sa Manager::MAX_DEVICES, Manager::Update
    */
    bool                        mUseXInput;
	/*! [PC Only] If you want input to sent to the window only when it is in the foreground, set ths to true.
	( the default is true ).
	*/
	bool						mUseForeground;
	/*! [PC Only] Must pass in a pointer to the main window handle to allow DirectInput to set the cooperative level.  ( Default is 0, which will fail ) */
	void*						mWnd;
#endif
};

/**
    \brief Manager is the controller class for the Controller library.
    It is a singleton class, and must be instantiated with
    Manager::CreateInstance before any other Controller library
    calls are made.
*/
class RWCORELIB_API Manager
{
public:

    /*! \brief Platform-dependent constant that defines the maximum
        number of devices for that system.  Use GetNumControllers()
        instead of MAX_DEVICES when iterating over a controller
        array, because GetNumControllers can return the actual number
        of devices that are connected and may be polled.
        \sa GetNumControllers
    */
    enum ManagerConstants
    {
#if defined EA_PLATFORM_WINDOWS 
        MAX_DEVICES = 8,
#elif defined EA_PLATFORM_XBOXONE
		MAX_DEVICES = 4, //Boooo, it should be 8!
#elif defined EA_PLATFORM_PS4
		MAX_DEVICES = 4 // A guess
#endif
    };

    /**
    \brief Create the singleton of Manager with the given options.
    \importlib rwcontroller
    */
    static void     CreateInstance(const Options &options);
    
    /**
    \brief Return the Manager singleton.
    \importlib rwcontroller
    */
    static Manager* GetInstance();
    
    /**
    \brief Destroy the Manager singleton.
    \importlib rwcontroller
    */
    static void     DestroyInstance();
    
    /**
    \brief Check if the singleton is initialized.
    */
    static bool     IsInitialized() { return rw::core::controller::Manager::sInstance != NULL; }
    
    /**
    \brief GetAllocator returns the allocator object that the
    Controller library uses to allocate and deallocate heap memory.
    */
    static EA::Allocator::ICoreAllocator* GetAllocator() { return rw::core::controller::Manager::sAllocator; }

    /**
    \brief Enable dead zone filtering
    */
    static void EnableDeadZoneFilter()
    { mEnableDeadZoneFilter = true; }

    /**
    \brief Disable dead zone filtering
    */
    static void DisableDeadZoneFilter()
    { mEnableDeadZoneFilter = false; }

    /**
    \brief Returns whether dead zone filtering is enabled.
    */
    static bool IsDeadZoneFilteringEnabled()
    { return mEnableDeadZoneFilter; }

    /**
    \brief Updates all the devices, will check for new/removed devices
    and updates the state data
    \importlib rwcontroller
    */
    void Update();

    /** \brief Gets the DeviceInfo object for a device.

    On Windows, if you have XInput support enabled, this function can return
    NULL even if the deviceNumber parameter is less than the number returned
    by GetNumControllers because the first four devices are reserved for the
    XInput devices.  If there are not four XInput devices attached, some of
    the first four devices will be NULL.
    \param deviceNumber The index of the device to get
    \return The info for the device, or NULL if no device is connected
    \importlib rwcontroller
    */
    const DeviceInfo* GetDeviceInfo(unsigned int deviceNumber) const;

    /** \brief Gets the DeviceState object for a device.

    On Windows, if you have XInput support enabled, this function can
    return NULL even if the deviceNumber parameter is less than the
    number returned by GetNumControllers because the first four devices
    are reserved for the XInput devices.  If there are not four XInput
    devices attached, some of the first four devices will be NULL.
    Also on Windows, the data returned from DirectX is passed back 
    unmodified to the caller.  In some controllers, DirectX maps the
    right analog stick to the Z-axis.  This behavior is not modified
    by this library, so it is up to the caller to remap the Z-axis 
    values to the X-axis.
    \param deviceNumber The index of the device to get
    \return The state for the device, or NULL if no device is connected
    \importlib rwcontroller
    \sa GetNumControllers
    */
    const DeviceState* GetDeviceState(unsigned int deviceNumber) const;

    /** \brief Gets the DeviceEffect object for a device.

    On Windows, if you have XInput support enabled, this function can
    return NULL even if the deviceNumber parameter is less than the
    number returned by GetNumControllers because the first four devices
    are reserved for the XInput devices.  If there are not four XInput
    devices attached, some of the first four devices will be NULL.
    \return If the platform or device supports effects, this will be a
    DeviceEffect object that can provide force-feedback.  If the device
    does not support effects, NULL is returned.  On PS3, this function
    always returns NULL, as that platform's controllers do not support
    force-feedback.
    \importlib rwcontroller
    \sa GetDeviceInfo, GetDeviceState, GetNumControllers
    */
    DeviceEffect* GetDeviceEffect(unsigned int deviceNumber) const;

    /**
    \brief Returns the number of controller slots for the system

    For consoles this is a fixed number (MAX_DEVICES).  For PC, if you are
    using XInput, this will be four plus the number of attached DirectInput
    devices.  If you are not using XInput, it will be the number
    of devices attached to the system when the Controller library is
    initialized.
    \importlib rwcontroller
    \sa Options, MAX_DEVICES, GetDeviceInfo, GetDeviceState, GetDeviceEffect
    */
    unsigned int GetNumControllers() const;


	const EA::Controller::IEAController* GetController(int index);

#if defined(EA_PLATFORM_XBOXONE)
	uint64_t GetGamePadID(int index);	
	void SetNullPads(bool enable);
	void SimulateButtonPush(unsigned int deviceNumber, const int* buttonIds, unsigned int numButtons, unsigned int numFrames);
#endif

private:
    explicit Manager(const Options &options);
    ~Manager();

    // new/delete operators
    static void *operator new(size_t size);
    static void operator delete(void *p);

    static Manager *sInstance;

    // allocator for the Controller library
    static EA::Allocator::ICoreAllocator *sAllocator;

    static bool mEnableDeadZoneFilter;

    // This class uses a Pointer-To-Implementation approach
    // to hide the messiness of Windows and DirectX
    LLManager *pImpl;
};

}}}

#endif
