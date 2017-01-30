/* OBSLIDER.C
   This file contains the functions for creating a slider object */

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "object.h"
#include "node.h"
#include "window.h"
#include "tabchain.h"
#include "ndresize.h"
#include "id.h"
#include "obslider.h"

#if ALLEGRO_SUB_VERSION == 0
#define textout_ex(bmp, f, s, x, y, col, bgcol) textout(bmp, f, s, x, y, col)
#endif

typedef struct t_slider {
   int view;
   int orglen, len;/* Slider length (the actual groove for sliding) */
   /* Pointer to the location in the app which is controlled by the slider (and which value that controls the slider position).
      Only one of them is used (the other is NULL), so each access must first check this. */
   int *ctrl;
   float *ctrlf;
   int start, end; /* The min- and max-value of `ctrl' */
   float startf, endf; /* The min- and max-value of `ctrlf' */
   char sstart[30], send[30]; /* String equivalents for above */
   int lstart, lend; /* The lengths of above strings */
   int scale;      /* Option for drawing a scale */
   int labels;     /* Option for drawing lables (numbers) above the scale */
   int dx, dy;     /* Start pos within object for the slider groove */
   int hw, hh;     /* Handle height and width */
   int h;          /* The height of the slider part. */
   int x;          /* The current position (center) of the handle (relative */
                   /* to the beginning of the groove) */
   t_object *edb;   /* Reference to a hooked object */
   int moffs;      /* The mouse offset on the handle */
   int scaleh;     /* The hight of an optional scale */
   int labelh;     /* The hight of optional number labels */
   int vertical;
   void (*AppCallBack) (void *data); /* App. callback (optional) */
   void *data;     /* App. data (to be passed to the callback) */
   void (*Notify) (void *); /* App. notifier for optional re-sizing (we must wrap it) */
   void *re_appdata; /* Data for above */
   void (*Draw) (BITMAP*, t_object*, struct t_slider*, int, int); /* The style drawer */
   t_object *b;    /* Ref. to object */
} t_slider;

static void DrawScale(BITMAP *bmp, t_object *b, t_slider *sl, int scol)
{
   int x, x1, x2, y1, y2, n, i;

   x1 = b->x1;
   y1 = b->y1 + sl->labelh;
   x2 = b->x2;
   y2 = y1 + sl->scaleh - 1;
   rectfill(bmp, x1, y1, x2, y2, cgui_colors[CGUI_COLOR_CONTAINER]);
   x1 = b->x1 + sl->dx;
   x2 = x1 + sl->len - 1;
   n = sl->len / 10;
   if (n > 10)
      n = 10;
   else if (n & 1)
      n++;
   for (i=0; i<n+1; i++) {
      x = x1 + i*(sl->len-1)/(double)n + 0.5;
      if (i && i<n)
         vline(bmp, x, y1+1,  y2, scol);
      else
         vline(bmp, x, y1,  y2, scol);
   }
}

static void DrawScale2(BITMAP *bmp, t_object *b, t_slider *sl, int scol, int sx1, int sx, int sx2)
{
   int x, x1, y1, y2, n, i, col;

   x1 = b->x1 + sl->dx;
   y1 = b->y1 + sl->dy - 2;
   y2 = y1 + 4;
   n = sl->len / 5;
   if (n > 20)
      n = 20;
   else if (n & 1)
      n++;
   for (i=0; i<n+1; i++) {
      x = x1 + i*(sl->len-1)/(double)n + 0.5;
      if (x != sx) {
         if ((x >= sx1 && x <= sx2) && scol == cgui_colors[CGUI_COLOR_LABEL])
            col = cgui_colors[CGUI_COLOR_SHADOWED_BORDER];
         else
            col = scol;
         if (i && i<n)
            vline(bmp, x, y1+1,  y2, col);
         else
            vline(bmp, x, y1,  y2, col);
      }
   }
}

static void DrawScale3(BITMAP *bmp, t_object *b, t_slider *sl, int col)
{
   int y, x1, y1, x2, x3, x4, n, i;

   x1 = b->x1 + 1;
   y1 = b->y2 - sl->dx;
   x2 = x1 + 1;
   x4 = b->x2 - sl->labelh - 1;
   x3 = x4 - 1;
   n = sl->len / 10;
   if (n > 10)
      n = 10;
   else if (n & 1)
      n++;
   for (i=0; i<n+1; i++) {
      y = y1 - i*(sl->len-1)/(double)n + 0.5;
      hline(bmp, x1, y,  x2, col);
      hline(bmp, x3, y,  x4, col);
   }
}

