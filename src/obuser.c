/* Module OBTAG.C
   Contains functions for creating a tag-object. This is a
   simple object with the text on one row wihtout any frame. It may typically
   be used in conjunction with other objects like radio-buttons, or to give
   an extra information to the user. */

#include <string.h>

#include <allegro.h>
#include "mstates.h"
#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "tabchain.h"

typedef struct {
   void *appdata;
   void (*Draw)(void *appdata, BITMAP *bmp, enum cgui_widget_sates);
   void (*SingleClick)(void *appdata);
   int w;
   int h;
   BITMAP *bmp;
   const BITMAP *parent_bmp;
} t_widget_data;

static void DrawWrapper(t_object *b)
{
   t_widget_data *widget_data;
   
   if (b->parent->bmp) {
      widget_data = b->appdata;
      if (widget_data->parent_bmp != b->parent->bmp) {
         destroy_bitmap(widget_data->bmp);
         widget_data->bmp = NULL;
      }
      if (widget_data->bmp == NULL) {
         widget_data->bmp = create_sub_bitmap(b->parent->bmp, b->x1, b->y1, widget_data->w, widget_data->h);
      }
      switch (b->state) {
      case MS_INIT:
         widget_data->Draw(widget_data->appdata, widget_data->bmp, CGUI_WIDGET_STATE_NORMAL);
         break;
      case MS_OVER:
      case MS_SECOND_OVER:
         widget_data->Draw(widget_data->appdata, widget_data->bmp, CGUI_WIDGET_STATE_OVER);
         break;
      case MS_RESIZING:
         break;
      case MS_FIRST_DOWN:
      case MS_SECOND_DOWN:
         widget_data->Draw(widget_data->appdata, widget_data->bmp, CGUI_WIDGET_STATE_DOWN);
         break;
      case MS_FIRST_DOWN_OFF:
      case MS_SECOND_DOWN_OFF:
         widget_data->Draw(widget_data->appdata, widget_data->bmp, CGUI_WIDGET_STATE_DOWN_BESIDE);
         break;
      case MS_GRIPPED:
         widget_data->Draw(widget_data->appdata, widget_data->bmp, CGUI_WIDGET_STATE_GRIPPED);
         break;
      case MS_POSSIBLE_GRIP:
         break;
      case MS_DRAGGING_OVER:
         widget_data->Draw(widget_data->appdata, widget_data->bmp, CGUI_WIDGET_STATE_DRAGGING_OVER);
         break;
      case MS_DRAGGING_OVER_ILLEGAL:
         break;
      case MS_POSSIBLE_SLIDE:
         break;
      case MS_SLIDING:
         break;
      }
   }
}

static void UserFree(t_object *b)
{
   t_widget_data *widget_data;
   widget_data = b->appdata;
   if (widget_data->bmp) {
      destroy_bitmap(widget_data->bmp);
   }
   XtendedFree(b);
}

void ActionWrapper(void *data)
{
   t_widget_data *widget_data = data;
   widget_data->SingleClick(widget_data->appdata);
}

static void SetSize(t_object *b)
{
   t_widget_data *widget_data;
   
   widget_data = b->appdata;
   if (b->dire)
      b->x1 = b->y1 = 0;
   
   b->x2 = b->x1 + widget_data->w - 1;
   b->y2 = b->y1 + widget_data->h - 1;
}

/* Application interface: */

extern int AddUserDefinedWidget(int x, int y, int w, int h, void (*Draw)(void *, BITMAP *bmp, enum cgui_widget_sates), void (*single_click)(void*), void *data)
{
   t_object *b;
   static t_typefun tf;
   static t_tcfun tcfun;
   static int virgin = 1;
   t_widget_data *widget_data;

   if (virgin) {
      virgin = 0;
      tf = default_type_functions;
      tf.Draw = DrawWrapper;
      tf.SetSize = SetSize;
      tf.Free = UserFree;
      tcfun = default_tabchain_functions;
   }
   b = CreateObject(x, y, opwin->win->opnode);
   b->autohk = 0;
   b->tf = &tf;
   b->tcfun = &tcfun;
   widget_data = GetMem0(t_widget_data, 1);
   b->appdata = widget_data;
   b->Action = ActionWrapper;
   widget_data->w = w;
   widget_data->h = h;
   widget_data->Draw = Draw;
   widget_data->SingleClick = single_click;
   widget_data->appdata = data;
   return b->id;
}

