/* Module OBDROP.C
   Contains functions for creating drop-down boxes */

#include <string.h>
#include <allegro.h>

#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "obdrop.h"
#include "obedbox.h"
#include "obbutton.h"
#include "gencm.h"
#include "menu.h"
#include "list.h"
#include "mstates.h"
#include "id.h"

#if ALLEGRO_SUB_VERSION == 0
#define textout_ex(bmp, f, s, x, y, col, bgcol) textout(bmp, f, s, x, y, col)
#define set_clip_rect set_clip
#endif

int cgui_drop_down_list_row_spacing;
#define MAXBOXCHARS 100

typedef struct t_ddrow {
   char text[MAXBOXCHARS];
   const void *approw;
   struct t_dropbox *dd;
} t_ddrow;

static char ddtext[10000];

static t_ddrow *MakeDDrow(struct t_dropbox *dd, const void *approw)
{
   t_ddrow *ddrow;
   dd->garbage = ResizeMem(t_ddrow, dd->garbage, ++dd->ngarbage);
   ddrow = dd->garbage+dd->ngarbage-1;
   ddrow->dd = dd;
   ddrow->approw = approw;
   strncpy(ddrow->text, ddtext, MAXBOXCHARS);
   ddrow->text[MAXBOXCHARS-1];
   return ddrow;
}

/* This wrapper is needed to conform to the standard list requirements of iterator call back in 
   case of indexed list, at the same time give the application a simple an suitable callback interface. */
static void *IndexWrapper(void *data, int i)
{
   t_dropbox *dd = data;
   dd->RowCallBack(dd->listdata, i, ddtext);
   return MakeDDrow(dd, NULL);
}

/* Similar in case of linked list. */
void *IterateWrapper(void *listdata, void *it)
{
   t_dropbox *dd = listdata;
   t_ddrow *ddrow = it;
   const void *approw;
   static char text[10000];
   if (it == NULL) {
      approw = NULL;
   } else {
      approw = ddrow->approw;
   }
   approw = dd->Iterate(dd->listdata, approw, ddtext);
   if (approw == NULL) {
      return  NULL;
   } else {
      return MakeDDrow(dd, approw);
   }
}

static int GetMaxwidth(t_dropbox *dd)
{
   int i, width;
   t_ddrow *ddrow;

   width = 0;
   if (dd->RowCallBack) {
      for (i = 0; i < *dd->np; i++) {
         ddrow = IndexWrapper(dd, i);
         width = MAX(text_length(dd->b->font, ddrow->text), width);
      }
   } else {
      for (ddrow = IterateWrapper(dd, NULL); ddrow; ddrow = IterateWrapper(dd, ddrow)) {
         width = MAX(text_length(dd->b->font, ddrow->text), width);
      }
   }
   width += 4;
   return width;
}

static void Close2Win(void *data)
{
   (void)data;
   _CguiCancel();
   _CguiCancel();
   StopProcessEvents();
}

static void ListItemSelected(int id, void *obj)
{
   t_ddrow *di = obj;
   t_dropbox *dd;

   dd = di->dd;
   *dd->sel = GetListIndex(id);
   Close2Win(NULL);
   dd->b->tf->Refresh(dd->b);
   if (dd->Action)
      DelayedCallBack(dd->Action, dd->actdata);
}

