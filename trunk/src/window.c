/* Module WINDOW.C
   Contains functions for handling of the window-object */

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "node.h"
#include "window.h"
#include "mouse.h"
#include "rectlist.h"
#include "hotkeyau.h"
#include "obdrag.h"
#include "obheader.h"
#include "tabchain.h"
#include "memint.h"
#include "graphini.h"
#include "cursor.h"
#include "msm.h"
#include "id.h"

#define INIT_RECT_SIZE 50

t_typefun default_window_type_functions;
t_tcfun default_window_tc_functions;

static int window_openeing_in_progress;

/* NB! This function draws directly onto its own bitmap, not on the parent's */
static void DrawDefaultWinFrame(t_object *b)
{
   int x = 0, y = 0, x2, y2;
   t_node *nd;
   int width, height;
   BITMAP *bmp;

   nd = b->node;
   bmp = nd->bmp;
   if (bmp == NULL)
      return;
   width = b->x2 - b->x1 + 1;
   height = b->y2 - b->y1 + 1;
   x2 = width - 1;
   y2 = height - 1;
   rectfill(bmp, x, y + 2, x + 1, y2 - 1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   rectfill(bmp, x2 - 1, y, x2, y2 - 2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   hline(bmp, x, y, x2 - 1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   hline(bmp, x, y + 1, x2 - 2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   hline(bmp, x, y2, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   hline(bmp, x + 1, y2 - 1, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
}

extern int WinIsFree(t_object *b)
{
   return b->node->win->hidden == 0;
}

/* Returns the topmost (where "top" refers to the viewing - this corresponds
   to a low position i the tree) descendant window of b0. Input is the object
   of an existing window. Returns its input if there were no child window
   found. */
static t_object *GetFirstDescendantWindow(t_object *b0)
{
   t_object *b;

   for (b = b0->node->last; b; b = b->prev)
      if (IsWindow(b))
         return GetFirstDescendantWindow(b);
   return b0;
}

/* As GetFirstDescendantWindow but bypasses any window that is hidden by
   modal child(ren) */
static t_object *GetFirstFreeDescendantWindow(t_object *b0)
{
   t_object *b, *bx;

   for (b = b0->node->last; b; b = b->prev) {
      if (IsWindow(b)) {
         bx = GetFirstFreeDescendantWindow(b);
         if (bx)
            return bx;
      }
   }
   if (b0->node->win->hidden)
      return NULL;
   return b0;
}

/* Returns the next (seen from top) object that is a window. Input is the
   object of an existing window. Returns NULL if no more windows were found
   in the tree. */
extern t_object *NextWin(t_object *b0)
{
   t_object *b;

   for (b = b0->prev; b; b = b->prev)
      if (IsWindow(b))
         return GetFirstDescendantWindow(b);
   if (b0->parent && b0->parent->ob->id)
      return b0->parent->ob;
   return NULL;
}

/* As NextWin but bypasses any window that is hidden by modal child(ren) */
extern t_object *NextFreeWin(t_object *b0)
{
   t_object *b, *bx;

   for (b = b0->prev; b; b = b->prev) {
      if (IsWindow(b)) {
         bx = GetFirstFreeDescendantWindow(b);
         if (bx)
            return bx;
      }
   }
   if (b0->parent) {
      if (b0->parent->win->hidden)
         return NextFreeWin(b0->parent->ob);
      return b0->parent->ob;
   }
   return NULL;
}

/* Returns the window of all existing that is wiewed as the top one. */
extern t_object *FirstWin(void)
{
   t_object *b;

   if (_win_root == NULL)
      return NULL;
   b = _win_root->node->last;
   if (b == NULL)
      return NULL;
   if (IsWindow(b))
      return GetFirstDescendantWindow(b);
   return NextWin(b);
}

/* As FirstWin but bypasses any window that is hidden by modal child(ren) */
extern t_object *FirstFreeWin(void)
{
   t_object *b;

   if (_win_root == NULL)
      return NULL;
   b = _win_root->node->last;
   if (b == NULL)
      return NULL;
   if (IsWindow(b))
      return GetFirstFreeDescendantWindow(b);
   return NextFreeWin(b);
}

/* This function will update a screen rectangle with the contents from the
   window-ob b. - If that rectangle is completely covered by window b, window
   b will be drawn with clipping set to that area. - If that rectangle is
   partially covered by window b, window b will be drawn with clipping set to
   the fraction of the rectangle that it intersects. The rest of the
   rectangle will be partitioned into sub- rectangles. Each of these will be
   recursively updated by RefreshScreenRect using the rest of the window
   stack. - If that rectangle is not at all intersected by window b,
   RefreshScreenRect will recursively update that rectangle by use of the
   rest of the window stack. - If there is no window left, but a rectangle is
   to be updated, the rectangle is filled with white (this is an error
   condition).
   The coordinates of the rectangle shall be given in screen coordinates */
static void RefreshScreenRect(t_object *b, int x1, int y1, int x2, int y2)
{
   int wx1, wx2, wy1, wy2, xx1, xx2, xy1, xy2;
   t_node *nd;

   if (b == NULL)
      return;
   if (b->node->bmp == NULL)
      RefreshScreenRect(NextWin(b), x1, y1, x2, y2);
   nd = b->node;
   wx1 = b->x1;                 /* The coordinates of the window ob are
                                   always specified */
   wx2 = b->x2;                 /* in screen coordinates */
   wy1 = b->y1;
   wy2 = b->y2;
   if (wy2 >= y1 && wy1 <= y2 && wx2 >= x1 && wx1 <= x2) {
      xx1 = x1;
      xx2 = x2;
      xy1 = y1;
      xy2 = y2;
      if (wy1 > y1) {
         RefreshScreenRect(NextWin(b), x1, y1, x2, wy1 - 1);
         xy1 = wy1;
      }
      if (wy2 < y2) {
         RefreshScreenRect(NextWin(b), x1, wy2 + 1, x2, y2);
         xy2 = wy2;
      }
      if (wx1 > x1) {
         RefreshScreenRect(NextWin(b), x1, xy1, wx1 - 1, xy2);
         xx1 = wx1;
      }
      if (wx2 < x2) {
         RefreshScreenRect(NextWin(b), wx2 + 1, xy1, x2, xy2);
         xx2 = wx2;
      }
      if (nd && nd->bmp) {
         blit(nd->bmp, cgui_bmp, xx1 - wx1, xy1 - wy1, xx1, xy1,
              xx2 - xx1 + 1, xy2 - xy1 + 1);
      } else {
         RefreshScreenRect(NextWin(b), xx1, xy1, xx2, xy2);
      }
   } else {       /* this window doesn't intersect the current rectangle */
      RefreshScreenRect(NextWin(b), x1, y1, x2, y2);
   }
}

/* Refreshes the specified screen area, starting with top-window The screen
   area shall be given in screen coordinates The refreshing only concerns
   updating the secondary screen buffer and the screen with pixel from the
   concerned windows' bitmaps - not redrawing. */
extern void RefreshScreen(int x1, int y1, int x2, int y2)
{
   RefreshScreenRect(FirstWin(), x1, y1, x2, y2);
   if (cgui_use_vsync)
      vsync();
   scare_mouse_area(MAX(0,x1), MAX(0,y1), x2 - x1 + 1, y2 - y1 + 1);
   blit(cgui_bmp, screen, x1, y1, x1, y1, x2 - x1 + 1, y2 - y1 + 1);
   unscare_mouse();
}

/* Performes the same as above but for a rectangle list. */
/* ###FIXME
static void RefreshScreenRL(t_coord *rects, int n)
{
   int i;

   for (i = 0; i < n; i++)
       RefreshScreenRect(FirstWin(), rects[i].x1, rects[i].y1, rects[i].x2,
                                     rects[i].y2);
   if (cgui_use_vsync)
      vsync();
   for (i = 0; i < n; i++)
       blit(cgui_bmp, screen, rects[i].x1, rects[i].y1,
                              rects[i].x1, rects[i].y1,
                              rects[i].x2 - rects[i].x1 + 1,
                              rects[i].y2 - rects[i].y1 + 1);
}
*/
static int WindowSetFocus(t_object *b)
{
   if (fwin && fwin!= b->node)
      fwin->ob->tcfun->UnSetFocus(fwin->ob);
   fwin = opwin = b->node;
   b->hasfocus = 1;
   if (b->node->win->header) {
      b->node->win->header->tcfun->SetFocus(b->node->win->header);
      b->node->win->header->tf->Refresh(b->node->win->header);
   }
   SetFocusHighlighted(b->node->win->tc);
   return 1;
}

static void WindowUnSetFocus(t_object *b)
{
   b->hasfocus = 0;
   if (b->node->win->header) {
      b->node->win->header->tcfun->UnSetFocus(b->node->win->header);
      b->node->win->header->tf->Refresh(b->node->win->header);
   }
   SetFocusDiscrete(b->node->win->tc);
}

static t_object *PromoteWin(t_object *b0)
{
  t_object *b, *tb, *tbnext;

  if (b0->parent == NULL)
     return NULL;
  if (b0->node->win->modal) {
     for (b = b0->parent->last; b; b = b->prev) {
        if (IsWindow(b)) {
           if (b->node->win->modal) {
              if (b0 != b) {
                 MoveObjectInTree(b0, b);
                 tb = b0;
              } else
                 tb = NULL;
              tbnext = PromoteWin(b0->parent->ob);
              if (tbnext)
                 return tbnext; /* There was needed a move higher up in tree */
              if (tb)
                 return tb; /* We needed to move, but none above */
              break; /* Return NULL: no changes from here and up in tree */
           }
        }
     }
  } else {
     for (b = b0->parent->last; b; b = b->prev) {
        if (IsWindow(b)) {
           if (b0 != b) {
              MoveObjectInTree(b0, b);
              tb = b0;
           } else
              tb = NULL;
           tbnext = PromoteWin(b0->parent->ob);
           if (tbnext)
              return tbnext;
           if (tb)
              return tb;
           break;
        }
     }
  }
  return NULL;
}

#define MAKE_NEW(xr, rects, n, m, x1, y1, x2, y2) \
               xr.x1 = x1, \
               xr.y1 = y1, \
               xr.x2 = x2, \
               xr.y2 = y2, \
               *rects = InsertRect(&xr, *rects, n, m)
/* The left argument will be updated using of data in the right argument */
#define JUSTIFY_LEFT_TO_RIGHT(x, y) x->x1 = y->x2 + 1
#define JUSTIFY_RIGHT_TO_LEFT(x, y) x->x2 = y->x1 - 1
#define JUSTIFY_BOTTOM_TO_TOP(x, y) x->y2 = y->y1 - 1
#define JUSTIFY_TOP_TO_BOTTOM(x, y) x->y1 = y->y2 + 1
/* Checks if the left argument intersects the right argument */
#define BOTTOM_INTERSECT(x, y) x->y2 >= y->y1 && x->y2 <= y->y2
#define TOP_INTERSECT(x, y) x->y1 >= y->y1 && x->y1 <= y->y2
#define LEFT_INTERSECT(x, y) x->x1 >= y->x1 && x->x1 <= y->x2
#define RIGHT_INTERSECT(x, y) x->x2 >= y->x1 && x->x2 <= y->x2
#define HORIZONTAL_CUTTING(x, y) x->x2 > y->x2 && x->x1 < y->x1
#define TOP_BELOW_BOTTOM(x, y) x->y1 > y->y2
static t_coord *InsertRect(t_coord *nr, t_coord *rects, int *n, int *m);

/* If the new rectangle `nr' overlaps with the current `cr', the two areas
   will be justified to not overlap. The overlapping may occaisonally be
   such that yet one rectangle needs to be created, which will then be done
   recursively.
   If there is a choice between joining an overlapping section in horizontal
   and vertical direction it will choose the horizontal to make each
   section as wide as possible.
   Returns 0 if nothing is remaining from the new rectangle, else non-0.
   When finished there may be some rectangle that is marked as 0-sized
   (all coordinates = -1), and these can be removed from the list when
   all recursion has been finished. */
static int JustifyRect(t_coord *nr, t_coord *cr, t_coord **rects, int *n, int *m)
{
   t_coord xr;
   int x1, y1, x2, y2;

   if (BOTTOM_INTERSECT(nr, cr)) {
      if (TOP_INTERSECT(nr, cr)) {
         if (LEFT_INTERSECT(nr, cr)) {
            if (RIGHT_INTERSECT(nr, cr)) {
               return 0; /* ready: new is entirely covered by the current */
            } else {
               JUSTIFY_LEFT_TO_RIGHT(nr, cr);
            }
         } else {
            if (RIGHT_INTERSECT(nr, cr)) {
               JUSTIFY_RIGHT_TO_LEFT(nr, cr);
            } else {
               if (HORIZONTAL_CUTTING(nr, cr)) {
                  /* Keep new and split the current into an upper and a lower
                     rectangle */
                  x1 = cr->x1;
                  y1 = nr->y2 + 1;
                  x2 = cr->x2;
                  y2 = cr->y2;
                  JUSTIFY_BOTTOM_TO_TOP(cr, nr);
                  MAKE_NEW(xr, rects, n, m, x1, y1, x2, y2);
               }
            }
         }
      } else {
         if (LEFT_INTERSECT(nr, cr)) {
            if (RIGHT_INTERSECT(nr, cr)) {
               JUSTIFY_BOTTOM_TO_TOP(nr, cr);
            } else {
               x1 = cr->x1;
               y1 = cr->y1;
               x2 = nr->x2;
               y2 = nr->y2;
               JUSTIFY_BOTTOM_TO_TOP(nr, cr);
               JUSTIFY_TOP_TO_BOTTOM(cr, nr);
               MAKE_NEW(xr, rects, n, m, x1, y1, x2, y2);
            }
         } else {
            if (RIGHT_INTERSECT(nr, cr)) {
               x1 = nr->x1;
               y1 = cr->y1;
               x2 = cr->x2;
               y2 = nr->y2;
               JUSTIFY_BOTTOM_TO_TOP(nr, cr);
               JUSTIFY_TOP_TO_BOTTOM(cr, nr);
               MAKE_NEW(xr, rects, n, m, x1, y1, x2, y2);
            } else {
               if (HORIZONTAL_CUTTING(nr, cr))
                  JUSTIFY_TOP_TO_BOTTOM(cr, nr);
            }
         }
      }
   } else {
      if (TOP_INTERSECT(nr, cr)) {
         if (LEFT_INTERSECT(nr, cr)) {
            if (RIGHT_INTERSECT(nr, cr)) {
               JUSTIFY_TOP_TO_BOTTOM(nr, cr);
            } else {
               x1 = cr->x1;
               y1 = nr->y1;
               x2 = nr->x2;
               y2 = cr->y2;
               JUSTIFY_TOP_TO_BOTTOM(nr, cr);
               JUSTIFY_BOTTOM_TO_TOP(cr, nr);
               MAKE_NEW(xr, rects, n, m, x1, y1, x2, y2);
            }
         } else {
            if (RIGHT_INTERSECT(nr, cr)) {
               x1 = nr->x1;
               y1 = nr->y1;
               x2 = cr->x2;
               y2 = cr->y2;
               JUSTIFY_TOP_TO_BOTTOM(nr, cr);
               JUSTIFY_BOTTOM_TO_TOP(cr, nr);
               MAKE_NEW(xr, rects, n, m, x1, y1, x2, y2);
            } else {
               if (HORIZONTAL_CUTTING(nr, cr))
                  JUSTIFY_BOTTOM_TO_TOP(cr, nr);
            }
         }
      } else {
         if (LEFT_INTERSECT(nr, cr)) {
            if (RIGHT_INTERSECT(nr, cr)) {
               if (TOP_BELOW_BOTTOM(nr, cr))
                  ;
               else {
                  x1 = nr->x1;
                  y1 = cr->y2 + 1;
                  x2 = nr->x2;
                  y2 = nr->y2;
                  JUSTIFY_BOTTOM_TO_TOP(nr, cr);
                  MAKE_NEW(xr, rects, n, m, x1, y1, x2, y2);
               }
            } else {
               JUSTIFY_LEFT_TO_RIGHT(cr, nr);
            }
         } else {
            if (RIGHT_INTERSECT(nr, cr)) {
               JUSTIFY_RIGHT_TO_LEFT(cr, nr);
            } else {
               /* No edge intersects: either outside or covering */
               if (TOP_BELOW_BOTTOM(nr, cr))
                  ;
               else {
                  /* The current one must be removed, but we can't move it
                     since that would interfere the loops and/or recursions we
                     are currently within. Easier is to mark it beeing
                     outside. */
                  cr->x1 = cr->y1 = cr->x2 = cr->y2 = -1;
               }
            }
         }
      }
   }
   return 1;
}

/* Inserts a new area `nr' into the rectangle list rects. First it will
   justify the size of it, checking against each of all the existing
   rectangles to avoid overlapping. If JustifyRect returns zero there exist
   a complete overlapping */
static t_coord *InsertRect(t_coord *nr, t_coord *rects, int *n, int *m)
{
   int i;

   for (i = *n - 1; i >= 0; i--) {
      if (JustifyRect(nr, rects + i, &rects, n, m) == 0)
         return rects;
   }
   if (*n >= *m) {
      *m += INIT_RECT_SIZE;
      rects = ResizeMem(t_coord, rects, *m);
   }
   rects[*n] = *nr;
   (*n)++;
   return rects;
}

/* InsertAreaWin will insert the rectangle of its area into the rectangle
   list, and it will also tell all its kids to insert theirs. The point is
   to get all windows (starting from b0) areas in a list. */
/* ###FIXME
static t_coord *InsertAreaWin(t_object *b0, t_coord *rects, int *n, int *m)
{
   t_object *b;
   t_coord nr;

   nr.x1 = MAX(b0->x1, 0);
   nr.y1 = MAX(b0->y1, 0);
   nr.x2 = MIN(b0->x2, SCREEN_W - 1);
   nr.y2 = MIN(b0->y2, SCREEN_H - 1);
   rects = InsertRect(&nr, rects, n, m);
   for (b = b0->node->last; b; b = b->prev)
      rects = b->tf->InsertArea(b, rects, n, m);
   return rects;
}

static int sortcoord(const void *e1, const void *e2)
{
   const t_coord *c1 = e1;
   const t_coord *c2 = e2;
   int x;

   if ((x = c1->y1 - c2->y2) == 0)
      return c1->x1 - c2->x2;
   else
      return x;
}
*/
/* If b is the object of a window, `CreateWinAreaRectList' will create a
   rectangle list with no overlapping rectangles, all together constituting
   the area on the screen covered by the window b and all its descendants.
   The list will be sorted.
   Even if there were no rectangles there will be memroy allocated.
   The caller is responsible for freeing the list. */
/* ###FIXME
static t_coord *CreateWinAreaRectList(t_object *b, int *n)
{
   t_coord *rects;
   int m, i;

   *n = 0;
   m = INIT_RECT_SIZE;
   rects = GetMem(t_coord, m);
   rects = b->tf->InsertArea(b, rects, n, &m);
   for (i = 0; i < *n; ) {
      if (rects[i].x2 < 0)
         rects[i] = rects[--(*n)];
      else
         i++;
   }
   if (*n > 1)
      qsort(rects, *n, sizeof(t_coord), sortcoord);
   return rects;
}
*/
/* `rb' is the object of the window that shall take the focus. The default
   is to just move the window to the top (the visible moving corresponds to
   moving it in the list). However there may be other windows of
   "higher priority" that shall be over it (such that are floating).
   Furthermore the moving may imply that other windows must move too. This
   is the case if the requested one is part of a child(ren)/parent(s) group,
   i.e. it is a child itself or it is a parent (or both). In such a case
   The internal order between the windows should be kept (except from
   possibly the requested one), because one don't suppose other windows to
   interleave such a group.
   The concept: The group is typically
   - a pair: one parent and one child
   - two-level: one parent with multiple children (a main window with e.g.
     a number of "tool-palettes")
   - multi-level single width: one parent, with one child that is a parent
     with one child that is ... (Sub-menus are always like this).
   however the widows may be related in any way that is possible to express
   with a tree-structure. */
static int WindowMoveFocusTo(t_object *rb)
{
   t_window *w;
   t_object *tb;

   if (rb->node == NULL)
      return 0;
   w = rb->node->win;
   if (w == NULL)
      return 0;
   /* Move the window and all its predecessors to be the topmost along
      its sisters. */
   tb = PromoteWin(rb);
   rb->tcfun->SetFocus(rb);
   if (tb) {
/* ###FIXME
      t_coord *rects;
      int n = 0;
      rects = CreateWinAreaRectList(tb, &n);
      RefreshScreenRL(rects, n);
      Release(rects);
*/
   }
   RefreshScreen(0, 0, SCREEN_W-1, SCREEN_H-1);
   return 1;
}

/* Closes the specified window and all its children. All the data will be
   freed and the screen updated. */
extern void _CguiCloseWindow(t_window *win)
{
   if (win)
      win->node->ob->tf->Remove(win->node->ob);
}

/* This function will place the window at coordinates so that the current
   mouse position will be at the center of the window - if this is possible
   keeping the window completely in the screen. If not, the centering will be
   fullfilled as far as possible. If the width or height of the window is
   more than the width or height of the screen the uppermost an leftmost
   edges will be visible. */
extern void CenterWindow(t_node *nd)
{
   int x1, y1, mx, my, mz;
   t_object *b;

   b = nd->ob;
   _CguiMousePos(&mx, &my, &mz);
   x1 = mx - (b->x2 >> 1);
   if (b->x2 + x1 >= SCREEN_W)
      x1 = SCREEN_W - b->x2;
   if (x1 < 0)
      x1 = 0;
   b->x1 = x1;
   b->x2 += x1;
   y1 = my - (b->y2 >> 1);
   if (b->y2 + y1 >= SCREEN_H)
      y1 = SCREEN_H - b->y2;
   if (y1 < 0)
      y1 = 0;
   b->y1 = y1;
   b->y2 += y1;
   if (IsWindow(b->parent->ob)) {
      if (b->parent->ob->x1 == b->x1 &&
          b->parent->ob->y1 == b->y1 &&
          b->parent->ob->x2 == b->x2 &&
          b->parent->ob->y2 == b->y2) {
          if (b->x1 == 0 && b->y1 == 0 &&
              b->x2 == SCREEN_W - 1 && b->y2 == SCREEN_H)
             ;
          else {
             b->x1 += 5;
             b->y1 += 5;
             b->x2 += 5;
             b->y2 += 5;
          }
      }
   }
   fwin->wx = fwin->wy = 0;
}

/* Window handler for state transition. The mouse is over object b0 and there
   are no obstacles in between, and b0 is the object of a window. */
static int MouseEvWin(t_mousedata *m, t_object *b0)
{
   t_object *b;

   if (b0->node->win->hidden)
      return 1;
   if (b0->node != fwin && m->mb)
      b0->tcfun->MoveFocusTo(b0);
   for (b = b0->node->last; b; b = b->prev) {
      if (!IsWindow(b)) {
         if (b->me->MouseEv(m, b))
            return 1;
      }
   }
   return 1;
}

static void RefreshWin(t_object *b)
{
   ReMakeWindow(b->node);
}

static void MakeWinRects(t_object *b)
{
   t_coord r;
   t_node *nd;
   int width, height;

   nd = b->node;
   width = b->x2 - b->x1 + 1;
   height = b->y2 - b->y1 + 1;
   r.x1 = DEFAULT_WINFRAME;
   r.y1 = DEFAULT_WINFRAME;
   r.x2 = width - DEFAULT_WINFRAME * 2 + 1;
   r.y2 = height - DEFAULT_WINFRAME * 2 + 1;
   MakeNodeRects(nd, &r);
}

static void FreeWin(t_object *b)
{
   t_window *w;

   if (b->node == _mouse_touched_win1)
      _mouse_touched_win1 = NULL;
   if (b->node == _mouse_touched_win2)
      _mouse_touched_win2 = NULL;
   if (b->node == fwin)
      fwin = NULL;
   if (b->node == opwin)
      opwin = NULL;
   w = b->node->win;
   w->Free(b); /* Run default free (node) to recurse the tree */
   _RemoveTabChain(w);
   Release(w);
}

static void RemoveWin(t_object *b)
{
/* ###FIXME
   t_coord *rects = NULL;
   int n = 0;
*/
   t_window *w, *parwin = NULL;
   static int recurse_level = 0;
   if (recurse_level == 0) {
      /* This is used to avoid multiple blits to screen when the window has
      one or more children (The effect of closing e.g. the entire sequence of
      multiple open submenus will otherwise be a small delay between
      each. Another effect would be that the same area might be updated
      several times because the child-window(s) have a high probability to
      be over the parent. */
/* ###FIXME
      rects = CreateWinAreaRectList(b, &n);
*/
   }
   recurse_level++;
   w = b->node->win;
   if (w) {
      if (b->parent)
         parwin = b->parent->win;
      if (w->modal && parwin)
         parwin->hidden--;
      b->tf->Unlink(b);
      b->tf->Free(b);
   }
   recurse_level--;
   if (w && recurse_level == 0) { /* All (possible) kids are gone now */
      if (fwin == NULL) { /* Try to find another */
         if (parwin && parwin->node->ob->id != ID_DESKTOP && !parwin->hidden)
            fwin = parwin->node;
         else {
            b = FirstFreeWin();
            if (b)
               fwin = b->node;
         }
         if (opwin==NULL)
            opwin = fwin;
         fwin->ob->tcfun->SetFocus(fwin->ob);
      }
/* ###FIXME                      Not enough tested
      RefreshScreenRL(rects, n);
      Release(rects);
*/
      RefreshScreen(0, 0, SCREEN_W-1, SCREEN_H-1);
   }
}

static void SetSize(t_object *b)
{
   if (!b->node->fixsize) {
      b->x2 = b->x1;
      b->y2 = b->y1;
   } else if (b->node->win->fillscreen) {
      b->x2 = b->x1 + SCREEN_W - 1;
      b->y2 = b->y1 + SCREEN_H - 1;
   } else {
      b->x2 = b->x1 + b->node->width - 1;
      b->y2 = b->y1 + b->node->height - 1;
   }
}

static void SetSizeCore(t_object *b)
{
   if (!b->node->fixsize) {
      b->x1 = b->y1 = 0;
      b->x2 = b->x1;
      b->y2 = b->y1;
   } else if (b->node->win->fillscreen) {
      b->x1 = b->y1 = 0;
      b->x2 = SCREEN_W - 1;
      b->y2 = SCREEN_H - 1;
   } else {
      b->x1 = b->y1 = 0;
      b->x2 = b->x1 + b->node->width - 1;
      b->y2 = b->y1 + b->node->height - 1;
   }
}

extern t_node *MakeWin(int width, int height, const char *label, int attr)
{
   t_node *nd, *par_node;
   t_window *w;
   t_object *b;
   static t_mevent me;
   static int virgin = 1;

   if (window_openeing_in_progress)
      return NULL;
   if (!cgui_started || _win_root == NULL)
      InitCgui(0, 0, 0);
   window_openeing_in_progress = 1;
   w = GetMem0(t_window, 1);
   w->modal = (attr & W_FLOATING) == 0;
   if (fwin == NULL)
      par_node = _win_root->node;
   else if (attr & W_SIBLING)
      par_node = fwin->ob->parent;
   else
      par_node = fwin;

   if (height == AUTOINDICATOR && width == 1) {
      nd = MkNode(par_node, 0, 0, SCREEN_W, SCREEN_H, DrawDefaultWinFrame);
      w->fillscreen = 1;
   } else {
      nd = MkNode(par_node, 0, 0, width, height, DrawDefaultWinFrame);
   }
   nd->type = NODETYPE_WINDOW;
   nd->ob->exclude_add = 1;
   opwin = fwin = nd;
   if (virgin) {
      virgin = 0;
      SetDefaultStateMachine(&me);
      me.MouseEv = MouseEvWin;
   }
   w->Free = nd->ob->tf->Free;
   nd->ob->tf = &default_window_type_functions;
   nd->ob->tcfun = &default_window_tc_functions;
   nd->SetSize = SetSize;
   nd->data = w;
   nd->win = w;
   w->node = nd;
   w->opnode = nd;
   w->autohk = 1;

   /* Determine where to place the window. If it is floating (non-modal) then
      it is already in right position. Otherwise it must (possibly ) be moved
      away to join the other modal ones.
         Parent
       / /|\ \ \
      / / | \ \ \
     M M  M  F F F
   */
   if (w->modal) {
      for (b = par_node->last; b; b = b->prev) {
         if (IsWindow(b) && b->node->win->modal) {
            if (b != nd->ob) {
               MoveObjectInTree(nd->ob, b);
               break;
            }
         }
      }
      if (par_node->win)
         par_node->win->hidden++;
   }

   if (!(attr & W_NOMOVE))
      w->dragger = DraggableWin();
      /* Make window possible to move with use of the mouse */

   w->options = attr;

   SetDistance(DEFAULT_WINFRAME, DEFAULT_WINFRAME);
   if (label)
      MkWinHeader(label);
   nd->ob->me = &me;
   nd = MkNode(nd, DOWNLEFT | FILLSPACE | VERTICAL | HORIZONTAL, ADAPTIVE,
              StubOb);
   nd->SetSize = SetSizeCore;
   w->opnode = nd;
   SetDistance(DEFAULT_WINFRAME, DEFAULT_WINFRAME);
   nd->type = NODETYPE_WINDOW;
   return nd;
}

static void DeInitWindows(void *data nouse)
{
   t_object *b;

   if (_win_root) {
      b = _win_root;
      fwin = opwin = NULL;
      _win_root = NULL;
      b->tf->Free(b);
   }
}

extern void InitWindows(void)
{
   t_node *nd;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      default_window_type_functions = default_node_type_functions;
      default_window_type_functions.MakeNodeRects = MakeWinRects;
      default_window_type_functions.Free = FreeWin;
      default_window_type_functions.Remove = RemoveWin;
      default_window_type_functions.Refresh = RefreshWin;
/*    default_window_type_functions.InsertArea = InsertAreaWin; ###FIXME */

      default_window_tc_functions = default_tabchain_functions;
      default_window_tc_functions.MoveFocusTo = WindowMoveFocusTo;
      default_window_tc_functions.SetFocus = WindowSetFocus;
      default_window_tc_functions.UnSetFocus = WindowUnSetFocus;
   }
   if (_win_root == NULL) {
      nd = MkNode(NULL, 0, 0, 0, 0, NULL);
      _win_root = nd->ob;
      HookCguiDeInit(DeInitWindows, NULL);
   }
}

/* Application interface: */

extern int CurrentWindow(void)
{
   return opwin->last->id;
}

extern void SetOperatingWindow(int id)
{
   t_node *nd;

   nd = GetNode(id);
   if (nd && nd->type == NODETYPE_WINDOW)
      opwin = nd->win->node;
}

extern void GetWinInfo(int id, int *x, int *y, int *width, int *height)
{
   t_node *nd;
   t_object *b;

   nd = GetNode(id);
   if (nd && nd->type == NODETYPE_WINDOW) {
      b = nd->ob;
      *x = b->x1;
      *y = b->y1;
      *width = b->x2 - b->x1 + 1;
      *height = b->y2 - b->y1 + 1;
   }
}

extern int _VisableWindow(void)
{
   if (fwin)
      return fwin->last->id;
   return -1;
}

/* Closes the window in focus. */
extern void _CguiCancel(void)
{
   if (fwin)
      _CguiCloseWindow(fwin->win);
}

extern void CloseWin(void *dummy nouse)
{
   _CguiCancel();
}

static void BrCallBack(void *data nouse)
{
}

static void AddWindowVerticalBrowser(t_node *nd)
{
   t_window *w;

   w = nd->win;
   if (w->verticalbr == 0) {
      MkVerticalBrowser(RIGHT|ALIGNBOTTOM, BrCallBack, nd, &w->pos);
   }
}

static void AddWindowHorizontalBrowser(t_node *nd)
{
   t_window *w;

   w = nd->win;
   if (w->horizontalbr == 0) {
   }
}

extern void AdjustWindowSize(t_node *nd)
{
   t_object *b;
   t_window *w;

   b = nd->ob;
   w = nd->win;
   switch (w->mode) {
   case W_SIZE_ADAPTIVE:
   case W_SIZE_MINIMIZED:
      break;
   case W_SIZE_MAXIMIZED:
      if (b->x2-b->x1+1 > SCREEN_W) {
         b->x2 = SCREEN_W + b->x1 - 1;
         AddWindowHorizontalBrowser(nd);
      }
      if (b->y2-b->y1+1 > SCREEN_H) {
         b->y2 = SCREEN_H + b->y1 - 1;
         AddWindowVerticalBrowser(nd);
      }
      break;
   case W_SIZE_NORMALIZED:
      if (b->x2-b->x1+1 > w->prefered_w) {
         b->x2 = w->prefered_w + b->x1 - 1;
         AddWindowHorizontalBrowser(nd);
      }
      if (b->y2-b->y1+1 > w->prefered_h) {
         b->y2 = w->prefered_h + b->y1 - 1;
         AddWindowVerticalBrowser(nd);
      }
      break;
   }
}

/* The initial "Mk..."-functions will only allocate memory, set up some
   initial pointers and set some initial data. To complete the building
   process of a window there are needed 7 passes of traversal (through the
   object tree), before it is possible to draw it. 1 - Set the initial size
   of the objects. This is an operation that is individual for each
   object-type. 2 - This pass executes the equal-size command, i.e. sequences
   of objects that shall have the same size will be justified. 3 - This pass
   executes the direction-commands, e.g. an object that shall be to the
   "RIGHT" of the previous, will be placed there. During the same pass all
   "ADAPTIVE"-sized nodes will be extended up to a size enough to display all
   its objects. 4 - During this pass the ALIGN-commands will be issued. Since
   the size of the nodes is not yet (certainly) determined, the
   align-operation is maybe not ready. 5 - During this pass the commands that
   share additional space will be run. The align (pass 4) must have been run
   first because otherwise all objects will not be certain to find their
   surrounding space. Eventually some objects may really extend during this
   pass, and some of them may be nodes that contains objects that needs to be
   aligned before Extended. Therefore such objects must re-align its own
   contents. 6 - Generation of bitmaps for all nodes. During this pass (which
   is only for the internal nodes of the tree) the rectangle-lists will also
   be generated (these defines the non-used areas between objects). 7 -
   Genrating hot-keys, i.e. labelled object without explicitly stated
   hot-keys will be assaigned these.

   This function completes the building of a window by running the passes
   1-6. */
extern void Complete(t_node *nd)
{
   t_object *b;
   t_window *w;

   b = nd->ob;
   w = nd->win;

   b->tf->CatchImage(b);
   b->tf->SetSize(b);
   w->completed = 1;
   b->EqualSize(b);
   b->Position(b);
   b->Pack(b);
   b->tf->DestroyBitmap(b);
   b->tf->MakeBitmap(b);
   GenerateAutoHotKeys(nd);
}

extern void WindowIsFinished(void)
{
   window_openeing_in_progress = 0;
}

extern int IsWindowCompleated(void)
{
   return window_openeing_in_progress == 0;
}

static void ReMakeWindowAtPosition(t_node *nd, int x, int y)
{
   t_window *w;
   t_object *b;
   int x2, y2;
   if (nd) {
      w = nd->data;
      b = nd->ob;
      /* Save to know size of area requireing update */
      x2 = b->x2;
      y2 = b->y2;
      /* Traverse the tree */
      b->tf->CatchImage(b);
      b->tf->SetSize(b);
      b->EqualSize(b);
      b->Position(b);
      b->Pack(b);
      b->tf->DestroyBitmap(b);
      b->tf->MakeBitmap(b);
      y2 = MAX(b->y2, y2);
      x2 = MAX(b->x2, x2);
      /* Set same position and possibly new size*/
      b->tf->Draw(b);
      nd->wx = nd->wy = 0;
      /* Update the visible parts of the window onto the screen */
      RefreshScreen(x, y, x2, y2);
      window_openeing_in_progress = 0;
   }
}

/* This finction will rebuild a window that has already been completed. The
   window will thereby resize all its contents, and if it is adaptive it
   may be larger. */
extern void ReMakeWindow(t_node *nd)
{
   if (nd)
      ReMakeWindowAtPosition(nd, nd->ob->x1, nd->ob->y1);
}

/* Displays a window, after it has been created, and all the stuff has been
   put into it. Before the actual diplaying there are lots of stuff that
   needs to be done first. The Blocking of mouse-event for 9 ticks is to
   avoid that the user makes unwished selections in the new window due to
   double-click (if the window- opening is a result of a list-item selection)
 */
extern void DisplayWin(void)
{
   t_window *w;
   t_object *b, *dummy, *sf;
   t_node *nd;
   t_coord sc, a;
   int x,y;

   if (fwin) {
      w = fwin->win;
      b = fwin->ob;
      if (w->completed) {
         GenerateAutoHotKeys(w->node);
         ReMakeWindow(fwin);
      } else {
         Complete(fwin);
         if (!w->fillscreen && !w->pos_preset) {
            if (w->options & W_CENTRE) {
               /* W_CENTRE include all flags */
               x = y = 0; /* Default is top left (i.e. 0,0) */
               if ((w->options & W_CENTRE_V) == W_CENTRE_V) {
                  y = (SCREEN_H - b->y2)/2;
               } else if ((w->options & W_BOTTOM) == W_BOTTOM) {
                  y = SCREEN_H - b->y2;
               }
               if ((w->options & W_CENTRE_H) == W_CENTRE_H) {
                  x = (SCREEN_W - b->x2)/2;
               } else if ((w->options & W_RIGHT) == W_RIGHT) {
                  x = SCREEN_W - b->x2;
               }
               fwin->win->pos_preset = 1;
               if (x < 0)
                  x = 0;
               if (y < 0)
                  y = 0;
               fwin->ob->x1 = x;
               fwin->ob->y1 = y;
               fwin->ob->x2 += x;
               fwin->ob->y2 += y;
            } else {
               CenterWindow(fwin);
            }
         }
         AdjustWindowSize(fwin);
         nd = b->parent;
         sc.x1 = b->x1;
         sc.y1 = b->y1;
         sc.x2 = b->x2;
         sc.y2 = b->y2;
         b->tf->Draw(b);
         a.x1 = 0;
         a.y1 = 0;
         a.x2 = b->x2 - b->x1;
         a.y2 = b->y2 - b->y1;
         BlitObject(&a, &sc, b->node);
         InitializeFocus(w->tc);
         GetTabChainFocus(w, &dummy, &sf);
         if (sf)
            sf->tf->Refresh(sf);
      }
      WindowIsFinished();
   }
}

static void RemakeWindows2(t_object *b, int orgw, int orgh)
{
   int x, y;

   if (b) {
      RemakeWindows2(NextWin(b), orgw, orgh);
      if (b->node->win->fillscreen)
         x = y = 0;
      else {
         x = b->x1;
         if (orgw)
            x = x * SCREEN_W / orgw;
         x = MIN(x, SCREEN_W - (b->x2 - b->x1));
         y = b->y1;
         if (orgh)
            y = y * SCREEN_H / orgh;
         y = MIN(y, SCREEN_H - (b->y2 - b->y1));
      }
      ReMakeWindowAtPosition(b->node, x, y);
   }
}

extern void RemakeWindows(int orgw, int orgh)
{
   RemakeWindows2(FirstWin(), orgw, orgh);
   RefreshScreen(0, 0, SCREEN_W, SCREEN_H);
}

extern void SetWindowPosition(int x, int y)
{
   if (opwin) {
      opwin->win->pos_preset = 1;
      if (x > SCREEN_W - 20)
         x = SCREEN_W - 20;
      if (y > SCREEN_H - 20)
         y = SCREEN_H - 20;
      opwin->ob->x1 = x;
      opwin->ob->y1 = y;
   }
}
