/* CGUI Example program showing some advanced topics about grouping objects
   by use of the `direction commands' and containers in conjunction. */
#include <allegro.h>
#include "cgui.h"

static void show_code(void *data nouse)
{
   int id;
   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/13direct.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "~Close", CloseWin, NULL);
   DisplayWin();
}

static void close_direct(void *data nouse)
{
   CloseWin(NULL);
   StopProcessEvents();
}

static void dummy_func(void *data nouse)
{
}

static void make_object_window(void *dummy nouse)
{
   static char string1[300 + 1], string2[300 + 1], string3[300 + 1],
               string4[300 + 1], string5[300 + 1], string6[300 + 1];
   static int selvar1 = 0, selvar2 = 0, selvar3 = 0, selvar4 = 0, selvar5 = 0,
          selvar6 = 0;

   MkDialogue(ADAPTIVE, "\"Direction commands\"", 0);
   AutoHotKeys(0); /* Turn off - there are to many - the seach will be inifnite */

   StartContainer(DOWNLEFT, ADAPTIVE, "", CT_BORDER);
   AddTextBox(TOPLEFT, "In the topmost of the two containers below, one "
                         "button is bigger because its label is commanded "
                         "to be drawn in two rows. In the bottommost, the "
                         "FILLSPACE and VERTICAL flags are set so that all "
                         "buttons become as high as the container (which "
                         "in turn is first streched to be large enough to "
                         "contain all its objects). You will get a new "
                         "window if you click any of the buttons", 400, 0, 0);

   StartContainer(DOWNLEFT, ADAPTIVE, "", 0);
   AddButton(TOPLEFT, "One row button", make_object_window, NULL);
   AddButton(RIGHT, "Two rows_button", make_object_window, NULL);
   AddButton(RIGHT, "Another button", make_object_window, NULL);
   EndContainer();

   StartContainer(DOWNLEFT, ADAPTIVE, "", 0);
   AddButton(TOPLEFT|FILLSPACE|VERTICAL, "One row button", make_object_window, NULL);
   AddButton(RIGHT|FILLSPACE|VERTICAL, "Two rows_button", make_object_window, NULL);
   AddButton(RIGHT|FILLSPACE|VERTICAL, "Another button", make_object_window, NULL);
   EndContainer();
   EndContainer();

   StartContainer(DOWNLEFT, ADAPTIVE, "", CT_BORDER);
   AddTextBox(TOPLEFT, "In the leftmost of the two containers below, "
                         "the buttons are of different width because of "
                         "their different label texts. In the next one a "
                         "more uniform layout has been achieved by setting "
                         "the EQUALWIDTH flag. As a side effect, buttons "
                         "with very short labels will be larger and easier "
                         "to hit. Nothing will happen if you click any of "
                         "the buttons.", 400, 0, 0);

   StartContainer(DOWNLEFT, ADAPTIVE, "", 0);
   AddButton(TOPLEFT, "OK", dummy_func, NULL);
   AddButton(RIGHT, "Cancel", dummy_func, NULL);
   EndContainer();

   StartContainer(RIGHT|ALIGNRIGHT, ADAPTIVE, "", 0);
   AddButton(TOPLEFT|EQUALWIDTH, "OK", dummy_func, NULL);
   AddButton(RIGHT|EQUALWIDTH, "Cancel", dummy_func, NULL);
   EndContainer();
   EndContainer();

   StartContainer(DOWNLEFT, ADAPTIVE, "", CT_BORDER);
   AddTextBox(TOPLEFT, "In the leftmost of the two containers below, "
                         "the edit boxes are just utputted in sequence. In "
                         "the rightmost one they are right aligned (using "
                         "the ALIGNRIGHT flag) which makes them more "
                         "attractive.", 400, 0, 0);

   StartContainer(DOWNLEFT, ADAPTIVE, "", CT_BORDER);
   AddEditBox(TOPLEFT, 100, "Name:", FSTRING, 300, string1);
   AddEditBox(DOWNLEFT, 100, "Phone:", FSTRING, 300, string2);
   AddEditBox(DOWNLEFT, 100, "Postal address:", FSTRING, 300, string3);
   EndContainer();

   StartContainer(RIGHT, ADAPTIVE, "", CT_BORDER);
   AddEditBox(TOPLEFT|ALIGNRIGHT, 100, "Name:", FSTRING, 300, string4);
   AddEditBox(DOWNLEFT|ALIGNRIGHT, 100, "Phone:", FSTRING, 300, string5);
   AddEditBox(DOWNLEFT|ALIGNRIGHT, 100, "Postal address:", FSTRING, 300,
                                   string6);
   EndContainer();
   EndContainer();

   StartContainer(DOWNLEFT, ADAPTIVE, "", CT_BORDER);
   AddTextBox(TOPLEFT, "In the leftmost of the two containers below, "
                         "the check boxes are just utputted in sequence. In "
                         "the rightmost, they are made with equal width.",
                         400, 0, 0);
   StartContainer(DOWNLEFT, ADAPTIVE, "", 0);
   AddCheck(DOWNLEFT, "Check!", &selvar1);
   AddCheck(DOWNLEFT, "A check button", &selvar2);
   AddCheck(DOWNLEFT, "OK", &selvar3);
   EndContainer();

   StartContainer(RIGHT, ADAPTIVE, "", 0);
   /* Default direction for FILLSPACE is HORIZONTAL */
   AddCheck(DOWNLEFT|FILLSPACE, "Check!", &selvar4);
   AddCheck(DOWNLEFT|FILLSPACE, "A check button", &selvar5);
   AddCheck(DOWNLEFT|FILLSPACE, "OK", &selvar6);
   EndContainer();
   EndContainer();

   AddButton(DOWNLEFT, "\33E~xit", close_direct, NULL);
   AddButton(RIGHT, "Show code", show_code, NULL);
   DisplayWin();
}

int main(void)
{
   InitCgui(1024, 768, 15);
   make_object_window(NULL);
   ProcessEvents();
   return 0;
}
END_OF_MAIN()