static void DrawDropBox(t_object *b)
{
   t_dropbox *dd = b->appdata;
   int drawcol, bcol, x, y;
   unsigned u, i;
   t_image *im;
   char *s, *p, c;
   BITMAP *bmp;
   t_ddrow *ddrow;

   bmp = b->parent->bmp;
   if (bmp == NULL)
      return;
   DrawBoxFrame(b);

   /* Determine background and drawing colours from current state */
   if (b->state == MS_FIRST_DOWN) {
      bcol = cgui_colors[CGUI_COLOR_EDITBOX_BACKGROUND_MARK];
      drawcol = cgui_colors[CGUI_COLOR_EDITBOX_TEXT_MARK];
   } else if (b->inactive) {
      bcol = cgui_colors[CGUI_COLOR_EDITBOX_BACKGROUND_INACTIVE];
      drawcol = cgui_colors[CGUI_COLOR_EDITBOX_TEXT_INACTIVE];
   } else {
      bcol = cgui_colors[CGUI_COLOR_EDITBOX_BACKGROUND];
      drawcol = cgui_colors[CGUI_COLOR_EDITBOX_TEXT];
   }

   x = b->x1 + b->dx1 + 1;
   y = b->y1 + TEXTOFFSETY;

   /* Clean everything */
   rectfill(bmp, x, y, b->x2 - INPUTBORDER - 1, b->y2 - INPUTBORDER - 1, bcol);

   s = "";
   if (dd->RowCallBack) {
      if (*dd->np > 0) {
         u = *dd->sel;
         if (u >= (unsigned) *dd->np)
            u = *dd->np - 1;
         ddrow = IndexWrapper(dd, u);
         s = ddrow->text;
      }
   } else {
      i = 0;
      for (ddrow = IterateWrapper(dd, NULL); ddrow; ddrow = IterateWrapper(dd, ddrow)) {
         if (i==*dd->sel) {
            s = ddrow->text;
         }
         ++i;
      }
      dd->n = i;
   }
   set_clip_rect(bmp, x, y, b->x2 - INPUTBORDER - 1, b->y2-INPUTBORDER - 1);
   if (s[0] == '#' && s[1] != '#') {
      /* Draw a (possible) image */
      for (p = ++s; *s && *s != ';'; s++);
      c = *s;
      *s = 0;
      im = GetImage(b->parent, p);
      *s = c;
      if (*s)
         s++;
      if (im) {
         DrawImage(im, bmp, x, y);
         x += im->w + 1;
      }
   }
   /* Draw the (possible) text */
   textout_ex(bmp, b->font, s, x, y, drawcol, -1);
   set_clip_rect(bmp, 0, 0, bmp->w - 1, bmp->h - 1);
}

static int DrawRow(void *rowdata, char *s)
{
   t_ddrow *ddrow = rowdata;
   strcpy(s, ddrow->text);
   return 0;
}

static void DoDropDown(void *data)
{
   t_dropbox *dd = data;
   int n, id, required_y, x, y1, y2, rh, center_y, winy, winx, width,
       vspace, multirow;
   t_object *b = dd->b;
   t_node *nd;

   nd = b->parent;
   winx = nd->win->node->ob->x1;
   winy = nd->win->node->ob->y1;
   /* Recalcualte the witdth and the string since they may be dynamically calulated. */
   width = GetMaxwidth(dd);
   rh = cgui_drop_down_list_row_spacing + text_height(CGUI_list_font) + 1;
   if (rh < 1)
      rh = 10;
   if (width < 48)
      width = 48;
   n = *dd->np;
   required_y = n * rh + 12;
   center_y = b->y1 + nd->wy + winy + ((b->y2 - b->y1) >> 1);
   /* if object is above screen-mid, put the drop-win below, else above */
   y2 = b->y2 + nd->wy + winy;
   y1 = b->y1 + nd->wy + winy;
   /* justify the size to make it all fit onto the screen */
   if (center_y < (SCREEN_H >> 1)) {
      while (y2 + required_y > SCREEN_H - rh)
         required_y -= rh;
   } else {
      while (y1 - required_y < rh)
         required_y -= rh;
   }
   n = required_y / rh;
   if (n < 2)
      n = 2;
   x = b->x1 + b->dx1 + nd->wx + winx - 1;
   CatchMouseWindow(Close2Win, NULL);
   nd = MakeWin(ADAPTIVE, NULL, W_NOMOVE|W_FLOATING);
   if (nd == NULL)
      return;
   SetHotKey(nd->ob->id, Close2Win, NULL, KEY_ESC, 27);

   vspace = CGUI_list_vspace;
   CGUI_list_vspace = cgui_drop_down_list_row_spacing;
   multirow = cgui_list_no_multiple_row_selection;
   cgui_list_no_multiple_row_selection = 1;
   SetDistance(0, 0);
   id = AddList(TOPLEFT, dd, dd->np, width, LEFT_MOUSE, DrawRow, ListItemSelected, n);
   if (dd->RowCallBack) {
      SetIndexedList(id, IndexWrapper);
   } else {
      SetLinkedList(id, IterateWrapper);
   }
   CGUI_list_vspace = vspace;
   cgui_list_no_multiple_row_selection = multirow;

   Complete(fwin);
   PositionWindow(x, y1, y2, fwin->ob);
   WindowIsFinished();
   fwin->ob->tf->Refresh(fwin->ob);
   BrowseTo(id, *dd->sel, 0);
   ProcessEvents();
}

