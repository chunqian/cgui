/* CGUI Example program showing how to use some optional features of edit-boxes */
#include <ctype.h>
#include <allegro.h>
#include "cgui.h"

typedef struct {
   char *s;
   int *i;
   int id;
} t_stdata;

static void show_code(void *data)
{
   int id;
   (void)data;

   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/06edit.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "Close", CloseWin, NULL);
   DisplayWin();
}

static void shut_down(void *data)
{
   (void)data;
   StopProcessEvents();
}

void create_status_string(void *data, char *s)
{
   t_stdata *st_data = data;

   sprintf(s, "Current state: string=\"%s\" intval=%d", st_data->s, *st_data->i);
}

void my_uppercase(void *data)
{
   int scan, ascii, offset;
   (void)data;

   GetEditData(&scan, &ascii, &offset);
   ascii = utoupper(ascii);
   SetEditData(scan, ascii, offset);
}

void refresh_later(void *data)
{
   Refresh(*(int*)data);
}

void catch_edit(void *data)
{
   /* We don't want to do `Refresh(st_data->id);' now because the edit-box
      has not yet finished inserting the character into the string. */
   GenEvent(refresh_later, data, 0, 0);
}

static void TimeUpdate(void *data)
{
   GenEvent(TimeUpdate, data, 1000000, 0);
}

void make_object_window(void)
{
   static char string[300] = "junk";
   static char string2[300] = "james jones";
   static char string3[300] = "";
   static int intval = 13;
   static t_stdata st_data;
   static time_t t;
   int id;

   MkDialogue(ADAPTIVE, "Edit boxes - advanced options", 0);
   AddTextBox(DOWNLEFT, "This example shows how to use some optional features "
   "of edit-boxes. You can see following 3 things:_"
   "- how to affect the key-presses to the edit-box. The first one uses the "
   "built-in \"FNAME\" flag. The next one uses a call-back that hooks the "
   "typing and simply changes to uppercase_"
   "- how to make continous updating (after each key-press) of some other "
   "object. The implementation may seem a bit confusing. The reason why it "
   "must be done like this: To make it possible for the call-back to change "
   "the key-press value (like making it to upper-case), the call-back from "
   "the edit-box is always performed before the edit-box itself take "
   "any action. As a result of this, a call to `Refresh(id);' directly in the "
   "call-back function would not have reflected the key that was pressed (the "
   "string is not yet updated). This is however quite easy to work around by "
   "use of the event queue, that forces the Refresh-call to be done after "
   "the key-event has been finished and the edit box has updated the string._"
   "- Telling the exit-boxes to automatically tab to next object after the "
   "`enter' key-press. This will be more user-friendly, specially if there are "
   "multiple boxes to enter data into.", 500, 0, TB_FRAMESINK|TB_LINEFEED_);
   id = AddEditBox(DOWNLEFT, 100, "Try to type in this with caps lock", FSTRING | FNAME, 300, string2);
   TabOnCR(id);
   id = AddEditBox(DOWNLEFT, 100, "Try this one with only lower-case", FSTRING, 300, string3);
   AddHandler(id, my_uppercase, NULL);
   TabOnCR(id);
   id = AddEditBox(DOWNLEFT, 40, "An integer valued edit b~ox", FINT, 0, &intval);
   AddHandler(id, catch_edit, &st_data.id);
   TabOnCR(id);
   id = AddEditBox(DOWNLEFT, 100, "Changes are reflected in the status field", FSTRING, 300, string);
   AddHandler(id, catch_edit, &st_data.id);
   TabOnCR(id);
   id = AddEditBox(DOWNLEFT, 100, "Time", ISO8601_DATE, 0, &time);
   TabOnCR(id);
   AddButton(DOWNLEFT, "\33E~xit", shut_down, NULL);
   AddButton(RIGHT, "Show c~ode", show_code, NULL);

   st_data.i = &intval;
   st_data.s = string;
   st_data.id = AddStatusField(DOWNLEFT, 300, create_status_string, &st_data);
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
