/* Module BROWSBAR.C
   This module contains functions for the browse-bar handle. */

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "browser.h"
#include "browsbar.h"

typedef struct t_browsebar {
   struct t_object *b;
   void (*CallBack) (void *data);
   void *data;
   int handle_pos;              /* Current top/left position along the
                                   sliding direction of the browser
                                   handle (relative to the object pos) */
   int moffs;                   /* The mouse location counted from the
                                   beginning of the handle (top or left
                                   edge). */
   int *pos;                    /* The position within the scrolled area
                                   which location is at the top of the view
                                   port. */
   int scrolled_area_length;    /* The length of the scrolled area. */
   int view_port_length;        /* The length of the view port */
   int sliding_area_length;     /* The length of the sliding area (within
                                   which the handle can slide, so the total
                                   number of sliding pixels is
                                   sliding_area_length - handle_length) */
   int handle_length;           /* The length of the browser handle */
   int *pstart, *pend, *pwidth; /* Pointers to locations in the object
                                   struct, depending on the direction of
                                   the browser */
   int *prel, *prew;
   int width;                   /* The width of the browsing object */
} t_browsebar;

#define DRAW_BB_FRAME(bmp, x1, y1, x2, y2) hline(bmp, x1, y1, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]); \
   vline(bmp, x1, y1, y2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]); \
   hline(bmp, x1, y2, x2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]); \
   vline(bmp, x2, y1, y2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]); \
   rectfill(bmp, x1 + 1, y1 + 1, x2 - 1, y2 - 1, cgui_colors[CGUI_COLOR_BROWSEBAR_BACKGROUND]);

#define DRAW_BB_HANDLE(bmp, x1, y1, x2, y2) hline(bmp, x1, y1, x2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]); \
   hline(bmp, x1, y2, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]); \
   vline(bmp, x1, y1 + 1, y2 - 1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]); \
   vline(bmp, x2, y1 + 1, y2 - 1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]); \
   rectfill(bmp, x1 + 1, y1 + 1, x2 - 1, y2 - 1, cgui_colors[CGUI_COLOR_BROWSEBAR_HANDLE_BACKGROUND]);

/* Draws the vertical browsebar including the handle. */
static void DrawVerticalBrowseBar(t_object *b)
{
   t_browsebar *bb;
   int x1, y1, x2, y2;
   BITMAP *bmp;

   bmp = b->parent->bmp;
   if (bmp == NULL)
      return;
   bb = b->appdata;
   DRAW_BB_FRAME(bmp, b->x1, b->y1, b->x2, b->y2);

   /* draw handle */
   x1 = b->x1 + 1;
   x2 = b->x2 - 1;
   y1 = bb->handle_pos + b->y1 + 1;
   y2 = y1 + bb->handle_length - 1;
   if (y2 >= b->y2) {
      y2 = b->y2 - 1;
      y1 = y2 - (bb->handle_length - 1);
   }
   if (y1 < b->y1 + 1)
      y1 = b->y1 + 1;
   DRAW_BB_HANDLE(bmp, x1, y1, x2, y2);
}

/* Draws the horizontal browsebar including the handle. */
static void DrawHorizontalBrowseBar(t_object *b)
{
   t_browsebar *bb;
   int x1, y1, x2, y2;
   BITMAP *bmp;

   bmp = b->parent->bmp;
   if (bmp == NULL)
      return;
   bb = b->appdata;
   vline(bmp, b->x2 + 1, b->y1, b->y2, cgui_colors[CGUI_COLOR_BROWSEBAR_BACKGROUND]); /* ### */
   /* draw frame */
   DRAW_BB_FRAME(bmp, b->x1, b->y1, b->x2, b->y2);

   /* draw handle */
   y1 = b->y1 + 1;
   y2 = b->y2 - 1;
   x1 = bb->handle_pos + b->x1 + 1;
   x2 = x1 + bb->handle_length - 1;
   if (x2 >= b->x2) {
      x2 = b->x2 - 1;
      x1 = x2 - (bb->handle_length - 1);
   }
   if (x1 < b->x1 + 1)
      x1 = b->x1 + 1;
   DRAW_BB_HANDLE(bmp, x1, y1, x2, y2);
}

/* `newpos' is the mouse location on the browsebar */
static int MoveHandle(t_browsebar *bb, int newpos)
{
   t_object *b = bb->b;
   int pos, movement_length, handle_pos;

   bb = b->appdata;
   /* justify according to the mouse pos of the handle */
   handle_pos = MAX(newpos - bb->moffs, 0);
   movement_length = bb->sliding_area_length - bb->handle_length;
   if (handle_pos > movement_length) {
      bb->handle_pos = movement_length;
      pos = bb->scrolled_area_length - bb->view_port_length;
   } else {
      /* scale the position to a pos on the scrolled area */
      pos = handle_pos * bb->scrolled_area_length / bb->sliding_area_length;
      if (pos >= bb->scrolled_area_length - bb->view_port_length) {
         pos = bb->scrolled_area_length - bb->view_port_length;
         bb->handle_pos = movement_length;
      } else
         bb->handle_pos = handle_pos;
   }
   if (pos != *bb->pos) {
      *bb->pos = pos;
      return 1;
   }
   return 0;
}

static int BrowserGrip(int newpos, t_browsebar *bb, int reason)
{
   switch (reason) {
   case SL_STARTED:
      /* if cursor is on handle, preserve relative position on it */
      if (newpos >= bb->handle_pos && newpos <= bb->handle_pos + bb->handle_length - 1) {
         bb->moffs = newpos - bb->handle_pos;
         break;
      } else                    /* set middle of handle */
         bb->moffs = bb->handle_length / 2;
   case SL_PROGRESS:
      if (MoveHandle(bb, newpos)) {
         bb->b->tf->Refresh(bb->b);
         bb->CallBack(bb->data);
      }
      break;
   case SL_STOPPED:             /* button up */
      break;
   case SL_OVER:
      return 0;
      break;
   default:
      return 0;
   }
   return 1;
}

