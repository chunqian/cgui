/* Module MSM.C

   Contains the default mouse state-machine, which will be used by all
   objects if not overridden. The sm consists of one table of function
   pointers and the functions. The table is not a state-table, i.e. it does
   not explicitly specify the state transitions. The state transition is
   preformed by the functions. Input for the transition functions are "over
   object", "button state" and (implicitly) the current state of the machine.
   The reason why it is made this way, is to try to make it more readable, and
   still achieve a good modularity.

   Specific objects may have thier own machines, however this one tries to
   fulfill the most obvious requirements.

   A large number of changes could be achieved by only creating a new table of
   state-handlers for the certain type of object. Some or all of the state
   handlers in the table could be other than the default ones. Do not change
   the SetDefaultStateMachine to make some cathegory of object act in a
   different way (if you do so it will change the behaviour for all objects),
   instead: create a new table, initialise to default values using
   SetDefaultStateMachine and plug in new handlers for required states and
   finally replace the default table-pointer after creating the object.

   For more extensive changes the table-size will of course be extended with
   new states.

   Changes in mouse-cursor selection should be done by call to NewCursor, it's
   no good idea with direct access to Allegros mouse, since the state-machine
   may like to change the mouse due to some mouse event. */

#include <allegro.h>

#include "cgui.h"

#include "object.h"
#include "node.h"
#include "tabchain.h"
#include "window.h"
#include "mouse.h"
#include "event.h"
#include "msm.h"
#include "id.h"

#include "statelog.h"

t_mousefocus msmf1;             /* Data concerning the object currently
                                   catched by the mouse (in some respect). */
t_mousefocus msmf2;             /* Data concerning a secondary object -
                                   currently this is only used for
                                   drag-and-drop operations, where two objects
                                   at will be involved at the same time */

static unsigned long used_drag_flags;
int slidetolerance = 8;        /* The number of spaces within which the mouse
                                  must be kept during the slidedelay to
                                  recognice it as an sliding operation (an
                                  additional requirement) */
int cgui_doubleclick_up = 250; /* The delay for the second click (the maximum
                                  delay that the mouse can be up between the
                                  two clicks */
int cgui_click_tolerance = 8;  /* For objects that have both grip/slide and
                                  click on the same button, this specifies the
                                  maximum dragging distance (in pixels) until
                                  "button up" to accept it as a click
                                  operation */
int cgui_resizing_offset = 3;  /* For objects having the "re-sizing" property
                                  this is used as max distance from the right
                                  and bottom edges of the object where the
                                  mouse can be used for resizing. The area to
                                  the left or above cathces other event types*/
static int eventid;            /* The id of the click event. It is needed to
                                  kill the event in case of successful click
                                  (the time did not expire */
static int click_waiting;      /* A flag indicating for the state-handler
                                  whether a preset click-delay has expired or
                                  is still waiting */


static void RequestCursor(t_mousedata *m, int cursor_key)
{
   m->requestcursor = cursor_key;
}

static t_node *SetTouchedWindow(t_mousefocus *msmf, t_object *b)
{
   if (b->node) {
      msmf->winnode = b->node->win->node;
   } else if (b->parent) {
      msmf->winnode = b->parent->win->node;
   } else {
      msmf->winnode = NULL;
   }
   return msmf->winnode;
}

static void UnLinkMouseEventAndObject(t_mousefocus *msmf, t_object *b)
{
   if (msmf && msmf->b != b) {
      UnLinkMouseEventAndObject(msmf, msmf->b);
      UnLinkMouseEventAndObject(b->md, b);
   } else {
      if (msmf) {
         msmf->b = NULL;
      }
      if (b) {
         b->md = NULL;
      }
   }
}

static void LinkMouseEventAndObject(t_mousefocus *msmf, t_object *b)
{
   UnLinkMouseEventAndObject(msmf, b);
   msmf->b = b;
   b->md = msmf;
}

extern void ResetMouseState(void)
{
   t_object *b;

   if (msmf2.isset) {
      b = msmf2.b;
      if (b) {                  /* should always exist */
         UnLinkMouseEventAndObject(&msmf2, b);
         b->state = MS_INIT;
         b->tf->Refresh(b);
         msmf2.winnode = _mouse_touched_win2 = NULL;
      }
      msmf2.isset = 0;
   }

   b = msmf1.b;
   if (msmf1.isset) {
      if (b) {                  /* should always exist */
         UnLinkMouseEventAndObject(&msmf1, b);
         b->state = MS_INIT;
         b->tf->Refresh(b);
         msmf1.winnode = _mouse_touched_win1 = NULL;
      }
      msmf1.isset = 0;
   }

   SelectCursor(CURS_DEFAULT);
}

