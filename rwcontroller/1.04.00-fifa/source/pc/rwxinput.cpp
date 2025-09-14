// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved

/*      controller PC manager class implementation */
#include "rw/core/controller/deviceinfo.h"
#include "rw/core/controller/devicestate.h"
#include "rw/core/controller/manager.h"
#include "rw/core/controller/deviceeffect.h"
#include "private_rwxinput.h"

#include "rw/math/fpu/vector2.h"
#include "rw/math/fpu/vector2_operation.h"

#include <stdio.h>

#pragma warning(push)
#pragma warning(disable: 4917)
#include <oleauto.h>
#include <wbemidl.h>
#pragma warning(pop)

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

namespace rw
{
    namespace core
    {
        namespace controller
        {

#ifndef RW_CORE_CONTROLLER_NORMALIZE_AXIS_DEFINED
#define RW_CORE_CONTROLLER_NORMALIZE_AXIS_DEFINED 1

            // Macro to normalize the axis values to a range
            inline int32_t NormalizeAxis(int32_t value, int32_t minValue, int32_t maxValue, int32_t minRange, int32_t maxRange)
            {
                return ((((maxRange - minRange) * (value - minValue)) / (maxValue - minValue)) - ((maxRange - minRange) / 2));
            }
#endif

            /**
            check to see if it is a XInput device
            http://msdn.microsoft.com/library/default.asp?url=/library/en-us/directx9_c/dx9_xinput_xinput_dinput.asp

            //-----------------------------------------------------------------------------
            // Enum each PNP device using WMI and check each device ID to see if it contains 
            // "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it's an XInput device
            // Unfortunately this information can not be found by just using DirectInput 
            //-----------------------------------------------------------------------------

            \param pGuidProductFromDirectInput Guid from device instance
            \return true if device is XInput verified.
            */

#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

            bool XInputManager::IsXInputDevice( const GUID* pGuidProductFromDirectInput )
            {
                IWbemLocator*           pIWbemLocator  = NULL;
                IEnumWbemClassObject*   pEnumDevices   = NULL;
                IWbemClassObject*       pDevices[20]   = {0};
                IWbemServices*          pIWbemServices = NULL;
                BSTR                    bstrNamespace  = NULL;
                BSTR                    bstrDeviceID   = NULL;
                BSTR                    bstrClassName  = NULL;
                DWORD                   uReturned      = 0;
                bool                    bIsXinputDevice= false;
                UINT                    iDevice        = 0;
                VARIANT                 var;
                HRESULT                 hr;

                // CoInit if needed
                hr = CoInitialize(NULL);
                bool bCleanupCOM = SUCCEEDED(hr);

                // Create WMI
                hr = CoCreateInstance( __uuidof(WbemLocator),
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    __uuidof(IWbemLocator),
                    (LPVOID*) &pIWbemLocator);
                if( FAILED(hr) || pIWbemLocator == NULL )
                    goto LCleanup;

                bstrNamespace = SysAllocString( L"\\\\.\\root\\cimv2" );if( bstrNamespace == NULL ) goto LCleanup;
                bstrClassName = SysAllocString( L"Win32_PNPEntity" );   if( bstrClassName == NULL ) goto LCleanup;
                bstrDeviceID  = SysAllocString( L"DeviceID" );          if( bstrDeviceID == NULL )  goto LCleanup;

                // Connect to WMI 
                hr = pIWbemLocator->ConnectServer( bstrNamespace, NULL, NULL, 0L, 
                    0L, NULL, NULL, &pIWbemServices );
                if( FAILED(hr) || pIWbemServices == NULL )
                    goto LCleanup;

                // Switch security level to IMPERSONATE. 
                CoSetProxyBlanket( pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, 
                    RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );

                hr = pIWbemServices->CreateInstanceEnum( bstrClassName, 0, NULL, &pEnumDevices ); 
                if( FAILED(hr) || pEnumDevices == NULL )
                    goto LCleanup;

                // Loop over all devices
                for( ;; )
                {
                    // Get 20 at a time
                    hr = pEnumDevices->Next( 10000, 20, pDevices, &uReturned );
                    if( FAILED(hr) )
                        goto LCleanup;
                    if( uReturned == 0 )
                        break;

                    for( iDevice=0; iDevice<uReturned; iDevice++ )
                    {
                        // For each device, get its device ID
                        hr = pDevices[iDevice]->Get( bstrDeviceID, 0L, &var, NULL, NULL );
                        if( SUCCEEDED( hr ) && var.vt == VT_BSTR && var.bstrVal != NULL )
                        {
                            // Check if the device ID contains "IG_".  If it does, then it's an XInput device
                            // This information can not be found from DirectInput 
                            if( wcsstr( var.bstrVal, L"IG_" ) )
                            {
                                // If it does, then get the VID/PID from var.bstrVal
                                DWORD dwPid = 0, dwVid = 0;
                                WCHAR* strVid = wcsstr( var.bstrVal, L"VID_" );
                                if( strVid && swscanf( strVid, L"VID_%4X", &dwVid ) != 1 )
                                    dwVid = 0;
                                WCHAR* strPid = wcsstr( var.bstrVal, L"PID_" );
                                if( strPid && swscanf( strPid, L"PID_%4X", &dwPid ) != 1 )
                                    dwPid = 0;

                                // Compare the VID/PID to the DInput device
                                DWORD dwVidPid = static_cast<DWORD>(MAKELONG( dwVid, dwPid ));
                                if( dwVidPid == pGuidProductFromDirectInput->Data1 )
                                {
                                    bIsXinputDevice = true;
                                    goto LCleanup;
                                }
                            }
                        }
                        SAFE_RELEASE( pDevices[iDevice] );
                    }
                }

LCleanup:
                if(bstrNamespace)
                    SysFreeString(bstrNamespace);
                if(bstrDeviceID)
                    SysFreeString(bstrDeviceID);
                if(bstrClassName)
                    SysFreeString(bstrClassName);
                for( iDevice=0; iDevice<20; iDevice++ )
                    SAFE_RELEASE( pDevices[iDevice] );
                SAFE_RELEASE( pEnumDevices );
                SAFE_RELEASE( pIWbemLocator );
                SAFE_RELEASE( pIWbemServices );

                if( bCleanupCOM )
                    CoUninitialize();

                return bIsXinputDevice;
            }

