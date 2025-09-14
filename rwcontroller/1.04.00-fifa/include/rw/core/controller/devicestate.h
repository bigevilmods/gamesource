// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved

/**
\file
Header file of rw::core::controller::DeviceState
*/

#ifndef RW_CORE_CONTROLLER_DEVICESTATE_H
#define RW_CORE_CONTROLLER_DEVICESTATE_H

#if defined _MSC_VER
    #pragma once
#endif

#include "EABase/eabase.h"

#if !defined RWCORELIB_API
    #if 0 // defined EA_DLL && !defined RWCROSS
        #define RWCORELIB_API __declspec(dllimport)
    #else
        #define RWCORELIB_API
    #endif
#endif

namespace rw {
namespace core {
namespace controller {

/**
\brief list of possible button Ids, this is platform specific
*/
enum PadButtonId
{
    #if defined(EA_PLATFORM_PS3) || defined(EA_PLATFORM_PS4)
        PAD_BUTTON_TOUCH_PAD,
        PAD_BUTTON_LTHUMB,
        PAD_BUTTON_RTHUMB,
        PAD_BUTTON_START,
        PAD_BUTTON_UP,
        PAD_BUTTON_RIGHT,
        PAD_BUTTON_DOWN,
        PAD_BUTTON_LEFT,
        PAD_BUTTON_L2,
        PAD_BUTTON_R2,
        PAD_BUTTON_L1,
        PAD_BUTTON_R1,
        PAD_BUTTON_TRIANGLE,
        PAD_BUTTON_CIRCLE,
        PAD_BUTTON_X,
        PAD_BUTTON_SQUARE,
    #elif defined EA_PLATFORM_WINDOWS
        PAD_BUTTON_0,
        PAD_BUTTON_1,
        PAD_BUTTON_2,
        PAD_BUTTON_3,
        PAD_BUTTON_4,
        PAD_BUTTON_5,
        PAD_BUTTON_6,
        PAD_BUTTON_7,
        PAD_BUTTON_8,
        PAD_BUTTON_9,
        PAD_BUTTON_10,
        PAD_BUTTON_11,
        PAD_BUTTON_12,
        PAD_BUTTON_13,
        PAD_BUTTON_14,
        PAD_BUTTON_15,
        PAD_BUTTON_16,
        PAD_BUTTON_17,
        PAD_BUTTON_18,
        PAD_BUTTON_19,
        PAD_BUTTON_20,
        PAD_BUTTON_21,
        PAD_BUTTON_22,
        PAD_BUTTON_23,
    #elif defined EA_PLATFORM_XENON || defined EA_PLATFORM_XBOXONE
        PAD_BUTTON_UP,
        PAD_BUTTON_DOWN,
        PAD_BUTTON_LEFT,
        PAD_BUTTON_RIGHT,
        PAD_BUTTON_START,
        PAD_BUTTON_BACK,
        PAD_BUTTON_LTHUMB,
        PAD_BUTTON_RTHUMB,
        PAD_BUTTON_LEFT_SHOULDER,
        PAD_BUTTON_RIGHT_SHOULDER,
        PAD_BUTTON_A,
        PAD_BUTTON_B,
        PAD_BUTTON_X,
        PAD_BUTTON_Y,
        PAD_BUTTON_LTRIGGER,
        PAD_BUTTON_RTRIGGER,
    #endif

    PAD_BUTTON_ID_MAX
};

/**
\brief list of possible axis Ids
*/
enum PadAxisId
{
    // First four axis IDs are common to all platforms
    PAD_AXIS_LEFTX,
    PAD_AXIS_LEFTY,
    PAD_AXIS_RIGHTX,
    PAD_AXIS_RIGHTY,

#if defined EA_PLATFORM_PS3
    PAD_AXIS_SENSORX,
    PAD_AXIS_SENSORY,
    PAD_AXIS_SENSORZ,
    PAD_AXIS_SENSORG,
#elif defined EA_PLATFORM_WINDOWS
    // These ones are only available on Windows, and then only
    // on some controller types
    PAD_AXIS_LEFTZ,
    PAD_AXIS_RIGHTZ,
    PAD_AXIS_SLIDER0,
    PAD_AXIS_SLIDER1,
    PAD_AXIS_POV0,
    PAD_AXIS_POV1,
    PAD_AXIS_POV2,
    PAD_AXIS_POV3,
#endif

