/* Module MENUITEM.C
   This file contains the generic parts of menu-items, and also the
   "Scratch-menu" function. */

#include <allegro.h>

#include "cgui.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "obbutton.h"
#include "cgui/mem.h"
#include "msm.h"
#include "menu.h"
#include "mouse.h"
#include "menuitem.h"
#include "tabchain.h"
#include "gencm.h"
#include "obmensel.h"
#include "id.h"

#define MS_MENU_ITEM_DEF_INIT     0
#define MS_MENU_ITEM_DEF_OVER     1
#define MS_MENU_ITEM_DEF_OVERDOWN 2

#if ALLEGRO_SUB_VERSION == 0
#define textout_ex(bmp, f, s, x, y, col, bgcol) textout(bmp, f, s, x, y, col)
#endif

t_typefun default_menu_item_type_functions;

extern void ResetCatcher(t_menuitem *mi)
{
   mi->menu->topwin->node->ob->state = 0;
}

/* ----------- Generic mouse state-machine for menu-items */
static int MSMenuItemDefInit(int x, int y, int over, int mb, t_mousedata *m nouse,
                          t_object * b)
{
   t_menuitem *mi;

   if (over) {
      mi = b->appdata;
      CloseOtherSubs(mi->menu, mi);
      if (mb) {
         b->state = MS_MENU_ITEM_DEF_OVERDOWN;
      } else {
         b->state = MS_MENU_ITEM_DEF_OVER;
      }
      b->usedbutton = mb;
      b->tcfun->SetFocus(b);
      b->tf->Refresh(b);
      b->tcfun->MoveFocusTo(b);
      /* Temporary set state to generic 'over' since we use the generic Over function, which don't know about menu's OVER. Then restore state. */
      b->state = MS_OVER;
      b->Over(b);
      if (mb) {
         b->state = MS_MENU_ITEM_DEF_OVERDOWN;
      } else {
         b->state = MS_MENU_ITEM_DEF_OVER;
      }
      StopHotKeys();
      SetClick(b, x, y);
      return 1;
   } else {
      return 0;
   }
}

static int MSMenuItemDefOver(int x nouse, int y nouse, int over, int mb,
                          t_mousedata *m nouse, t_object * b)
{
   if (over) {
      if (mb)
         b->state = MS_MENU_ITEM_DEF_OVERDOWN;
      return 1;
   } else {
      b->tcfun->UnSetFocus(b);
      b->state = MS_MENU_ITEM_DEF_INIT;
      b->Over(b);
      StartHotKeys();
      ReturnState1();
      return 0;
   }
}

static int MSMenuItemDefOverDown(int x nouse, int y nouse, int over,
                              int mb nouse, t_mousedata *m nouse, t_object * b)
{
   if (over) {
      if (mb) {
         return 1;
      } else {
         ReturnState1();
         b->tcfun->UnSetFocus(b);
         b->state = MS_MENU_ITEM_DEF_INIT;
         b->Over(b);
         StartHotKeys();
         b->Action(b->appdata);
         return 1;
      }
   } else {
      b->tcfun->UnSetFocus(b);
      b->state = MS_MENU_ITEM_DEF_INIT;
      b->Over(b);
      StartHotKeys();
      ReturnState1();
      return 0;
   }
}

/* ------------- Generic drawing routine for menu-items. */
extern void DrawMenuItem(t_object * b)
{
   BITMAP *bmp;
   int x1, y1, x2, y2, col, offs, xoffs;
   t_menuitem *mi;

   mi = b->appdata;
   bmp = b->parent->bmp;
   x1 = b->x1 + mi->menu->imw1;
   y1 = b->y1;
   x2 = b->x2;
   y2 = b->y2;
   if (b->inactive)
      col = cgui_colors[CGUI_COLOR_LABEL_INACTIVE_1];
   else if (b->hasfocus) {
      if (b->hasfocus == 2)
         col = cgui_colors[CGUI_COLOR_LABEL_HIDDEN_FOCUS];
      else
         col = cgui_colors[CGUI_COLOR_LABEL_FOCUS];
   } else
      col = cgui_colors[CGUI_COLOR_LABEL];
   rectfill(bmp, b->x1, b->y1, b->x2, b->y2, cgui_colors[CGUI_COLOR_WIDGET_BACKGROUND]);
   if (b->hasfocus) {
      MenuItemFrameIn(bmp, x1, y1, x2, y2);
      offs = 1;
   } else {
      MenuItemFrameOut(bmp, x1, y1, x2, y2);
      offs = 0;
   }
   ButtonTextL(mi->menu->imw1 - 2 + offs, offs, b, b->label);
   if (mi->issub)
      DrawSubArrow(bmp, x2, y1, offs, col);
   if (mi->sc) {
      /* Right align short-cut key text */
      xoffs = offs - text_length(b->font, mi->sc);
      if (mi->issub)
         offs -= MENU_DROP_ARROW_WIDTH;
      if (b->inactive)
         textout_ex(bmp, b->font, mi->sc, x2 + xoffs + 1, y1 + TEXTOFFSETY + offs + 1, cgui_colors[CGUI_COLOR_LABEL_INACTIVE_2], -1);
      textout_ex(bmp, b->font, mi->sc, x2 + xoffs, y1 + TEXTOFFSETY + offs, col, -1);
   }
}