            // Updates all of the devices and checks for new/removed devices
            void XInputManager::Update(uint32_t timeStamp)
            {
                for (int ctr = 0; ctr < GetNumXInputControllers(); ++ctr)
                {
                    DeviceState *curDeviceState = mDeviceStateList[ctr];
                    DeviceInfo  *curDeviceInfo  = mDeviceInfoList[ctr];

                    // get pad state
                    XINPUT_STATE xInputState;
                    bool isConnected = (XInputGetState(static_cast<DWORD>(ctr), &xInputState) == ERROR_SUCCESS);

                    // if a device is connected, update it
                    if (isConnected)
                    {
                        // if there was no device connected before, set up the info & state
                        if (!curDeviceState->IsConnected())
                        {
                            XINPUT_CAPABILITIES deviceCapabilities;

                            XInputGetCapabilities(static_cast<DWORD>(ctr), XINPUT_FLAG_GAMEPAD, &deviceCapabilities);

                            switch(deviceCapabilities.SubType)
                            {
                            case XINPUT_DEVSUBTYPE_GAMEPAD:
                                // This library doesn't offer specific support for
                                // any controller types other than the wheel at this 
                                // time.
                            default:
                                // setup device info
                                curDeviceInfo->Reset(DeviceInfo::PAD, 16, 4);
                                break;
/* Hack AY: 
                            case XINPUT_DEVSUBTYPE_WHEEL:

                                // TODO: check this against an actual device.
                                curDeviceInfo->Reset(DeviceInfo::WHEEL, 16, 4);
                                break;
*/								
                            }

                            // first 14 buttons are digital, rest are analog
                            for (unsigned int i = 14; i < curDeviceInfo->GetNumberButtons(); ++i)
                            {
                                curDeviceInfo->SetButtonCapability(i, DeviceInfo::ANALOG);
                            }

                            // setup device state
                            curDeviceState->Reset(DeviceState::CONNECTED, 14, 2, 4);
                        }

                        // set the timestamp
                        curDeviceState->SetTimestamp(timeStamp);

                        // set digital buttons
                        for (uint32_t i = 0; i < 10; ++i)
                        {
                            curDeviceState->SetButtonValue(i,
                                static_cast<uint8_t>(((xInputState.Gamepad.wButtons & (1 << i)) == (1 << i)) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE));
                        }

                        for (uint32_t i = 10; i < 14; ++i)
                        {
                            curDeviceState->SetButtonValue(i,
                                static_cast<uint8_t>(((xInputState.Gamepad.wButtons & (1 << (i+2))) == (1 << (i+2))) ? DeviceState::MAX_BUTTON_VALUE : DeviceState::MIN_BUTTON_VALUE));
                        }

                        // set analog buttons
                        curDeviceState->SetButtonValue(
                            curDeviceState->GetNumberDigitalButtons() + 0,
                            xInputState.Gamepad.bLeftTrigger);

                        curDeviceState->SetButtonValue(
                            curDeviceState->GetNumberDigitalButtons() + 1,
                            xInputState.Gamepad.bRightTrigger);

                        if (Manager::IsDeadZoneFilteringEnabled() &&
                            (curDeviceInfo->GetType() == DeviceInfo::PAD))
                        {
                            const float XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE_FLOAT = static_cast<const float>(
                                XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                            const float XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE_FLOAT = static_cast<const float>(
                                XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

							rw::math::fpu::Vector2 LeftThumb(xInputState.Gamepad.sThumbLX, xInputState.Gamepad.sThumbLY);
                            float thumbLen = MIN(rw::math::fpu::Magnitude(LeftThumb), 32767.0f);

                            if (thumbLen < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE_FLOAT)
                            {
                                // in dead zone
                                xInputState.Gamepad.sThumbLX = 0;
                                xInputState.Gamepad.sThumbLY = 0;
                            }
                            else
                            {
                                // not in dead zone, so let's scale both axes at once
                                const float scale =
                                    (thumbLen - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE_FLOAT) /
                                    (32767.0f - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE_FLOAT);

                                xInputState.Gamepad.sThumbLX =
                                    static_cast<SHORT>(scale * xInputState.Gamepad.sThumbLX);

                                xInputState.Gamepad.sThumbLY =
                                    static_cast<SHORT>(scale * xInputState.Gamepad.sThumbLY);
                            }

                            rw::math::fpu::Vector2 RightThumb(xInputState.Gamepad.sThumbRX, xInputState.Gamepad.sThumbRY);
                            thumbLen = MIN(rw::math::fpu::Magnitude(RightThumb), 32767.0f);

                            if (thumbLen < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE_FLOAT)
                            {
                                // in dead zone
                                xInputState.Gamepad.sThumbRX = 0;
                                xInputState.Gamepad.sThumbRY = 0;
                            }
                            else
                            {
                                // not in dead zone, so let's scale both axes at once
                                const float scale =
                                    (thumbLen - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE_FLOAT) /
                                    (32767.0f - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE_FLOAT);

                                xInputState.Gamepad.sThumbRX =
                                    static_cast<SHORT>(scale * xInputState.Gamepad.sThumbRX);

                                xInputState.Gamepad.sThumbRY =
                                    static_cast<SHORT>(scale * xInputState.Gamepad.sThumbRY);
                            }
                        }

                        // set axis
                        curDeviceState->SetAxisValue(PAD_AXIS_LEFTX,  NormalizeAxis(xInputState.Gamepad.sThumbLX, -32768, 32767, DeviceState::MIN_AXIS_VALUE, DeviceState::MAX_AXIS_VALUE));
                        curDeviceState->SetAxisValue(PAD_AXIS_LEFTY,  NormalizeAxis(xInputState.Gamepad.sThumbLY, -32768, 32767, DeviceState::MIN_AXIS_VALUE, DeviceState::MAX_AXIS_VALUE));
                        curDeviceState->SetAxisValue(PAD_AXIS_RIGHTX, NormalizeAxis(xInputState.Gamepad.sThumbRX, -32768, 32767, DeviceState::MIN_AXIS_VALUE, DeviceState::MAX_AXIS_VALUE));
                        curDeviceState->SetAxisValue(PAD_AXIS_RIGHTY, NormalizeAxis(xInputState.Gamepad.sThumbRY, -32768, 32767, DeviceState::MIN_AXIS_VALUE, DeviceState::MAX_AXIS_VALUE));
                    }
                    else
                    {
                        // see if a device used to be connected
                        if (curDeviceState->IsConnected())
                        {
                            // device was removed
                            curDeviceInfo->Reset(DeviceInfo::UNKNOWN, 0, 0);
                            curDeviceState->Reset(DeviceState::DISCONNECTED, 0, 0, 0);
                        }
                   }
                }
            }

            // The constructor for the XInputManager class
            XInputManager::XInputManager(bool active, DeviceInfo **ppDevInfo, DeviceState **ppDevState)
                : mActive(active), mDeviceInfoList(ppDevInfo), mDeviceStateList(ppDevState)
            {
                if (mActive)
                {
                    for (unsigned int i = 0; i < static_cast<unsigned int>(GetNumXInputControllers()); ++i)
                    {
                        // alloc space for device info & state
                        mDeviceInfoList[i] = new DeviceInfo(i);
                        mDeviceStateList[i] = new DeviceState(i, DeviceState::LOCAL);
                        mDeviceEffectList[i] = new DeviceEffect(i);
                    }
                }
            }

            // Destructor for the XInputManager class
            XInputManager::~XInputManager()
            {
                if (!mActive)
                {
                    return;
                }

                for (int i = NUM_XINPUT_CONTROLLERS-1; i >= 0; --i)
                {
                    delete mDeviceEffectList[i];
                    delete mDeviceStateList[i];
                    delete mDeviceInfoList[i];
                }
            }

            void *XInputManager::operator new(size_t size)
            {
                return Manager::GetAllocator()->Alloc(size, "rw::core::controller::XInputManager", EA::Allocator::MEM_PERM);
            }

            void XInputManager::operator delete(void *p)
            {
                Manager::GetAllocator()->Free(p);
            }
        }
    }
}