    PAD_AXIS_ID_MAX
};

enum MouseAxisId
{
    MOUSE_AXIS_X,
    MOUSE_AXIS_Y,
    MOUSE_AXIS_Z,

    MOUSE_AXIS_ID_MAX
};

#if defined EA_PLATFORM_WINDOWS
/** \brief PC keyboard scan codes

    Scancode is 16-bit, where the top byte contains device information,
    and the low byte is the actual scancode. For keyboard, DirectInput's
    scancode is used, which is different from Windows' scancode. For example,
    because NEC keyboards support different scan codes than the PC-enhanced
    keyboards, DirectInput translates NEC key-scan codes into PC-enhanced scan
    codes where possible.
    These codes can be used to determine if a specific physical key is being
    pressed on a keyboard.
*/
enum PcKeyScancode
{
    SC_NULL =          0x00,
    SC_ESCAPE =        0x01,        ///< Escape (Esc)
    SC_1 =             0x02,        ///< 1
    SC_2 =             0x03,        ///< 2
    SC_3 =             0x04,        ///< 3
    SC_4 =             0x05,        ///< 4
    SC_5 =             0x06,        ///< 5
    SC_6 =             0x07,        ///< 6
    SC_7 =             0x08,        ///< 7
    SC_8 =             0x09,        ///< 8
    SC_9 =             0x0A,        ///< 9
    SC_0 =             0x0B,        ///< 0
    SC_MINUS =         0x0C,        ///< minus on main keyboard
    SC_EQUALS =        0x0D,        ///< equals on main keyboard
    SC_BACK =          0x0E,        ///< backspace
    SC_TAB =           0x0F,        ///< tab
    SC_Q =             0x10,        ///< Q
    SC_W =             0x11,        ///< W
    SC_E =             0x12,        ///< E
    SC_R =             0x13,        ///< R
    SC_T =             0x14,        ///< T
    SC_Y =             0x15,        ///< Y
    SC_U =             0x16,        ///< U
    SC_I =             0x17,        ///< I
    SC_O =             0x18,        ///< O
    SC_P =             0x19,        ///< P
    SC_LBRACKET =      0x1A,        ///< Left bracket
    SC_RBRACKET =      0x1B,        ///< Right bracket
    SC_RETURN =        0x1C,        ///< Enter on main keyboard
    SC_LCONTROL =      0x1D,        ///< Left control
    SC_A =             0x1E,        ///< A
    SC_S =             0x1F,        ///< S
    SC_D =             0x20,        ///< D
    SC_F =             0x21,        ///< F
    SC_G =             0x22,        ///< G
    SC_H =             0x23,        ///< H
    SC_J =             0x24,        ///< J
    SC_K =             0x25,        ///< K
    SC_L =             0x26,        ///< L
    SC_SEMICOLON =     0x27,        ///< Semicolon
    SC_APOSTROPHE =    0x28,        ///< Apostrophe
    SC_GRAVE =         0x29,        ///< accent grave
    SC_LSHIFT =        0x2A,        ///< Left Shift
    SC_BACKSLASH =     0x2B,        ///< Backslash
    SC_Z =             0x2C,        ///< Z
    SC_X =             0x2D,        ///< X
    SC_C =             0x2E,        ///< C
    SC_V =             0x2F,        ///< V
    SC_B =             0x30,        ///< B
    SC_N =             0x31,        ///< N
    SC_M =             0x32,        ///< M
    SC_COMMA =         0x33,        ///< Comma
    SC_PERIOD =        0x34,        ///< period on main keyboard
    SC_SLASH =         0x35,        ///< slash on main keyboard
    SC_RSHIFT =        0x36,        ///< Right shift
    SC_MULTIPLY =      0x37,        ///< multiply on numeric keypad
    SC_LMENU =         0x38,        ///< left Alt
    SC_SPACE =         0x39,        ///< Space
    SC_CAPITAL =       0x3A,        ///< Caps Lock
    SC_F1 =            0x3B,        ///< F1
    SC_F2 =            0x3C,        ///< F2
    SC_F3 =            0x3D,        ///< F3
    SC_F4 =            0x3E,        ///< F4
    SC_F5 =            0x3F,        ///< F5
    SC_F6 =            0x40,        ///< F6
    SC_F7 =            0x41,        ///< F7
    SC_F8 =            0x42,        ///< F8
    SC_F9 =            0x43,        ///< F9
    SC_F10 =           0x44,        ///< F10
    SC_NUMLOCK =       0x45,        ///< Numlock
    SC_SCROLL =        0x46,        ///< Scroll Lock
    SC_NUMPAD7 =       0x47,        ///< Numpad7
    SC_NUMPAD8 =       0x48,        ///< Numpad8
    SC_NUMPAD9 =       0x49,        ///< Numpad9
    SC_SUBTRACT =      0x4A,        ///< subtract on numeric keypad
    SC_NUMPAD4 =       0x4B,        ///< Numpad4
    SC_NUMPAD5 =       0x4C,        ///< Numpad5
    SC_NUMPAD6 =       0x4D,        ///< Numpad6
    SC_ADD =           0x4E,        ///< add on numeric keypad
    SC_NUMPAD1 =       0x4F,        ///< Numpad1
    SC_NUMPAD2 =       0x50,        ///< Numpad2
    SC_NUMPAD3 =       0x51,        ///< Numpad3
    SC_NUMPAD0 =       0x52,        ///< Numpad0
    SC_DECIMAL =       0x53,        ///< decimal on numeric keypad
    SC_OEM_102 =       0x56,        ///< < > | on UK/Germany keyboards
    SC_F11 =           0x57,        ///< F11
    SC_F12 =           0x58,        ///< F12
    SC_F13 =           0x64,        ///< F13 (NEC PC98)
    SC_F14 =           0x65,        ///< F14 (NEC PC98)
    SC_F15 =           0x66,        ///< F15 (NEC PC98)
    SC_KANA =          0x70,        ///< Kana (Japanese keyboard)
    SC_ABNT_C1 =       0x73,        ///< / ? on Portugese (Brazilian) keyboards
    SC_CONVERT =       0x79,        ///< Convert (Japanese keyboard)
    SC_NOCONVERT =     0x7B,        ///< No Convert (Japanese keyboard)
    SC_YEN =           0x7D,        ///< Yen (Japanese keyboard)
    SC_ABNT_C2 =       0x7E,        ///< Numpad on Portuguese (Brazilian) keyboards
    SC_NUMPADEQUALS =  0x8D,        ///< equals on numeric keypad (NEC PC98)
    SC_PREVTRACK =     0x90,        ///< Previous Track (SC_CIRCUMFLEX on Japanese keyboard)
    SC_AT =            0x91,        ///< AT (NEC PC98)
    SC_COLON =         0x92,        ///< Colon (NEC PC98)
    SC_UNDERLINE =     0x93,        ///< Underline (NEC PC98)
    SC_KANJI =         0x94,        ///< Kanji (Japanese keyboard)
    SC_STOP =          0x95,        ///< Stop (NEC PC98)
    SC_AX =            0x96,        ///< AX (Japan AX)
    SC_UNLABELED =     0x97,        ///< Unlabeled (J3100)
    SC_NEXTTRACK =     0x99,        ///< Next Track
    SC_NUMPADENTER =   0x9C,        ///< Enter on numeric keypad
    SC_RCONTROL =      0x9D,        ///< Right Control
    SC_MUTE =          0xA0,        ///< Mute
    SC_CALCULATOR =    0xA1,        ///< Calculator
    SC_PLAYPAUSE =     0xA2,        ///< Play / Pause
    SC_MEDIASTOP =     0xA4,        ///< Media Stop
    SC_VOLUMEDOWN =    0xAE,        ///< Volume -
    SC_VOLUMEUP =      0xB0,        ///< Volume +
    SC_WEBHOME =       0xB2,        ///< Web home
    SC_NUMPADCOMMA =   0xB3,        ///< , on numeric keypad (NEC PC98)
    SC_DIVIDE =        0xB5,        ///< / on numeric keypad
    SC_SYSRQ =         0xB7,        ///< Sys Request
    SC_RMENU =         0xB8,        ///< right Alt
    SC_PAUSE =         0xC5,        ///< Pause
    SC_HOME =          0xC7,        ///< Home on arrow keypad
    SC_UP =            0xC8,        ///< UpArrow on arrow keypad
    SC_PRIOR =         0xC9,        ///< PgUp on arrow keypad
    SC_LEFT =          0xCB,        ///< LeftArrow on arrow keypad
    SC_RIGHT =         0xCD,        ///< RightArrow on arrow keypad
    SC_END =           0xCF,        ///< End on arrow keypad
    SC_DOWN =          0xD0,        ///< DownArrow on arrow keypad
    SC_NEXT =          0xD1,        ///< PgDn on arrow keypad
    SC_INSERT =        0xD2,        ///< Insert on arrow keypad
    SC_DELETE =        0xD3,        ///< Delete on arrow keypad
    SC_LWIN =          0xDB,        ///< Left Windows key
    SC_RWIN =          0xDC,        ///< Right Windows key
    SC_APPS =          0xDD,        ///< AppMenu key
    SC_POWER =         0xDE,        ///< System Power
    SC_SLEEP =         0xDF,        ///< System Sleep
    SC_WAKE =          0xE3,        ///< System Wake
    SC_WEBSEARCH =     0xE5,        ///< Web Search
    SC_WEBFAVORITES =  0xE6,        ///< Web Favorites
    SC_WEBREFRESH =    0xE7,        ///< Web Refresh
    SC_WEBSTOP =       0xE8,        ///< Web Stop
    SC_WEBFORWARD =    0xE9,        ///< Web Forward
    SC_WEBBACK =       0xEA,        ///< Web Back
    SC_MYCOMPUTER =    0xEB,        ///< My Computer
    SC_MAIL =          0xEC,        ///< Mail
    SC_MEDIASELECT =   0xED,        ///< Media Select
    SC_LALT =      SC_LMENU,        ///< Left Alt
    SC_RALT =      SC_RMENU         ///< Right Alt
};
#endif

/**
\brief A class that holds the current state of a device. 
This includes the state of all the buttons and axes as well as the
time when the state was last updated.
*/
class RWCORELIB_API DeviceState
{
public:
    /**
    \brief a list of keyboard state flags
    \see GetKeyState
    */
    enum KeyboardState
    {
        KS_SHIFT = 0x01,    ///< Shift Keystate Flag
        KS_CTRL = 0x02,     ///< Ctrl Keystate Flag
        KS_ALT = 0x04       ///< Alt Keystate Flag
    };

