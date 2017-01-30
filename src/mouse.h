#ifndef MOUSE_H
#define MOUSE_H

typedef struct t_mousedata {
   /* The coordinates of the mouse, set by the mouse-callback */
   int x, y, z;

   /* Mousebuttonstate at time for event generation */
   int mb;

   int requestcursor;

   /* Previous mouse button (when button was released during last even one need to know which one during the following events) */
   int prev;
} t_mousedata;

extern void InitMouse(void);
extern int LaunchMouseEvent(void *data);

/* Returns the current coordinates of the mouse. */
extern void _CguiMousePos(int *x, int *y, int *z);

/* Changes the mouse movement range. */
extern void _CguiSetRange(int x, int y, int w, int h);

/* Moves the mouse pointer to a certain position. */
extern void _CguiForceMousePos(int x, int y);

extern struct t_node *_mouse_touched_win1, *_mouse_touched_win2;

/* Latest mouse-button pressed, used to mask from more buttons touched. (The first
   one pressed in an operation is the one that shall fulfill it). */
extern int _previous_mouse_button;

#endif
