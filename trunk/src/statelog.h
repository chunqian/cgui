#ifndef STATELOG_H
#define STATELOG_H

//#define STATE_LOGG_ON
#ifdef STATE_LOGG_ON

#define STATE_LOGG_REASON(x) mouse_state_logg_reason = x
#define CREATE_MOUSE_STATE_LOGG_WINDOW() create_mouse_state_logg_window()
struct t_object;
struct t_mousedata;
extern char *mouse_state_logg_reason;
extern void create_mouse_state_logg_window(void);
extern void MouseStateTransitionDone(struct t_object*, int, int, struct t_mousedata *);
#define MOUSE_STATE_TRANSITION_DONE(b, o, mb, m) MouseStateTransitionDone(b, o, mb, m)

#else

#define STATE_LOGG_REASON(x)
#define CREATE_MOUSE_STATE_LOGG_WINDOW()
#define MOUSE_STATE_TRANSITION_DONE(b, o, mb, m)

#endif


#endif
