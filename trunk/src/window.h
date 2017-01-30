#ifndef WINDOW_H
#define WINDOW_H


#define DEFAULT_WINFRAME 2
#define MAXOBJ_DISTANCE (WINFRAMEWIDTH + 2)

#define W_SIZE_NORMALIZED  0x10
#define W_SIZE_ADAPTIVE    0x20
#define W_SIZE_MAXIMIZED   0x30
#define W_SIZE_MINIMIZED   0x40

typedef struct t_window {
   int modal;                   /* Flag telling that the window is modal */
   int hidden;                  /* A counter telling how many modal children
                                   a window has. If >0 it means that the
                                   window is stopped for interaction. */
   int dragger;                 /* Reference to the dragging object */
   int fillscreen;              /* A flag telling that the windows' size
                                   shall be the same as for the screen. */
   int completed;               /* A flag indigating if a window has already
                                   been completely generated (and displayed)*/
   struct t_tabchain *tc;       /* `tc' ("Tab-Chain") is a list of recipients
                                   of keyboard commands, investigated by the
                                   hot-key handler. See "tabchain.c". */
   struct t_node *node;         /* Pointer to the node of the window */
   struct t_node *opnode;       /* Pointer to the operating node, i.e. the
                                   node that is "open for input" for the
                                   application (e.g. for adding objects). */
   struct t_object *header;     /* Points to a labelled header object of the
                                   window. NULL if none. */
   void (*Free) (struct t_object *); /* Default freer */
   int autohk;                  /* Flag telling if hot-keys shall be
                                   automatically generated. */
   int pos_preset;              /* Flag telling if the position of the window
                                   is pre-set, and need not be calculated */
   int prefered_w;
   int prefered_h;
   int mode;                    /* The current screen mode, either of W_SIZE_*  */
   int horizontalbr;
   int verticalbr;
   int pos;
   int options;
} t_window;


extern struct t_node *MakeWin(int width, int height, const char *label,
                     int attr);
extern void _CguiCancel(void);
extern void RefreshScreen(int x1, int y1, int x2, int y2);
extern void InitWindows(void);
extern void ReMakeWindow(struct t_node *nd);
extern void _CguiCloseWindow(t_window *win);
extern void Complete(struct t_node *nd);
extern void CenterWindow(struct t_node *nd);
extern void RemakeWindows(int orgw, int orgh);
extern void WindowIsFinished(void);
extern int IsWindowCompleated(void);
extern int _VisableWindow(void);

extern struct t_object *NextWin(struct t_object *b);
extern struct t_object *FirstWin(void);
extern struct t_object *NextFreeWin(struct t_object *b);
extern struct t_object *FirstFreeWin(void);
extern int WinIsFree(struct t_object *b);

#define IsWindow(b) ((b)->node ? ((b)->node->win ? (b)->node->win->node==(b)->node:0):0)

#include "object.h"
#include "tabchain.h"
extern t_typefun default_window_type_functions;
extern t_tcfun default_window_tc_functions;

#endif
