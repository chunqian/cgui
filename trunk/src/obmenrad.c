/* Module OBMENRAD.C
   This file contains functions for radio-buttons intended for menus */

#include <string.h>
#include <stdarg.h>

#include <allegro.h>
#include "cgui.h"

#include "node.h"
#include "object.h"
#include "menuitem.h"
#include "menu.h"
#include "obmenchk.h"
#include "obmenrad.h"

#define WIDTH_RADIO_INDICATOR 12

static void DrawRadioDot(t_object *b)
{
   int x, y, col;

   x = b->x1 + 2;
   y = b->y1 + 4;
   if (b->inactive)
      col = cgui_colors[CGUI_COLOR_LABEL_INACTIVE_1];
   else
      col = cgui_colors[CGUI_COLOR_LABEL];
   hline(b->parent->bmp, x + 1, y + 5, x + 2, col);
   hline(b->parent->bmp, x, y + 6, x + 3, col);
   hline(b->parent->bmp, x, y + 7, x + 3, col);
   hline(b->parent->bmp, x + 1, y + 8, x + 2, col);
}

static void DrawMenuItemRadio(t_object * b)
{
   t_menuitem *mi;

   if (b->parent->bmp == NULL)
      return;
   mi = b->appdata;
   DrawMenuItem(b);
   if (*mi->indicateval == mi->seqno)
      DrawRadioDot(b);
}

static void MenuRadio(void *data)
{
   t_menuitem *mi = data;

   *mi->indicateval = mi->seqno;
   ItemSelected(mi);
}

static int AddRadioMenuItemHandler(t_object *b, void (*Handler) (void *data),
                          void *data)
{
   t_menuitem *mi;

   do {
      mi = b->appdata;
      mi->AdditionalCB = Handler;
      mi->adddata = data;
      b = b->prev;
   } while (b && mi->seqno);
   return 1;
}

/* Application interface: */

extern int MkMenuRadio(int *selvar, int n, ...)
{
   static t_typefun tf;
   t_menuitem *mi = NULL;
   t_object *b = NULL;
   va_list va;
   int i;
   char *text;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      tf = default_menu_item_type_functions;
      tf.Draw = DrawMenuItemRadio;
      tf.SetSize = SetCheckSize;
      tf.AddHandler = AddRadioMenuItemHandler;
   }
   va_start(va, n);
   for (i = 0; i < n; i++) {
      text = va_arg(va, char *);

      b = MkMenuObject(text, "");
      if (b == NULL)
         return 0;
      b->Action = MenuRadio;
      b->tf = &tf;
      mi = b->appdata;
      mi->indicateval = selvar;
      mi->seqno = i;
   }
   if (b == NULL)
      return -1;
   if (mi)
      mi->menu->imw1 = MAX(mi->menu->imw1, WIDTH_RADIO_INDICATOR);
   va_end(va);
   return b->id;
}
