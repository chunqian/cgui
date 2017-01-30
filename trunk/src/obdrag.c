/* Module OBDRAG.H
   This file contains the functions for creating an object that manages the
   dragging of a winodow */

#include <allegro.h>
#include "cgui.h"

#include "window.h"
#include "node.h"
#include "cursor.h"
#include "obdrag.h"

static void RedrawDraggingWindow(t_object * b, int diffx, int diffy)
{
   int x1, y1, x2, y2;

   x1 = b->x1;
   y1 = b->y1;
   x2 = b->x2;
   y2 = b->y2;
   b->x1 += diffx;
   b->y1 += diffy;
   b->x2 += diffx;
   b->y2 += diffy;
   x1 = MIN(x1, b->x1);
   y1 = MIN(y1, b->y1);
   x2 = MAX(x2, b->x2);
   y2 = MAX(y2, b->y2);
   RefreshScreen(x1 - 1, y1, x2 + 1, y2);
}

/* Will be called when the drag-object of a window is gripped.
   DD_SLIDING_STARTED will come when user pressed mouse button above
   drag-object continous DD_SLIDING will come when user moves mouse
   DD_SLIDING_STOPPED will come when user releases the mouse button x and y
   is in the node coordinate system NOTE: As soon as the object has got focus
   the coordinates of it will be kept, and it is assumed that the window
   keeps its coordinates, but that is not true for this function, so we must
   remember the origin coordinates and kompensate for the movements, each
   time a new movement is to be done. */
static int DragWindow(int x, int y, void *src, int id nouse, int reason nouse)
{
   static int oldx, oldy;
   t_object *b = src, *w;

   w = b->parent->win->node->ob;
   switch (reason) {
   case SL_STARTED:
      oldx = x;
      oldy = y;
      break;
   case SL_PROGRESS:
      if (x != oldx  || y != oldy) {
         RedrawDraggingWindow(w, x - oldx, y - oldy);
         oldx = x - (x - oldx);
         oldy = y - (y - oldy);
      }
      break;
   case SL_STOPPED:
      break;
   default:
      return 0;
   }
   return 1;
}

static void DraggerPosition(t_object * b)
{
   b->x2 = 0;
   b->y2 = 0;
}

static void DraggerRightPack(t_object * b)
{
   t_node *nd;

   nd = b->parent;
   b->x2 = nd->ob->x2 - nd->ob->x1;
   b->y2 = nd->ob->y2 - nd->ob->y1;
}

/* Creates the object that allows to move the window. This is an invisible
   object. */
extern int DraggableWin(void)
{
   t_object *b;

   b = CreateObject(0, 0, opwin->win->opnode);
   b->Position = DraggerPosition;
   b->Pack = DraggerRightPack;
   b->exclude_add = 1;
   b->exclude_rect = 1;
   b->click = 0;
   SetObjectSlidable(b->id, DragWindow, LEFT_MOUSE, b);
   return b->id;
}
