/* Module MOUSE
   Contains the mouse driver interface routines, including the
   application-defined event-handler */

#include <allegro.h>
#if ALLEGRO_VERSION == 3 && ALLEGRO_SUB_VERSION == 12
#include <src/internal.h>
#else
#include <allegro/internal/aintern.h>
#endif
#include <string.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "statelog.h"

#include "msm.h"
#include "window.h"
#include "node.h"
#include "event.h"
#include "cursor.h"
#include "mouse.h"

static int mouse_blocked;
static int mouse_pre_installed;
static BITMAP *old_mouse_sprite;
static BITMAP *old_mouse_screen;
static void (*MouseInputHandler)(int *x, int *y, int *z, int *buttons);
static void (*ForcePosHandler)(int x, int y);
static void (*RangeHandler)(int x, int y, int w, int h);

/* cgui-internal globals */
t_node *_mouse_touched_win1, *_mouse_touched_win2;
int _previous_mouse_button;

/* exported variable */
int cgui_mouse_draw_in_interrupt;

static void DefaultMouseInput(int *x, int *y, int *z, int *buttons)
{
   *x = mouse_x;
   *y = mouse_y;
   *z = mouse_z;
   *buttons = mouse_b;
}

static void DefaultForcePos(int x, int y)
{
   position_mouse(x, y);
}

static void DefaultSetRange(int x, int y, int w, int h)
{
   set_mouse_range(x, y, x + w - 1, y + h - 1);
}

static void HandleMouseEvent(void *data nouse)
{
   static t_mousedata localdata;
   t_mousedata *m;
   int curcurs, stop = 0;
   t_object *b;
   t_node *nd;

   m = &localdata;
   MouseInputHandler(&m->x, &m->y, &m->z, &m->mb);
   if (MouseInputHandler != DefaultMouseInput) {
      DefaultForcePos(m->x, m->y);
   }
   curcurs = m->requestcursor = GetSelectedCursorNo();
   if (m->mb) {
      if (_previous_mouse_button) {
         m->mb &= _previous_mouse_button; /* don't accept any additional mouse buttons */
      } else {
         _previous_mouse_button = m->mb;    /* initialize */
      }
   }

   /* Find the uppermost window that is under the mouse */
   for (b = FirstWin(); b; b = NextWin(b)) {
      if (isOver(m->x, m->y, b)) {
         if (!WinIsFree(b))
            b = NULL;
         break;
      }
   }
   /* If an object is already in a non-init state (that is: it took the
      mouse-event last time there was one), then it must be called directly
      (before passing the event to the tree search).
      This is necessary because it must get the chance to return to its
      init-state before asking the tree for some other recipient of the event.
      It may also promptly say that it don't want the event to be passed on to
      any other (by returning 1). However it can keep a non-init state and
      return 0 (possibly yielding in another object getting the focus), this
      will be the case if e.g. a drag-and-drop is in progress.
      This is why we call "CurMouseFocus". We pass it the current window beeing
      under the mouse (which is not necessarily the window of the object). */
   if (_mouse_touched_win1 || _mouse_touched_win2) {
      STATE_LOGG_REASON("pre");
      if (b)
         nd = b->node;
      else
         nd = NULL;
      if (_mouse_touched_win1) {
         stop = _mouse_touched_win1->ob->me->CurMouseFocus(m, nd);
      } else {
         stop = _mouse_touched_win2->ob->me->CurMouseFocus(m, nd);
      }
      if (b) {
         /* The above call may wrap some action resulting in that
            b will be closed. Therefore we need to make a new search. */
         for (b = FirstWin(); b; b = NextWin(b)) {
            if (isOver(m->x, m->y, b)) {
               if (!WinIsFree(b))
                  b = NULL;
               break;
            }
         }
      }
   }
   STATE_LOGG_REASON("sea");
   if (b) {
      if (!stop && _win_root)
         b->me->MouseEv(m, b);     /* ordnary tree search for a recipient */
      else if (stop && b->node != fwin && m->mb)
         b->tcfun->MoveFocusTo(b);
   }

   if (GetSelectedCursorNo() == curcurs
      /* Application call-back did not change cursor */
       && GetSelectedCursorNo() != m->requestcursor) {
      SelectCursor(m->requestcursor);
   }
   if (!m->mb)
      _previous_mouse_button = 0;              /* reset */
}

static void PollMouse(void *data)
{
   _GenEventOfCgui(PollMouse, data, 10, 0);
   HandleMouseEvent(data);
}

extern int LaunchMouseEvent(void *data)
{
   return _GenEventOfCgui(HandleMouseEvent, data, 0, 0);
}

extern void _CguiSetRange(int x, int y, int w, int h)
{
   if (RangeHandler)
      RangeHandler(x, y, w, h);
}

extern void _CguiForceMousePos(int x, int y)
{
   if (ForcePosHandler)
      ForcePosHandler(x, y);
}

extern void _CguiMousePos(int *x, int *y, int *z)
{
   int mb;
   if (MouseInputHandler)
      MouseInputHandler(x, y, z, &mb);
}

/* This function moves the mouse cursor (or rather the hot-spot of it) to the
   specified (x,y) location. The coordinates shall be given in
   window-relative values */
extern void SetMousePos(int x, int y)
{
   _CguiForceMousePos(fwin->ob->x + x, fwin->ob->y + y);
}

static int mouse_inited = 0;
static void DeInitMouseInternal(void *data nouse)
{
   if (mouse_inited) {
      if (mouse_pre_installed) {  /* restore state */
         set_mouse_sprite(old_mouse_sprite);
         show_mouse(old_mouse_screen);
         old_mouse_sprite = NULL;
         old_mouse_screen = NULL;
         mouse_pre_installed = 0;
      } else
         remove_mouse();
      mouse_inited = 0;
   }
}

extern void InitMouse(void)
{
   int mouse_present;

#ifdef DJGPP
   extern BITMAP *mouse_sprite;
#else
   BITMAP *mouse_sprite = NULL;
#endif

   if (mouse_inited)
      return;

   mouse_pre_installed = _mouse_installed == -1;
   if (mouse_pre_installed) {
      old_mouse_sprite = mouse_sprite;
      old_mouse_screen = _mouse_screen;
      mouse_present = 1;
   } else {
      mouse_present = install_mouse() != -1;
   }
   if (mouse_present) {
      show_mouse(NULL);
   }
   HookCguiDeInit(DeInitMouseInternal, NULL);
   CguiSetMouseInput(NULL, NULL, NULL);
   RangeHandler(0, 0, SCREEN_W, SCREEN_H);
   ForcePosHandler(SCREEN_W/2, SCREEN_H/2);
   PollMouse(NULL);
   mouse_inited = 1;
}

/* Application interface. */

extern void CguiSetMouseInput(void (*MouseInput)(int *x, int *y, int *z, int *buttons), void (*ForcePos)(int x, int y), void (*SetRange)(int x, int y, int w, int h))
{
   if (MouseInput == NULL) {
      MouseInputHandler = DefaultMouseInput;
      ForcePosHandler = DefaultForcePos;
      RangeHandler = DefaultSetRange;
   } else {
      MouseInputHandler = MouseInput;
      ForcePosHandler = ForcePos;
      RangeHandler = SetRange;
   }
}