extern int inProgress(void)
{
   return msmf1.isset;
}

extern void SetClick(t_object *b, int x, int y)
{
   msmf1.isset = 4;
   msmf1.Action = b->Action;
   msmf1.data = b->appdata;
   LinkMouseEventAndObject(&msmf1, b);
   _mouse_touched_win1 = SetTouchedWindow(&msmf1, b);
   msmf1.id = b->id;
   b->x = x;
   b->y = y;
}

extern void ReturnState1(void)
{
   msmf1.isset = 0;
   UnLinkMouseEventAndObject(&msmf1, msmf1.b);
   msmf1.winnode = _mouse_touched_win1 = NULL;
}

extern void ResetState(t_mousedata *m, t_object *b)
{
   b->state = MS_INIT;
   RequestCursor(m, CURS_DEFAULT);
}

extern int isOver(int x, int y, t_object *b)
{
   return   !b->inactive &&
            x >= b->x1 + b->dx1 &&
            x <= b->x2 - b->dx2 &&
            y >= b->y1 + b->dy1 &&
            y <= b->y2 - b->dy2;
}

extern void SetDropper(t_object *b, int x, int y)
{
   msmf2.isset = 1;
   msmf2.flags = b->dflags;
   msmf2.Drop = b->Drop;
   msmf2.data = b->dropdata;
   LinkMouseEventAndObject(&msmf2, b);
   _mouse_touched_win2 = SetTouchedWindow(&msmf2, b);
   msmf2.id = b->id;
   b->x = x;
   b->y = y;
}

extern void ReturnDropper(void)
{
   msmf2.isset = 0;
   msmf2.winnode = _mouse_touched_win2 = NULL;
   UnLinkMouseEventAndObject(&msmf2, msmf2.b);
}

static void SetGripped(t_object *b, int x, int y)
{
   msmf1.isset = 1;
   msmf1.flags = b->gflags;
   msmf1.Grip = b->Grip;
   msmf1.data = b->gripdata;
   LinkMouseEventAndObject(&msmf1, b);
   _mouse_touched_win1 = SetTouchedWindow(&msmf1, b);
   msmf1.id = b->id;
   b->x = x;
   b->y = y;
}

static void SetSlider(t_object *b, int x, int y)
{
   msmf1.isset = 2;
   msmf1.Slide = b->Slide;
   msmf1.data = b->slidedata;
   LinkMouseEventAndObject(&msmf1, b);
   _mouse_touched_win1 = SetTouchedWindow(&msmf1, b);
   msmf1.id = b->id;
   b->x = x;
   b->y = y;
}

static void SetReSizing(t_object *b, int x, int y)
{
   msmf1.isset = 8;
   msmf1.Slide = (b)->ReSize;
   msmf1.data = b->resizedata;
   LinkMouseEventAndObject(&msmf1, b);
   _mouse_touched_win1 = SetTouchedWindow(&msmf1, b);
   b->x = x;
   b->y = y;
}

static int isGripped(void)
{
   return msmf1.isset == 1;
}

static int isSliding(void)
{
   return  msmf1.isset == 2;
}

static int isClicking(void)
{
   return msmf1.isset == 4;
}

static int isReSizing(void)
{
   return msmf1.isset == 8;
}

static int isDropper(void)
{
   return msmf2.isset;
}

static int isGrippable(t_object *b, int mb)
{
   return b->grip & mb;
}

static int isClickable(t_object *b, int mb)
{
   return b->click & mb;
}

static int is2Clickable(t_object *b, int mb)
{
   return b->click2 & mb;
}

static int isSlidable(t_object *b, int mb)
{
   return b->slide & mb;
}

static int isSizeable(t_object *b, int mb)
{
   return b->resize & mb;

}

static int isSizeableH(t_object *b, int mb)
{
   return (b->resize & 0x80) && (mb & b->resize);
}

static int isSizeableV(t_object *b, int mb)
{
   return (b->resize & 0x40) && (mb & b->resize);
}

static int isDroppable(t_object *b)
{
   return isGripped() && (b->dflags & msmf1.flags);
}

/* Indicate for the state machine that it time has expired */
static void ClickChecker(void *data)
{
   t_mousedata *m =data;

   click_waiting = eventid = 0;
   LaunchMouseEvent(m);
}

