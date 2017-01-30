/* Module OBCHECK Contains the functions for handling a check-box-object.
   This object type consists of two parts, one is the "hot" area where the
   user can click. This is looks similar to a push-button. The other is a
   small area to the left indicating the current status of the application
   data. */
#include <string.h>

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "obbutton.h"
#include "obedbox.h"
#include "obflip.h"
#include "obheader.h"
#include "mstates.h"
#include "obcheck.h"
#include "menuitem.h"

typedef struct t_checkbox {
   void (*Action) (void *);
   void *cbdata;
   int *sel;                    /* A pointer to the application data that
                                   contains the status */
   struct t_object *b;                /* Pointer back to the button */
} t_checkbox;

#define MAXCHECKX 16

extern void DrawCheckMark(t_object *b, int x, int y)
{
   int col;
   BITMAP *bmp;

   bmp = b->parent->bmp;
   if (b->inactive)
      col = cgui_colors[CGUI_COLOR_LIGHTENED_BORDER];
   else
      col = cgui_colors[CGUI_COLOR_LABEL];
   if (y > b->y2 - 3 - 8)
      y = b->y2 - 3 - 8;
   line(bmp, x, y + 5, x + 2, y + 7, col);
   line(bmp, x + 1, y + 5, x + 3, y + 7, col);
   line(bmp, x + 3, y + 6, x + 7, y + 2, col);
   line(bmp, x + 4, y + 6, x + 7, y + 3, col);
}

static void DrawCheckMarkBox(t_object * b)
{
   BITMAP *bmp;
   int x1, y1, x2, y2;

   bmp = b->parent->bmp;
   x1 = b->x1;
   x2 = b->x1 + MAXCHECKX + 1;
   y1 = b->y1;
   y2 = b->y2;
   rect(bmp, x1 + 1, y1 + 1, x2 - 2, y2 - 1, cgui_colors[CGUI_COLOR_WIDGET_BACKGROUND]);
   rect(bmp, x1 + 2, y1 + 2, x2 - 2, y2 - 2, cgui_colors[CGUI_COLOR_WIDGET_BACKGROUND]);
   rectfill(bmp, x1 + 4, y1 + 4, x2 - 4, y2 - 4, cgui_colors[CGUI_COLOR_CHECKBOX_MARK_BACKGROUND]);
   DrawThinFrame(bmp, x1+3, y1+3, x2 - 3, y2 - 3);
}

static void DrawCheckBox(t_object *b)
{
   t_checkbox *ch = b->appdata;
   int x1;

   if (b->parent->bmp == NULL)
      return;
   x1 = b->x1 + MAXCHECKX;
   if (b->state == MS_FIRST_DOWN) {
      DrawLeftSidedImage(b, x1 + 1, b->x2, 1);
      MenuItemFrameIn(b->parent->bmp, x1, b->y1 + 1, b->x2 - 1, b->y2 - 1);
   } else {
      DrawLeftSidedImage(b, x1 + 1, b->x2, 0);
      MenuItemFrameOut(b->parent->bmp, x1, b->y1 + 1, b->x2 - 1, b->y2 - 1);
   }
   DrawCheckMarkBox(b);            /* always do to rub */
   rect(b->parent->bmp, b->x1, b->y1, b->x2, b->y2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   if (*ch->sel)
      DrawCheckMark(b, b->x1 + 5, TEXTY(b, 2));
}

static void Check(void *data)
{
   t_checkbox *ch = data;

   FlipValue(ch->sel);
   ch->b->tf->Refresh(ch->b);
   if (ch->Action)
      DelayedCallBack(ch->Action, ch->cbdata);
}

static int CheckAddHandler(t_object * b, void (*Handler) (void *data),
                           void *data)
{
   t_checkbox *ch;

   ch = b->appdata;
   ch->cbdata = data;
   ch->Action = Handler;
   return 1;
}

static void SetSize(t_object * b)
{
   int textwidth, h;

   if (b->dire)
      b->x1 = b->y1 = 0;
   textwidth = text_length(b->font, b->label);
   h = _cgui_button1_height - 1 + b->rey;
   b->x2 = b->x1 + textwidth + (TEXTOFFSETX << 1) - 1 + MAXCHECKX + b->rex;
   b->y2 = b->y1 + h;
   if (b->im) {
      b->x2 += b->im->w + 1;
      if (b->im->h + 4 > h)
         b->y2 = b->y1 + b->im->h + 4;
   }
}

/* Adapts the desired entries in the function table that are specific for the
   "check-object" type. */
static void SetFunctions(t_typefun * tf)
{
   *tf = default_type_functions;
   tf->Free = XtendedFree;      /* deletes the "application data" of the
                                   object, which in this case is the
                                   check-data structure */
   tf->Draw = DrawCheckBox;
   tf->AddHandler = CheckAddHandler;
   tf->SetSize = SetSize;
}

extern int AddCheck(int x, int y, const char *label, int *sel)
{
   t_object *b;
   t_checkbox *ch;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      SetFunctions(&tf);
   }
   b = CreateObject(x, y, opwin->win->opnode);
   InsertLabel(b, label);
   b->tf = &tf;
   b->Action = Check;
   ch = GetMem0(t_checkbox, 1);
   ch->sel = sel;
   ch->b = b;
   b->appdata = ch;
   if (b->tablink == NULL)
      b->tf->DoJoinTabChain(b);
   return b->id;
}
