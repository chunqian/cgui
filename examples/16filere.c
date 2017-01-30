/* CGUI Example program. This example shows how to use file-type
   registration (in conjuction with the file-selector and
   the file-browser). */
#include <allegro.h>
#include <string.h>
#include <ctype.h>
#include "cgui.h"
#include "cgui/mem.h"

static void quit(void *data nouse)
{
   StopProcessEvents();
}

static void start_disk_browse(void *data nouse)
{
   FileManager("", FM_BROWSE_DAT);
}

static void show_code(void *data nouse)
{
   int id;
   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/16filere.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "~Close", CloseWin, NULL);
   DisplayWin();
}

static void cfg_handler(void *data nouse, char *path)
{
   int id;
   char label[10000] = "File displayed: ";

   strcat(label, path);
   MkDialogue(ADAPTIVE, label, W_FLOATING);
   id = AddTextBox(TOPLEFT, path, 500, 30, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|8|TB_FIXFONT);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "Close", CloseWin, NULL);
   DisplayWin();
}

static void text_pre_viewer(void *privatedata nouse, void *calldata)
{
   int id;

   id = AddTextBox(TOPLEFT, calldata, 300, 30, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|8|TB_FIXFONT);
   MakeStretchable(id, NULL, NULL, 0);
}

static void binary_pre_viewer(void *privatedata nouse, void *calldata)
{
   int id;
   DATAFILE *datob = calldata;
   char *s, *text;
   int i, j;

   if (datob) {
      s = datob->dat;
      if (s) {
         for (i=j=0; i<datob->size; i++) {
            if (s[i] < 32 && !isspace(s[i])) {
               if (i-j > 10)
                  break;
               j = i;
            }
         }
         text = GetMem(char, i-j + 1);
         memmove(text, s+j, i-j);
         text[i-j] = 0;
         id = AddTextBox(TOPLEFT, text, 300, 30, TB_FRAMESINK|TB_PREFORMAT|8|TB_FIXFONT);
         MakeStretchable(id, NULL, NULL, 0);
         Release(text);
      }
   }
}

int main(void)
{
   InitCguiWindowedMode();
   CguiLoadImage("../examples/examples.dat#file_reg", "", IMAGE_TRANS_BMP, 0);
   RegisterFileType("cfg", cfg_handler, NULL, "Inspect", "tools", "Allegro config file", NULL);
   RegisterFileType("ttf", NULL,        NULL, "",        "font",  "True type font file", NULL);
   RegisterFileType("txt", NULL,        NULL, "",        "plain_text", "",               text_pre_viewer);
   RegisterFileType("#data", NULL,      NULL, "",        "",      "",                    binary_pre_viewer);

   MkDialogue(FILLSCREEN, "File-registration test", 0);
   AddTextBox(TOPLEFT, "This example shows how various properties can be "
   "hooked to file types_"
   "- cfg files will get a tool-icon and an `action' call-back when you click "
   "on it (here is just the text contents displayed, but you may do wathever "
   "needed in such a call-back)._"
   "- ttf files will get an icon._"
   "- txt files will get an icon and will be previewed_"
   "- the DATA type of data file objects will be previewed", 400, 0, TB_FRAMESINK|TB_LINEFEED_);
   AddButton(DOWNLEFT, "\33E~xit", quit, NULL);
   AddButton(RIGHT, "~Browse disk", start_disk_browse, NULL);
   AddButton(RIGHT, "Show code", show_code, NULL);
   DisplayWin();
   ProcessEvents();
   return 0;
}
END_OF_MAIN()
