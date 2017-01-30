/* Module NODE.C
   Contains functions for handling of "nodes", i.e. handling of objects that
   are internal nodes in the object tree. */

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "msm.h"
#include "mouse.h"
#include "object.h"
#include "event.h"
#include "window.h"
#include "id.h"
#include "rectlist.h"
#include "hotkeyau.h"
#include "ndresize.h"
#include "refresh.h"
#include "tabchain.h"
#include "node.h"

#define INIT_RECT_SIZE 50

t_typefun xtended_node_type_functions, default_node_type_functions;
t_node *fwin, *opwin;
t_hkfun default_node_hotkey_functions;

static void CreateBitmap(t_node *nd)
{
   t_object *b;
   int width, height;

   b = nd->ob;
   width = b->x2 - b->x1 + 1;
   height = b->y2 - b->y1 + 1;
   if (nd->bmp) {
      Release(nd->bmp);
   }
   if (b == _win_root) {                 /*no bitmap for the root-node */
      ;
   } else if (IsWindow(b)) {  /* it's a window */
      if (width > 0 && height > 0)
         nd->bmp = create_bitmap(width, height);
      /* clear_to_color(nd->bmp, cgui_colors[CGUI_COLOR_CONTAINER]); */
   } else if (b->parent->bmp &&
             (b->x1 < b->parent->bmp->w && b->y1 < b->parent->bmp->h)) {
      if (width > 0 && height > 0) {
         nd->bmp = create_sub_bitmap(b->parent->bmp, b->x1, b->y1, width, height);
      }
   } else {
      nd->bmp = NULL;
   }
}

/* Fills up the parts of the node that no sub-object is responsible for. */
static void DrawRectangles(t_node *nd)
{
   t_coord *r;
   int i;

   if (nd->bmp)
      for (i = 0, r = nd->r; i < nd->nr; i++, r++)
         rectfill(nd->bmp, r->x1, r->y1, r->x2, r->y2, cgui_colors[CGUI_COLOR_CONTAINER]);
}

/* This function takes the entire window surface, removes each part of the
   surface that an object has responsibility for, and split the rest of the
   surface into rectangles. This rectangle list will be stored in the
   window-struct and is used when drawing the window. */
extern void MakeNodeRects(t_node *nd, t_coord *r0)
{
   t_coord *rects;
   t_object *b;
   int n, init;

   n = 1;
   init = INIT_RECT_SIZE;
   rects = ResizeMem(t_coord, nd->r, init);
   *rects = *r0;
   for (b = nd->last; b; b = b->prev)
      if (!b->exclude_rect && !IsWindow(b))
         rects = _RectList(b->x1, b->y1, b->x2, b->y2, rects, &n, init);
   if (n < init / 2) {
      if (n == 0) {
         Release(rects);
         rects = NULL;
      } else
         rects = ResizeMem(t_coord, rects, n);
   }
   nd->nr = n;
   nd->r = rects;
}

static void DefaultNodeRects(t_object *b)
{
   t_coord r;
   t_node *nd;
   int width, height;

   width = b->x2 - b->x1 + 1;
   height = b->y2 - b->y1 + 1;
   nd = b->node;
   r.x1 = 0;
   r.y1 = 0;
   r.x2 = width - 1;
   r.y2 = height - 1;
   MakeNodeRects(nd, &r);
}

/* If the node is adaptive, it has been initataed to w=0,h=0. As soon as an
   object requires more space than is available in the node, this has to be
   extended. That is what this function does. It should be called by any
   object-building function if node is adaptive. */
static void Adapt(t_object *b0, t_object *b)
{
   t_node *nd;
   int width, height;

   nd = b0->node;
   width = b0->x2 - b0->x1;
   height = b0->y2 - b0->y1;
   if (b->x2 > width - nd->rightx && !b->exclude_rect)
      b0->x2 = b0->x1 + b->x2 + nd->rightx;
   if (b->y2 > height - nd->boty && !b->exclude_rect)
      b0->y2 = b0->y1 + b->y2 + nd->boty;
}