static int MouseStateInit(int x, int y, int over, int mb, t_mousedata *m,
                          t_object *b)
{
   void *data;

   if (over) {
      if (mb) {                 /* a mousebutton is down */
         b->usedbutton = mb;
         if (isSlidable(b, mb) && !inProgress() && !isSizeable(b, mb)) {
            b->state = MS_SLIDING;
            SetSlider(b, x, y);
            msmf1.Slide(x, y, msmf1.data, msmf1.id, SL_STARTED);
         } else if (isGripped()) {
            if (isDroppable(b) && b != msmf1.b) {
               SetDropper(b, x, y);
               if (msmf2.Drop(msmf2.data, msmf2.id, msmf1.gripdata, DD_OVER_DROP,
                        msmf1.flags & msmf2.flags)) {
                  b->state = MS_DRAGGING_OVER;
                  RequestCursor(m, CURS_DRAGGABLE);
               } else {         /* The recipient rejects the drop request
                                   (temporarly) */
                  b->state = MS_DRAGGING_OVER_ILLEGAL;
                  RequestCursor(m, CURS_ILLEGAL);
               }
               MOUSE_STATE_TRANSITION_DONE(b, over, mb, m);
            } else {            /* We don't accept dropping on this */
               RequestCursor(m, CURS_ILLEGAL);
            }
         }
      } else {
         if (!inProgress()) {
            if (isSizeableH(b, 0xff) && (x > b->x2 - cgui_resizing_offset)) {
               SetReSizing(b, x, y);
               b->ReSize(x, y, b->resizedata, b->id, SL_OVER);
               b->state = MS_OVER;
               b->Over(b);
            } else if (isSizeableV(b, 0xff) && (y > b->y2 - cgui_resizing_offset)) {
               SetReSizing(b, x, y);
               b->ReSize(x, y, b->resizedata, b->id, SL_OVER);
               b->state = MS_OVER;
               b->Over(b);
            } else if (isGrippable(b, 0xff) && (data = b->Grip(b->gripdata, b->id, DD_OVER_GRIP)) != NULL) {
               SetGripped(b, x, y);
               msmf1.gripdata = data;
               b->state = MS_OVER;
               b->Over(b);
               if (!(isGrippable(b, 0xff) & isClickable(b, 0xff)))
                  /* Only change cursor if same button not is used also for click */
                  RequestCursor(m, CURS_DRAGGABLE);
            } else if (isSlidable(b, 0xff) && b->Slide(x, y, b->slidedata, b->id, SL_OVER)) {
               SetSlider(b, x, y);
               b->state = MS_OVER;
               b->Over(b);
            } else if (isClickable(b, 0xff) || is2Clickable(b, 0xff)) {
               SetClick(b, x, y);
               b->state = MS_OVER;
               b->Over(b);
            }                   /* else we don't want that object */
            MOUSE_STATE_TRANSITION_DONE(b, over, mb, m);
         }
      }
      return 1;
   } else {
      return 0;
   }
}

/* The state is "mouse cursor is over us" and object is possible recipient of
   at least some mouse-button actions. Mouse cursor has been over the object
   (and maybe still is, we will soon know...) Grip, slide and click must
   share the MS_OVER state since we want to allow for these differnt actions
   to be allowed with different buttons and no button has been pressed (until
   possibly now) In addition we allow for some of these actions to share the
   same mouse button, using only a timeslice to distinguish between them */
