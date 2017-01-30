/* Module SCROLL.C

   This module contains functions for the scroll-buttons. (This has nothing
   to do with screen-scrolling: it is all about list-scrolling!).

   This is written for scrolling in lists, but may as well be used for
   scrolling any window-object.

   Normally you both want to have scrollers and a browse-bar. If so there is
   a predefined browse-block containing all 3 components, see browse.c If you
   just want to create a listbox with scrolling possiblities don't bother -
   you will get this facility automatically! */

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "obbutton.h"
#include "browsbar.h"
#include "mstates.h"
#include "scroll.h"
#include "event.h"
#include "id.h"

typedef struct t_scroll {
   struct t_object *b;
   void (*CallBack) (void *data);
   void *data;
   int scrolldir;          /* The scrolling direction (signed increments) */
   int id;                 /* The event-id used when continous scrolling */
   int *pstart, *pend, *pwidth;
   int start_delay;        /* The time from first click to start of the
                              continous scrolling */
   int repeat_delay;       /* The time between each increment in the
                              continous scrolling mode (from 2:nd on). */
   int default_size;       /* Default size (w and h) of the button */
} t_scroll;

int cgui_scroll_start_delay = 500, cgui_scroll_repeat_delay = 50;
static int scroll_start_delay, scroll_repeat_delay;
static int fetching_in_progress = 0;

static int FetchData(struct t_object *b)
{
   t_scroll *scr;

   if (fetching_in_progress) {
      scr = b->appdata;
      scr->start_delay = scroll_start_delay;
      scr->repeat_delay = scroll_repeat_delay;
      fetching_in_progress = 0;
      return 1;
   }
   return 0;
}

extern int   SetScrollerRate(int id, int start_delay, int repeat_delay)
{
   t_object *b;

   b = GetObject(id);
   if (b) {
      fetching_in_progress = 1;
      scroll_start_delay = start_delay;
      scroll_repeat_delay = repeat_delay;
      b->tf->FetchData(b);
      if (fetching_in_progress) {
         fetching_in_progress = 0;
         return 0;
      }
      return 1;
   }
   return 0;
}

static void Upper(BITMAP *bmp, t_object *b, int col, int ofs)
{
   int ofsy, ofsx, h;

   h = b->y2 - b->y1 - 1;
   h += h&1;
   ofsy  = ofs + h / 2 + 2;
   ofsx = ofs + (b->x2 - b->x1) / 2;
   if (h > 6) {
      hline(bmp, b->x1 + ofsx - 4, b->y1 + ofsy--, b->x1 + ofsx + 4, col);
      hline(bmp, b->x1 + ofsx - 3, b->y1 + ofsy--, b->x1 + ofsx + 3, col);
   } else
      ofsy--;
   if (h > 4) {
      hline(bmp, b->x1 + ofsx - 2, b->y1 + ofsy--, b->x1 + ofsx + 2, col);
      hline(bmp, b->x1 + ofsx - 1, b->y1 + ofsy--, b->x1 + ofsx + 1, col);
   } else
      ofsy--;
   if (h > 2)
      hline(bmp, b->x1 + ofsx - 0, b->y1 + ofsy--, b->x1 + ofsx + 0, col);
}

static void Lower(BITMAP *bmp, t_object *b, int col, int ofs)
{
   int ofsy, ofsx, h;

   h = b->y2 - b->y1 - 1;
   h += h&1;
   ofsy  = ofs + h / 2 - 2;
   ofsx = ofs + (b->x2 - b->x1) / 2;
   if (h > 6) {
      hline(bmp, b->x1 + ofsx - 4, b->y1 + ofsy++, b->x1 + ofsx + 4, col);
      hline(bmp, b->x1 + ofsx - 3, b->y1 + ofsy++, b->x1 + ofsx + 3, col);
   } else
      ofsy++;
   if (h > 4) {
      hline(bmp, b->x1 + ofsx - 2, b->y1 + ofsy++, b->x1 + ofsx + 2, col);
      hline(bmp, b->x1 + ofsx - 1, b->y1 + ofsy++, b->x1 + ofsx + 1, col);
   } else
      ofsy++;
   if (h > 2)
      hline(bmp, b->x1 + ofsx - 0, b->y1 + ofsy++, b->x1 + ofsx + 0, col);
}

