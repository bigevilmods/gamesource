#ifndef __TIMER_H__
#define __TIMER_H__

// -------------- Defines ------------------ //
#define TICK_HZ         1000

// -------------- Structures ------------------ //
// -------------- Globals ------------------ //
// -------------- Prototypes ------------------ //
int Timer_Init();
void Timer_StartFrame();
float Timer_GetTime();
unsigned Timer_GetTicks();

#endif //__TIMER_H__