/* CGUI Example program showing how to use the browsing object. */
#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

typedef struct {
   int viewpos;
   int prevpos;
   int id;
   int scrolled_area_length;
   int view_port_length;
   struct t_canvas *c;
} t_browser;

typedef struct t_canvas {
   BITMAP *bmp;
   t_browser v, h;
   int id;
} t_canvas;

#define VIEW_HEIGHT 90
#define VIEW_WIDTH  100
#define RESOLUTION  1

static void show_code(void *data nouse)
{
   int id;
   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/24browse.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "~Close", CloseWin, NULL);
   DisplayWin();
}

static void close_example(void *data nouse)
{
   CloseWin(NULL);
   StopProcessEvents();
}

static void browser_call_back(void *data)
{
   t_browser *b=data;
   Refresh(b->c->id); /* Re-draw the canvas */
}

/* This is what is needed for making any object scrollable. One might think
   that it is unnessesary to set the size separately, but if you look at
   the re-sizeable version you will understand why. */
static void AddBrowser(int x, int y, t_canvas *c, t_browser *b, int sc_area_len, int vplen, int isvertical)
{
   b->viewpos = 0;
   b->c = c;
   b->view_port_length = vplen;
   b->scrolled_area_length = sc_area_len;
   if (isvertical)
      b->id = MkVerticalBrowser(x, y, browser_call_back, b, &b->viewpos);
   else
      b->id = MkHorizontalBrowser(x, y, browser_call_back, b, &b->viewpos);
   SetBrowserSize(b->id, vplen, vplen);
   NotifyBrowser(b->id, RESOLUTION, sc_area_len);
}

static void canvas_event_handler(BITMAP *bmp, int x, int y, void *data)
{
   t_canvas *canvas=data;
   if (y < 0 || x < 0) { /* same action for init and refresh */
      clear_to_color(bmp, makecol(255,255,255)); /* Needed if image is smaller than canvas */
      blit(canvas->bmp, bmp, canvas->h.viewpos,
           canvas->v.viewpos, 0, 0, canvas->h.view_port_length, canvas->v.view_port_length);
   } /* else: no mouse event implemented */
}

/* === The below functions shows how to browse in a "view port" of fixed size === */

static void make_fix_size_canvas_window(BITMAP *bmp)
{
   static t_canvas c;

   c.bmp = bmp;
   MkDialogue(ADAPTIVE, "Simple browsing", 0);
   AddTextBox(DOWNLEFT, "This example shows how to use the browsing object.",
      400, 0, TB_FRAMESINK|TB_LINEFEED_);
   c.id = MkCanvas(DOWNLEFT, VIEW_WIDTH, VIEW_HEIGHT, canvas_event_handler, &c);
   AddBrowser(RIGHT, &c, &c.v, bmp->h, VIEW_HEIGHT, 1);
   AddBrowser(DOWNLEFT, &c, &c.h, bmp->w, VIEW_WIDTH, 0);
   AddButton(DOWNLEFT, "\33Exit", close_example, NULL);
   AddButton(RIGHT, "Show code", show_code, NULL);
   DisplayWin();
}

/* === The below functions shows how to browse in a "view port" that can be re-sized === */

/* Inform the browsers about the new size of the "view port". Here we
   prefere the browsers to have the same size as the canvas, so 2:nd
   parameter is the same as 1:st.
   We also need to save the new "view port" size for our own usage. */
static void resize_call_back(void *data)
{
   t_canvas *canvas=data;
   int w,h;

   /* Get the change. */
   GetSizeOffset(canvas->id, &w, &h);
   canvas->v.view_port_length = h + VIEW_HEIGHT;
   SetBrowserSize(canvas->v.id, canvas->v.view_port_length, canvas->v.view_port_length);
   canvas->h.view_port_length = w + VIEW_WIDTH;
   SetBrowserSize(canvas->h.id, canvas->h.view_port_length, canvas->h.view_port_length);
}

static void make_resizable_canvas_window(BITMAP *bmp)
{
   static t_canvas c;

   c.bmp = bmp;
   MkDialogue(ADAPTIVE, "Simple browsing", W_FLOATING);
   AddTextBox(DOWNLEFT, "This example shows how to use the browsing object with a re-sizable object.",
      200, 0, TB_FRAMESINK|TB_LINEFEED_);
   c.id = MkCanvas(DOWNLEFT, VIEW_WIDTH, VIEW_HEIGHT, canvas_event_handler, &c);
   MakeStretchable(c.id, resize_call_back, &c, 0);
   AddBrowser(RIGHT, &c, &c.v, bmp->h, VIEW_HEIGHT, 1);
   AddBrowser(DOWNLEFT, &c, &c.h, bmp->w, VIEW_WIDTH, 0);
   AddButton(DOWNLEFT, "\33Exit", close_example, NULL);
   AddButton(RIGHT, "Show code", show_code, NULL);
   DisplayWin();
}

/*  `main* loads and stretches a bitmap, just to have something to view. */
int main(void)
{
   DATAFILE *datl, *datob;
   BITMAP *orig, *bmp;
   int ok = 0;

   InitCgui(1024, 768, 15);
   datob = load_datafile_object("examples.dat#running", "pallette");
   if (datob) {
      if (datob->type == DAT_PALETTE) {
         set_palette(datob->dat);
         unload_datafile_object(datob);
         datl = load_datafile("examples.dat#running");
         if (datl) {
            if (datl[0].type==DAT_BITMAP) {
               orig = datl[0].dat;
               bmp = create_bitmap(orig->w*5, orig->h*5);
               stretch_blit(orig, bmp, 0, 0, orig->w, orig->h, 0, 0, orig->w*5, orig->h*5);
               make_fix_size_canvas_window(bmp);
               make_resizable_canvas_window(bmp);
               ProcessEvents();
               destroy_bitmap(bmp);
               ok = 1;
            }
            unload_datafile(datl);
         }
      }
   }
   if (!ok)
      Req("", "Error when reading the animation data| OK ");
   return 0;
}
END_OF_MAIN()
