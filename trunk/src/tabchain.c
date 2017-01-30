/* Module TABCHAIN.C

   This file contains the functions for tab-chains. A tab chain is one or
   more objects, all within the same window, that shares the "tab" and
   "backtab" as hot-keys. When the tab- or backtab-key is pressed, the
   "tab-focus" moves to the next/previous object in the chain. The chain does
   not follow the logical tree-structure of the window, it just follows the
   order in which the objects joined the chain. The chain may be joined by
   either a single object or by a node.

   Single object: The object in tab-focus may be selected with enter or
   ctrl-enter keys (action as when clicked with left and right mouse
   buttons). The tab-focus is visually indicated in a way that is determined
   by the type of the object (hotkey.Focus and hotkey.Unfocus).

   Node: The node in tab-focus will have one single object in sub-focus -
   which is the actual focus. The object in sub-focus may be selected with
   enter or ctrl-enter keys (action as when clicked with left and right mouse
   buttons). The sub-focus may be moved to another single object while the
   node is in tab-focus. The sub-focus is visually indicated in a way that is
   determined by the type of the single object (hotkey.Focus and
   hotkey.Unfocus). The keys to be used for moving the sub-focus, and the
   meaning of these, is determined by the node-type. When
   a node "re-takes" tab-focus after beeing unfocused, the single object that
   will get sub-focus is the same that was in sub-focus the last time the
   node was in tab-focus. The first time a node gets tab-focus the object in
   sub-focus will be the one specified when the node joined the chain.

   Lists will by default join the chain as a node, as well as "object-tables"
   do. An object table is group of symmetrical objects. The focus is moved
   from one object to another within the table by use of the arrow-keys. */

#include <ctype.h>
#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "id.h"
#include "tabchain.h"

#define isSpace(scan, key) ((scan==KEY_SPACE)&(key==' '))
#define isTab(scan, key) ((scan==KEY_TAB) && !(key_shifts&KB_SHIFT_FLAG))
#define isBackTab(scan, key) ((scan==KEY_TAB) && (key_shifts&KB_SHIFT_FLAG))
#define isSingle(tc)  (tc->tf == tc->sfocus)

typedef struct t_tabchain {
   struct t_object *tf;         /* pointer to the chained object, may be a
                                   node */
   struct t_object *sfocus;     /* pointer to the object within a node that
                                   has focus */
   struct t_tabchain *next;
   struct t_tabchain *prev;
   struct t_window *win;
} t_tabchain;

/* These tables must replace the default ones when an object joins the chain
   (this is carried out by JoinTabChain) */
t_tcfun default_master_tc_functions;
t_tcfun default_single_tc_functions;
t_tcfun default_slave_tc_functions;

int insert_tab_links_after_focus = 0;

/* Some stubs to be for the default behaviour */
static int MoveFocusToNextSubObjectStub(t_object *b nouse, t_object *f nouse,
                         int scan nouse, int ascii nouse)
{
   return 0;
}
static int SFocusStub(t_object *sf nouse)
{
   return 1;
}
static void NotifyInactivationStub(t_tabchain *tc nouse, t_object *b nouse)
{
}
static void SetFocusShadowedStub(t_object *b nouse, int on nouse)
{
}

static int SingleMoveFocusTo(t_object *b);

t_tcfun default_tabchain_functions = {
   MoveFocusToNextSubObjectStub,
   SFocusStub,
   StubOb,
   StubOb,
   /* This one is common for all objects, even those  that don't joins the
      chain. The point is that e.g. at least the blinking cursor in an
      edit-box should stop when selecting any object that is selectable. */
   SingleMoveFocusTo,
   NotifyInactivationStub,
   SetFocusShadowedStub
};

/* The Focus and Unfocus functions represent the behavior of a single object
   when taking/leaving sub-focus. These functions gets the default behaviour.
   A specific type of object may create its own t_tcfun-table to achieve a
   different behaviour. */
static int TabChainSetFocus(t_object *sf)
{
   sf->hasfocus = 1;
   sf->tf->SetFocus(sf);
   return 1;
}

static void UnSetFocus(t_object *b)
{
   b->hasfocus = 0;
   b->tf->UnSetFocus(b);
}

static int NodeSetFocus(t_object *b)
{
   if (b->node && b->tablink && b->tablink->sfocus && b->tablink->sfocus != b) {
      b->tablink->sfocus->tf->SetFocus(b->tablink->sfocus);
      return b->tablink->sfocus->tcfun->SetFocus(b->tablink->sfocus);
   }
   return 0;
}

