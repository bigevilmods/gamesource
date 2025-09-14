// -------------- Includes ------------------ //
#include "Apt.h"
#include "timer.h"
#include <realsystem.h>
#include <realsystem.h>
#include <realmem.h>
#include <realmath.h>
#include <realcodec.h>
#include <realfile.h>
#include <realinput.h>

#include <stdio.h>

// PSP emulator hack (see notes below)
#if defined(PSP_EMU)
extern "C" {
    int sceEmuGetPerformanceTimer();
}
#endif

// -------------- Globals ------------------ //
int nFrequency;
#if defined(EA_PLATFORM_XBOX)
unsigned __int64 nCurrentCycles, nStartCycles;
#endif
#if defined(EA_PLATFORM_PLAYSTATION2)
unsigned long nCurrentCycles, nStartCycles;
#endif

#if defined(EA_PLATFORM_PSP)
    unsigned long nCurrentCycles, nStartCycles;
    // note that this is using an undocumented function for the emulator only
    //  and returns current value in milliseconds
    //      so there is no need to adjust with TICK_HZ or CPUhz
    #if defined(PSP_EMU)
        #ifdef CPU_getcycle
            #undef CPU_getcycle
        #endif
        #define CPU_getcycle    sceEmuGetPerformanceTimer
    #endif
#endif
#if defined(EA_PLATFORM_WINDOWS)
unsigned __int64 nCurrentCycles, nStartCycles;
#ifdef CPU_getcycle
#undef CPU_getcycle
#endif
#define CPU_getcycle CPU_getcycle64
#endif

#if defined(EA_PLATFORM_GAMECUBE)
unsigned long long nCurrentCycles, nStartCycles;
#endif

#if defined(EA_PLATFORM_WINDOWS)
unsigned __int64 nPrevCycles, nFrameCycles, nPrevTicks, nCurrentTicks, nFrameTicks;
#else
unsigned int nPrevCycles, nFrameCycles, nPrevTicks, nCurrentTicks, nFrameTicks;
#endif

float fFrameTime;

#if defined(EA_PLATFORM_WINDOWS)
unsigned __int64 CPU_getcycle64()
{
    unsigned int i32_0;
    unsigned int i32_1;
    unsigned __int64 i64;
    __asm rdtsc;
    __asm mov i32_0,eax;
    __asm mov i32_1,edx;
    i64 = (unsigned __int64)i32_1;
    i64 = i64<<32;
    i64|=i32_0;
    return i64;
}
#endif
/* -----------------------------------------------------------------------
// initialize the timer
// ----------------------------------------------------------------------- */
int Timer_Init()
{
    nFrequency = CPUhz;
    nCurrentCycles = nStartCycles = CPU_getcycle();

    fFrameTime = 0.f;
    nFrameTicks = 0;

    return 1;
}

/* -----------------------------------------------------------------------
// tell the timer another frame is starting
// ----------------------------------------------------------------------- */
void Timer_StartFrame()
{
    nPrevCycles = nCurrentCycles;
    nPrevTicks = nCurrentTicks;

    nFrameCycles = CPU_getcycle() - nPrevCycles;
    nCurrentCycles += nFrameCycles;

#if defined(PSP_EMU)
    // --- on the PSP_EMU, CPU_getcycles returns milliseconds!
    fFrameTime = (float)nFrameCycles;
    nCurrentTicks = (unsigned)(nCurrentCycles - nStartCycles);

#else
    fFrameTime = (float)nFrameCycles / CPUhz;
    nCurrentTicks = (unsigned)TICK_HZ*(nCurrentCycles - nStartCycles)/CPUhz;

#endif

    nFrameTicks = nCurrentTicks - nPrevTicks;
}

/* -----------------------------------------------------------------------
// get the time elapsed from the last frame (seconds)
// ----------------------------------------------------------------------- */
float Timer_GetTime()
{
    return fFrameTime;
}

/* -----------------------------------------------------------------------
// get the time elapsed from the last frame (ticks -- 60hz)
// ----------------------------------------------------------------------- */
unsigned Timer_GetTicks()
{
    return (unsigned)nFrameTicks;
}
