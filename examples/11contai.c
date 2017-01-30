/* CGUI Example program showing the usage of containers for grouping of objects. */
#include <allegro.h>
#include "cgui.h"

static void show_code(void *data)
{
   int id;
   (void)data;
   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/11contai.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "~Close", CloseWin, NULL);
   DisplayWin();
}

static void shut_down(void *data)
{
   (void)data;
   CloseWin(NULL);
   StopProcessEvents();
}

static void dummy_func(void *data)
{
   (void)data;
}

void make_object_window(void)
{
   static char string[300 + 1];
   static int selvar = 0, selvar2 = 1, radiosel = 1, dropsel = 2, intval = 13;
   static const char *strs[] = {"Off", "On"};
   static const char *ddstrs[] = {"dropsel=0", "dropsel=1", "dropsel=2", "dropsel=3", "dropsel=4"};

   MkDialogue(ADAPTIVE, "Containers", 0);
   AddTextBox(TOPLEFT, "Below are two containers, the first with a label "
   "and the second without. Both are ADAPTIVE. The containers are used to "
   "group some random objects", 300, 0, TB_FRAMESINK);

   StartContainer(DOWNLEFT, ADAPTIVE, "A group of controls:", CT_BORDER);
   AddButton(DOWNLEFT, "A button", dummy_func, NULL);
   AddTag(DOWNLEFT, "A tag");
   AddCheck(DOWNLEFT, "A check button", &selvar);
   AddFlip(DOWNLEFT, "A flip button", strs, &selvar2);
   AddDropDownS(DOWNLEFT, 0, "A drop down box", &dropsel, ddstrs, 5);
   MkRadioContainer(DOWNLEFT, &radiosel, R_HORIZONTAL);
   AddRadioButton("radiosel=0");
   AddRadioButton("radiosel=1");
   AddRadioButton("radiosel=2");
   EndRadioContainer();
   EndContainer();

   StartContainer(RIGHT, ADAPTIVE, "", CT_BORDER);
   AddTextBox(TOPLEFT, "Two edit boxes here.", 200, 0, 0);
   AddEditBox(DOWNLEFT, 100, "An edit box", FSTRING, 300, string);
   AddEditBox(DOWNLEFT, 40, "An integer valued edit box", FINT, 0, &intval);
   EndContainer();
   AddButton(DOWNLEFT, "\33E~xit", shut_down, NULL);
   AddButton(RIGHT, "Show code", show_code, NULL);
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