extern void NoAdapt(t_object *b0 nouse, t_object *b nouse)
{
}

/* This function builds and draws a node. Normally this is not needed - only
   if it has been Cleaned and than a new one is created in the same place
   this is necessary (like in tab-windows). If the node is self-sizing (and
   you are not sure that it will fill exactely the space of the previously
   removed one) you should instead run the DisplayWin, to make sure that the
   new does not overlap other objects in the parent or that space outside the
   new one is not left over. In the latter case it should be enough to
   re-create the rectangles of the parent. */
extern void DisplayNode(t_node *nd)
{
   t_object *b;

   b = nd->ob;
   b->tf->SetSize(b);
   b->EqualSize(b);
   b->Position(b);
   b->Pack(b);
   b->tf->MakeBitmap(b);
   GenerateAutoHotKeys(nd->win->node);
   b->tf->Refresh(b);
}

/* Default function transfering mouse events down the tree */
static int MouseEvNode(t_mousedata *m, t_object *mb)
{
   t_node *nd;
   t_object *b;
   int x, y, isover;

   nd = mb->node;
   x = m->x - mb->parent->win->node->ob->x1 - mb->parent->wx;
   y = m->y - mb->parent->win->node->ob->y1 - mb->parent->wy;
   isover = isOver(x, y, mb);
   for (b = nd->firstob; b; b = b->next) {
      if (!IsWindow(b)) {
         if (b->me->MouseEv(m, b)) {
            return 1;
         }
      }
   }
   return isover;
}


/* This function will not update the screen. It will not either destroy the
   given node. It will only release the _contents_ of the node The node
   object itself, the node with its bitmap is still left and fit for fight.
   I.e. it is intact concerning its life in the parent node but the content
   of the window is undefined unless the part eventually taken care of by the
   node-drawing routine. A call to this is meaningfull if you want to
   re-create other objects in the node, but wants to keep the previous size
   of the node in the parent node. */
extern void CleanNode(t_node *nd)
{
   t_object *b, *next;

   for (b = nd->last; b; b = next) {
      next = b->prev;
      if (!IsWindow(b))
         b->tf->Free(b);
   }
   nd->last = NULL;
   nd->firstob = NULL;
   nd->ip = NULL;
   if (nd->r) {
      Release(nd->r);
      nd->r = NULL;
      nd->nr = 0;
   }
}

static void DelayedFreeNode(void *data)
{
   t_node *nd = data;

   nd->ob->tf->Free(nd->ob);
}

/* This will entirely erase the node. The tree of decendants will be
   traversed bottom up to free all objects. Bitmap and rectangle-list will be
   restored If this node happens to be the operating node, then the operating
   point in the tree will be moved to next level above. The default free
   handling for the object will be processed The node-data itself will be
   released Note!!: the node as an object in the parent window will not be
   unlinked i.e. at end of this function the links in the parent window are
   inconsistent. */
static void FreeNode(t_object *b0)
{
   t_node *nd;
   t_object *b, *next;

   nd = b0->node;
   if (nd->processing_callback) {
      _GenEventOfCgui(DelayedFreeNode, nd, 0, b0->id);
      nd->processing_callback = 0;
      return;
   }
   for (b = nd->firstob; b; b = next) { /* release bottom up */
      next = b->next;
      b->tf->Free(b);
   }
   nd->firstob = nd->last = NULL;
   if (nd->bmp)
      destroy_bitmap(nd->bmp);
   if (nd->r)
      Release(nd->r);
   if (nd->win && nd == nd->win->opnode)
      nd->win->opnode = b0->parent;
   nd->tf->Free(nd->ob);
   if (nd->image)
      DestroyImages(nd->image);
   Release(nd);
}

