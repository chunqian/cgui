/* Module CURSOR.C
   Contains functions that handles the mouse cursor */

#include <allegro.h>

#include "cgui.h"
#include "cgui/mem.h"
#include "graphini.h"
#include "cursor.h"
#include "window.h"
#include "object.h"
#include "node.h"
#include "rectlist.h"
#include "mouse.h"
#include "id.h"
#include "assert.h"
#include "cursdata.h"
#include "obbutton.h"

#if ALLEGRO_SUB_VERSION == 0
#define textout_ex(bmp, f, buf, x, y, col, bgcol) textout(bmp, f, buf, x, y, col)
#endif

typedef struct t_cursor {
   BITMAP *sprite;
   int x, y;                   /* Hot spot */
   int nr;                     /* Index in array */
} t_cursor;

typedef struct t_appcursor {
   t_cursor c;                  /* The merged cursor */
   BITMAP *org;                 /* This is original overlay bitmap */
   t_cursor *topc;              /* Pointer to the current top cursor */
} t_appcursor;

static t_appcursor app;
static t_cursor *cursors, *curptr;
static int nr_cursors;

static void MakeCursorSprite(BITMAP *bmp, int i)
{
   t_image *im;
   char s[100];

   sprintf(s, "cursor42_%d", i);
   RegisterImageCopy(bmp, s, IMAGE_TRANS_BMP, 0);
   im = GetImage(_win_root->node, s);
   if (im)
      cursors[i].sprite = im->bmp;
   cursors[i].nr = i;
}

static void MakeCursor(DATAFILE *datob, int i)
{
   MakeCursorSprite(datob->dat, i);
   sscanf(get_datafile_property(datob, DAT_ID('H','S','_','X')), "%d",
             &cursors[i].x);
   sscanf(get_datafile_property(datob, DAT_ID('H','S','_','Y')), "%d",
             &cursors[i].y);
}

static void MakeCursors(void)
{
   nr_cursors = CURS_COUNT;
   cursors = GetMem0(t_cursor, nr_cursors);
   MakeCursor(curs_cursors+CURS_BUSY, CURS_BUSY);
   MakeCursor(curs_cursors+CURS_DEFAULT, CURS_DEFAULT);
   MakeCursor(curs_cursors+CURS_DRAGGABLE, CURS_DRAGGABLE);
   MakeCursor(curs_cursors+CURS_ILLEGAL, CURS_ILLEGAL);
   MakeCursor(curs_cursors+CURS_DRAG_H, CURS_DRAG_H);
   MakeCursor(curs_cursors+CURS_DRAG_V, CURS_DRAG_V);
   MakeCursor(curs_cursors+CURS_CROSS, CURS_CROSS);
}

static void DestroyCursors(void)
{
   Release(cursors);
   cursors = NULL;
}

static void ReCreateBitmap(BITMAP **orgp)
{
   BITMAP *bmp, *org;
   int cd, ocd, tc, otc, x, y;

   org = *orgp;
   if (org) {
      ocd = bitmap_color_depth(org);
      cd = bitmap_color_depth(screen);
      if (ocd != cd) {
         bmp = create_bitmap(org->w, org->h);
         blit(org, bmp, 0, 0, 0, 0, org->w, org->h);
         if (ocd == 8 || cd == 8) {
            otc = bitmap_mask_color(org);
            tc = bitmap_mask_color(bmp);
            for (x = 0; x < org->w; x++)
               for (y = 0; y < org->h; y++)
                  if (getpixel(org, x, y) == otc)
                     putpixel(bmp, x, y, tc);
         }
         destroy_bitmap(org);
         *orgp = bmp;
      }
   }
}

static void RemakeCursorBitmaps(void)
{
   int i;
   t_image *im;
   char s[100];

   for (i = 0; i < nr_cursors; i++) {
      sprintf(s, "cursor42_%d", i);
      im = GetImage(_win_root->node, s);
      if (im)
         cursors[i].sprite = im->bmp;
      else
         cursors[i].sprite = NULL;
   }
   ReCreateBitmap(&app.c.sprite);
   ReCreateBitmap(&app.org);
}