static void DrawLabels(BITMAP *bmp, t_object *b, t_slider *sl, int scol)
{
   int len, x1, x2;

   rectfill(bmp, b->x1, b->y1, b->x2, b->y1+sl->labelh - 1, cgui_colors[CGUI_COLOR_CONTAINER]);
   len = sl->lstart;
   if (len/2 > sl->dx)
      x1 = b->x1;
   else
      x1 = b->x1 + sl->dx - len/2;
   textout_ex(bmp, b->font, sl->sstart, x1, b->y1, scol, -1);
   x2 = x1 + sl->len - 1;
   len = sl->lend;
   if (len/2 > sl->dx)
      x2 = b->x2 - len;
   else
      x2 = b->x2 - sl->dx - len/2;
   textout_ex(bmp, b->font, sl->send, x2, b->y1, scol, -1);
}

static void DrawLabels3(BITMAP *bmp, t_object *b, t_slider *sl, int scol)
{
   int fh, x1, x2, y1, y2;

   x2 = b->x2;
   x1 = x2 - sl->labelh + 1;
   y1 = b->y1;
   y2 = b->y2;
   rectfill(bmp, x1, y1, x2, y2, cgui_colors[CGUI_COLOR_CONTAINER]);
   fh = text_height(b->font) / 2;
   y2 -= sl->dx + fh;
   textout_ex(bmp, b->font, sl->sstart, x1, y2, scol, -1);
   y1 += sl->dx - fh;
   textout_ex(bmp, b->font, sl->send, x1, y1, scol, -1);
}

