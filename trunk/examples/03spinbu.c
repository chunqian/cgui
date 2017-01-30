/* CGUI Example program showing the usage of a spin-button.
   Except the 2 code-lines for creating the 2 spin-buttons, They may also
   be associated with other type of objects. */
#include <allegro.h>
#include "cgui.h"

static void shut_down(void *data)
{
   (void)data;
   StopProcessEvents();
}

void make_object_window(void)
{
   static int dropsel = 2, intval = 13;
   static const char *ddstrs[] = {"dropsel=0", "dropsel=1", "dropsel=2", "dropsel=3", "dropsel=4"};
   int id;

   MkDialogue(ADAPTIVE, "Spin buttons", 0);
   id = AddDropDownS(DOWNLEFT, 0, "A drop down box with spinbuttons", &dropsel, ddstrs, 5);
   HookSpinButtons(id, &dropsel, 1, 1, 0, 4);
   id = AddEditBox(DOWNLEFT, 40, "An edit box with spinbuttons", FINT, 0, &intval);
   HookSpinButtons(id, &intval, 1, 10, 13, 253);
   AddButton(DOWNLEFT, "\33E~xit", shut_down, NULL);
   DisplayWin();
}

int main(void)
{
   InitCgui(1024, 768, 15);
   make_object_window();
   ProcessEvents();
   return 0;
}
END_OF_MAIN()
