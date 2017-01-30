/* Module OBICON.C
   Contains the functions for icon-objects */

#include <string.h>

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "node.h"
#include "object.h"
#include "obbutton.h"
#include "mstates.h"
#include "tabchain.h"

typedef struct t_icon {
   int x, y;
   int isgripped;
   void (*CallBack) (void *);
   void *data;
} t_icon;

static void DisplaySprite(int x1, int y1, int x2, t_image *im, BITMAP *parbmp)
{
   int x, y, space;

   if (im) {
      space = x2 - x1 + 1 - BUTTONBORDER * 2 - im->w;
      x = x1 + BUTTONBORDER + space / 2;
      y = y1 + BUTTONBORDER;
      DrawImage(im, parbmp, x, y);
   }
}

static void DrawIcon(t_object *b)
{
   if (b->parent->bmp == NULL)
      return;
   ButtonText(0, b->x1, b->y1, b->x2, b->y2, b, b->parent->bmp);
   DisplaySprite(b->x1, b->y1, b->x2, b->im, b->parent->bmp);
}

static int Drop(void *dest nouse, int id nouse, void *src nouse,
                int reason nouse, int flags nouse)
{
   return 1;
}

static void *Grip(void *src, int id nouse, int reason)
{
   t_object *b = src;
   t_node *nd;
   int w, h, x1, y1, x2, y2;
   BITMAP *bmp;
   t_icon *ic;

   ic = b->appdata;
   if (reason == DD_GRIPPED) {
      b->tcfun->MoveFocusTo(b);
      nd = b->parent;
      w = b->x2 - b->x1 + 1;
      h = b->y2 - b->y1 + 1;
      bmp = create_bitmap(w, h);
      clear_to_color(bmp, bitmap_mask_color(screen));
      ButtonText(0, 0, 0, b->x2 - b->x1, b->y2 - b->y1, b, bmp);
      if (b->im)
         DisplaySprite(0, 0, w - 1, b->im, bmp);
      OverlayPointer(bmp, b->x - b->x1, b->y - b->y1);
      destroy_bitmap(bmp);
      ic->isgripped = 1;
      ic->x = b->x;
      ic->y = b->y;
   } else if (reason == DD_UNGRIPPED) {
      if (ic->isgripped) {
         RemoveOverlayPointer();
         x1 = b->x1;
         x2 = b->x2;
         y1 = b->y1;
         y2 = b->y2;
         w = x2 - x1 + 1;
         h = y2 - y1 + 1;
         b->x1 += b->x - ic->x;
         b->y1 += b->y - ic->y;
         b->x2 = b->x1 + w - 1;
         b->y2 = b->y1 + h - 1;
         x1 = MIN(x1, b->x1);
         x2 = MAX(x2, b->x2);
         y1 = MIN(y1, b->y1);
         y2 = MAX(y2, b->y2);
         SetBlitLimit(x1, y1, x2, y2);
         Refresh(ID_DESKTOP);
         ic->isgripped = 0;
      }
   }
   return b;
}

static void CallBackWrapper(void *data)
{
   t_icon *ic = data;

   if (ic->CallBack)
      ic->CallBack(ic->data);
   ic->isgripped = 0;
}

static void ReMakeIcon(t_object *b)
{
   BITMAP *bmp, *prevbmp;
   int tr, oldtr, i, j;
   t_image *im;

   im = b->im;
   if (im && (im->type == IMAGE_TRANS_BMP || im->type == IMAGE_BMP)
      && bitmap_color_depth(im->bmp) != bitmap_color_depth(screen)) {
      prevbmp = im->bmp;
      bmp = create_bitmap(im->w, im->h);
      blit(prevbmp, bmp, 0, 0, 0, 0, bmp->w, bmp->h);
      if (bitmap_color_depth(screen) == 8 || bitmap_color_depth(prevbmp) == 8) {
         oldtr = bitmap_mask_color(prevbmp);
         tr = bitmap_mask_color(bmp);
         for (i = 0; i < bmp->w; i++) {
            for (j = 0; j < bmp->h; j++) {
               if (getpixel(prevbmp, i, j) == oldtr)
                  putpixel(bmp, i, j, tr);
            }
         }
      }
      destroy_bitmap(prevbmp);
      im->bmp = bmp;
   }
}

extern int AddIcon(int id, int x, int y, const char *label,
                   void (*CallBack) (void *), void *data)
{
   t_object *b;
   t_node *nd;
   static t_typefun tf;
   static int virgin = 1;
   t_icon *ic;

   if (virgin) {
      virgin = 0;
      tf = default_type_functions;
      tf.Draw = DrawIcon;
      tf.Free = XtendedFree;
      tf.SetSize = SetPushButtonSize;
      tf.MakeBitmap = ReMakeIcon;
   }
   nd = GetNode(id);
   b = CreateObject(x, y, nd);
   InsertLabel(b, label);
   SetObjectGrippable(b->id, Grip, 0x40000000, LEFT_MOUSE, b);
   SetObjectDroppable(b->id, Drop, 0x40000000, b);
   ic = GetMem0(t_icon, 1);
   ic->data = data;
   ic->CallBack = CallBack;
   b->tf = &tf;
   b->appdata = ic;
   b->Action = CallBackWrapper;
   if (b->tablink == NULL)
      b->tf->DoJoinTabChain(b);
   return b->id;
}
