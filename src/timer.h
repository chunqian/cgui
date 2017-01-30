#ifndef TIMER_H
#define TIMER_H

extern void InitTimer(void);
extern unsigned long _cgui_tick_counter;
#define GetTicks() _cgui_tick_counter

#endif
