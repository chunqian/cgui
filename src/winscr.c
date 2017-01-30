/* Module WINSCR.C
   Creates desk-top window. */

#include "winscr.h"
#include "window.h"
#include "node.h"
#include "object.h"
#include "graphini.h"
#include "memint.h"

#include "cgui.h"
#include "event.h"
#include "cgui/mem.h"

#include <allegro.h>

static BITMAP *desktop, *desktop_memory;
int cgui_desktop_id;

static void ReturnDesktop(void *data nouse)
{
   if (desktop_memory) {
      destroy_bitmap(desktop_memory);
      desktop_memory = NULL;
   }
}

static void DrawScreen(t_object * b)
{
   t_node *nd;

   nd = b->node;
   if (desktop && nd->bmp) {
      if (desktop->w != SCREEN_W || desktop->h != SCREEN_H) {
         stretch_blit(desktop, nd->bmp, 0, 0, desktop->w, desktop->h, 0, 0, SCREEN_W, SCREEN_H);
      } else {
         blit(desktop, nd->bmp, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
      }
   } else {
      clear_to_color(nd->bmp, cgui_colors[CGUI_COLOR_DESKTOP]);
   }
}

#ifdef DJGPP
#ifndef CGUI_SCAN_DEPEND
#include <sys/movedata.h>
#endif
/* If bios timer (which is supposed to reflect "real time") has advanced more
   than what is reasonable since last time `CheckQ' executed, then this
   can be caused by 3 things
   - the application has not executed for while because the user has alt-tabbed
   - some event(s) in between made a real tough work or if the application
      didn't use the event q in the proposed manner.
   We can't detect the case, but it doesn't matter: we always updates
   (occasionally this will lead to unnecessary updates - so what?)
*/
static void CheckQ(void *data)
{
   int *prev = data, t;

   dosmemget(0x46C, 4, &t);
   if (t - *prev > 40) {
      scare_mouse();
      blit(cgui_bmp, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
      unscare_mouse();
   }
   *prev = t;
   _GenEventOfCgui(CheckQ, prev, 500, 0);
}
#endif

static void UpdateScreen(void *data nouse)
{
   /* vsync(); */
   scare_mouse();
   blit(cgui_bmp, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);/* draw_to_screen_tag */
   unscare_mouse();
}

static void DeInitDesktop(void *data)
{
   int *created = data;
   *created = 0;
}

static void SetSize(t_object *b)
{
   b->x1 = b->y1 = 0;
   b->x2 = SCREEN_W - 1;
   b->y2 = SCREEN_H - 1;
}
/*
static void SaveScreen(void *data)
{
   char *fn = data;
   PALETTE p;

   CloseWin(NULL);
   get_palette(p);
   save_bitmap(fn, cgui_bmp, p);
}

static void PrintScreen(void *data nouse)
{
   static char fn[50];
   MkDialogue(ADAPTIVE, "Screen-shot request", 0);
   AddEditBox(DOWNLEFT, 250, "Enter filename:", FSTRING, 49, fn);
   AddButton(DOWNLEFT, " OK ", SaveScreen, fn);
   DisplayWin();
}
*/
/* This creates the desk-top which is a window without border. */
extern void MkDesktop(void)
{
   t_node *nd, *up;
   static t_typefun tf, toptf;
   static int virgin = 1, created = 0;

   if (created)
      return;
   created = 1;
   HookCguiDeInit(DeInitDesktop, &created);
   nd = MakeWin(FILLSCREEN, NULL, W_NOMOVE);
   if (nd == NULL)
      return;
   if (virgin) {
      virgin = 0;
      toptf = default_window_type_functions;
      toptf.MakeNodeRects = StubOb;
      tf = default_node_type_functions;
      tf.MakeNodeRects = StubOb;
   }
   up = nd->ob->parent;
   up->topy = up->boty = up->leftx = up->rightx = 0;
   SetDistance(0, 0);
   nd->Draw = DrawScreen;
   nd->SetSize = SetSize;
   nd->ob->tf = &tf;
   up->ob->tf = &toptf;
   DisplayWin();
   SetHotKey(0, UpdateScreen, NULL, KEY_F5, 0);
// SetHotKey(0, PrintScreen, NULL, KEY_PRTSCR, 0);
   cgui_desktop_id = nd->ob->id;
#ifdef DJGPP
// dosmemget(0x46C, 4, &t);
// _GenEventOfCgui(CheckQ, &t, 500, 0);
   /* install_int(SetWintabCheck, 100); */
#endif
}

/* Application interface: */

extern void RedrawScreen(void)
{
   UpdateScreen(NULL);
}

extern void DesktopImage(BITMAP * bmp)
{
   int x, y;

   if (!cgui_started)
      HookCguiDeInit(ReturnDesktop, NULL);
   ReturnDesktop(NULL);
   if (is_screen_bitmap(bmp)) {
      desktop = desktop_memory = create_bitmap(SCREEN_W, SCREEN_H);
   } else {
      if (bmp->w >= SCREEN_W && bmp->h >= SCREEN_H) {
         desktop = bmp;
      } else {
         desktop = desktop_memory = create_bitmap(SCREEN_W, SCREEN_H);
         for (x = 0; x < SCREEN_W; x += bmp->w) {
            for (y = 0; y < SCREEN_H; y += bmp->h) {
               blit(bmp, desktop, 0, 0, x, y, bmp->w, bmp->h);
            }
         }
      }
   }
}
