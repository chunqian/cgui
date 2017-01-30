/* Module RECTLIST Contains the function for splitting an retangular area
   into pieces of sub-rectangles.

   This function removes the rectangular area given by the coordinates
   (nx1,ny1) and (nx2, ny2) from the area described by the list of
   rectangles, rects (i.e. the intersecting part will be removed). If the new
   rectangle is not intersecting any of the rectangles in the list, there
   will be no change. If a rectangle in the list is intersected this will
   lead to either of: - the rectangle is splitted into two new rectngles (a
   corner of the new rectangle is on the surface) - the rectangle is removed
   (the new rectangle is covering it completely) - the rectangle gets smaller
   (a border line of the new rectangle, but no corner intersects this
   rectangle) This will be done for all rectangles in the list. The point is
   to make repeated calls to this function. -In first call the rectangle list
   must contain the original rectangle area. The rectangle to remove is
   described by the first 4 coordinates. -In successive calls the resulting
   rectangle list from the prior call shall be used, together with the next
   rectangle to be removed. The rectangle list is kept in an linear array. To
   avoid numerous memory re-allocations which would have slow down the
   process, the caller has to allocate memory with place for "init"
   rectangles. To make optimal performance: allocate memory enough for the
   largest number of rectangles. If caller doesnt know how much is necessary,
   it doesn't matter, the function will re-allocate memory for the rectngle
   list, to make place for all rectangles. However this requires that the
   caller uses the returned pointer to reassign its rect-pointer. It's
   possible, but no good convention to use stack-memory. Of course, it will
   work if you are _sure_ that there will be no nore ractangles than will fit
   into the array (if it doesen't your program will die). A proper call will
   look like: ... rects = GetMem(sizeof(t_coord)*INIT_RECT_SIZE); rects[0] =
   originarea; n = 1; for (i=0; i<ncover; i++) rects =
   _RectList(coverrects[i].x1, coverrects[i].y1, coverrects[i].x2,
   coverrects[i].y2, rects, &n, INIT_RECT_SIZE); ... */

#include "cgui.h"
#include "cgui/mem.h"

#include "rectlist.h"

extern t_coord *_RectList(int nx1, int ny1, int nx2, int ny2,
                          t_coord * rects, int *m, int init)
{
   t_coord *r, *q;
   int i, n, max, y1, y2, del, rx1, rx2, ry1, ry2;

   n = max = *m;
   for (i = 0; i < n; i++) {
      r = &rects[i];
      rx1 = r->x1;
      rx2 = r->x2;
      ry1 = r->y1;
      ry2 = r->y2;
      if (nx1 <= rx1 && nx2 >= rx2 && ny1 <= ry1 && ny2 >= ry2) {
         /* The window covers entirely the current rectangle */
         max--;
         rects[i] = rects[max];
         if (max < n) {
            n = max;
            i--;
         }
      } else {
         del = 0;
         y1 = ry1;
         y2 = ry2;
         if (ny1 > ry1 && ny1 <= ry2 && nx1 <= rx2 && nx2 >= rx1) {
            /* The window's upper border intersects the current rectangle */
            max++;
            if (max > init)
               rects = ResizeMem(t_coord, rects, max);
            q = &rects[max - 1];
            q->x1 = rx1;
            q->x2 = rx2;
            q->y1 = ry1;
            q->y2 = ny1 - 1;
            del = 1;
            y1 = ny1;
         }
         if (ny2 < ry2 && ny2 >= ry1 && nx1 <= rx2 && nx2 >= rx1) {
            /* The window's lower border intersects the current rectangle */
            max++;
            if (max > init)
               rects = ResizeMem(t_coord, rects, max);
            q = &rects[max - 1];
            q->x1 = rx1;
            q->x2 = rx2;
            q->y1 = ny2 + 1;
            q->y2 = ry2;
            del = 1;
            y2 = ny2;
         }
         if (nx1 > rx1 && nx1 <= rx2 && ny1 <= ry2 && ny2 >= ry1) {
            /* The window's left border intersects the current rectangle */
            max++;
            if (max > init)
               rects = ResizeMem(t_coord, rects, max);
            q = &rects[max - 1];
            q->x1 = rx1;
            q->x2 = nx1 - 1;
            q->y1 = y1;
            q->y2 = y2;
            del = 1;
         }
         if (nx2 < rx2 && nx2 >= rx1 && ny1 <= ry2 && ny2 >= ry1) {
            /* The window's right border intersects the current rectangle */
            max++;
            if (max > init)
               rects = ResizeMem(t_coord, rects, max);
            q = &rects[max - 1];
            q->x1 = nx2 + 1;
            q->x2 = rx2;
            q->y1 = y1;
            q->y2 = y2;
            del = 1;
         }
         if (del) {
            max--;
            rects[i] = rects[max];
         }
      }
   }
   *m = max;
   return rects;
}