static int AddDDHandler(t_object * b, void (*cb) (void *), void *data)
{
   t_dropbox *dd;

   dd = b->appdata;
   dd->Action = cb;
   dd->actdata = data;
   return 1;
}

static void FreeDrop(t_object * b)
{
   t_dropbox *dd;

   dd = b->appdata;
   free(dd->garbage);
   XtendedFree(b);
}

static void SetSize(t_object * b)
{
   t_dropbox *dd;

   dd = b->appdata;
   if (dd->width == 0) {
      dd->width = GetMaxwidth(dd);
   }
   SetBoxSize(b, dd->width);
}

static void DefaultCallBack(const void *data, int i, char *s)
{
   const char *const*strs = data;
   strcpy(s, strs[i]);
}

extern void DropDown(t_dropbox *dd)
{
   DoDropDown(dd);
}

extern t_dropbox *CreateDropDownData(t_object *editbox, int *sel, const char * const *strings, const int *n)
{
   t_dropbox *dd;
   dd = GetMem0(t_dropbox, 1);
   dd->np = n;
   dd->RowCallBack = DefaultCallBack;
   dd->listdata = strings;
   dd->sel = sel;
   dd->b = editbox;
   return dd;
}

static void AddDropDown2(int x, int y, const char *label, t_dropbox *dd, int *sel, const void *listdata, int width)
{
   t_object *b;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      tf = default_type_functions;
      tf.Draw = DrawDropBox;
      tf.AddHandler = AddDDHandler;
      tf.SetSize = SetSize;
      tf.Free = FreeDrop;
   }
   b = CreateObject(x, y, opwin->win->opnode);
   InsertLabel(b, label);
   b->tf = &tf;
   b->appdata = dd;
   b->Action = DoDropDown;
   if (b->tablink == NULL)
      b->tf->DoJoinTabChain(b);
   dd->sel = sel;
   dd->np = &dd->n;
   dd->listdata = listdata;
   dd->width = width;
   dd->b = b;
}

extern int AddDropDown(int x, int y, int width, const char *label, int *sel, const void *listdata, int n, void (*CallBack) (const void *, int, char *))
{
   t_dropbox *dd;
   dd = GetMem0(t_dropbox, 1);
   dd->n = n;
   dd->RowCallBack = CallBack;
   AddDropDown2(x, y, label, dd, sel, listdata, width);
   return dd->b->id;
}

extern int AddDropDownS(int x, int y, int width, const char *label, int *sel, const char * const *strs, int n)
{
   return AddDropDown(x, y, width, label, sel, (void*)strs, n, DefaultCallBack);
}

extern int AddDropDownD(int x, int y, int width, const char *label, int *sel, const void *listdata, 
                        const void *(*Iterate) (const void *listdata, const void *row, char *text))
{
   t_dropbox *dd;
   dd = GetMem0(t_dropbox, 1);
   dd->Iterate = Iterate;
   AddDropDown2(x, y, label, dd, sel, listdata, width);
   return dd->b->id;
}

