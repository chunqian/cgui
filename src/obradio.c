/* Module OBRADIO.C
   Contains functions for creating radio-buttons. These buttons may be used
   when the user should be given the opportunity to select a certain value
   from a set of values in sequence. Each value is symbolised with a text.
   Each possible selection will generate a push-button style selector with
   the corresponding label. The list of selectors (push-buttons) may be
   placed either vertically or or horizontally. Radio-buttons may be used in
   the same cases as when drop-down-boxes are used. Radio-buttons are more
   useful because they proved a direct selection (need not drop-down first),
   but when there are many alternatives the dialogue will be unreadeble and
   the drop-down will be preferable. Supported formats for the
   application-items are FBYTE, FINT and FLONG */

#include <string.h>
#include <stdarg.h>

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "id.h"
#include "obbutton.h"
#include "obradio.h"
#include "mstates.h"
#include "tabchain.h"

typedef struct t_radiogroup {
   int *selvar;                 /* pointer to application variable */
   int n;                       /* number of buttons */
   t_object *focus;             /* the one currently selected */
   t_node *nd;
   int dir;
} t_radiogroup;

typedef struct t_radio {
   void (*Action) (void *);
   void *appdata;
   int i;                       /* index-number of button */
   t_object *b;
} t_radio;

static t_radiogroup *radiogroup;

static void ButtonXin(void *data)
{
   t_object *b = data;
   int x1, y1, x2, y2;

   x1 = b->x1;
   y1 = b->y1;
   x2 = b->x2;
   y2 = b->y2;
   ButtonText(2, b->x1, b->y1, b->x2, b->y2, b, b->parent->bmp);
   DisplayBitmap(b, 2);

   hline(b->parent->bmp, x1, y1, x2, cgui_colors[CGUI_COLOR_HEAVY_SHADOWED_BORDER]);
   vline(b->parent->bmp, x1, y1 + 1, y2, cgui_colors[CGUI_COLOR_HEAVY_SHADOWED_BORDER]);

   hline(b->parent->bmp, x1 + 1, y1 + 1, x2 - 1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   vline(b->parent->bmp, x1 + 1, y1 + 1, y2 - 1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);

   hline(b->parent->bmp, x1 + 2, y1 + 2, x2 - 1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   vline(b->parent->bmp, x1 + 2, y1 + 3, y2 - 1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);

   hline(b->parent->bmp, x1 + 1, y2, x2, cgui_colors[CGUI_COLOR_HEAVY_LIGHTENED_BORDER]);
   vline(b->parent->bmp, x2, y1 + 1, y2 - 1, cgui_colors[CGUI_COLOR_HEAVY_LIGHTENED_BORDER]);
}

static void RadioDraw(t_object * b)
{
   t_radio *r = b->appdata;
   t_radiogroup *rg;

   if (b->parent->bmp == NULL)
      return;
   rg = b->parent->data;
   if (b->state == MS_FIRST_DOWN) {
      FillText(b, 1);
      ButtonXin(b);
      DisplayBitmap(b, 1);
   } else {
      if (*rg->selvar == r->i) {
         rg->focus = b;
         ButtonInFrame(b->parent->bmp, b->x1, b->y1, b->x2, b->y2);
         FillText(b, 1);
         ButtonText(1, b->x1, b->y1, b->x2, b->y2, b, b->parent->bmp);
         DisplayBitmap(b, 1);
      } else {
         ButtonFrame(b->parent->bmp, b->x1, b->y1, b->x2, b->y2, b->hasfocus);
         FillText(b, 0);
         ButtonText(0, b->x1, b->y1, b->x2, b->y2, b, b->parent->bmp);
         DisplayBitmap(b, 0);
      }
   }
}

static void RadioState(void *data)
{
   t_radio *r = data;
   t_radiogroup *rg;
   t_object *b, *prevfocus;
   int newstate;

   b = r->b;
   rg = b->parent->data;
   prevfocus = rg->focus;
   rg->focus = b;
   newstate = *rg->selvar != r->i;
   *rg->selvar = r->i;
   b->tf->Refresh(b);
   if (prevfocus)
      prevfocus->tf->Refresh(prevfocus);
   if (newstate && r->Action)
      DelayedCallBack(r->Action, r->appdata);
}

static int RadioSetFocus(t_object *sf)
{
   t_radiogroup *rg;
   t_radio *r;

   rg = sf->parent->data;
   r = sf->appdata;
   if (*rg->selvar != r->i)
      RadioState(r);
   sf->hasfocus = 1;
   return 1;
}

static void UnSetFocus(t_object *b)
{
   b->hasfocus = 0;
}

static int AddRadioHandler(t_object * b, void (*Handler) (void *data),
                           void *appdata)
{
   t_radio *r;

   r = b->appdata;
   r->appdata = appdata;
   r->Action = Handler;
   return 1;
}

/* Application interface: */

extern int AddRadioButton(const char *label)
{
   t_radio *r;
   t_object *b;
   static t_typefun tf;
   static t_tcfun tc;
   static int virgin = 1;

   if (radiogroup == NULL)
      return -1;
   r = GetMem0(t_radio, 1);
   if (radiogroup->dir&R_VERTICAL)
      b = KernalAddButton(DOWNLEFT | FILLSPACE, label, RadioState, r);
   else
      b = KernalAddButton(RIGHT|EQUALHEIGHT|(radiogroup->dir&EQUALWIDTH), label, RadioState, r);
   b->Action = RadioState;
   r->b = b;
   r->i = radiogroup->n++;
   if (r->i == *radiogroup->selvar) {
      radiogroup->focus = r->b;
   }
   if (virgin) {
      virgin = 0;
      tf = *r->b->tf;
      tf.Draw = RadioDraw;
      tf.Free = XtendedFree;
      tf.AddHandler = AddRadioHandler;
      tc = default_slave_tc_functions;
      tc.SetFocus = RadioSetFocus;
      tc.UnSetFocus = UnSetFocus;
   }
   b->tf = &tf;
   b->tcfun = &tc;
   return b->id;
}

extern int MkRadioContainer(int x, int y, int *var, int direction)
{
   t_node *nd;
   t_radiogroup *r;

   if (radiogroup)
      EndRadioContainer();
   nd = MkNode(opwin->win->opnode, x, y, ADAPTIVE, StubOb);
   r = radiogroup = GetMem0(t_radiogroup, 1);
   r->selvar = var;
   r->dir = direction;
   r->nd = nd;
   nd->data = r;
   nd->ob->tf = &xtended_node_type_functions;
   SetDistance(0, 0);
   nd->ob->tf->DoJoinTabChain(nd->ob);
   return nd->ob->id;
}

extern void EndRadioContainer(void)
{
   if (radiogroup) {
      if (radiogroup->focus)
         SetSubFocusOfLink(radiogroup->focus->tablink, radiogroup->focus);
      CloseNode();
      radiogroup = NULL;
   }
}
