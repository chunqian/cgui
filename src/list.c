/* Module LIST.C

   Contains functions for list-box handling

   List implementation: Muliple (chained) lists are possible to create.
   Multiple lists are lists created in a sequence preferably horizontally and
   The idea is to share the browsing unit between multiple lists. This
   may be useful for the application if the data for some reason are
   connected index by index. E.g. two lists showing married people: males in
   the left and females in the right one, will make no sense if the lists are
   individually browsable, and putting the pairs into one common list will
   remove the possibility to click-select a single person individaully.

   Lists are handled in a general way, i.e. internally there are no special
   handling for single lists, so a list starts with a list box (which is a
   node) containing one or more lists (each of them a node), and a browsing
   unit (which is also a node).
   Within that one there is a row-arena, whith eaxactely the size sufficient
   to contain all the rows. The rows are specialized objects which existence
   is independent of the application data, i.e. they will present even when
   there is not data to display.
   The browsing works like this: The start index in the application data is
   stored in the listbox. E.g. if the first two data objects are scrolled
   out, the start index is set to 2, and the first list row will refere to
   the application data with index 2. No special action is needed by the
   application programmer to achieve browsing. The end-user may browse either
   by use of the page up/down keys or by use of a browsing unit which will
   automatically be de-activated when not needed. The browsing unit is
   external (see browser.c) and is not designed specially for the list-box.
   The list may be scrolled by the end-user. The arrow up/down keys and the
   scroll-buttons on the browsing unit is the tool for this. The page up/down
   and arrow up/down keys have special handlers designed by the list-box.
   Multiple row selections are possible. When multiple rows are selected,
   this will affect the following commands: delete, grip(for dragging),
   enter-key and ctrl-enter keys (for emulating left-click and right-click
   respectively).
   The application is responsible for updating the list when the data to
   display has changed in any way (eg. contents of one or more data items
   has changed, number of item has increased or decreased). */

#include <string.h>

#include <ctype.h>
#include <allegro.h>
#include <allegro/internal/aintern.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "list.h"
#include "node.h"
#include "mstates.h"
#include "window.h"
#include "object.h"
#include "tabchain.h"
#include "obsizer.h"
#include "obcheck.h"
#include "obbutton.h"
#include "font.h"
#include "memint.h"
#include "ndresize.h"
#include "browser.h"
#include "id.h"
#include "event.h"
#include "mouse.h"

#define ITERATE_INDEXED 0
#define ITERATE_LINKED  1
#define CHECKSPACE   12

/* Rows are numbered from top to bottom, staring with 0. Index is calculated
   as */
/* current start index of the list + the row number. */
#define SetRowNumber(b, i) ((t_listrow*)b->appdata)->rn = i
#define GetRowNumber(b)  ((t_listrow*)b->appdata)->rn
#define SetEventId(b, id) ((t_listrow*)b->appdata)->eventid = id
#define GetEventId(b)  ((t_listrow*)b->appdata)->eventid
#define SetZ(b, z) ((t_listrow*)b->appdata)->prevz = z
#define GetZ(b) ((t_listrow*)b->appdata)->prevz
#define SetListRef(b, l) ((t_listrow*)b->appdata)->l = l
#define GetListRef(b) ((t_listrow*)b->appdata)->l
#define SetClickedColumn(b, x) ((t_listrow*)b->appdata)->clicked_column = (x)
#define GetClickedColumn(b) ((t_listrow*)b->appdata)->clicked_column
#define GetRowIndex(b) (((t_listrow*)b->appdata)->l->lc->sti+((t_listrow*)b->appdata)->rn)
#define IsExpanded(b) (((t_listrow*)b->appdata)->tn->expanded)
#define IsLast(b) (((t_listrow*)b->appdata)->tn->last)
#define GetAppDataOfTreeNode(b) (((t_listrow*)b->appdata)->tn->data)
#define GetAppDataOfLinkListRow(b) (void*)((t_listrow*)b->appdata)->tn
#define GetTreeNode(b) (((t_listrow*)b->appdata)->tn)
#define IsInternal(b) (((t_listrow*)b->appdata)->tn->internal)
#define UserStartX(b) (((t_listrow*)b->appdata)->tn->startx)
#define GetLevel(b) (((t_listrow*)b->appdata)->tn->level)
/* Row select set the select flag in a row-object (meaning selected for input */
/* this is specially important for multi-line selection */
#define SetMetaClick(b, x) ((t_listrow*)(b)->appdata)->metaclick = x
#define IsMetaClick(b) (((t_listrow*)(b)->appdata)->metaclick)
#define TellSubFocusOfRow(b) SetSubFocusOfLink(b->tablink, b)
#define SetSubFocusOfRow(b) TellSubFocusOfRow(b),(b)->tcfun->SetFocus(b)
#define ToggleExpanded(b) ((t_listrow*)b->appdata)->tn->expanded ^= 1
#define SetExpanded(b, x) ((t_listrow*)b->appdata)->tn->expanded = x
#define SetLast(b) ((t_listrow*)b->appdata)->tn->last = 1
#define SetAppDataOfTreeNode(b, d) ((t_listrow*)b->appdata)->tn->data = d
#define SetAppDataOfLinkListRow(b, d) ((t_listrow*)b->appdata)->tn = d
#define SetTreeNode(b, x) ((t_listrow*)b->appdata)->tn = x
#define SetInternal(b) ((t_listrow*)b->appdata)->tn->internal = 1

#define HAS_IMAGE(str) ((str)[0] == '#' && (str)[1] != '#')

#if ALLEGRO_SUB_VERSION == 0
#define textout_ex(bmp, f, s, x, y, col, bgcol) textout(bmp, f, s, x, y, col)
#define set_clip_rect set_clip
#endif

/* To hold values for a bar diagram bar of a specific row (to be set by the application). */
static int CGUI_list_row_bar_diagram_color;
static double CGUI_list_row_bar_diagram_percentage;
static int CGUI_list_row_bar_diagram_used;

typedef struct t_listchain t_listchain;
typedef struct t_tree_node t_tree_node;
typedef struct t_listrow t_listrow;
typedef struct t_list t_list;

struct t_list {
   /* Pointer to the node where the row objects lives. */
   t_node *rowsnode;

   /* Pointer to the frame node (enclosing the rows node and an optionally header object). */
   t_node *framenode;

   /* A handle to a dummy object to make it possible to later insert a list header object (i.e. column labels header object). */
   int handle_id;

   /* Tells which of left and/or right mouse buttons to be recognised. */
   int events;

   /* Application call-back in case of a click on a row in the list. */
   void (*Action) (int, void *);

   /* Application call-back in case of a click below the list items. */
   void (*BelowListEndCallBack) (int id, void *data);

   /* Data associated with the above call back. */
   void *below_list_end_data;

   /* The default pack function for the rows node (is is overrided for the rows node).  */
   void (*Pack) (struct t_object *);

   /* Pointer to the list-data of the application. */
   void *applist;

   /* Pointer to a location (of the application) where  number of list elements are stored. */
   int *n;

   /* Pointer to font used in this list box */
   FONT *font;

   /* Flag indicating that the type of the users list data (indexed or linked list). */
   int user_data_type;

   /* Flag that tells if the row that is in focus shall be indicated (e.g. by a different background color) and shall respond to user commands. */
   int showfocus;

   /* y-offset from start of row to top of text */
   int offs;

   /* Tells the color of the delimiter between two rows. */
   int row_delimiter_color;

   /* Tells the color of the delimiter between two columns. */
   int column_delimiter_color;

   /* Pointer to application array that specifies the widths of columns (in case of columns, else unused, i.e. NULL). */
   int *column_widths;

   /* Number of columns (initially set to 1) */
   int nrcols;

   /* Indicates that the user may adjust the width of columns. Only valid if column drawing has been specified by the app. */
   int resize_columns;

   /* Indicates that the when the column widths are stretched the size of the listbox must not extend. Valid if `resize_columns' is set. */
   int resize_columns_in_fix_width_listbox;

   /* Indicates that a delimiter shall be drawn between columns (or rather at the end of one). Only valid if column drawing has been specified by
      the app. */
   int col_delimiter;

   /* If true the listbox will show tool tips for the rows */
   int show_tool_tip;

   /* Call-back to application Grip function. */
   int (*Grip) (void *obj, int reason, void *applist, int i);

   /* Call-back to application Drop function. */
   int (*Drop) (void *destobj, int reason, void *srcobj, void *destlistdata, int i, int flags);

   /* Call-back to application that creates the text string that will be drawn as a row. */
   int (*CreateRowText) (void *data, char *s);

   /* Alternative call-back to application that creates the text of one column in one row in the list. The column numbers starts with 0. */
   int (*CreateColumnTextOfRow) (void *data, char *s, int colnr);


   /* If not NULL, NotifyMove will be called each time a new row gets focus. */
   void (*NotifyMove) (int id, void *data);

   /* Application call-back function in case double click is specified (is NULL else). */
   void (*Double) (int, void *data, int);

   /* The grip flags passed to the default set grip. */
   int gripflags;

   /* The grip buttons to the default set grip. */
   int gripbuttons;

   /* The drop flags passed to the default set drop. */
   int dropflags;

   /* The double-click button passed to the default set double. */
   int doublebuttons;

   /* Flag that prevent infinite recursion when calling NotfyMove. */
   int notifying_in_progress;

   /* If set only allows for single rows to be selected */
   int single_row_sel;

   /* If set a certain column will be selected by the mouse. The call back will
      not be differently handled, only the view is affected by this option. The
      user will need to check explicitly for the column. If the arrow keys are used
      the entire row is marked.*/
   int column_selection;

   /* Iterator functions for indexed and linked list respectively. */
   union {
      void *(*GetUserDataAtIndex) (void *, int);
      void *(*IterateUsersLinkedList) (void *, void *);
   } item;

   /* A call-back the application in case the user pressed the delete key when one or more list rows are in focus. NULL when not used. */
   void (*DelHandler) (int id, void *object);

   /* A call-back the application in case the user pressed the insert key when one or more list rows are in focus. NULL when not used. */
   void (*InsHandler) (void *, int);

   /* Pointer to the next list in the list chain. */
   t_list *next;

   /* Pointer to the previous list in the list chain. */
   t_list *prev;

   /* Pointer to a the list chain which the list is a part of. */
   t_listchain *lc;

   /* The width of the list in pixels */
   int width;

   /* The original width of the list in pixels */
   int orgwidth;

   /* The height of the list in pixels */
   int height;

   /* The original height of the list in pixels */
   int orgheight;

   /* Flag to indicates if list is during "build" or is completed */
   int done;

   /* Conatins the index of a pending browse-command */
   int browsepending;

   /* A pointer to the data to be launched to the 'Action' call-back */
   struct t_action_wrapper *action_wrapper;

   /* Pointer to the node that contains the labels. */
   struct t_node *header;

   /* Tree view stuff: */
   int memofn;

   /* Flag that tells that the list should be handled as a tree rather than as a list. */
   int treeview;

   /* The options in case of tree view. */
   int tree_view_options;

   /* Tells the total width of the tree-part of the list. */
   int level_width;

   /* State variable used in some recursions to avoid infinite recursion. */
   int tree_rebuild_disabled;

   /* Pointer the the tree node. */
   struct t_tree_node *root;

   /* Application call-back. Will be called when we need to know if a node is a leaf or an internal node. */
   int (*IsLeaf)(void*);

   /* Application call-back, optional, else NULL. Will be called when we need to know if a node is expanded or not. */
   int (*IsExpandedCallBack)(void *data);
};

/* The data needed for a node in a tree used in tree view mode of a list */
struct t_tree_node {
   /* A flag indicating if this node is expanded or collapsed (non-zero if expanded). */
   int expanded;

   int last;

   /* A flag telling if this node is an internal node or a leaf (non-zero if internal). */
   int internal;

   /* Tells where the application data starts on the row, relative coordinite within the row (to the left of this point there are
      only tree-controlling stuff) */
   int startx;

   /* Tells which level this node is on (root is level 0). */
   int level;

   /* Number of kids to this node. */
   int nkids;

   /* Pointer to the parent object (NULL if this is the root). */
   t_tree_node *parent;

   /* A list of kids to this node. */
   t_tree_node **kids;

   /* The application data for this node. */
   void *data;
};

/* A row object */
struct t_listrow {
   /* The row-number (first row is 0). */
   int rn;

   /* The id to the event that iterates the mouse wheel handler. */
   int eventid;

   /* The z from the check of the mouse wheel. */
   int prevz;

   /* Reference to the list object where this row is a part. */
   t_list *l;

   /* Flag indicating that the row was clicked with either the ctrl or shift keys down, and used to distinguish this event from that of
   ctrl-enter key command. */
   int metaclick;

   /* The last clicked column, if known, else -1. */
   int clicked_column;

   /* Pointer to the visible row object that this is a part of. */
   t_object *b;

   /* A reference to a tree node in case of tree view mode, or NULL if there are fewer tree nodes than rows */
   t_tree_node *tn;
};

struct t_listchain {
   /* Logical start of list chain (the rightmost, last issued). */
   t_list *l;

