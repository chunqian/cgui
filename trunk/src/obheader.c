/* Module OBHEADER.C

   This file contains functions for creating an object displaying a label
   intended for window-headers, but may be used even for other purposes. The
   width is initially set to the width necessary for printing the label-text,
   but it will later be extended to the full size of the window width, when
   the window has been adjusted. If the window is of fix size, the object
   will take that width immediately. Normally labels don't need to change
   text during their lifetime. In some cases a window label may be an
   exception to that. Therefor the window label contains a function for
   resizing the label-text. The window label is created automatically by the
   window-creating functions, and need not be called by the application. The
   background of the window label will differ between inactive and active
   windows. The application interface, just gets a label-object outlined with
   a border. */

#include <string.h>
#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "id.h"
#include "obheader.h"
#include "tabchain.h"

#if ALLEGRO_SUB_VERSION == 0
#define textout_ex(bmp, f, s, x, y, col, bgcol) textout(bmp, f, s, x, y, col)
#define set_clip_rect set_clip
#endif

extern void DrawThinFrame(BITMAP *bmp, int x1, int y1, int x2, int y2)
{
   hline(bmp, x1, y1, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   vline(bmp, x1, y1, y2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   hline(bmp, x1, y2, x2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   vline(bmp, x2, y1, y2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
}

static void DrawFramedLabel(char *label, t_object *b, int fcol, int bcol)
{
   int ofs;
   int cl, cr, ct, cb;
   int x1, x2, y1, y2;
   BITMAP *bmp;

   x1 = b->x1;
   x2 = b->x2;
   y1 = b->y1;
   y2 = b->y2;
   bmp = b->parent->bmp;
   ofs = (x2 - x1 + 1 - text_length(b->font, label)) / 2;
   if (ofs < HEADERFRAME + HEADERSPACE)
      ofs = HEADERFRAME + HEADERSPACE;
   rect(bmp, x1, y1, x2, y2, cgui_colors[CGUI_COLOR_CONTAINER]);
   rect(bmp, x1 + 1, y1 + 1, x2 - 1, y2 - 1, cgui_colors[CGUI_COLOR_CONTAINER]);
   DrawThinFrame(bmp, x1+2, y1+2, x2-2, y2-2);
   rectfill(bmp, x1 + HEADERFRAME, y1 + HEADERFRAME, x2 - HEADERFRAME, y2 - HEADERFRAME, bcol);
   cl = bmp->cl;
   cr = bmp->cr;
   ct = bmp->ct;
   cb = bmp->cb;
   set_clip_rect(bmp, x1 + HEADERFRAME, y1 + HEADERFRAME, x2 - HEADERFRAME - DEFAULT_WINFRAME, y2 - HEADERFRAME);
   textout_ex(bmp, b->font, label, x1 + ofs, y1 + HEADERFRAME + HEADERSPACE, fcol, -1);
   set_clip_rect(bmp, cl, ct, cr, cb);
}

static void DrawWinHead(t_object *b)
{
   if (b->parent->bmp == NULL)
      return;
   if (b->hasfocus) {
      DrawFramedLabel(b->label, b, cgui_colors[CGUI_COLOR_TITLE_FOCUS_TEXT], cgui_colors[CGUI_COLOR_TITLE_FOCUS_BACKGROUND]);
   } else {
      DrawFramedLabel(b->label, b, cgui_colors[CGUI_COLOR_TITLE_UNFOCUS_TEXT], cgui_colors[CGUI_COLOR_TITLE_UNFOCUS_BACKGROUND]);
   }
}

extern int ChangeText(t_object * b, void *data, int type)
{
   char *label = data;

   if (type != OB_CHGTEXT)
      return 0;
   Release(b->label);
   b->label = MkString(label);
   return 1;
}

static void SetWinHeadSize(t_object *b)
{
   b->x1 = b->y1 = 0;
   b->y2 = text_height(b->font) - 1 + HEADERFRAME * 2 + HEADERSPACE * 2;
   b->x2 = text_length(b->font, b->label) - 1 + HEADERFRAME * 2 + HEADERSPACE * 2;
}

extern int MkWinHeader(const char *label)
{
   t_object *b;
   t_window *w;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      tf = default_type_functions;
      tf.Free = XtendedFree;
      tf.Draw = DrawWinHead;
      tf.ChangeData = ChangeText;
      tf.SetSize = SetWinHeadSize;
   }
   w = opwin->data;
   w->header = b = CreateObject(TOPLEFT | FILLSPACE, opwin->win->opnode);
   b->label = MkString(label);
   b->tf = &tf;
   b->tcfun = &default_single_tc_functions;
   b->click = 0;
   b->tf->DeActivate(b);
   b->hasfocus = 1;

   return b->id;
}

/* Application interface */

extern void ModifyHeader(int id, char *newtag)
{
   t_object *b;

   b = GetObject(id);
   if (b)
      b->tf->ChangeData(b, newtag, OB_CHGTEXT);
}
