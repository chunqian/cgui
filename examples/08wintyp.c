/* CGUI example - this one shows the effect of various mode settings of windows. */
#include <allegro.h>
#include "cgui.h"

int win_nr = 1;
void ButtonSet(void (*close_fun)(void*));

static void show_code(void *data)
{
   int id;
   (void)data;

   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/08wintyp.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "Close", CloseWin, NULL);
   DisplayWin();
}

void quit(void *data)
{
   (void)data;
   StopProcessEvents();
}

void close_win(void *data)
{
   (void)data;
   CloseWin(NULL);
}

void modal_win(void *data)
{
   char s[500];
   (void)data;

   sprintf(s, "0 (modal) number %d", win_nr++);
   MkDialogue(ADAPTIVE, s, 0);
   ButtonSet(close_win);
   DisplayWin();
}

void float_win(void *data)
{
   (void)data;
   MkDialogue(ADAPTIVE, "W_FLOATING", W_FLOATING);
   ButtonSet(close_win);
   DisplayWin();
}

void sibling_win(void *data)
{
   (void)data;
   MkDialogue(ADAPTIVE, "W_SIBLING (multiple modal children to parent)", W_SIBLING);
   ButtonSet(close_win);
   DisplayWin();
}

void ButtonSet(void (*close_fun)(void*))
{
   AddButton(TOPLEFT, "\33E~xit", quit, NULL);
   AddButton(RIGHT, "~Close", close_fun, NULL);
   AddButton(RIGHT, "Show code", show_code, NULL);
   AddButton(DOWNLEFT, "0 (modal)", modal_win, NULL);
   AddButton(RIGHT, "W__FLOATING", float_win, NULL);
   AddButton(RIGHT, "W__SIBLING", sibling_win, NULL);
   AddTextBox(DOWNLEFT, "You can reach a parent window as long as there "
                          "is no modal window \"hiding\", that is: as long "
                          "as it has no modal child. You are allowed to "
                          "make modal and floating siblings to the same "
                          "parent, but that makes no sense - the point with "
                          "\"sibling-windows\" is to use them when you want "
                          "multiple windows on top of one window, and be "
                          "able to switch between these but not back to the "
                          "parent", 200, 0, 0);
}

int main(void)
{
   InitCgui(1024, 768, 15);
   MkDialogue(FILLSCREEN, "0 (modal)", 0);
   ButtonSet(quit);
   DisplayWin();
   ProcessEvents();
   return 0;
}
END_OF_MAIN()
