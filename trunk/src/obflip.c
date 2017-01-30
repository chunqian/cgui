/* Module OBFLIP.C

   Contains functions for creating a flip-object. This is an object with an
   interface identical to the drop-down box The difference is that only two
   alternative texts are available, and therefore the displayed text will
   immediately flip over to the other alternative instead of dropping down
   the list for selection. The first of the texts corresponds to the value 0
   and the other 1. */

#include <string.h>
#include "cgui.h"

#include "object.h"
#include "obdrop.h"
#include "obflip.h"
#include "id.h"

extern void FlipValue(int *ind)
{
   if (*ind > 0)
      *ind = 0;
   else
      *ind = 1;
}

static void Flip(void *data)
{
   t_dropbox *dd = data;

   FlipValue(dd->sel);
   dd->b->tf->Refresh(dd->b);
   if (dd->Action)
      dd->Action(dd->actdata);
}

static void CalcFlip(const void *data, int i, char *s)
{
   const char *const*strs = data;

   strcpy(s, strs[i]);
}

extern int AddFlip(int x, int y, const char *label, const char *const*strs, int *sel)
{
   t_object *b;
   int id;

   id = AddDropDown(x, y, 0, label, sel, strs, 2, CalcFlip);
   b = GetObject(id);
   b->Action = Flip;
   if (b->tablink == NULL)
      b->tf->DoJoinTabChain(b);
   return b->id;
}
