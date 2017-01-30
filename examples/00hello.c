/* CGUI Example program showing how open a window with one close button.
   The program is 100% meaningless. */
#include <allegro.h>
#include "cgui.h"

void make_a_window(void);

static void new_window(void *data)
{
   (void)data;
   /* Here goes the code that is a respons to the button-click */
   /* This code just starts a new copy of the window. */
   make_a_window();
}

void stop_program(void *data)
{
   (void)data;
   exit(0);
}

void close_window(void *data)
{
   (void)data;
   /* This terminates the top level of event proscessing */
   StopProcessEvents();
   CloseWin(NULL);
}

void make_a_window(void)
{
   MkDialogue(ADAPTIVE, "Hello world", 0);
   AddButton(TOPLEFT, "A new_window", new_window, NULL);
   AddTextBox(RIGHT, "A simple window showing some buttons. Press the button to the left and you will get a new identical window.", 200, 0, 0);
   AddButton(DOWNLEFT, "\33E~xit", stop_program, NULL);
   AddButton(RIGHT, "Close", close_window, NULL);
   DisplayWin();

   /* Check for events - like mouse clicks and key-presses */
   ProcessEvents();
}

int main(void)
{
   InitCgui(1024, 768, 15);
   make_a_window();
   return 0;
}
END_OF_MAIN()
