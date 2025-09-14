// (c) 2004-2005 Criterion Software Limited and its licensors. All Rights Reserved

#include <EAStdC/EAMemory.h>
#include <eathread/eathread_thread.h>
#include <unit/unit.h>
#include <EAAssert/eaassert.h>
#include <stdio.h>

#if defined(EA_PLATFORM_WINDOWS)

// Use DirectInput 8
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x800
#endif

#include <dinput.h>

#elif defined(EA_PLATFORM_PS3)

#include <cell/pad/libpad.h> // cellPadInit/cellPadEnd

#endif

#include "rw/core/controller/manager.h"
#include "rw/core/controller/deviceeffect.h"
#include "rw/core/controller/devicestate.h"
#include "rw/core/controller/deviceinfo.h"

// Comment for verbose output
#define EA_TEST_MESSAGE(x)

#ifndef EA_TEST_MESSAGE
#define EA_TEST_MESSAGE(x) printf x
#endif

using namespace rw::core;

namespace
{
    const int TEST_TIMEOUT = 5; // in seconds

    class Allocator : public EA::Allocator::ICoreAllocator
    {
    public:
        Allocator() : bufPtr(buffer), endBufPtr(buffer + sizeof(buffer)), alloccount(0)
        {
            // empty
        }

        ~Allocator()
        {
            // assert that all the blocks have been released
            EA_ASSERT_MESSAGE(alloccount == 0, "Memory leak detected");
        }

    private:
        virtual void *Alloc(size_t size, const char *, unsigned int)
        {
            char *retval = bufPtr;
            bufPtr += size;
            ++alloccount;
            EA_ASSERT_MESSAGE(bufPtr <= endBufPtr, "Out of space in controller heap");

            return bufPtr <= endBufPtr ? retval : NULL;
        }

        virtual void *Alloc(size_t, const char *, unsigned int, unsigned int, unsigned int)
        {
            EA_FAIL_MESSAGE("Aligned allocations are not supported in this implementation");
            return NULL;
        }

        virtual void Free(void *block, size_t = 0)
        {
            --alloccount;

            EA_ASSERT_MESSAGE(alloccount >= 0, "One or more blocks has been freed twice");

            if (alloccount == 0)
            {
                // when the last allocation is released, reset
                // bufPtr to buffer
                bufPtr = buffer;
            }
        }

        char *bufPtr, *endBufPtr;
        int alloccount;
#if defined(EA_PLATFORM_WINDOWS)
        // assuming one keyboard, one mouse, and a single
        // XInput device is attached, with XInput enabled.
        static const int HEAP_SIZE = 2187 +
            16 + 40;    // DeviceState copy ctor & assignment operator tests
#elif defined(EA_PLATFORM_XENON)
        static const int HEAP_SIZE = 444 +
            60 + 40;    // DeviceState copy & assignment operator tests
#elif defined(EA_PLATFORM_PS3)
        // Correct size as of rwcontroller-1.03.00, SDK v1.90.
        // Assuming a single attached controller
        static const int HEAP_SIZE = 2469 +
            89 * 2; // DeviceState copy & assignment operator tests
#endif
        char buffer[HEAP_SIZE];
    };

    Allocator gAllocator;
}

class TestController : public EATest::TestSuite
{
    virtual void Initialize()
    {
        SuiteName("Controller");

        // only one test case here
        EATEST_REGISTER("Default", "Controller test", TestController, UnitTest);
        EATEST_REGISTER("Copy", "Controller test", TestController, Copy);

        controller::Options options(&gAllocator);

#if defined(EA_PLATFORM_WINDOWS)
        // Can also be false if you want the Universal Game Controller
        // to interface via DirectInput instead of XInput.  Only XInput
        // supports force-feedback.
        options.mUseXInput = true;
#elif defined(EA_PLATFORM_PS3)

        // It is the responsiblity of the rwcontroller library user
        // to call cellPadInit because there is an sequential order
        // dependency with the graphics system - graphics should be
        // started first.
        cellPadInit(controller::Manager::MAX_DEVICES);
#endif

        controller::Manager::CreateInstance(options);
        gControllerManager = controller::Manager::GetInstance();
    }

    virtual void Uninitialize()
    {
#if defined(EA_PLATFORM_PS3)
        cellPadEnd();
#endif
        gControllerManager->DestroyInstance();
        gControllerManager = NULL;
    }

