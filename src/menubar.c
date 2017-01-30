/* Module MENUBAR.C
   This file contains functions for creating a menu-bar. This includes
   the functions for starting/ending the menu-bar creation, and the
   main-menu item function, as well as the single-menu-function.

   A menu bar is not much more than a generaic node: the main difference
   is an adapted hot-key handling. */

#include <string.h>

#include <allegro.h>
#include "cgui.h"

#include "msm.h"
#include "mouse.h"
#include "cgui/mem.h"
#include "obbutton.h"
#include "menubar.h"
#include "menuitem.h"
#include "menu.h"
#include "menbarcm.h"
#include "tabchain.h"
#include "object.h"
#include "node.h"
#include "window.h"
#include "msm.h"
#include "id.h"

#define MS_MENU_BAR_INIT  0
#define MS_MENU_BAR_OVER  1
#define MS_MENU_BAR_OPEN  2

/* A reference to the node that is currently open for input of menu-items.
   This is used as a handle during the input process of menu-items. */
static t_node *menu_bar_build_progr_node;

/* ----------- Menu-bar functions: */

/* Special key event handler: */
static int MenuBarMoveFocusToNextSubObject(t_object *nf, t_object *sf, int scan, int ascii)
{
   (void)ascii;
   switch (scan) {
   case KEY_DOWN:
   case KEY_UP:
      if (sf)
         sf->Action(sf->appdata);
      return 1;
   case KEY_LEFT:
   case KEY_RIGHT:
      return MoveFocusToNextSubObject(nf, sf, scan, ascii);
   }
   return 0;
}

/* Special mouse state machine: */
static int MSMenuBarInit(int x, int y, int over, int mb, t_mousedata * m nouse,
                      t_object * b)
{
   if (over) {
      b->usedbutton = mb;
      if (mb) {
         b->tcfun->MoveFocusTo(b);
         b->Action(b->appdata); /* open menu */
         b->state = MS_MENU_BAR_OPEN;
      } else {
         b->tcfun->MoveFocusTo(b);
         b->state = MS_MENU_BAR_OVER;
      }
      SetClick(b, x, y);
      StopHotKeys();
      return 1;
   } else {
      return 0;
   }
}

static int MSMenuBarOver(int x nouse, int y nouse, int over, int mb,
                      t_mousedata * m, t_object * b)
{
   if (!inProgress()) {
      ResetState(m, b);
      return 0;
   }
   if (over) {
      if (mb) {
         b->tcfun->MoveFocusTo(b);
         b->Action(b->appdata);
         b->state = MS_MENU_BAR_OPEN;
      }
      return 1;
   } else {
      b->state = MS_MENU_BAR_INIT;
      StartHotKeys();
      ReturnState1();
      return 0;
   }
}

static int MSMenuBarOpen(int x nouse, int y nouse, int over, int mb nouse,
                      t_mousedata * m, t_object * b)
{
   if (!inProgress()) {
      ResetState(m, b);
      return 0;
   }
   if (over) {
      return 1;
   } else {
      b->state = MS_MENU_BAR_INIT;
      StartHotKeys();
      ReturnState1();
      return 0;
   }
}

/* ----------- Menu-bar item functions: */

static void CloseAllMenusWrapper(void *data)
{
   t_object *mib = data;
   t_menuitem *mi;

   mi = mib->appdata;
   if (mi->submenu)
      CloseAllMenus(mi->submenu);
}

/* Drops down a menu from a menu-item in a menu-bar */
static void MenuBarItemDropDown(void *data)
{
   t_menuitem *mi = data;
   t_window *win;
   t_node *nd;
   t_object *winob;
   int x, y1, y2;

   nd = mi->b->parent;
   win = nd->win;
   winob = win->node->ob;
   x = winob->x1 + nd->wx + mi->b->x1;
   y1 = winob->y1 + nd->wy + mi->b->y1 + 1;
   y2 = winob->y1 + nd->wy + mi->b->y2 + 1;
   win = MenuBarCatchMouse(CloseAllMenusWrapper, mi->b);
   mi->submenu = MenuDropDown(mi->CallBack, mi->data, win, NULL, mi);
   SetHotKey(mi->submenu->nd->ob->id, CloseAllMenusWrapper, mi->b, KEY_ESC, 27);
   PositionWindow(x, y1, y2, fwin->ob);
   WindowIsFinished();
   fwin->ob->tf->Refresh(fwin->ob);
}