static int MouseStateOver(int x, int y, int over, int mb, t_mousedata *m, t_object *b)
{
   int ret = 1;

   if (over) {
      if (mb) {
         b->usedbutton = mb;
         m->prev = mb;
         b->x = x;
         b->action_by_mouse = 1;
         b->tcfun->MoveFocusTo(b);
         if ((isSizeableH(b, mb) && (x > b->x2 - cgui_resizing_offset)) ||
             (isSizeableV(b, mb) && (y > b->y2 - cgui_resizing_offset))) {
            b->ReSize(x, y, b->resizedata, b->id, SL_STARTED);
            b->state = MS_RESIZING;
         } else if (isGripped()) {
            if (!isGrippable(b, mb)) {
               /* We have to check again because we didn't know which mouse button the user was thinking about when
               he/she move the mouse over... If we made the wrong guess, then immediately go to the state as specified
               by clicks. */
               if (isClickable(b, mb) || is2Clickable(b, mb)) {
                  b->state = MS_FIRST_DOWN;
                  b->tf->Refresh(b);
               }
            } else if (isClickable(b, mb) || is2Clickable(b, mb)) {
               b->x = x;
               b->y = y;
               b->state = MS_POSSIBLE_GRIP;
            } else {
               SetGripped(b, x, y); /* Set pos again */
               b->Grip(b->gripdata, b->id, DD_GRIPPED);
               b->state = MS_GRIPPED;
            }
         } else if (isSlidable(b, mb)) {
            if (isClickable(b, mb) || is2Clickable(b, mb)) {
               b->state = MS_POSSIBLE_SLIDE;
            } else {
               SetSlider(b, x, y);
               b->state = MS_SLIDING;
               msmf1.Slide(x, y, msmf1.data, msmf1.id, SL_STARTED);
            }
         } else if (isClickable(b, mb) || is2Clickable(b, mb)) {
            b->state = MS_FIRST_DOWN;
            b->tf->Refresh(b);
         }
      } else {
         if (isReSizing()) {
            if (x <= b->x2 - cgui_resizing_offset
                && y <= b->y2 - cgui_resizing_offset)  {
               b->ReSize(x, y, b->resizedata, b->id, SL_OVER_END);
               ReturnState1();
               b->state = MS_INIT;
               RequestCursor(m, CURS_DEFAULT);
               /* Let init find the new state for us */
               MouseStateInit(x, y, over, mb, m, b);
            }
         } else if ((isSizeableH(b, 0xff) && (x > b->x2 - cgui_resizing_offset)) ||
                    (isSizeableV(b, 0xff) && (y > b->y2 - cgui_resizing_offset))) {
            SetReSizing(b, x, y);
            b->ReSize(x, y, b->resizedata, b->id, SL_OVER);
         }
      }
   } else {
      if (isSliding())
         msmf1.Slide(x, y, msmf1.data, msmf1.id, SL_OVER_END);
      else if (isReSizing())
         b->ReSize(x, y, b->resizedata, b->id, SL_OVER_END);
      b->state = MS_INIT;
      RequestCursor(m, CURS_DEFAULT);
      ReturnState1();
      MOUSE_STATE_TRANSITION_DONE(b, over, mb, m);
      ret = 0;
   }
   if (b->state != MS_OVER) /* state has changed */
      b->Over(b);
   return ret;
}

static int MouseStateReSizing(int x, int y, int over, int mb, t_mousedata *m, t_object *b)
{
   if (mb) {
      b->ReSize(x, y, b->resizedata, b->id, SL_PROGRESS);
   } else {
      b->ReSize(x, y, b->resizedata, b->id, SL_STOPPED);
      ReturnState1();
      b->state = MS_INIT;
      RequestCursor(m, CURS_DEFAULT);
      return MouseStateInit(x, y, over, mb, m, b);
   }
   return 1;
}

static void PerformClick(int x, int y, t_mousedata *m, t_object *b, void (*Action)(void*), void *data)
{
   SetClick(b, x, y);   /* resets position to accurate values */
   b->state = MS_INIT;
   ReturnState1();
   b->tf->Refresh(b);
   RequestCursor(m, CURS_DEFAULT);
   LaunchMouseEvent(m);
   b->tcfun->MoveFocusTo(b);
   b->x = x;
   b->action_by_mouse = 1;
   Action(data);
}

/* Current state is now "mouse over and button down", i.e. the first step of a click, or a double click or a grip */
static int MouseStateFirstDown(int x, int y, int over, int mb, t_mousedata *m, t_object *b)
{
   if (!inProgress()) {
      ResetState(m, b);
      return 0;
   }
   if (over) {
      if (!mb) {
         if (is2Clickable(b, b->usedbutton)) {
            eventid = _GenEventOfCgui(ClickChecker, m, cgui_doubleclick_up, b->id);
            click_waiting = 1;
            b->state = MS_SECOND_OVER;
         } else {
            PerformClick(x, y, m, b, b->Action, b->appdata);
         }
      }
   } else {
      if (mb) {
         b->state = MS_FIRST_DOWN_OFF;
         b->tf->Refresh(b);
      } else {
         b->state = MS_INIT;
         ReturnState1();
         b->tf->Refresh(b);
         MOUSE_STATE_TRANSITION_DONE(b, over, mb, m);
         return 0;
      }
   }
   MOUSE_STATE_TRANSITION_DONE(b, over, mb, m);
   return 1;
}

