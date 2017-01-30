/* Module MENU.C
   This file contains the functions for creating menus */

#include <allegro.h>

#include "cgui.h"
#include "cgui/mem.h"

#include "mouse.h"
#include "window.h"
#include "node.h"
#include "object.h"
#include "msm.h"
#include "menuitem.h"
#include "tabchain.h"
#include "menu.h"

t_menu *current_open_menu;

/* ---------- Adapted event behaviour */

/* Mouse-events */
static int CurMouseFocusSubmenu(t_mousedata *m, struct t_node *hitnode nouse)
{
   if (msmf1.isset)
      return msmf1.b->me->MouseEv(m, msmf1.b);
   if (msmf2.isset)
      return msmf2.b->me->MouseEv(m, msmf2.b);
   return 1;
}

static int MouseEvMenuWin(t_mousedata *m, t_object *b0)
{
   t_object *b;

   if (b0->node != fwin)
      b0->tcfun->MoveFocusTo(b0);
   for (b = b0->node->last; b; b = b->prev) {
      if (!IsWindow(b)) {
         if (b->me->MouseEv(m, b))
            return 1;
      }
   }
   return 1;
}

/* Key-events */
static int MoveLeft(t_object *nf)
{
   t_menu *menu;

   menu = nf->appdata;
   if (menu->up)
      CloseCurrentMenu(menu);
   else {
      CloseAllMenus(menu);
      return 0;
   }
   return 1;
}

static int MoveRight(t_object *sf)
{
   t_menuitem *mi;

   mi = sf->appdata;
   if (mi) {
      if (mi->issub) {
         sf->Action(sf->appdata);
      } else {
         CloseAllMenus(mi->menu);
         return 0;
      }
   }
   return 1;
}

static int MenuMoveFocusToNextSubObject(t_object *nf, t_object *sf, int scan, int ascii)
{
   (void)ascii;
   switch (scan) {
   case KEY_DOWN:
   case KEY_UP:
      return MoveFocusToNextSubObject(nf, sf, scan, ascii);
   case KEY_LEFT:
      if (nf && MoveLeft(nf))
         return 1;
      else {
         simulate_keypress((scan<<8) | ascii);
         simulate_keypress((KEY_ENTER<<8) | '\r');
         return 1;
      }
   case KEY_RIGHT:
      if (sf && MoveRight(sf))
         return 1;
      else {
         simulate_keypress((scan<<8) | ascii);
         simulate_keypress((KEY_ENTER<<8) | '\r');
         return 1;
      }
      return 1;
   }
   return 0;
}