    /// \brief a list of connection states
    enum ConnectedState
    {
        DISCONNECTED,
        CONNECTED
    };

    /// \brief a list of network states
    enum NetworkState
    {
        /// The device is local to this system
        LOCAL,

        /// The device is on a remote (networked) system.  Its data
        /// is being collected by another system in the game and sent
        /// over the network.  Networked devices may need to be treated
        /// differently in the game due to increased latency over
        /// local devices.
        NETWORKED
    };

    enum ValueRanges
    {
        MIN_BUTTON_VALUE = 0x00,    ///< Minimum range of a button value
        MAX_BUTTON_VALUE = 0xFF,    ///< Maximum range of a button value
        MIN_AXIS_VALUE = -1000,     ///< Minimum range of an axis value
        MAX_AXIS_VALUE = 1000       ///< Maximum range of an axis value
    };

    static void *operator new(size_t size);
    static void operator delete(void *p);

    /// \brief Construct a DeviceState from an ID and specifies
    /// whether the device is networked (i.e. on a remote machine) or local
    DeviceState(unsigned int id, NetworkState networked);

    /// \brief Destructor
    ~DeviceState();

    /**
    \brief Returns the ID for the device.

    The device ID will be from 0-n based on the slot
    the device is plugged into on the system.
    \return the device ID
    */
    unsigned int GetID() const { return mDeviceID; }

#if defined(EA_PLATFORM_PS3)
    /// \brief Set whether the controller device has sensor data (SixAxis)
    void HasSensorData(bool b) { mHasSensorData = b; }

