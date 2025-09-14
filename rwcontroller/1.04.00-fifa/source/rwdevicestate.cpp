// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved
/**
\file
Implements rw::core::controller::DeviceState.
*/

/* Device State class implementation */
#include "rw/core/controller/deviceinfo.h"
#include "rw/core/controller/devicestate.h"
#include "rw/core/controller/manager.h"

#include "EAStdC/EAMemory.h"

#include "EAAssert/eaassert.h"

#if defined EA_PLATFORM_WINDOWS
// need this for keyboard
#include <windows.h>

// Defines for unidentified virtual keys
#define VK_SCOLON   186
#define VK_EQUAL    187
#define VK_COMMA    188
#define VK_MINUS    189
#define VK_PERIOD   190
#define VK_SLASH    191
#define VK_TILDE    192
#define VK_LBRACK   219
#define VK_BSLASH   220
#define VK_RBRACK   221
#define VK_APOST    222

// Defines for identified, but incorrectly defined (in winuser.h) virtual keys
#define VKN_NUMPAD0  45
#define VKN_NUMPAD1  35
#define VKN_NUMPAD2  40
#define VKN_NUMPAD3  34
#define VKN_NUMPAD4  37
#define VKN_NUMPAD5  12
#define VKN_NUMPAD6  39
#define VKN_NUMPAD7  36
#define VKN_NUMPAD8  38
#define VKN_NUMPAD9  33
#define VKN_MULTIPLY 106
#define VKN_ADD      107
#define VKN_SUBTRACT 109
#define VKN_DECIMAL  46
#define VKN_DIVIDE   181

// The user might want to use the following defines:
#define VK_PAGEUP    201
#define VK_PAGEDOWN  209
#define VKN_END      207
#define VKN_HOME     199
#define VKN_INSERT   210
#define VKN_DELETE   211
#define VKN_UP       200
#define VKN_LEFT     203
#define VKN_RIGHT    205
#define VKN_DOWN     208

#endif //defined RW_PLATFORM_WINDOWS

