/* CGUI Example program. This example shows how to use the desktop */
#include <allegro.h>
#include "cgui.h"

static void quit(void *data nouse)
{
   StopProcessEvents();
}

static void start_disk_browse(void *data nouse)
{
   FileManager("", FM_BROWSE_DAT);
}

static void open_win(void *data nouse)
{
   MkDialogue(ADAPTIVE, "Dummy window", W_FLOATING);
   AddButton(RIGHT, "Close", CloseWin, NULL);
   DisplayWin();
}

static void show_code(void *data nouse)
{
   int id;

   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/19deskt.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "~Close", CloseWin, NULL);
   DisplayWin();
}

static void close_show_info(void *data)
{
   int *id = data;

   *id = 0;
   CloseWin(NULL);
}

static void show_info(void *data nouse)
{
   static int id;

   if (id) {
      SetFocusOn(id);
   } else {
      id = MkDialogue(ADAPTIVE, "Info window", W_FLOATING);
      AddTextBox(TOPLEFT, "This example shows how the desktop can be used in "
      "cgui. Niether the icons nor the windows has any meaning, except for "
      "showing how it works:_"
      "- You are allowed to put any type of object onto the desktop, but the "
      "point (well, if there is any...) is to put only icons there._"
      "- Icons can be moved freely on the desktop, but they won't work "
      "properly if you try to put them in normal windows._"
      "- Icons can be used for anytning that buttons can be used for._"
      "- To make it possible to select the desktop again without closing a "
      "window, this must be W__FLOATING._"
      "Test: try to open more instances of this info window! Try the same "
      "with the 'dummy window'!_ _"
      "(Sorry, for not making any nice icons.)", 400, 0, TB_FRAMESINK|TB_LINEFEED_);
      AddButton(DOWNLEFT, "Close", close_show_info, &id);
      DisplayWin();
   }
}


int main(void)
{
   int id;
   InitCgui(1024, 768, 15);
   CguiLoadImage("examples.dat#desktop", "", IMAGE_TRANS_BMP, 0);
   AddIcon(ID_DESKTOP,  10,  10, "#show_code;Show code", show_code, NULL);
   AddIcon(ID_DESKTOP, 100, 200, "#show_info;Show example_information", show_info, NULL);
   AddIcon(ID_DESKTOP, 200, 200, "#start_browse;File handler", start_disk_browse, NULL);
   AddIcon(ID_DESKTOP, 200, 400, "#open_win;Open a window", open_win, &id);
   id = AddIcon(ID_DESKTOP, 500, 200, "#exit;E~xit", quit, NULL);
   SetFocusOn(id);
   DisplayWin();
   ProcessEvents();
   return 0;
}
END_OF_MAIN()