static void DeInitCursor(void *data)
{
   int *inited = data;

   if (*inited) {
      *inited = 0;
      RemoveOverlayPointer();
      DestroyCursors();
      nr_cursors = 0;
      curptr = NULL;
   }
}

extern int CursorInited(void)
{
   return curptr != NULL;
}

extern void InitCursor(void)
{
   static int inited = 0;

   if (inited) {
      RemakeCursorBitmaps();
      return;
   } else {
      fixup_datafile(curs_cursors);
   }
   
   MakeCursors();
   SelectCursor(CURS_DEFAULT);
   HookCguiDeInit(DeInitCursor, &inited);
   inited = 1;
   show_mouse(screen);
}

extern int GetSelectedCursorNo(void)
{
   if (curptr == NULL)
      return 0;
   return curptr->nr;
}

/* Application interface */

static void InsertTopPointerIntoOverlay(int cursor_nr)
{
   int x, y;

   app.topc = cursors + cursor_nr;
   blit(app.org, app.c.sprite, 0, 0, 0, 0, app.org->w, app.org->h);
   y = app.c.y - app.topc->y;
   x = app.c.x - app.topc->x;
   if (y < 0) {
      y = 0;
   }
   if (x < 0) {
      x = 0;
   }
   draw_sprite(app.c.sprite, app.topc->sprite, x, y);
}

extern void InstallCursor(int cursor_nr, BITMAP *sprite, int x, int y)
{
   int i;

   if (curptr && curptr->nr == cursor_nr)
      show_mouse(NULL);
   if (cursor_nr >= nr_cursors) {
      cursors = ResizeMem(t_cursor, cursors, cursor_nr+1);
      curptr = cursors + cursor_nr;
      for (i = nr_cursors; i <= cursor_nr; i++) {
         cursors[i].sprite = NULL;
      }
      nr_cursors = cursor_nr + 1;
   }
   if (sprite) {
      MakeCursorSprite(sprite, cursor_nr);
      cursors[cursor_nr].x = x;
      cursors[cursor_nr].y = y;
      if (curptr) {
         if (app.c.sprite && app.topc->nr == cursor_nr) {
            InsertTopPointerIntoOverlay(cursor_nr);
            set_mouse_sprite(curptr->sprite);
            show_mouse(screen);
         } else if (curptr->nr == cursor_nr) {
            show_mouse(screen);
         }
      }
   } else {
      cursors[cursor_nr].sprite = NULL;
      if (curptr && curptr->nr == cursor_nr) {
         for (i = 0; i < nr_cursors; i++)
            if (cursors[i].sprite)
               break;
         if (i < nr_cursors)
            SelectCursor(i);
         else
            show_mouse(NULL);
      }
   }
}

extern void SelectCursor(int cursor_nr)
{
   if ((unsigned)cursor_nr < (unsigned)nr_cursors) {
      if (cursors[cursor_nr].sprite) { /* there may be holes */
         if (curptr == &app.c)
            InsertTopPointerIntoOverlay(cursor_nr);
         else
            curptr = cursors + cursor_nr;
         show_mouse(NULL);
         set_mouse_sprite(curptr->sprite);
         set_mouse_sprite_focus(curptr->x, curptr->y);
         show_mouse(screen);
      }
   }
}

extern void PointerLocation(int id, int *x, int *y)
{
   t_object *b;
   int mz;

   _CguiMousePos(x, y, &mz);
   b = GetObject(id);
   if (b && b->parent && b->parent->win) {
      *x -= (b->x1 + b->parent->wx + b->parent->win->node->ob->x1);
      *y -= (b->y1 + b->parent->wy + b->parent->win->node->ob->y1);
   }
}

