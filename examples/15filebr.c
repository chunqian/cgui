/* CGUI Example program. This example shows how to use the file-selector and the file-browser.*/
#include <allegro.h>
#include <string.h>
#include "cgui.h"

static void quit(void *data nouse)
{
   StopProcessEvents();
}

static void show_code(void *data nouse)
{
   int id;

   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/15filebr.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "~Close", CloseWin, NULL);
   DisplayWin();
}

static void start_file_selector(void *data nouse)
{
   char label[10000] = "File displayed: ";
   const char *fn;
   int id;
   fn = FileSelect("*.txt;*.c", "", FS_DIRECT_SELECT_BY_DOUBLE_CLICK|FS_DISABLE_EDIT_DAT|FS_SHOW_MENU|FS_NO_DRAG_DROP|FS_SHOW_DIR_TREE,
                   "Open a file to view", "Open");
   if (*fn) {
      strcat(label, fn);
      MkDialogue(ADAPTIVE, label, W_FLOATING);
      id = AddTextBox(TOPLEFT, fn, 500, 30, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|8|TB_FIXFONT);
      MakeStretchable(id, NULL, NULL, 0);
      AddButton(DOWNLEFT, "\33Close", CloseWin, NULL);
      DisplayWin();
   }
}

static void start_disk_browse(void *data nouse)
{
   FileManager(NULL, FM_BROWSE_DAT|FS_SHOW_DIR_TREE|FS_MULTIPLE_SELECTION);
}

static void file_menu(void *data nouse)
{
   MkMenuItem(0, "Open a file", "Ctrl-O", start_file_selector, NULL);
   MkMenuItem(0, "Browse the disk", "Ctrl-F", start_disk_browse, NULL);
   MkMenuItem(0, "Show code", "", show_code, NULL);
   MkMenuItem(0, "Quit", "Ctrl-Q", quit, NULL);
}

int main(void)
{
   int id;

   InitCgui(1024, 768, 32);
   id = MkDialogue(FILLSCREEN, "File-selector test", 0);
   SetHotKey(id, start_file_selector, NULL, KEY_O, KEY_O);
   SetHotKey(id, start_disk_browse, NULL, KEY_F, KEY_F);
   SetHotKey(0, quit, NULL, KEY_Q, KEY_Q);
   MkSingleMenu(TOPLEFT, "File", file_menu, NULL);
   AddTextBox(DOWNLEFT, "The file selector can be tested by selecting some text file. You will find such a file right into the examples directory._ _"
   "The file manager is also capable to copy disk entries into data files (i.e. importing data file objects) as well as extracting data file objects "
   "to disk entries. Just drag them with the mouse from inside the data file to the directory where you want to put it.",
   400, 0, TB_FRAMESINK|TB_LINEFEED_);
   DisplayWin();
   ProcessEvents();
   return 0;
}
END_OF_MAIN()
