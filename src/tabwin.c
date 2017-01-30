/* Module TABWIN.C
   Contains functions for handling of "tab-windows" */

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "font.h"
#include "tabchain.h"
#include "tabs.h"
#include "tabwin.h"

static void DrawDummyTab(t_object * b)
{
   t_node *nd;
   t_tabwin *tw;
   int x1, x2, y1, y2;

   if (b->parent->bmp == NULL)
      return;
   nd = b->node;
   tw = nd->data;
   x1 = nd->last->x2 + 1;
   y1 = 0;
   x2 = b->x2;
   y2 = b->y2;
   rectfill(nd->bmp, x1, y1, x2, y2 - 4, cgui_colors[CGUI_COLOR_CONTAINER]);
   hline(nd->bmp, x1, y2 - 3, x2 - 1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   putpixel(nd->bmp, x2, y2 - 1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   hline(nd->bmp, x1, y2 - 2, x2 - 2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   hline(nd->bmp, x2 - 1, y2 - 2, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
}

extern void TabWinDrawer(t_object * b)
{
   t_node *nd;
   t_tabwin *tw;
   int x = 0, y = 0, y1, x2, y2;

   if (b->parent->bmp == NULL)
      return;
   nd = b->node;
   tw = nd->data;
   y1 = tw->head->ob->y2 + 1;
   x2 = b->x2 - b->x1;
   y2 = b->y2 - b->y1;
   rectfill(nd->bmp, x, y1, x + 1, y2 - 1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   rectfill(nd->bmp, x2 - 1, y1, x2, y2 - 2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   hline(nd->bmp, x + 1, y2 - 1, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   hline(nd->bmp, x, y2, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   rectfill(nd->bmp, x, y + 2, x + 1, y1 - 1, cgui_colors[CGUI_COLOR_CONTAINER]);
   rectfill(nd->bmp, x2 - 1, y + 2, x2, y1 - 1, cgui_colors[CGUI_COLOR_CONTAINER]);
   rectfill(nd->bmp, x, y, x2, y + 1, cgui_colors[CGUI_COLOR_CONTAINER]);
   rectfill(nd->bmp, x + 2, y1, x2 - 2, y1 + 1, cgui_colors[CGUI_COLOR_CONTAINER]);
   putpixel(nd->bmp, x + 1, y1 - 1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   putpixel(nd->bmp, x2 - 1, y1 - 1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
}

extern t_object *GetTab(t_tabwin *tw)
{
   int i;
   t_object *b0;

   for (i = 0, b0 = tw->head->firstob; b0; i++, b0 = b0->next) {
      if (i == *tw->index)
         return b0;
   }
   *tw->index = i - 1;
   return tw->head->last;
}

static void SetSize(t_object *b)
{
   t_node *nd;
   t_tabwin *tw;
   int width = 0, height = 0;
   t_object *b0;

   nd = b->node;
   tw = nd->data;
   if (tw->new_tab_selected)
      tw->new_tab_selected = 0; /* Don't re-build it all when just switched tab */
   else if (!tw->orig_fixsize) {
      /* Find the largest needed size by building the windows for all tabs,
      one at a time, take the size and destroy it. This is a waste of
      x-time, but on the other hand: to keep all the windows in memory would
      have been a waste of memory. */
      if (nd->firstob) {
         b0 = GetTab(tw);
         if (b0) {
            nd->ob->x2 = nd->ob->x1 = 0;
            nd->ob->y2 = nd->ob->y1 = 0;
            tw->SetSize(b);
            b->Position(b);
            nd->width = MAX(nd->ob->x2 - nd->ob->x1, nd->width);
            nd->height = MAX(nd->ob->y2 - nd->ob->y1, nd->height);
         }
      } else {
         nd->width = 0;
         nd->height = 0;
         for (b0 = tw->head->firstob; b0; b0 = b0->next) {
            if (nd->firstob)
               CleanNode(nd);
            MakeTabWin(b0);
            nd->ob->x2 = nd->ob->x1 = 0;
            nd->ob->y2 = nd->ob->y1 = 0;
            tw->SetSize(b);
            b->Position(b);
            width = MAX(nd->ob->x2 - nd->ob->x1, width);
            height = MAX(nd->ob->y2 - nd->ob->y1, height);
         }
         nd->ob->x2 = nd->ob->x1 = 0;
         nd->ob->y2 = nd->ob->y1 = 0;
         if (nd->firstob)
            CleanNode(nd);
         nd->fixsize = 1;
         nd->width = width;
         nd->height = height;
      }
   }
   if (nd->last == NULL && nd->r == NULL) {
      b0 = GetTab(tw);
      if (b0) {
         MakeTabWin(b0);
         SetTabIndex(tw, b0);
         SetSubFocusOfLink(b0->tablink, b0);
      }
   }
   tw->SetSize(b);              /* recurse kids */
}

extern void ReMakeTabWin(t_tabwin *tw)
{
   CleanNode(tw->app);
   DisplayNode(tw->app);
}

static void FreeTabWin(t_object * b)
{
   t_node *nd;
   t_tabwin *tw;

   nd = b->node;
   tw = nd->data;
   if (tw->Leave)
      tw->Leave(tw->leave_data);
   tw->Free(b);
   Release(tw);
}

/* Sets specified tab-index to be selected */
extern void SetTabIndex(t_tabwin *tw, t_object *b)
{
   int i;
   t_object *b0;

   for (i = 0, b0 = tw->head->firstob; b0; i++, b0 = b0->next) {
      if (b0 == b) {
         *tw->index = i;
         ((t_tab *) b0->appdata)->selected = 1;
      } else
         ((t_tab *) b0->appdata)->selected = 0;
   }
}

static int TabWinMoveFocusTo(t_object *b)
{
   t_tabwin *tw;

   tw = b->node->data;
   b = tw->head->ob;
   return SetFocusOn(b->id);
}

/* Application interface: */
extern int CreateTabWindow(int x, int y, int width, int height, int *status)
{
   t_tabwin *tw;
   t_node *top, *head, *app;
   static t_typefun tf, apptf;
   static t_tcfun tcfun;
   static int virgin = 1;

   tw = GetMem0(t_tabwin, 1);
   tw->tabh = TABSPACE * 2 + text_height(_cgui_prop_font);
   tw->index = status;

   /* The entire sub-window */
   top = tw->top = MkNode(opwin->win->opnode, x, y, width, height, TabWinDrawer);
   tw->orig_fixsize = top->fixsize;
   SetDistance(2, 2);

   top->data = tw;
   top->ob->tf = &tf;

   /* The node for the part with the tabs (the head). The tabs will be
      inserted later */
   head = tw->head = MkNode(opwin->win->opnode, TOPLEFT | FILLSPACE, ADAPTIVE, DrawDummyTab);
   SetDistance(0, 0);
   head->data = tw;
   head->ob->tf = &tf;
   CloseNode();              /* head The node for the application objects. */
   if (height == AUTOINDICATOR) {
      app = tw->app = MkNode(opwin->win->opnode, DOWNLEFT | FILLSPACE |
            VERTICAL | HORIZONTAL, width, height, StubOb);
   } else {
      /* -4 because:left+right-distance; -6:top,left,between head and main */
      app = tw->app = MkNode(opwin->win->opnode, DOWNLEFT | FILLSPACE |
          VERTICAL | HORIZONTAL, width - 4, height - tw->tabh - 6, StubOb);
   }
   if (virgin) {
      virgin = 0;
      apptf = default_node_type_functions;
      apptf.Free = FreeTabWin;
      apptf.SetSize = SetSize;
      tf = default_node_type_functions;
      tf.MakeNodeRects = StubOb;
      tcfun = default_tabchain_functions;
      tcfun.MoveFocusTo = TabWinMoveFocusTo;
   }
   SetDistance(2, 2);
   app->data = tw;
   /* Save recursive functions for chained call */
   tw->Free = app->ob->tf->Free;
   tw->SetSize = app->ob->tf->SetSize;
   app->ob->tf = &apptf;
   app->ob->tcfun = &tcfun;
   app->type = NODETYPE_TAB;
   head->ob->tf->DoJoinTabChain(head->ob);
   CloseNode();                 /* Application */
   CloseNode();                 /* Tab-win */
   return app->ob->id;
}
