#include <allegro.h>
#include <string.h>
#include <stdio.h>
#include "cgui.h"
#include "event.h"
#include "object.h"
#include "window.h"
#include "node.h"
#include "cgui/mem.h"

typedef struct t_treelist {
   char **ss;
   int n;
   int id;
} t_treelist;

static void MakeObjectStringList(t_object *b0, t_treelist *tl, int tabs)
{
   t_object *b;
   char tmp[100], s[1000]="                                                                                              ";

   s[tabs*3] = 0;
   if (IsWindow(b0)) {
      strcat(s, "(Window)");
      if (b0->node->win->header && b0->node->win->header->label)
         strcat(s, b0->node->win->header->label);
   } else if (b0->node)
      strcat(s, "(Node)");
   else if (b0->label)
      strcat(s, b0->label);
   sprintf(tmp, "%d", b0->id);
   strcat(s, tmp);
   if (b0->node) {
      tl->n++;
      tl->ss = ResizeMem(char*, tl->ss, tl->n);
      tl->ss[tl->n - 1] = MkString(s);
      for (b = b0->node->firstob; b; b = b->next)
         MakeObjectStringList(b, tl, tabs+1);
   }
}

static void *create_index(void *listdata, int i)
{
   t_treelist *tl = listdata;

   return tl->ss[i];
}

static int row_text_creater(void *rowdata, char *s)
{
   strcpy(s, rowdata);
   return 0;
}

static void destroy_tree_list(void *data)
{
   t_treelist *tl = data;
   int i;

   for (i = 0; i< tl->n; i++)
       Release(tl->ss[i]);
   tl->n = 0;
}

static void update_object_tree(void *data)
{
   t_treelist *tl = data;

   destroy_tree_list(tl);
   MakeObjectStringList(_win_root, tl, 0);
   Refresh(tl->id);
   _GenEventOfCgui(update_object_tree, tl, 1000, tl->id);
}

extern void CreateObjectTreeViewer(void)
{
   static t_treelist tl={0,0,0};
   MakeObjectStringList(_win_root, &tl, W_FLOATING|W_SIBLING);
   MkDialogue(ADAPTIVE, "Object-tree log", 0);
   tl.id = AddList(DOWNLEFT, &tl, &tl.n, 200, 0, row_text_creater, NULL, 50);
   SetIndexedList(tl.id, create_index);
   DisplayWin();
   _GenEventOfCgui(update_object_tree, &tl, 1000, tl.id);
   HookExit(0, destroy_tree_list, &tl);
}
