/* Module NDCONT.C

   Contains functions for creating a container. A container is a node object
   which purpose is to make an area to group any number and type of other
   objects in. Optionally it may be outlined by a frame, and it may have a
   header. There is also a possibility to (optionally) join the tab chain.

   The container musts first be "opened" with a StartContainer call. When all
   the objects have been put into it, it must be closed with a call to
   EndContainer. The opened container(s) will be kept track of in a stack -
   last opened first closed - so no reference is needed at closing.

   Container may be recursively created, and there may be an arbitrary number
   of containers "in parallell" in a window or in another container.
   Containters may be "relative positioned" in the same manner as simple
   objects. They may also be have the dimensions "ADAPTIVE", which means that
   it will get a size large enough to contain its objects */

#include <string.h>
#include <allegro.h>

#include "cgui.h"
#include "cgui/mem.h"

#include "node.h"
#include "window.h"
#include "ndcont.h"

#if ALLEGRO_SUB_VERSION == 0
#define textout_ex(bmp, f, s, x, y, col, bgcol) textout(bmp, f, s, x, y, col)
#endif

typedef struct t_cst {
   int options;
   t_object *cont;
   struct t_cst *next;
   int nonodes;
} t_cst;

t_cst *cont_stack;

/* This function draws the border of an outlined container that has no label.
 */
static void DrawBorder(BITMAP * bmp, int x1, int y1, int x2, int y2)
{
   hline(bmp, x1,     y1,     x2,     cgui_colors[CGUI_COLOR_SHADOWED_BORDER]); // Top
   hline(bmp, x1 + 1, y1 + 1, x2,     cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]); // Top+1
   vline(bmp, x1,     y1 + 1, y2,     cgui_colors[CGUI_COLOR_SHADOWED_BORDER]); // Left
   vline(bmp, x1 + 1, y1 + 2, y2 - 1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]); // Left+1
   hline(bmp, x1 + 2, y2 - 1, x2 - 1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]); // Bottom-1
   hline(bmp, x1 + 1, y2,     x2,     cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]); // Bottom
   vline(bmp, x2 - 1, y1 + 2, y2 - 2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]); // Right-1
   vline(bmp, x2,     y1 + 2, y2 - 1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]); // Right
}

/* This function draws the border of an outlined container that has no label.
 */
static void DrawContainerBorder(t_object * b)
{
   if (b->parent->bmp)
      DrawBorder(b->parent->bmp, b->x1, b->y1, b->x2, b->y2);
}

/* This function draws the lower part of the border, not contained in the
   header part. */
static void DrawContainerBorderLabel(t_object * b)
{
   BITMAP *bmp;
   int x1, y1, x2, y2, h, tw;
   t_node *nd;
   char *label;

   bmp = b->parent->bmp;
   if (bmp == NULL)
      return;
   nd = b->node;
   label = nd->data;
   h = text_height(b->font);
   tw = text_length(b->font, label);
   x1 = b->x1;
   y1 = b->y1 + (h >> 1) - 1;
   x2 = b->x2;
   y2 = b->y2;
   /* Fill space at top not occupide by the border-lines */
   rectfill(bmp, x1, b->y1, x2, y1 - 1, cgui_colors[CGUI_COLOR_CONTAINER]);
   rectfill(bmp, x1 + CT_FRAME_WIDTH, y1 + CT_FRAME_WIDTH, x2 - CT_FRAME_WIDTH, b->y1 + h - 1, cgui_colors[CGUI_COLOR_CONTAINER]);
   DrawBorder(bmp, b->x1, y1, b->x2, b->y2);
   /* Now all frame-space is filled. We must rub the part of the border-line
      where we want to put the label */
   rectfill(bmp, x1 + CT_LABEL_OFFSET - 2, y1, x1 + tw + CT_LABEL_OFFSET + 2, y1 + 1, cgui_colors[CGUI_COLOR_CONTAINER]);
   textout_ex(bmp, b->font, label, x1 + CT_LABEL_OFFSET, b->y1, cgui_colors[CGUI_COLOR_CONTAINER_LABEL], -1);
}

static void FrameLabelStickSize(t_object * b)
{
   b->y1 = 0;
   b->y2 = -1;
   b->x2 = text_length(b->font, b->label) + ((CT_LABEL_OFFSET + CT_FRAME_WIDTH) << 1) - 1;
}

static void LabelStickSize(t_object * b)
{
   b->y1 = 0;
   b->y2 = -1;
   b->x2 = text_length(b->font, b->label);
}

static void SetNodeSize(t_object *b)
{
   if (b->node->fixsize) {
      if (b->dire)
         b->x1 = b->y1 = 0;
      b->x2 = b->x1 + b->node->width - 1 + b->rex;
      b->y2 = b->y1 + b->node->height - 1 + b->rey;
   } else {
      b->x1 = b->y1 = b->x2 = b->y2 = 0;
   }
}

static void Stretch(t_object * b, int dx, int dy)
{
   t_object *app = b->node->last;

   b->rex += dx;
   b->rey += dy;
   app->rex += dx;
   app->rey += dy;
}