/* As FreeNode, but also releases memory for the data-component */
static void FreeXtendedNode(t_object *b)
{
   t_node *nd;

   nd = b->node;
   if (nd->processing_callback) {
      _GenEventOfCgui(DelayedFreeNode, nd, 0, b->id);
      nd->processing_callback = 0;
      return;
   }
   if (nd->data)
      Release(nd->data);
   FreeNode(b);
}

static void DefaultNodeSetView(t_object *b0, int flags)
{
   t_node *nd;
   t_object *b;

   nd = b0->node;
   for (b = nd->last; b; b = b->prev)
      if (!IsWindow(b))
         b->tf->SetView(b, flags);
   b0->view = flags;
}

static void NodeActivate(t_object *b0)
{
   t_node *nd;
   t_object *b;

   nd = b0->node;
   for (b = nd->last; b; b = b->prev)
      if (!IsWindow(b))
         b->tf->Activate(b);
   b0->inactive = 0;
}

static void NodeDeActivate(t_object *b0)
{
   t_node *nd;
   t_object *b;

   nd = b0->node;
   for (b = nd->last; b; b = b->prev)
      if (!IsWindow(b))
         b->tf->DeActivate(b);
   b0->inactive = 1;
}

/* May run in any direction */
static void NodeDraw(t_object *b0)
{
   t_node *nd;
   t_object *b;

   nd = b0->node;
   if (nd->bmp) {
      nd->Draw(b0);          /* this may typically draw some kind of frame */
      for (b = nd->firstob; b; b = b->next) {
         if (!IsWindow(b)) {
            b->tf->Draw(b);
         }
      }
      DrawRectangles(nd);    /* always fill unused space */
   }
}

static int NodeAddHandler(t_object *b0, void (*Handler) (void *data),
                          void *data)
{
   t_node *nd;
   t_object *b;
   int success = 0;

   nd = b0->node;
   for (b = nd->last; b; b = b->prev)
      if (!IsWindow(b))
         success |= b->tf->AddHandler(b, Handler, data);
   return success;
}

static void NodeCatchImage(struct t_object *b0)
{
   t_node *nd;
   t_object *b;

   nd = b0->node;
   for (b = nd->firstob; b; b = b->next)
      if (!IsWindow(b))
         b->tf->CatchImage(b);
}

static int NodeChangeData(t_object *b0, void *data, int type)
{
   t_node *nd;
   t_object *b;

   nd = b0->node;
   for (b = nd->last; b; b = b->prev)
      if (!IsWindow(b))
         if (b->tf->ChangeData(b, data, type))
            return 1;
   return 0;
}

/* Traversal order is unimportant */
static void TransferSetSize(t_object *b0)
{
   t_node *nd;
   t_object *b;

   nd = b0->node;
   nd->SetSize(b0);
   for (b = nd->firstob; b; b = b->next) {
      if (!IsWindow(b))
         b->tf->SetSize(b);
   }
}

/* Traversal order is unimportant */
static void TransferEqualSize(t_object *b0)
{
   t_node *nd;
   t_object *b;

   nd = b0->node;
   for (b = nd->firstob; b; b = b->next)
      if (!IsWindow(b))
         b->EqualSize(b);
   nd->EqualSize(b0);
}

/* Must run in postorder since a parent objects may be adaptive, i.e. the
   will adapt their size to be large enough to show all the kids. The
   direction-commands requires that objects within same node are processed in
   the order they were issued. */
static void TransferPosition(t_object *b0)
{
   t_node *nd;
   t_object *b;

   nd = b0->node;
   for (b = nd->firstob; b; b = b->next) {
      if (!IsWindow(b)) {
         b->Position(b);
         b0->Adapt(b0, b);
      }
   }
   nd->Position(b0);
}

/* After the "Position"-pass has been run, the positions has to be adjusted
   again. This time all "external references" has to be resolved, i.e.
   objects refering to size/position of a parent object will be set.
   Therefore the tree-traversal must be postorder. Currently the issuing
   order within an object is unimportant. */
