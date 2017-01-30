/* CGUI Example program. This example shows how to use the progress-bar
   and the progress-bar window. */
#include <allegro.h>
#include "cgui.h"

static void quit(void *data nouse)
{
   StopProcessEvents();
}

static void show_code(void *data nouse)
{
   int id;

   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/17progr.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "~Close", CloseWin, NULL);
   DisplayWin();
}

/* The busy wait procedure */
static void busy_wait(void *data nouse)
{
   int id, i;

   id = MkProgressWindow("Working...", "", 100);
   for (i = 1; i <= 100; i++) {
      /* Perform a chunk of a heavy load work here */
      rest(50);
      UpdateProgressValue(id, i);
   }
   CloseWin(NULL);
}

/* A simple progress window */
static int idwin;
static void update_win(void *data)
{
   int *xp = data;

   /* Perform a chunk of a heavy load work here. */
   if (*xp < 100) {
      UpdateProgressValue(idwin, ++(*xp));
      GenEvent(update_win, xp, 200, idwin);
   } else
      CloseWin(NULL);
}

static void progress_window(void *data nouse)
{
   static int x;

   x = 0;
   idwin = MkProgressWindow("Please wait...", "Stop", 100);
   GenEvent(update_win, &x, 200, idwin);
}

/* A simple progress bar */
static void update(void *idp)
{
   static int x = 0;
   int id = *(int*)idp;

   /* Perform a chunk of a heavy load work here. */
   if (x < 100) {
      UpdateProgressValue(id, ++x);
      GenEvent(update, idp, 50, id);
   } else {
      CloseWin(NULL);
      x = 0;
   }
}

static void progress_bar(void *data nouse)
{
   static int id;

   MkDialogue(ADAPTIVE, "Loading ...", 0);
   id = AddProgressBar(DOWNLEFT, 200, 20);
   DisplayWin();
   GenEvent(update, &id, 20, id);
}

int main(void)
{
   InitCgui(1024, 768, 15);
   MkDialogue(FILLSCREEN, "Progress-bar test", 0);
   SetHotKey(0, quit, NULL, KEY_ESC, '\x1b');
   AddButton(TOPLEFT, "E~xit", quit, NULL);
   AddButton(RIGHT, "A bar", progress_bar, NULL);
   AddButton(RIGHT, "A window", progress_window, NULL);
   AddButton(RIGHT, "Busy-wait", busy_wait, NULL);
   AddButton(RIGHT, "Show code", show_code, NULL);
   AddTextBox(DOWNLEFT, "This example shows how to use the progress-bar "
   "and the progress-bar window._"
   "Here is also shown 3 different ways of doing the heavy load/update, the "
   "methods chosen is of course not related to the type of progress-object "
   "used together with it.", 400, 0, TB_FRAMESINK|TB_LINEFEED_);
   DisplayWin();
   ProcessEvents();
   return 0;
}
END_OF_MAIN()
