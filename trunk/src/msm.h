#ifndef MSM_H
#define MSM_H

#include "mstates.h"

struct t_object;
typedef struct t_mousefocus {
   struct t_object *b; /* Points to the object that has been caught by the
                          mouse, i.e. that has got some state other than init*/
   int isset;          /* The category of state-machine currently running
                          E.g. click, drag..; objects may have the capability
                          of having several. */
   int flags;          /* Copy of a gripped object's flag-variable */
   void *data;         /* Copy of the object's data-pointer */
   void *gripdata;     /* Copy of the object's gripdata-pointer */
   int id;             /* Copy of the object's id-key */
   void *(*Grip) (void *src, int id, int reason); /* Grip-function of `b' */
   int (*Drop) (void *dest, int id, void *src, int reason, int flags);
                       /* Drop-function of `b' */
   int (*Slide) (int x, int y, void *src, int id, int reason);
                       /* Slide-function of `b' */
   void (*Action) (void *);
                       /* Action-function of `b' */
   struct t_node *winnode; /* Pointer to the window of previous event to `b',
                            used to make it possible to swap windows */
} t_mousefocus;

struct t_mousedata;
typedef struct t_mevent {
   int (*MouseEv) (struct t_mousedata *m, struct t_object *b);
   int (*CurMouseFocus) (struct t_mousedata *m, struct t_node *hitnode);
   int (*StateHandlers[MS_MAX]) (int x, int y, int over, int mb,
                                 struct t_mousedata *m, struct t_object *b);
} t_mevent;

struct t_mevent;
struct t_mousedata;
struct t_window;
struct t_object;

extern void SetDefaultStateMachine(struct t_mevent *me);
extern int MouseEv(struct t_mousedata *m, struct t_object *b);
extern int SetWindowStates(struct t_mousedata *m, struct t_window *win);
extern void StopMouseEvent(struct t_mousefocus *f);

/* wait so long before determining if it was a click or slide (used when both */
/* click and slide are assigned to the same mouse button) */
extern int slidedelay;

/* the maximum mouse move within slidedelay to be interpreted as a click
   (used */
/* when both click and slide are assigned to same mouse button) */
extern int slidetolerance;

/* wait so long before determining if it was a grip or click (used when both */
/* click and grip are assigned to the same mouse button) */
extern int gripdelay;
extern void InitMouseStateMachine(void);

extern t_mousefocus msmf1;
extern t_mousefocus msmf2;

extern void ResetMouseState(void);

extern int inProgress(void);
extern void SetClick(struct t_object *b, int x, int y);
extern void ReturnState1(void);
extern void ResetState(struct t_mousedata *m, struct t_object *b);
extern int isOver(int x, int y, struct t_object *b);
extern void SetDropper(struct t_object *b, int x, int y);
extern void ReturnDropper(void);

#endif
