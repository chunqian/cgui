/* Module MENBARCM.C
   "Catch mouse"-window for menu bars.
   Contains the function for creating a mouse catching window. The window
   is invisible (it has no bitmap, it is just an virtual window "seen" by
   the handler). Such a window is needed if one want the mouse events (e.g.
   clicks) outside the current window to be caught for special processing
   (normally such events are transfered to the nearest window, so this one
   is just there to cover them all).
   This one is specially desingated for a menu bar. */

#include <allegro.h>
#include "cgui.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "menbarcm.h"
#include "menuitem.h"
#include "menu.h"
#include "mouse.h"
#include "msm.h"

#define MS_INIT            0
#define MS_OVERUP          1
#define MS_OVERDOWN        2
#define MS_OVER_MENU_UP    3
#define MS_OVER_MENU_DOWN  4
#define MS_OVER_MENU_CLICK 5

static int overmenuitem;

static int MSCatchInit(int x nouse, int y nouse, int over, int mb,
                       t_mousedata *m nouse, t_object* b)
{
   if (overmenuitem) {
      b->state = MS_OVER;
      b->Over(b);
      if (mb)
         b->state = MS_OVER_MENU_DOWN;
      else
         b->state = MS_OVER_MENU_UP;
   } else if (over) {
      b->state = MS_OVER;
      b->Over(b);
      if (mb)
         b->state = MS_OVERDOWN;
      else
         b->state = MS_OVERUP;
   }
   return 1;
}

static int MSCatchOverUp(int x nouse, int y nouse, int over, int mb,
                         t_mousedata *m nouse, t_object *b)
{
   if (overmenuitem) {
      b->state = MS_OVER_MENU_UP;
   } else if (over) {
      if (mb) {
         b->state = MS_INIT;
         b->Over(b);
         b->Action(b->appdata);
         LaunchMouseEvent(m);
      }
   } else {
      b->state = MS_INIT;
      b->Over(b);
   }
   return 1;
}

static int MSCatchOverDown(int x nouse, int y nouse, int over, int mb,
                           t_mousedata *m, t_object *b)
{
   if (overmenuitem) {
      b->state = MS_OVER_MENU_DOWN;
   } else if (over) {
      if (!mb) {
         b->state = MS_INIT;
         b->Over(b);
         b->Action(b->appdata);
         LaunchMouseEvent(m);
      }
   } else {
      b->state = MS_INIT;
      b->Over(b);
   }
   return 1;
}

static int MSCatchOverMenuUp(int x nouse, int y nouse, int over, int mb,
                             t_mousedata *m nouse, t_object *b)
{
   if (overmenuitem) {
      if (mb) {
         b->state = MS_OVER_MENU_CLICK;
      }
   } else if (over) {
      b->state = MS_OVERUP;
   } else {
      b->state = MS_INIT;
      b->Over(b);
   }
   return 1;
}

static int MSCatchOverMenuDown(int x nouse, int y nouse, int over, int mb,
                               t_mousedata *m nouse, t_object *b)
{
   if (overmenuitem) {
      if (!mb) {
         b->state = MS_OVER_MENU_UP;
      }
   } else if (over) {
      b->state = MS_OVERDOWN;
   } else {
      b->state = MS_INIT;
      b->Over(b);
   }
   return 1;
}

static int MSCatchOverMenuClick(int x nouse, int y nouse, int over, int mb,
                                t_mousedata *m, t_object *b)
{
   if (overmenuitem) {
      if (!mb) {
         b->state = MS_INIT;
         b->Over(b);
         b->Action(b->appdata);
         LaunchMouseEvent(m);
      }
   } else if (over) {
      b->state = MS_OVER;
   } else {
      b->state = MS_INIT;
      b->Over(b);
   }
   return 1;
}

/* Same in mencm.c  ### winob == b??; why the p-loop?? */
static int CatchMouseEvMenu(t_mousedata *m, t_object *b)
{
   t_object *winob, *p;
   t_node *minode;
   int overmenubar, x, y;
   t_object *mib;

   /* Appdata is the object of the menu-item in the menu-bar */
   mib = b->appdata;
   minode = mib->parent;
   winob = minode->win->node->ob;
   x = m->x - winob->x1 - minode->wx;
   y = m->y - winob->y1 - minode->wy;
   overmenubar = x > 0 && y > 0 && x < minode->ob->x2 - minode->ob->x1 &&
                                   y < minode->ob->y2 - minode->ob->y1;
   overmenuitem = x >= mib->x1 && y >= mib->y1 &&
                  x <= mib->x2 && y <= mib->y2;
   if (overmenubar && !overmenuitem) {
      b->me->StateHandlers[b->state] (0, 0, 0, 0, m, b); /* reset state */
      b->Action(b->appdata);
      LaunchMouseEvent(m);
      return 1;
   }
   for (p = b; p; p = NextFreeWin(p)) {
      if (isOver(m->x, m->y, p)) {
         if (IsWindow(p) && p != b)
            b->me->StateHandlers[b->state] (0, 0, 0, 0, m, b); /* reset state */
         break;
      }
   }
   if ((unsigned) b->state < MS_MAX) {
      b->me->StateHandlers[b->state] (0, 0, 1, m->mb, m, b); /* always over */
   } else {
      ResetState(m, b);
   }
   return 1;
}

static void InitSM(struct t_mevent *me)
{
   SetDefaultStateMachine(me);
   me->StateHandlers[MS_INIT] = MSCatchInit;
   me->StateHandlers[MS_OVERUP] = MSCatchOverUp;
   me->StateHandlers[MS_OVERDOWN] = MSCatchOverDown;
   me->StateHandlers[MS_OVER_MENU_UP] = MSCatchOverMenuUp;
   me->StateHandlers[MS_OVER_MENU_DOWN] = MSCatchOverMenuDown;
   me->StateHandlers[MS_OVER_MENU_CLICK] = MSCatchOverMenuClick;
   me->MouseEv = CatchMouseEvMenu;
}

extern t_window *MenuBarCatchMouse(void (*Action) (void *), void *data)
{
   t_object *b;
   static int virgin = 1;
   static struct t_mevent me;
   static struct t_tcfun tc;
   t_node *nd;
   static struct t_typefun tf;

   ResetMouseState();
   if (virgin) {
      tf = default_window_type_functions;
      tf.MakeNodeRects = StubOb;
      tf.Refresh = StubOb;
      tf.Draw = StubOb;
      virgin = 0;
      InitSM(&me);
      tc = default_window_tc_functions;
      tc.SetFocus = default_tabchain_functions.SetFocus;
   }
   nd = MakeWin(FILLSCREEN, "", W_NOMOVE);
   b = nd->win->node->ob;
   b->x2 = SCREEN_W;
   b->y2 = SCREEN_H;
   b->me = &me;
   b->tcfun = &tc;
   b->tf = &tf;
   b->Action = Action;
   b->appdata = data;
   b->exclude_add = 1;
   b->exclude_rect = 1;
   b->click = LEFT_MOUSE + RIGHT_MOUSE;
   WindowIsFinished();
   return nd->win;
}
