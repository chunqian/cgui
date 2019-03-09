#ifndef TABCHAIN_H
#define TABCHAIN_H

#include "hotkey.h"

struct t_tabchain;

typedef struct t_tcfun {

   /* MoveFocusToNextSubObject searches `b' for a possible mapping of the keypress
      onto a moving-focus command of a (possible) sub-object */
   int (*MoveFocusToNextSubObject) (struct t_object *b, struct t_object *sf,
                              int scan, int key);
   /* SetFocus will do what is needed to be done in `b' when it has got the
      "focus". */
   int (*SetFocus) (struct t_object * sf);
   /* UnSetFocus does whatever the object needs to do to be unfocused. A
      component in the object-struct is available for the needings. The
      function is not expected to do any updatings */
   void (*UnSetFocus) (struct t_object *b);
   /* LeaveTabChain notifies `b' that it is no longer within the tab-chain. */
   void (*LeaveTabChain) (struct t_object * b);
   /* MoveFocusTo unfocuses any object (disregarding the category) currently
      beeing in focus, and then makes the object `b' the one in
      focus. If `b' is a node, the `subid' within b will be set as the actual
      user-indicated focus. subid must refere to a leaf or be 0 to be ignored
      (default actions should be taken). If b is a sub-object within a
      `tabbed' node then that node, will be put in focus (with b as the actual
      simple object. */
   int (*MoveFocusTo)(struct t_object *b);
   /* NotifyInactivation notifies the link `tc' in the tab-chain that the
      object `b' goes inactive. */
   void (*NotifyInactivation)(struct t_tabchain *tc, struct t_object *b);
   /* Sets a focused object's apperance when the window goes out of focus.
      (screen update is included) */
   void (*SetFocusShadowed)(struct t_object *b, int on);
} t_tcfun;

/* Default behaviour, non-joiners */
extern t_tcfun default_tabchain_functions;
/* Default behaviour for a node joining the chain and controling a group of
   simple objects */
extern t_tcfun default_master_tc_functions;
/* Default behaviour for simple objects controled by node that have joined the
   chain */
extern t_tcfun default_slave_tc_functions;
/* Default behaviour for simple objects directly joining the chain */
extern t_tcfun default_single_tc_functions;

#define isEnter(scan, key) ((scan==KEY_ENTER)&(key=='\r'))
#define isCtrlEnter(scan, key) ((scan==KEY_ENTER)&(key=='\n'))

struct t_window;
struct t_tabchain;
struct t_object;

extern void _RemoveTabChain(struct t_window *w);
extern void GetTabChainFocus(struct t_window *w, struct t_object **linkob, struct t_object **subob);

extern struct t_object *SubFocusOfLink(struct t_tabchain *tc);
extern void SetSubFocusOfLink(struct t_tabchain *tc, struct t_object *sf);
extern void UnlinkFromTabChain(struct t_window *w, struct t_tabchain *link, struct t_object *sf);

/* Returns 1 if the key is used for navigation from the currently focused object in the window `w'
   to another object in the tab chain, or for navigation between subobjects within the focused object
   if that is a "master object" type, else 0.
   Which keys that are used depends on the object type, typically the cursor keys are used, but other
   might as well (currently no normal keys are used), and specially any key can be rejected. */
extern int TabChainNotifyKeyPress(struct t_window *w, int scan, int ascii);

extern int MoveFocusToNextSubObject(struct t_object *nf, struct t_object * sf, int scan, int key);
extern struct t_tabchain *SetLinkInFocus(struct t_tabchain *tc);

extern int SingleDoJoinTabChain(struct t_object *b);
extern int NodeDoJoinTabChain(struct t_object *b);
extern int SlaveMoveFocusTo(struct t_object *sf);

extern void NotifyTabChainWindowIsFinished(struct t_tabchain *tc);
extern void SetFocusHighlighted(struct t_tabchain *tc);
extern void SetFocusDiscrete(struct t_tabchain *tc);

extern void InitializeFocus(struct t_tabchain *tc);

extern void InitTabChain(void);

extern t_object *GetSubFocus(struct t_node *nd);

extern int insert_tab_links_after_focus;
#endif