static void TransferPack(t_object *b0)
{
   t_node *nd;
   t_object *b;

   nd = b0->node;

   nd->Pack(b0);

   /* Align is not a recursive function (just the kids to current node will
      be run) */
   for (b = nd->last; b; b = b->prev)
      if (!IsWindow(b))
         b->Align(b);
   for (b = nd->last; b; b = b->prev)
      if (!IsWindow(b))
         b->Pack(b);

}

/* Must run in preorder, since the kids uses their parent bitmaps. */
static void TransferMakeBitmaps(t_object *b0)
{
   t_node *nd;
   t_object *b;

   nd = b0->node;
   if (nd != nd->win->node) {   /* don't adjust position of the actual window
                                 */
      nd->wx = b0->x1 + b0->parent->wx;
      nd->wy = b0->y1 + b0->parent->wy;
   }
   CreateBitmap(nd);
   for (b = nd->last; b; b = b->prev)
      if (!IsWindow(b))
         b->tf->MakeBitmap(b);
   b0->tf->MakeNodeRects(b0);
}

/* Must run in postorder, subbitmaps shall be released before their parents. */
static void TransferDestroyBitmaps(t_object *b0)
{
   t_node *nd;
   t_object *b;

   nd = b0->node;
   for (b = nd->last; b; b = b->prev)
      if (!IsWindow(b))
         b->tf->DestroyBitmap(b);
   if (nd->bmp) {
      destroy_bitmap(nd->bmp);
      nd->bmp = NULL;
   }
}

static void TransferGetLabel(t_object *b0)
{
   t_node *nd;
   t_object *b;

   nd = b0->node;
   for (b = nd->firstob; b; b = b->next)
      if (!IsWindow(b))
         b->hkfunc->GetLabel(b);
}

static void NodeNewScreenMode(t_object *b0)
{
   t_node *nd;
   t_object *b;
   struct t_image *im;

   nd = b0->node;
   im = nd->image;
   nd->image = NULL;
   NewScreenModeImageList(im, nd);
   for (b = nd->firstob; b; b = b->next)
      b->tf->NewScreenMode(b);
}

static void NodePrepareNewScreenMode(t_object *b0)
{
   t_node *nd;
   t_object *b;

   nd = b0->node;
   for (b = nd->firstob; b; b = b->next)
      b->tf->PrepareNewScreenMode(b);
}

extern t_node *GetNode(int id)
{
   t_object *b;

   for (b = GetObject(id); b && !b->node; b = b->parent->ob)
      ;
   if (b)
      return b->node;
   return NULL;
}

static void SetSize(t_object *b)
{
   t_node *nd;

   nd = b->node;
   if (b->dire) {
      b->x1 = 0;
      b->y1 = 0;
   }
   if (nd->fixsize) {
      b->x2 = b->x1 + nd->width - 1;
      b->y2 = b->y1 + nd->height - 1;
   } else {
      b->x2 = 0;
      b->y2 = 0;
   }
}

extern void SetFixSizeNode(t_node *nd, int width, int height)
{
   nd->fixsize = 1;
   nd->width = width;
   nd->height = height;
   nd->ob->Adapt = NoAdapt;
}

extern void SetAdaptiveNode(t_node *nd)
{
   nd->fixsize = 0;
   nd->ob->Adapt = Adapt;
}

extern void SetNodeStateMachine(struct t_mevent *node_me)
{
   SetDefaultStateMachine(node_me);
   node_me->MouseEv = MouseEvNode;
}

static void NodeOver(t_object *b)
{
   (void)b;
}

/* Makes an internal node in the object tree. Coordinates shall refere to the
   parent window's coordinate system (or use the direction-command). Size my
   be SELFSIZING. The Draw function shall only draw the specific stuff for
   the node e.g. frame or header. */
