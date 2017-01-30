/* Module SETPOS
   Contains functions that sets and adjusts the position of an object. */

#include <allegro.h>
#include "cgui.h"

#include "node.h"
#include "object.h"
#include "setpos.h"

extern void PositionTopLeft(t_object * b)
{
   t_node *nd;

   nd = b->parent;
   b->x1 = b->parent->leftx;
   b->x2 += b->x1;
   b->y1 = nd->topy;
   b->y2 += b->y1;
}

static void PositionDown(t_object * b)
{
   t_object *b0;
   t_node *nd;

   nd = b->parent;
   for (b0 = b->prev; b0 && b0->exclude_add; b0 = b0->prev);
   if (b0 == NULL)
      PositionTopLeft(b);
   else {
      b->x1 = b0->x1;
      b->x2 += b->x1;
      b->y1 = b0->y2 + nd->dy + 1;
      b->y2 += b->y1;
   }
}

static void PositionRight(t_object * b)
{
   t_object *b0;
   t_node *nd;

   nd = b->parent;
   for (b0 = b->prev; b0 && b0->exclude_add; b0 = b0->prev);
   if (b0 == NULL)
      PositionTopLeft(b);
   else {
      b->x1 = b0->x2 + nd->xdist + 1;
      b->x2 += b->x1;
      b->y1 = b0->y1;
      b->y2 += b->y1;
   }
}

static void PositionDownLeft(t_object * b)
{
   t_object *b0;
   t_node *nd;
   int y2 = 0;

   nd = b->parent;
   for (b0 = b->prev; b0 && b0->exclude_add; b0 = b0->prev);
   for (; b0; b0 = b0->prev)
      if (!b0->exclude_add)
         y2 = MAX(b0->y2, y2);
   if (y2 == 0)
      PositionTopLeft(b);
   else {
      b->x1 = b->parent->leftx;
      b->x2 += b->x1;
      b->y1 = y2 + nd->dy + 1;
      b->y2 += b->y1;
   }
}

static void PositionLeft(t_object * b)
{
   t_object *b0;
   t_node *nd;
   int x, y0, dx, lastfound;

   nd = b->parent;
   for (b0 = b->prev; b0 && b0->exclude_add; b0 = b0->prev);
   if (b0 == NULL)
      PositionTopLeft(b);
   else {
      if (b0->dire == DIR_DOWN) {
         b->y1 = b0->y1;
         b->y2 += b->y1;
         b->x1 = b0->x1 - 1 - nd->xdist - b->x2;
         b->x2 = b0->x1 - 1 - nd->xdist;
      } else {
         x = b0->x1;
         y0 = b->y1 = b0->y1;
         b->y2 += b->y1;
         lastfound = 0;
         /* move all objects on to the right */
         do {
            dx = b->x2 - b->x1;
            b->x1 = x;
            b->x2 = x + dx;
            x = b->x2 + 1 + nd->xdist;
            b0 = b;
            if (lastfound)
               break;
            b = b->prev;
            if (b == NULL)
               break;
            if (b->dire != DIR_LEFT) {
               if (b->y1 == y0)
                  lastfound = 1;
               else
                  break;
            }
         } while (1);
         nd->ob->Adapt(nd->ob, b0); /* adjust node-size according to the
                                       rightmost one */
      }
   }
}

static void LeftFrom(t_object * b)
{
   t_object *p, *b0;
   t_node *nd;
   int w;

   nd = b->parent;
   for (b0 = b->next, p = b; b0; b0 = b0->next) {
      if (!b0->exclude_add) {
         if (b0->y1 != b->y1 || b0->dire != DIR_LEFT)
            break;
         w = b0->x2 - b0->x1;
         b0->x1 = p->x1 - nd->xdist - w - 1;
         b0->x2 = p->x1 - nd->xdist - 1;
         p = b0;
      }
   }
}

static void AlignRight(t_object * b)
{
   int dx;
   t_node *nd;

   nd = b->parent;
   dx = b->x2 - b->x1;
   b->x2 = nd->ob->x2 - nd->ob->x1 - nd->rightx;
   b->x1 = b->x2 - dx;
   /* The LEFT-directed ones have already been processed so they have to be
      adjusted after the rightmost has received its final position */
   LeftFrom(b);
}

static void AlignBottom(t_object * b)
{
   int dy;
   t_node *nd;

   nd = b->parent;
   dy = b->y2 - b->y1;
   b->y2 = nd->ob->y2 - nd->ob->y1 - nd->boty;
   b->y1 = b->y2 - dy;
}

static void AlignBottomRight(t_object * b)
{
   AlignBottom(b);
   AlignRight(b);
}

static void AlignCentre(t_object * b)
{
   int needs, available, space;
   t_object *parentob;

   parentob = b->parent->ob;;
   needs = b->x2 - b->x1 + 1;
   available = parentob->x2 - parentob->x1 + 1;
   space = available - needs;
   b->x1 = space / 2;
   b->x2 = b->x1 + needs - 1;
}

static void AlignBottomCentre(t_object * b)
{
   AlignBottom(b);
   AlignCentre(b);
}