/* The behaviour for a chained node-object when leaving tab-focus is to
   transfer the command to the single object currently in sub-focus. */
static void NodeUnFocus(t_object *b)
{
   if (b->node && b->tablink && b->tablink->sfocus && b->tablink->sfocus != b) {
      b->tablink->sfocus->tf->UnSetFocus(b->tablink->sfocus);
      b->tablink->sfocus->tcfun->UnSetFocus(b->tablink->sfocus);
   }
}

static void LeaveTabChain(t_object *b)
{
   b->tablink = NULL;
}

/* A node that possibly was joined to the chain will restore its state to
   "not in chain", which also will be applied to its children */
static void NodeLeaveTabChain(t_object *nb)
{
   t_object *b;

   if (nb->node == NULL)
      return;
   for (b = nb->node->firstob; b; b = b->next)
       b->tablink = NULL;
   nb->tablink = NULL;
}

static int BeTabFocus(t_object *b)
{
   t_tabchain *tc;

   tc = b->parent->win->tc;
   if (tc == NULL)
      return 0;
   do {
      if (tc->tf == b) {
         b->parent->win->tc = tc;
         return 1;
      }
      tc = tc->next;
   } while (tc != b->parent->win->tc);
   return 0;
}

/* Takes focus from the current focused object and gives it to the single
   object `sf' which is the sub-object of a node that has joined the tab-
   chain. The process includes re-drawing. */
extern int SlaveMoveFocusTo(t_object *sf)
{
   t_object *prevtf, *prevsf, *tf;

   if (sf->parent == NULL || sf->tablink == NULL || sf->parent->win == NULL) {
      return 0;
   }
   tf = sf->parent->ob;
   /* Ask the tabchain who is currently in focus */
   GetTabChainFocus(sf->parent->win, &prevtf, &prevsf);
   /* ... and tell it that it is no longer focused */
   if (prevtf && prevtf != tf) {
      prevtf->tcfun->UnSetFocus(prevtf);
      prevtf->tf->Refresh(prevtf);
      /* Tell the tab-chain who is now in focus */
      if (BeTabFocus(tf) == 0) {
         return 0;
      }
   } else if (prevsf) {
      prevsf->tcfun->UnSetFocus(prevsf);
   }
   if (sf->tablink) {
      sf->tablink->sfocus = sf;
   }
   sf->tcfun->SetFocus(sf);
   /* Show the changes. */
   if (prevtf == tf) {
      tf->tf->Refresh(tf);
   } else {
      sf->tf->Refresh(sf);
   }
   return 1;
}

/* Takes focus from the current focused object and gives it to the single
   object `b' The process includes re-drawing. */
static int SingleMoveFocusTo(t_object *b)
{
   t_object *prevtf, *prevsf;

   if (b->parent == NULL || b->parent->win == NULL) {
      return 0;
   }
   /* Ask the tabchain who is currently in focus */
   GetTabChainFocus(b->parent->win, &prevtf, &prevsf);
   /* ... and tell it that it is no longer focused */
   if (b != prevtf && prevtf) {
      prevtf->tcfun->UnSetFocus(prevtf);
      prevtf->tf->Refresh(prevtf);
   }
   /* Tell the tab-chain who is now in focus */
   BeTabFocus(b);
   b->tcfun->SetFocus(b);
   /* Show the changes. */
   b->tf->Refresh(b);
   return 1;
}

/* Takes focus from the current focused object and gives it to the node.
   If there is no `actual object' in focuse: take the first on.
   The process includes re-drawing. */
static int NodeMoveFocusTo(t_object *b)
{
   t_object *prevtf, *prevsf, *sf;
   t_window *w;

   if (b->node == NULL || b->tablink == NULL || b->node->win == NULL)
      return 0;

   w = b->node->win;
   /* Ask the tabchain who is currently in focus */
   GetTabChainFocus(w, &prevtf, &prevsf);
   /* ... and tell it that it is no longer focused */
   if (prevtf && prevtf != b) {
      prevtf->tcfun->UnSetFocus(prevtf);
      prevtf->tf->Refresh(prevtf);
      /* Tell the tab-chain who is now in focus */
      if (BeTabFocus(b) == 0)
         return 0;
   }
   sf = w->tc->sfocus;
   if (sf == NULL || sf->inactive) {
      for (; sf; sf = sf->next)
         if (!sf->inactive)
            break;
      if (sf == NULL)
         for (sf = b->node->firstob; sf; sf = sf->next)
            if (!sf->inactive)
               break;
   }
   w->tc->sfocus = sf;
   /* Tell the sub-object that it has got the focus. */
   if (sf)
      sf->tcfun->SetFocus(sf);
   /* Show the changes. */
   b->tf->Refresh(b);
   return 1;
}