extern BITMAP *ObjectApearance(int id)
{
   t_object *b;
   BITMAP *bmp = NULL;
   t_node *nd;

   b = GetObject(id);
   if (b) {
      if (b->parent) {
         nd = b->parent;
         if (nd->bmp) {
            bmp = create_bitmap(b->x2 - b->x1 + 1, b->y2 - b->y1 + 1);
            if (bmp)
               blit(nd->bmp, bmp, b->x1, b->y1, 0, 0, bmp->w, bmp->h);
         }
      }
   }
   return bmp;
}

/* Creates a bitmap that is big enough for the both, put the first one
   into it and returns also the new common hot-spot */
static BITMAP *CreateMergedSprite(BITMAP *org, int *x, int *y,
                             BITMAP *top, int thx, int thy)
{
   int i, w, h, xpos1, ypos1, below, above, left, right, hx, hy;
   BITMAP *tmp;
   t_cursor *c;

   hx = *x;
   hy = *y;
   left = MAX(hx, thx);
   right = MAX(org->w - hx, top->w - thx);
   above = MAX(hy, thy);
   below = MAX(org->h - hy, top->h - thy);
   for (i = 0; i < nr_cursors; i++) {
      c = cursors + i;
      if (c->sprite) {
         left = MAX(left, c->x);
         right = MAX(right, c->sprite->w - c->x);
         above = MAX(above, c->y);
         below = MAX(below, c->sprite->h - c->y);
      }
   }
   w = left + right;
   h = above + below;
   tmp = create_bitmap(w, h);
   clear_to_color(tmp, bitmap_mask_color(screen));
   xpos1 = MAX(0, thx - hx);
   ypos1 = MAX(0, thy - hy);
   draw_sprite(tmp, org, xpos1, ypos1);
   *x = MAX(hx, thx);
   *y = MAX(hy, thy);
   return tmp;
}

extern void OverlayPointer(BITMAP *sprite, int x, int y)
{
   BITMAP *bmp;
   int nr;

   if (curptr==NULL)
      return;
   show_mouse(NULL);
   if (curptr == &app.c) {
      nr = app.topc->nr;
      bmp = CreateMergedSprite(sprite, &x, &y, app.org, curptr->x, curptr->y);
      destroy_bitmap(app.org);
      destroy_bitmap(app.c.sprite);
   } else {
      nr = curptr->nr;
      bmp = CreateMergedSprite(sprite, &x, &y, curptr->sprite, curptr->x, curptr->y);
      curptr = &app.c;
   }
   app.c.sprite = create_bitmap(bmp->w, bmp->h);
   app.org = bmp;
   app.c.x = x;
   app.c.y = y;
   InsertTopPointerIntoOverlay(curptr->nr);
   set_mouse_sprite(curptr->sprite);
   set_mouse_sprite_focus(curptr->x, curptr->y);
   show_mouse(screen);
}

extern void RemoveOverlayPointer(void)
{
   if (app.c.sprite) {
      show_mouse(NULL);
      curptr = app.topc;
      destroy_bitmap(app.c.sprite);
      destroy_bitmap(app.org);
      app.c.sprite = NULL;
      app.org = NULL;
      app.topc = NULL;
      if (curptr->sprite) {
         set_mouse_sprite(curptr->sprite);
         set_mouse_sprite_focus(curptr->x, curptr->y);
         show_mouse(screen);
      }
   }
}

extern void MkTextPointer(FONT *f, const char *text)
{
   BITMAP *bmp;
   int w, h;

   w = text_length(f, text);
   h = text_height(f);
   bmp = create_bitmap(w, h);
   clear_to_color(bmp, bitmap_mask_color(screen));
   textout_ex(bmp, f, text, 0, 0, cgui_colors[CGUI_COLOR_DRAGGED_TEXT], -1);
   OverlayPointer(bmp, w / 2, h / 2);
   destroy_bitmap(bmp);
}

/* Obsolete */
extern void ShowPointer(void)
{
   show_mouse(screen);
}

extern void HidePointer(void)
{
   show_mouse(NULL);
}
