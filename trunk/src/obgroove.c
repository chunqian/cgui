/* Module OBGROOVE.C
   This file contains functions for creating a menu groove */

#include <allegro.h>
#include "cgui.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "obgroove.h"

static void DrawVerticalGroove(t_object * b)
{
   if (b->parent->bmp == NULL)
      return;
   rectfill(b->parent->bmp, b->x1, b->y1, b->x2, b->y2, cgui_colors[CGUI_COLOR_CONTAINER]);
   hline(b->parent->bmp, b->x1 + 4, b->y1 + 1, b->x2 - 4, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   hline(b->parent->bmp, b->x1 + 4, b->y1 + 2, b->x2 - 4, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
}

static void SetSize(t_object * b)
{
   b->x1 = b->y1 = b->x2 = 0;
   b->y2 = 4;
}

/* Application interface: Just a delimiter to group stuff visually together */
extern int MkGroove(void)
{
   t_object *b;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      tf = default_type_functions;
      tf.Draw = DrawVerticalGroove;
      tf.SetSize = SetSize;
      virgin = 0;
   }
   b = CreateObject(DOWNLEFT | FILLSPACE, opwin->win->opnode);
   b->tf = &tf;
   b->tf->DeActivate(b);
   return b->id;
}