/* Here are the default handlers for the action to be done when user presses
   the arrow-keys when a node in the tab-chain is in focus. The function will
   seach for a new object within the same node to get the focus. Input is the
   node and the current focused object. The subfocus will be moved in desired
   direction. The functions only searches for active objects with exactely
   the same coordinates in the direction perpendicular to the moving
   direction. If come to the end of the objects in that direction it will
   wrap to the other side but still the same direction. If no other object in
   the same line of direction it will return NULL to indicate this (it should
   work well to return the current focus) */
static t_object *ArrowDownFocus(t_object *nf, t_object *f)
{
   t_object *b;
   t_node *nd;
   int x1, y1;

   if (f) {
      x1 = f->x1;
      y1 = f->y1;
   } else {
      x1 = 0;
      y1 = 0;
   }
   nd = nf->node;
   if (f) {
      for (b = f->next; b; b = b->next)
         if (!b->inactive && b->x1 == x1 && b->y1 > y1)
            return b;
   }
   if (nd) {
      for (b = nd->firstob; b != f; b = b->next)
         if (!b->inactive && (b->x1 == x1 || f == NULL) && b->y1 < y1)
            return b;
   }
   return f;
}

static t_object *ArrowUpFocus(t_object *nf, t_object *f)
{
   t_object *b;
   t_node *nd;
   int x1, y1;

   if (f) {
      x1 = f->x1;
      y1 = f->y1;
   } else {
      x1 = 0;
      y1 = 0;
   }
   nd = nf->node;
   if (f) {
      for (b = f->prev; b; b = b->prev)
         if (!b->inactive && b->x1 == x1 && b->y1 < y1)
            return b;
   }
   if (nd) {
      for (b = nd->last; b != f; b = b->prev)
         if (!b->inactive && (b->x1 == x1 || f == NULL) && b->y1 > y1)
            return b;
   }
   return f;
}

static t_object *ArrowLeftFocus(t_object *nf, t_object *sf)
{
   t_object *b, *newbw = sf, *newbl = NULL;
   t_node *nd;
   int maxx, maxxleft, x1, y1;

   nd = nf->node;
   if (nd) {
      if (sf) {
         x1 = sf->x1;
         y1 = sf->y1;
      } else {
         x1 = 1000000;
         y1 = 0;
      }
      maxx = maxxleft = 0;
      for (b = nd->firstob; b; b = b->next) {
         if ((b->y1 == y1 || sf == NULL) && !b->inactive && !b->exclude_add) {
            if (b->x1 < x1 && b->x1 >= maxxleft) {
               maxxleft = b->x1;
               newbl = b;
            }
            if (b->x1 > maxx) { /* select object in case of wrapping */
               maxx = b->x1;
               newbw = b;
            }
         }
      }
   }
   if (newbl)
      return newbl;
   return newbw;
}

static t_object *ArrowRightFocus(t_object *nf, t_object *sf)
{
   t_object *b, *newbw = sf, *newbr = NULL;
   t_node *nd;
   int minx, minxright, x1, y1;

   nd = nf->node;
   if (nd) {
      if (sf) {
         x1 = sf->x1;
         y1 = sf->y1;
      } else {
         x1 = 0;
         y1 = 0;
      }
      minx = minxright = nf->x2;
      for (b = nd->firstob; b; b = b->next) {
         if ((b->y1 == y1 || sf == NULL) && !b->inactive && !b->exclude_add) {
            if (b->x1 > x1 && b->x1 < minxright) {
               minxright = b->x1;
               newbr = b;
            }
            if (b->x1 < minx) { /* select object in case of wrapping */
               minx = b->x1;
               newbw = b;
            }
         }
      }
   }
   if (newbr)
      return newbr;
   return newbw;
}