static void Left(BITMAP *bmp, t_object *b, int col, int ofs)
{
   int ofsw, ofsl;

   ofsl = ofs + (b->x2 - b->x1 - 4) / 2;
   ofsw = ofs + (b->y2 - b->y1) / 2 - 1;
   if (ofsl > -3)
      vline(bmp, b->x1 + ofsl + 3, b->y1 + ofsw - 3, b->y1 + ofsw + 3, col);
   if (ofsl > -2)
      vline(bmp, b->x1 + ofsl + 2, b->y1 + ofsw - 2, b->y1 + ofsw + 2, col);
   if (ofsl > -1)
      vline(bmp, b->x1 + ofsl + 1, b->y1 + ofsw - 1, b->y1 + ofsw + 1, col);
   if (ofsl > 0)
      vline(bmp, b->x1 + ofsl + 0, b->y1 + ofsw - 0, b->y1 + ofsw + 0, col);
}

static void Right(BITMAP *bmp, t_object *b, int col, int ofs)
{
   int ofsw, ofsl;

   ofsl = ofs + (b->x2 - b->x1 - 4) / 2;
   ofsw = ofs + (b->y2 - b->y1) / 2 - 1;
   if (ofsl > -3)
      vline(bmp, b->x1 + ofsl + 3, b->y1 + ofsw - 0, b->y1 + ofsw + 0, col);
   if (ofsl > -2)
      vline(bmp, b->x1 + ofsl + 2, b->y1 + ofsw - 1, b->y1 + ofsw + 1, col);
   if (ofsl > -1)
      vline(bmp, b->x1 + ofsl + 1, b->y1 + ofsw - 2, b->y1 + ofsw + 2, col);
   if (ofsl > 0)
      vline(bmp, b->x1 + ofsl + 0, b->y1 + ofsw - 3, b->y1 + ofsw + 3, col);
}

static void DrawScrollButton(t_object *b,
                      void (*Arrow) (BITMAP *, t_object *, int col, int ofs))
{
   BITMAP *bmp;
   int col;

   bmp = b->parent->bmp;
   if (bmp == NULL)
      return;
   if (b->inactive)
      col = cgui_colors[CGUI_COLOR_LIGHTENED_BORDER];
   else
      col = cgui_colors[CGUI_COLOR_HEAVY_SHADOWED_BORDER];
   if (b->state == MS_SLIDING) {
      ButtonInFrame(b->parent->bmp, b->x1, b->y1, b->x2, b->y2);
      rectfill(bmp, b->x1 + 2, b->y1 + 2, b->x2 - 2 + 1, b->y2 - 2 + 1, cgui_colors[CGUI_COLOR_WIDGET_BACKGROUND]);
      Arrow(bmp, b, col, 1);
   } else {
      ButtonFrame(b->parent->bmp, b->x1, b->y1, b->x2, b->y2, 0);
      rectfill(bmp, b->x1 + 2, b->y1 + 2, b->x2 - 2, b->y2 - 2, cgui_colors[CGUI_COLOR_WIDGET_BACKGROUND]);
      Arrow(bmp, b, col, 0);
   }
}

static void DrawUpperScrollButton(t_object *b)
{
   DrawScrollButton(b, Upper);
}

static void DrawLowerScrollButton(t_object *b)
{
   DrawScrollButton(b, Lower);
}

static void DrawLeftScrollButton(t_object *b)
{
   DrawScrollButton(b, Left);
}

static void DrawRightScrollButton(t_object *b)
{
   DrawScrollButton(b, Right);
}

static void Scroll(void *data)
{
   t_scroll *scr = data;

   scr->CallBack(scr->data);
   scr->id = _GenEventOfCgui(Scroll, scr, scr->repeat_delay, scr->b->id);
}

static int ScrollGrip(int x nouse, int y nouse, void *data, int id nouse,
                       int reason)
{
   t_scroll *scr = data;

   switch (reason) {
   case SL_STARTED:
      scr->CallBack(scr->data);
      scr->id = _GenEventOfCgui(Scroll, scr, scr->start_delay, scr->b->id);
      scr->b->tf->Refresh(scr->b);
      break;
   case SL_PROGRESS:
      break;
   case SL_STOPPED:
      _KillEventOfCgui(scr->id);
      scr->id = 0;
      scr->b->tf->Refresh(scr->b);
      break;
   case SL_OVER:
      return 0;
      break;
   default:
      return 0;
   }
   return 1;
}