/* ------------- */
static void FreeMenuItem(t_object *b)
{
   t_menuitem *mi;

   mi = b->appdata;
   if (mi->sc)
      Release(mi->sc);
   mi->Free(b);
   Release(mi);
}

static int AddMenuItemHandler(t_object *b, void (*Handler) (void *data),
                          void *data)
{
   t_menuitem *mi;

   mi = b->appdata;
   mi->AdditionalCB = Handler;
   mi->adddata = data;
   return 1;
}

extern void InitMenuItems(void)
{
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      default_menu_item_type_functions = default_type_functions;
      default_menu_item_type_functions.AddHandler = AddMenuItemHandler;
      default_menu_item_type_functions.Free = FreeMenuItem;
   }
}

/* Creating a generic menu-item object. */
extern t_object *MkMenuObjectTop(int x, int y, const char *text)
{
   static int virgin = 1;
   static struct t_mevent me;
   t_menuitem *mi;
   t_object *b;

   if (virgin) {
      virgin = 0;
      SetDefaultStateMachine(&me);
      me.StateHandlers[MS_MENU_ITEM_DEF_INIT] = MSMenuItemDefInit;
      me.StateHandlers[MS_MENU_ITEM_DEF_OVER] = MSMenuItemDefOver;
      me.StateHandlers[MS_MENU_ITEM_DEF_OVERDOWN] = MSMenuItemDefOverDown;
   }
   b = CreateObject(x, y, opwin->win->opnode);
   InsertLabel(b, text);
   b->me = &me;
   b->tf = &default_menu_item_type_functions;
   mi = GetMem0(t_menuitem, 1);
   mi->menu = current_open_menu;
   mi->b = b;
   mi->Free = default_type_functions.Free;
   b->appdata = mi;
   return b;
}

extern t_object *MkMenuObject(const char *text, const char *shortcut)
{
   t_menuitem *mi;
   t_object *b;

   if (current_open_menu == NULL)
      return NULL;
   b = MkMenuObjectTop(DOWNLEFT | FILLSPACE, text);
   mi = b->appdata;
   if (shortcut && *shortcut)
      mi->sc = MkString(shortcut);
   return b;
}

static void DestroyMenuPointer(void *data)
{
   Release(data);
}

static void CloseAllMenusWrapper(void *data)
{
   t_menu **pmenu = data;

   CloseAllMenus(*pmenu);
}

/* ---------- Application interface: */

/* Creates a menu from the point of the object "id" */
extern int MkScratchMenu(int id, void (*CallBack) (void *), void *data)
{
   t_node *nd = NULL;
   t_object *b;
   t_menu **pmenu;
   t_window *win;
   int x, y1, y2, mz;

   b = GetObject(id);
   if (b)
      nd = b->parent;
   if (nd) {
      x = nd->win->node->ob->x1 + nd->wx + b->x2;
      y1 = nd->win->node->ob->y1 + nd->wy + b->y1;
      y2 = nd->win->node->ob->y1 + nd->wy + b->y2 + 1;
   } else {
      _CguiMousePos(&x, &y2, &mz);
      y1 = y2;
   }
   pmenu = GetMem0(t_menu*, 1);
   win = CatchMouseWindow(CloseAllMenusWrapper, pmenu);
   *pmenu = MenuDropDown(CallBack, data, win, NULL, NULL);
   HookExit((*pmenu)->nd->firstob->id, DestroyMenuPointer, pmenu);
   SetHotKey((*pmenu)->nd->ob->id, CloseAllMenus, *pmenu, KEY_ESC, 27);
   PositionWindow(x, y1, y2, fwin->ob);
   WindowIsFinished();
   fwin->ob->tf->Refresh(fwin->ob);
   return id;
}
