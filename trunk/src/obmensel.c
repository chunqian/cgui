/* Module OBMENSEL.C
   This file contains functions for objects intended for menus. Such an
   object may either be a direct-command-button or a selector for opening a
   new sub-menu */

#include <string.h>
#include <stdarg.h>

#include <allegro.h>
#include "cgui.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "menuitem.h"
#include "menu.h"
#include "msm.h"
#include "obbutton.h"
#include "obmensel.h"
#include "mouse.h"

#define MS_MENU_SEL_INIT    0
#define MS_MENU_SEL_OPEN    1
#define MS_MENU_SEL_ISOPEN  2

/* --------- Special mouse state machine required: */
static int MSMenuItemSelInit(int x, int y, int over, int mb nouse,
                         t_mousedata *m nouse, t_object *b)
{
   t_menuitem *mi;

   if (over) {
      b->tf->Refresh(b);
      mi = b->appdata;
      if (CloseOtherSubs(mi->menu, mi)) {
         b->Action(b->appdata);
      }
      b->usedbutton = mb;
      b->tcfun->MoveFocusTo(b);
      b->state = MS_MENU_SEL_OPEN;
      StopHotKeys();
      SetClick(b, x, y);
      return 1;
   } else {
      return 0;
   }
}

static int MSMenuItemSelOpen(int x, int y, int over, int mb nouse,
                         t_mousedata *m nouse, t_object *b)
{
   if (over) {
      ReturnState1();
      b->state = MS_MENU_SEL_ISOPEN;
      SetDropper(b, x, y);
      return 1;
   } else {
      b->state = MS_MENU_SEL_INIT;
      StartHotKeys();
      ReturnState1();
      ResetCatcher(b->appdata);
      return 0;
   }
}

static int MSMenuItemSelIsOpen(int x nouse, int y nouse, int over,
                           int mb nouse, t_mousedata *m nouse, t_object *b)
{
   if (over) {
      return 1;
   } else {
      b->state = MS_MENU_SEL_INIT;
      StartHotKeys();
      ReturnDropper();
      ResetCatcher(b->appdata);
      return 0;
   }
}

/* --------- Drawing of "select"-menu-items: */

/* Draws the small arrrow that is used to indicate that an menu-item is a
   selctor for a new sub-menu */
extern void DrawSubArrow(BITMAP * bmp, int x2, int y1, int disp, int col)
{
   x2 += disp;
   y1 += disp;
   vline(bmp, x2 - 5, y1 + 7, y1 + 11, col);
   vline(bmp, x2 - 4, y1 + 8, y1 + 10, col);
   putpixel(bmp, x2 - 3, y1 + 9, col);
}

static void DrawMenuItemSelect(t_object * b)
{
   t_menuitem *mi;
   int y1;

   mi = b->appdata;
   if (b->parent->bmp == NULL)
      return;
   DrawMenuItem(b);
   if (b->im) {
      y1 = (b->y1 + b->y2 + 1 - b->im->h) / 2;
      DrawImage(b->im, b->parent->bmp, b->x1, y1);
   }
}

static void PositionMenuItemSelWindow(int x1, int x2, int y, t_object * b)
{
   if (y + b->y2 > SCREEN_H)
      y = SCREEN_H - b->y2;
   if (y < 0)
      y = 0;
   if (x2 + b->x2 > SCREEN_W) {
      if (x1 - b->x2 < 0) {     /* then it won't fit above either */
      } else {
         x2 = x1 - b->x2;
      }
   }
   b->x1 += x2;
   b->x2 += x2;
   b->y1 += y;
   b->y2 += y;
}

static void SubMenuDropDown(void *data)
{
   t_menuitem *mi = data;
   t_menu *menu;
   t_object *winob;
   t_node *nd;
   int x1, x2, y;

   nd = mi->menu->nd;
   winob = nd->win->node->ob;
   x1 = winob->x1 + nd->wx + mi->b->x1;
   x2 = winob->x1 + nd->wx + mi->b->x2;
   y = winob->y1 + nd->wy + mi->b->y1;
   menu = MenuDropDown(mi->CallBack, mi->data, mi->menu->topwin, mi->menu, mi);
   mi->submenu = menu;
   SetHotKey(menu->nd->ob->id, CloseCurrentMenu, menu, KEY_ESC, 27);
   PositionMenuItemSelWindow(x1, x2, y, fwin->ob);
   WindowIsFinished();
   fwin->ob->tf->Refresh(fwin->ob);
}

/* A wrapper to the user-callback for a menu-item of type "direct-selected action" */
static void MenuSelect(void *data)
{
   t_menuitem *mi = data;
   void (*Action) (void *);
   void *calldata;

   Action = mi->CallBack;
   calldata = mi->data;
   CloseAllMenus(mi->menu);
   if (Action)
      Action(calldata);
}

static void SetSize(t_object * b)
{
   t_menuitem *mi;
   int textwidth;

   mi = b->appdata;
   b->x1 = b->y1 = 0;
   textwidth = text_length(b->font, b->label);
   if (mi->sc)
      /* Extra space for the other text and some space between them */
      textwidth += text_length(b->font, mi->sc) + MENU_LABEL_SPACE;
   b->x2 = b->x1 + textwidth + TEXTOFFSETX * 2 - 1 + mi->menu->imw1 +
                                                     mi->menu->imw2;
   b->y2 = _cgui_button1_height - 1;
   if (b->im)
      b->y2 = MAX(b->y2, b->im->h);
}

static void MenuItemSelUnSetFocus(t_object *b)
{
   t_menuitem *mi;

   mi = b->appdata;
//   CloseOtherSubs(mi->menu, mi); FIXME!
}

/* Application interface: */

extern int MkMenuItem(int sub, const char *text, const char *shortcut,
                      void (*CallBack) (void *), void *data)
{
   t_object *b;
   static t_typefun tf;
   t_menuitem *mi;
   static struct t_mevent me;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      tf = default_menu_item_type_functions;
      tf.Draw = DrawMenuItemSelect;
      tf.SetSize = SetSize;
      tf.UnSetFocus = MenuItemSelUnSetFocus;
      SetDefaultStateMachine(&me);
      me.StateHandlers[MS_MENU_SEL_INIT] = MSMenuItemSelInit;
      me.StateHandlers[MS_MENU_SEL_OPEN] = MSMenuItemSelOpen;
      me.StateHandlers[MS_MENU_SEL_ISOPEN] = MSMenuItemSelIsOpen;
   }
   b = MkMenuObject(text, shortcut);
   if (b == NULL)
      return 0;
   mi = b->appdata;
   b->tf = &tf;
   if (b->im)
      mi->menu->imw1 = MAX(mi->menu->imw1, b->im->w+2);
   mi->CallBack = CallBack;
   mi->data = data;
   if (sub) {
      mi->menu->imw2 = MENU_DROP_ARROW_WIDTH;
      b->Action = SubMenuDropDown;
      b->me = &me;
      b->tcfun = &default_slave_tc_functions;
      mi->issub = 1;
   } else {
      b->Action = MenuSelect;
   }
   return b->id;
}
