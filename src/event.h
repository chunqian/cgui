#ifndef EVENT_H
#define EVENT_H

extern int SafeInsertMessage(void (*Action) (void *), void *msg);
extern void CheckEvents(void);
extern void DestroyLinkedEvents(int objid);
extern void ExecuteAllPendingEvents(void);
extern int _KillEventOfCgui(unsigned id);
extern unsigned _GenEventOfCgui(void (*Action) (void *), void *msg, unsigned delay, int objid);

#endif
