/* CGUI Example program showing the usage of tab window. */
#include <allegro.h>
#include "cgui.h"

static void show_code(void *data nouse)
{
   int id;

   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/12tabwin.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "Close", CloseWin, NULL);
   DisplayWin();
}

static void shut_down(void *data nouse)
{
   StopProcessEvents();
}

static void first_tab(void *data nouse, int id nouse)
{
   AddTag(TOPLEFT, "This is the first tab");
   AddButton(DOWNLEFT, "Show code", show_code, NULL);
}

static void example_info(void *data nouse, int id nouse)
{
   AddTextBox(TOPLEFT, "This example shows how to use tabs. Any number of "
   "tabs can be used, but they will all be put on the same row._"
   "This may be seen as a limitation, but on the other hand it encourages "
   "you to design better user interfaces, its not user friendly to make "
   "multiple rows of tabs. And it is definitely not good to make them scrollable."
   "The point with tab-windows is to have all the selections viewed at the same "
   "time.", 400, 0, TB_FRAMESINK|TB_LINEFEED_);
}

static void last_tab(void *data, int id)
{
   id = AddEditBox(TOPLEFT, 50, "The tab number:", FINT, 2, data);
   DeActivate(id);
   AddButton(DOWNLEFT, "\33Another_e~xit button", shut_down, NULL);
}

static void make_window(void)
{
   static int tabsel = 1;
   int id;

   MkDialogue(ADAPTIVE, "Simple tab window", 0);
   id = CreateTabWindow(TOPLEFT, ADAPTIVE, &tabsel);
   AddTab(id, first_tab, NULL, "First tab");
   AddTab(id, example_info, NULL, "Example info tab");
   AddTab(id, last_tab, &tabsel, "Last tab");
   AddButton(DOWNLEFT, "\33E~xit", shut_down, NULL);
   DisplayWin();
}

int main(void)
{
   InitCgui(1024, 768, 15);
   make_window();
   ProcessEvents();
   return 0;
}
END_OF_MAIN()
