/* CGUI Example program showing the usage of a status-field object. It also
   shows how to use the optional call-back for edit-boxes. Normally we don't
   need to be involved when the user types things - we are only interested
   in the result (i.e. the final string) which can be inspected when closing
   the window.
   In this case we catches all key-presses an updates the status-field when
   the user is ready, indicated by a carrage return.
   In some cases you may indicate some modifiable information of impartance
   in the winodw header (i.e. uses as status-viewer). How this can be done
   in cgui is also demonstrated.
*/

#include <allegro.h>
#include <string.h>
#include "cgui.h"

typedef struct {
   char *s, *hdr;
   int *i;
   int id, did;
} t_stdata;

#define mk_head(tmp, s) strcat(strcpy(tmp, "Current document: "), s)

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

void catch_edit(void *data)
{
   t_stdata *st_data = data;
   int scan, ascii, offset;

   GetEditData(&scan, &ascii, &offset);
   if (ascii == '\r')
      Refresh(st_data->id);
}

void catch_edit_win_head(void *data)
{
   t_stdata *st_data = data;
   int scan, ascii, offset;
   char tmp[1000];

   GetEditData(&scan, &ascii, &offset);
   if (ascii == '\r') {
      ModifyHeader(st_data->did, mk_head(tmp, st_data->hdr));
      Refresh(st_data->did);
   }
}

void make_object_window(void)
{
   static char string[300 + 1];
   static char win_header[300 + 1] = "Scratch";
   static int intval = 13;
   static t_stdata st_data;
   int id;
   char tmp[1000];

   st_data.did = MkDialogue(ADAPTIVE, mk_head(tmp, win_header), 0);
   AddTag(DOWNLEFT, "Status-field example");

   id = AddEditBox(DOWNLEFT, 100, "Type things and press enter:", FSTRING, 300, string);
   AddHandler(id, catch_edit, &st_data);

   id = AddEditBox(DOWNLEFT, 40, ".. and a number:", FINT, 0, &intval);
   AddHandler(id, catch_edit, &st_data);

   id = AddEditBox(DOWNLEFT, 100, "Change the window header:", FSTRING, 300, win_header);
   AddHandler(id, catch_edit_win_head, &st_data);

   AddButton(DOWNLEFT, "\33E~xit", shut_down, NULL);
   st_data.id = AddStatusField(DOWNLEFT, 300, create_status_string, &st_data);

   st_data.i = &intval;
   st_data.s = string;
   st_data.hdr = win_header;
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