/* Current state is "object has previously been hit, but now cursor slided off while mb held down" */
static int MouseStateFirstDownOff(int x nouse, int y nouse, int over, int mb, t_mousedata *m, t_object *b)
{
   if (!inProgress()) {
      ResetState(m, b);
      return 0;
   }
   if (over) {
      if (mb) {
         b->state = MS_FIRST_DOWN;
         b->tf->Refresh(b);
      } else {
         b->state = MS_OVER;
      }
   } else {
      if (!mb) {
         b->state = MS_INIT;
         ReturnState1();
         b->tf->Refresh(b);
         MOUSE_STATE_TRANSITION_DONE(b, over, mb, m);
         return 0;
      }
   }
   MOUSE_STATE_TRANSITION_DONE(b, over, mb, m);
   return 1;
}

/* Current state is "one click encountered, button up, over" and we are double-clickable. If no button-down event in time,
   we must take the previous click as a single click if 1-clickable else drop it all */
static int MouseStateSecondOver(int x, int y, int over, int mb, t_mousedata *m, t_object *b)
{
   if (!inProgress()) {
      ResetState(m, b);
      return 0;
   }
   if (over) {
      if (click_waiting) {
         if (mb) {
            _KillEventOfCgui(eventid);
            click_waiting = eventid = 0;
            b->state = MS_SECOND_DOWN;
         }
      } else {   /* Timer has expired -> no double click */
         if (isClickable(b, b->usedbutton))
            PerformClick(x, y, m, b, b->Action, b->appdata);
         else
            b->state = MS_OVER;
      }
   } else {
      if (click_waiting) {
         _KillEventOfCgui(eventid);
         click_waiting = eventid = 0;
         b->state = MS_INIT;
         ReturnState1();
      } else if (isClickable(b, b->usedbutton))
         PerformClick(x, y, m, b, b->Action, b->appdata);
      return 0;
   }
   return 1;
}

/* Current state is now "mouse over and button down and there has been a
   single click", i.e. the second step of a double-click has started */
static int MouseStateSecondDown(int x, int y, int over, int mb, t_mousedata *m, t_object *b)
{
   if (!inProgress()) {
      ResetState(m, b);
      return 0;
   }
   if (over) {
      if (!mb) {
         SetClick(b, x, y);
         b->state = MS_INIT;
         ReturnState1();
         b->tf->Refresh(b);
         b->tcfun->MoveFocusTo(b);
         PerformClick(x, y, m, b, b->Action2, b->data2);
      }
   } else {
      if (mb) {
         b->state = MS_SECOND_DOWN_OFF;
         b->tf->Refresh(b);
      } else {
         b->state = MS_INIT;
         ReturnState1();
         b->tf->Refresh(b);
         return 0;
      }
   }
   return 1;
}

/* Current state is "a single click has been made, and a double has started with a button down-event,
   but i mouse did just slippered off" we are not grippable. */
static int MouseStateSecondDownOff(int x nouse, int y nouse, int over, int mb, t_mousedata *m, t_object *b)
{
   if (!inProgress()) {
      ResetState(m, b);
      return 0;
   }
   if (over) {
      if (mb) {
         b->state = MS_SECOND_DOWN;
         b->tf->Refresh(b);
      } else {
         b->state = MS_OVER;
         b->tf->Refresh(b);
      }
   } else {
      if (!mb) {
         b->state = MS_INIT;
         ReturnState1();
         b->tf->Refresh(b);
         return 0;
      }
   }
   return 1;
}


/* Current state is "mb down, we was previously hit, and we have the capacity to be both dragged and clicked on the same mouse boutton".
   So this can be either the beginning of a drag-drop operation or the first step of an click operation. To determine which we must wait for
   futher operator actions.*/
static int MouseStatePossibleGrip(int x, int y, int over, int mb, t_mousedata *m, t_object *b)
{
   if (!inProgress()) {
      ResetState(m, b);
      return 0;
   }
   if (mb) {
      if (ABS(b->x - x) > cgui_click_tolerance || ABS(b->y - y) > cgui_click_tolerance) {
         /* If the mouse have moved long enough since we entered this state, then we classify it as the initial grip of a
            dragging operation, and change the state accordingly. */
         RequestCursor(m, CURS_DRAGGABLE);
         SetGripped(b, x, y); /* Set new pos */
         b->Grip(b->gripdata, b->id, DD_GRIPPED);
         b->state = MS_GRIPPED;
      }
   } else { /* Button up - then it was a click or double click */
      if (over) {
         if (is2Clickable(b, b->usedbutton)) {
            eventid = _GenEventOfCgui(ClickChecker, m, cgui_doubleclick_up, b->id);
            click_waiting = 1;
            b->state = MS_SECOND_OVER;
         } else { /* Then it is only single clickable */
            PerformClick(x, y, m, b, b->Action, b->appdata);
         }
      } else { /* ?? slippered ? */
         msmf1.Grip(msmf1.gripdata, msmf1.id, DD_UNGRIPPED);
         b->state = MS_INIT;
         ReturnState1();
         RequestCursor(m, CURS_DEFAULT);
         LaunchMouseEvent(m);
      }
      return 1;
   }
   return 1;
}

