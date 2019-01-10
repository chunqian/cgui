/* CGUI Example program showing how to use a combo box */
#include "cgui.h"
#include "cgui/mem.h"
#include <allegro.h>

#define NR_OF_ITEMS 4
#define STRING_LEN 20

static void show_code(void *data)
{
   int id;
   (void)data;
   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/30combo.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "Close", CloseWin, NULL);
   DisplayWin();
}

static void shut_down(void *data)
{
   (void)data;
   StopProcessEvents();
}

int main(void)
{
   int id;
   static const char *combo_strings[NR_OF_ITEMS] = {"123 Ham", "397 Cabbage", "228 Pork", "499 Coffe"};
   static char edit_string[STRING_LEN];
   static int sel;
   InitCgui(1024, 768, 15);
   MkDialogue(ADAPTIVE, "Combo boxes", 0);
   AddTextBox(DOWNLEFT, "This example shows how to use a combo box.", 500, 0, TB_FRAMESINK|TB_LINEFEED_);
   id = AddEditBox(DOWNLEFT, 80, "", FSTRING, STRING_LEN, edit_string);
   AttachComboProperty(id, &sel, combo_strings, NR_OF_ITEMS);
   AddButton(DOWNLEFT, "\33E~xit", shut_down, NULL);
   AddButton(RIGHT, "Show c~ode", show_code, NULL);
   DisplayWin();
   ProcessEvents();
   return 0;
}
END_OF_MAIN()