static void EqualWidth(t_object * b)
{
   t_object *b0, *first, *last;
   int w;

   for (b0 = b->prev; b0 && b0->exclude_add; b0 = b0->prev);
   if (b0 == NULL || b0->EqualSize != b->EqualSize || b0->y1 != b->y1) {
      first = b;
      /* find the widest object and count them */
      for (w = 0, last = first, b0 = first; b0; b0 = b0->next) {
         if (b0->exclude_add)
            continue;
         if (b0->EqualSize != b->EqualSize || b0->y1 != b->y1)
            break;
         w = MAX(w, b0->x2 - b0->x1);
         last = b0;
      }

      for (b0 = first; b0 != last->next; b0 = b0->next) {
         if (b0->exclude_add)
            continue;
         b0->x2 = b0->x1 + w;
      }
   }
}

static void EqualHeight(t_object * b)
{
   t_object *b0, *first, *last;
   int h;

   for (b0 = b->prev; b0 && b0->exclude_add; b0 = b0->prev);
   if (b0 == NULL || b0->EqualSize != b->EqualSize || b0->x1 != b->x1) {
      first = b;
      /* find the highest object and count them */
      for (h = 0, last = first, b0 = first; b0; b0 = b0->next) {
         if (b0->exclude_add)
            continue;
         if (b0->EqualSize != b->EqualSize || b0->x1 != b->x1)
            break;
         h = MAX(h, b0->y2 - b0->y1);
         last = b0;
      }

      for (b0 = first; b0 != last->next; b0 = b0->next) {
         if (b0->exclude_add)
            continue;
         b0->y2 = b0->y1 + h;
      }
   }
}

static void EqualBoth(t_object * b)
{
   EqualHeight(b);
   EqualWidth(b);
}

static void FillHorizontal(t_object * b)
{
   t_object *b0, *first, *last;
   int x1, n, space, each, rest, diff, used, leftx, rightx;
   t_node *nd;

   nd = b->parent;

   /* Find next visible object */
   for (b0 = b->next; b0 && b0->exclude_add; b0 = b0->next);

   /* Next real object (i.e. b0) is outside the sequence, so b is the last
      one in the sequence (we will calculate the entire sequence when
      recursing the last object) */
   if (b0 == NULL || (b0->seqdir & b->seqdir) != b->seqdir
       || b0->y1 != b->y1) {
      last = b;

      /* Find which of the objects after, that are leftmost and rightmost */
      leftx = -1;
      rightx = nd->ob->x2 - nd->ob->x1 + 1;
      for (; b0; b0 = b0->next) {
         if (b0->exclude_add)
            continue;
         if (b0->y1 <= b->y2 && b0->y2 >= b->y1) {
            if (b0->x2 < b->x1)
               leftx = MAX(leftx, b0->x2);
            else
               rightx = MIN(rightx, b0->x1);
         }
      }

      /* count number of objects to share the space, and sum the used space */
      used = b->x2 - b->x1 + 1;
      for (n = 1, first = last, b0 = last->prev; b0; b0 = b0->prev) {
         if (b0->exclude_add)
            continue;
         if ((b0->seqdir & b->seqdir) != b->seqdir || b0->y1 != b->y1)
            break;
         used += b0->x2 - b0->x1 + 1;
         n++;
         first = b0;
      }

      /* Find which of the objects after the sequence, that are leftmost and
         rightmost */
      for (; b0; b0 = b0->prev) {
         if (b0->exclude_add)
            continue;
         if (b0->y1 <= b->y2 && b0->y2 >= b->y1) {
            if (b0->x2 < b->x1)
               leftx = MAX(leftx, b0->x2);
            else
               rightx = MIN(rightx, b0->x1);
         }
      }

      space = rightx - leftx - 1 - nd->xdist * n - nd->rightx - used;
      x1 = leftx + nd->leftx + 1;
/*      x1 = leftx + 1;
      if (leftx == -1) {
         space -= nd->leftx;
         x1 += nd->leftx;
      } else {
         space -= nd->xdist;
         x1 += nd->xdist;
      }
      if (rightx > nd->ob->x2 - nd->ob->x1)
         space -= nd->rightx;
      else
         space -= nd->xdist;
*/
      if (space <= 0)
         return;
      if (n==0)
         return;
      each = space / n;
      rest = space % n;
      /* Distribute the space and set the coordinates accordingly */
      b0 = first;
      do {
         diff = b0->x2 - b0->x1;
         b0->x1 = x1;
         b0->x2 = x1 + diff + each;
         if (rest > 0) {        /* distribute the exess pixels evenly */
            b0->x2++;
            rest--;
         } else if (rest < 0) { /* distribute the exess pixels evenly */
            b0->x2--;
            rest++;
         }
         x1 = b0->x2 + 1 + nd->xdist;
         if (b0 == last)
            break;
         do
            b0 = b0->next;
         while (b0 && b0->exclude_add);
      } while (b0);
   }
}

