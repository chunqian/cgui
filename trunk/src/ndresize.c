/* Module NDRESIZE.C

   Contains the default implementaition for making nodes re-sizeble.

   A new node will encapsulate the requested one. A dragger-object will be
   placed to the left and/or right side of it, and in case of both a third
   one at the bottom right corner. */

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "object.h"
#include "node.h"
#include "window.h"
#include "ndresize.h"
#include "obsizer.h"
#include "mouse.h"
#include "setpos.h"
#include "id.h"

#define MAXTHICKNESS 5
#define NARROWTHICKNESS 2

typedef struct t_resize {
   void (*CallBack) (void *, int, int);
   void *data;
   int thick;
   int dir;
   t_object *b;
   int vlimit, hlimit;
} t_resize;

static void StretchHorizontal(void *data, int dx, int dy nouse)
{
   t_stretch *st = data;
   t_object *b;

   b = st->b;
   b->tf->Stretch(b, dx, 0);
   if (st->Notify)
      st->Notify(st->data);
}

static void StretchVertical(void *data, int dx nouse, int dy)
{
   t_stretch *st = data;
   t_object *b;

   b = st->b;
   b->tf->Stretch(b, 0, dy);
   if (st->Notify)
      st->Notify(st->data);
}

static void Stretch(void *data, int dx, int dy)
{
   t_stretch *st = data;
   t_object *b;

   b = st->b;
   b->tf->Stretch(b, dx, dy);
   if (st->Notify)
      st->Notify(st->data);
}

static int GetStretcherWidth(void *data)
{
   t_stretch *st = data;

   return st->b->x2;
}

static int GetStretcherHeight(void *data)
{
   t_stretch *st = data;

   return st->b->y2;
}

static int DragNode(int x, int y, void *data, int id nouse, int reason)
{
   t_object *b = data;
   t_resize *rs;
   static int oldx, oldy;
   int minx, miny, maxx, maxy, framex, framey;
   t_node *nd;
   t_object *w;

   rs = b->appdata;
   switch (reason) {
   case SL_OVER:
      if (rs->dir == HORIZONTAL)
         SelectCursor(CURS_DRAG_H);
      else if (rs->dir == VERTICAL)
         SelectCursor(CURS_DRAG_V);
      else
         SelectCursor(CURS_CROSS);
      break;
   case SL_OVER_END:
      SelectCursor(CURS_DEFAULT);
      break;
   case SL_STARTED:
      oldx = x;
      oldy = y;
      w = b->parent->win->node->ob;
      /* Find out if the object is contianed (recursively) in a fix-sized node, and if so set mouse limit */
      framex = framey = 0;
      maxx = SCREEN_W;
      maxy = SCREEN_H;
      for (nd = b->parent; nd != w->parent; nd = nd->ob->parent) {
         if (nd->fixsize) {
            if (nd == w->node) {
               /* We reached the actual window, so ob's coordinates are already expressed in screen coordinates */
               maxx = nd->ob->x2;
               maxy = nd->ob->y2;
            } else {
               maxx = nd->ob->x2 + nd->wx + w->x1;
               maxy = nd->ob->y2 + nd->wy + w->y1;
            }
         }
         framex += nd->rightx;
         framey += nd->boty;
      }
      /* find current nodes minima (always set the parent's left edge as
         mouse limit. */
      minx = b->parent->wx + w->x1 + rs->hlimit;
      miny = b->parent->wy + w->y1 + rs->vlimit;
      _CguiSetRange(minx, miny, maxx - framex - minx, maxy - framey - miny);
      break;
   case SL_PROGRESS:
      if (continous_update_resize) {
         if (oldx == x && oldy == y) {
            ;
         } else {
            if (rs->CallBack)
               rs->CallBack(rs->data, x - oldx, y - oldy);
            ReMakeWindow(fwin);
            oldx = x;
            oldy = y;
         }
      }
      break;
   case SL_STOPPED:
      _CguiSetRange(0, 0, SCREEN_W, SCREEN_H);
      if (rs->CallBack)
         rs->CallBack(rs->data, x - oldx, y - oldy);
      ReMakeWindow(fwin);
      LaunchMouseEvent(NULL);
      break;
   }
   return 1;
}


