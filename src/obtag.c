/* Module OBTAG.C
   Contains functions for creating a tag-object. This is a
   simple object with the text on one row wihtout any frame. It may typically
   be used in conjunction with other objects like radio-buttons, or to give
   an extra information to the user. */

#include <string.h>

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "obbutton.h"
#include "obedbox.h"
#include "tabchain.h"
#include "obtag.h"

static void DisplayTag(t_object *b)
{
   if (b->parent->bmp == NULL)
      return;
   DrawLeftSidedImage(b, b->x1, b->x2, 0);
}

static void SetSize(t_object *b)
{
   int dx, h;

   if (b->dire)
      b->x1 = b->y1 = 0;
   dx = text_length(b->font, b->label);
   b->x2 = b->x1 + dx + 1 + b->rex;
   h = _cgui_button1_height - 1 + b->rey;
   b->y2 = b->y1 + h;
   if (b->im) {
      b->x2 += b->im->w + 1;
      if (b->im->h > h)
         b->y2 = b->y1 + b->im->h;
   }
}

static int MoveFocusToStub(t_object *b nouse)
{
   return 1;
}

/* Application interface: */

extern int AddTag(int x, int y, const char *label)
{
   t_object *b;
   static t_typefun tf;
   static t_tcfun tcfun;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      tf = default_type_functions;
      tf.Draw = DisplayTag;
      tf.SetSize = SetSize;
      tcfun = default_tabchain_functions;
      tcfun.MoveFocusTo = MoveFocusToStub;
   }
   b = CreateObject(x, y, opwin->win->opnode);
   b->Over = StubOb;
  InsertLabel(b, label);
   b->autohk = 0;
   b->tf = &tf;
   b->tcfun = &tcfun;
   return b->id;
}
