/* Module TABS.C
   Contains functions for handling of tabs in "tab-windows" */

#include <string.h>

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "tabwin.h"
#include "obbutton.h"
#include "tabchain.h"
#include "id.h"
#include "tabs.h"

static void DrawTab(t_object *b)
{
   t_node *nd;
   t_tab *t;
   int x1, y1, x2, y2;

   if (b->parent->bmp == NULL)
      return;
   nd = b->parent;
   t = b->appdata;
   x1 = b->x1;
   y1 = b->y1;
   x2 = b->x2;
   y2 = b->y2;
   if (!t->selected)
      y1++;
   /* ----- upper frame top 2 pixellines */
   hline(nd->bmp, x1 + 2, y1, x2 - 2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   hline(nd->bmp, x1 + 1, y1 + 1, x2 - 1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);

   /* left pixelcolumns */
   rectfill(nd->bmp, x1, y1 + 2, x1 + 1, y2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   /* right pixelcolumns */
   rectfill(nd->bmp, x2 - 1, y1 + 2, x2, y2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   /* the corners: upper left */
   hline(nd->bmp, x1, y1, x1 + 1, cgui_colors[CGUI_COLOR_CONTAINER]);
   putpixel(nd->bmp, x1, y1 + 1, cgui_colors[CGUI_COLOR_CONTAINER]);
   /* upper right */
   hline(nd->bmp, x2 - 1, y1, x2, cgui_colors[CGUI_COLOR_CONTAINER]);
   putpixel(nd->bmp, x2, y1 + 1, cgui_colors[CGUI_COLOR_CONTAINER]);

   /* Tab background */
   if (t->selected) {
      /* Join the selected tab with the client are by hiding the top frame part of the client area's border. */
      rectfill(nd->bmp, x1 + 2, y1 + 2, x2 - 2, y2, cgui_colors[CGUI_COLOR_SELECTED_TAB]);
   } else {
      /* Unselected tab */
      hline(nd->bmp, x1, y1 - 1, x2, cgui_colors[CGUI_COLOR_CONTAINER]);
      rectfill(nd->bmp, x1 + 2, y1 + 2, x2 - 2, y2 - 2, cgui_colors[CGUI_COLOR_UNSELECTED_TAB]);
      rectfill(nd->bmp, x1, y2 - 1, x2, y2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   }

   if (t->selected)
      ButtonTextC(1, 2, b, b->label);
   else
      ButtonTextC(1, 3, b, b->label);
}

static void SetSize(t_object *b)
{
   b->x1 = b->y1 = 0;
   b->x2 = text_length(b->font, b->label) + TABSPACE * 2;
}

static t_tabwin *current_tabwin;
extern void HookLeaveTab(void (*callback) (void *data), void *data)
{
   if (current_tabwin) {
      current_tabwin->Leave = callback;
      current_tabwin->leave_data = data;
   }
}

extern void MakeTabWin(t_object *b)
{
   t_tabwin *tw, *tmp;
   t_tab *t;
   int id;

   t = b->appdata;
   tw = t->tw;
   if (t->callback) {
      tmp = current_tabwin;
      current_tabwin = tw;
      id = opwin->win->opnode->ob->id;
      SelectContainer(tw->app->ob->id);
      t->callback(t->data, t->b->id);
      SelectContainer(id);
      current_tabwin = tmp;
   }
}

static void TabSelected(void *data)
{
   t_tab *t = data;

   SetTabIndex(t->tw, t->b);
   t->tw->new_tab_selected = 1;
   t->tw->head->ob->tf->Refresh(t->tw->head->ob);
   ReMakeTabWin(t->tw);
}

static t_tabwin *GetTabWin(int id)
{
   t_node *nd;

   nd = GetNode(id);
   if (nd && nd->type == NODETYPE_TAB)
      return nd->data;
   return NULL;
}

static void RemoveTab(t_object *b)
{
   t_tabwin *tw;
   t_tab *t;

   t = b->appdata;
   tw = t->tw;
   t->Unlink(b);
   t->Free(b);
   ReMakeTabWin(tw);
   DisplayNode(tw->head);
}

static void TabSetFocus(t_object *sf)
{
   t_tab *t;
   t_tabwin *tw;

   t = sf->appdata;
   tw = t->tw;
   if (tw->Leave) {
      tw->Leave(tw->leave_data);
      tw->Leave = NULL;
   }
   TabSelected(t);
}

/* Application interface: */
extern int AddTab(int id, void (*callback) (void *data, int mb),
                  void *data, const char *label)
{
   t_tabwin *tw;
   t_object *b;
   t_tab *t;
   static t_typefun tf;
   static int virgin = 1;
   int first;

   tw = GetTabWin(id);
   if (tw) {
      first = tw->head->firstob == NULL;
      t = GetMem0(t_tab, 1);
      b = CreateObject(RIGHT, tw->head);
      InsertLabel(b, label);
      if (virgin) {
         virgin = 0;
         tf = default_type_functions;
         tf.Draw = DrawTab;
         tf.Free = XtendedFree;
         tf.Remove = RemoveTab;
         tf.SetSize = SetSize;
         tf.SetFocus = TabSetFocus;
      }
      b->click = LEFT_MOUSE | RIGHT_MOUSE;
      t->b = b;
      b->Action = TabSelected;
      b->appdata = t;
      b->y2 = tw->tabh - 1;
      t->Free = b->tf->Free;
      t->Unlink = b->tf->Unlink;
      t->selected = 0;
      t->tw = tw;
      t->data = data;
      t->callback = callback;
      b->tf = &tf;
      b->tcfun = &default_slave_tc_functions;
      return b->id;
   }
   return 0;
}