static void DrawSlider0(BITMAP *bmp, t_object *b, t_slider *sl, int col, int scol)
{
   int x1, y1, x2, y2, x, y;

   x1 = b->x1;
   y1 = b->y1 + sl->scaleh + sl->labelh;
   x2 = b->x2;
   y2 = b->y2;
   /* draw frame */
   hline(bmp, x1, y1, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   vline(bmp, x1, y1+1, y2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   hline(bmp, x1+1, y2, x2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   vline(bmp, x2, y1+1, y2-1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   /* Fill centre */
   x1++;
   y1++;
   x2--;
   y2--;
   rectfill(bmp, x1, y1, x2, y2, cgui_colors[CGUI_COLOR_CONTAINER]);
   /* Draw groove (3 pixels) */
   x1 = b->x1 + sl->dx;
   x2 = x1 + sl->len - 1;
   y = b->y1 + sl->dy;
   hline(bmp, x1, y - 1, x2, col);
   hline(bmp, x1, y + 1, x2, col);
   hline(bmp, x1-1, y, x2+1, col);
   /* draw handle */
   x = b->x1 + sl->dx + sl->x;
   x1 = x - sl->hw/2;
   x2 = x1 + sl->hw - 1;
   y1 = y - sl->hh/2;
   y2 = y1 + sl->hh - 1;
   /* Outline */
   hline(bmp, x1, y1, x2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   hline(bmp, x1, y2, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   vline(bmp, x1, y1 + 1, y2 - 1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   vline(bmp, x2, y1 + 1, y2 - 1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   /* Fill */
   rectfill(bmp, x1+1, y1+1, x2-1, y2-1, cgui_colors[CGUI_COLOR_CONTAINER]);
   /* Mark center position */
   vline(bmp, x,  y1-1, y1+sl->hh/2, cgui_colors[CGUI_COLOR_HEAVY_LIGHTENED_BORDER]);
   vline(bmp, x-1, y1,  y1+sl->hh/2-1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   vline(bmp, x+1, y1,  y1+sl->hh/2-1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   if (sl->scale)
      DrawScale(bmp, b, sl, scol);
   if (sl->labels)
      DrawLabels(bmp, b, sl, scol);
}

static void DrawSlider1(BITMAP *bmp, t_object *b, t_slider *sl, int col, int scol)
{
   int x1, y1, x2, y2, x, y, i;

   x1 = b->x1;
   y1 = b->y1 + sl->scaleh + sl->labelh;
   x2 = b->x2;
   y2 = b->y2;
   /* Fill slider area */
   rectfill(bmp, x1, y1, x2, y2, cgui_colors[CGUI_COLOR_CONTAINER]);
   /* Draw groove */
   x1 = b->x1 + sl->dx;
   x2 = x1 + sl->len - 1;
   y = b->y1 + sl->dy;
   hline(bmp, x1, y, x2, col);
   /* Draw handle */
   x = b->x1 + sl->dx + sl->x;
   vline(bmp, x, y1,  y1 + 1, col);
   vline(bmp, x, y1 + 2,  y1 + sl->hh, cgui_colors[CGUI_COLOR_HEAVY_LIGHTENED_BORDER]);
   y = y1;
   for (x1 = x-1, y1 = y, i = 0; y1 <= y + sl->hh/2 + 1; y1++, x1-=i++&1)
      hline(bmp, x1, y1, x-1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   x1++;
   i = 0;
   for (; y1 <= y + sl->hh; y1++, x1+=i++&1)
      hline(bmp, x1, y1, x-1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   x++;
   for (x2 = x, y1 = y, i = 0; y1 <= y + sl->hh/2 + 1; y1++, x2+=i++&1)
      hline(bmp, x, y1, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   x2--;
   i = 0;
   for (; y1 <= y + sl->hh; y1++, x2-=i++&1)
      hline(bmp, x, y1, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   if (sl->scale)
      DrawScale(bmp, b, sl, scol);
   if (sl->labels)
      DrawLabels(bmp, b, sl, scol);
}

static void DrawSlider2(BITMAP *bmp, t_object *b, t_slider *sl, int col, int scol)
{
   int x1, y1, x2, y2, x;

   x1 = b->x1;
   y1 = b->y1;
   x2 = b->x2;
   y2 = b->y2;
   /* Draw rails */
   hline(bmp, x1, y1++, x2, cgui_colors[CGUI_COLOR_CONTAINER]);
   hline(bmp, x1, y1++, x2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   hline(bmp, x1, y1++, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   hline(bmp, x1, y2--, x2, cgui_colors[CGUI_COLOR_CONTAINER]);
   hline(bmp, x1, y2--, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   hline(bmp, x1, y2--, x2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   /* Fill slide area */
   rectfill(bmp, x1, y1, x2, y2, cgui_colors[CGUI_COLOR_CONTAINER]);
   /* Draw handle */
   x = b->x1 + sl->dx + sl->x;
   x1 = x - sl->hw / 2;
   x2 = x1 + sl->hw - 1;
   y1 = b->y1;
   y2 = b->y2;
   hline(bmp, x1, y1, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   hline(bmp, x1, y2, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   vline(bmp, x1, y1+1,  y2-1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   vline(bmp, x2, y1+1,  y2-1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   vline(bmp,  x, y1+1,  y2-1, col);
   /* Draw scale */
   if (sl->scale)
      DrawScale2(bmp, b, sl, scol, x1, x, x2);
}

static void DrawSlider3(BITMAP *bmp, t_object *b, t_slider *sl, int col, int scol)
{
   int x1, y1, x2, y2, x, y;

   x1 = b->x1;
   y1 = b->y1;
   x2 = b->x2 - sl->labelh;
   y2 = b->y2;
   /* draw frame */
   hline(bmp, x1, y1, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   vline(bmp, x1, y1+1, y2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   hline(bmp, x1+1, y2, x2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   vline(bmp, x2, y1+1, y2-1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   /* Fill centre */
   x1++;
   y1++;
   x2--;
   y2--;
   rectfill(bmp, x1, y1, x2, y2, cgui_colors[CGUI_COLOR_CONTAINER]);
   /* Draw groove */
   y1 = b->y2 - sl->dx;
   y2 = y1 - (sl->len - 1);
   x = b->x1 + sl->dy;
   vline(bmp, x, y1, y2, col);
   /* Draw handle */
   y = b->y2 - sl->dx - sl->x;
   x1 = x - sl->hh/2;
   x2 = x1 + sl->hh - 1;
   y1 = y - sl->hw/2;
   y2 = y1 + sl->hw - 1;
   /* ..outline.. */
   hline(bmp, x1, y1, x2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   hline(bmp, x1, y2, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   vline(bmp, x1, y1 + 1, y2 - 1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   vline(bmp, x2, y1 + 1, y2 - 1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   /* ..fill.. */
   rectfill(bmp, x1+1, y1+1, x2-1, y2-1, cgui_colors[CGUI_COLOR_CONTAINER]);
   /* ..groove */
   hline(bmp, x1+1, y-1, x2-1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   hline(bmp, x1+1, y, x2-1, cgui_colors[CGUI_COLOR_HEAVY_LIGHTENED_BORDER]);
   hline(bmp, x1+1, y+1, x2-1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   if (col == cgui_colors[CGUI_COLOR_LABEL_FOCUS]) {
      putpixel(bmp, x1, y, col);
      putpixel(bmp, x2, y, col);
   }
   if (sl->scale)
      DrawScale3(bmp, b, sl, scol);
   if (sl->labels)
      DrawLabels3(bmp, b, sl, scol);
}


static void CalcPositionFromValue(t_slider *sl)
{
   int dx;
   double per, dxf;

   if (sl->ctrl) {
      if (*sl->ctrl < sl->start)
         *sl->ctrl = sl->start;
      else if (*sl->ctrl > sl->end)
         *sl->ctrl = sl->end;
      dx = sl->end - sl->start;
      if (dx)
         per = (*sl->ctrl - sl->start) / (double)dx;
      else
         per = 0;
      sl->x = per * (sl->len-1) + 0.5;
   } else {
      if (*sl->ctrlf < sl->startf)
         *sl->ctrlf = sl->startf;
      else if (*sl->ctrlf > sl->endf)
         *sl->ctrlf = sl->endf;
      dxf = sl->endf - sl->startf;
      if (dxf > 0)
         per = (*sl->ctrlf - sl->startf) / dxf;
      else
         per = 0;
      sl->x = per * (sl->len - 1);
   }
}

static void DrawSlider(t_object *b)
{
   t_slider *sl;
   int col, scol;
   BITMAP *bmp;

   bmp = b->parent->bmp;
   if (bmp == NULL)
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
   if (b->inactive)
      scol = cgui_colors[CGUI_COLOR_LABEL_INACTIVE_1];
   else
      scol = cgui_colors[CGUI_COLOR_LABEL];
   sl = b->appdata;
   CalcPositionFromValue(sl);
   /* If the value did change, we must update eventual hooked on numeric viewer */
   sl->Draw(bmp, b, sl, col, scol);
}

static void CalcValueFromPosition(t_slider *sl, int x)
{
   if (x < 0)
      sl->x = 0;
   else if (x >= sl->len)
      sl->x = sl->len - 1;
   else
      sl->x = x;
   if (sl->ctrl) {
      if (sl->len == 1)
         *sl->ctrl = sl->start;
      else
         *sl->ctrl = sl->start + (sl->end - sl->start) * sl->x / (double)(sl->len - 1) + 0.5;
   } else {
      if (sl->len == 1)
         *sl->ctrlf = sl->startf;
      else
         *sl->ctrlf = sl->startf + (sl->endf - sl->startf) * sl->x / (double)(sl->len - 1);
   }
}

static int SliderGrip(int x0, int y0, void *data, int id nouse, int reason)
{
   t_slider *sl = data;
   int x;

   /* convert the mouse posititon to be relative to the groove */
   if (sl->vertical)
      x = (sl->b->y2 - sl->dx) - y0;
   else
      x = x0 - (sl->b->x1 + sl->dx);
   switch (reason) {
   case SL_STARTED:
      /* If gripped on the handle, keep relative position on the handle */
      if (x >= sl->x - sl->hw/2 && x <= sl->x + sl->hw/2) {
         sl->moffs = x - sl->x;
         x = sl->x;
      } else {                  /* set relative pos to middle of handle */
         sl->moffs = 0;
      }
      CalcValueFromPosition(sl, x);
      sl->b->tf->Refresh(sl->b);
      if (sl->edb)
         sl->edb->tf->Refresh(sl->edb);
      if (sl->AppCallBack)
         sl->AppCallBack(sl->data);
      SetFocusOn(sl->b->id);
      break;
   case SL_PROGRESS:
      CalcValueFromPosition(sl, x - sl->moffs);
      sl->b->tf->Refresh(sl->b);
      if (sl->edb)
         sl->edb->tf->Refresh(sl->edb);
      if (sl->AppCallBack)
         sl->AppCallBack(sl->data);
      break;
   case SL_STOPPED:             /* button up */
      sl->moffs = 0;
      break;
   case SL_OVER:
      return 0;
      break;
   default:
      return 0;
   }
   return 1;
}

static int SliderAddHandler(t_object *b, void (*CallBack) (void *data), void *data)
{
   t_slider *sl;

   sl = b->appdata;
   sl->data = data;
   sl->AppCallBack = CallBack;
   return 1;
}

static void CommonUpdater(t_slider *sl)
{
   sl->b->tf->Refresh(sl->b);
   if (sl->edb)
      sl->edb->tf->Refresh(sl->edb);
   if (sl->AppCallBack)
      sl->AppCallBack(sl->data);
}

static int ArrowKey(t_object *b, t_object *xb nouse, int scan, int ascii)
{
   t_slider *sl;
   float dxf;
   (void)ascii;

   sl = b->appdata;
   switch (scan) {
   case KEY_LEFT:
   case KEY_4_PAD:
   case KEY_DOWN:
   case KEY_2_PAD:
      if (sl->ctrl) {
         if (*sl->ctrl > sl->start) {
            (*sl->ctrl)--;
            CommonUpdater(sl);
         }
      } else {
         /* Decrement with a value corresponding to one pixel. */
         dxf = (sl->endf - sl->startf) / sl->len;
         *sl->ctrlf -= dxf;
         if (*sl->ctrlf < sl->startf) {
            *sl->ctrlf = sl->startf;
         }
         CommonUpdater(sl);
      }
      break;
   case KEY_RIGHT:
   case KEY_6_PAD:
   case KEY_UP:
   case KEY_8_PAD:
      if (sl->ctrl) {
         if (*sl->ctrl < sl->end) {
            (*sl->ctrl)++;
            CommonUpdater(sl);
         }
      } else {
         dxf = (sl->endf - sl->startf) / sl->len;
         *sl->ctrlf += dxf;
         if (*sl->ctrlf > sl->endf) {
            *sl->ctrlf = sl->endf;
         }
         CommonUpdater(sl);
      }
      break;
   case KEY_END:
   case KEY_1_PAD:
      if (sl->ctrl) {
         *sl->ctrl = sl->end;
      } else {
         *sl->ctrlf = sl->endf;
      }
      CommonUpdater(sl);
      break;
   case KEY_HOME:         /* Home-key */
   case KEY_7_PAD:
      if (sl->ctrl) {
         *sl->ctrl = sl->start;
      } else {
         *sl->ctrlf = sl->startf;
      }
      CommonUpdater(sl);
      break;
   default:
      return 0;
   }
   return 1;
}

static void NumericDisplayCallBack(void *data)
{
   t_slider *sl = data;
   int scan=0, ascii=0, dummy;

   GetEditData(&scan, &ascii, &dummy);
   if (ascii == '\r')
      CommonUpdater(sl);
}

static void ResizeCallBack(void *data)
{
   t_slider *sl = data;

   sl->b->tf->Refresh(sl->b);
   if (sl->Notify)
      sl->Notify(sl->re_appdata);
}

static int MakeSliderStretchable(t_object *b, void (*Notify) (void *), void *data, int options)
{
   t_slider *sl;

   sl = b->appdata;
   sl->Notify = Notify;
   sl->re_appdata = data;
   return MakeNodeStretchable(b, ResizeCallBack, sl, options);
}

static void SetSize(t_object *b)
{
   t_slider *sl;

   sl = b->appdata;
   if (b->dire)
      b->x1 = b->y1 = 0;
   b->x2 = b->x1 - 1;
   b->y2 = b->y1 - 1;
   if (sl->vertical) {
      sl->len = sl->orglen + b->rey;
      b->x2 += sl->h;
      b->y2 += sl->len + sl->dx * 2;
   } else {
      sl->len = sl->orglen + b->rex;
      b->x2 += sl->len + sl->dx * 2;
      b->y2 += sl->h;
   }
}

static t_slider *AddGenericSlider(int x, int y, int length, int option, int id)
{
   t_object *b;
   t_slider *sl;
   static t_typefun tf;
   static t_tcfun tc;
   static int virgin = 1;
   int heights[] = {17, 17, 21, 15};
   int dxs[] =     { 8,  8, 10,  8};
   int dys[] =     { 8,  8, 10,  7};
   int scale_h[] = { 3,  3,  0,  2};
   int hws[] =     {15, 15, 21, 15};
   int hhs[] =     {15, 15, 21, 13};

   if (virgin) {
      virgin = 0;
      tf = default_type_functions;
      tf.Free = XtendedFree;
      tf.SetSize = SetSize;
      tf.AddHandler = SliderAddHandler;
      tf.MakeStretchable = MakeSliderStretchable;
      tf.Draw = DrawSlider;
      tc = default_single_tc_functions;
      tc.MoveFocusToNextSubObject =  ArrowKey;
   }
   b = CreateObject(x, y, opwin->win->opnode);
   b->click = 0;
   sl = GetMem0(t_slider, 1);
   SetObjectSlidable(b->id, SliderGrip, LEFT_MOUSE, sl);
   switch (option & (SL_STYLE1|SL_STYLE2|SL_STYLE3)) {
   case SL_STYLE1:
      sl->view = 1;
      sl->Draw = DrawSlider1;
      break;
   case SL_STYLE2:
      sl->view = 2;
      sl->Draw = DrawSlider2;
      break;
   case SL_STYLE3:
      sl->view = 3;
      sl->vertical = 1;
      sl->Draw = DrawSlider3;
      break;
   default:
      sl->Draw = DrawSlider0;
   }
   sl->len = sl->orglen = length;
   if (id) {
      sl->edb = GetObject(id);
      if (sl->edb)
         AddHandler(id, NumericDisplayCallBack, sl);
   }
   sl->scale = option & SL_SCALE;
   sl->labels = option & SL_LABEL;
   sl->h = heights[sl->view];
   sl->dx = dxs[sl->view];
   sl->dy = dys[sl->view];
   sl->hw = hws[sl->view];
   sl->hh = hhs[sl->view];
   if (sl->scale) {
      sl->scaleh = scale_h[sl->view];
      sl->dy += sl->scaleh;
      if (sl->vertical)
         sl->h += 2 * sl->scaleh;
      else
         sl->h += sl->scaleh;
   }
   sl->b = b;
   b->appdata = sl;
   if (b->tablink == NULL)
      b->tf->DoJoinTabChain(b);
   b->tcfun = &tc;
   b->tf = &tf;
   return sl;
}

static void CalculateLabelPositions(t_slider *sl)
{
   t_object *b;
   int h;

   b = sl->b;
   sl->lstart = text_length(b->font, sl->sstart);
   sl->lend = text_length(b->font, sl->send);
   if (sl->vertical) {
      h = MAX(sl->lstart, sl->lend);
   } else {
      h = text_height(b->font);
      sl->dy += h;
   }
   sl->h += h;
   sl->labelh = h;
}

/* Application interface: */

extern int AddSlider(int x, int y, int length, int *ctrl, int start, int end, int option, int id)
{
   t_slider *sl;

   sl = AddGenericSlider(x, y, length, option, id);
   sl->ctrl = ctrl;
   sl->start = start;
   sl->end = end;
   if (sl->labels && sl->view != 2) {
      sprintf(sl->sstart, "%d", start);
      sprintf(sl->send, "%d", end);
      CalculateLabelPositions(sl);
   }
   return sl->b->id;
}

extern int AddSliderFloat(int x, int y, int length, float *ctrl, float start, float end, int ndecimals, int option, int id)
{
   t_slider *sl;
   char fmt[20];

   sl = AddGenericSlider(x, y, length, option, id);
   sl->ctrlf = ctrl;
   sl->startf = start;
   sl->endf = end;
   if (sl->labels && sl->view != 2) {
      sprintf(fmt, "%%.%df", ndecimals);
      sprintf(sl->sstart, fmt, start);
      sprintf(sl->send, fmt, end);
      CalculateLabelPositions(sl);
   }
   return sl->b->id;
}
