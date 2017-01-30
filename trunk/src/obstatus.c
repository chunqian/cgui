/* Module OBSTATUS.C
   Contains the functions for creating a status field object outlined by a
   frame */

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "obheader.h"
#include "obstatus.h"

#if ALLEGRO_SUB_VERSION == 0
#define textout_ex(bmp, f, s, x, y, col, bgcol) textout(bmp, f, s, x, y, col)
#define set_clip_rect set_clip
#endif

typedef struct t_statusfield {
   void (*FormatFunc) (void *data, char *s);
   void *data;
   int w;
   /* For additional handler */
   void (*Action) (void *);
   void *actdata;
} t_statusfield;

static void StatusDraw(t_object *b)
{
   t_statusfield *st;
   char s[1000];
   int xofs, ty;
   int cl, cr, ct, cb;
   int x1, x2, y1, y2;
   BITMAP *bmp;

   x1 = b->x1;
   x2 = b->x2;
   y1 = b->y1;
   y2 = b->y2;
   bmp = b->parent->bmp;
   ty = TEXTY(b, 0);
   xofs = 2;
   st = b->appdata;
   st->FormatFunc(st->data, s);
   DrawThinFrame(bmp, x1, y1, x2, y2);
   rectfill(bmp, x1 + 1, y1 + 1, x2 - 1, y2 - 1, cgui_colors[CGUI_COLOR_STATUSFIELD_BACKGROUND]);
   cl = bmp->cl;
   cr = bmp->cr;
   ct = bmp->ct;
   cb = bmp->cb;
   set_clip_rect(bmp, x1 + 1, y1 + 1, x2 - 1, y2 - 1);
   textout_ex(bmp, b->font, s, x1 + xofs, ty, cgui_colors[CGUI_COLOR_STATUSFIELD_TEXT], -1);
   set_clip_rect(bmp, cl, ct, cr, cb);
}

static void ActionWrapper(void *data)
{
   t_statusfield *st = data;

   if (st->Action)
      st->Action(st->actdata);
}

static void SetSize(t_object *b)
{
   t_statusfield *st;

   st = b->appdata;
   if (b->dire)
      b->x1 = b->y1 = 0;
   b->x2 = b->x1 + st->w - 1 + HEADERFRAME * 2 + HEADERSPACE * 2 + b->rex;
   b->y2 = b->y1 + _cgui_button1_height - 1 + b->rey;
}

static int AddStHandler(t_object *b, void (*cb) (void *), void *data)
{
   t_statusfield *st;

   st = b->appdata;
   st->Action = cb;
   st->actdata = data;
   return 1;
}

extern int AddStatusField(int x, int y, int width,
                          void (*FormatFunc) (void *data, char *s),
                          void *data)
{
   t_object *b;
   t_statusfield *st;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      tf = default_type_functions;
      tf.Draw = StatusDraw;
      tf.Free = XtendedFree;
      tf.SetSize = SetSize;
      tf.AddHandler = AddStHandler;
   }
   b = CreateObject(x, y, opwin->win->opnode);
   st = GetMem(t_statusfield, 1);
   b->appdata = st;
   st->FormatFunc = FormatFunc;
   st->data = data;
   st->w = width;
   b->tf = &tf;
   b->tf->DeActivate(b);
   b->click = 0;
   b->Action = ActionWrapper;
   return b->id;
}
