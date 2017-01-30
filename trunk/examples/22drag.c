/* CGUI Example program. This example shows how to drag-and-drop objects. */
#include <allegro.h>
#include <string.h>
#include "cgui.h"
#include "cgui/mem.h"

#define MAXLEN 3

typedef struct t_slist {
   char **strs;
   int n;
   int id;
} t_slist;

static void show_code(void *data nouse)
{
   int id;
   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/22drag.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "~Close", CloseWin, NULL);
   DisplayWin();
}

static void show_info(void *data nouse)
{
   MkDialogue(ADAPTIVE, "Info window", W_FLOATING);
   AddTextBox(TOPLEFT, "This examples show how to make objects draggable._"
   "Probably you will first think of drag-and-drop between lists. This is "
   "also fairly easy to do in CGUI. However you can use the drag-and-drop "
   "method between any type of objects. The most obvious one is maybe the edit-box_ _"
   "You can type things into the editbox and then drop it into some of the "
   "lists, as well as drop things in it._ _"
   "The `flags' can be used to restrict the dropping, i.e. you can use the "
   "bits in `flags' to make groups of objects that handles the same category "
   "of objects. This is not used here." , 400, 0, TB_FRAMESINK|TB_LINEFEED_);
   AddButton(DOWNLEFT, "Close", CloseWin, NULL);
   DisplayWin();
}

static void quit(void *data nouse)
{
   StopProcessEvents();
}

static int row_text_creater(void *rowdata, char *s)
{
   strcpy(s, rowdata);
   return 0;
}

static void *index_creater(void *listdata, int i)
{
   t_slist *sl = listdata;
   return sl->strs[i];
}

static int fix_grip_list(void *srcobj nouse, int reason, void *srclist, int i)
{
   t_slist *sl = srclist;
   int j;

   if (reason == DD_SUCCESS) {
      /* we are called before drop, wait until copy is ready */
      GenEvent(free, sl->strs[i], 0, 0);
      for (j=i+1; j<sl->n; j++) /* compact list */
         sl->strs[j-1] = sl->strs[j];
      sl->n--;
      Refresh(sl->id);
   }
   return i < sl->n;
}

static int fix_drop_list(void *destobj nouse, int reason, void *srcobj,
         void *destlist, int i nouse, int flags nouse)
{
   t_slist *sl = destlist;

   if (reason == DD_OVER_DROP)
      return sl->n < MAXLEN;  /* don't overfill */
   else if (reason == DD_SUCCESS) {
      if (flags==2)
         sl->strs[sl->n++] = MkString(*(char**)srcobj);
      else
         sl->strs[sl->n++] = MkString(srcobj);
      Refresh(sl->id);
   }
   return 1;
}

static void fix_list_window(t_slist *sl)
{
   int id;

   MkDialogue(ADAPTIVE, "Fix list", W_FLOATING);

   AddTextBox(TOPLEFT, "- Maximum 3 rows_"
   "- Things are moved from this one_"
   "- Things are dropped at the end" , 220, 0, TB_LINEFEED_);
   id = AddList(DOWNLEFT, sl, &sl->n, 100, 0, row_text_creater, NULL, 10);
   SetIndexedList(id, index_creater);
   SetListGrippable(id, fix_grip_list, 1, LEFT_MOUSE);
   SetListDroppable(id, fix_drop_list, 3);
   sl->id = id;

   SetWindowPosition(300, 200);
   DisplayWin();
}

static int dyn_grip_list(void *srcobj nouse, int reason nouse,
                              void *srclist nouse, int i nouse)
{
   return srcobj!=NULL;
}

static int dyn_drop_list(void *destobj nouse, int reason, void *srcobj,
         void *destlist, int i nouse, int flags)
{
   t_slist *sl = destlist;
   int j;

   if (reason == DD_SUCCESS) {
      sl->strs = ResizeMem(char*, sl->strs, ++sl->n);
      for (j=sl->n-1; j>i; j--)
         sl->strs[j] = sl->strs[j-1];
      if (flags==2)
         sl->strs[i] = MkString(*(char**)srcobj);
      else
         sl->strs[i] = MkString(srcobj);
      Refresh(sl->id);
   }
   return 1;
}

static void dyn_list_window(t_slist *sl)
{
   int id;

   MkDialogue(ADAPTIVE, "Dynamic list", W_FLOATING);

   AddTextBox(TOPLEFT, "- No limit of the size of this list_"
   "- Things are copied from this one_"
   "- Things are inserted where they are dropped" , 220, 0, TB_LINEFEED_);
   id = AddList(DOWNLEFT, sl, &sl->n, 100, LEFT_MOUSE, row_text_creater, NULL, 10);
   SetIndexedList(id, index_creater);
   SetListGrippable(id, dyn_grip_list, 1, LEFT_MOUSE);
   SetListDroppable(id, dyn_drop_list, 3);
   sl->id = id;

   SetWindowPosition(20, 100);
   DisplayWin();
}

static int Drop(void *dest, int id, void *src, int reason, int flags nouse)
{
   char **str = dest;

   if (reason == DD_SUCCESS) {
      Release(*str);
      *str = MkString(src);
      Refresh(id);
   }
   return 1;
}

static void *Grip(void *src, int id nouse, int reason nouse)
{
   char **pstr = src;
   if (reason == DD_GRIPPED)
      MkTextPointer(GetCguiFont(), *pstr);
   else if (reason == DD_UNGRIPPED || reason == DD_SUCCESS)
      RemoveOverlayPointer();
   return src;
}

static void main_window(char **str)
{
   int id;
   MkDialogue(FILLSCREEN, "\"Drag and drop\" example", 0);
   AddButton(TOPLEFT|EQUALHEIGHT, "~Quit", quit, NULL);
   AddButton(RIGHT|EQUALHEIGHT, "Example_info", show_info, NULL);
   AddButton(RIGHT, "Show_code", show_code, NULL);
   id = AddEditBox(DOWNLEFT, 100, "Use right mouse button to grip:", FPTRSTR, 0, str);
   SetObjectGrippable(id, Grip, 2, RIGHT_MOUSE, str);
   SetObjectDroppable(id, Drop, 1, str);
   DisplayWin();
}

static t_slist *create_string_list(int n)
{
   t_slist *sl;
   int i;
   char s[100];

   sl = GetMem0(t_slist, 1);
   sl->strs = GetMem0(char*, n);
   for (i=0; i<n; i++) {
      sprintf(s, "String number %d", i+1);
      sl->strs[i] = MkString(s);
   }
   sl->n = n;
   return sl;
}

static void destroy_string_list(t_slist *sl)
{
   int i;
   for (i=0; i<sl->n; i++)
      Release(sl->strs[i]);
   Release(sl->strs);
   Release(sl);
}

int main(void)
{
   char *strs[MAXLEN], *str = NULL;
   t_slist *sl, sl2={strs,1,0};
   int i;

   strs[0] = MkString("Almost empty");
   sl = create_string_list(20);

   InitCgui(1024, 768, 15);
   main_window(&str);
   dyn_list_window(sl);
   fix_list_window(&sl2);
   ProcessEvents();
   destroy_string_list(sl);
   for (i=0; i<sl2.n; i++)
      Release(sl2.strs[i]);
   Release(str);
   return 0;
}
END_OF_MAIN()