/* Creates a new link in the tab chain and completes references from the
   tab-chain struct to the objects(s). The first link in a window will be
   the one in focus at start-up. This may be overridden by the application */
static t_tabchain *DoJoinTabChain(t_object *tf, t_object *sf)
{
   t_tabchain *tc;
   t_window *w;

   if (tf->parent == NULL || tf->parent->win == NULL)
      return NULL;
   w = tf->parent->win;
   if (tf->tablink)
      UnlinkFromTabChain(w, tf->tablink, tf);
   tc = GetMem0(t_tabchain, 1);
   tc->win = w;
   if (w->tc) {                 /* not first: insert last/first in the circle. */
      if (insert_tab_links_after_focus) {
         w->tc->next->prev = tc;
         tc->next = w->tc->next;
         w->tc->next = tc;
         tc->prev = w->tc;
      } else {
         w->tc->prev->next = tc;
         tc->prev = w->tc->prev;
         w->tc->prev = tc;
         tc->next = w->tc;
      }
      if (w->tc->sfocus && w->tc->sfocus->inactive) {
         w->tc = tc;
      } else if (w->tc->tf && w->tc->tf->inactive) {
         w->tc = tc;
      }
   } else {                     /* the first one in this window create circle
                                   and set start and focus */
      w->tc = tc->next = tc->prev = tc;
   }
   tc->tf = tf;               /* Makes a permanent link to the object */
   tc->sfocus = sf;           /* This may change during the lifetime of this
                                 tab-link in case the `tf' is a node,
                                 otherwise not */
   return tc;
}

/* This function sets a single object to be in the tab-chain */
extern int SingleDoJoinTabChain(t_object *b)
{
   b->tcfun = &default_single_tc_functions;
   b->tablink = DoJoinTabChain(b, b);
   return 1;
}

/* This function sets a node to be in the tab-chain, all its children (only
   at the first level, which therefore must be leafs in the tree) will be
   properly set-up */
extern int NodeDoJoinTabChain(t_object *b)
{
   t_node *nd;
   t_tabchain *tc;
   t_object *sf;

   nd = GetNode(b->id);
   if (nd) {
      b->tcfun = &default_master_tc_functions;
      for (sf = nd->firstob; sf; sf = sf->next)
         if (!sf->inactive)
            break;
      tc = DoJoinTabChain(nd->ob, sf);
      nd->ob->tablink = tc;
      for (b = nd->firstob; b; b = b->next) {
         if (b->tablink)
            UnlinkFromTabChain(nd->win, b->tablink, b);
         b->tcfun = &default_slave_tc_functions;
         b->tablink = tc;
         if (tc->sfocus == NULL && !b->inactive) {
            SetSubFocusOfLink(b->tablink, b);
            if (tc == nd->win->tc)
               b->hasfocus = 1;
         }
      }
      return 1;
   }
   return 0;
}

extern void InitializeFocus(t_tabchain *tc)
{
   if (tc) {
      if (tc->sfocus && !tc->sfocus->inactive)
         tc->sfocus->tcfun->SetFocus(tc->sfocus);
   }
}

/* Removes and destroys the link `link' from the tab-chain. The concerned
   object will be notified. It will search through the chain to ensure that
   the link is really in it. */
extern void UnlinkFromTabChain(t_window *w, t_tabchain *link, t_object *sf)
{
   if (sf != link->tf) {
      if (sf == link->sfocus) {
         link->sfocus->tcfun->LeaveTabChain(link->sfocus);
         link->sfocus = NULL;
      }
      return;
   }
   if (w->tc == link) {
      if (link == link->next)
         w->tc = NULL;
      else
         w->tc = link->next;
   }
   link->prev->next = link->next;
   link->next->prev = link->prev;
   if (link->tf)
      link->tf->tcfun->LeaveTabChain(link->tf);
   Release(link);
}

extern int MoveFocusToNextSubObject(t_object *nf, t_object *sf, int scan, int ascii)
{
   t_object *newsf = NULL;      /* new sub focus */
   (void)ascii;

   switch (scan) {
   case KEY_DOWN:
      if (nf) {
         newsf = ArrowDownFocus(nf, sf);
      }
      break;
   case KEY_LEFT:
      if (nf) {
         newsf = ArrowLeftFocus(nf, sf);
      }
      break;
   case KEY_UP:
      if (nf) {
         newsf = ArrowUpFocus(nf, sf);
      }
      break;
   case KEY_RIGHT:
      if (nf) {
         newsf = ArrowRightFocus(nf, sf);
      }
      break;
   default:
      return 0;
   }
   if (newsf) {
      newsf->tcfun->MoveFocusTo(newsf);
   }
   return 1;
}

