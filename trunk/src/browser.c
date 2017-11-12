/* Module BROWSER.C

   This module contains function for the entire browsing object, which
   consists of the scroll up, scroll down and the browse slider. So the
   browsing object is actually just a container.

   The browsing object will notify the browsed object that there has been
   some browsing done, as soon as it gets info about that form any of the
   scroll buttons or the browse slider.

   The browsing object may be notified by the browsed object to update its
   copies of data (data about the browsed object that is used for
   calculations). These data are just passed on to the sub objects.
*/

#include "cgui.h"
#include "cgui/mem.h"

#include "browser.h"
#include "browsbar.h"
#include "node.h"
#include "window.h"
#include "scroll.h"
#include "id.h"

#define BR_HORIZONTAL 0
#define BR_VERTICAL   1

/* Data defining a specific container encapsulating a browse bar and two scroll buttons. */
typedef struct t_browser {
   /* A pointer to the browse bar. */
   struct t_browsebar *bb;
   /* A pointer to the decrementing scroll button (located to the top or left). */
   struct t_scroll *decr;
   /* A pointer to the incrementing scroll button (located to the bottom or right). */
   struct t_scroll *incr;
   /* A pointer to the location of the current scroll position (vertical or horizontal).
   Initially this may be non-0 (i.e. not top/left). */
   int *pos;
   /* The number of pixels to move for each click on a scroll button. */
   int step;
   /* The length (vertical or horizontal) of the object (or area) to scroll. */
   int scrolled_length;
   /* The id of the browsebar object. */
   int bbid;
   /* A flag indicating if this is a vertical or horizontal browser. */
   int vertical;
   /* A pointer back to the general container data structure. */
   t_node *nd;
   /* A call-back function invoked when the user has interacted with the
   browser elements to give the opportunity to update the browsed object. */
   void (*CallBack) (void *data);
   /* A pointer to a data object associated with CallBack. */
   void *data;
   /* The length (vertical or horizontal) of this browser object. */
   int len;
   /* A flag telling if this browser is active. It is inactive when the browsed
   object is less than or equal to the view port. */
   int active;
} t_browser;

static int browsed_object_length, browsing_object_length;
static int scroll_step, scrolled_length;
static int fetching_in_progress = 0;

static int FetchData(struct t_object *b)
{
   int sc_length, sc_length2, br_length;
   t_browser *br;

   if (fetching_in_progress) {
      br = b->node->data;
      if (fetching_in_progress == 1) {
         sc_length = BROWSERWIDTH;
         br_length = browsing_object_length - sc_length *2;
         if (browsing_object_length > 0 && br_length < 5) {
            br_length = 5;
            sc_length2 = browsing_object_length - br_length;
            sc_length = sc_length2 / 2;
            br_length += sc_length2 % 2;
         }
         br->len = sc_length * 2 + br_length;
         b->y2 = b->y1 + br->len - 1;
//         b->Align(b);
         SetScrollSize(br->decr, sc_length);
         SetScrollSize(br->incr, sc_length);
         SetBrowseBarSize(br->bb, browsed_object_length, br_length);
         b->tf->SetSize(b);
         b->Position(b);
//         b->Pack(b);
      } else {
         br->scrolled_length = scrolled_length;
         br->step = scroll_step;
         br->active = NotifyBrowseBar(br->bb, scrolled_length);
         NotifyScroller(br->decr, br->active);
         NotifyScroller(br->incr, br->active);
      }
      fetching_in_progress = 0;
      return 1;
   }
   return 0;
}

static void SetSize(t_object *b)
{
   t_browser *br;

   if (b->dire)
      b->x1 = b->y1 = 0;
   br = b->node->data;
   if (br->vertical) {
      b->y2 = br->len-1;
      b->x2 = BROWSERWIDTH-1;
   } else {
      b->x2 = br->len-1;
      b->y2 = BROWSERWIDTH-1;
   }
}

static void Decrement(void *data)
{
   t_browser *br = data;

   if (*br->pos > 0) {
      *br->pos -= br->step;
      NotifyBrowseBar(br->bb, br->scrolled_length);
      br->CallBack(br->data);
      Refresh(br->bbid);
   }
}