static void DrawHorizontalSizer(t_object * b nouse)
{
   int y1, y2, i;
   if (b->parent->bmp) {
      y1 = (b->y2 - b->y1) / 2 + b->y1 - 5;
      y2 = y1 + 10;
      if (y1<b->y1)
         y1 = b->y1;
      if (y2>b->y2)
         y2 = b->y2;
      rectfill(b->parent->bmp, b->x1, b->y1, b->x2, b->y2, cgui_colors[CGUI_COLOR_CONTAINER]);
      for (i=y1; i<y2; i+=2)
         putpixel(b->parent->bmp, b->x1, i, cgui_colors[CGUI_COLOR_RESIZER_HANDLE]);
      for (i=y1+1; i<=y2; i+=2)
         putpixel(b->parent->bmp, b->x2, i, cgui_colors[CGUI_COLOR_RESIZER_HANDLE]);
   }
}

static void DrawVerticalSizer(t_object * b nouse)
{
   int x1, x2, i;
   if (b->parent->bmp) {
      x1 = (b->x2 - b->x1) / 2 + b->x1 - 5;
      x2 = x1 + 10;
      if (x1<b->x1)
         x1 = b->x1;
      if (x2>b->x2)
         x2 = b->x2;
      rectfill(b->parent->bmp, b->x1, b->y1, b->x2, b->y2, cgui_colors[CGUI_COLOR_CONTAINER]);
      for (i=x1; i<x2; i+=2)
         putpixel(b->parent->bmp, i, b->y1, cgui_colors[CGUI_COLOR_RESIZER_HANDLE]);
      for (i=x1+1; i<=x2; i+=2)
         putpixel(b->parent->bmp, i, b->y2, cgui_colors[CGUI_COLOR_RESIZER_HANDLE]);
   }
}

static void DrawDoubleSizer(t_object * b nouse)
{
   int i;
   if (b->parent->bmp) {
      rectfill(b->parent->bmp, b->x1, b->y1, b->x2, b->y2, cgui_colors[CGUI_COLOR_CONTAINER]);
      for (i=b->x1; i<b->x2-1; i+=2) {
         putpixel(b->parent->bmp, i, b->y1, cgui_colors[CGUI_COLOR_RESIZER_HANDLE]);
         putpixel(b->parent->bmp, i+1, b->y1+1, cgui_colors[CGUI_COLOR_RESIZER_HANDLE]);
         putpixel(b->parent->bmp, i, b->y1+2, cgui_colors[CGUI_COLOR_RESIZER_HANDLE]);
      }
   }
}

static void SetSize(t_object * b)
{
   t_resize *rs;

   rs = b->appdata;
   if (b->dire)
      b->x1 = b->y1 = 0;
   if (rs->dir == HORIZONTAL) {
      b->x2 = rs->thick - 1;
      b->y2 = GetStretcherHeight(rs->data);
   } else if (rs->dir == VERTICAL) {
      b->y2 = rs->thick - 1;
      b->x2 = GetStretcherWidth(rs->data);
   } else {
      b->x2 = rs->thick - 1;
      b->y2 = rs->thick - 1;
   }
}

/* Creates the object that allows to move the extend/decrease the dimensions
   of a node. The apperance of the object is plain gray (i.e. invisible). If
   nd is encapsulated within a fix-sized node it will be extended up to the
   maximum of that node. Objects to the right may be pushed outside the node
   (clipped), but the handle to size the node will remain inside the node to
   offer the user the possibilty to decrease it. If all outer nodes are
   self-sized there is no max-limit, and all outer nodes will be extended fit
   their content. The minimum size is the left-edge of the sizeable node
   (+5). parameters: CallBack - will be called when time for re-sizing the
   node. Parameters in the call is: data - the data passed to MkNodeSizer is
   transparently transefered; dx - the relative movement in x-direction; dx -
   the relative movement in y-direction data - data to be passed to CallBack
   later on nd - the node to be adjustable dir -  the direction(s) in which
   the node may be adjustable, use either or both of SIZE_HORIZONTAL and
   SIZE_VERTICAL thick - the thickness of the object to grip. If horizontally
   adjusting: this will be the width of the object to grip (for the user when
   wanting to re-size the node) and the height will be that of the the node.
   cont - indicates if callback should be called each mouse move (otherwise
   only called when re-size operation is finished). */
static t_object *MkGenericStretcher(void (*CallBack) (void *, int, int),
                                    void *data, int x, int y, int thick)
{
   t_object *b;
   t_resize *rs;

   rs = GetMem0(t_resize, 1);
   rs->CallBack = CallBack;
   rs->data = data;
   rs->thick = thick;
   rs->vlimit = 6;
   rs->hlimit = 10;
   b = CreateObject(x, y, opwin->win->opnode);
   b->click = 0;
   b->exclude_add = 1;
   b->appdata = rs;
   SetObjectSlidable(b->id, DragNode, LEFT_MOUSE, b);
   return b;
}

