/* Module WINDIAL.C
   This file contains functions for creating a dialogue-window of pop-up
   type. */

#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "windial.h"
#include "tabchain.h"

/* Application interface: */

extern int MkDialogue(int width, int height, const char *label, int attr)
{
   t_node *nd;

   if (fwin)
      fwin->ob->tcfun->UnSetFocus(fwin->ob);
   nd = MakeWin(width, height, label, attr);
   if (nd == NULL)
      return 0;
   return opwin->ob->id;
}