static void DrawMenuBarItem(t_object *b)
{
   BITMAP *bmp;
   int x1, y1, x2, y2;
   t_menuitem *m;

   m = b->appdata;
   bmp = b->parent->bmp;
   if (bmp == NULL)
      return;
   x1 = b->x1;
   y1 = b->y1;
   x2 = b->x2;
   y2 = b->y2;
   rectfill(bmp, b->x1, b->y1, b->x2, b->y2, cgui_colors[CGUI_COLOR_WIDGET_BACKGROUND]);
   if (b->hasfocus) {
      MenuItemFrameIn(bmp, x1, y1, x2, y2);
      ButtonTextC(1, 1, b, b->label);
   } else {
      MenuItemFrameOut(bmp, x1, y1, x2, y2);
      ButtonTextC(0, 0, b, b->label);
   }
}

static void SetSize(t_object * b)
{
   b->x1 = b->y1 = 0;
   b->x2 = text_length(b->font, b->label) + TEXTOFFSETX * 2 - 1;
   b->y2 = _cgui_button1_height - 1;
}

static int MenuBarItemSetFocus(t_object *b)
{
   t_menuitem *mi;

   mi = b->appdata;
   b->hasfocus = 1;
   return 1;
}

static void MenuBarItemUnSetFocus(t_object *b)
{
   t_menuitem *mi;

   mi = b->appdata;
   b->hasfocus = 0;
   b->tf->Refresh(b);
}

/* ---------- The Single Menu functions: */

static void ArrowDown(BITMAP * bmp, t_object * b, int ofs, int col)
{
   int ofsx, ofsy;

   ofsy = (b->y2 + b->y1) / 2;
   ofsx = b->x2 - 8 + ofs;
   hline(bmp, b->x1 + ofsx - 0, b->y1 + ofsy + 3, b->x1 + ofsx + 0, col);
   hline(bmp, b->x1 + ofsx - 1, b->y1 + ofsy + 2, b->x1 + ofsx + 1, col);
   hline(bmp, b->x1 + ofsx - 2, b->y1 + ofsy + 1, b->x1 + ofsx + 2, col);
   hline(bmp, b->x1 + ofsx - 3, b->y1 + ofsy + 0, b->x1 + ofsx + 3, col);
}

static void DrawSingleMenuItem(t_object * b)
{
   BITMAP *bmp;
   int x1, y1, x2, y2, dx, col;

   bmp = b->parent->bmp;
   if (bmp == NULL)
      return;
   x1 = b->x1;
   if (b->im)
      dx = b->im->w + 4;
   else
      dx = 0;
   y1 = b->y1;
   x2 = b->x2;
   y2 = b->y2;
   rectfill(bmp, b->x1, b->y1, b->x2, b->y2, cgui_colors[CGUI_COLOR_WIDGET_BACKGROUND]);
   if (b->inactive)
      col = cgui_colors[CGUI_COLOR_LABEL_INACTIVE_1];
   else if (b->hasfocus) {
      if (b->hasfocus == 2)
         col = cgui_colors[CGUI_COLOR_LABEL_HIDDEN_FOCUS];
      else
         col = cgui_colors[CGUI_COLOR_LABEL_FOCUS];
   } else
      col = cgui_colors[CGUI_COLOR_LABEL];
   if (b->hasfocus) {
      MenuItemFrameIn(bmp, x1, y1, x2, y2);
      ButtonTextL(dx + 1, 1, b, b->label);
      ArrowDown(bmp, b, 1, col);
   } else {
      MenuItemFrameOut(bmp, x1, y1, x2, y2);
      ButtonTextL(dx + 0, 0, b, b->label);
      ArrowDown(bmp, b, 0, col);
   }
   if (b->im) {
      y1 = (b->y1 + b->y2 + 1 - b->im->h) / 2 + b->hasfocus;
      DrawImage(b->im, bmp, b->x1 + 2 + b->hasfocus, y1);
   }
}