static void MakeGenericTypeFunctions(t_typefun *tf)
{
   *tf = default_type_functions;
   tf->Free = XtendedFree;
   tf->SetSize = SetSize;
}

static t_object *MkHorizontalStretcher(void (*CallBack) (void *, int, int),
                                 void *data, int width, int extsens)
{
   t_object *b;
   t_resize *rs;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      MakeGenericTypeFunctions(&tf);
      tf.Draw = DrawHorizontalSizer;
   }
   b = MkGenericStretcher(CallBack, data, RIGHT|FILLSPACE|VERTICAL, width);
   b->tf = &tf;
   rs = b->appdata;
   rs->dir = HORIZONTAL;
   b->dx1 = -extsens;
   b->dx2 = -extsens;
   return b;
}

static t_object *MkVerticalStretcher(void (*CallBack) (void *, int, int),
                               void *data, int height, int extsens)
{
   t_object *b;
   t_resize *rs;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      MakeGenericTypeFunctions(&tf);
      tf.Draw = DrawVerticalSizer;
   }
   b = MkGenericStretcher(CallBack, data, DOWNLEFT|FILLSPACE|ALIGNBOTTOM, height);
   b->tf = &tf;
   rs = b->appdata;
   rs->dir = VERTICAL;
   b->dy1 = -extsens;
   b->dy2 = -extsens;
   return b;
}

static t_object *MkDoubleStretcher(void (*CallBack) (void *, int, int), void *data,
                       int size, int extsens)
{
   t_object *b;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      MakeGenericTypeFunctions(&tf);
      tf.Draw = DrawDoubleSizer;
   }
   b = MkGenericStretcher(CallBack, data, DOWN|ALIGNRIGHT, size);
   b->tf = &tf;
   b->dy1 = -extsens;
   b->dy2 = -extsens;
   b->dx1 = -extsens;
   b->dx2 = -extsens;
   return b;
}

extern int MakeNodeStretchable(t_object *b, void (*Notify) (void *),
                               void *data, int options)
{
   t_node *nd, *tmp;
   t_stretch *st;

   b = b->tf->GetMainNode(b);   /* if b is a node the id given to the
                                   application may possibly not refere to the
                                   main node of the container */
   tmp = opwin->win->opnode;
   nd = MkNode(opwin->win->opnode, b->x1, b->y1, ADAPTIVE, StubOb);
   SetDistance(0,0);

   nd->ob->dire = b->dire;
   nd->ob->seqdir = b->seqdir;
   if (b->node) {
      nd->Pack = b->node->Pack;
      nd->Position = b->node->Position;
      nd->EqualSize = b->node->EqualSize;
   } else {
      nd->Pack = b->Pack;
      nd->Position = b->Position;
      nd->EqualSize = b->EqualSize;
   }
   nd->ob->Align = b->Align;
   MoveNodeInTree(nd, b);
   opwin->win->opnode = nd;

   st = GetMem0(t_stretch, 1);
   st->Notify = Notify;
   st->data = data;
   st->b = b;
   nd->ob->appdata = st;
   HookExit(nd->ob->id, FreeResizer, st);
   if (!(options & NO_HORIZONTAL))
      st->h = MkHorizontalStretcher(StretchHorizontal, st,
            NARROWTHICKNESS, NARROWTHICKNESS);
   if (!(options & NO_VERTICAL))
      st->v = MkVerticalStretcher(StretchVertical, st, NARROWTHICKNESS,
            NARROWTHICKNESS);
   if (!(options & NO_VERTICAL) && !(options & NO_HORIZONTAL))
      st->d = MkDoubleStretcher(Stretch, st, MAXTHICKNESS, MAXTHICKNESS);

   b->dire = DIR_TOPLEFT;
   b->x1 = b->y1 = 0;
   if (b->node) {
      b->node->Position = PositionTopLeft;
   } else {
      b->Position = PositionTopLeft;
   }
   opwin->win->opnode = tmp;
   return nd->ob->id;
}

extern int SetStretchMin(int id, int w, int h)
{
   t_object *b;
   t_stretch *st;
   t_resize *rs;

   b = GetObject(id);
   if (b) {
      st = b->appdata;
      if (st->v) {
         rs = st->v->appdata;
         rs->vlimit = h;
      }
      if (st->h) {
         rs = st->h->appdata;
         rs->hlimit = w;
      }
      if (st->d) {
         rs = st->d->appdata;
         rs->vlimit = h;
         rs->hlimit = w;
      }
      return 1;
   }
   return 0;
}
