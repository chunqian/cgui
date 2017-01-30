/* Module OBSIZER.C
   This file contains the function making an object re-sizeable. The default
   implementation for simple objects, and the application interface are here.
   The default implementation for nodes is different */

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "id.h"
#include "cursor.h"
#include "mouse.h"
#include "obsizer.h"

int continous_update_resize = 1;

extern void FreeResizer(void *data)
{
   t_stretch *st = data;

   Release(st);
}

static int ReSizeObject(int x, int y, void *data, int id nouse, int reason)
{
   t_stretch *st = data;
   t_object *b;
   t_node *nd;
   static int oldx, oldy;
   int minx, miny, maxx, maxy;
   t_object *w;

   b = st->b;
   nd = b->parent;
   switch (reason) {
   case SL_OVER:
      if ((st->options & (NO_HORIZONTAL|NO_VERTICAL)) == 0 && b->x > b->x2 - 3
          && b->y > b->y2 - 3) {
         SelectCursor(CURS_CROSS);
         st->dir = VERTICAL | HORIZONTAL;
      } else if ((st->options & NO_VERTICAL) == 0 && b->y > b->y2 - 3) {
         SelectCursor(CURS_DRAG_V);
         st->dir = VERTICAL;
      } else if ((st->options & NO_HORIZONTAL) == 0 && b->x > b->x2 - 3) {
         SelectCursor(CURS_DRAG_H);
         st->dir = HORIZONTAL;
      } else
         return 0;
      break;
   case SL_OVER_END:
      SelectCursor(CURS_DEFAULT);
      break;
   case SL_STARTED:
      oldx = x;
      oldy = y;
      w = b->parent->win->node->ob;
      /* find out if the object is contianed (recursively) in a fix-sized
         node, and if so set mouse limit */
      for (maxx = SCREEN_W, maxy = SCREEN_H, nd = b->parent;
           nd != w->parent; nd = nd->ob->parent) {
         if (nd->fixsize) {
            if (nd == w->node) {
               /* if reached the actual window, so the coordinates is already
                  expressed in screen coordinates */
               maxx = nd->ob->x2;
               maxy = nd->ob->y2;
            } else {
               maxx = nd->ob->x2 + nd->ob->parent->wx + w->x1;
               maxy = nd->ob->y2 + nd->ob->parent->wy + w->y1;
            }
         }
      }
      minx = b->x1 + b->parent->wx + w->x1 + 1;
      miny = b->y1 + b->parent->wy + w->y1 + 1;
      _CguiSetRange(minx, miny, maxx - 2 - minx, maxy - 2 - miny);
      break;
   case SL_PROGRESS:
      if ((st->dir & HORIZONTAL) == 0)
         x = oldx;
      if ((st->dir & VERTICAL) == 0)
         y = oldy;
      b->tf->Stretch(b, x - oldx, y - oldy);
      oldx = x;
      oldy = y;
      if (continous_update_resize) {
         ReMakeWindow(fwin);
      }
      break;
   case SL_STOPPED:
      _CguiSetRange(0, 0, SCREEN_W, SCREEN_H);
      if (st->Notify)
         st->Notify(st->data);
      ReMakeWindow(fwin);
      LaunchMouseEvent(NULL);
      break;
   }
   return 1;
}

extern int MakeObjectStretchable(t_object *b, void (*Notify) (void *),
                                 void *data, int options)
{
   t_stretch *st;

   b = b->tf->GetMainNode(b);
   st = GetMem(t_stretch, 1);
   st->Notify = Notify;
   st->data = data;
   st->options = options;
   st->b = b;
   b->ReSize = ReSizeObject;
   b->resizedata = st;
   b->resize = LEFT_MOUSE;
   if ((options & NO_HORIZONTAL) == 0)
      b->resize |= 0x80;
   if ((options & NO_VERTICAL) == 0)
      b->resize |= 0x40;
   HookExit(b->id, FreeResizer, st);
   return b->id;
}

/* Application interface: */

extern int MakeStretchable(int id, void (*Notify) (void *), void *data,
                           int options)
{
   t_object *b;

   b = GetObject(id);
   if (b)
      return b->tf->MakeStretchable(b, Notify, data, options);
   return 0;
}
