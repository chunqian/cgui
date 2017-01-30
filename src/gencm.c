/* Module GENCM.C
   Generic "Catch mouse"-window.
   Contains the function for creating a mouse catching window. The window
   is invisible (it has no bitmap, it is just an virtual window "seen" by
   the handler). Such a window is needed if one want the mouse events (e.g.
   clicks) outside the current window to be caught for special processing
   (normally such events are transfered to the nearest window, so this one
   is just there to cover them all).
   This is the generic cathc-mouse-window. */

#include <allegro.h>
#include "cgui.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "gencm.h"
#include "mouse.h"
#include "msm.h"

#define MS_INIT            0
#define MS_OVERUP          1
#define MS_OVERDOWN        2

static int MSCatchInit(int x nouse, int y nouse, int over, int mb,
                       t_mousedata * m nouse, t_object * b)
{
   if (over) {
      if (mb)
         b->state = MS_OVERDOWN;
      else
         b->state = MS_OVERUP;
   }
   return 1;
}

static int MSCatchOverUp(int x nouse, int y nouse, int over, int mb,
                         t_mousedata * m nouse, t_object * b)
{
   if (over) {
      if (mb) {
         b->state = MS_INIT;
         b->Action(b->appdata);
         LaunchMouseEvent(m);
      }
   } else {
      b->state = MS_INIT;
   }
   return 1;
}

static int MSCatchOverDown(int x nouse, int y nouse, int over, int mb,
                           t_mousedata * m, t_object * b)
{
   if (over) {
      if (!mb) {
         b->state = MS_INIT;
         b->Action(b->appdata);
         LaunchMouseEvent(m);
      }
   } else {
      b->state = MS_INIT;
   }
   return 1;
}

/* Take a look later... ### winob == b??; why the p-loop?? */
static int CatchMouseEvMenu(t_mousedata *m, t_object *b)
{
   t_object *winob, *p;
   t_node *selwin;

   winob = b->node->win->node->ob;
   for (p = FirstFreeWin(), selwin = NULL; p; p = NextFreeWin(p)) {
      if (isOver(m->x, m->y, p)) {
         selwin = p->node;
         if (selwin && selwin != winob->node)
            b->me->StateHandlers[b->state] (0, 0, 0, 0, m, b); /* reset state
                                                                */
         break;
      }
   }
   if ((unsigned) b->state < MS_MAX) {
      /* Always over (by definition) */
      b->me->StateHandlers[b->state] (0, 0, 1, m->mb, m, b);
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
   me->MouseEv = CatchMouseEvMenu;
}

extern t_window *CatchMouseWindow(void (*Action) (void *), void *data)
{
   t_object *b;
   static int virgin = 1;
   static struct t_mevent me;
   static struct t_typefun tf;
   t_node *nd;

   ResetMouseState();
   nd = MakeWin(FILLSCREEN, "", W_NOMOVE);
   if (virgin) {
      virgin = 0;
      InitSM(&me);
      tf = default_window_type_functions;
      tf.MakeNodeRects = StubOb;
      tf.Refresh = StubOb;
      tf.Draw = StubOb;
   }
   b = nd->win->node->ob;
   b->x2 = SCREEN_W;
   b->y2 = SCREEN_H;
   b->me = &me;
   b->tf = &tf;
   b->Action = Action;
   b->appdata = data;
   b->exclude_add = 1;
   b->exclude_rect = 1;
   b->click = LEFT_MOUSE + RIGHT_MOUSE;
   WindowIsFinished();
   return nd->win;
}