static void Increment(void *data)
{
   t_browser *br = data;

   if (*br->pos + br->step < br->scrolled_length) {
      *br->pos += br->step;
      NotifyBrowseBar(br->bb, br->scrolled_length);
      br->CallBack(br->data);
      Refresh(br->bbid);
   }
}

static t_browser *MkBrowser(int x, int y, int bbx, int bby, int scx, int scy,
      void (*CallBack) (void *data), void *data, int *viewpos,
      struct t_scroll *(*CreateDecrementer)(int, int, void (*)(void *), void *),
      struct t_browsebar *(*CreateBB)(int, int, int *, int *, void (*)(void *), void *),
      struct t_scroll *(*CreateIncrementer)(int, int, void (*)(void *), void *))
{
   t_browser *br;
   t_node *nd;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      tf = xtended_node_type_functions;
      tf.FetchData = FetchData;
   }
   br = GetMem0(t_browser, 1);
   nd = MkNode(opwin->win->opnode, x, y, BROWSERWIDTH, BROWSERWIDTH, StubOb);
   nd->SetSize = SetSize;
   SetDistance(0, 0);
   nd->data = br;
   nd->ob->tf = &tf;
   br->decr = CreateDecrementer(TOPLEFT, Decrement, br);
   br->bb = CreateBB(bbx, bby, viewpos, &br->bbid, CallBack, data);
   br->incr = CreateIncrementer(scx, scy, Increment, br);
   br->nd = nd;
   br->CallBack = CallBack;
   br->data = data;
   br->pos = viewpos;
   br->len = BROWSERWIDTH;
   CloseNode();
   return br;
}

/* Application interface: */

/* Notifies the browser and the scroll-buttons that a state change may have
   occured. It needs to be called e.g. when the scrolled area has moved
   within the view port because of reasons not controlled by the browser or
   because the size of it has changed. This is needed if the browser shall
   be able to maintain the correct position of the handle. */
extern int NotifyBrowser(int id, int step, int scrolled_area_length)
{
   t_object *b;

   b = GetObject(id);
   if (b == NULL || b->node == NULL)
      return 0;
   fetching_in_progress = 2;
   scrolled_length = scrolled_area_length;
   scroll_step = step;
   b->tf->FetchData(b);
   if (fetching_in_progress) {
      fetching_in_progress = 0;
      return 0;
   }
   return 1;
}

extern int SetBrowserSize(int id, int obh, int bbh)
{
   t_object *b;

   b = GetObject(id);
   if (b == NULL || b->node == NULL)
      return 0;
   fetching_in_progress = 1;
   browsed_object_length = obh;
   browsing_object_length = bbh;
   b->tf->FetchData(b);
   if (fetching_in_progress) {
      fetching_in_progress = 0;
      return 0;
   }
   return 1;
}

/* Creates the complete browsing object, consisting of one slider for
   browsing and two buttons for scrolling.
*/
extern int MkVerticalBrowser(int x, int y,
      void (*CallBack) (void *data), void *data, int *viewpos)
{
   t_browser *br;
   br = MkBrowser(x, y, DOWNLEFT|FILLSPACE|VERTICAL, DOWNLEFT|ALIGNBOTTOM,
        CallBack, data, viewpos,CreateScrollUpButton, CreateVerticalBrowseBar,
        CreateScrollDownButton);
   br->vertical = 1;
   return br->nd->ob->id;
}

extern int MkHorizontalBrowser(int x, int y,
      void (*CallBack) (void *data), void *data, int *viewpos)
{
   return MkBrowser(x, y, RIGHT|FILLSPACE, RIGHT, CallBack, data, viewpos,
          CreateScrollLeftButton, CreateHorizontalBrowseBar,
          CreateScrollRightButton)->nd->ob->id;
}

extern int IsBrowserActive(int id)
{
   t_object *b;
   t_browser *br;

   b = GetObject(id);
   if (b) {
      br = b->node->data;
      return br->active;
   }
   return 0;
}
