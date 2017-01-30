/* CGUI Example program showing how to create some basic objects and how
   these look.
   Note! The local variables in `make_object_window' would no longer be
   valid when the function returns if they were not static. Be careful, never
   pass the address of `normal' local varaiables (i.e. auto) to the
   cgui-functions (the only exception are the strings for all the labels
   which will be copied). In most case this will never be a problem
   since you of course prefere using dynamic memory.
*/

#include <allegro.h>
#include "cgui.h"

void make_object_window(void);

static void shut_down(void *data)
{
   (void)data;
   CloseWin(NULL);
   /* Here goes the code that handles the result of the users' actions */
   StopProcessEvents(); /* terminates the program via `return 0;' in main */
}

static void dummy_func(void *data)
{
   (void)data;
   /* Here goes the code that is a respons to the button-click */
   /* This code just starts a new copy of the window. */
   make_object_window();
}

void make_object_window(void)
{
   static char string[200 + 1];
   static int selvar = 0, selvar2 = 1, radiosel = 1, dropsel = 2;
   static const char *strs[] = {"Off", "On"};
   static const char *ddstrs[] = {"dropsel=0", "dropsel=1", "dropsel=2", "dropsel=3", "dropsel=4"};

   MkDialogue(ADAPTIVE, "Some various object types", 0);
   AddButton(TOPLEFT, "A button", dummy_func, NULL);
   AddTag(DOWNLEFT, "A 'tag'");
   AddCheck(DOWNLEFT, "A check button", &selvar);
   AddFlip(DOWNLEFT, "A flip button", strs, &selvar2);
   AddDropDownS(DOWNLEFT, 0, "A drop down box", &dropsel, ddstrs, 5);
   MkRadioContainer(DOWNLEFT, &radiosel, R_HORIZONTAL);
   AddRadioButton("radiosel=0");
   AddRadioButton("radiosel=1");
   AddRadioButton("radiosel=2");
   EndRadioContainer();
   AddEditBox(DOWNLEFT, 100, "An edit box", FSTRING, 200, string);
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