static void FillVertical(t_object * b)
{
   t_object *b0, *first, *last;
   int y1, n, space, each, rest, diff, used, topy, bottomy;
   t_node *nd;

   nd = b->parent;

   /* Find first visible object */
   for (b0 = b->prev; b0 && b0->exclude_add; b0 = b0->prev);

   /* Previous real object (i.e. b0) is outside the sequence, so b is the
      first one in the sequence (we only need to calculate for this object,
      because we will now handle all the other objects in the sequence too) */
   if (b0 == NULL || (b0->seqdir & b->seqdir) != b->seqdir
       || b0->x1 != b->x1) {
      first = b;

      /* Find which of the objects before, that are at top and bottom */
      topy = -1;
      bottomy = nd->ob->y2 - nd->ob->y1 + 1;
      for (; b0; b0 = b0->prev) {
         if (b0->exclude_add)
            continue;
         if (b0->x1 <= b->x2 && b0->x2 >= b->x1) {
            if (b0->y2 < b->y1)
               topy = MAX(topy, b0->y2);
            else
               bottomy = MIN(bottomy, b0->y1);
         }
      }

      /* count number of objects to share the space, and sum the used space */
      used = b->y2 - b->y1 + 1;
      for (n = 1, last = first, b0 = first->next; b0; b0 = b0->next) {
         if (b0->exclude_add)
            continue;
         if ((b0->seqdir & b->seqdir) != b->seqdir || b0->x1 != b->x1)
            break;
         used += b0->y2 - b0->y1 + 1;
         n++;
         last = b0;
      }

      /* Find which of the objects after the sequence, that are at top and
         bottom */
      for (; b0; b0 = b0->next) {
         if (b0->exclude_add)
            continue;
         if (b0->x1 <= b->x2 && b0->x2 >= b->x1) {
            if (b0->y2 < b->y1)
               topy = MAX(topy, b0->y2);
            else
               bottomy = MIN(bottomy, b0->y1);
         }
      }

      space = bottomy - topy - 1 - nd->dy * (n - 1) - used;
      y1 = topy + 1;
      if (topy == -1) {
         space -= nd->topy;
         y1 += nd->topy;
      } else {
         space -= nd->dy;
         y1 += nd->dy;
      }
      if (bottomy > nd->ob->y2 - nd->ob->y1)
         space -= nd->boty;
      else
         space -= nd->dy;
      if (space <= 0)
         return;
      if (n==0)
         return;
      each = space / n;
      rest = space % n;

      /* Distribute the space and set the coordinates accordingly */
      b0 = first;
      do {
         diff = b0->y2 - b0->y1;
         b0->y1 = y1;
         b0->y2 = y1 + diff + each;
         if (rest > 0) {        /* distribute the excess pixels evenly */
            b0->y2++;
            rest--;
         } else if (rest < 0) { /* distribute the excess pixels evenly */
            b0->y2--;
            rest++;
         }
         y1 = b0->y2 + 1 + nd->dy;
         if (b0 == last)
            break;
         do
            b0 = b0->next;
         while (b0 && b0->exclude_add);
      } while (b0);
   }
}

static void FillBoth(t_object * b)
{
   FillVertical(b);
   FillHorizontal(b);
}

extern void SetPosition(t_object * b, int x, int y)
{
   t_node *nd;
   int addcom = 0;

   nd = b->parent;
   b->EqualSize = StubOb;
   b->Position = StubOb;
   b->Align = StubOb;
   b->Pack = StubOb;
   if (y >= 0) {
      addcom = x | y;
      x &= AUTOINDICATOR;
      y &= AUTOINDICATOR;
   }
   if (nd) {
      if (y == AUTOINDICATOR) {
         b->dire = x;
         switch (b->dire) {
         case DIR_TOPLEFT:
            b->Position = PositionTopLeft;
            break;
         case DIR_DOWNLEFT:
            b->Position = PositionDownLeft;
            break;
         case DIR_DOWN:
            b->Position = PositionDown;
            break;
         case DIR_LEFT:
            b->Position = PositionLeft;
            break;
         case DIR_RIGHT:
            b->Position = PositionRight;
            break;
         }
      } else {
         b->x1 = x;
         b->y1 = y;
      }
      if (addcom & ALIGNRIGHT) {
         if (addcom & ALIGNBOTTOM)
            b->Align = AlignBottomRight;
         else
            b->Align = AlignRight;
      } else if (addcom & ALIGNBOTTOM) {
         b->Align = AlignBottom;
      } else if (addcom & ALIGNCENTRE) {
         if (addcom & ALIGNBOTTOM)
            b->Align = AlignBottomCentre;
         else
            b->Align = AlignCentre;
      }
      if (addcom & FILLSPACE) {
         if (addcom & VERTICAL) {
            if (addcom & HORIZONTAL)
               b->Pack = FillBoth;
            else
               b->Pack = FillVertical;
         } else {
            b->Pack = FillHorizontal;
            addcom |= HORIZONTAL;
         }
      }
      if (addcom & EQUALHEIGHT) {
         if (addcom & EQUALWIDTH)
            b->EqualSize = EqualBoth;
         else
            b->EqualSize = EqualHeight;

      } else if (addcom & EQUALWIDTH)
         b->EqualSize = EqualWidth;
      b->seqdir = addcom & (HORIZONTAL | VERTICAL);
   }
}