namespace rw {
namespace core {
namespace controller {

// get size in bytes for an array of bit flags
inline uint32_t BitGetFlagSize(uint32_t maxFlags)
{
    return ((maxFlags / (sizeof(uint8_t) * 8)) + 1);
}

// set a bit flag
inline void BitFlagSet(uint8_t *data, uint32_t flag)
{
    data[flag / 8] |= (1 << (flag % 8));
}

// clear a bit flag
inline void BitFlagClear(uint8_t *data, uint32_t flag)
{
    data[flag / 8] &= ~(static_cast<uint32_t>(1 << (flag % 8)));
}

// determine if a bit flag is set
inline bool BitFlagIsSet(const uint8_t *data, uint32_t flag)
{
    return (data[flag / 8] & (1 << (flag % 8))) == (1 << (flag % 8));
}

bool DeviceState::GetButtonPressed(unsigned int buttonIndex) const
{
    bool pressed = false;

    EA_ASSERT_MSG(buttonIndex < (mNumDigitalButtons + mNumAnalogButtons), "ButtonIndex is too large");

    if (buttonIndex < mNumDigitalButtons)
    {
        // the button is digital
        EA_ASSERT_MSG(mDigitalButtonValues != NULL, "Digital button values not allocated");
        pressed = BitFlagIsSet(mDigitalButtonValues, buttonIndex);
    }
    else if (buttonIndex < (mNumDigitalButtons + mNumAnalogButtons))
    {
        // the button is analog
        EA_ASSERT_MSG(mButtonValues != NULL, "Analog button values not allocated");

        // get the index for the analog array
        buttonIndex -= mNumDigitalButtons;
        pressed = (mButtonValues[buttonIndex] > 0);
    }

    return pressed;
}

void DeviceState::GetAllButtonPressed(bool *pButtonPressed, unsigned int numButtons) const
{
    EA_ASSERT_MSG(numButtons <= (mNumDigitalButtons + mNumAnalogButtons), "Too many buttons requested");
    EA_ASSERT_MSG(pButtonPressed != NULL, "Return array is NULL");

    for (uint32_t buttonIndex = 0; buttonIndex < numButtons; buttonIndex++)
    {
        pButtonPressed[buttonIndex] = GetButtonPressed(buttonIndex);
    }
}

uint8_t DeviceState::GetButtonValue(unsigned int buttonIndex) const
{
    uint8_t value = 0;

    EA_ASSERT_FORMATTED(buttonIndex < (mNumDigitalButtons + mNumAnalogButtons), ("buttonIndex %d is too large.  max is %d", buttonIndex, mNumDigitalButtons + mNumAnalogButtons));

    if (buttonIndex < mNumDigitalButtons)
    {
        // the button is digital
        EA_ASSERT_MSG(mDigitalButtonValues != NULL, "Digital button values not allocated");
        if (BitFlagIsSet(mDigitalButtonValues, buttonIndex))
        {
            value = MAX_BUTTON_VALUE;
        }
    }
    else if (buttonIndex < (mNumDigitalButtons + mNumAnalogButtons))
    {
        // the button is analog
        EA_ASSERT_MSG(mButtonValues != NULL, "Analog button values not allocated");

        // get the index for the analog array
        buttonIndex -= mNumDigitalButtons;
        value = mButtonValues[buttonIndex];
    }

    return value;
}

void DeviceState::GetAllButtonValues(uint8_t *pButtonValues, unsigned int numButtons) const
{
    EA_ASSERT_MSG(numButtons <= (mNumDigitalButtons + mNumAnalogButtons), "Too many buttons requested");
    EA_ASSERT_MSG(pButtonValues != NULL, "Return array is NULL");

    for (uint32_t buttonIndex = 0; buttonIndex < numButtons; buttonIndex++)
    {
        pButtonValues[buttonIndex] = GetButtonValue(buttonIndex);
    }
}

int DeviceState::GetAxisValue(unsigned int axisIndex) const
{
    EA_ASSERT_MSG(axisIndex < mNumberAxis, "axisIndex is too large");
    EA_ASSERT_MSG(mAxisValues != NULL, "Axis values not allocated");

    return mAxisValues[axisIndex];
}

void DeviceState::GetAllAxisValues(int *pAxisValues, unsigned int numAxis) const
{
    EA_ASSERT_MSG(numAxis <= mNumberAxis, "Too many axis requested");
    EA_ASSERT_MSG(pAxisValues != NULL, "Return array is NULL");

    for (uint32_t axisIndex = 0; axisIndex < numAxis; axisIndex++)
    {
        pAxisValues[axisIndex] = GetAxisValue(axisIndex);
    }
}

void DeviceState::SetButtonValue(unsigned int buttonIndex, uint8_t buttonValue)
{
    EA_ASSERT_MSG(buttonIndex < (mNumDigitalButtons + mNumAnalogButtons), "buttonIndex is too large");

    if (buttonIndex < mNumDigitalButtons)
    {
        // the button is digital
        EA_ASSERT_MSG(mDigitalButtonValues != NULL, "Digital button values not allocated");
        if (buttonValue == 0)
        {
            BitFlagClear(mDigitalButtonValues, buttonIndex);
        }
        else
        {
            BitFlagSet(mDigitalButtonValues, buttonIndex);
        }
    }
    else if (buttonIndex < (mNumDigitalButtons + mNumAnalogButtons))
    {
        // the button is analog
        EA_ASSERT_MSG(mButtonValues != NULL, "Analog button values not allocated");

        // get the index for the analog array
        buttonIndex -= mNumDigitalButtons;
        mButtonValues[buttonIndex] = buttonValue;
    }
}

void DeviceState::SetAllButtonValues(const uint8_t *buttonValues, unsigned int numButtons)
{
    EA_ASSERT_MSG(buttonValues != NULL, "Source array is NULL");

    for (uint32_t buttonIndex = 0; buttonIndex < numButtons; buttonIndex++)
    {
        SetButtonValue(buttonIndex, buttonValues[buttonIndex]);
    }
}

void DeviceState::SetAxisValue(unsigned int axisIndex, int axisValue)
{
    EA_ASSERT_MSG(axisIndex < mNumberAxis, "axisIndex is too large");
    EA_ASSERT_MSG(mAxisValues != NULL, "Axis values not allocated");

    mAxisValues[axisIndex] = axisValue;
}

void DeviceState::SetAllAxisValues(const int *axisValues, unsigned int numAxis)
{
    EA_ASSERT_MSG(axisValues != NULL, "Source array is NULL");

    for (uint32_t axisIndex = 0; axisIndex < numAxis; axisIndex++)
    {
        SetAxisValue(axisIndex, axisValues[axisIndex]);
    }
    return;
}

unsigned int DeviceState::GetAscii(unsigned int buttonIndex) const
{
    EA_ASSERT_MSG(Manager::GetInstance()->GetDeviceInfo(mDeviceID)->GetType() == DeviceInfo::KEYBOARD, "GetAscii called on a non-keyboard device");
    EA_ASSERT_MSG(buttonIndex < mNumDigitalButtons, "ButtonIndex is too large");

    uint32_t ACode = 0;

#if defined EA_PLATFORM_WINDOWS
    uint32_t VCode = 0;

    // Find vcode of undefined scancodes
    switch (buttonIndex)
    {
        case 156:
            ACode = 13;
            break;
        case 199:
        case 207:
        case 201:
        case 209:
        case 210:
        case 211:
        case 181:
        case VKN_UP:
        case VKN_LEFT:
        case VKN_RIGHT:
        case VKN_DOWN:
            VCode = buttonIndex;
            break;
        default:
            VCode = GetVirtKey(buttonIndex);
            break;
    }

    // Find equivalent vcode to acode values
    switch (VCode)
    {
        case VK_BACK:
        case VK_TAB:
        case VK_RETURN:
        case VK_ESCAPE:
        case VK_SPACE:
            ACode = VCode;
            break;
        case VKN_MULTIPLY:
            ACode = '*';
            break;
        case VKN_ADD:
            ACode = '+';
            break;
        case VKN_SUBTRACT:
            ACode = '-';
            break;
        case VKN_DIVIDE:
            ACode = '/';
            break;
        default:
            break;
    }

    // Obtain ASCII codes of letters and numbers
    if (((VCode>='A') && (VCode<='Z')) || ((VCode>='0') && (VCode<='9')))
    {
        ACode = VCode;
    }

    // Lowercase conversion if not caps-lock
    if (((::GetKeyState(VK_CAPITAL) & 1) == 0) && (ACode>='A') && (ACode<='Z'))
    {
        ACode += 'a'-'A';
    }

    // Process shift keys
    if ((::GetKeyState(VK_SHIFT) & 0x80) == 0x80)
    {
        switch(VCode)
        {
            case '1':
                ACode = '!';
                break;
            case '2':
                ACode = '@';
                break;
            case '3':
                ACode = '#';
                break;
            case '4':
                ACode = '$';
                break;
            case '5':
                ACode = '%';
                break;
            case '6':
                ACode = '^';
                break;
            case '7':
                ACode = '&';
                break;
            case '8':
                ACode = '*';
                break;
            case '9':
                ACode = '(';
                break;
            case '0':
                ACode = ')';
                break;
            case VK_SCOLON:
                ACode = ':';
                break;
            case VK_EQUAL:
                ACode = '+';
                break;
            case VK_COMMA:
                ACode = '<';
                break;
            case VK_MINUS:
                ACode = '_';
                break;
            case VK_PERIOD:
                ACode = '>';
                break;
            case VK_SLASH:
                ACode = '?';
                break;
            case VK_TILDE:
                ACode = '~';
                break;
            case VK_LBRACK:
                ACode = '{';
                break;
            case VK_BSLASH:
                ACode = '|';
                break;
            case VK_RBRACK:
                ACode = '}';
                break;
            case VK_APOST:
                ACode = '"';
                break;
            default:
                if ((ACode >= 'A') && (ACode <= 'Z'))
                {
                    ACode += 'a'-'A';
                }
                else if ((ACode >= 'a') && (ACode <= 'z'))
                {
                    ACode -= 'a'-'A';
                }
                break;
        }
    }
    else
    {
        // Non-shift characters
        switch (VCode)
        {
            case VK_SCOLON:
                ACode = ';';
                break;
            case VK_EQUAL:
                ACode = '=';
                break;
            case VK_COMMA:
                ACode = ',';
                break;
            case VK_MINUS:
                ACode = '-';
                break;
            case VK_PERIOD:
                ACode = '.';
                break;
            case VK_SLASH:
                ACode = '/';
                break;
            case VK_TILDE:
                ACode = '`';
                break;
            case VK_LBRACK:
                ACode = '[';
                break;
            case VK_BSLASH:
                ACode = '\\';
                break;
            case VK_RBRACK:
                ACode = ']';
                break;
            case VK_APOST:
                ACode = 39;
                break;
            default:
                break;
        }
    }

    // Numlock
    if ((::GetKeyState(VK_NUMLOCK) & 0x01) == 0x01)
    {
        switch (VCode)
        {
            case VKN_NUMPAD0:
                ACode = '0';
                break;
            case VKN_NUMPAD1:
                ACode = '1';
                break;
            case VKN_NUMPAD2:
                ACode = '2';
                break;
            case VKN_NUMPAD3:
                ACode = '3';
                break;
            case VKN_NUMPAD4:
                ACode = '4';
                break;
            case VKN_NUMPAD5:
                ACode = '5';
                break;
            case VKN_NUMPAD6:
                ACode = '6';
                break;
            case VKN_NUMPAD7:
                ACode = '7';
                break;
            case VKN_NUMPAD8:
                ACode = '8';
                break;
            case VKN_NUMPAD9:
                ACode = '9';
                break;
            case VKN_DECIMAL:
                ACode = '.';
                break;
            default:
                break;
        }
    }
    else
    {
        switch (VCode)
        {
            case VKN_NUMPAD0:
                VCode = VKN_INSERT;
                break;
            case VKN_NUMPAD1:
                VCode = VKN_END;
                break;
            case VKN_NUMPAD2:
                VCode = VKN_DOWN;
                break;
            case VKN_NUMPAD3:
                VCode = VK_PAGEDOWN;
                break;
            case VKN_NUMPAD4:
                VCode = VKN_LEFT;
                break;
            case VKN_NUMPAD5:
                VCode = 0;
                break;
            case VKN_NUMPAD6:
                VCode = VKN_RIGHT;
                break;
            case VKN_NUMPAD7:
                VCode = VKN_HOME;
                break;
            case VKN_NUMPAD8:
                VCode = VKN_UP;
                break;
            case VKN_NUMPAD9:
                VCode = VK_PAGEUP;
                break;
            case VKN_DECIMAL:
                VCode = VKN_DELETE;
                break;
            default:
                break;
        }
    }
#else
    buttonIndex = 0;
#endif

    return ACode;
}

unsigned int DeviceState::GetVirtKey(unsigned int buttonIndex) const
{
    EA_ASSERT_MSG(Manager::GetInstance()->GetDeviceInfo(mDeviceID)->GetType() == DeviceInfo::KEYBOARD, "GetVirtKey called on a non-keyboard device");
    EA_ASSERT_MSG(buttonIndex < mNumDigitalButtons, "ButtonIndex is too large");

#if defined EA_PLATFORM_WINDOWS
    return MapVirtualKeyEx(buttonIndex, 1, GetKeyboardLayout(0)) & 0xFF;
#else
    buttonIndex = 0;
    return 0;
#endif
}

unsigned int DeviceState::GetKeyState() const
{
    EA_ASSERT_MSG(Manager::GetInstance()->GetDeviceInfo(mDeviceID)->GetType() == DeviceInfo::KEYBOARD, "GetKeyState called on a non-keyboard device");

    uint32_t State = 0;

#if defined EA_PLATFORM_WINDOWS

    if (GetButtonPressed(SC_LSHIFT) || GetButtonPressed(SC_RSHIFT))
    {
        State |= KS_SHIFT;
    }

    if (GetButtonPressed(SC_LCONTROL) || GetButtonPressed(SC_RCONTROL))
    {
        State |= KS_CTRL;
    }

    if (GetButtonPressed(SC_LALT) || GetButtonPressed(SC_RALT))
    {
        State |= KS_ALT;
    }

#endif

    return State;
}

DeviceState::DeviceState(unsigned int id, NetworkState networked)
: mDeviceID(id),
#if defined(EA_PLATFORM_PS3)
  mHasSensorData(false),
  mHasPressData(false),
#endif
  mIsConnected(DISCONNECTED),
  mIsNetworked(networked),
  mTimestamp(0),
  mNumDigitalButtons(0),
  mNumAnalogButtons(0),
  mNumberAxis(0),
  mDigitalButtonValues(NULL),
  mButtonValues(NULL),
  mAxisValues(NULL)
{
    // empty
}

DeviceState::DeviceState(const DeviceState &device)
: mDeviceID(device.mDeviceID),
  mIsConnected(device.mIsConnected),
  mIsNetworked(device.mIsNetworked),
  mTimestamp(device.mTimestamp),
  mNumDigitalButtons(device.mNumDigitalButtons),
  mNumAnalogButtons(device.mNumAnalogButtons),
  mNumberAxis(device.mNumberAxis),
  mDigitalButtonValues(NULL),
  mButtonValues(NULL),
  mAxisValues(NULL)
{
    EA::Allocator::ICoreAllocator *pAllocator = Manager::GetAllocator();

    if (mNumDigitalButtons > 0)
    {
        mDigitalButtonValues = static_cast<uint8_t*>(pAllocator->Alloc(BitGetFlagSize(mNumDigitalButtons), "rw::core::controller::DeviceState::mDigitalButtonValues", EA::Allocator::MEM_PERM));
        if (mDigitalButtonValues != NULL)
        {
            EA::StdC::Memcpy(mDigitalButtonValues, device.mDigitalButtonValues, BitGetFlagSize(mNumDigitalButtons));
        }
    }

    if (mNumAnalogButtons > 0)
    {
        mButtonValues = static_cast<uint8_t*>(pAllocator->Alloc(sizeof(*mButtonValues) * mNumAnalogButtons, "rw::core::controller::DeviceState::mButtonValues", EA::Allocator::MEM_PERM));
        if (mButtonValues != NULL)
        {
            EA::StdC::Memcpy(mButtonValues, device.mButtonValues, sizeof(*mButtonValues) * mNumAnalogButtons);
        }
    }

    if (mNumberAxis > 0)
    {
        mAxisValues = static_cast<int*>(pAllocator->Alloc(sizeof(*mAxisValues) * mNumberAxis, "rw::core::controller::DeviceState::mAxisValues", EA::Allocator::MEM_PERM));
        if (mAxisValues != NULL)
        {
            EA::StdC::Memcpy(mAxisValues, device.mAxisValues, sizeof(*mAxisValues) * mNumberAxis);
        }
    }
}

DeviceState& DeviceState::operator=(const DeviceState &device)
{
    if (&device != this)
    {
        mDeviceID = device.mDeviceID;
        mIsNetworked = device.mIsNetworked;
        mTimestamp = device.mTimestamp;

        Reset(device.mIsConnected, device.mNumDigitalButtons, device.mNumAnalogButtons, device.mNumberAxis);

        if (mNumDigitalButtons > 0)
        {
            EA::StdC::Memcpy(mDigitalButtonValues, device.mDigitalButtonValues, BitGetFlagSize(mNumDigitalButtons));
        }

        if (mNumAnalogButtons > 0)
        {
            EA::StdC::Memcpy(mButtonValues, device.mButtonValues, sizeof(*mButtonValues) * mNumAnalogButtons);
        }

        if (mNumberAxis > 0)
        {
            EA::StdC::Memcpy(mAxisValues, device.mAxisValues, sizeof(*mAxisValues) * mNumberAxis);
        }
    }

    return (*this);
}

DeviceState::~DeviceState()
{
    EA::Allocator::ICoreAllocator *pAllocator = Manager::GetAllocator();

    if (mDigitalButtonValues != NULL)
    {
        pAllocator->Free(mDigitalButtonValues);
        mDigitalButtonValues = NULL;
    }

    if (mButtonValues != NULL)
    {
        pAllocator->Free(mButtonValues);
        mButtonValues = NULL;
    }

    if (mAxisValues != NULL)
    {
        pAllocator->Free(mAxisValues);
        mAxisValues = NULL;
    }
}

// Resets the info for a DeviceState including if it is connected, the number
// of buttons and the number of axis. This will free and allocate memory for
// the button and axis arrays upon a reconnect.
void DeviceState::Reset(ConnectedState connected, unsigned int numDigitalButtons,
                        unsigned int numAnalogButtons, unsigned int numAxis)
{
    mIsConnected = connected;

    // do not release the buffers associated with this device until a new
    // device is connected with different attributes.  On consoles, freeing
    // and re-allocating the buffers leads to needless fragmentation and on
    // a PC, it only wastes a few bytes of memory.
    if (mIsConnected == DISCONNECTED)
        return;

    EA::Allocator::ICoreAllocator *pAllocator = Manager::GetAllocator();

    // if the number of buttons changed, realloc the button values array
    if (mNumDigitalButtons != numDigitalButtons)
    {
        // free old values if needed
        if (mDigitalButtonValues != NULL)
        {
            pAllocator->Free(mDigitalButtonValues);
            mDigitalButtonValues = NULL;
        }
    }

    if (numDigitalButtons > 0)
    {
        // alloc a new array if needed
        if (mDigitalButtonValues == NULL)
        {
            mDigitalButtonValues = static_cast<uint8_t*>(pAllocator->Alloc(BitGetFlagSize(numDigitalButtons), "rw::core::controller::DeviceState::mDigitalButtonValues", EA::Allocator::MEM_PERM));
        }
        EA::StdC::Memclear(mDigitalButtonValues, BitGetFlagSize(numDigitalButtons));
    }

    // if the number of buttons changed, realloc the button values array
    if (mNumAnalogButtons != numAnalogButtons)
    {
        // free old values if needed
        if (mButtonValues != NULL)
        {
            pAllocator->Free(mButtonValues);
            mButtonValues = NULL;
        }
    }

    if (numAnalogButtons > 0)
    {
        // alloc a new array if needed
        if (mButtonValues == NULL)
        {
            mButtonValues = static_cast<uint8_t*>(pAllocator->Alloc(sizeof(*mButtonValues) * numAnalogButtons, "rw::core::controller::DeviceState::mButtonValues", EA::Allocator::MEM_PERM));
        }
        EA::StdC::Memclear(mButtonValues, sizeof(*mButtonValues) * numAnalogButtons);
    }

    // if the number of buttons changed, realloc the button values array
    if (mNumberAxis != numAxis)
    {
        // free old values if needed
        if (mAxisValues != NULL)
        {
            pAllocator->Free(mAxisValues);
            mAxisValues = NULL;
        }
    }

    if (numAxis > 0)
    {
        // alloc a new array if needed
        if (mAxisValues == NULL)
        {
            mAxisValues = static_cast<int*>(pAllocator->Alloc(sizeof(*mAxisValues) * numAxis, "rw::core::controller::DeviceState::mAxisValues", EA::Allocator::MEM_PERM));
        }
        EA::StdC::Memclear(mAxisValues, sizeof(*mAxisValues) * numAxis);
    }

    mNumDigitalButtons = numDigitalButtons;
    mNumAnalogButtons = numAnalogButtons;
    mNumberAxis = numAxis;
}

void *DeviceState::operator new(size_t size)
{
    return Manager::GetAllocator()->Alloc(size, "rw::core::controller::DeviceState", EA::Allocator::MEM_PERM);
}

void DeviceState::operator delete(void *p)
{
    Manager::GetAllocator()->Free(p);
}

}
}
}
