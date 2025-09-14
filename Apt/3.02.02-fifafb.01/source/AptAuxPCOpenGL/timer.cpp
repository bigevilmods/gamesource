// -------------- Includes ------------------ //
#include "timer.h"
#include <windows.h>

// -------------- Globals ------------------ //
_int64 liFrequency, liStartProgramTick;
_int64 liPrevFrameTick, liCurrentTick;
float fFrameTime, fCurrentTime, fPrevTime;
unsigned uFrameTime, uCurrentTime, uPrevTime;
static float fDelta = 0.f;

/* -----------------------------------------------------------------------
// initialize the timer
// ----------------------------------------------------------------------- */
int Timer_Init()
{
    if (!QueryPerformanceFrequency((LARGE_INTEGER *)&liFrequency))
        return 0;

    QueryPerformanceCounter((LARGE_INTEGER *)&liStartProgramTick);
    liCurrentTick = liStartProgramTick;
    fCurrentTime = 0.f;
    uCurrentTime = 0;

    return 1;
}

/* -----------------------------------------------------------------------
// for manually overriding the time between frames (0 for actual time)
// ----------------------------------------------------------------------- */
void Timer_SetFrameDelta(float f)
{
    fDelta = f;

    if ( f == 0.0f )
    {
        fCurrentTime = fPrevTime;
        uCurrentTime = uPrevTime;
    }

    Timer_StartFrame();
}

/* -----------------------------------------------------------------------
// tell the timer another frame is starting
// ----------------------------------------------------------------------- */
void Timer_StartFrame()
{
    if (fDelta == 0.f)
    {
        liPrevFrameTick = liCurrentTick;
        QueryPerformanceCounter((LARGE_INTEGER *)&liCurrentTick);
        liCurrentTick -= liStartProgramTick;

        fPrevTime = fCurrentTime;
        uPrevTime = uCurrentTime;
        fCurrentTime = (float)liCurrentTick/liFrequency;
        uCurrentTime = (unsigned)((unsigned)TICK_HZ*liCurrentTick/liFrequency);

        fFrameTime = fCurrentTime - fPrevTime;
        uFrameTime = uCurrentTime - uPrevTime;
    }
    else
    {
        fCurrentTime += fDelta;
        fFrameTime = fDelta;

        // tick update
        uPrevTime = uCurrentTime;
        uCurrentTime = (unsigned)(fCurrentTime * 60.f);
        uFrameTime = uCurrentTime - uPrevTime;
    }
}

/* -----------------------------------------------------------------------
// get the time elapsed from the last frame (seconds)
// ----------------------------------------------------------------------- */
float Timer_GetTime()
{
    return fFrameTime;
}

/* -----------------------------------------------------------------------
// get the time elapsed since the start of the program  (seconds)
// ----------------------------------------------------------------------- */
float Timer_GetTotalTime()
{
    return fCurrentTime;
}

/* -----------------------------------------------------------------------
// get the time elapsed from the last frame (ticks -- 60hz)
// ----------------------------------------------------------------------- */
unsigned Timer_GetTicks()
{
    return uFrameTime;
}

/* -----------------------------------------------------------------------
// get the time elapsed since the start of the program  (ticks -- 60hz)
// ----------------------------------------------------------------------- */
unsigned Timer_GetTotalTicks()
{
    return uCurrentTime;
}