    bool CheckButtonPresses(const controller::DeviceState *pDeviceState, uint32_t numButtons);
    bool CheckKeyPresses(const controller::DeviceState *pDeviceState, uint32_t numButtons);
    bool CheckAnalogPresses(const controller::DeviceState *pDeviceState, uint32_t numButtons);
    int GetTotalMovement(const int32_t *, const int32_t *, uint32_t);
    bool CheckAxis(const controller::DeviceState *pDeviceState, uint32_t numAxis);
    bool CheckMouse(const controller::DeviceState *pDeviceState, uint32_t numButtons, uint32_t numAxis);

    void UnitTest();
    void Copy();

    controller::Manager          *gControllerManager;
    controller::DeviceEffect     *gDeviceEffect;
};

bool TestController::CheckButtonPresses(const controller::DeviceState *pDeviceState, uint32_t numButtons)
{
    static bool lastButtonPresses[256];
    bool buttonPresses[256];
    char printBuf[1024];
    char *printBufPtr = printBuf;

    pDeviceState->GetAllButtonPressed(buttonPresses, numButtons);

    if (EA::StdC::Memcmp(lastButtonPresses, buttonPresses, sizeof(bool) * numButtons) != 0)
    {
        printBufPtr += stdc::Sprintf(printBuf, "Controller %d digital buttons:", pDeviceState->GetID());

        for (uint32_t i = 0; i < numButtons; ++i)
        {
            // verify both methods are the same
            EA_ASSERT(buttonPresses[i] == pDeviceState->GetButtonPressed(i)); // ("Get methods returned different values\n"));

            // if button is pressed, print it
            if (pDeviceState->GetButtonPressed(i))
            {
                printBufPtr += EA::StdC::Sprintf(printBufPtr, " %d", i);
            }
        }

        stdc::StringCopy(printBufPtr, "\n");
        EA_TEST_MESSAGE((printBuf));
        stdc::MemCopy(lastButtonPresses, buttonPresses, sizeof(bool) * numButtons);

        return true;
    }

    return false;
}

bool TestController::CheckKeyPresses(const controller::DeviceState *pDeviceState, uint32_t numButtons)
{
    static bool lastKeyPresses[256];
    bool KeyPresses[256];
    char printBuf[1024];
    char *printBufPtr = printBuf;

    pDeviceState->GetAllButtonPressed(KeyPresses, numButtons);

    if (stdc::MemCompare(lastKeyPresses, KeyPresses, sizeof(bool) * numButtons) != 0)
    {
        printBufPtr += stdc::Sprintf(printBuf, "Controller %d keys (State=0x%x):",
            pDeviceState->GetID(), pDeviceState->GetKeyState());

        for (uint32_t i = 0; i < numButtons; ++i)
        {
            // verify both methods are the same
            EA_ASSERT(KeyPresses[i] == pDeviceState->GetButtonPressed(i)); //    ("Get methods returned different values\n"));

            // if Key is pressed, print it
            if (pDeviceState->GetButtonPressed(i))
            {
                printBufPtr += stdc::Sprintf(printBufPtr, " Scan code: %d; VirtKey = %d; ASCII value = '%c'.",
                    i, pDeviceState->GetVirtKey(i), pDeviceState->GetAscii(i));
            }
        }

        stdc::StringCopy(printBufPtr, "\n");
        EA_TEST_MESSAGE((printBuf));
        stdc::MemCopy(lastKeyPresses, KeyPresses, sizeof(bool) * numButtons);

        return true;
    }

    return false;
}

bool TestController::CheckAnalogPresses(const controller::DeviceState *pDeviceState, uint32_t numButtons)
{
    static uint8_t lastAnalogPresses[256];
    uint8_t analogPresses[256];
    char printBuf[1024];
    char *printBufPtr = printBuf;

    pDeviceState->GetAllButtonValues(analogPresses, numButtons);

    if (stdc::MemCompare(lastAnalogPresses, analogPresses, sizeof(uint8_t) * numButtons) != 0)
    {
        printBufPtr += stdc::Sprintf(printBufPtr,
            "Controller %d analog buttons:", pDeviceState->GetID());

        for (uint32_t ctr = 0; ctr < numButtons; ctr++)
        {
            // verify both methods are the same
            EA_ASSERT(analogPresses[ctr] == pDeviceState->GetButtonValue(ctr));

            printBufPtr += stdc::Sprintf(printBufPtr, " %d(%d)", ctr, analogPresses[ctr]);
        }

        stdc::StringCopy(printBufPtr, "\n");
        EA_TEST_MESSAGE((printBuf));
        stdc::MemCopy(lastAnalogPresses, analogPresses, sizeof(uint8_t) * numButtons);

        return true;
    }

    return false;
}

