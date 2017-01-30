/* Module OBBUTTON.C
   Contains the functions for push-buttons */

#include <string.h>

#include <allegro.h>
#include "cgui.h"

#include "object.h"
#include "node.h"
#include "window.h"
#include "obbutton.h"
#include "mstates.h"

#if ALLEGRO_SUB_VERSION == 0
#define textout_ex(bmp, f, s, x, y, col, bgcol) textout(bmp, f, s, x, y, col)
#define set_clip_rect set_clip
#endif

extern void DisplayBitmap(t_object *b, int disp)
{
   t_image *im;
   int x, y, space;

   if (b->inactive && b->im_inactive) {
      im = b->im_inactive;
   } else if ((b->state == MS_OVER || b->hasfocus) && b->im_focus) {
      im = b->im_focus;
   } else {
      im = b->im;
   }
   if (im && ((b->view & SV_HIDE_ICON)==0 || b->label==NULL)) {
      space = b->x2 - b->x1 + 1 - BUTTONBORDER * 2 - im->w;
      x = b->x1 + BUTTONBORDER + disp + space / 2;
      y = b->y1 + BUTTONBORDER + disp;
      DrawImage(im, b->parent->bmp, x, y);
      if (disp)
         hline(b->parent->bmp, b->x1 + 2, b->y1 + 2, b->x2 - 1, cgui_colors[CGUI_COLOR_WIDGET_BACKGROUND]);
   }
}

/* Draws the label text of a single row push-button */
extern void ButtonText1(int dx, int dy, t_object *b, int center,
                        char *label)
{
   int x, y, xh, yh, xh2;
   int col, end = 0, clip_x2;
   BITMAP *bmp;
   bmp = b->parent->bmp;
   if (b->inactive)
      col = cgui_colors[CGUI_COLOR_LABEL_INACTIVE_1];
   else if (b->hasfocus) {
      if (b->hasfocus == 2)
         col = cgui_colors[CGUI_COLOR_LABEL_HIDDEN_FOCUS];
      else
         col = cgui_colors[CGUI_COLOR_LABEL_FOCUS];
   } else
      col = cgui_colors[CGUI_COLOR_LABEL];

   if (center) {
      x = TEXTCENTERX(b, dx);
   } else {
      x = b->x1 + dx + TEXTOFFSETX;
      end = x + text_length(b->font, label);
   }
   y = TEXTY(b, dy);
   if (x < b->x1 + 2 || end > b->x2 - 2) {
      x = b->x1 + 2 + dx;
      clip_x2 = bmp->cr;
      set_clip_rect(bmp, bmp->cl, bmp->ct, b->x2 - 2, bmp->cb);
      if (b->inactive)
         textout_ex(bmp, b->font, label, x+1, y+1, cgui_colors[CGUI_COLOR_LABEL_INACTIVE_2], -1);
      textout_ex(bmp, b->font, label, x, y, col, -1);
      set_clip_rect(bmp, bmp->cl, bmp->ct, clip_x2, bmp->cb);
   } else {
      if (b->inactive)
         textout_ex(bmp, b->font, label, x+1, y+1, cgui_colors[CGUI_COLOR_LABEL_INACTIVE_2], -1);
      textout_ex(bmp, b->font, label, x, y, col, -1);
   }
   if (b->hklen) {
      xh = x + b->hkxoffs;
      xh2 = xh + b->hklen - 1;
      yh = _cgui_hot_key_line + y;
      hline(bmp, xh, yh, xh2, col);
   }
}

extern void ButtonText(int disp, int x1, int y1, int x2, int y2,
                       t_object *b, BITMAP *bmp)
{
   int xh, xh2, dy, x, y, width, height, col, len, n, i, space;
   char *tag;

   if ((b->view & SV_HIDE_LABEL) || b->label == NULL)
      return;
   if (b->inactive)
      col = cgui_colors[CGUI_COLOR_LABEL_INACTIVE_1];
   else if (b->hasfocus) {
      if (b->hasfocus == 2)
         col = cgui_colors[CGUI_COLOR_LABEL_HIDDEN_FOCUS];
      else
         col = cgui_colors[CGUI_COLOR_LABEL_FOCUS];
   } else
      col = cgui_colors[CGUI_COLOR_LABEL];
   width = x2 - x1 + 1;
   dy = text_height(b->font) + 1;   /* extra pixel for underlining */
   y = y1 + BUTTONBORDER;
   if (b->im && (b->view & SV_HIDE_ICON) == 0)
      y += b->im->h;
   height = y2 - y - BUTTONBORDER + 1;
   for (tag = b->label, n = 0; *tag; tag += strlen(tag) + 1)
      n++;
   space = height - dy * n;
   y += space / 2;
   for (tag = b->label, i = 0; *tag; tag += strlen(tag) + 1, i++, y += dy) {
      len = text_length(b->font, tag);
      if (len > width - BUTTONBORDER * 2) {
         x = TEXTOFFSETX + x1 + disp;  /* leftjustify */
      } else {
         x = ((width - len) >> 1) + x1 + disp;
      }
      if (b->inactive)
         textout_ex(bmp, b->font, tag, x+1, y+1, cgui_colors[CGUI_COLOR_LABEL_INACTIVE_2], -1);
      textout_ex(bmp, b->font, tag, x, y, col, -1);
      if (b->hkrow == i && b->hklen) {
         xh = x + b->hkxoffs;
         xh2 = xh + b->hklen - 1;
         hline(bmp, xh, y + _cgui_hot_key_line, xh2, col);
      }
   }
}

