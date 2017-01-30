/* CGUI Example program. This example shows simple usage of list-boxes */
#include <string.h>
#include <allegro.h>
#include "cgui.h"

#define MAXITEMS 15

int list_id;

static void show_info(void *data)
{
   (void)data;
   MkDialogue(ADAPTIVE, "Info window", W_FLOATING);
   AddTextBox(TOPLEFT, "This examples shows how to use a list-box in a simpe way._ _"
      "The three small functions above main are typical for any list. Even if "
      "the list-box is used to display a lot more complex data structures the "
      "code of these will often remain small._ _"
      "Often one need list-boxes to display a variable amount of objects. "
      "That is shown in the example `22drag'._"
      "List-boxes are used also in example `14resize' to show in some other "
      "context._ _"
      "If you want to find out how to use yet more features of list-boxes, "
      "you can have a look into the code of the file-browser in the `cgui/src'"
      "directory.", 400, 0, TB_FRAMESINK|TB_LINEFEED_);
   AddButton(DOWNLEFT, "Close", CloseWin, NULL);
   DisplayWin();
}

static void show_code(void *data)
{
   int id;
   (void)data;

   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/10lists.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "Close", CloseWin, NULL);
   DisplayWin();
}

static void quit(void *data)
{
   (void)data;
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

static void click_action(int id, void *rowdata)
{
   char *string = rowdata;
   char s[1000];
   sprintf(s, "You clicked on: %s| OK ", string);
   Req("", s);
}

static void increase_items(void *data)
{
   int *n = data;
   if (*n < MAXITEMS) {
      (*n)++;
      Refresh(list_id);
   }
}

int main(void)
{
   char *my_data[MAXITEMS] = {"List-box row 1", "List-box row 2", "List-box row 3",
   "List-box row 4", "List-box row 5", "List-box row 6", "List-box row 7",
   "List-box row 8", "List-box row 9", "List-box row 10", "List-box row 11",
   "List-box row 12", "List-box row 13", "List-box row 14", "List-box row 15"};
   int id, n = 10, nr_rows = 8;

   InitCgui(1024, 768, 15);
   MkDialogue(ADAPTIVE, "Simple list-boxes example", 0);
   AddButton(TOPLEFT, "\33E~xit", quit, NULL);
   AddButton(RIGHT, "Show code", show_code, NULL);
   AddButton(RIGHT, "Example info", show_info, NULL);
   AddTag(DOWNLEFT, "The height is enough to show all items, no need for browsing:");
   list_id = AddList(DOWNLEFT, my_data, &nr_rows, 300, LEFT_MOUSE, row_text_creater, click_action, 10);
   SetIndexedList(list_id, index_creater);

   AddButton(RIGHT, "Add one_list item", increase_items, &nr_rows);
   AddTag(DOWNLEFT, "Not all items can be shown at the same time, the browsing object is active:");
   id = AddList(DOWNLEFT, my_data, &n, 240, LEFT_MOUSE, row_text_creater, click_action, 5);

   SetIndexedList(id, index_creater);
   DisplayWin();
   ProcessEvents();
   return 0;
}
END_OF_MAIN()