   /* Logical end of list chain (the leftmost, first issued). */
   t_list *last;

   /* Start index (always common for all the lists. */
   int sti;

   /* The start position in the list expressed in pixels. */
   int pos;

   /* The maximum of elements in the longest list (lists in a list chain are allowed to have different max no of elements). */
   int n;

   /* Number of rows to view in a list. */
   int rows;

   /* Height of each row in a list. */
   int rh;

   /* Application coordinate specification. */
   int x, y;

   /* Flags indicating if FILLSPACE was specified for the list-box. */
   int fillw, fillh;

   /* A pointer to the browsed object of the list chain. */
   struct t_object *brob;

   /* A reference to the browser object of the list chain. */
   int br;

   /* A pointer tho the outermost node of the list box (optionally there are more than on list-boxe in a list chain). */
   t_node *outer_node;
};

int cgui_list_no_multiple_row_selection;
int cgui_list_show_focused_row = 1;
FONT *CGUI_list_font;
int CGUI_list_vspace;
FONT *CGUI_list_row_font;
int CGUI_list_fixfont;
int cgui_list_fix_digits;
int CGUI_list_row_f_color;
int CGUI_list_row_b_color;
static int color_conversion_prepared, rfr, rfg, rfb, rbr, rbg, rbb;

static void ListPrepareNewScreenMode(t_object *b nouse)
{
   if (!color_conversion_prepared) {
      color_conversion_prepared = 1;
      rfr  = getr(CGUI_list_row_f_color);
      rfg  = getg(CGUI_list_row_f_color);
      rfb  = getb(CGUI_list_row_f_color);
      rbr  = getr(CGUI_list_row_b_color);
      rbg  = getg(CGUI_list_row_b_color);
      rbb  = getb(CGUI_list_row_b_color);
   }
}

static void ListNewScreenMode(t_object *b)
{
   t_list *l;
   if (color_conversion_prepared) {
      CGUI_list_row_f_color = makecol(rfr, rfg, rfb);
      CGUI_list_row_b_color = makecol(rbr, rbg, rbb);
      color_conversion_prepared = 0;
   }
   l = b->node->data;
}

static void CalcMaxN(t_listchain *lc)
{
   t_list *l;
   int n;

   for (l = lc->l, n = 0; l; l = l->next)
      n = MAX(*l->n, n);
   lc->n = n;
}

/* This function will build the entire tree (which entirely maps to the tree structure of the user.
   Each node will point to corresponding data object of the user's tree.
   Each new node will be set to collapsed state and each node that is the last kid will be marked as
   'last'.
   If the tree was re-built whenever the user refreshes the list, all expanded nodes would be
   automatically collapsed. To avoid this, the functions tries to re-use the nodes to keep the
   expanded state. */
static t_tree_node *GenerateTree2(t_list *l, int level, void *startdata, void *data, t_tree_node *old_node, int index)
{
   t_tree_node *k;
   t_tree_node *tree_node;
   void *nextdata, *appnode;
   int i;
   if (level >= 0) {
      if (l->user_data_type == ITERATE_INDEXED)
         appnode = l->item.GetUserDataAtIndex(startdata, index);
      else
         appnode = l->item.IterateUsersLinkedList(startdata, data);
      if (appnode == NULL) {
         return NULL;
      }
   } else {
      appnode = l->applist;
   }
   tree_node = GetMem0(t_tree_node, 1);
   if (old_node && old_node->data == appnode) {
      tree_node->expanded = old_node->expanded;
   }
   tree_node->data = appnode;
   tree_node->level = level+1;
   if (level < 0)
      tree_node->last = 1;
   if (!l->IsLeaf(tree_node->data)) {
      tree_node->internal = 1;
      nextdata = NULL;
      i = 0;
      do {
         if (old_node && i<old_node->nkids) {
            k = GenerateTree2(l, level+1, tree_node->data, nextdata, old_node->kids[i], i);
         } else {
            k = GenerateTree2(l, level+1, tree_node->data, nextdata, NULL, i);
         }
         if (k) {
            k->parent = tree_node;
            tree_node->nkids++;
            tree_node->kids = ResizeMem(t_tree_node *, tree_node->kids, tree_node->nkids);
            tree_node->kids[i] = k;
            nextdata = k->data;
            i++;
         }
      } while (k);
      if (tree_node->nkids > 0)
         tree_node->kids[tree_node->nkids-1]->last = 1;
   }
   return tree_node;
}

/* Will perform a depth first search over tree `tree_node', bypassing *i nodes and start
   to assign nodes to object from there */
static t_object *AssignTreeNodesToRows(t_object *b, t_tree_node *tree_node, t_list *l)
{
   int i;

   if (tree_node==NULL)
      return b;
   if (b==NULL)
      return NULL;
   if (tree_node != l->root || !(l->tree_view_options&TR_HIDE_ROOT)) {
      SetTreeNode(b, tree_node);
      b = b->next;
   }
   if (tree_node->expanded) {
      for (i=0; i<tree_node->nkids && b; i++) {
         b = AssignTreeNodesToRows(b, tree_node->kids[i], l);
      }
   }
   return b;
}

/* Will bypass the n first tree nodes of a view, then assigning the following ones to list rows.
   The return value is 0 if n nodes were bypassed, else non-0.
   `tree_node' is a pointer the current tree node in the recursion, `b' is a pointer to the current row object (the first row as long as
   bypassing is in progress, the location pointed to by `end_row' will be assigned a pointer to the first row object not assigned any tree
   node (or NULL if all of them where assigned tree nodes). */
static t_object *BypassTreeNodes(t_object *b, t_tree_node *tree_node, t_list *l, int *n)
{
   int i;
   if (tree_node==NULL)
      return b;
   if (b==NULL)
      return b;
   if (*n > 0) {
      if (tree_node != l->root || !(l->tree_view_options&TR_HIDE_ROOT)) {
         (*n)--;
      }
   }
   if (tree_node->expanded) {
      for (i=0; i<tree_node->nkids && b; i++) {
         if (*n > 0) {
            b = BypassTreeNodes(b, tree_node->kids[i], l, n);
         } else {
            b = AssignTreeNodesToRows(b, tree_node->kids[i], l);
         }
      }
   }
   return b;
}

/* Simply returns memory of tree 'tree_node' */
static void DestroyTree(t_tree_node *tree_node)
{
   int i;

   for (i=0; i<tree_node->nkids; i++)
      DestroyTree(tree_node->kids[i]);
   Release(tree_node->kids);
   Release(tree_node);
}

/* Simply counts the nodes to view in tree 'tree_node' */
static int CountItems(t_tree_node *tree_node)
{
   int i, n=1;

   if (tree_node->internal && tree_node->expanded) {
      for (i=0; i<tree_node->nkids; i++)
         n += CountItems(tree_node->kids[i]);
   }
   return n;
}

static void MoveFocusToLastNonEmptyRow(t_list *l)
{
   int i, newrow, prev_tree_rebuild_disabled;
   t_object *b;

   newrow = *l->n - l->lc->sti - 1;
   for (i = 0, b = l->rowsnode->firstob; i < newrow && b; i++, b = b->next) {
      ;
   }
   if (b) {
      prev_tree_rebuild_disabled = l->tree_rebuild_disabled; /* This is terrible... */
      l->tree_rebuild_disabled = 1;
      b->tcfun->MoveFocusTo(b);
      l->tree_rebuild_disabled = prev_tree_rebuild_disabled;
   }
}

static void GenerateTree(t_list *l)
{
   t_object *end_row = NULL;
   t_tree_node *old_root;
   int nr_of_rows_to_bypass;

   if (!l->tree_rebuild_disabled) {
      old_root = l->root;
      l->root = GenerateTree2(l, -1, l->applist, NULL, old_root, 0);
      if (old_root) {
         DestroyTree(old_root);
      }
   }
   if (l->tree_view_options&TR_HIDE_ROOT)
      l->root->expanded = 1;
   *l->n = CountItems(l->root);
   if (l->tree_view_options&TR_HIDE_ROOT) {
      (*l->n)--;
   }
   NotifyBrowser(l->lc->br, l->lc->rh, *l->n*l->lc->rh);
   nr_of_rows_to_bypass = l->lc->sti;
   end_row = BypassTreeNodes(l->rowsnode->firstob, l->root, l, &nr_of_rows_to_bypass);
   /* If the tree don't need all the rows: */
   while (end_row) {
      SetTreeNode(end_row, NULL);
      end_row = end_row->next;
   }
}

static void GenerateTreeAndSetFocus(t_list *l)
{
   t_object *focused_row;
   GenerateTree(l);
   focused_row = GetSubFocus(l->rowsnode);
   if (focused_row) {
      if (GetRowIndex(focused_row) + l->lc->sti >= *l->n) {
         MoveFocusToLastNonEmptyRow(l);
      }
   }
}

static void ListTreeSetNodesExpandedState2(t_tree_node *tree_node, t_list *l)
{
   int i;
   if (tree_node) {
      tree_node->expanded = l->IsExpandedCallBack(tree_node->data);
      for (i=0; i<tree_node->nkids; i++) {
         ListTreeSetNodesExpandedState2(tree_node->kids[i], l);
      }
   }
}

/*static void PrintTree(int level, t_tree_node *tree_node, t_list *l)
{
   int i;
   char s[100], s2[100];

   if (tree_node==NULL)
      return;
   for (i=0; i<level; i++)
      printf("   ");
   l->CreateRowText(tree_node->data, s);
   if (tree_node->parent) {
      l->CreateRowText(tree_node->parent->data, s2);
      printf("%s (parent = %s)\n", s, s2);
   } else {
      printf("%s root \n", s);
   }
   for (i=0; i<tree_node->nkids; i++)
      PrintTree(level+1, tree_node->kids[i], l);
}
*/
/* This function will ask the application for fresh data and assign those to the list rows, and also update the number of data elements.
   It assumes the list is a linked list, and that it is not a tree. */
static void RefreshLinkedUserData(t_list *l)
{
   void *data;
   t_object *b;
   int j;
   for (j=0, data=l->item.IterateUsersLinkedList(l->applist, NULL); data && j < l->lc->sti; data=l->item.IterateUsersLinkedList(l->applist, data), j++) {
      ;
   }
   /* App data scrolled out above list has now been winded. In case 'data' breaks the above loop, the only effect of the
   below loop will be initialization of 'b' which will lead to initialization of proper NULL-values in all object in yet
   next loop (btw: this case should normally not occure, but we take care of it for safety). In case 'j' breaks the above
   loop, then 'data' referes to the first visible row of the list-box, and the below loop will assign it (and all existing
   subsequent data items properly to the corresponding object. */
   for (b=l->rowsnode->firstob; b && data; b=b->next, data=l->item.IterateUsersLinkedList(l->applist, data), j++) {
      SetAppDataOfLinkListRow(b, data);
   }
   /* The below loop is just to ensure there are no old values left below the end of the last item */
   while (b) {
      SetAppDataOfLinkListRow(b, NULL);
      b = b->next;
   }
   /* We need to count the rest of the items */
   while (data) {
      data = l->item.IterateUsersLinkedList(l->applist, data);
      j++;
   }
   *l->n = j;
   CalcMaxN(l->lc);
}

static void *GetApplicationData(int i, t_list *l, t_object *b)
{
   if (l->item.GetUserDataAtIndex == NULL) {
      return NULL;
   }
   if (l->treeview) {
      /* When called for first row we generate the entire tree, and maps
      all rows to the right tree-node. In subsequent calls we just return
      the data calcualted in the first call. */
      if (b == l->rowsnode->firstob) {
         GenerateTree(l);
      }
      if (GetTreeNode(b)) {
         return GetAppDataOfTreeNode(b);
      }
      return NULL;
   }
   if (l->user_data_type == ITERATE_INDEXED) {
      /* Array based list, no tree view */
      if (i >= *l->n) {
         return NULL;
      } else {
         return l->item.GetUserDataAtIndex(l->applist, i);
      }
   } else if (!l->treeview) {
      /* A linked list, no tree view */
      if (i == l->lc->sti) {
         if (b==NULL)
            return NULL;
         RefreshLinkedUserData(l);
      }
      return GetAppDataOfLinkListRow(b);
   }
   return NULL;
}

static t_list *GetList(int id)
{
   t_node *nd;

   nd = GetNode(id);
   if (nd) {
      if (nd->type != NODETYPE_LIST)
         if (nd->ob->parent && nd->ob->parent->ob->parent)
            nd = nd->ob->parent->ob->parent;
      if (nd->type == NODETYPE_LIST) {
         if (nd->firstob && nd->firstob->node)
            return nd->firstob->node->data;
      }
   }
   return NULL;
}

/* Returns the window-object of the specified ROW-number */
static t_object *GetRowObject(t_list *l, int rn)
{
   int i;
   t_object *b;

   if (rn >= l->lc->rows)
      return NULL;
   for (b = l->rowsnode->firstob, i = 0; i < rn && b; i++, b = b->next);
   return b;
}

