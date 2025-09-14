#ifndef __TIMER_H__
#define __TIMER_H__

// -------------- Defines ------------------ //
#define TICK_HZ         1000

// -------------- Structures ------------------ //
// -------------- Globals ------------------ //
// -------------- Prototypes ------------------ //
int Timer_Init();
void Timer_SetFrameDelta(float f);
void Timer_StartFrame();
float Timer_GetTime();
float Timer_GetTotalTime();
unsigned Timer_GetTicks();
unsigned Timer_GetTotalTicks();

#endif //__TIMER_H__