    /// \brief Sensor is 6DOF 
    bool HasSensorData() const { return mHasSensorData; }

    /// \brief Set whether the controller device has pressure data
    void HasPressData(bool b) { mHasPressData = b; }

    /// \brief Press is analogue buttons 
    bool HasPressData() const { return mHasPressData; }
#endif

    /**
    \brief Check whether a given device is connected
    \return TRUE if the device is connected, FALSE otherwise
    */
    bool IsConnected() const { return (mIsConnected == CONNECTED); }

    /**
    \brief Returns TRUE if the device is a network device
    \return TRUE if the device is a network device, FALSE otherwise
    */
    bool IsNetworked() const { return (mIsNetworked == NETWORKED); }

    /**
    \brief Returns the timestamp of when the data in the device was updated
    \return the time when the device was updated
    */
    unsigned int GetTimeStamp() const { return mTimestamp; }

    /** \brief Returns the boolean pressed state for a button.

    If the button is analog it is flagged as pressed when it is past a
    threshold value.  Use GetButtonValue to retrieve the data range.
    \param buttonIndex the button to check
    \return true if the button is pressed, false if it is not pressed
    \importlib rwcontroller
    \sa GetAllButtonPressed, GetButtonValue
    */
    bool GetButtonPressed(unsigned int buttonIndex) const;