#ifndef ABS
#define ABS(x) ((x)>=0?(x):-(x))
#endif

// smoothing function to reduce jitter on the analog sticks
int TestController::GetTotalMovement(const int32_t *prev_val,
                                     const int32_t *val, uint32_t numAxis)
{
    int total_diff(0);

    for (uint32_t i=0; i<numAxis; ++i)
    {
        total_diff += ABS(val[i]-prev_val[i]);
    }

    return total_diff;
}

bool TestController::CheckAxis(const controller::DeviceState *pDeviceState, uint32_t numAxis)
{
    static int32_t lastAxisValues[128];
    int32_t axisValues[128];
    char printBuf[1024];
    char *printBufPtr = printBuf;
    const int MIN_AXIS_THRESHOLD(100);

    pDeviceState->GetAllAxisValues(axisValues, numAxis);

    if (GetTotalMovement(lastAxisValues, axisValues, numAxis) > MIN_AXIS_THRESHOLD)
    {
        printBufPtr += stdc::Sprintf(printBufPtr, "Controller %d axis:", pDeviceState->GetID());

        for (uint32_t i = 0; i < numAxis; ++i)
        {
            // verify both methods are the same
            EA_ASSERT(axisValues[i] == pDeviceState->GetAxisValue(i)); //          ("Get methods returned different values\n"));

            printBufPtr += stdc::Sprintf(printBufPtr, " %d(%d)", i, axisValues[i]);
        }

        stdc::StringCopy(printBufPtr, "\n");
        EA_TEST_MESSAGE((printBuf));
        stdc::MemCopy(lastAxisValues, axisValues, sizeof(int32_t) * numAxis);

        return true;
    }

    return false;
}

bool TestController::CheckMouse(const controller::DeviceState *pDeviceState, uint32_t numButtons, uint32_t numAxis)
{
    static bool lastButtonPresses[256];
    bool buttonPresses[256];
    char tempPrintBuf[4096];
    char *pPrintBuf = tempPrintBuf;
    bool result = false;

    pDeviceState->GetAllButtonPressed(buttonPresses, numButtons);

    if (stdc::MemCompare(lastButtonPresses, buttonPresses, sizeof(bool) * numButtons) != 0)
    {
        pPrintBuf += stdc::Sprintf(pPrintBuf,
            "Controller %d digital buttons:", pDeviceState->GetID());

        for (uint32_t ctr = 0; ctr < numButtons; ctr++)
        {
            // verify both methods are the same
            EA_ASSERT(buttonPresses[ctr] == pDeviceState->GetButtonPressed(ctr)); //               ("Get methods returned different values\n"));

            // if button is pressed, print it
            if (pDeviceState->GetButtonPressed(ctr))
            {
                pPrintBuf += stdc::Sprintf(pPrintBuf, " %d", ctr);
            }
        }
        stdc::StringCopy(pPrintBuf, "\n");

        EA_TEST_MESSAGE((tempPrintBuf));

        result = true;
    }

    pPrintBuf = tempPrintBuf;

    stdc::MemCopy(lastButtonPresses, buttonPresses, sizeof(bool) * numButtons);

    static int32_t lastAxisValues[128]; // no more than 128 for now
    int32_t axisValues[128]; // no more than 128 for now

    pDeviceState->GetAllAxisValues(axisValues, numAxis);

    if (axisValues[0] || axisValues[1] || axisValues[2])
    {
        pPrintBuf += stdc::Sprintf(pPrintBuf, "Controller %d axis:", pDeviceState->GetID());
        for (uint32_t ctr = 0; ctr < numAxis; ctr++)
        {
            // verify both methods are the same
            EA_ASSERT(axisValues[ctr] == pDeviceState->GetAxisValue(ctr)); //             ("Get methods returned different values\n"));

            pPrintBuf += stdc::Sprintf(pPrintBuf, " %d(%d)", ctr, axisValues[ctr]);
        }

        stdc::StringCopy(pPrintBuf, "\n");
        EA_TEST_MESSAGE((tempPrintBuf));
        stdc::MemCopy(lastAxisValues, axisValues, sizeof(int32_t) * numAxis);

        result = true;
    }

    return result;
}