/* Current state is "mb down, we was previously gripped and is now dragged around over anywhere" */
static int MouseStateGripped(int x nouse, int y nouse, int over nouse, int mb, t_mousedata *m, t_object *b)
{
   if (!inProgress()) {
      ResetState(m, b);
      return 0;
   }
   if (!mb) {            /* dropped somewhere */
      b->state = MS_INIT;
      SetGripped(b, x, y); /* Set pos again */
      ReturnState1();
      RequestCursor(m, CURS_DEFAULT);
      msmf1.Grip(msmf1.gripdata, msmf1.id, DD_UNGRIPPED);
      LaunchMouseEvent(m);
      return 1;
   }
   return 0;
}

/* Current state is "mb down, some other object has been gripped and now held
   over us" (we are possible recipient of the dragged object) */
static int MouseStateDraggingOver(int x nouse, int y nouse, int over,
                                  int mb, t_mousedata *m, t_object *b)
{
   if (!inProgress()) {
      ResetState(m, b);
      return 0;
   }
   if (mb) {
      m->prev = mb;
      if (over) {
         /* keep on */
      } else {
         b->state = MS_INIT;
         ReturnDropper();
         msmf2.Drop(msmf2.data, msmf2.id, msmf1.gripdata, DD_END_OVER_DROP,
                    msmf1.flags & msmf2.flags);
         RequestCursor(m, CURS_DEFAULT);
         MOUSE_STATE_TRANSITION_DONE(b, over, mb, m);
         return 0;
      }
   } else {                     /* dropped */
      if (over) {               /* compleated drag-drop */
         RequestCursor(m, CURS_DEFAULT);
         b->state = MS_INIT;
         msmf1.Grip(msmf1.gripdata, msmf1.id, DD_SUCCESS);
         msmf2.Drop(msmf2.data, msmf2.id, msmf1.gripdata, DD_SUCCESS,
                    msmf1.flags & msmf2.flags);
         ReturnDropper();
      } else {
         b->state = MS_INIT;    /* slippered ? */
         ReturnDropper();
      }
      LaunchMouseEvent(m);
   }
   MOUSE_STATE_TRANSITION_DONE(b, over, mb, m);
   return 1;
}

/* Current state is "mb down, some other object has been gripped and is now
   held over us but we do not accept it", so this state is for a dest-object
   that rejected request. The gripped object will be in the
   "MouseStateGripped"-state */
static int MouseStateDraggingOverIllegal(int x nouse, int y nouse, int over,
           int mb, t_mousedata * m, t_object *b)
{
   if (!inProgress()) {
      ResetState(m, b);
      return 0;
   }
   if (over) {
      if (mb) {
         /* default: keep holding state */
         m->prev = mb;
      } else {                  /* dropped on illegal object */
         b->state = MS_INIT;
         ReturnDropper();
         return 0;
      }
   } else {                     /* drag off */
      if (mb) {                 /* still gripped */
         m->prev = mb;
         if (!(isGrippable(b, mb) & isClickable(b, mb)))
            RequestCursor(m, CURS_DRAGGABLE);
      } else {                  /* dropped at the same time, maybe slippered?
                                 */
         RequestCursor(m, CURS_DEFAULT);
      }
      b->state = MS_INIT;
      ReturnDropper();
      return 0;
   }
   return 1;
}

/* Current state is "mb down, we was previously hit, and we have the capacity to be both slided and clicked on the same mouse boutton".
   So this can be either the beginning of a sliding operation or the first step of an click operation. To determine which we must wait for
   futher operator actions.*/