    /** \brief Returns the boolean pressed state for multiple buttons.

    If the button is analog it is flagged as pressed when it is past a
    threshold value.  Use GetAllButtonValues to retrieve the data range
    of many buttons.
    \param pButtonPressed [OUT] array of pressed values to be filled by the call
    \param numButtons number of entries in the array, must be <= to the number of buttons
    \importlib rwcontroller
    \sa GetButtonPressed, GetAllButtonValues
    */
    void GetAllButtonPressed(bool *pButtonPressed, unsigned int numButtons) const;

    /** \brief Returns the value of a button.

    If the button is digital, the returned values will be either
    DeviceState::MIN_BUTTON_VALUE or DeviceState::MAX_BUTTON_VALUE.
    If the button is analog it will be the actual button reading, scaled
    to be between DeviceState::MIN_BUTTON_VALUE and
    DeviceState::MAX_BUTTON_VALUE.
    \param buttonIndex the button to get
    \return true if the button is pressed, false if it is not pressed
    \importlib rwcontroller
    \sa GetButtonPressed, GetAllButtonValues
    */
    uint8_t GetButtonValue(unsigned int buttonIndex) const;

    /** \brief Returns the value of multiple buttons.

    If the button is digital, the returned values will be either
    DeviceState::MIN_BUTTON_VALUE or DeviceState::MAX_BUTTON_VALUE.
    If the button is analog it will be the actual button reading, scaled
    to be between DeviceState::MIN_BUTTON_VALUE and
    DeviceState::MAX_BUTTON_VALUE.
    \param pButtonValues [OUT] array of button values to be fill out
    \param numButtons number of entries in the array, must be <= to the number of buttons
    \importlib rwcontroller
    \sa GetAllButtonPressed, GetButtonValue
    */
    void GetAllButtonValues(uint8_t *pButtonValues, unsigned int numButtons) const;

    /** \brief Returns the value of an axis.

    It will be in the range of DeviceState::MIN_AXIS_VALUE to
    DeviceState::MAX_AXIS_VALUE.  For mouse devices, this is the delta
    offset from the last update.
    \param axisIndex The axis to get
    \return The value of the axis
    \importlib rwcontroller
    */
    int GetAxisValue(unsigned int axisIndex) const;

    /** \brief Returns the value for an array of axis.

    They will be in the range of DeviceState::MIN_AXIS_VALUE to
    DeviceState::MAX_AXIS_VALUE.  For mouse devices, this is the delta
    offset from the last update.  For Xbox 360 Game Pads and Universal
    Game Controllers for Windows, the axis values for both the left and
    right analog sticks will have a substantial dead zone (about 20%).
    This is to comply with TCR requirements on Xbox 360.  For all other
    devices, dead-zones must be implemented by the library caller.
    \param pAxisValues [OUT] array of axis values to be fill out
    \param numAxis number of entries in the array, must be <= to the number of axis
    \importlib rwcontroller
    */
    void GetAllAxisValues(int *pAxisValues, unsigned int numAxis) const;

    /**
    \brief Gets ASCII value of a button on a keyboard device

    Only valid on Keyboard devices on the Windows platform.
    All other platforms return 0.
    \param buttonIndex The button to get
    \return The ASCII value of the button
    \importlib rwcontroller
    \todo Add support for Keyboard devices to PS3 & Xbox
    */
    unsigned int GetAscii(unsigned int buttonIndex) const;

