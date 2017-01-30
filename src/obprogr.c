/* Module OBPROGR.C
   Contains the functions for creating a progress bars */

#include <allegro.h>

#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "obheader.h"
#include "id.h"

#define FRAMEW 1

typedef struct t_progressbar {
   int w;
   int h;
   int per;
   int x;
   /* For additional handler */
   void (*Action) (void *);
   void *data;
} t_progressbar;

typedef struct t_progresswin {
   t_object *b;
   t_progressbar *pb;
   int idst;
} t_progresswin;

static unsigned transfer_buffer, data_available;

static void ProgressDraw(t_object *b)
{
   t_progressbar *pb;
   int x;

   pb = b->appdata;
   if (b->parent->bmp) {
      DrawThinFrame(b->parent->bmp, b->x1, b->y1, b->x2, b->y2);
      if (pb->x <= 0) {
         rectfill(b->parent->bmp, b->x1 + FRAMEW, b->y1 + FRAMEW, b->x2 - FRAMEW, b->y2 - FRAMEW, cgui_colors[CGUI_COLOR_PROGRESSBAR_BACKGROUND]);
      } else {
         x = b->x1 + pb->x + FRAMEW;
         if (x >= b->x2 - FRAMEW) {
            rectfill(b->parent->bmp, b->x1 + FRAMEW, b->y1 + FRAMEW, b->x2 - FRAMEW, b->y2 - FRAMEW, cgui_colors[CGUI_COLOR_PROGRESSBAR]);
         } else {
            rectfill(b->parent->bmp, b->x1 + FRAMEW, b->y1 + FRAMEW, x - 1, b->y2 - FRAMEW, cgui_colors[CGUI_COLOR_PROGRESSBAR]);
            rectfill(b->parent->bmp, x, b->y1 + FRAMEW, b->x2 - FRAMEW, b->y2 - FRAMEW, cgui_colors[CGUI_COLOR_PROGRESSBAR_BACKGROUND]);
         }
      }
   }
}

static void SetSize(t_object *b)
{
   t_progressbar *pb;

   pb = b->appdata;
   if (b->dire)
      b->x1 = b->y1 = 0;
   b->x2 = b->x1 + pb->w - 1 + FRAMEW * 2 + b->rex;
   b->y2 = b->y1 + pb->h - 1 + FRAMEW * 2 + b->rey;
}

static int AddPrHandler(t_object *b, void (*cb) (void *), void *data)
{
   t_progressbar *pb;

   pb = b->appdata;
   pb->Action = cb;
   pb->data = data;
   return 1;
}

static int FetchData(t_object *b)
{
   t_progressbar *pb;
   int prev_x, x1, x2;

   if (data_available) {
      data_available = 0;
      if (transfer_buffer > 100U)
         return 0;
      pb = b->appdata;
      pb->per = transfer_buffer;
      prev_x = pb->x;
      pb->x = pb->w * pb->per / 100;
      if (prev_x < pb->x) {
         x1 = prev_x+1;
         x2 = pb->x;
      } else {
         x2 = prev_x+1;
         x1 = pb->x;
      }
      SetBlitLimit(x1, FRAMEW, x2, b->y2 - b->y1 - FRAMEW);
      b->tf->Refresh(b);
      return 1;
   }
   return 0;
}

static void ActionWrapper(void *data)
{
   t_progressbar *pb = data;

   if (pb->Action)
      pb->Action(pb->data);
}

/* Adds a progress bar. Return value is the id of the object. The current
   value can be updated by a call to UpdateProgressBar. */
extern int AddProgressBar(int x, int y, int w, int h)
{
   t_object *b;
   t_progressbar *pb;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      tf = default_type_functions;
      tf.Draw = ProgressDraw;
      tf.Free = XtendedFree;
      tf.SetSize = SetSize;
      tf.AddHandler = AddPrHandler;
      tf.FetchData = FetchData;
   }
   b = CreateObject(x, y, opwin->win->opnode);
   pb = GetMem0(t_progressbar, 1);
   pb->w = w;
   pb->h = h;
   b->tf->DeActivate(b);
   b->click = 0;
   b->Action = ActionWrapper;
   b->appdata = pb;
   b->tf = &tf;
   return b->id;
}

/* Updates the value of the progress bar `id'. */
extern int UpdateProgressValue(int id, int percent)
{
   t_object *b;
   int ok;

   b = GetObject(id);
   if (b) {
      transfer_buffer = percent;
      data_available = 1;
      ok = b->tf->FetchData(b);
      transfer_buffer = 0;
      if (data_available)
         data_available = 0;
      else
         return ok;
   }
   return 0;
}

/* The progress window */

static int FetchDataWinProgress(t_object *b)
{
   t_progresswin *pwin;

   if (data_available) {
      pwin = b->appdata;
      if (pwin->b)
         pwin->b->tf->FetchData(pwin->b);
      Refresh(pwin->idst);
      return 1;
   }
   return 0;
}

static void UpdateStatus(void *data, char *s)
{
   t_progresswin *pwin = data;

   if (pwin->pb)
      sprintf(s, "%d %%", pwin->pb->per);
}

static void DestroyPwin(void *data)
{
   t_progresswin *pwin = data;

   Release(pwin);
}

/* A complete progress-bar window. Return value is the id of the object.
   The current value can be updated by a call to UpdateProgressBar.
   option - PRG_STOP_BUTTON */
extern int MkProgressWindow(const char *wlabel, const char *blabel, int w)
{
   int id, idw;
   t_object *b, *bw;
   t_progresswin *pwin;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      tf = default_window_type_functions;
      tf.FetchData = FetchDataWinProgress;
   }
   idw = MkDialogue(ADAPTIVE, wlabel, 0);
   bw = GetObject(idw);
   if (bw == NULL)
      return idw;
   pwin = GetMem0(t_progresswin, 1);
   bw->appdata = pwin;
   bw->tf = &tf;
   pwin->idst = AddStatusField(DOWNLEFT, 30, UpdateStatus, pwin);
   id = AddProgressBar(RIGHT, w, 20);
   b = GetObject(id);
   if (b) {
      pwin->pb = b->appdata;
      pwin->b = b;
   }
   if (*blabel)
      AddButton(DOWNLEFT, blabel, CloseWin, NULL);
   DisplayWin();
   HookExit(idw, DestroyPwin, pwin);
   return idw;
}
