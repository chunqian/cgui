/* CGUI Example program showing the usage of images as icons for various objects. */
#include <allegro.h>
#include "cgui.h"

static void show_code(void *data nouse)
{
   int id;
   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/18icons.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "~Close", CloseWin, NULL);
   DisplayWin();
}

static void shut_down(void *data nouse)
{
   CloseWin(NULL);
   StopProcessEvents();
}

static void dummy_func(void *data nouse)
{
}

static void open_menu(void *data nouse)
{
   MkMenuItem(0, "#tools2;Settings", "", dummy_func, NULL);
   MkMenuItem(0, "#printer;Print", "", dummy_func, NULL);
   MkMenuItem(0, "#start_browse;Browse", "", dummy_func, NULL);
}

static void make_object_window(void)
{
   static char string[300 + 1];
   static int radiosel = 1, dropsel = 1;
   static const char *ddstrs[] = {"#find;Find", "#open;Open", "#start_browse;Browse"};

   MkDialogue(ADAPTIVE, "Icons usage", 0);
   MkSingleMenu(TOPLEFT, "#find;Menu-test", open_menu, NULL);
   AddTextBox(DOWNLEFT, "This example shows the usage of images as icons for "
   "various object types. As you can see most objects can get icons in "
   "addition to the label. If you put all your icons in a data file, you "
   "only need one single call to load them all._"
   "Also the rows in lists can show icons. You can see how it looks in the "
   "examples that shows the file browser. The usage is the same, just put "
   "'#iconname;' at the beginning of the string._ _"
   "Maybe I don't need to mention that I have put a poor energy in making "
   "the icons...", 400, 0, TB_FRAMESINK|TB_LINEFEED_);

   StartContainer(DOWNLEFT, ADAPTIVE, "Buttons with and without text:", CT_BORDER);
   AddButton(DOWNLEFT, "#open;", dummy_func, NULL);
   AddButton(RIGHT, "#open;Open", dummy_func, NULL);
   EndContainer();

   StartContainer(DOWNLEFT, ADAPTIVE, "A \"tag\" (that's how to draw a plain image):", CT_BORDER);
   AddTag(RIGHT, "#sound;");
   EndContainer();


   StartContainer(DOWNLEFT, ADAPTIVE, "A drop down box:", CT_BORDER);
   cgui_drop_down_list_row_spacing = 2;
   AddDropDownS(DOWNLEFT, 0, "#tools", &dropsel, ddstrs, 3);
   EndContainer();

   StartContainer(DOWNLEFT, ADAPTIVE, "A group of radio buttons:", CT_BORDER);
   MkRadioContainer(DOWNLEFT, &radiosel, R_HORIZONTAL);
   AddRadioButton("#find;");
   AddRadioButton("#open;");
   AddRadioButton("#start_browse;");
   EndRadioContainer();
   EndContainer();

   StartContainer(DOWNLEFT, ADAPTIVE, "An editbox with icon as label:", CT_BORDER);
   AddEditBox(DOWNLEFT, 100, "#sound;", FSTRING, 300, string);
   EndContainer();

   AddButton(DOWNLEFT, "\33E~xit", shut_down, NULL);
   AddButton(RIGHT, "Show code", show_code, NULL);
   DisplayWin();
}

int main(void)
{
   InitCgui(1024, 768, 15);
   CguiLoadImage("examples.dat#icons", "", 1, 0);
   make_object_window();
   ProcessEvents();
   return 0;
}
END_OF_MAIN()