/* Drawing (the frame of the menu, i.e. window) */
static void DrawMenu(t_object * b)
{
   int x = 0, y = 0, x2, y2;
   t_node *nd;
   BITMAP *bmp;
   int width, height;

   nd = b->node;
   bmp = nd->bmp;
   if (bmp == NULL)
      return;
   width = b->x2 - b->x1 + 1;
   height = b->y2 - b->y1 + 1;
   x2 = width - 1;
   y2 = height - 1;
   hline(bmp, x, y, x2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   vline(bmp, x, y + 1, y2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   hline(bmp, x + 1, y2, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   vline(bmp, x2, y + 1, y2 - 1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   rect(bmp, x + 1, y + 1, x2 - 1, y2 - 1, cgui_colors[CGUI_COLOR_CONTAINER]);
}

/* Clean up all menus including removing the catcher. Memory is freed the normal way. */
extern void CloseAllMenus(void *data)
{
   t_menu *menu = data;
   _CguiCloseWindow(menu->topwin);
}

/* This function will close sub-menus of the menu-
   item at the same level as b (i.e. closing a menu dropped down under a
   sister to b). If working correct this should not be a menu of b itself and
   there shall be at most one.
   Note! This is not same as CloseCurrentMenu: It may occure that the one
   closed below has an arbitrary number of children.
   Return non-zero if submenus was removed or was not there. */
extern int CloseOtherSubs(t_menu *menu, t_menuitem *req)
{
   t_menuitem *mi;

   mi = menu->down;
   if (mi) {
      if (mi->submenu) {
         if (mi == req) {
            menu = req->submenu;
            mi = menu->down;
            if (mi && mi->submenu) {
               _CguiCloseWindow(mi->submenu->nd->win);
               mi->submenu = NULL;
               menu->down = NULL;
               mi->b->tf->Refresh(mi->b);
               mi->b->hasfocus = 0;
            }
            return 0;
         } else {
            _CguiCloseWindow(mi->submenu->nd->win);
            mi->submenu = NULL;
            menu->down = NULL;
            mi->b->tf->Refresh(mi->b);
            mi->b->hasfocus = 0;
         }
      }
   }
   return 1;
}

/* This function will close the latest openend menu (which is also the one
   in focus). If this happens to be the only, then cleanup all. */
extern void CloseCurrentMenu(void *data)
{
   t_menu *menu = data;

   if (menu->up == NULL)
      CloseAllMenus(menu);
   else {
      menu->up->down->submenu = NULL;
      menu->up->down = NULL;
      _CguiCloseWindow(menu->nd->win);
   }
}

static int MenuTakeHotKey(t_object *b, int scan, int key)
{
   t_object *sf;
   t_window *w;
   struct t_tabchain *tc;

   if (b) {
      w = b->parent->win;
      tc = w->tc;
      sf = SubFocusOfLink(tc);
      if (sf == NULL)
         return 0;
      if ((isEnter(scan, key) || isCtrlEnter(scan, key)) && !b->hasfocus) {
         ;
      } else {
         return default_node_hotkey_functions.TakeHotKey(b, scan, key);
      }
   }
   return 0;
}

static void DummyMenuAlternative(void *data nouse)
{
   /* used just to make the menu non-empty */
}

static int MenuMoveFocusTo(t_object *b)
{
   t_menu *menu;

   menu = b->appdata;
   if (menu->down) {
      b->tcfun->SetFocus(b);
      _CguiCloseWindow(menu->down->menu->nd->win);
   }
   return 1;
}

/* Used to calcualte a suitable position for a menu relative to a menu-item */
extern void PositionWindow(int x, int y1, int y2, t_object *b)
{
   if (x + b->x2 > SCREEN_W)
      x = SCREEN_W - b->x2;
   if (x < 0)
      x = 0;
   if (y2 + b->y2 > SCREEN_H) {
      if (y1 - b->y2 < 0) {     /* then it won't fit above either */
      } else {
         y2 = y1 - b->y2;
      }
   }
   b->x1 += x;
   b->x2 += x;
   b->y1 += y2;
   b->y2 += y2;
}

static void MenuFree(t_object *b)
{
   t_menu *menu;

   menu = b->appdata;
   menu->Free(b); /* Run default Free-er */
   Release(menu);
}

/* Performs the basic work to drop down a menu: Opens a window where the
   menu items can be put, and call the application to put the items. */
extern t_menu *MenuDropDown(void (*CallBack)(void*), void *data, t_window *win,
       t_menu *prevm, struct t_menuitem *mi)
{
   static int virgin = 1;
   static struct t_mevent wme;
   static t_hkfun nhk;
   static t_tcfun tcfun;
   static t_typefun tf;
   t_node *nd;
   t_menu *m;

   if (virgin) {
      virgin = 0;
      SetNodeStateMachine(&wme);
      wme.MouseEv = MouseEvMenuWin;
      wme.CurMouseFocus = CurMouseFocusSubmenu;
      nhk = default_node_hotkey_functions;
      nhk.TakeHotKey = MenuTakeHotKey;
      tcfun = default_master_tc_functions;
      tcfun.MoveFocusTo = MenuMoveFocusTo;
      tcfun.MoveFocusToNextSubObject = MenuMoveFocusToNextSubObject;
      tf = default_node_type_functions;
      tf.Free = MenuFree;
   }

   nd = MakeWin(ADAPTIVE, NULL, W_NOMOVE|W_FLOATING);
   opwin->Draw = DrawMenu;
   JoinTabChain(nd->ob->id);
   nd->ob->parent->ob->me = &wme;
   nd->ob->hkfunc = &nhk;
   nd->ob->tcfun = &tcfun;
   nd->ob->appdata = current_open_menu = m = GetMem0(t_menu, 1);
   m->Free = nd->ob->tf->Free;
   nd->ob->tf = &tf;
   m->nd = nd;
   m->imw1 = m->imw2 = 0;
   m->topwin = win;
   m->up = prevm;
   if (prevm)
      prevm->down = mi;

   if (CallBack)
      CallBack(data);
   if (nd->firstob == NULL) /* ... no items added? insert a dummy item */
      MkMenuItem(0, "       ", "", DummyMenuAlternative, NULL);
   Complete(fwin);
   InitializeFocus(fwin->win->tc);
   current_open_menu = NULL;
   return m;
}

/* --------- Application interface: */
extern int HookMenuClose(void (*CloseHook) (void *), void *data)
{
   if (current_open_menu == NULL)
      return 0;
   HookExit(current_open_menu->nd->ob->id, CloseHook, data);
   return 1;
}