static void SetLabelNodeSize(t_object * b)
{
   if (b->dire || !b->node->fixsize)
      b->x1 = b->y1 = b->x2 = b->y2 = 0;
   if (b->node->fixsize) {
      b->x2 += b->rex;
      b->y2 += b->rey;
      b->rex = 0;
      b->rey = 0;
   }
}

static void ContainerSetSizeOffset(t_object *b, int w, int h)
{
   DefaultSetSizeOffset(b, w, h);
   DefaultSetSizeOffset(b->parent->ob, w, h);
}

static t_object *GetMainNode(t_object *b)
{
   return b->parent->ob;
}

/* Application interface: */

/* A container will consist of either of:
   - A plain node (the case when no border and no label)
   - An inner node into which the app can put objects and an outer node that
   encapsulates the inner one and all the possible decorations (in case of
   label but not frame, the tag-object itsefl needs to be wrapped with yet
   one node). The return id in this case is the app id. Subsequent calls
   from the app that concerns the outer node must therefore take notice
   about this (as well as calls concerning both) */
extern int StartContainer(int x, int y, int width, int height,
       const char *label, int options)
{
   t_node *top, *app;
   t_cst *cl;
   static t_typefun tf_fl, tf_l, tf_main, wrapped_app_tf;
   static int virgin = 1;
   t_object *b;

   if (label == NULL)
      return 0;
   cl = GetMem(t_cst, 1);
   cl->nonodes = 1;
   if ((options & CT_BORDER) || *label) {
      if (*label == 0) {        /* border = YES; label = NO */
         top = MkNode(opwin->win->opnode, x, y, width, height,
                                          DrawContainerBorder);
         SetDistance(CT_FRAME_WIDTH, CT_FRAME_WIDTH);
      } else if ((options & CT_BORDER) == 0) {  /* border = NO; label = YES */
         top = MkNode(opwin->win->opnode, x, y, width, height, StubOb);
         SetDistance(0, 0);
         MkNode(opwin->win->opnode, TOPLEFT | FILLSPACE, ADAPTIVE, StubOb);
         AddTag(TOPLEFT, label);
         CloseNode();
      } else {                  /* border = YES; label = YES */
         top = MkNode(opwin->win->opnode, x, y, width, height,
                                          DrawContainerBorderLabel);
         SetDistance(CT_FRAME_WIDTH, CT_FRAME_WIDTH);
         top->topy = text_height(top->ob->font);
      }
      if (virgin) {
         virgin = 0;
         tf_main = xtended_node_type_functions;
         tf_main.Stretch = Stretch;
         tf_main.MakeNodeRects = StubOb;

         tf_fl = tf_l = default_type_functions;
         tf_fl.SetSize = FrameLabelStickSize;
         tf_l.SetSize = LabelStickSize;
         wrapped_app_tf = default_node_type_functions;
         wrapped_app_tf.SetSizeOffset = ContainerSetSizeOffset;
         wrapped_app_tf.GetMainNode = GetMainNode;
      }
      top->SetSize = SetNodeSize;
      top->ob->tf = &tf_main;

      app = MkNode(opwin->win->opnode, DOWNLEFT | FILLSPACE | VERTICAL |
                                       HORIZONTAL, ADAPTIVE, StubOb);
      cl->nonodes++;
      SetDistance(2, 2);
      if (*label) {
         /* put an empty stick in it to keep the min size (...and let it be
            the holder of the label) */
         b = CreateObject(0, 0, app);
         b->label = MkString(label);
         b->exclude_add = 1;
         b->inactive = 1;
         top->data = MkString(label);
         if (options & CT_BORDER)
            b->tf = &tf_fl;
         else
            b->tf = &tf_l;
         app->SetSize = SetLabelNodeSize;
      } else
         app->SetSize = SetNodeSize;
      app->ob->tf = &wrapped_app_tf;
   } else {                     /* border = NO; label = NO */
      app = MkNode(opwin->win->opnode, x, y, width, height, StubOb);
      app->SetSize = SetNodeSize;
   }
   cl->cont = app->ob;
   cl->options = options;
   cl->next = cont_stack;
   cont_stack = cl;
   app->type = NODETYPE_CONT;
   return app->ob->id;
}

extern void EndContainer(void)
{
   t_node *nd;
   t_cst *cl;

   if (cont_stack) {
      nd = cont_stack->cont->node;
      if (cont_stack->options & CT_OBJECT_TABLE)
         JoinTabChain(cont_stack->cont->id);
      cl = cont_stack;
      cont_stack = cont_stack->next;
      for (; cl->nonodes--;)
         CloseNode();
      Release(cl);
   }
}

extern void EmptyContainer(int id)
{
   t_node *nd;

   nd = GetNode(id);
   if (nd->type == NODETYPE_CONT)
      CleanNode(nd);
}

extern void SelectContainer(int id)
{
   t_node *nd;

   nd = GetNode(id);
   if (nd && nd->win) {
      opwin = nd->win->node;
      nd->win->opnode = nd;
   }
}

extern void ReBuildContainer(int id)
{
   t_node *nd;

   nd = GetNode(id);
   if (nd)
      DisplayNode(nd);
}