/* Draws the generic button-frame in the "pushed in" view.*/
extern void ButtonInFrame(BITMAP *bmp, int x1, int y1, int x2, int y2)
{
   if (bmp == NULL)
      return;
   hline(bmp, x1, y1, x2, cgui_colors[CGUI_COLOR_HEAVY_SHADOWED_BORDER]);
   hline(bmp, x1 + 1, y1 + 1, x2 - 1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   vline(bmp, x1, y1 + 1, y2 - 1, cgui_colors[CGUI_COLOR_HEAVY_SHADOWED_BORDER]);
   vline(bmp, x1 + 1, y1 + 2, y2 - 1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);

   hline(bmp, x1, y2, x2, cgui_colors[CGUI_COLOR_HEAVY_LIGHTENED_BORDER]);
   vline(bmp, x2, y1 + 1, y2 - 1, cgui_colors[CGUI_COLOR_HEAVY_LIGHTENED_BORDER]     );
}

/* Draws the generic button-frame. If indic is true there will some kind of
   indication on the frem (can be used for marking the focused button
   when no other possibility is avialible. */
extern void ButtonFrame(BITMAP *bmp, int x1, int y1, int x2, int y2, int indic)
{
   int col;
   if (bmp == NULL)
      return;
   if (indic)
      col = cgui_colors[CGUI_COLOR_BUTTON_FRAME_FOCUS];
   else
      col = cgui_colors[CGUI_COLOR_SHADOWED_BORDER];

   /* Top */
   hline(bmp, x1, y1, x2, cgui_colors[CGUI_COLOR_HEAVY_LIGHTENED_BORDER]);
   hline(bmp, x1 + 1, y1 + 1, x2 - 1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);

   /* Left */
   vline(bmp, x1, y1 + 1, y2 - 1, cgui_colors[CGUI_COLOR_HEAVY_LIGHTENED_BORDER]);
   vline(bmp, x1 + 1, y1 + 2, y2 - 2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);

   /* Bottom */
   hline(bmp, x1, y2, x2, cgui_colors[CGUI_COLOR_HEAVY_SHADOWED_BORDER]);
   hline(bmp, x1 + 1, y2 - 1, x2 - 1, col);

   /* Right */
   vline(bmp, x2, y1 + 1, y2 - 1, cgui_colors[CGUI_COLOR_HEAVY_SHADOWED_BORDER]);
   vline(bmp, x2 - 1, y1 + 2, y2 - 2, col);
}

static void DrawButton(t_object *b)
{
   if (b->parent->bmp == NULL)
      return;
   if (b->state == MS_FIRST_DOWN) {
      ButtonInFrame(b->parent->bmp, b->x1, b->y1, b->x2, b->y2);
      FillText(b, 1);
      ButtonText(1, b->x1, b->y1, b->x2, b->y2, b, b->parent->bmp);
      DisplayBitmap(b, 1);
   } else {
      ButtonFrame(b->parent->bmp, b->x1, b->y1, b->x2, b->y2, b->hasfocus);
      FillText(b, 0);
      ButtonText(0, b->x1, b->y1, b->x2, b->y2, b, b->parent->bmp);
      DisplayBitmap(b, 0);
   }
}

extern void SetPushButtonSize(t_object *b)
{
   int textw = 0, norows = 0, imageh = 0, imagew = 0, texth = 0;
   char *tag;

   if (b->dire)
      b->x1 = b->y1 = 0;
   if ((b->view & SV_HIDE_LABEL) == 0 && b->label) {
      for (tag = b->label; *tag; tag += strlen(tag) + 1) {
         textw = MAX(textw, text_length(b->font, tag));
         norows++;
      }
      texth = (text_height(b->font) + 1) * norows;
      if (norows)
         texth += 2;               /* extra pixel-line at top and bottom */
   }
   if (b->im && (b->view & SV_HIDE_ICON)==0) {
      imageh = b->im->h;
      imagew = b->im->w;
   }
   b->x2 = b->x1 - 1 + b->rex + BUTTONBORDER * 2 + 6;
   b->x2 += MAX(textw, imagew);

   /* -1: height to coord, +1: for hot-key underlining, rey: pre-defined
      offset, imageh:image, always add border */
   if (imageh == 0 && norows == 1) { /* use default height */
      b->y2 = b->y1 + _cgui_button1_height - 1 + b->rey;
   } else {
      b->y2 = b->y1 - 1 + b->rey + BUTTONBORDER * 2;
      b->y2 += texth + imageh;
   }
}

extern t_object *KernalAddButton(int x, int y, const char *label,
                               void (*cb) (void *), void *data)
{
   t_object *b;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      tf = default_type_functions;
      tf.Draw = DrawButton;
      tf.SetSize = SetPushButtonSize;
   }
   b = CreateObject(x, y, opwin->win->opnode);
   InsertLabel(b, label);
   b->tf = &tf;
   b->appdata = data;
   b->Action = cb;
   return b;
}

#include "id.h"
extern int AddButton(int x, int y, const char *label,
                         void (*CallBack) (void *), void *data)
{
   t_object *b;

   b = KernalAddButton(x, y, label, CallBack, data);
   if (b->tablink == NULL)
      b->tf->DoJoinTabChain(b);
   return b->id;
}