static int MouseStatePossibleSlide(int x, int y, int over, int mb, t_mousedata *m, t_object *b)
{
   if (!inProgress()) {
      ResetState(m, b);
      return 0;
   }
   if (mb) {
      m->prev = mb;
      if (ABS(b->x - x) > cgui_click_tolerance || ABS(b->y - y) > cgui_click_tolerance) {
         SetSlider(b, x, y); /* Set pos again */
         b->Slide(x, y, b->slidedata, b->id, SL_STARTED);
         b->state = MS_SLIDING;
      }
   } else { /* Button up - then it was a click or double click */
      if (over) {
         if (is2Clickable(b, b->usedbutton)) {
            eventid = _GenEventOfCgui(ClickChecker, m, cgui_doubleclick_up, b->id);
            click_waiting = 1;
            b->state = MS_SECOND_OVER;
         } else { /* Then it is only single clickable */
            PerformClick(x, y, m, b, b->Action, b->appdata);
         }
      } else { /* ?? slippered ? */
         b->state = MS_INIT;
         ReturnState1();
         RequestCursor(m, CURS_DEFAULT);
         b->Slide(x, y, b->slidedata, b->id, SL_STOPPED);
         LaunchMouseEvent(m);
      }
   }
   return 1;
}

/* Current state is "sliding is in progress, and mb is down" */
static int MouseStateSliding(int x, int y, int over nouse, int mb,
                             t_mousedata *m, t_object *b)
{
   if (!inProgress()) {
      ResetState(m, b);
      return 0;
   }
   if (mb) {
      m->prev = mb;
      SetSlider(b, x, y); /* Set pos again */
      msmf1.Slide(x, y, msmf1.data, msmf1.id, SL_PROGRESS);
   } else {                     /* mouse up -> sliding stopped */
      msmf1.Slide(x, y, msmf1.data, msmf1.id, SL_STOPPED);
      b->state = MS_INIT;
      ReturnState1();
      RequestCursor(m, CURS_DEFAULT);
      b->tf->Refresh(b);
      LaunchMouseEvent(m);
   }
   return 1;
}

/* Investigate the current mouse event for possiblel objects that cought a
   mouse event last time, and performs relevant actions.
   - hitnode is the main node of the window currently beeing under the mouse
   - return value: 0 = continue the searcing for an recipient. 1 = some object
     has caught this event, don't search any more.
   Drag/slide operation is in progress:
      Dropper has caught the mouse 2:nd focus.
         The mouse is still over its window.
           - Notify handler for that object about new mouse event.
           - Also notify handler for the gripped object (should be one)
         Else
           - Notify handler for that object about the new mouse event passing
             irrelvant coordinates.
           - I should give it all up, but for safety: reset state.
      Else (only gripped)
         The mouse is no longer over its window.
           - Do nothing
         Else
           - Notify handler for that object about new mouse event.
   Else (no drag/slide):
      There is an object that has caught the mouse focus
         The mouse is now over some other window.
           - Notify the object with irrelevant coordinates to make beleve that
           the mouse is no longer over it. Then clear it anyway for safety.
         The mouse is still over the same window (which must be the focuse
         one ??).
           - Notify the object with proper data.
      Else (no object prior knowledge)
         - Do nothing, returning 0 indicates
*/
extern int was_here;
static int CurMouseFocus(t_mousedata *m, struct t_node *hitnode)
{
   t_object *b;
   int stop;

   if (msmf1.isset && msmf1.winnode != hitnode && !isGripped() && !isSliding() && !isReSizing()) {
      b = msmf1.b;
      MOUSE_STATE_TRANSITION_DONE(b, 0, 0, m);
      if (b) {                  /* should always exist */
         b->me->StateHandlers[b->state] (-10000, -10000, 0, 0, m, b);
         b->state = MS_INIT;
         UnLinkMouseEventAndObject(&msmf1, b);
         b->tf->Refresh(b);
      }
      RequestCursor(m, CURS_DEFAULT);
      msmf1.isset = 0;
   }
   if (msmf2.isset && msmf2.winnode != hitnode) {
      b = msmf2.b;
      if (b) {                  /* should always exist */
         b->me->StateHandlers[b->state] (-10000, -10000, 0, 0, m, b);
         b->state = MS_INIT;
         UnLinkMouseEventAndObject(&msmf2, b);
         b->tf->Refresh(b);
      }
      if (!isGripped())
         RequestCursor(m, CURS_DEFAULT);
      msmf2.isset = 0;
   }
   if ((isGripped() || isSliding() || isReSizing()) || (msmf1.isset && hitnode==fwin)) {
      if (msmf2.isset) {
         stop = msmf2.b->me->MouseEv(m, msmf2.b);
         if (msmf1.isset) {
            msmf1.b->me->MouseEv(m, msmf1.b);
         }
         /* if focus2 needs further searching focus must not override that */
         return stop;
      } else if (msmf1.isset) {
         return msmf1.b->me->MouseEv(m, msmf1.b);
      }
   }
   return 0;
}