static void SetSizeSingle(t_object * b)
{
   SetSize(b);
   b->x2 += 15;
   if (b->im)
      b->x2 += b->im->w + 4;
}


/* ---------- Application interface: */

/* Start the "insertion of items" process. */
extern int MakeMenuBar(void)
{
   if (menu_bar_build_progr_node == NULL) {
      menu_bar_build_progr_node = MkNode(opwin->win->opnode, TOPLEFT, ADAPTIVE, StubOb);
      menu_bar_build_progr_node->topy = menu_bar_build_progr_node->boty =
                                        menu_bar_build_progr_node->xdist = 0;
   }
   return menu_bar_build_progr_node->ob->id;
}

/* EndMenuBar stops the possibility of entering menu-items into the menu */
extern void EndMenuBar(void)
{
   static t_tcfun tcfun;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      tcfun = default_master_tc_functions;
      tcfun.MoveFocusToNextSubObject = MenuBarMoveFocusToNextSubObject;
   }
   if (menu_bar_build_progr_node) {
      CloseNode();
      JoinTabChain(menu_bar_build_progr_node->ob->id);
   }
   menu_bar_build_progr_node->ob->tcfun = &tcfun;
   menu_bar_build_progr_node = NULL;
}


/* MkMenuBarItem adds one menu-item within a menu bar, which is currently
   in building progress. */
extern int MkMenuBarItem(const char *text, void (*CallBack) (void *),
                                                void *data)
{
   t_object *b;
   int textwidth;
   static t_typefun tf;
   static int virgin = 1;
   static struct t_mevent me;
   t_menuitem *mi;
   static t_tcfun menutc;

   if (menu_bar_build_progr_node == NULL)
      return 0;
   if (virgin) {
      virgin = 0;
      SetDefaultStateMachine(&me);
      me.StateHandlers[MS_MENU_BAR_INIT] = MSMenuBarInit;
      me.StateHandlers[MS_MENU_BAR_OVER] = MSMenuBarOver;
      me.StateHandlers[MS_MENU_BAR_OPEN] = MSMenuBarOpen;
      tf = default_menu_item_type_functions;
      tf.Draw = DrawMenuBarItem;
      tf.SetSize = SetSize;
      menutc = default_tabchain_functions;
      menutc.SetFocus = MenuBarItemSetFocus;
      menutc.UnSetFocus = MenuBarItemUnSetFocus;
   }
   b = MkMenuObjectTop(RIGHT, text);
   b->me = &me;
   b->tf = &tf;
   b->tcfun = &menutc;
   textwidth = text_length(b->font, b->label);
   b->x2 = b->x1 + textwidth + (TEXTOFFSETX << 1) - 1;
   b->y2 = b->y1 + _cgui_button1_height - 1;
   b->Action = MenuBarItemDropDown;
   mi = b->appdata;
   mi->CallBack = CallBack;
   mi->data = data;
   return b->id;
}

/* A single call to create one single "stand-alone" menu-item. Implemented
   as a menu-bar with free positioning, containing one item. */
extern int MkSingleMenu(int x, int y, const char *text,
                        void (*CallBack) (void *), void *data)
{
   static int virgin = 1;
   static t_typefun tf;
   t_object *b;
   int id;

   menu_bar_build_progr_node = MkNode(opwin->win->opnode, x, y, ADAPTIVE, StubOb);
   SetDistance(0, 0);
   menu_bar_build_progr_node->topy = menu_bar_build_progr_node->boty =
                                     menu_bar_build_progr_node->xdist = 0;
   id = MkMenuBarItem(text, CallBack, data);
   b = GetObject(id);
   if (virgin) {
      virgin = 0;
      tf = *b->tf;
      tf.Draw = DrawSingleMenuItem;
      tf.SetSize = SetSizeSingle;
   }
   b->tf = &tf;
   EndMenuBar();
   return b->id;
}

/* MkMenu is Obsolete: */
extern int MkMenu(const char *text, void (*CallBack) (void *), void *data)
{
   return MkMenuBarItem(text, CallBack, data);
}
