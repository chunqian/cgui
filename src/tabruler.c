/* Module TABRULER.C

Contains the functions for creating a "tab-tablist". A tab tablist allows for
the user to mark special positions. It may typically be placed at the top
of a list or editwindow and can then be used to mark tab-positions.
 */

#include <allegro.h>
#include "cgui.h"
#include "window.h"
#include "node.h"
#include "object.h"
#include "id.h"
#include "cgui/mem.h"

#include "tabruler.h"

typedef struct t_tabrul {
   int *tablist;
   int n;
   int options;
   int width, height;
   int dx;
   void (*CallBack)(void *data, int x, int *tabs, int n);
   void *data;
   t_object *b;
} t_tabrul;

static int *tabs, ntabs, tab_to_set;
static int fetching_in_progress = 0;

static void DrawTabRuler(t_object *b)
{
   t_tabrul *tr;
   BITMAP *bmp;
   int x,i;

   tr = b->appdata;
   bmp = b->parent->bmp;
   rectfill(bmp, b->x1, b->y1, b->x2, b->y2, cgui_colors[CGUI_COLOR_HEAVY_LIGHTENED_BORDER]);
   if (tr->options & TR_SHOWPOS)
      for (x=b->x1; x<b->x2; x+=tr->dx)
         vline(bmp, x, b->y1, b->y2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   for (i=0; i<tr->n; i++) {
      x = b->x1 + tr->tablist[i];
      hline(bmp, x, b->y1+1, x+tr->dx-1, cgui_colors[CGUI_COLOR_HEAVY_SHADOWED_BORDER]);
      vline(bmp, x, b->y1+2, b->y2, cgui_colors[CGUI_COLOR_HEAVY_SHADOWED_BORDER]);
   }
}

static int DoFlipTab(t_tabrul *tr, int x)
{
   int i, j;

   for (i=0; i<tr->n; i++)
      if (tr->tablist[i] >= x)
         break;
   j = i;
   if (i < tr->n && tr->tablist[i] == x) {
      for (;i<tr->n-1; i++)
         tr->tablist[i] = tr->tablist[i+1];
      tr->n--;
   } else {
      tr->n++;
      tr->tablist = ResizeMem(int, tr->tablist, tr->n);
      for (i=tr->n-1; i>j; i--)
         tr->tablist[i] = tr->tablist[i-1];
      tr->tablist[j] = x;
   }
   return j;
}

static void FlipTab(void *data)
{
   t_tabrul *tr=data;
   t_object *b;
   int x;

   b = tr->b;
   if (b->x < b->x2) {
      x = b->x - b->x1;
      x -= x % tr->dx;
      DoFlipTab(tr, x);
      Refresh(b->id);
      if (tr->CallBack)
         tr->CallBack(tr->data, x, tr->tablist, tr->n);
   }
}

static int FetchData(t_object *b)
{
   t_tabrul *tr;

   tr = b->appdata;
   switch (fetching_in_progress) {
   case 1:
      tabs = tr->tablist;
      ntabs = tr->n;
      fetching_in_progress = 0;
      break;
   case 2:
      tab_to_set -= tab_to_set % tr->dx;
      DoFlipTab(tr, tab_to_set);
      fetching_in_progress = 0;
      break;
   default:
      return 0;
   }
   return 1;
}

static void SetSize(t_object *b)
{
   t_tabrul *tr;

   tr = b->appdata;
   if (b->dire)
      b->x1 = b->y1 = 0;
   b->x2 = b->x1 + tr->width - 1 + b->rex;
   b->y2 = b->y1 + tr->height + b->rey;
}

extern int *GetRulerTabList(int id, int *n)
{
   t_object *b = GetObject(id);

   if (b) {
      fetching_in_progress = 1;
      b->tf->FetchData(b);
      if (fetching_in_progress) {
         fetching_in_progress = 0;
         return NULL;
      }
      *n = ntabs;
      return tabs;
   }
   return 0;
}

extern int FlipRulerTab(int id, int x)
{
   t_object *b = GetObject(id);

   if (b) {
      fetching_in_progress = 2;
      b->tf->FetchData(b);
      if (fetching_in_progress) {
         fetching_in_progress = 0;
         return 0;
      }
      tab_to_set = x;
      return 1;
   }
   return 0;
}

extern int AddTabRuler(int x, int y, int width, int height, int dx, int options,
           void (*CallBack)(void *data, int x, int *tabs, int n), void *data)
{
   t_object *b;
   static t_typefun tf;
   static int virgin=1;
   t_tabrul *tr;

   if (virgin) {
      virgin = 0;
      tf = default_type_functions;
      tf.Draw = DrawTabRuler;
      tf.SetSize = SetSize;
      tf.Free = XtendedFree;
      tf.FetchData = FetchData;
   }
   b = CreateObject(x, y, opwin->win->opnode);
   tr = GetMem0(t_tabrul, 1);
   tr->CallBack = CallBack;
   tr->data = data;
   tr->width = width;
   tr->options = options;
   tr->dx = dx;
   tr->height = height;
   tr->b = b;
   b->tf = &tf;
   b->appdata = tr;
   b->Action = FlipTab;
   return b->id;
}