void TestController::UnitTest()
{
    // Allocate space to track connection status
    bool *pConnections = new bool[gControllerManager->GetNumControllers()];
    for (uint32_t i = 0; i < gControllerManager->GetNumControllers(); ++i)
    {
        pConnections[i] = false;
    }

    bool *pEffectSet = new bool[gControllerManager->GetNumControllers()];

    for (uint32_t i = 0; i < gControllerManager->GetNumControllers(); ++i)
    {
        pEffectSet[i] = false;
    }

    EA::Thread::ThreadTime testTimeOut = EA::Thread::GetThreadTime() + TEST_TIMEOUT*1000;

#if defined(EA_PLATFORM_PS3)
    bool interceptState = false;
#endif

    // loop till done
    while ( EA::Thread::GetThreadTime() < testTimeOut )
    {
        // update the controllers
        gControllerManager->Update();

#if defined(EA_PLATFORM_PS3)
        // It is necessary for the graphics system to be initialized before
        // pressing the PS button will engage the intercept state, so this 
        // won't actually work.  But it demonstrates how it would be done.
        if (gControllerManager->GetInterceptState() != interceptState)
        {
            // flip the boolean to the other value
            interceptState ^= true;

            if (interceptState)
            {
                EA_TEST_MESSAGE(("Controller input has been intercepted.\n"));
            }
            else
            {
                EA_TEST_MESSAGE(("Controller input has been restored.\n"));
            }
        }
#endif

        for (uint32_t i = 0; i < gControllerManager->GetNumControllers(); ++i)
        {
            // check if the controller is connected
            const controller::DeviceState *pDeviceState = gControllerManager->GetDeviceState(i);

            if (pDeviceState->IsConnected())
            {
                const rw::core::controller::DeviceInfo *pDeviceInfo =
                    gControllerManager->GetDeviceInfo(i);

                if (!pConnections[i])
                {
                    EA_TEST_MESSAGE(("Controller %d connected: %d buttons, %d axis, type %d", i,
                        pDeviceInfo->GetNumberButtons(), pDeviceInfo->GetNumberAxis(), pDeviceInfo->GetType()));
                    pConnections[i] = true;
                }

                if (pDeviceInfo->GetType() == controller::DeviceInfo::PAD)
                {
                    // check for button presses
                    if (CheckButtonPresses(pDeviceState, pDeviceInfo->GetNumberButtons()))
                    {
                        testTimeOut = EA::Thread::GetThreadTime() + TEST_TIMEOUT*1000;
                    }

                    // check for analog botton values
                    if (CheckAnalogPresses(pDeviceState, pDeviceInfo->GetNumberButtons()))
                    {
                        testTimeOut = EA::Thread::GetThreadTime() + TEST_TIMEOUT*1000;
                    }

                    // check for axis values
                    if (CheckAxis(pDeviceState, pDeviceInfo->GetNumberAxis()))
                    {
                        testTimeOut = EA::Thread::GetThreadTime() + TEST_TIMEOUT*1000;
                    }

                    // if start is pressed, break out
#if defined EA_PLATFORM_WINDOWS
                    // up/down/left/right (DPad) on Universal controller
                    if (pDeviceState->GetButtonPressed(controller::PAD_BUTTON_0))
#else
                    if (pDeviceState->GetButtonPressed(controller::PAD_BUTTON_START))
#endif
                    {
                        // stop the test by setting the timeout to 0
                        testTimeOut = 0;
                    }
                }
                else if (pDeviceInfo->GetType() == controller::DeviceInfo::KEYBOARD)
                {
                    // check for key presses
                    if (CheckKeyPresses(pDeviceState, pDeviceInfo->GetNumberButtons()))
                    {
                        testTimeOut = EA::Thread::GetThreadTime() + TEST_TIMEOUT*1000;
                    }

#if defined EA_PLATFORM_WINDOWS
                    // if ESC is pressed, break out
                    if (pDeviceState->GetButtonPressed(controller::SC_ESCAPE))
                    {
                        testTimeOut = 0;
                    }
#endif
                }
                else if (pDeviceInfo->GetType() == controller::DeviceInfo::MOUSE)
                {
                    // check for button presses
                    if (CheckMouse(pDeviceState, pDeviceInfo->GetNumberButtons(), pDeviceInfo->GetNumberAxis()))
                    {
                        testTimeOut = EA::Thread::GetThreadTime() + TEST_TIMEOUT*1000;
                    }

                    // if Back is pressed, break out
                    if (pDeviceState->GetButtonPressed(3))
                    {
                        testTimeOut = 0;
                    }
                }

                gDeviceEffect = gControllerManager->GetDeviceEffect(i);
                if (gDeviceEffect)
                {
                    // if up is pressed, setup an effect and start playing
#if defined EA_PLATFORM_WINDOWS
                    if (pDeviceState->GetButtonPressed(controller::PAD_BUTTON_4))   // start on the Universal controller
#else
                    if (pDeviceState->GetButtonPressed(controller::PAD_BUTTON_UP))
#endif
                    {
                        // supports effects
                        controller::DeviceEffect::Info effectInfo;

                        effectInfo.MagnitudeLeft  = controller::DeviceEffect::Info::MAX_MAGNITUDE;
                        effectInfo.MagnitudeRight = controller::DeviceEffect::Info::MAX_MAGNITUDE;
                        effectInfo.StartPlaying   = controller::DeviceEffect::Info::START;

                        gDeviceEffect->SetInfo(&effectInfo);
                        pEffectSet[i] = true;
                    }
#if defined EA_PLATFORM_WINDOWS
                    else if (pDeviceState->GetButtonPressed(controller::PAD_BUTTON_5))  // back on the Universal controller
#else
                    else if (pDeviceState->GetButtonPressed(controller::PAD_BUTTON_DOWN))
#endif
                    {
                        gDeviceEffect = gControllerManager->GetDeviceEffect(i);
                        if (gDeviceEffect && pEffectSet[i])
                        {
                            // supports effects
                            if (gDeviceEffect->IsPlaying())
                            {
                                gDeviceEffect->Stop();
                            }
                            else
                            {
                                gDeviceEffect->Start();
                            }
                        }
                    }
                }

                // On right-button press, enable dead-zone filtering
#if defined EA_PLATFORM_WINDOWS
                if (pDeviceState->GetButtonPressed(controller::PAD_BUTTON_3))
#else
                if (pDeviceState->GetButtonPressed(controller::PAD_BUTTON_RIGHT))
#endif
                {
                    EA_TEST_MESSAGE(("Dead zone filtering enabled\n"));
                    controller::Manager::EnableDeadZoneFilter();
                }

                // On left-button press, disable dead-zone filtering
#if defined EA_PLATFORM_WINDOWS
                if (pDeviceState->GetButtonPressed(controller::PAD_BUTTON_2))
#else
                if (pDeviceState->GetButtonPressed(controller::PAD_BUTTON_LEFT))
#endif
                {
                    EA_TEST_MESSAGE(("Dead zone filtering disabled\n"));
                    controller::Manager::DisableDeadZoneFilter();
                }
            }
            else
            {
                if (pConnections[i])
                {
                    EA_TEST_MESSAGE(("Controller %d disconnected", i));
                    pConnections[i] = false;
                }
            }
        }

        // sleep a bit to allow text output stream a chance to keep up.
        EA::Thread::ThreadSleep(10);
    }

    // shutdown the controller system
    delete pEffectSet;
    delete pConnections;
}

void TestController::Copy()
{
    // Test assignment operator and copy constructor
    gControllerManager->Update();

    controller::DeviceState *pDeviceState = new
        controller::DeviceState(*gControllerManager->GetDeviceState(0));

    const controller::DeviceInfo *devInfo = gControllerManager->GetDeviceInfo(0);

    if (devInfo != NULL)
    {
        unsigned int numButtons = devInfo->GetNumberButtons();
        CheckButtonPresses(pDeviceState, numButtons);
        CheckAnalogPresses(pDeviceState, numButtons);

        unsigned int numAxis = devInfo->GetNumberAxis();
        CheckAxis(pDeviceState, numAxis);
    }

    delete pDeviceState;

    pDeviceState = new controller::DeviceState(0, controller::DeviceState::LOCAL);
    pDeviceState = const_cast<controller::DeviceState *>(gControllerManager->GetDeviceState(0));
    pDeviceState->SetTimestamp(5);
    delete pDeviceState;
}

TestController gTestController;