/* Flushes the entire tab-chain. It is not really needed since
   `UnlinkFromTabChain' will do the same. However the latter requires O(n*n)
   for searching. */
extern void RemoveTabChain(t_window *w)
{
   t_tabchain *tc, *next;

   tc = w->tc;
   if (tc) {
      do {
         next = tc->next;
         if (tc->tf)
            tc->tf->tcfun->LeaveTabChain(tc->tf);
         Release(tc);
         tc = next;
      } while (tc != w->tc);
      w->tc = NULL;
   }
}

extern struct t_object *SubFocusOfLink(t_tabchain *tc)
{
   return tc->sfocus;
}

extern void SetSubFocusOfLink(t_tabchain *tc, t_object *sf)
{
   if (tc)
      tc->sfocus = sf;
}
/*
extern t_tabchain *SetLinkInFocus(t_tabchain *tc)
{
   t_tabchain *prevtc;

   if (tc) {
      prevtc = tc->win->tc;
      tc->win->tc = tc;
      if (tc->sfocus)
         tc->sfocus->tcfun->SetFocus(tc->sfocus);
      return prevtc;
   }
   return NULL;
}
*/
static void SetFocusShadowed(t_object *b, int on)
{
   if (b->hasfocus) {
      if (on)
         b->hasfocus = 2;
      else
         b->hasfocus = 1;
      b->tf->Refresh(b);
   }
}

static void NodeSetFocusShadowed(t_object *b, int on)
{
   b = SubFocusOfLink(b->tablink);
   if (b)
      b->tcfun->SetFocusShadowed(b, on);
}

extern void SetFocusHighlighted(t_tabchain *tc)
{
   if (tc) {
      tc->tf->tcfun->SetFocusShadowed(tc->tf, 0);
//      tc->tf->tf->SetFocus(tc->tf);
   }
}

extern void SetFocusDiscrete(t_tabchain *tc)
{
   if (tc) {
      tc->tf->tcfun->SetFocusShadowed(tc->tf, 1);
//      tc->tf->tf->UnSetFocus(tc->tf);
   }
}

/* This function tells which node and single object (the focused object
   within the focused node) that are currently in focus. If the object that
   joined the tab-chain is not a node (it is a single object like an
   edit-box) the "node" parameter will rather point to that single object
   (same value as single). */
extern void GetTabChainFocus(t_window *w, t_object **tabobj, t_object **slave)
{
   if (w->tc) {
      *tabobj = w->tc->tf;
      *slave = w->tc->sfocus;
   } else {
      *tabobj = NULL;
      *slave = NULL;
   }
}

/* Returns a pointer to the object that is in subfocus of the node `nd'. This may be NULL. */
extern t_object *GetSubFocus(t_node *nd)
{
   t_tabchain *tc;
   tc = nd->win->tc;
   if (tc) {
      do {
         if (tc->tf == nd->ob) {
            return tc->sfocus;
         }
         tc = tc->next;
      } while (tc != nd->win->tc);
   }
   return NULL;
}

static void NotifyInactivation(t_tabchain *tc, t_object *b)
{
   t_tabchain *ftc;

   ftc = tc->win->tc;
   if (tc == ftc) {
      for (tc = tc->next; tc != ftc; tc = tc->next)
         if (tc->tf && !tc->tf->inactive)
            break;
      if (tc != ftc) {
         tc->tf->tcfun->MoveFocusTo(tc->tf);
         return;
      }
   }
   b->tcfun->UnSetFocus(b);
}

static void SlaveNotifyInactivation(t_tabchain *tc, t_object *b)
{
   if (tc->sfocus == b)
      tc->sfocus = NULL;
   b->tcfun->UnSetFocus(b);
}

