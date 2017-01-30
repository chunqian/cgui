/* CGUI Example program. The windows are simlar to `0hello.c'. The purpose
   is to show another way to check for events. Since a gui-program need to
   check the event queue all the time it is enough to call ProcessEvents()
   once: No extra instance of ProcessEvents() is needed for checking the
   other window.
*/

#include <allegro.h>
#include "cgui.h"

void make_a_window(int);

static void new_window(void *data)
{
   (void)data;
   make_a_window(0);
}

void stop_program(void *data)
{
   (void)data;
   /* Of course exit(0); would do as well, but this gives the upportunity
      to do some clean-ups at end of main. */
   StopProcessEvents();
}

void make_a_window(int first)
{
   MkDialogue(ADAPTIVE, "Hello world - (2)", 0);
   AddButton(TOPLEFT, "A new_window", new_window, NULL);
   AddTextBox(RIGHT, "A simple window showing some buttons. Press the button to the left and you will get a new identical window.", 200, 0, 0);
   AddButton(DOWNLEFT, "\33E~xit", stop_program, NULL);
   if (!first)
   /* Closing the first is the same as exit, it is not needed (if we do, the
      desktop will be the window in focus, and there are no controls on it) */
      AddButton(RIGHT, "~Close", CloseWin, NULL);
   DisplayWin();
}

int main(void)
{
   InitCgui(1024, 768, 15);
   make_a_window(1);
   ProcessEvents();
   return 0;
}
END_OF_MAIN()