static void DrawListFrame(t_object *b)
{
   BITMAP *bmp;
   t_list *l;
   t_listchain *lc;

   l = b->node->data;
   lc = l->lc;
   if (l->treeview) {
      GenerateTreeAndSetFocus(l);
   } else if (l->user_data_type == ITERATE_LINKED) {
      RefreshLinkedUserData(l);
   } else {
      CalcMaxN(lc);
   }
   NotifyBrowser(lc->br, lc->rh, lc->n*lc->rh);
   if (!IsBrowserActive(lc->br)) {
      lc->sti = 0;
   }
   bmp = b->parent->bmp;
   if (bmp == NULL)
      return;
   hline(bmp, b->x1, b->y1, b->x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   vline(bmp, b->x1, b->y1 + 1, b->y2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   hline(bmp, b->x1 + 1, b->y2, b->x2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   vline(bmp, b->x2, b->y1 + 1, b->y2 - 1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
}

extern void FixTextOut(BITMAP *bmp, FONT *font, const char *s, int x, int y, int color, int bcolor, int fixwidth)
{
   for (; *s; s++, x += fixwidth) {
      font->vtable->render_char(font, *s, color, bcolor, bmp, x, y);
   }
}

static int FixDigitTextOut(BITMAP *bmp, FONT *font, char *s, int right_align, int x, int x2, int y, int color, int bcolor)
{
   char buf[] = " ", *s0;
   static char *str = "023456789";
   static FONT *primfont, *secfont;
   static int prim_digit_width, sec_digit_width;
   int w, i, x0;

   /* The maximum width of the selected font's digits shall be used. First
      see if this is already calculated. We remember up to 2 fonts. */
   if (font == primfont)
      w = prim_digit_width;
   else if (font == secfont)
      w = sec_digit_width;
   else {
      *buf = str[0];
      w = text_length(font, buf);
      for (i = 1; i < 9; i++) {
         *buf = str[i];
         w = MAX(w, text_length(font, buf));
      }
      if (primfont == NULL) {
         primfont = font;
         prim_digit_width = w;
      } else {
         if (secfont) {
            primfont = secfont;
            prim_digit_width = sec_digit_width;
         }
         secfont = font;
         sec_digit_width = w;
      }
   }
   if (right_align) {
      if (*s) {
         x0 = x;
         s0 = s;
         s += strlen(s) - 1;
         if (isdigit(*s))
            x = x2 - w - 1;
         else
            x = x2 - 1 - text_length(font, s);
         do {
            font->vtable->render_char(font, *s, color, bcolor, bmp, x, y);
            s--;
            if (s < s0)
               break;
            if (isdigit(*s))
               x -= w;
            else {
               *buf = *s;
               x -= text_length(font, buf);
            }
         } while (1);
      }
   } else {
      for (; *s; s++) {
         font->vtable->render_char(font, *s, color, bcolor, bmp, x, y);
         if (isdigit(*s))
            x += w;
         else {
            *buf = *s;
            x += text_length(font, buf);
         }
      }
   }
   return w;
}

static int DrawNodeBranches(BITMAP *bmp, t_tree_node *tree_node, int x1, int y1, int y2, int w)
{
   if (tree_node==NULL)
      return x1;
   x1 = DrawNodeBranches(bmp, tree_node->parent, x1, y1, y2, w);
   if (!tree_node->last)
      vline(bmp, x1 + w/2, y1, y2, cgui_colors[CGUI_COLOR_TREE_CONTROL_OUTLINE]);
   return x1 + w;
}

static int DrawNode(BITMAP *bmp, t_tree_node *tree_node, int x1, int y1, int y2, int w, t_list *l)
{
   int x, y, x0;

   if (tree_node==NULL)
      return x1;
   y = y1 + l->lc->rh / 2;
   if (l->tree_view_options&TR_HIDE_ROOT) {
      x0 = x1;
      x1 = DrawNodeBranches(bmp, tree_node->parent, x1 - w, y1, y2, w);
      if (x1 < x0)
         x1 = x0;
   } else {
      x1 = DrawNodeBranches(bmp, tree_node->parent, x1, y1, y2, w);
   }
   x = x1 + w/2;
   x1 += w;
   if (tree_node->last) {
      vline(bmp, x, y1, y, cgui_colors[CGUI_COLOR_TREE_CONTROL_OUTLINE]);
   } else {
      vline(bmp, x, y1, y2, cgui_colors[CGUI_COLOR_TREE_CONTROL_OUTLINE]);
   }
   hline(bmp, x, y, x1-1, cgui_colors[CGUI_COLOR_TREE_CONTROL_OUTLINE]);
   if (tree_node->internal) {
      rectfill(bmp, x-4, y-4, x+4, y+4, cgui_colors[CGUI_COLOR_TREE_CONTROL_BACKGROUND]);
      rect(bmp, x-5, y-5, x+5, y+5, cgui_colors[CGUI_COLOR_TREE_CONTROL_OUTLINE]);
      hline(bmp, x-3, y, x+3, cgui_colors[CGUI_COLOR_TREE_CONTROL_OUTLINE]);
      if (!tree_node->expanded) {
         vline(bmp, x, y-3, y+3, cgui_colors[CGUI_COLOR_TREE_CONTROL_OUTLINE]);
      }
   }
   tree_node->startx = x1;
   return x1;
}

static t_image *ExtractImage(t_object *b, char **str)
{
   t_image *im;
   char *s = *str, *start, *p, c;

   for (start = s + 1; *start && *start == ' '; start++) {
      ;
   }
   for (p = start; *p && *p != ';'; p++) {
      ;
   }
   c = *p;
   *p = 0;
   im = GetImage(b->parent, start);
   if (c) {
      p++;
   }
   *str = p;
   return im;
}

static void ComputeClickedColumn(t_object *b)
{
   t_list *l;
   int column_number = -1;
   int current_column_end;
   int i;

   l = GetListRef(b);
   if (l->column_widths) {
      current_column_end = 0;
      for (i=0; i<l->nrcols; i++) {
         current_column_end += l->column_widths[i];
         if (b->x < current_column_end) {
            break;
         }
      }
      column_number = i;
   }
   SetClickedColumn(b, column_number);
}

static void Draw(t_object *b)
{
   t_list *l;
   int i;
   int colnr, x1, x2, imy, bgx=0, y1, y2, cx2, alx, strike = 0, uline = 0, ret = 0, strike_x, col_x, text_x, fcolor, bcolor, len, right_align;
   int is_clipped, tot_text_length, available_text_width, x_end, fixw, j, prev_parse_label_state, bar_end_x;
   int colum_color;
   int column_start;
   int column_end;
   int column_underline;
   char *s, *p, buf[50000], tttext[50000];
   BITMAP *bmp;
   t_image *im;
   void *appdata;
   int red, green, blue;

   bmp = b->parent->bmp;
   s = buf;
   if (bmp == NULL)
      return;
   i = GetRowIndex(b);
   l = GetListRef(b);
   x1 = b->x1;
   x2 = b->x2;
   y1 = b->y1;
   y2 = b->y2;
   column_start = x1;
   column_end = x2;
   strike_x = col_x = x1 + 2;
   set_clip_rect(bmp, x1, y1, x2, y2);
   hline(bmp, x1, y2, x2, l->row_delimiter_color);

   if (b->state == MS_FIRST_DOWN && GetRowIndex(b) < *l->n && (b->usedbutton == LEFT_MOUSE || b->usedbutton == RIGHT_MOUSE)) {
      b->hasfocus = 1;
   } else if (b->state == MS_FIRST_DOWN_OFF) {
      b->hasfocus = 0;
   }
   ComputeClickedColumn(b);
   if (b->hasfocus && cgui_list_show_focused_row) {
      if (b->hasfocus == 2) {
         bcolor = cgui_colors[CGUI_COLOR_LISTBOX_HIDDEN_FOCUS_BACKGROUND];
         fcolor = cgui_colors[CGUI_COLOR_LISTBOX_HIDDEN_FOCUS_TEXT];
      } else {
         bcolor = cgui_colors[CGUI_COLOR_LISTBOX_FOCUS_BACKGROUND];
         fcolor = cgui_colors[CGUI_COLOR_LISTBOX_FOCUS_TEXT];
      }
   } else {
      fcolor = cgui_colors[CGUI_COLOR_LISTBOX_TEXT];
      bcolor = cgui_colors[CGUI_COLOR_LISTBOX_BACKGROUND];
   }
   is_clipped = 0;
   CGUI_list_row_bar_diagram_used = 0;
   if (l->nrcols) {
      appdata = GetApplicationData(i, l, b);
      if (appdata) {
         for (colnr = 0; colnr < l->nrcols; colnr++) {
            /* Prepare globals before calling user function (these may be uppdated by the caller) */
            CGUI_list_row_font = l->font;
            CGUI_list_row_f_color = fcolor;
            CGUI_list_row_b_color = bcolor;
            s = buf;
            if (l->CreateColumnTextOfRow) {
               ret = l->CreateColumnTextOfRow(appdata, s, colnr);
            } else if (l->CreateRowText) {
               ret = l->CreateRowText(appdata, s);
            }
            right_align = (ret & COL_RIGHT_ALIGN) && l->column_widths;

            text_x = col_x;
            if (l->column_widths) {
               x_end = col_x + l->column_widths[colnr] - 1;
               if (colnr > 0) {
                  column_start = column_end + 1;
               }
               column_end = column_start + l->column_widths[colnr] - 1;
            } else {
               x_end = x2;
               column_end = x2;
            }
            /* Note that the colors may have been changed by the user here */
            if (colnr == 0) {
               bgx = x1;
               if (l->treeview) {
                  bgx += GetLevel(b) * l->level_width;
                  if ((l->tree_view_options&TR_HIDE_ROOT)==0)
                     bgx += l->level_width;
               }
               if (HAS_IMAGE(s)) {
                  bgx += 2;
               }
               if (bgx > x1) {
                  rectfill(bmp, x1, y1, bgx-1, y2 - 1, cgui_colors[CGUI_COLOR_TREE_VIEW_BACKGROUND]);
               }
               if (CGUI_list_row_bar_diagram_used) {
                  /* Application wants to use this row to display the bar of a bar diagram. */
                  bar_end_x = (x2 - bgx + 1) * CGUI_list_row_bar_diagram_percentage + 0.5;
                  if (b->hasfocus == 1) {
                     red = getr(CGUI_list_row_b_color) / 2 + getr(CGUI_list_row_bar_diagram_color) / 2;
                     green = getg(CGUI_list_row_b_color) / 2 + getg(CGUI_list_row_bar_diagram_color) / 2;
                     blue = getb(CGUI_list_row_b_color) / 2 + getb(CGUI_list_row_bar_diagram_color) / 2;
                     CGUI_list_row_bar_diagram_color = makecol(red, green, blue);
                  }
                  rectfill(bmp, bgx, y1, bar_end_x-1, y2 - 1, CGUI_list_row_bar_diagram_color);
                  rectfill(bmp, bar_end_x, y1, x2, y2 - 1, CGUI_list_row_b_color);
               } else if (l->column_selection && b->hasfocus && b->action_by_mouse) {
                  /* We wait a while with drawing the backgroud */
               } else {
                  /* Normal case */
                  rectfill(bmp, bgx, y1, x2, y2 - 1, CGUI_list_row_b_color);
               }
            }
            if (l->column_selection && b->hasfocus && b->action_by_mouse) {
               /* Then the back-ground is not yet drawn: we should only highlight the background of
                  the clicked column. */
               if (colnr == GetClickedColumn(b)) {
                  /* Set to cgui's default selected color. */
                  colum_color = cgui_colors[CGUI_COLOR_LISTBOX_FOCUS_BACKGROUND];
                  CGUI_list_row_f_color = fcolor = cgui_colors[CGUI_COLOR_LISTBOX_FOCUS_TEXT];
               } else {
                  /* Set to cgui's default unselected color. */
                  colum_color = cgui_colors[CGUI_COLOR_LISTBOX_BACKGROUND];
                  CGUI_list_row_f_color = fcolor = cgui_colors[CGUI_COLOR_LISTBOX_TEXT];
               }
               rectfill(bmp, column_start, y1, column_end, y2 - 1, colum_color);
            }
            if (l->treeview && colnr == 0) {
               text_x = DrawNode(bmp, GetTreeNode(b), text_x, y1, y2-1, l->level_width, l);
            }
            if (ret & (ROW_UNCHECK | ROW_CHECK)) {
               if (ret & ROW_CHECK)
                  DrawCheckMark(b, text_x + 4, TEXTY(b, 2));
               if (ret & (ROW_UNDERLINE | ROW_STRIKE))
                  strike_x = CHECKSPACE;
               text_x += CHECKSPACE;
            }
            column_underline = ret & ROW_COLUMN_UNDERLINE;
            uline |= ret & ROW_UNDERLINE;
            strike |= ret & ROW_STRIKE;
            if (b->hasfocus == 1 && CGUI_list_row_b_color != bcolor) {
               /* The row is selected and should be highlighed and at the same time app wants to have a
                  different background. We use some heuristics to get merged colors. */
               red = getr(CGUI_list_row_b_color) / 2;
               green = getg(CGUI_list_row_b_color)/2;
               blue = (getb(CGUI_list_row_b_color) + 255)/2;
               CGUI_list_row_b_color = makecol(red, green, blue);
               CGUI_list_row_f_color = makecol(255-red, 255-green, 255-blue);
            }
            available_text_width = x_end - text_x;
            if (l->column_widths) {
               set_clip_rect(bmp, x1, y1, column_end, y2);
            }
            if (HAS_IMAGE(s)) {
               im = ExtractImage(b, &s);
               if (im) {
                  if (l->column_widths) {
                     set_clip_rect(bmp, x1, y1, column_end, y2);
                  }
                  imy = y1 + (l->lc->rh - im->h) / 2;
                  DrawImage(im, bmp, text_x, imy);
                  text_x += im->w;
                  available_text_width -= im->w;
               }
            }
            if (CGUI_list_fixfont) {
               if (right_align) {
                  alx = text_x + l->column_widths[colnr] - strlen(s) * CGUI_list_fixfont - 2;
               } else {
                  alx = text_x;
               }
               FixTextOut(bmp, CGUI_list_row_font, s, alx, y1 + l->offs, CGUI_list_row_f_color, CGUI_list_row_b_color, CGUI_list_fixfont);
               tot_text_length = CGUI_list_fixfont * strlen(s);
            } else if (cgui_list_fix_digits) {
               fixw = FixDigitTextOut(bmp, CGUI_list_row_font, s, right_align, text_x, x_end, y1 + l->offs,
                                      CGUI_list_row_f_color, CGUI_list_row_b_color);
               tot_text_length = fixw * strlen(s);
            } else {
               tot_text_length = text_length(CGUI_list_row_font, s);
               if (right_align) {
                  alx = x_end - tot_text_length - 1;
               } else {
                  alx = text_x;
               }
               textout_ex(bmp, CGUI_list_row_font, s, alx, y1 + l->offs, CGUI_list_row_f_color, -1);
               if (column_underline) {
                  hline(bmp, alx, y1 + GetBaseLine(CGUI_list_row_font), alx+tot_text_length, CGUI_list_row_f_color);
               }
            }
            set_clip_rect(bmp, x1, y1, x2, y2);
            if (tot_text_length >= available_text_width) {
               is_clipped = 1;
            }
            if (l->column_widths) {
               col_x += l->column_widths[colnr];
            }
            if (l->col_delimiter) {
               vline(bmp, column_end - 1, y1, y2, l->column_delimiter_color);
            }
         } /* End column iteration */
         if (l->column_selection && b->hasfocus && l->column_widths) {
            /* Fill the unused space to the right of the last column. */
            rectfill(bmp, column_end+1, y1, x2, y2 - 1, cgui_colors[CGUI_COLOR_LISTBOX_BACKGROUND]);
         }
      } else {
         /* Below list end - no selection, no user data just an empty row */
         rectfill(bmp, x1, y1, x2, y2 - 1, bcolor);
      }
      /* Handle line strike and underline. */
      if (appdata && l->nrcols && strike) {
         if (l->column_widths)
            col_x -= l->column_widths[l->nrcols - 1];
         col_x += text_length(CGUI_list_row_font, s);
         if (strike)
            hline(bmp, strike_x, y1 + l->offs + (text_height(l->font) >> 1) + 2, col_x, CGUI_list_row_f_color);
         if (uline)
            hline(bmp, strike_x, y1 + GetBaseLine(CGUI_list_row_font), col_x, CGUI_list_row_f_color);
      }
      /* Handle tool tip. */
      if (l->show_tool_tip) {
         prev_parse_label_state = CguiParseLabels(0);
         if (is_clipped) {
            j = 0;
            for (colnr = 0; colnr < l->nrcols; colnr++) {
               s = buf;
               if (l->CreateColumnTextOfRow) {
                  l->CreateColumnTextOfRow(appdata, s, colnr);
               } else if (l->CreateRowText) {
                  l->CreateRowText(appdata, s);
               }
               if (HAS_IMAGE(s)) {
                  ExtractImage(b, &s);
               }
               for (p=s; *p; p++) {
                  tttext[j++] = *p;
               }
               tttext[j++] = ' ';
               tttext[j++] = ' ';
            }
            tttext[j] = 0;
            ToolTipText(b->id, tttext);
         } else {
            ClearToolTip(b);
         }
         CguiParseLabels(prev_parse_label_state);
      }
   } else {
      rectfill(bmp, x1, y1, x2, y2 - 1, bcolor);
   }
}

static void ListUnFocus(t_object *nb)
{
   t_list *l;
   t_object *b;

   if (nb->node == NULL)
      return;
   l = nb->node->data;
   if (l) {
      for (b = l->rowsnode->firstob; b; b = b->next) {
         if (b->hasfocus) {
            b->tcfun->UnSetFocus(b);
         }
      }
      nb->tf->Refresh(nb);
   }
}

static int RowSetFocus(t_object *sf)
{
   t_list *l, *l2;
   int i, id;

   l = GetListRef(sf);
   sf->hasfocus = 1;
   if (l->NotifyMove) {
      i = GetRowIndex(sf);
      if (!l->notifying_in_progress && i < *l->n) {
         /* it is quite reasonable if Notifier call back wants to refresh all
            prevent from infinite recurson for such cases */
         l->notifying_in_progress = 1;
         id = l->rowsnode->ob->parent->ob->parent->ob->id;
         l->NotifyMove(sf->id, GetApplicationData(i, l, sf));
         l2 = GetList(id);
         if (l2 != l)
            return 0;
         l->notifying_in_progress = 0;
      }
   }
   return 1;
}

static int isShift(void)
{
   return key[KEY_LSHIFT] || key[KEY_RSHIFT];
}

static int isControl(void)
{
   return key[KEY_LCONTROL] == -1 || key[KEY_RCONTROL];
}

/* This function switches selection state of b */
static void XorClick(t_object *b)
{
   if (b->hasfocus) {
      b->tcfun->UnSetFocus(b);
      b->tf->Refresh(b);
   } else {
      SetSubFocusOfRow(b);
      b->tf->Refresh(b);
   }
   SetMetaClick(b, 1);
}

/* This function sets the focus on b. Then it selects all rows between b and
   the previous focus (including) */
static void BlockClick(t_object *b, t_object *nf, t_object *cursf)
{
   t_object *p;
   int currn, rn;

   currn = GetRowNumber(cursf);
   rn = GetRowNumber(b);
   TellSubFocusOfRow(b);
   if (rn < currn) {
      for (p = b; p && p != cursf; p = p->next)
         p->tcfun->SetFocus(p);
   } else {
      for (p = b; p && p != cursf; p = p->prev)
         p->tcfun->SetFocus(p);
   }
   nf->tf->Refresh(nf);
   SetMetaClick(b, 1);
}

/* This function unselects all other rows but b in the list l, and focuses b */
static void MakeSingleRowSelected(t_list *l, t_object *b)
{
   t_object *sf;

   for (sf = l->rowsnode->firstob; sf; sf = sf->next) {
      if (sf->hasfocus && sf != b) {
         sf->tcfun->UnSetFocus(sf);
      }
   }
   SlaveMoveFocusTo(b);
}

/* This function handles the processing needed when a row in a list is taking
   focus, this may include selecting/unselecting that or other rows. - moves
   the rowfocus in all the other lists within the same listbox to the
   correesponding row - if the current list already has tab-focus: check if
   meta-key pressed and handle according to that (shiftkey=mark block;
   ctrlkey=change selection;noneofthem=default single click otherwise -
   ignore all key-presses and process the handle a default single click */
static int RowClickPerformed(t_object *b)
{
   t_object *nf, *sf, *lsf;
   t_node *rowsnode;
   struct t_window *w;
   t_list *l, *p;
   t_listchain *lc;
   int i, rn;

   l = GetListRef(b);
   lc = l->lc;
   rn = GetRowNumber(b);
   rowsnode = l->rowsnode;
   if (rn >= *l->n - lc->sti) {
      static int was_here;
      if (was_here)
         return 1;
      was_here = 1;
      if (*l->n > 0)
         BrowseTo(rowsnode->ob->id, *l->n-1, 1);
      else if (rowsnode->firstob)
         MakeSingleRowSelected(l, rowsnode->firstob);
      was_here = 0;
      return 1;
   }
   /* search (optional) chained lists to set the subfocus of the
      corresponding row in these */
   for (p = lc->l; p; p = p->next) {
      if (p != l) {
         for (i = 0, lsf = p->rowsnode->firstob; i < rn && lsf; i++, lsf = lsf->next) {
            ;
         }
         if (lsf) {
            TellSubFocusOfRow(lsf);
            /* ... but don't show it, that *list-box* is not in focus */
         }
      }
   }
   w = rowsnode->win;
   GetTabChainFocus(w, &nf, &sf);
   /* if b's list is not in focus the shift/ctrl-keys shall be ignored */
   if (nf == rowsnode->ob) {
      if (isShift() && !l->single_row_sel) {
         BlockClick(b, nf, sf);
      } else if (isControl() && !l->single_row_sel) {
         XorClick(b);
      } else if (b->usedbutton == LEFT_MOUSE || b->usedbutton == RIGHT_MOUSE) {
         MakeSingleRowSelected(l, b);
      } else if (!b->hasfocus) {
         /* if right button then handle as left if not within a marked block */
         MakeSingleRowSelected(l, b);
      }
   } else {
      MakeSingleRowSelected(l, b);
   }
   return 1;
}

typedef struct t_action_wrapper {
   void **data;
   int *ids;
   void (*CallBack) (int, void *);
   int n;
   t_list *l;
} t_action_wrapper;

/* This function generates necessary data strucuters for making a series of call-backs,
   indicates in the list object that launching is in progress, and then perform the
   call-backs. If there are severeal rows involved (i.e. several call-backs, then the
   launching can occasionally be interrupted by the list destructor.
   When finished indicate to the list object (if it still exits) that launching of
   call-backs is no longer in progress. */
static void LaunchCallBacks(void (*CallBack) (int, void *), t_list *l, t_object *b)
{
   t_object *sf;
   t_action_wrapper *action_wrapper;
   int i, j, n;

   if (CallBack && (*l->n > 0)) {
      /* Count selected rows */
      for (sf = l->rowsnode->firstob, n = 0; sf; sf = sf->next)
         if (sf->hasfocus && (n < *l->n))
            n++;
      /* Collect all data necessary for the calls, then call from a secondary event */
      action_wrapper = GetMem0(t_action_wrapper, 1);
      action_wrapper->ids = GetMem(int, n);
      action_wrapper->data = GetMem(void*, n);
      action_wrapper->n = n;
      action_wrapper->CallBack = CallBack;
      action_wrapper->l = l;
      l->action_wrapper = action_wrapper;
      for (sf = l->rowsnode->firstob, i = j = 0; sf; sf = sf->next, i++) {
         if (sf->hasfocus && (i < *l->n)) {
            action_wrapper->ids[j] = sf->id;
            action_wrapper->data[j] = GetApplicationData(i + l->lc->sti, l, sf);
            if (b)
               sf->usedbutton = b->usedbutton;
            else
               sf->usedbutton = 0;
            j++;
         }
      }
      for (i=0; i<action_wrapper->n && action_wrapper->l; i++) {
         action_wrapper->CallBack(action_wrapper->ids[i], action_wrapper->data[i]);
      }
      if (action_wrapper->l)
         action_wrapper->l->action_wrapper = NULL;
      Release(action_wrapper->data);
      Release(action_wrapper->ids);
      Release(action_wrapper);
   }
}

static void DrawNewStart(t_listchain *lc)
{
   t_list *l;

   CalcMaxN(lc);
   lc->pos = lc->sti*lc->rh;
   NotifyBrowser(lc->br, lc->rh, lc->n*lc->rh);
   if (!IsBrowserActive(lc->br)) {
      lc->sti = 0;
   }
   Refresh(lc->br);
   for (l = lc->l; l; l = l->next)
      l->rowsnode->ob->tf->Refresh(l->rowsnode->ob);
}

static void UpdateTreeView(t_list *l)
{
   l->tree_rebuild_disabled = 1;
   Refresh(l->rowsnode->ob->id);
   l->tree_rebuild_disabled = 0;
   DrawNewStart(l->lc);
}

/* This function will take an row-object, find its list-box, and launch the Action specified by the application for all selected
   rows in that list-box. If the selected row is in the empty part, below the list of existing rows, then nothing will be done*/
static void RowAction(void *data)
{
   t_listrow *rd = data;
   t_object *b;
   t_list *l;

   b = rd->b;
   ComputeClickedColumn(b);
   l = GetListRef(b);
   if (IsMetaClick(b)) {        /* ... but ignore if it is a meta click */
      SetMetaClick(b, 0);
      LaunchCallBacks(l->Action, l, b);
      return;
   }
   if (GetRowIndex(b) < *l->n) {
      if (l->treeview && IsInternal(b) && b->x < UserStartX(b)) {
         ToggleExpanded(b);
         UpdateTreeView(l);
      } else {
         LaunchCallBacks(l->Action, l, b);
      }
   } else {
      if (l->BelowListEndCallBack)
         l->BelowListEndCallBack(b->id, l->below_list_end_data);
   }
}

static void RowDouble(void *data)
{
   t_object *b = data;
   t_list *l;
   int i;

   l = GetListRef(b);
   i = GetRowIndex(b);
   if (l->Action && (*l->n > 0) && (i < *l->n)) {
      l->Double(b->id, GetApplicationData(i, l, b), b->usedbutton);
   }
}

/* This function will notify the application that the delete-key was pressed
   There will be one call for each marked row. To the event q is used to
   serialize these actions. */
static void DeleteListItem(t_list *l)
{
   LaunchCallBacks(l->DelHandler, l, NULL);
}

static t_object *InsertListItem(t_list *l, t_object *f)
{
   int i = 0;

   if (l->InsHandler) {
      if (f)
         i = GetRowIndex(f);
      if (i < *l->n && (i >= 0))
         l->InsHandler(l->applist, i);
   }
   return NULL;
}

/* Will be called each time the user uses the browser (i.e. clicks or drags
   on the browse-bar or clicks on the scroll-buttons) */
static void BrCallBack(void *data)
{
   t_listchain *lc = data;
   t_list *l;
   int n, sti;
   if (lc->rh)
      sti = lc->pos / lc->rh;
   else
      sti = 0;
   if (sti != lc->sti) {
      n = lc->n;
      if (sti >= n) {           /* should not occure? Check for safety */
         for (l = lc->l, n = 0; l; l = l->next) {
            if (*l->n > n)
               n = *l->n;
         }
         if (lc->n != n)
            lc->n = n;
         sti = n - 1;
      }
      if (sti < 0)
         sti = 0;
      lc->sti = sti;
      lc->l->rowsnode->ob->tcfun->MoveFocusTo(lc->l->rowsnode->ob);
      for (l = lc->l; l; l = l->next)
         l->rowsnode->ob->tf->Refresh(l->rowsnode->ob);
      lc->pos = sti*lc->rh;
      Refresh(lc->br);
   }
}

static void DrawRowsNode(t_object *ob)
{
   t_list *l;
   t_listchain *lc;
   t_object *b, *tmp = NULL;
   t_object *focused_row;
   t_node *rowsnode;
   int issel = 0, selbelow = 0;

   rowsnode = ob->node;
   l = rowsnode->data;
   lc = l->lc;
   CalcMaxN(lc);
   for (b = rowsnode->firstob; b; b = b->next) {
      if (b->hasfocus) {
         if (GetRowIndex(b) >= *l->n) {
            if (*l->n) {
               b->tcfun->UnSetFocus(b);
               selbelow = 1;
               if (tmp == NULL)
                  tmp = b;
            } else {
               issel = 1;
            }
         } else {
            issel = 1;
         }
      }
   }
   if (!issel) {
      if (selbelow) {           /* just removed focus from below end: find last row */
         if (tmp)
            tmp = tmp->prev;
         if (tmp == NULL)
            tmp = rowsnode->firstob;
         if (tmp)
            tmp->tcfun->MoveFocusTo(tmp);
         if (*l->n <= lc->sti)
            lc->sti = *l->n - 1;
         if (lc->sti < 0)
            lc->sti = 0;
      }
   }
   lc->pos = lc->sti*lc->rh;
   focused_row = GetSubFocus(l->rowsnode);
   if (focused_row) {
      if (GetRowIndex(focused_row) + lc->sti >= *l->n) {
//         MoveFocusToLastNonEmptyRow(l);
      }
   }
}

/* ArrowUp scrolls up the list on user command (pressing the arrow-up key) */
static t_object *ArrowUp(t_list *l, t_object *f)
{
   t_listchain *lc;
   int rn, i;
   t_object *b = NULL, *lsf;
   t_list *p;

   lc = l->lc;
   if (f == NULL) {
      b = l->rowsnode->last;
      rn = GetRowNumber(b);
   } else {
      rn = GetRowNumber(f);
      if (rn > 0) { /* Then there is at least one object before */
         b = f->prev;
         rn = GetRowNumber(b);
         /* If the list is not properly refreshed the current row may be below
         bottom, go upward to find the position above the last row */
         for (; b && GetRowNumber(b) + lc->sti >= *l->n - 1; b = b->prev)
            ;
         if (b == NULL)
            b = f->prev;
         rn = GetRowNumber(b);
      } else if (lc->sti > 0) {
         lc->sti--;
         DrawNewStart(lc);
      }
   }
   for (p = lc->l; p; p = p->next) {
      if (p != l) {
         for (i = 0, lsf = p->rowsnode->firstob; i < rn && lsf;
              i++, lsf = lsf->next);
         if (lsf)
            TellSubFocusOfRow(lsf);
      }
   }
   return b;
}

static void ShiftArrowUp(t_object *f)
{
   t_object *b;

   if (f == NULL)
      return;
   b = f->prev;
   if (b) {
      SetSubFocusOfRow(b);
      b->tf->Refresh(b);
   }
}

/* ArrowDown scrolls down the list on user command (pressing the arrow-down
   key) */
static t_object *ArrowDown(t_list *l, t_object *f)
{
   t_listchain *lc;
   int i, rn;
   t_object *b = NULL, *lsf;
   t_list *p;

   if (f == NULL)
      f = l->rowsnode->last;
   if (f) {
      lc = l->lc;
      rn = GetRowNumber(f);
      i = GetRowIndex(f);
      if (i < *l->n - 1) {
         if (rn < lc->rows - 1) {
            b = f->next;
            rn++;
         } else {
            lc->sti++;
            DrawNewStart(lc);
            b = f;
         }
         for (p = lc->l; p; p = p->next) {
            if (p != l) {
               for (i = 0, lsf = p->rowsnode->firstob; i < rn && lsf;
                    i++, lsf = lsf->next);
               if (lsf)
                  TellSubFocusOfRow(lsf);
            }
         }
      }
   }
   return b;
}

static void ShiftArrowDown(t_list *l, t_object *f)
{
   t_listchain *lc;
   t_object *b;
   int rn;

   b = f->next;
   rn = GetRowNumber(f);
   lc = l->lc;
   if (b && rn + lc->sti + 1 < lc->n) {
      SetSubFocusOfRow(b);
      b->tf->Refresh(b);
   }
}

static t_object *PageUpList(t_list *l, t_object *sf)
{
   t_listchain *lc;
   int sti, rn;
   t_object *b = NULL, *lsf;
   t_list *p;

   if (sf == NULL)
      sf = l->rowsnode->last;
   if (sf) {
      lc = l->lc;
      rn = GetRowNumber(sf);
      sti = lc->sti - lc->rows + rn + 1;
      if (sti < 0)
         sti = 0;
      lc->sti = sti;
      if (rn)
         if (l->rowsnode->firstob)
            l->rowsnode->firstob->tcfun->MoveFocusTo(l->rowsnode->firstob);
      for (p = lc->l; p; p = p->next) {
         if (p != l) {
            lsf = p->rowsnode->firstob;
            if (lsf)
               TellSubFocusOfRow(lsf);
         }
      }
      DrawNewStart(lc);
   }
   return b;
}

static void ShiftPageUpList(t_node *rowsnode, t_object *f)
{
   t_object *b;

   b = rowsnode->firstob;
   if (b) {
      SetSubFocusOfRow(b);
   }
   for (b = b->next; b && b != f->next; b = b->next)
      b->tcfun->SetFocus(b);
   rowsnode->ob->tf->Refresh(rowsnode->ob);
}

static t_object *PageDownList(t_list *l, t_object *sf)
{
   t_listchain *lc;
   t_list *p;
   int sti, i, rn, newrow, d;
   t_object *b = NULL, *lsf;

   if (sf == NULL)
      sf = l->rowsnode->last;
   if (sf) {
      lc = l->lc;
      rn = GetRowNumber(sf);
      d = lc->rows - rn;
      sti = lc->sti + lc->rows - d;
      if (sti >= lc->n)
         sti = lc->n - 1;
      if (sti < 0) /* May be if empty list! */
         sti = 0;
      lc->sti = sti;
      if (lc->sti + lc->rows > lc->n) {
         newrow = lc->n - lc->sti - 1;
         if (newrow != rn) {
            for (i = 0, b = l->rowsnode->firstob; i < newrow && b; i++, b = b->next)
               ;
            if (b)
               b->tcfun->MoveFocusTo(b);
         }
         rn = newrow;
      } else {
         if (rn != lc->rows - 1) {
            sf->tcfun->UnSetFocus(sf);
            sf->tf->Refresh(sf);
            if (l->rowsnode->last) {
               SetSubFocusOfRow(l->rowsnode->last);
            }
            rn = lc->rows - 1;
         }
      }
      for (p = lc->l; p; p = p->next) {
         if (p != l) {
            for (i = 0, lsf = p->rowsnode->firstob; i < rn && lsf;
                 i++, lsf = lsf->next);
            if (lsf)
               TellSubFocusOfRow(lsf);
         }
      }
      DrawNewStart(lc);
   }
   return b;
}

static void ShiftPageDownList(t_list *l, t_object *sf)
{
   t_listchain *lc;
   t_node *rowsnode;
   t_object *b;
   int i;

   rowsnode = l->rowsnode;
   lc = l->lc;
   for (b = rowsnode->last, i = lc->rows; b && i > lc->n - lc->sti; b = b->prev)
      i--;
   if (b) {
      SetSubFocusOfRow(b);
   }
   for (b = b->prev; b && b != sf->prev; b = b->prev)
      b->tcfun->SetFocus(b);
   rowsnode->ob->tf->Refresh(rowsnode->ob);
}

static t_object *ArrowRightFocus(t_list *l, t_object *sf)
{
   t_object *b;
   t_node *rowsnode;

   if (sf) {
      sf->tcfun->UnSetFocus(sf);
      sf->tf->Refresh(sf);
   }
   if (l->prev)
      l = l->prev;
   else
      l = l->lc->last;
   rowsnode = l->rowsnode;
   rowsnode->ob->tcfun->MoveFocusTo(rowsnode->ob);
   GetTabChainFocus(rowsnode->win, &b, &sf);
   return sf;
}

static t_object *ArrowLeftFocus(t_list *l, t_object *sf)
{
   t_object *b;
   t_node *rowsnode;

   if (sf) {
      sf->tcfun->UnSetFocus(sf);
      sf->tf->Refresh(sf);
   }
   if (l->next)
      l = l->next;
   else
      l = l->lc->l;
   rowsnode = l->rowsnode;
   rowsnode->ob->tcfun->MoveFocusTo(rowsnode->ob);
   GetTabChainFocus(rowsnode->win, &b, &sf);
   return sf;
}

/* This is the "MoveFocusToNextSubObject" function for the list node. In addition
to the keys for moving focus it also catches the delete/insert keys. */
static int ListMoveFocusToNextSubObject(t_object *nf, t_object *sf, int scan, int ascii)
{
   t_node *rowsnode;
   t_list *l;
   t_object *newsf = NULL;
   (void)ascii;

   rowsnode = nf->node;
   l = rowsnode->data;
   sf->action_by_mouse = 0;
   switch (scan) {
   case KEY_DOWN:
      if (sf && isShift() && !l->single_row_sel) {
         ShiftArrowDown(l, sf);
      } else {
         newsf = ArrowDown(l, sf);
      }
      break;
   case KEY_UP:
      if (sf && isShift() && !l->single_row_sel) {
         ShiftArrowUp(sf);
      } else {
         newsf = ArrowUp(l, sf);
      }
      break;
   case KEY_PGUP:
      if (sf && isShift() && !l->single_row_sel) {
         ShiftPageUpList(rowsnode, sf);
      } else {
         PageUpList(l, sf);
      }
      break;
   case KEY_PGDN:
      if (sf && isShift() && !l->single_row_sel) {
         ShiftPageDownList(l, sf);
      } else {
         PageDownList(l, sf);
      }
      break;
   case KEY_HOME:
      l->lc->sti = 0;
      PageUpList(l, sf);
      break;
   case KEY_END:
      l->lc->sti = l->lc->n - (l->lc->rows - 1);
      if (l->lc->sti < 0)
         l->lc->sti = 0;
      PageDownList(l, sf);
      break;
   case KEY_LEFT:
      if (l->treeview && IsInternal(sf) && IsExpanded(sf)) {
         ToggleExpanded(sf);
         UpdateTreeView(l);
         newsf = NULL;
      } else {
         newsf = ArrowLeftFocus(l, sf);
      }
      break;
   case KEY_RIGHT:
      if (l->treeview && IsInternal(sf) && !IsExpanded(sf)) {
         ToggleExpanded(sf);
         UpdateTreeView(l);
         newsf = NULL;
      } else {
         newsf = ArrowRightFocus(l, sf);
      }
      break;
   case KEY_DEL:
      DeleteListItem(l);
      break;
   case KEY_INSERT:
      InsertListItem(l, sf);
      break;
   default:
      return 0;
   }
   if (newsf) {
      newsf->action_by_mouse = 0;
      newsf->usedbutton = LEFT_MOUSE;
      newsf->tcfun->MoveFocusTo(newsf);
   }
   return 1;
}

/* This is just a wrapper for the default behaviour. The main-container needs
   since it is that one who will get the request from the application, but
   it is the inner node (the one which containes the row), that is actually
   in the chain, and there by the only one who can take the focus. */
static int ListContMoveFocusTo(t_object *b)
{
   t_list *l;

   l = GetList(b->id);
   return l->rowsnode->ob->tcfun->MoveFocusTo(l->rowsnode->ob);
}

static int RowDrop(void *dest, int id nouse, void *src, int reason, int flags)
{
   t_object *b = dest;
   t_list *l;
   int i, ok;
   void *appdata;

   l = GetListRef(b);
   i = GetRowIndex(b);
   switch (reason) {
   case DD_OVER_DROP:
      if (i < *l->n) {
         appdata = GetApplicationData(i, l, b);
      } else {
         appdata = NULL;
      }
      ok = l->Drop(appdata, reason, src, l->applist, i, flags);
      if (ok) {
         MakeSingleRowSelected(l, b);
      }
      return ok;
   case DD_END_OVER_DROP:
   case DD_SUCCESS:
      if (i < *l->n) {
         appdata = GetApplicationData(i, l, b);
      } else {
         appdata = NULL;
      }
      ok = l->Drop(appdata, reason, src, l->applist, i, flags);
      return ok;
   }
   return 1;
}

static void *RowGrip(void *data, int id nouse, int reason)
{
   static t_object *b;
   static t_list *l;
   char s[1000], *p, c;
   t_image *im;
   static int index;
   int st, i, x, hotx;
   BITMAP *bmp;
   void *appdata = NULL;
   int mx, my, mz;

   switch (reason) {
   case DD_OVER_GRIP:
      b = data;
      l = GetListRef(b);
      index = GetRowIndex(b);
      appdata = GetApplicationData(index, l, b);
      st = l->Grip(appdata, reason, l->applist, index);
      if (!st)
         return NULL;
      break;
   case DD_GRIPPED:
      b = data;
      if (l->user_data_type == ITERATE_INDEXED)
         if (index >= *l->n)
            return NULL;
      appdata = GetApplicationData(index, l, b);
      if (l->CreateColumnTextOfRow) {
         if (l->column_widths) {
            for (i = x = 0; i < l->nrcols; i++)
               x += l->column_widths[i];
            bmp = create_bitmap(x, l->lc->rh);
            if (bmp) {
               clear_to_color(bmp, bitmap_mask_color(bmp));
               for (i = x = 0; i < l->nrcols; i++) {
                  l->CreateColumnTextOfRow(appdata, s, i);
                  if (*s == '#' && s[1] != '#') {
                     for (p = s + 1; *p && *p != ';'; p++)
                        ;
                     c = *p;
                     *p = 0;
                     im = GetImage(b->parent, s + 1);
                     if (im)
                        DrawImage(im, bmp, x, 0);
                  } else {
                     textout_ex(bmp, l->font, s, x, 0, cgui_colors[CGUI_COLOR_DRAGGED_TEXT], -1);
                  }
                  x += l->column_widths[i];
               }
               _CguiMousePos(&mx, &my, &mz);
               hotx = mx - (b->parent->win->node->ob->x1+b->parent->wx);
               OverlayPointer(bmp, hotx, l->lc->rh / 2);
               destroy_bitmap(bmp);
            }
         }
      } else if (l->CreateRowText) {
         l->CreateRowText(appdata, s);
         MkTextPointer(l->font, s);
      }
      l->Grip(appdata, reason, l->applist, index);
      break;
   case DD_UNGRIPPED:
      appdata = GetApplicationData(index, l, b);
      RemoveOverlayPointer();
      l->Grip(appdata, reason, l->applist, index);
      break;
   case DD_SUCCESS:
      RemoveOverlayPointer();
      appdata = GetApplicationData(index, l, b);
      l->Grip(appdata, reason, l->applist, index);
      break;
   }
   return appdata;
}

/* This function sets the object creation function (application call-back).It
   will also close the node for input to make it possible to add other objects
   in the same node as where the list-box is */
static int SetList(int id, void *creater, int user_data_type)
{
   t_list *l;

   l = GetList(id);
   if (l) {
      l->item.IterateUsersLinkedList = creater;
      l->user_data_type = user_data_type;
      if (user_data_type == ITERATE_LINKED) {
         l->n = &l->memofn;
      }
      return 1;
   }
   return 0;
}

static void SetRowSize(t_object *b)
{
   t_list *l;

   l = b->parent->data;
   b->x1 = b->y1 = 0;
   if (b == l->rowsnode->last)
      b->y2 = l->height - l->lc->rh *(l->lc->rows - 1) - 1;
   else
      b->y2 = l->lc->rh - 1;
}

static t_object *Row2ListNode(t_object *b)
{
   if (b->parent && b->parent->ob->parent
       && b->parent->ob->parent->ob->parent) return b->parent->ob->parent->
          ob->parent->ob;
   return NULL;
}

static void HandleMouseWheel(void *data)
{
   t_object *b=data;
   t_list *l;
   t_listchain *lc;
   int x, y, z, diff, eventid;

   l = GetListRef(b);
   lc = l->lc;
   _CguiMousePos(&x, &y, &z);
   diff = z - GetZ(b);
   if (diff != 0) {
      SetZ(b, z);
      lc->sti -= diff;
      if (lc->sti > lc->n - 1) {
         lc->sti = lc->n - 1;
      } else if (lc->sti < 0) {
         lc->sti = 0;
      }
      DrawNewStart(lc);
   }
   eventid = _GenEventOfCgui(HandleMouseWheel, b, 100, b->id);
   SetEventId(b, eventid);
}

static void RowOver(t_object *b)
{
   t_list *l;
   int x, y, mx, my, mz;
   t_object *object_of_window;
   t_node *parent;

   Refresh(b->id);
   l = GetListRef(b);
   if (b->state == MS_OVER) {
      if (l->show_tool_tip && b->tooltip) {
         parent = b->parent;
         object_of_window = parent->win->node->ob;
         x = b->x1 + parent->wx + object_of_window->x1;
         y = b->y1 + parent->wy + object_of_window->y1;
         ShowToolTipAt(b, b->tooltip, 1, text_length(b->font, b->tooltip), x, y);
      }
      _CguiMousePos(&mx, &my, &mz);
      SetZ(b, mz);
      HandleMouseWheel(b);
   } else {
      HideToolTip(b);
      _KillEventOfCgui(GetEventId(b));
   }
}

static t_object *MakeRow(t_list *l, int rowno)
{
   t_object *b;
   t_listrow *rd;
   static int virgin = 1;
   static t_tcfun rtc;
   static t_typefun rtf;

   if (virgin) {
      virgin = 0;
      rtc = default_slave_tc_functions;
      rtc.SetFocus = RowSetFocus;
      rtc.MoveFocusTo = RowClickPerformed;

      rtf = default_type_functions;
      rtf.Free = XtendedFree;
      rtf.Draw = Draw;
      rtf.SetSize = SetRowSize;
      rtf.GetMainNode = Row2ListNode;
   }
   b = CreateObject(DOWNLEFT | FILLSPACE, l->rowsnode);
   rd = GetMem0(t_listrow, 1);
   b->appdata = rd;
   rd->b = b;
   b->Action = RowAction;
   b->Over = RowOver;
   b->click = l->events;
   b->exclude_rect = 1;
   b->tf = &rtf;
   b->tcfun = &rtc;

   SetRowNumber(b, rowno);
   SetListRef(b, l);
   return b;
}

static void AdjustListHeight(t_list *l)
{
   t_object *b;
   t_listchain *lc;
   int height;

   lc = l->lc;
   while (l->height > lc->rh * lc->rows) {
      b = MakeRow(l, lc->rows++);
      if (l->Grip)
         SetObjectGrippable(b->id, RowGrip, l->gripflags, l->gripbuttons, b);
      if (l->Drop)
         SetObjectDroppable(b->id, RowDrop, l->dropflags, b);
      if (l->Double)
         SetObjectDouble(b->id, RowDouble, b, l->doublebuttons);
   }
   while (l->height < lc->rh * (lc->rows - 1)) {
      b = l->rowsnode->last;
      if (b) {
         b->tf->Unlink(b);
         b->tf->Free(b);
         lc->rows--;
      } else
         break;
   }
   height = l->framenode->ob->y2 - l->framenode->ob->y1 + 1;
   SetBrowserSize(lc->br, l->height-2, height);
}

static void AdjustRowsWidths(t_list *l)
{
   t_object *row;

   for (row = l->rowsnode->last; row; row = row->prev)
       row->x2 = row->x1 + l->width - 1;
}

static void ListStretch(t_object *b, int dx, int dy)
{
   t_listchain *lc;
   t_list *l;

   b->rex += dx;
   b->rey += dy;
   lc = b->node->data;
   l = lc->l;
   l->rowsnode->ob->rey += dy;
   l->rowsnode->ob->rex += dx;
   if (l->rowsnode->last)
      l->rowsnode->last->y2 = l->rowsnode->last->y1 + lc->rh - 1;
   l->width = l->orgwidth + b->rex;
   l->height = l->orgheight + b->rey;
   AdjustListHeight(l);
   AdjustRowsWidths(l);
}

/* Sets the logical focus to be row i for all lists in the list-chain. If
   any of the rows has the actual focus in the window (i.e. highlighted) it
   will be reset. Returns the object in list `l' that keeps row `i' */
static t_object *NewBrowseLFocus(int i, t_listchain *lc, t_list *l)
{
   t_list *p;
   int sti, n;
   t_object *b = NULL, *lsf;

   sti = i - (lc->rows - 1);
   if (sti < 0)
      sti = 0;
   lc->sti = sti;

   /* Set the row-focus properly (loop from bottom) */
   n = (lc->rows - 1) - (i - sti);
   for (p = lc->l; p; p = p->next) {
      for (i = 0, lsf = p->rowsnode->last; lsf; lsf = lsf->prev, i++) {
         if (lsf->hasfocus)
            lsf->tcfun->UnSetFocus(lsf);
         if (i == n) {
            TellSubFocusOfRow(lsf);
            if (p == l)
               b = lsf;
         }
      }
   }
   return b;
}

/* Call-back to rows node object, for packing. */
static void ListPack(t_object *b)
{
   t_list *l;
   t_object *bs;
   t_node *framenode;
   int i;

   l = b->node->data;
   l->Pack(b); /* Default packer */
   if (l->lc->l != l) {  /* a list-chain and not the logically first (rightmost) */
      for (i = 0, bs = b->node->firstob; bs; bs = bs->next, i++)
         ;
      l->lc->rows = i;
   }
   if (!l->done) {
      l->done = 1;
      framenode = l->framenode;
      if (l->lc->fillw)
         l->width = framenode->ob->x2 - framenode->ob->x1 - 1;
      if (l->lc->fillh) {
         l->height = framenode->ob->y2 - framenode->ob->y1 - 1;
         if (l->header) {
            l->height -= l->header->ob->y2 - l->header->ob->y1;
         }
         l->orgheight = l->height - framenode->ob->rey;
      }
   }
   AdjustListHeight(l);
   /* Re-Set size and position of kids */
   for (bs = b->node->firstob; bs; bs = bs->next)
      bs->tf->SetSize(bs);
   for (bs = b->node->firstob; bs; bs = bs->next) {
      bs->Position(bs);
      b->Adapt(b, bs);
   }
   if (l->browsepending) {
      if (l->browsepending < l->lc->sti) {
         l->lc->sti = l->browsepending;
      } else if (l->browsepending >= l->lc->sti + l->lc->rows) {
         b = NewBrowseLFocus(l->browsepending, l->lc, l);
         if (b)
            b->tcfun->SetFocus(b);
      } else { /* No need to browse, the index is among the visible rows */
         b = GetRowObject(l, l->browsepending - l->lc->sti);
         if (b) {
            SetSubFocusOfRow(b);
         }
      }
   }
}

/* This unlink function should never be invoked, but must be there for safety
   if the "Remove" is called with the single list-id instead of the entire
   listbox. */
static void ListRemove(t_object *b)
{
   (void)b;
}

static void ListDestroy(t_object *b)
{
   t_list *l = b->node->data;
   if (l->action_wrapper) {
      l->action_wrapper->l = NULL;
      l->action_wrapper->n = 0;
      l->action_wrapper = NULL;
   }
   if (l->root) {
      DestroyTree(l->root);
      l->root = NULL;
   }
   xtended_node_type_functions.Free(b);
}

/* Used for the "rows node" */
static void SetSize(t_object *b)
{
   t_node *rowsnode;
   t_list *l;

   rowsnode = b->node;
   l = rowsnode->data;
   b->x1 = b->y1 = 0;
   b->x2 = l->orgwidth - 1 + l->lc->outer_node->ob->rex;
   b->y2 = l->orgheight - 1 + l->lc->outer_node->ob->rey;
   l->height = l->orgheight + l->lc->outer_node->ob->rey;
   SetBrowserSize(l->lc->br, l->height-2, l->height+2);
}

/* Sets the offset for all 3 containers in the list hierachy */
static void ListContainerSetSizeOffset(t_object *b, int w, int h)
{
   t_list *l, *p;
   t_listchain *lc;

   l = GetList(b->id);
   if (l) {
      lc = l->lc;
      DefaultSetSizeOffset(lc->outer_node->ob, w, h);
      /* The frame is always the first object under the lc */
      DefaultSetSizeOffset(lc->outer_node->firstob, w, h);
      for (p = lc->l; p; p = p->next)
         DefaultSetSizeOffset(p->rowsnode->ob, w, h);
      if (l->height != l->orgheight) {
         lc->pos = lc->sti*lc->rh;
         NotifyBrowser(lc->br, lc->rh, lc->n*lc->rh);
      }
   }
}

static t_object *GetMainNode(t_object *b)
{
   return b->parent->ob;
}

static void Dummy(void *data nouse)
{
}

static int MakeListStretchable(t_object *b, void (*Notify) (void *), void *data, int options)
{
   int id;
   t_listchain *lc;

   lc = b->node->data;
   id = MakeNodeStretchable(b, Notify, data, options);
   SetStretchMin(id, 30, lc->rh + 9);
   return id;
}

static t_list *MkList(void *applist, int *n, int width, int events, int (*CreateRowText) (void *, char *), void (*Action) (int, void *), t_listchain *lc)
{
   t_list *l;
   t_node *framenode, *rowsnode;
   int i, yspace, addcom;
   static t_typefun rntf, fntf;
   static t_tcfun ntc, conttc;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      ntc = default_master_tc_functions;
      ntc.MoveFocusToNextSubObject = ListMoveFocusToNextSubObject;
      ntc.UnSetFocus = ListUnFocus;

      fntf = default_node_type_functions;
      fntf.MakeNodeRects = StubOb;
      fntf.GetMainNode = GetMainNode;

      rntf = xtended_node_type_functions;
      rntf.Remove = ListRemove;
      rntf.Free = ListDestroy;
      rntf.MakeNodeRects = StubOb;
      rntf.GetMainNode = GetMainNode;
      rntf.NewScreenMode = ListNewScreenMode;
      rntf.PrepareNewScreenMode = ListPrepareNewScreenMode;

      conttc = default_master_tc_functions;
      conttc.MoveFocusTo = ListContMoveFocusTo;
   }
   l = GetMem0(t_list, 1);
   if (n)
      l->n = n;
   else
      l->n = &l->memofn;
   l->applist = applist;
   l->Action = Action;
   l->nrcols = 1;
   l->font = CGUI_list_font;
   l->single_row_sel = cgui_list_no_multiple_row_selection;
   yspace = lc->rh - text_height(l->font);
   if (yspace)
      l->offs = yspace >> 1;
   l->row_delimiter_color = cgui_colors[CGUI_COLOR_LISTBOX_ROW_DELIMITER];
   l->column_delimiter_color = cgui_colors[CGUI_COLOR_LISTBOX_COLUMN_DELIMITER];
   l->CreateRowText = CreateRowText;
   l->orgwidth = width;
   /* N.1 this is the container that holds the container including the frame
      and the (eventual) browsebar. */
   lc->outer_node = MkNode(opwin->win->opnode, lc->x, lc->y, ADAPTIVE, StubOb);
   SetDistance(0, 0);
   lc->outer_node->data = lc;
   lc->outer_node->type = NODETYPE_LIST;
   /* N.1.1 this is the container that holds the header node, a frame and a
      slightly smaller container with the rows. */
   framenode = MkNode(opwin->win->opnode, DOWNLEFT | FILLSPACE | VERTICAL | HORIZONTAL, ADAPTIVE, DrawListFrame);
   l->framenode = framenode;
   l->handle_id = SetHotKey(framenode->ob->id, Dummy, NULL, 0, 0); /* Set an invisible object. */
   SetMeasure(1, 0, 1, 1, 0, 1);
   /* N.1.1.2 The container actually keeping the row-objects */
   rowsnode = MkNode(framenode, DOWNLEFT | FILLSPACE | VERTICAL | HORIZONTAL, ADAPTIVE, DrawRowsNode);
   SetDistance(0, 0);
   rowsnode->SetSize = SetSize;
   if ((lc->y & AUTOINDICATOR) == AUTOINDICATOR && lc->y >= 0) {
      addcom = (lc->x | lc->y) & ~AUTOINDICATOR;
      if (((addcom & FILLSPACE) && (addcom & VERTICAL)) || (addcom & EQUALHEIGHT)) {
         /* it may be extended, set number of rows later */
         lc->rows = 1;
         l->orgheight = lc->rh;
      } else
         l->orgheight = lc->rh * lc->rows;
      l->Pack = rowsnode->Pack;
      rowsnode->Pack = ListPack;
   } else
      l->orgheight = lc->rh * lc->rows;
   rowsnode->data = l;
   framenode->ob->tf = &fntf;
   framenode->data = l;
   l->rowsnode = rowsnode;
   l->events = events;
   JoinTabChain(rowsnode->ob->id); /* Note: must join before creating rows */
   for (i = 0; i < lc->rows; i++)
      MakeRow(l, i);
   CloseNode(); /* N.1.1.2 Closes rows node */
   CloseNode(); /* N.1.1 Closes frame node */
   lc->br = MkVerticalBrowser(RIGHT|ALIGNBOTTOM, BrCallBack, lc, &lc->pos);
   lc->pos = lc->sti*lc->rh;
   NotifyBrowser(lc->br, lc->rh, lc->n*lc->rh);
   CloseNode();  /* N.1 Closes the main node (the browser-node containing 3
                     objects has already been inserted - it may optionally
                     been taken out later if the chained list option is used,
                     in which case the main container is redundant but still
                     necessary for completeness) */
   lc->brob = lc->outer_node->last;
   /* Set default values */
   l->lc = lc;
   lc->outer_node->ob->tcfun = &conttc;
   rowsnode->ob->tf = &rntf;
   rowsnode->ob->tcfun = &ntc;
   return l;
}

extern void InitList(void)
{
   if (!cgui_started) {
      CGUI_list_vspace = 0;
      CGUI_list_fixfont = 0;
      cgui_list_fix_digits = 0;
   }
   CGUI_list_font = _cgui_prop_font;
}

static void ListTypeConfigure(t_typefun *tf)
{
   tf->Stretch = ListStretch;
   tf->SetSizeOffset = ListContainerSetSizeOffset;
   tf->MakeStretchable = MakeListStretchable;
}

/* Application interface: */

static void DrawListColHdr(t_object *b);
extern int AddList(int x, int y, void *data, int *n, int w, int events, int (*CreateRowText) (void *, char *), void (*Action) (int, void *), int norows)
{
   t_listchain *lc;
   int addcom;
   static t_typefun conttf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      conttf = xtended_node_type_functions;
      ListTypeConfigure(&conttf);
   }
   if (norows < 1 || opwin == NULL)
      return 0;
   lc = GetMem0(t_listchain, 1);
   lc->rh = CGUI_list_vspace + text_height(CGUI_list_font) + 1;
   lc->rows = norows;
   if (n)
      lc->n = *n;
   lc->x = x;
   lc->y = y;
   lc->l = lc->last = MkList(data, n, w, events, CreateRowText, Action, lc);
   if ((lc->y & AUTOINDICATOR) == AUTOINDICATOR && lc->y >= 0) {
      addcom = (lc->x | lc->y) & ~AUTOINDICATOR;
      if (addcom & FILLSPACE) {
         if (addcom & VERTICAL) {
            lc->fillh = 1;
            if (addcom & HORIZONTAL)
               lc->fillw = 1;
         } else {
            lc->fillw = 1;
         }
      }
   }
   lc->outer_node->ob->tf = &conttf;
   /* return reference to entire container */
   return lc->outer_node->ob->id;
}

extern int HookList(int id, void *data, int *n, int w, int events, int (*CreateRowText) (void *, char *), void (*Action) (int, void *))
{
   t_listchain *lc;
   t_list *l;
   static t_typefun conttf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      conttf = default_node_type_functions;
      ListTypeConfigure(&conttf);
      /* We must use different node-function set because there is only one
      `lc' object containing all common info, but the nodes are duplicated
      for each 'sub'-list. If we don't to this, there would be multiple free
      of the lc-object. */
   }
   l = GetList(id);
   if (l) {
      lc = l->lc;
      if (lc->brob) {              /* remove the browser from previous list */
         lc->brob->tf->Unlink(lc->brob);
         lc->brob->tf->Free(lc->brob);
      }
      lc->brob = NULL;
      if (lc->y & AUTOINDICATOR) {
         lc->x &= ~AUTOINDICATOR;
         lc->x |= RIGHT;
      }
      l = MkList(data, n, w, events, CreateRowText, Action, lc);
      /* Note: lc->outer_node has been updated here - it always points to the last
      (rightmost) listbox that contains the browser */
      l->next = lc->l;
      lc->l->prev = l;
      lc->l = l;
      if (*l->n >= lc->n) {
         lc->n = *l->n;
      }
      lc->outer_node->ob->tf = &conttf;
      /* return reference to the new container */
      return lc->outer_node->ob->id;
   }
   return 0;
}

extern int SetIndexedList(int id, void *(*ApplicationDataAtIndex) (void *data, int i))
{
   return SetList(id, ApplicationDataAtIndex, ITERATE_INDEXED);
}

extern int SetLinkedList(int id, void *(*IterateLinkedListOfApplication) (void *data, void *current))
{
   return SetList(id, IterateLinkedListOfApplication, ITERATE_LINKED);
}

/* The showfocus parameter indicates if the row in focus should be displayed or not. In lists where no selection is possible, it is maybe to prefere
   that nothing is displayed. should be called before the list is displayed the first time. */
extern int ListShowFocus(int listid, int showfocus)
{
   t_list *l;

   l = GetList(listid);
   if (l) {
      l->showfocus = showfocus;
      return 1;
   }
   return 0;
}

extern int SetInsertHandler(int id, void (*CallBack) (void *, int))
{
   t_list *l;

   l = GetList(id);
   if (l) {
      l->InsHandler = CallBack;
      return 1;
   }
   return 0;
}

extern int SetDeleteHandler(int id, void (*Handler) (int id, void *object))
{
   t_list *l;

   l = GetList(id);
   if (l) {
      l->DelHandler = Handler;
      return 1;
   }
   return 0;
}

/* Redraws an index in a list. If "ind" is out of scope: drop it all */
extern int RefreshListRow(int id, int i)
{
   t_list *l;
   t_object *b;
   t_listchain *lc;
   int rn;

   l = GetList(id);
   if (l) {
      lc = l->lc;
      rn = i - lc->sti;
      if (rn < 0)
         return 1;
      if (rn > *l->n)
         return 1;
      b = GetRowObject(l, rn);
      if (b)
         b->tf->Refresh(b);
      return 1;
   }
   return 0;
}

/* This function browses a list to the specified index. The row of that index
   will be focused and selected. All other rows will be unselected. If i is
   out of range the value will be adjusted to the closest legal. After the
   call i will be the last displayed index of data. */
extern int BrowseToF(int id, int i)
{
   t_list *l, *p;
   t_listchain *lc;
   t_object *b;

   l = GetList(id);
   if (l) {
      lc = l->lc;
      /* Adjust start index if out of range */
      if (l->treeview) {
         GenerateTreeAndSetFocus(l);
      } else if (l->user_data_type == ITERATE_LINKED) {
         RefreshLinkedUserData(l);
      } else {
         CalcMaxN(lc);
      }
      if (i >= *l->n)
         i = *l->n - 1;
      if (i < 0)
         i = 0;
      if (!l->done) {
         l->browsepending = i;
         return 1;
      }
      lc->sti = i;

      for (p = lc->l; p; p = p->next) {
         if (p != l) {
            b = p->rowsnode->firstob;
            if (b) {
               TellSubFocusOfRow(b);
            }
         }
      }

      l->rowsnode->firstob->tcfun->MoveFocusTo(l->rowsnode->firstob);
      DrawNewStart(lc);
      return 1;
   }
   return 0;
}

/* This function browses a list to the specified index. The row of that index
   will be focused and selected. All other rows will be unselected. If i is
   out of range the value will be adjusted to the closest legal. After the
   call i will be the last displayed index of data. */
extern int BrowseToL(int id, int i)
{
   t_list *l;
   t_listchain *lc;
   t_object *b;

   l = GetList(id);
   if (l) {
      lc = l->lc;
      /* Adjust start index if out of range */
      if (l->treeview) {
         GenerateTreeAndSetFocus(l);
      } else if (l->user_data_type == ITERATE_LINKED) {
         RefreshLinkedUserData(l);
      } else {
         CalcMaxN(lc);
      }
      if (i >= *l->n)
         i = *l->n - 1;
      if (i < 0)
         i = 0;
      if (!l->done) {           /* we can't set the start-index in this case
                                   since the number of rows may not be
                                   calcualted */
         l->browsepending = i;
         return 1;
      }
      b = NewBrowseLFocus(i, lc, l);
      /* Indicate focus */
      if (b == NULL)
         return 0;

      b->tcfun->MoveFocusTo(b);

      /* Update list-page */
      DrawNewStart(lc);
      return 1;
   }
   return 0;
}

/* This function browses a listbox to the specified index. The row of that
   index will be focused and selected. All other rows will be unselected. If
   i is prior to the first index currently displayed, i will be the first
   displayed index after the call. If i is below the last displayed index, i
   will be the last displayed index after the call. If i is out of range the
   value will be adjusted to the closest legal. If i is already displyed no
   browsing is needed but the row will be focused if not before. */
extern int BrowseTo(int id, int i, int uncond)
{
   t_list *l;
   t_listchain *lc;
   t_object *sf, *prevsf, *prevnf;
   t_window *w;
   int rn;

   l = GetList(id);
   if (l) {
      lc = l->lc;
      if (l->treeview) {
         GenerateTreeAndSetFocus(l);
      } else if (l->user_data_type == ITERATE_LINKED) {
         RefreshLinkedUserData(l);
      } else {
         CalcMaxN(lc);
      }
      if (i >= *l->n)
         i = *l->n - 1;
      if (i < 0)
         i = 0;
      if (!l->done) {
         l->browsepending = i;
         return 1;
      }
      if (i < lc->sti) {
         BrowseToF(id, i);
      } else if (i >= lc->sti + lc->rows) {
         BrowseToL(id, i);
      } else {
         /* No need to browse, the index is in view. Maybe on another row */
         rn = i - l->lc->sti;
         sf = GetRowObject(l, rn);
         w = l->rowsnode->win;
         GetTabChainFocus(w, &prevnf, &prevsf);
         if (prevsf != sf) {
            sf->tcfun->MoveFocusTo(sf);
         }
         if (uncond) {
            DrawNewStart(lc);
         }
      }
      return 1;
   }
   return 0;
}

extern int SetListGrippable(int listid, int (*AppGrip) (void *srcobj,
                  int reason, void *srclist, int i), int flags, int buttons)
{
   t_list *l;
   t_object *b;

   l = GetList(listid);
   if (l) {
      for (b = l->rowsnode->last; b; b = b->prev)
         SetObjectGrippable(b->id, RowGrip, flags, buttons, b);
      l->Grip = AppGrip;
      l->gripflags = flags;
      l->gripbuttons = buttons;
      return 1;
   }
   return 0;
}

extern int SetListDroppable(int listid, int (*AppDrop) (void *destobj,
      int reason, void *srcobj, void *destlist, int i, int flags), int flags)
{
   t_object *b;
   t_list *l;

   l = GetList(listid);
   if (l) {
      for (b = l->rowsnode->last; b; b = b->prev)
         SetObjectDroppable(b->id, RowDrop, flags, b);
      l->Drop = AppDrop;
      l->dropflags = flags;
      return 1;
   }
   return 0;
}

extern int SetListDoubleClick(int listid, void (*AppDouble) (int, void *, int),
                                                            int buttons)
{
   t_list *l;
   t_object *b;

   l = GetList(listid);
   if (l) {
      for (b = l->rowsnode->last; b; b = b->prev)
         SetObjectDouble(b->id, RowDouble, b, buttons);
      l->Double = AppDouble;
      l->doublebuttons = buttons;
      return 1;
   }
   return 0;
}

extern void **GetMarkedRows(int listid, int *n)
{
   t_list *l;
   t_object *b;
   int i;
   void **cbdata = NULL;

   *n = 0;
   l = GetList(listid);
   if (l) {
      for (b = l->rowsnode->firstob, i = *n = 0; b; b = b->next, i++) {
         if (b->hasfocus && (i < *l->n)) {
            cbdata = ResizeMem(void*, cbdata, *n + 1);
            cbdata[(*n)++] = GetApplicationData(i + l->lc->sti, l, b);
         }
      }
      if (l->action_wrapper) {
         l->action_wrapper->l = NULL;
         l->action_wrapper->n = 0;
         l->action_wrapper = 0;
      }
   }
   return cbdata;
}

extern void **GetQueuedSelections(int id, int *n)
{
   return GetMarkedRows(id, n);
}

extern int GetListIndex(int id)
{
   t_object *b = GetObject(id);
   t_object *lb;
   t_list *l;
   if (b) {
      lb = b->tf->GetMainNode(b);
      if (lb) {
         l = GetList(lb->id);
         if (l) {
            return GetRowIndex(b);
         }
      }
   }
   return -1;
}

extern int NotifyFocusMove(int id, void (*CallBack) (int id, void *rowobject))
{
   t_list *l;

   l = GetList(id);
   if (l) {
      l->NotifyMove = CallBack;
      return 1;
   }
   return 0;
}

typedef struct t_listcolhdr {
   int *w;
   char **label;
   void *appdata;
   void (*AppCallBack) (void *, int id, int i);
   int i;
   t_object *b;
   t_list *l;
} t_listcolhdr;

extern void SetColumnHeaderSize(t_object *b)
{
   t_listcolhdr *hdr;
   hdr = b->appdata;
   b->x1 = b->y1 = 0;
   b->x2 = *hdr->w - 1;
   b->y2 = text_height(b->font) + BUTTONBORDER * 2;
}

static void ClickLabel(void *data)
{
   t_listcolhdr *hdr = data;

   if (hdr->AppCallBack)
      hdr->AppCallBack(hdr->appdata, hdr->b->id, hdr->i);
   hdr->l->rowsnode->ob->tcfun->MoveFocusTo(hdr->l->rowsnode->ob);
}

static void DrawListColHdr(t_object *b)
{
   t_listcolhdr *hdr;
   hdr = b->appdata;
   if (b->state == MS_FIRST_DOWN) {
      ButtonInFrame(b->parent->bmp, b->x1, b->y1, b->x2, b->y2);
      FillText(b, 1);
      ButtonTextL(1, 1, b, *hdr->label);
   } else {
      ButtonFrame(b->parent->bmp, b->x1, b->y1, b->x2, b->y2, 0);
      FillText(b, 0);
      ButtonTextL(0, 0, b, *hdr->label);
   }
}

static void NotifyStretching(void *data)
{
   t_object *b = data;
   t_listcolhdr *hdr;

   hdr = b->appdata;
   *hdr->w = b->x2 - b->x1 + 1 + b->rex;
   if (*hdr->w < 4)
      *hdr->w = 4;
   b->rex = 0;
   AdjustRowsWidths(hdr->l);
}

static void ColumnStretch(t_object *b, int dx, int dy)
{
   t_node *nd;
   t_object *p;
   t_list *l;
   int newwidth, maxwidth, curwidth;
   t_listcolhdr *hdr;

   hdr = b->appdata;
   l = hdr->l;
   if (l->resize_columns_in_fix_width_listbox) {
      nd = b->parent;
      newwidth = 0;
      for (p=nd->firstob; p; p=p->next) {
         newwidth += (p->x2 - p->x1) + 1 + p->rex;
      }
      newwidth += dx;
      if (dx > 0) {
         maxwidth = hdr->l->orgwidth;
         if (newwidth > maxwidth) {
            dx -= newwidth - maxwidth;
            if (dx < 0) {
               dx = 0;
            }
         }
      }
   }
   b->rex += dx;
   b->rey += dy;
   NotifyStretching(b);
}

static t_object *CreateColumnLabel(int x, int y, int *w, char **label, void (*cb) (void *, int id, int i), void *appdata, int i,
                                   t_node *header_node, t_list *l)
{
   t_object *b;
   static t_typefun tf;
   static int virgin = 1;
   t_listcolhdr *hdr;

   if (virgin) {
      virgin = 0;
      tf = default_type_functions;
      tf.Draw = DrawListColHdr;
      tf.Free = XtendedFree;
      tf.SetSize = SetColumnHeaderSize;
      tf.Stretch = ColumnStretch;
   }
   b = CreateObject(x, y, header_node);

   hdr = GetMem(t_listcolhdr, 1);
   hdr->w = w;
   if (*w < 4)
      *w = 4;
   hdr->label = label;
   hdr->appdata = appdata;
   hdr->AppCallBack = cb;
   hdr->i = i;
   hdr->b = b;
   hdr->l = l;
   b->tf = &tf;
   b->appdata = hdr;
   b->Action = ClickLabel;
   return b;
}

extern int SetListColumns(int listid, int (*CreateColumnTextOfRow) (void *, char *, int), int *widths, int n, int opt, char **labels,
                          void (*cb) (void *, int id, int i), void *appdata)
{
   t_list *l;
   t_object *b;
   int i, prevpoint=0;
   static char *empty_string = "";

   l = GetList(listid);
   if (l) {
      l->CreateColumnTextOfRow = CreateColumnTextOfRow;
      l->column_widths = widths;
      l->nrcols = n;
      l->resize_columns = opt & (LIST_COLUMNS_ADJUSTABLE | LIST_COLUMNS_ADJUSTABLE_KEEP_BOX_WIDTH);
      l->resize_columns_in_fix_width_listbox = opt & LIST_COLUMNS_ADJUSTABLE_KEEP_BOX_WIDTH;
      l->col_delimiter = opt & LIST_COLUMNS_DELIMITER;
      if (l->resize_columns || labels) {
         if (l->header)
            CleanNode(l->header);
         else {
            prevpoint = InsertPoint(l->handle_id);
            l->header = MkNode(l->framenode, DOWNLEFT | FILLSPACE, ADAPTIVE, StubOb);
            SetDistance(0, 0);
            CloseNode();
         }
         for (i = 0; i < n; i++) {
            if (labels) {
               b = CreateColumnLabel(RIGHT, widths + i, labels + i, cb, appdata, i, l->header, l);
            } else {
               b = CreateColumnLabel(RIGHT, widths + i, &empty_string, cb, appdata, i, l->header, l);
            }
            if (!cb)
               b->click = 0;
            else
               b->click = LEFT_MOUSE | RIGHT_MOUSE;
            if (l->resize_columns)
               MakeStretchable(b->id, NotifyStretching, b, NO_VERTICAL);
         }
         if (prevpoint)
            InsertPoint(prevpoint);
         DisplayNode(l->header);
         return l->header->ob->id;
      }
      return 1;
   }
   return 0;
}

extern int ListTreeView(int listid, int level_width, int (*IsLeaf)(void*), int options)
{
   t_list *l;

   l = GetList(listid);
   if (l) {
      l->level_width = level_width;
      l->IsLeaf = IsLeaf;
      l->tree_view_options = options;
      l->treeview = 1;
      l->n = &l->memofn;
      return 1;
   }
   return 0;
}

extern int ListTreeSetNodesExpandedState(int listid, int (*IsExpandedCallBack)(void *data))
{
   t_list *l;

   l = GetList(listid);
   if (l && l->treeview) {
      GenerateTree(l);
      l->IsExpandedCallBack = IsExpandedCallBack;
      ListTreeSetNodesExpandedState2(l->root, l);
      return 1;
   }
   return 0;
}

extern int ListTreeSetNodeExpandedState(int listid, int new_expanded_state, void *data)
{
   t_list *l;
   t_object *b;

   l = GetList(listid);
   if (l && l->treeview) {
      for (b = l->rowsnode->firstob; b; b = b->next) {
         if (GetTreeNode(b) && GetAppDataOfTreeNode(b) == data) {
            SetExpanded(b, new_expanded_state);
            return 1;
         }
      }
   }
   return 0;
}

extern int InstallBelowListEndCallBack(int listid, void (*CallBack)(int id, void *data), void *data)
{
   t_list *l;

   l = GetList(listid);
   if (l) {
      l->BelowListEndCallBack = CallBack;
      l->below_list_end_data = data;
      return 1;
   }
   return 0;
}

extern int CguiListBoxSetToolTip(int listid, int mode, int options)
{
   t_list *l;
   t_object *b;

   (void)options;
   l = GetList(listid);
   if (l) {
      l->show_tool_tip = mode;
      if (l->show_tool_tip) {
         Refresh(listid);
      } else {
         /* Some of the rows may have been assigned a tooltip, so we must clear it.
            Further more one of the row may have a tooltip visible, so we must for
            safety hide it for all of the rows. */
         for (b = l->rowsnode->firstob; b; b = b->next) {
            HideToolTip(b);
            ClearToolTip(b);
         }
      }
      return 1;
   }
   return 0;
}

extern void CguiListBoxRowSetBar(int color, double percentage)
{
   CGUI_list_row_bar_diagram_color = color;
   CGUI_list_row_bar_diagram_percentage = percentage;
   CGUI_list_row_bar_diagram_used = 1;
}

extern int CguiListBoxRowGetClickedColumn(int rowid)
{
   t_object *b;
   int column_number = -1;

   b = GetObject(rowid);
   if (b) {
      column_number = GetClickedColumn(b);
   }
   return column_number;
}

extern void CguiListBoxSetColumnSelection(int listid, int state)
{
   t_list *l;

   l = GetList(listid);
   if (l) {
      l->column_selection = state;
   }
}

/*void _PrintListTree(int listid)
{
   t_list *l;
   l = GetList(listid);
   if (l) {
      PrintTree(0, l->root, l);
   }
}
*/

/* Obsolete: */
extern int AdjustableListWidth(int listid, void (*Notify) (void *), void *data)
{
   return MakeStretchable(listid, Notify, data, NO_VERTICAL);
}
extern int GetListWidth(int listid)
{
   int x=0, y;
   GetSizeOffset(listid, &x, &y);
   return x;
}

