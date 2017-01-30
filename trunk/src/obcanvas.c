/* Module OBCANVAS.C
   Contains the function for creating a canvas object. */

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "node.h"
#include "window.h"
#include "object.h"
#include "ndresize.h"
#include "obcanvas.h"
#include "mouse.h"
#include "msm.h"

typedef struct t_canvas {
   BITMAP *bmp;
   void (*CallBack) (BITMAP *, int x, int y, void *);
   void *data;
   int calling;
   int inited;
   t_typefun *tf;
   int w, h;
   int scrw, scrh;              /* saved screen resolution */
} t_canvas;

static void CanvasWrapper(t_object *b, volatile int x, int y)
{
   t_canvas *can;

   can = b->appdata;
   if (can->bmp && !can->calling) {
      can->calling = 1;
      can->CallBack(can->bmp, x, y, can->data);
      can->calling = 0;
   }
}

static void CanvasDraw(t_object *b)
{
   t_canvas *can;

   can = b->appdata;
   if (can->inited)
      CanvasWrapper(b, -1, 0);
   else {
      can->inited = 1;
      CanvasWrapper(b, 0, -1);
   }
}

static int CanvasMouseEv(t_mousedata *m, t_object *b)
{
   int over, x, y;

   x = m->x - b->parent->win->node->ob->x1 - b->parent->wx;
   y = m->y - b->parent->win->node->ob->y1 - b->parent->wy;
   over = isOver(x, y, b);
   if (over) {
      CanvasWrapper(b, x - b->x1, y - b->y1);
      return 1;
   }
   return 0;
}

static void CanvasFree(t_object *b)
{
   t_canvas *can;

   can = b->appdata;
   if (can->bmp)
      destroy_bitmap(can->bmp);
   can->tf->Free(b);
   Release(can);
}

static void SetSize(t_object *b)
{
   t_canvas *can;

   can = b->appdata;
   if (b->dire)
      b->x1 = b->y1 = 0;
   b->x2 = b->x1 + can->w - 1 + b->rex;
   b->y2 = b->y1 + can->h - 1 + b->rey;
}

static void DestroyCanvas(t_object *b)
{
   t_canvas *can;

   can = b->appdata;
   if (can->bmp) {
      destroy_bitmap(can->bmp);
      can->bmp = NULL;
      can->inited = 0;
   }
}

static void MakeCanvasBitmap(t_object *b)
{
   t_canvas *can;
   int w, h;

   can = b->appdata;
   if (can->bmp == NULL) {
      w = b->x2 - b->x1 + 1;
      h = b->y2 - b->y1 + 1;
      if (w > 0 && h > 0)
         can->bmp = create_sub_bitmap(b->parent->bmp, b->x1, b->y1, w, h);
   }
}

extern int MkCanvas(int x, int y, int width, int height,
                    void (*CallBack) (BITMAP *bmp, int x, int y,
                                      void *data), void *data)
{
   t_object *b;
   static int virgin = 1;
   static struct t_mevent me;
   static t_typefun tf;
   t_canvas *can;

   b = CreateObject(x, y, opwin->win->opnode);
   can = b->appdata = GetMem0(t_canvas, 1);
   if (virgin) {
      virgin = 0;
      SetDefaultStateMachine(&me);
      me.MouseEv = CanvasMouseEv;
      tf = default_type_functions;
      tf.Free = CanvasFree;
      tf.Draw = CanvasDraw;
      tf.SetSize = SetSize;
      tf.DestroyBitmap = DestroyCanvas;
      tf.MakeBitmap = MakeCanvasBitmap;
      tf.MakeStretchable = MakeNodeStretchable;
   }
   b->me = &me;
   can->tf = b->tf;
   b->tf = &tf;
   can->CallBack = CallBack;
   can->data = data;
   can->w = width;
   can->h = height;
   return b->id;
}