/* These functions updates the scroller state, i.e. if there is no longer any
   need for scrolling the button will hide or if there are now need for, but
   was recently not it will get visible. NotifyScrollerUpd updates screen
   while NotifyScroller only make state change. This is to give the
   opportunity to avoid unnecessry screen-blits. */
extern int NotifyScroller(t_scroll *scr, int active)
{
   int inactive;

   inactive = active == 0;
   if (inactive != scr->b->inactive) {
      scr->b->inactive = inactive;
      return 1;
   }
   return 0;
}

extern void SetScrollSize(t_scroll *scr, int length)
{
   *scr->pend = *scr->pstart + length - 1;
   scr->default_size = length - 1;
}

static void SetSize(t_object *b)
{
   t_scroll *scr;

   scr = b->appdata;
   if (b->dire)
      b->x1 = b->y1 = 0;
   *scr->pend = *scr->pstart + scr->default_size;
   *scr->pwidth = BROWSERWIDTH - 1;
}

static void SetFunctions(t_typefun *tf, void (*Draw)(t_object *))
{
   *tf = default_type_functions;
   tf->Free = XtendedFree;
   tf->FetchData = FetchData;
   tf->SetSize = SetSize;
   tf->Draw = Draw;
}

/* Crates a generic scroll-button at specified position. */
static t_scroll *CreateScrollButton(int x, int y,
                void (*CallBack) (void *data), void *data)
{
   t_object *b;
   t_scroll *scr;
   scr = GetMem0(t_scroll, 1);
   scr->CallBack = CallBack;
   scr->data = data;
   scr->repeat_delay = cgui_scroll_repeat_delay;
   scr->start_delay = cgui_scroll_start_delay;
   scr->b = b = CreateObject(x, y, opwin->win->opnode);
   b->appdata = scr;
   SetObjectSlidable(b->id, ScrollGrip, LEFT_MOUSE, scr);
   b->inactive = 1;
   b->click = 0;
   return scr;
}

extern t_scroll *CreateScrollUpButton(int x, int y,
                void (*CallBack) (void *data), void *data)
{
   t_scroll *scr;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      SetFunctions(&tf, DrawUpperScrollButton);
   }
   scr = CreateScrollButton(x, y, CallBack, data);
   scr->b->tf = &tf;
   scr->scrolldir = -1;
   scr->pwidth = &scr->b->x2;
   scr->pstart = &scr->b->y1;
   scr->pend = &scr->b->y2;
   scr->default_size = BROWSERWIDTH - 1;
   return scr;
}

extern t_scroll *CreateScrollDownButton(int x, int y,
                void (*CallBack) (void *data), void *data)
{
   t_scroll *scr;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      SetFunctions(&tf, DrawLowerScrollButton);
   }
   scr = CreateScrollButton(x, y, CallBack, data);
   scr->b->tf = &tf;
   scr->scrolldir = 1;
   scr->pwidth = &scr->b->x2;
   scr->pstart = &scr->b->y1;
   scr->pend = &scr->b->y2;
   scr->default_size = BROWSERWIDTH - 1;
   return scr;
}

extern t_scroll *CreateScrollLeftButton(int x, int y,
                void (*CallBack) (void *data), void *data)
{
   t_scroll *scr;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      SetFunctions(&tf, DrawLeftScrollButton);
   }
   scr = CreateScrollButton(x, y, CallBack, data);
   scr->b->tf = &tf;
   scr->scrolldir = -1;
   scr->pwidth = &scr->b->y2;
   scr->pstart = &scr->b->x1;
   scr->pend = &scr->b->x2;
   scr->default_size = BROWSERWIDTH - 1;
   return scr;
}

extern t_scroll *CreateScrollRightButton(int x, int y,
                void (*CallBack) (void *data), void *data)
{
   t_scroll *scr;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      SetFunctions(&tf, DrawRightScrollButton);
   }
   scr = CreateScrollButton(x, y, CallBack, data);
   scr->b->tf = &tf;
   scr->scrolldir = 1;
   scr->pwidth = &scr->b->y2;
   scr->pstart = &scr->b->x1;
   scr->pend = &scr->b->x2;
   scr->default_size = BROWSERWIDTH - 1;
   return scr;
}