extern int TabChainNotifyKeyPress(t_window *w, int scan, int ascii)
{
   t_tabchain *tc;
   int ok = 0;

   tc = w->tc;
   if (tc) {
      if (isTab(scan, ascii) || isBackTab(scan, ascii)) {
         if (isTab(scan, ascii)) {
            for (tc = tc->next; tc != w->tc; tc = tc->next) {
               if (tc->tf && !tc->tf->inactive) {
                  break;
               }
            }
         } else {
            for (tc = tc->prev; tc != w->tc; tc = tc->prev) {
               if (tc->tf && !tc->tf->inactive) {
                  break;
               }
            }
         }
         if (tc->tf) {
            tc->tf->tcfun->MoveFocusTo(tc->tf);
         }
         ok = 1;
      } else if (isEnter(scan, ascii) || isCtrlEnter(scan, ascii) || isSpace(scan, ascii)) {
         if (tc->sfocus && tc->sfocus->hasfocus && !tc->sfocus->inactive) {
            if (isCtrlEnter(scan, ascii)) {
               tc->sfocus->usedbutton = RIGHT_MOUSE;
            } else {
               tc->sfocus->usedbutton = LEFT_MOUSE;
            }
            tc->sfocus->x = tc->sfocus->x2 - 1;
            tc->sfocus->y = tc->sfocus->y2 - 1;
            tc->sfocus->Action(tc->sfocus->appdata);
         }
         ok = 1;
      } else {
         ok = tc->tf->tcfun->MoveFocusToNextSubObject(tc->tf, tc->sfocus, scan, ascii);
      }
   }
   return ok;
}

extern void NotifyTabChainWindowIsFinished(t_tabchain *tc)
{
   if (tc)
      tc->tf->tcfun->SetFocus(tc->tf);
}

extern void InitTabChain(void)
{
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      default_single_tc_functions = default_tabchain_functions;
      default_single_tc_functions.LeaveTabChain = LeaveTabChain;
      default_single_tc_functions.MoveFocusTo = SingleMoveFocusTo;
      default_single_tc_functions.UnSetFocus = UnSetFocus;
      default_single_tc_functions.SetFocus = TabChainSetFocus;
      default_single_tc_functions.NotifyInactivation = NotifyInactivation;
      default_single_tc_functions.SetFocusShadowed = SetFocusShadowed;

      default_slave_tc_functions = default_tabchain_functions;
      default_slave_tc_functions.LeaveTabChain = LeaveTabChain;
      default_slave_tc_functions.MoveFocusTo = SlaveMoveFocusTo;
      default_slave_tc_functions.UnSetFocus = UnSetFocus;
      default_slave_tc_functions.SetFocus = TabChainSetFocus;
      default_slave_tc_functions.NotifyInactivation = SlaveNotifyInactivation;
      default_slave_tc_functions.SetFocusShadowed = SetFocusShadowed;

      default_master_tc_functions = default_tabchain_functions;
      default_master_tc_functions.UnSetFocus = NodeUnFocus;
      default_master_tc_functions.LeaveTabChain = NodeLeaveTabChain;
      default_master_tc_functions.MoveFocusToNextSubObject =  MoveFocusToNextSubObject;
      default_master_tc_functions.SetFocus = NodeSetFocus;
      default_master_tc_functions.MoveFocusTo = NodeMoveFocusTo;
      default_master_tc_functions.NotifyInactivation = NotifyInactivation;
      default_master_tc_functions.SetFocusShadowed = NodeSetFocusShadowed;
   }
}

/* Application interface: */

extern int GetCurrentFocus(int id)
{
   t_window *w;
   t_object *b, *n;

   if (id == 0)
      return _VisableWindow();
   w = fwin->win;
   GetTabChainFocus(w, &n, &b);
   if (n)
      return n->id;
   return -1;
}

extern int JoinTabChain(int id)
{
   t_object *b;

   b = GetObject(id);
   if (b)
      return b->tf->DoJoinTabChain(b);
   return 0;
}

extern int SetFocusOn(int id)
{
   t_object *b;

   b = GetObject(id);
   if (b) {
      return b->tcfun->MoveFocusTo(b);
   }
   return 0;
}
/* Obsolete functions, not exported in the header. */
extern int SetVisibleWindow(int winid)
{  return SetFocusOn(winid);}
extern int FocusOn(int id)
{  return SetFocusOn(id);}
extern int SetSubFocus(int tid, int sid)
{  if (sid)
      return SetFocusOn(sid);
   else
      return SetFocusOn(tid);}
extern int SetTabFocus(int id)
{ return SetFocusOn(id);}
extern int SimpleJoinTabChain(int id)
{ return JoinTabChain(id);}
extern int NodeJoinTabChain(int tid, int sid nouse)
{ return JoinTabChain(tid);}