/* Default main handler for state transition */
extern int MouseEv(t_mousedata *m, t_object *b)
{
   int over, mb, x, y, take;    /* x, y is relative to the node */

   x = m->x - b->parent->win->node->ob->x1 - b->parent->wx;
   y = m->y - b->parent->win->node->ob->y1 - b->parent->wy;
   over = isOver(x, y, b);
   mb = m->mb;
   if ((unsigned) b->state < MS_MAX) {
      take = b->me->StateHandlers[b->state] (x, y, over, mb, m, b);
   } else {
      ResetState(m, b);
      take = 0;
   }
   MOUSE_STATE_TRANSITION_DONE(b, over, mb, m);
   return take;
}

extern void SetDefaultStateMachine(struct t_mevent *me)
{
   me->StateHandlers[MS_INIT] = MouseStateInit;
   me->StateHandlers[MS_OVER] = MouseStateOver;
   me->StateHandlers[MS_RESIZING] = MouseStateReSizing;
   me->StateHandlers[MS_FIRST_DOWN] = MouseStateFirstDown;
   me->StateHandlers[MS_FIRST_DOWN_OFF] = MouseStateFirstDownOff;
   me->StateHandlers[MS_SECOND_OVER] = MouseStateSecondOver;
   me->StateHandlers[MS_SECOND_DOWN] = MouseStateSecondDown;
   me->StateHandlers[MS_SECOND_DOWN_OFF] = MouseStateSecondDownOff;
   me->StateHandlers[MS_GRIPPED] = MouseStateGripped;
   me->StateHandlers[MS_POSSIBLE_GRIP] = MouseStatePossibleGrip;
   me->StateHandlers[MS_DRAGGING_OVER] = MouseStateDraggingOver;
   me->StateHandlers[MS_DRAGGING_OVER_ILLEGAL] = MouseStateDraggingOverIllegal;
   me->StateHandlers[MS_POSSIBLE_SLIDE] = MouseStatePossibleSlide;
   me->StateHandlers[MS_SLIDING] = MouseStateSliding;
   me->MouseEv = MouseEv;
   me->CurMouseFocus = CurMouseFocus;
}

/* Application interface */

extern void SetObjectGrippable(int id, void *(*Grip) (void *src, int id,
                            int reason), int flags, int buttons, void *data)
{
   t_object *b = GetObject(id);

   if (b) {
      b->Grip = Grip;
      b->grip = buttons;
      b->slide &= ~buttons;
      b->gflags = flags;
      b->gripdata = data;
   }
}

extern void SetObjectDroppable(int id, int (*Drop) (void *dest, int id,
       void *src, int reason, int flags), int flags, void *data)
{
   t_object *b = GetObject(id);

   if (b) {
      b->Drop = Drop;
      b->dflags = flags;
      b->dropdata = data;
   }
}

extern void SetObjectSlidable(int id, int (*Slide) (int x, int y, void *src,
            int id, int reason), int buttons, void *data)
{
   t_object *b = GetObject(id);

   if (b) {
      b->Slide = Slide;
      b->slidedata = data;
      b->slide = buttons;
      b->grip &= ~buttons;
   }
}

extern void SetObjectDouble(int id, void (*DoubleCall) (void *),
                                         void *data, int button)
{
   t_object *b = GetObject(id);

   if (b) {
      b->Action2 = DoubleCall;
      b->data2 = data;
      b->click2 = button;
   }
}

extern int RegisterDragFlag(int flag)
{
   int f;

   f = used_drag_flags & flag;
   used_drag_flags |= flag;
   return f;
}

extern int UnRegisterDragFlag(int flag)
{
   int f;

   f = used_drag_flags & ~flag;
   used_drag_flags &= ~flag;
   return f;
}

extern void InitMouseStateMachine(void)
{
   used_drag_flags = 0;
}

extern void StopMouseEvent(t_mousefocus *m)
{
   m->b = NULL;
   m->isset = 0;
   m->winnode = NULL;
   if (m == &msmf1)
      _mouse_touched_win1 = NULL;
   if (m == &msmf2)
      _mouse_touched_win2 = NULL;
   SelectCursor(CURS_DEFAULT);
}
