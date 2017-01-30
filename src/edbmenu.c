/* This file contains the functions that handles the drop-down-menu
   shown when the user presses the right mous button in an active edit-box.
*/

#include <allegro.h>
#include "cgui.h"
#include "cgui/clipwin.h"
#include "obedbox.h"
#include "edbmenu.h"

#define ID_COPY   0
#define ID_PASTE  1
#define ID_CUT    2
#define ID_COPY_W 3
#define ID_PASTE_W 4
#define ID_CUT_W  5
#define ID_NROF   6

typedef struct t_edit_menuitem {
   struct t_editbox *edb;
   int scan,ascii,i;
   char *txt, *keytxt;
} t_edit_menuitem;

static void MouseToKey(void *data)
{
   t_edit_menuitem *em = data;

   EditBoxKeyboardCallback(em->edb, em->scan, em->ascii);
}

static void ComposeEditMenuData(t_edit_menuitem *em, int i, char *txt,
            char *keytxt, int scan, int ascii, struct t_editbox *edb)
{
   em[i].txt = txt;
   em[i].keytxt = keytxt;
   em[i].scan = scan;
   em[i].ascii = ascii;
   em[i].edb = edb;
   em[i].i = i;
}

extern void CreateEditBoxMenu(struct t_editbox *edb, int isclipped, int ismarked)
{
   int id[ID_NROF], i;
   static t_edit_menuitem ems[ID_NROF];
   t_edit_menuitem *em;

   ComposeEditMenuData(ems, ID_COPY, "Copy", "Ctrl-ins", KEY_INSERT, 2, edb);
   ComposeEditMenuData(ems, ID_PASTE, "Paste", "Shift-ins", KEY_INSERT, 1, edb);
   ComposeEditMenuData(ems, ID_CUT, "Cut", "Shift-del", KEY_DEL, 1, edb);
   ComposeEditMenuData(ems, ID_COPY_W, "Copy", "Ctrl-C", KEY_C, KEY_C, edb);
   ComposeEditMenuData(ems, ID_PASTE_W, "Paste", "Ctrl-V", KEY_V, KEY_V, edb);
   ComposeEditMenuData(ems, ID_CUT_W, "Cut", "Ctrl-X", KEY_X, KEY_X, edb);
   for (i = 0, em = ems; i < 3; i++, em++)
      id[em->i] = MkMenuItem(0, em->txt, em->keytxt, MouseToKey, em);
#ifdef DJGPP
   { int ok;
   MkGroove();
   for (; i < 6; i++, em++)
      id[i] = MkMenuItem(0, em->txt, em->keytxt, MouseToKey, em);
   ok = CheckWinClip();
   if (ok == 1)
      DeActivate(id[ID_PASTE_W]);
   else if (ok == 0) {
      DeActivate(id[ID_COPY_W]);
      DeActivate(id[ID_PASTE_W]);
      DeActivate(id[ID_CUT_W]);
   }
   }
#endif
   if (!isclipped)
      DeActivate(id[ID_PASTE]);
   if (!ismarked) {
      DeActivate(id[ID_COPY]);
      DeActivate(id[ID_CUT]);
      DeActivate(id[ID_COPY_W]);
      DeActivate(id[ID_CUT_W]);
   }
}
