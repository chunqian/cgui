/*
 TextView - how to make a small text viewer

 by Rafal Szyja for Christer Sandberg
 rafalszyja@poczta.onet.pl
 http://rafalszyja.republika.pl */

#include <allegro.h>
#include "cgui.h"

static const char *fname; /* stores name of file to open */

void quit()
{
   exit(0);
}

/* makes text window */
void GUI_start(void)
{
   MkDialogue(ADAPTIVE,"Textview",0);
   AddButton(LEFT,"Quit",quit,NULL);
   AddTag(ALIGNRIGHT,0,"TextView - by Rafal Szyja for CGUI");
   AddTextBox(DOWNLEFT,fname,770,30,TB_FIXFONT|TB_TEXTFILE|TB_PREFORMAT|TB_FRAMESINK|TB_LINEFEED_);
   DisplayWin();
   ProcessEvents();
}


int main(int argc,char *argv[])
{
   InitCgui(800,600,15);
   /* Check command line */
   if (argc>2) {
      fname=argv[2];
      argc=1;
      GUI_start();
   } else { /* open file browser */
      fname = FileSelect("*.txt;*.doc;*.nfo;*.ion","",FS_BROWSE_DAT|FS_DISABLE_CREATE_DIR|FS_DISABLE_DELETING|FS_DISABLE_COPYING|FS_SHOW_MENU|FS_DISABLE_EDIT_DAT,"Open file","OK");
      GUI_start();
   }
   return 0;
}
END_OF_MAIN()
