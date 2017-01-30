/* CGUI Example program showing the usage of text-boxes. */
#include <allegro.h>
#include "cgui.h"

static void show_code(void *data)
{
   int id;
   (void)data;
   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/05txtbox.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "Close", CloseWin, NULL);
   DisplayWin();
}

static void shut_down(void *data)
{
   (void)data;
   StopProcessEvents();
}

void make_object_window(void)
{

   int id;
   MkDialogue(ADAPTIVE, "Text boxes", 0);
   AddTextBox(TOPLEFT, "This shows a text box with no outlining. The width "
      "\n\n is just 220 pixels,\nso not even this brief text will fit into a "
      "single row - however, the text box wraps it automatically! Extra    whitespace "
      "is ignored and linefeeds \n\n\n\n are treated as blanks.", 220, 0, 0);
   id = AddTextBox(RIGHT, "This is another text box. This one uses the TB_FRAMESINK "
      "option for outlining. Text boxes can be useful to give the user some "
      "hints about what to do in a dialogue, or to explain the meaning of a "
      "group of controls. This one is also set to be stretchable."
      "Just grip the right edge of the text-box and strech it to the desired "
      "size.",
      220, 0, TB_FRAMESINK);
   MakeStretchable(id, NULL, NULL, NO_VERTICAL);

   AddTextBox(DOWNLEFT, " This one uses the TB_FRAMERAISE\n"
      "option for outlining. In contrast to\n"
      "the other it is preformatted. Spaces \n"
      "as well as line-feeds controls the\n"
      "layout.\n\n"
      "If the \n"
      "text is badly formatted it will be clipped", 220, 0, TB_FRAMERAISE|TB_PREFORMAT);

   AddTextBox(DOWNLEFT, "Here is one where the TB__LINEFEED__ option is used._"
      "The __-character can be used to force new lines, in addition to the "
      "new lines autmatically inserted by the text box. To be able to really "
      "display a __ charater you need to type ____ (and to show that one typing "
      "________ is needed, and...;-).", 220, 0, TB_FRAMESINK|TB_LINEFEED_);

   id = AddTextBox(RIGHT, "Here is the\n"
      "\"number of rows\"\n"
      "parameter\n"
      "set to a\n"
      "certain value.\n"
      "This forces\n"
      "the textbox\n"
      "to also create\n"
      "a browsing\n"
      "unit. The browsing\n"
      "unit is enabled in\n"
      "this case since the\n"
      "text contains more\n"
      "rows than the specified\n"
      "number of rows (8). This\n"
      "one is also made stretchable in both directions.",
      120, 8, TB_FRAMESINK|TB_PREFORMAT);
   MakeStretchable(id, NULL, 0,0);
   AddTextBox(DOWNLEFT, "You will experiance yet another mode of the text-box "
      "if you click on the button \"Show code\"._"
      "That one uses the \"TB__TEXTFILE\" option, in which case the text-box "
      "will take the string for the name of a file, loads that file and displays "
      "the contents of it.", 220, 0, TB_LINEFEED_);
   AddButton(DOWNLEFT, "\33E~xit", shut_down, NULL);
   AddButton(RIGHT, "Show code", show_code, NULL);
   DisplayWin();
}

int main(void)
{
   InitCgui(1024, 768, 15);
   make_object_window();
   ProcessEvents();
   return 0;
}
END_OF_MAIN()
