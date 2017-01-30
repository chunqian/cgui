#ifndef  MEMINT_H
#define  MEMINT_H

extern void InitMemory(int (*atexit_ptr)(void (*func)(void)));
extern void DeInitChain(void);

extern int cgui_started;

#endif