extern t_node *MkNode(t_node *parent, int x, int y, int width, int height,
                      void (*DrawSw) (t_object *))
{
   t_node *nd;
   t_object *b;
   static struct t_mevent node_me;
   static t_typefun *tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      SetNodeStateMachine(&node_me);
   }
   b = CreateObject(x, y, parent);
   tf = b->tf;
   b->tf = &default_node_type_functions;
   b->hkfunc = &default_node_hotkey_functions;
   b->Action = Stub;
   b->Over = NodeOver;
   nd = GetMem0(t_node, 1);
   nd->SetSize = SetSize;
   nd->EqualSize = b->EqualSize;
   nd->Position = b->Position;
   nd->Pack = b->Pack;
   nd->me = b->me;
   /* exchange pointers */
   nd->ob = b;
   b->node = nd;

   b->me = &node_me;
   b->EqualSize = TransferEqualSize;
   b->Position = TransferPosition;
   b->Pack = TransferPack;
   b->click = 0;

   if (parent) {
      if (parent->ob == _win_root) { /* this is a window */
         nd->xdist = DEFAULT_XDIST;
         nd->dy = nd->topy = nd->boty = DEFAULT_YDIST;
      } else {
         nd->xdist = parent->xdist;
         nd->dy = parent->dy;
         nd->boty = parent->boty;
         nd->topy = parent->topy;
         nd->win = parent->win;
      }
      if (parent->win && parent->win->node != parent)
         parent->win->opnode = nd;
   } else {
      /* this is the rootnode, it has no parent. */
   }
   if (height != AUTOINDICATOR) {
      SetFixSizeNode(nd, width, height);
   } else {
      b->Adapt = Adapt;
   }
   nd->tf = tf;

   nd->Draw = DrawSw;

   return nd;
}

/* This function will close the input to the current node. It will not remove
   anything! This function shall be used in conjunction with tab- windows and
   containers. The result of the call is that the node that is currently the
   recipient of your "Add-Object"-commands, will no longer be so. The control
   returns to the previously recipient, i.e. the parent node. */
extern void CloseNode(void)
{
   if (opwin->win->opnode->ob->parent)
      opwin->win->opnode = opwin->win->opnode->ob->parent;
}

extern void InitNode(void)
{
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      default_node_type_functions = default_type_functions;
      default_node_type_functions.ChangeData = NodeChangeData;
      default_node_type_functions.Free = FreeNode;
      default_node_type_functions.Activate = NodeActivate;
      default_node_type_functions.DeActivate = NodeDeActivate;
      default_node_type_functions.Draw = NodeDraw;
      default_node_type_functions.AddHandler = NodeAddHandler;
      default_node_type_functions.SetSize = TransferSetSize;
      default_node_type_functions.DestroyBitmap = TransferDestroyBitmaps;
      default_node_type_functions.MakeBitmap = TransferMakeBitmaps;
      default_node_type_functions.MakeNodeRects = DefaultNodeRects;
      default_node_type_functions.MakeStretchable = MakeNodeStretchable;
      default_node_type_functions.DoJoinTabChain = NodeDoJoinTabChain;
      default_node_type_functions.CatchImage = NodeCatchImage;
      default_node_type_functions.SetView = DefaultNodeSetView;
      default_node_type_functions.PrepareNewScreenMode = NodePrepareNewScreenMode;
      default_node_type_functions.NewScreenMode = NodeNewScreenMode;

      xtended_node_type_functions = default_node_type_functions;
      xtended_node_type_functions.Free = FreeXtendedNode;

      default_node_hotkey_functions = default_hotkey_functions;
      default_node_hotkey_functions.GetLabel = TransferGetLabel;
      default_node_hotkey_functions.TakeHotKey = NodeTakeHotKey;
      default_node_hotkey_functions.IsHotKey = NodeIsHotKey;
   }
}

/* Application interface: */

extern void SetOperatingNode(int id)
{
   t_object *b = GetObject(id);

   if (b && b->node && opwin)
      opwin->win->opnode = b->node;
}