    /** \brief Gets VirtualKey value of a button on a keyboard device

    Returns the Virtual Key value of a given button
    This function calls MapVirtualKeyEx on Windows, and returns the value
    returned by MapVirtualKeyEx.  On other platforms, this function returns 0.
    \note Note that some keys (arrow keys, Insert, Delete, Home, End, PageUp,
    PageDown) cannot be translated from the scan code (button value) to a
    Virtual Key code.  In those cases, GetVirtKey will return 0.
    \param buttonIndex The button to get
    \return The Virtual Key value of the button
    \importlib rwcontroller
    */
    unsigned int GetVirtKey(unsigned int buttonIndex) const;

    /** \brief Gets Key state (shift, control, alt) of a keyboard device

    Returns the key state of the keyboard device.  Only valid on Keyboard devices.
    \return OR'ed flags of the KeyboardState flags.
    For example, if a shift key is pressed, but not a control or alt key, the return value will be KS_SHIFT.
    On platforms other than Windows, this function returns 0.
    \sa KeyboardState
    \importlib rwcontroller
    */
    unsigned int GetKeyState() const;

    /**
    \brief Returns the number of digital buttons on this device
    \return The number of digital buttons
    */
    unsigned int GetNumberDigitalButtons() const { return mNumDigitalButtons; }

    /**
    \brief Returns the number of analog buttons on this device
    \return The number of analog buttons
    */
    unsigned int GetNumberAnalogButtons() const { return mNumAnalogButtons; }

    /**
    \brief Returns the number of axes on this device
    \return The number of axes
    */
    unsigned int GetNumberAxis() const { return mNumberAxis; }

    /**
    \brief Function to set the values of a button
    \importlib rwcontroller
    */
    void SetButtonValue(unsigned int buttonIndex, uint8_t buttonValue);

    /**
    \brief Sets all of the buttons in the device from an array
    if a button is digital it will treat 0 as unpressed and anything else as pressed
    \param buttonValues array of button values to use
    \param numButtons number of entries in the array, must be <= to the number of buttons
    \importlib rwcontroller
    */
    void SetAllButtonValues(const uint8_t *buttonValues, unsigned int numButtons);

    /**
    \brief Function to set the values of an axis
    \param axisIndex The axis to set
    \param axisValue The value of the axis    \importlib rwcontroller
    */
    void SetAxisValue(unsigned int axisIndex, int axisValue);

    /**
    \brief Sets all of the axes in the device from an array
    \param axisValues array of axis values to be use
    \param numAxis number of entries in the array, must be <= to the number of axis
    \importlib rwcontroller
    */
    void SetAllAxisValues(const int *axisValues, unsigned int numAxis);

    /// \brief Set the timestamp
    /// \param newTimestamp - the updated timestamp
    void SetTimestamp(unsigned int newTimestamp) { mTimestamp = newTimestamp; }

    /**
    \brief Resets the connected status, number of buttons and number of axes
    \param isConnected Is the device connected
    \param numDigitalButtons The number of digital buttons on the device
    \param numAnalogButtons The number of analog buttons on the device
    \param numAxis The number of axis on the device
    \importlib rwcontroller
    */
    void Reset(ConnectedState connected, unsigned int numDigitalButtons,
        unsigned int numAnalogButtons, unsigned int numAxis);

    /// \brief Copy constructor copies the data and allocates arrays for the
    /// buttons and axis
    /// \importlib rwcontroller
    /// \param device the source device
    DeviceState(const DeviceState &device);

    /// \brief Assignment operator copies the data and allocates arrays for
    /// the buttons and axis
    /// \importlib rwcontroller
    /// \param device the source device
    /// \return the copy
    DeviceState& operator=(const DeviceState &device);

private:

    // device ID for the controller, 0-n for the different 'ports' on the system
    unsigned int mDeviceID;

#if defined(EA_PLATFORM_PS3)
    bool mHasSensorData;
    bool mHasPressData;
#endif

    ConnectedState mIsConnected;
    NetworkState mIsNetworked;
    unsigned int mTimestamp;
    unsigned int mNumDigitalButtons;
    unsigned int mNumAnalogButtons;
    unsigned int mNumberAxis;

    // these are arrays to allow the individual devices to allocate just enough memory to hold
    // the data needed. The Get* functions above will remap the button index into the correct
    // entry in one of these arrays.

    uint8_t *mDigitalButtonValues;
    uint8_t *mButtonValues;
    int     *mAxisValues;
};

}}}

#endif
