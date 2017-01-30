/* CGUI Example program. This example shows how to make objects stretchable. */
#include <allegro.h>
#include <string.h>
#include "cgui.h"

#define NAME_LEN 100

static void show_code(void *data nouse)
{
   int id;
   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/14resize.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "~Close", CloseWin, NULL);
   DisplayWin();
}

static void show_info(void *data nouse)
{
   MkDialogue(ADAPTIVE, "Info window", W_FLOATING);
   AddTextBox(TOPLEFT, "This examples show how to make objects stretchable "
   "i.e. how to offer the user a tool to re-size an object._"
   "In one window there are two quite meanigless re-sizeble buttons, "
   "just to show..._"
   "The equal-width command in the lower container makes 2, 3 and 4 adopting "
   "the width of the first one. The adaptive container will find it necessary "
   "to grow with its heightening first button, which will give more space for "
   "the other to fill. The minimum size of the leftmost one will however be "
   "limited to that of the largest of the other (as a consequence of the "
   "order of the calculation)._"
   "For edit-boxes: when the length of the text to be entered is not "
   "predictable this may be useful. If the text doesn't fit into the box "
   "it will be scrolled to view the latest charater written. If most of the "
   "texts are too long it will be annoying for the user to not see the "
   "entire string. Re-sizing is then a good complement to the built-in "
   "scrolling - it costs you a single line of code..._ _"
   "For lists: If all the stuff doesn't fit into a listbox, this can of course "
   "be scrolled. For a user it may however be important to see the entire list "
   "(if possible) since it gives a much better overview. And also: the width "
   "of lists are not scrollbale in cgui." , 400, 0, TB_FRAMESINK|TB_LINEFEED_);
   AddButton(DOWNLEFT, "Close", CloseWin, NULL);
   DisplayWin();
}

static void quit(void *data nouse)
{
   StopProcessEvents();
}

static void *index_creater(void *listdata, int i)
{
   char **strings = listdata;
   return strings[i];
}

static int row_text_creater(void *rowdata, char *s)
{
   strcpy(s, rowdata);
   return 0;
}

static void lists_window(void)
{
   int id;
   static int n = 20;
   static char *my_data[] =
   {"List-box row 1", "List-box row 2", "List-box row 3", "List-box row 4",
   "List-box row 5", "List-box row 6", "List-box row 7", "List-box row 8",
   "List-box row 9", "List-box row 10", "List-box row 11", "List-box row 12",
   "List-box row 13", "List-box row 14", "List-box row 15", "List-box row 16",
   "List-box row 17", "List-box row 18", "List-box row 19", "List-box row 20"};

   MkDialogue(ADAPTIVE, "Some list-boxes to re-size", W_FLOATING);

   /* The containers only serves as annotations - they are not necessary */
   StartContainer(DOWNLEFT, ADAPTIVE, "Not Stretchable", CT_BORDER);
   id = AddList(TOPLEFT, my_data, &n, 100, 0, row_text_creater, NULL, 10);
   SetIndexedList(id, index_creater);
   EndContainer();

   StartContainer(RIGHT, ADAPTIVE, "Stretchable", CT_BORDER);
   id = AddList(TOPLEFT, my_data, &n, 100, 0, row_text_creater, NULL, 10);
   SetIndexedList(id, index_creater);
   MakeStretchable(id, NULL, NULL, 0);
   EndContainer();

   AddTag(DOWNLEFT, "Stretchable");
   id = AddList(DOWNLEFT, my_data, &n, 100, 0, row_text_creater, NULL, 10);
   SetIndexedList(id, index_creater);
   MakeStretchable(id, NULL, NULL, 0);

   AddButton(DOWNLEFT, "~Close", CloseWin, NULL);
   DisplayWin();
}

static void dummy(void *data nouse)
{
}

static void resize_buttons_window(void)
{
   int id;
   static char str[NAME_LEN+1];

   MkDialogue(ADAPTIVE, "Some objects to re-size", W_FLOATING);

   StartContainer(DOWNLEFT, ADAPTIVE, "The second button is resizeable", CT_BORDER);
   AddButton(RIGHT, "Dummy", dummy, NULL);
   id = AddButton(RIGHT, "Resize", dummy, NULL);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(RIGHT, "Dummy", dummy, NULL);
   AddButton(RIGHT, "Dummy", dummy, NULL);
   EndContainer();

   StartContainer(DOWNLEFT, ADAPTIVE, "The first button is resizeable", CT_BORDER);
   id = AddButton(RIGHT | EQUALWIDTH| FILLSPACE | VERTICAL, "Resizeable", dummy, NULL);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(RIGHT | EQUALWIDTH| FILLSPACE | VERTICAL, "Dummy", dummy, NULL);
   AddButton(RIGHT | EQUALWIDTH| FILLSPACE | VERTICAL, "Dummy", dummy, NULL);
   AddButton(RIGHT | EQUALWIDTH| FILLSPACE | VERTICAL, "Dummy", dummy, NULL);
   EndContainer();

   id = AddEditBox(DOWNLEFT, 40, "Stretchable edit box:", FSTRING, NAME_LEN, str);
   MakeStretchable(id, NULL, NULL, NO_VERTICAL);

   SetWindowPosition(20, 100);
   DisplayWin();
}

static void main_window(void)
{
   MkDialogue(FILLSCREEN, "Resizing example", 0);
   AddButton(TOPLEFT|EQUALHEIGHT, "~Quit", quit, NULL);
   AddButton(RIGHT|EQUALHEIGHT, "Example_info", show_info, NULL);
   AddButton(RIGHT, "Show_code", show_code, NULL);
   DisplayWin();
}

int main(void)
{
   InitCgui(1024, 768, 15);
   main_window();
   resize_buttons_window();
   lists_window();
   ProcessEvents();
   return 0;
}
END_OF_MAIN()