static int VerticalBrowserGrip(int x nouse, int y, void *data, int id nouse,
                      int reason)
{
   t_browsebar *bb = data;
   return BrowserGrip(y - bb->b->y1 + 1, bb, reason);
}

static int HorizontalBrowserGrip(int x, int y nouse, void *data, int id nouse,
                      int reason)
{
   t_browsebar *bb = data;
   return BrowserGrip(x - bb->b->x1 + 1, bb, reason);
}

extern void SetBrowseBarSize(t_browsebar *bb, int view_port_length, int bblen)
{
   bb->view_port_length = view_port_length;
   if (bblen>0) {
      *bb->pend = *bb->pstart + bblen - 1;
      bb->sliding_area_length = bblen - 2;      /* 2 for the frame */
      if (bb->sliding_area_length < 3)
         bb->sliding_area_length = 3;
      if (bb->scrolled_area_length < view_port_length)
         *bb->pos = 0;
      else if (*bb->pos > bb->scrolled_area_length - view_port_length)
         *bb->pos = bb->scrolled_area_length - view_port_length;
      NotifyBrowseBar(bb, bb->scrolled_area_length);
   }
}

static void SetSize(t_object *b)
{
   t_browsebar *bb;

   if (b->dire)
      b->x1 = b->y1 = 0;
   bb = b->appdata;
   *bb->pend = *bb->pstart + bb->sliding_area_length + *bb->prel + 2 - 1;
   *bb->pwidth = bb->width + *bb->prew - 1;
}

/* This function should be used by the browsed object (list) to notify the
   browser that the browsing state may have been change, i.e. the total
   number of elements in the list may have been changed or the start index
   in a page may have been changed */
extern int NotifyBrowseBar(t_browsebar *bb, int scrolled_area_length)
{
   double scale;

   bb->scrolled_area_length = scrolled_area_length;
   if (scrolled_area_length == 0 || scrolled_area_length <= bb->view_port_length) {
      bb->handle_length = bb->sliding_area_length;
      bb->handle_pos = 0;
      bb->b->inactive = 1;
      *bb->pos = 0;
   } else {
      scale = (double) bb->sliding_area_length / scrolled_area_length;
      bb->handle_length = MAX(bb->view_port_length * scale + 0.5, 1);
      bb->handle_pos = MIN(*bb->pos * scale + 0.5, bb->sliding_area_length - bb->handle_length);
      bb->b->inactive = 0;
      bb->handle_length = MAX(bb->handle_length, 20);
   }
   return bb->b->inactive == 0;
}

/* This function creates a vertical browsebar. When the user clicks on a new
   segment of the browsebar, this will be drawn as selected and the CallBack
   will be executed to notify the object that is browsed. It is intended for
   lists, but may as well work for any object.
   x,y - the upperleft corner of the browser
   length - the desired length of the browse-bar itself
   pixel-height of the object, will give you a general browser.
   CallBack - Will be notified by one call each time the user makes a
   selection that will change pos (e.g browsing)
   data - the data transparently passed to user The browsebar needs to
   be "kicked on" with an initial NotifyBrowseBar to be visible. */
static t_browsebar *CreateBrowseBar(int x, int y, int *pos, int *id,
                        int (*Grip)(int, int, void *, int, int),
                        void (*CallBack) (void *data), void *data)
{
   t_object *b;
   t_browsebar *bb;

   b = CreateObject(x, y, opwin->win->opnode);
   *id = b->id;
   b->inactive = 1;
   bb = GetMem0(t_browsebar, 1);
   bb->pos = pos;
   SetObjectSlidable(b->id, Grip, LEFT_MOUSE, bb);
   b->click = 0;
   bb->CallBack = CallBack;
   bb->data = data;
   bb->width = BROWSERWIDTH;
   bb->b = b;
   b->appdata = bb;
   return bb;
}

static void SetFunctions(t_typefun *tf, void (*Draw)(t_object *))
{
   *tf = default_type_functions;
   tf->Free = XtendedFree;
   tf->SetSize = SetSize;
   tf->Draw = Draw;
}

extern t_browsebar *CreateVerticalBrowseBar(int x, int y, int *pos,
                    int *id, void (*CallBack) (void *data), void *data)
{
   t_browsebar *bb;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      SetFunctions(&tf, DrawVerticalBrowseBar);
   }
   bb = CreateBrowseBar(x, y, pos, id, VerticalBrowserGrip, CallBack, data);
   bb->b->tf = &tf;
   bb->pstart = &bb->b->y1;
   bb->pend = &bb->b->y2;
   bb->pwidth = &bb->b->x2;
   bb->prel = &bb->b->rey;
   bb->prew = &bb->b->rex;
   return bb;
}

extern t_browsebar *CreateHorizontalBrowseBar(int x, int y, int *pos,
                    int *id, void (*CallBack) (void *data), void *data)
{
   t_browsebar *bb;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      SetFunctions(&tf, DrawHorizontalBrowseBar);
   }
   bb = CreateBrowseBar(x, y, pos, id, HorizontalBrowserGrip, CallBack, data);
   bb->b->tf = &tf;
   bb->pstart = &bb->b->x1;
   bb->pend = &bb->b->x2;
   bb->pwidth = &bb->b->y2;
   bb->prel = &bb->b->rex;
   bb->prew = &bb->b->rey;
   return bb;
}
