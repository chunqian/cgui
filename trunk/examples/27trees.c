/* CGUI Example program. This example shows simple usage of tree mode list-boxes */
#include <string.h>
#include <allegro.h>
#include "cgui.h"

/* This is some primitve tree data doing nothing. This is the user data that
   has nothing to do with the use of tree view in CGUI - just that some
   tree structure is needed to make a tree view.  */
typedef struct NODE {
   const char *s;
   int n;
   struct NODE **kids;
} NODE;

static NODE *create_node(const char *s)
{
   NODE *nd;
   nd = calloc(sizeof(NODE), 1);
   nd->s = s;
   return nd;
}

static void insert_kid(NODE *nd, NODE *kid)
{
   nd->n++;
   nd->kids = realloc(nd->kids, sizeof(NODE*)*nd->n);
   nd->kids[nd->n-1] = kid;
}

static NODE *get_kid(NODE *nd, int i)
{
   if (i >= nd->n)
      return NULL;
   return nd->kids[i];
}

static int is_tree_leaf(NODE *nd)
{
   return nd->n == 0;
}

static void destroy_tree(NODE *nd)
{
   int i;
   for (i=0; i<nd->n; i++)
      destroy_tree(nd->kids[i]);
   free(nd->kids);
   free(nd);
}

static NODE *init_some_random_tree(void)
{
   NODE *root, *nd, *nd2;
   root = create_node("root"); /* This one will be hidden due to the option setting in call to `ListTreeView'*/
   nd = create_node("Leaf at top level");
   insert_kid(root, nd);
   nd = create_node("Internal node at top level");
   insert_kid(root, nd);
   insert_kid(nd, create_node("Leaf at second level (under node 1)"));
   insert_kid(nd, create_node("Leaf (2) at second level (under node 1)"));
   nd = create_node("Internal node (2) at top level");
   insert_kid(root, nd);
   nd2 = create_node("Internal node at second level (below node 2)");
   insert_kid(nd, nd2);
   insert_kid(nd2, create_node("Leaf at third level (under node 2.1)"));
   insert_kid(nd, create_node("Leaf at second level (under node 2)"));
   return root;
}

/* End tree fucntions -----------------------------------------------*/

static void show_info(void *data)
{
   (void)data;
   MkDialogue(ADAPTIVE, "Info window", W_FLOATING);
   AddTextBox(TOPLEFT, "This examples shows how to use a list-box for a tree view.",
      400, 0, TB_FRAMESINK|TB_LINEFEED_);
   AddButton(DOWNLEFT, "#27;~Close", CloseWin, NULL);
   DisplayWin();
}

static void show_code(void *data)
{
   int id;
   (void)data;

   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/29trees.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "#27;~Close", CloseWin, NULL);
   DisplayWin();
}

static void quit(void *data)
{
   (void)data;
   StopProcessEvents();
}

/* This function works similar as if it was a plain list and not a tree, but note that get_kid must be preparde that 'i' is not within range. */
static void *index_creater(void *listdata, int i)
{
   return get_kid(listdata, i);
}

/* This function works as for generic lists, but in a tree view we maybe want to indicate the internal nodes using some icon */
static int row_text_creater(void *rowdata, char *s)
{
   NODE *nd = rowdata;
   if (is_tree_leaf(rowdata))
      sprintf(s, "%s", nd->s);
   else
      sprintf(s, "#open; %s", nd->s);
   return 0;
}

/* This function works exactely as for generic lists. */
static void show_content(int id, void *rowdata)
{
   NODE *nd = rowdata;
   char s[1000];
   (void)id;
   sprintf(s, "Here we can do whatever needed. For now we just put a dummy requester here to have something. "
   "E.g. we can show the text of the object associated with the row you clicked on:\"%s\"|#27; Close ", nd->s);
   Req("", s);
}

/* This small function is needed for the tree view only. The list box needs the returned information
   to know how to draw the row and how to handle events. In our implementation of the tree
   we regard any node without kids as a leaf - of course it is up to your implementation
   to define how to distingwish between leafs and internal nodes. */
static int is_leaf(void *rowdata)
{
   return is_tree_leaf(rowdata);
}

int main(void)
{
   NODE *root;
   int list_id;

   root = init_some_random_tree();
   InitCgui(1024, 768, 15);
   CguiLoadImage("examples.dat#icons/open", "", 1, 0);
   MkDialogue(ADAPTIVE, "Tree view in a list-box", 0);
   AddButton(TOPLEFT, "#27;E~xit", quit, NULL);
   AddButton(RIGHT, "~Show code", show_code, NULL);
   AddButton(RIGHT, "~Example info", show_info, NULL);
   CGUI_list_vspace = 5;
   list_id = AddList(DOWNLEFT, root, NULL, 300, LEFT_MOUSE, row_text_creater, show_content, 4);
   MakeStretchable(list_id, NULL, NULL, 0);
   SetIndexedList(list_id, index_creater);
   ListTreeView(list_id, 20, is_leaf, TR_HIDE_ROOT);    /* <--- This will do it (i.e. the turning the list into a tree viewer) */
   DisplayWin();
   ProcessEvents();
   destroy_tree(root);
   return 0;
}
END_OF_MAIN()
