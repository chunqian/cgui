/* Module OBMENCHK.C
   This file contains functions for the check object-type for menus */

#include <string.h>
#include <stdarg.h>

#include <allegro.h>
#include "cgui.h"

#include "object.h"
#include "menuitem.h"
#include "menu.h"
#include "obcheck.h"
#include "obmenchk.h"

#define WIDTH_CHECK_INDICATOR 12

static void DrawMenuCheck(t_object *b)
{
   t_menuitem *mi;

   mi = b->appdata;
   DrawMenuItem(b);
   if (*mi->indicateval)
      DrawCheckMark(b, b->x1 + 4, TEXTY(b, 2));
}

extern void ItemSelected(t_menuitem *mi)
{
   void (*CallBack) (void*);
   void *data;

   CallBack = mi->AdditionalCB;
   data = mi->adddata;
   CloseAllMenus(mi->menu);
   if (CallBack)
      CallBack(data);
}

static void MenuCheck(void *data)
{
   t_menuitem *mi = data;

   *mi->indicateval = *mi->indicateval ? 0 : 1;
   ItemSelected(mi);
}

extern void SetCheckSize(t_object * b)
{
   t_menuitem *mi;

   mi = b->appdata;
   b->y1 = b->x1 = 0;
   b->x2 = b->x1 + text_length(b->font, b->label) + TEXTOFFSETX * 2 - 1 +
           mi->menu->imw1 + mi->menu->imw2;
   b->y2 = _cgui_button1_height - 1;
}

/* Application interface: */

extern int MkMenuCheck(int *checkvar, const char *text)
{
   static t_typefun tf;
   t_menuitem *mi;
   t_object *b;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      tf = default_menu_item_type_functions;
      tf.Draw = DrawMenuCheck;
      tf.SetSize = SetCheckSize;
   }
   b = MkMenuObject(text, "");
   if (b == NULL)
      return 0;
   b->tf = &tf;
   b->Action = MenuCheck;
   mi = b->appdata;
   mi->indicateval = checkvar;
   mi->menu->imw1 = MAX(mi->menu->imw1, WIDTH_CHECK_INDICATOR);
   return b->id;
}
