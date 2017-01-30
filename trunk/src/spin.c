/* SPIN.C
   This file contains the functions for creating spin-buttons */

#include "cgui.h"
#include "cgui/mem.h"

#include "object.h"
#include "tabchain.h"
#include "id.h"
#include "scroll.h"
#include "spin.h"

typedef struct t_spin {
   int delta1;
   int delta2;
   int minv;
   int maxv;
   int *var;
   t_object *b;
   struct t_scroll *up,*down;
} t_spin;

static void DestroySpin(void *data)
{
   t_spin *sp = data;

   Release(sp);
}

static void Increase(void *data)
{
   t_spin *sp = data;

   (*sp->var) += sp->delta1 + (sp->delta2 - sp->delta1) *
              (*sp->var - sp->minv) / (sp->maxv - sp->minv);
   if (*sp->var > sp->maxv)
      *sp->var = sp->maxv;
   sp->b->tcfun->MoveFocusTo(sp->b);
}

static void Decrease(void *data)
{
   t_spin *sp = data;

   (*sp->var) -= sp->delta1 + (sp->delta2 - sp->delta1) *
              (*sp->var - sp->minv) / (sp->maxv - sp->minv);
   if (*sp->var < sp->minv)
      *sp->var = sp->minv;
   sp->b->tcfun->MoveFocusTo(sp->b);
}

/* Application interface: */

extern int HookSpinButtons(int id, int *var, int delta1, int delta2,
                           int minv, int maxv)
{
   t_object *b = GetObject(id);
   t_spin *sp;
   int contid;

   if (b) {
      sp = GetMem0(t_spin, 1);
      sp->b = b;
      sp->delta1 = delta1;
      sp->delta2 = delta2;
      sp->var = var;
      sp->minv = minv;
      sp->maxv = maxv;
      contid = StartContainer(RIGHT, ADAPTIVE, "", 0);
      HookExit(contid, DestroySpin, sp);
      SetDistance(0, 0);
      sp->up = CreateScrollUpButton(RIGHT, Increase, sp);
      SetScrollSize(sp->up, _cgui_button1_height/2);
      sp->down = CreateScrollDownButton(DOWN, Decrease, sp);
      SetScrollSize(sp->down, _cgui_button1_height/2);
      EndContainer();
      Activate(contid);
      return 1;
   }
   return 0;
}
