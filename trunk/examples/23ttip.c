/* CGUI Example program showing how to use tool-tips */
#include <allegro.h>
#include "cgui.h"

static int winid;

static void quit(void *data nouse)
{
   CloseWin(NULL);
   /* Here goes the code that handles the result of the users' actions */
   StopProcessEvents(); /* terminates the program via `return 0;' in main */
}

static void show_code(void *data nouse)
{
   int id;
   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/23ttip.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "~Close", CloseWin, NULL);
   DisplayWin();
}

static void show_info(void *data nouse)
{
   MkDialogue(ADAPTIVE, "Example information", W_FLOATING);
   AddTextBox(DOWNLEFT, "This example shows how to use tool tips._"
   "`SetView' (called when a selection in the options menu is done) is not "
   "actually a tool tip function, but its usage will probably often be related "
   "to the use of tool tips._ _"
   "When seen the tool tips a lot of times, the user may want to turn them "
   "off. This can be arranged like in this example. An alternative is to not "
   "show tool tips at all when the objects are labelled. Yet one possibility "
   "of usage is to always show more expressive text like a small help-text, and "
   "do that regardless of if icons or labels are shown.", 400, 0, TB_FRAMESINK|TB_LINEFEED_);
   AddButton(DOWNLEFT, "~Close", CloseWin, NULL);
   DisplayWin();
}

static void option_checked(void *data)
{
   int *opts = data;
   int option = 0;

   if (opts[0])
      option |= SV_HIDE_LABEL;
   if (opts[1])
      option |= SV_HIDE_ICON;
   if (opts[2])
      option |= SV_NO_TOOLTIP;
   switch (opts[3]) {
   case 0:
      option |= SV_ONLY_BRIEF;
      break;
   case 1:
      option |= SV_ONLY_LONG;
      break;
   case 2:
      option |= SV_PREFERE_BRIEF;
      break;
   case 3:
      option |= SV_PREFERE_LONG;
      break;
   }
   SetView(winid, option);
   DisplayWin();
}

static void file_menu(void *data nouse)
{
   MkMenuItem(0, "Quit", "Ctrl-Q", quit, NULL);
}

static void options_menu(void *data)
{
   int *opt = data;
   int id;

   id = MkMenuCheck(opt + 0, "Hide label");
   AddHandler(id, option_checked, opt);
   id = MkMenuCheck(opt + 1, "Hide icon");
   AddHandler(id, option_checked, opt);
   id = MkMenuCheck(opt + 2, "No tool tips");
   AddHandler(id, option_checked, opt);
   id = MkMenuRadio(opt + 3, 4, "Only use brief tool tip", "Only use long tool tip",
             "Prefere brief tool tip if choice", "Prefere long tool tip if choice");
   AddHandler(id, option_checked, opt);
}

static void windows_menu(void *data nouse)
{
   MkMenuItem(0, "Show code", "", show_code, NULL);
   MkMenuItem(0, "Example info", "", show_info, NULL);
}

static void dummy_func(void *data nouse)
{
}

int main(void)
{
   int opts[4]={0,0,0,3}, id;

   InitCgui(1024, 768, 15);
   CguiLoadImage("examples.dat#icons", "", 1, 0);
   winid = MkDialogue(FILLSCREEN, "Tool tip example", 0);
   MakeMenuBar();
   MkMenuBarItem("File", file_menu, NULL);
   MkMenuBarItem("Options", options_menu, opts);
   MkMenuBarItem("Windows", windows_menu, NULL);
   EndMenuBar();
   AddTag(DOWNLEFT, "The buttons below are just dummies");
   id = AddButton(DOWNLEFT, "#open;Open", dummy_func, NULL);
   ToolTipText(id, "Click the button to open a file");
   id = AddButton(RIGHT, "#find;Search", dummy_func, NULL);
   ToolTipText(id, "Click the button if you_want to to search for something");
   id = AddButton(RIGHT, "#start_browse;Start browsing", dummy_func, NULL);
   ToolTipText(id, "Click the button to start browsing");
   DisplayWin();
   ProcessEvents();
   return 0;
}
END_OF_MAIN()
