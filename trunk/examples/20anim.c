/* CGUI Example program showing the usage of images as icons for various objects. */
#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

typedef struct t_frame {
   struct t_frame *next;
   BITMAP *bmp;
} t_frame;

typedef struct t_animcontrol {
   int id, ev, pause;
   t_frame *frame;
   char name[20];
   int wid;
} t_animcontrol;

static void show_code(void *data nouse)
{
   int id;
   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/20anim.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "~Close", CloseWin, NULL);
   DisplayWin();
}

static void close_example(void *data)
{
   t_animcontrol *ac = data;

   Release(ac);
   CloseWin(NULL);
   StopProcessEvents();
}

static void update_object(void *data)
{
   t_animcontrol *ac = data;

   if (!ac->pause) {
      ac->frame = ac->frame->next;
      RegisterImage(ac->frame->bmp, ac->name + 1, IMAGE_BMP, ac->wid);
      Refresh(ac->id);
   }
   ac->ev = GenEvent(update_object, ac, 100, ac->id);
}

static void make_object_window(void *data)
{
   t_frame *f = data;
   t_animcontrol *ac;
   int id;

   ac = GetMem0(t_animcontrol, 1);
   ac->frame = f;
   ac->wid = MkDialogue(ADAPTIVE, "Animated buttons", 0);
   sprintf(ac->name, "#animation%d", ac->wid);
   RegisterImage(f->bmp, ac->name + 1, IMAGE_BMP, ac->wid);
   AddTextBox(DOWNLEFT, "This example shows how to make animations on an object. "
   "Maybe you don't need that very often, but anyway: when you need it will not "
   "be any problem._ _Press the button to get a new window", 400, 0, TB_FRAMESINK|TB_LINEFEED_);
   MkRadioContainer(DOWNLEFT, &ac->pause, R_HORIZONTAL);
   AddRadioButton("Run");
   AddRadioButton("Stop");
   EndRadioContainer();
   ac->id = AddButton(DOWNLEFT, ac->name, make_object_window, f);
   AddButton(DOWNLEFT, "\33Close", close_example, ac);
   update_object(ac);
   DisplayWin();
   ProcessEvents();
}

static void free_anim(t_frame *f)
{
   if (f) {
      free_anim(f->next);
      Release(f);
   }
}

int main(void)
{
   DATAFILE *datl, *datob;
   t_frame *fs = NULL, *f, *end = NULL;
   int i, n, ok = 0;

   InitCgui(1024, 768, 15);
   datob = load_datafile_object("examples.dat#running", "pallette");
   if (datob) {
      if (datob->type == DAT_PALETTE) {
         set_palette(datob->dat);
         unload_datafile_object(datob);
         datl = load_datafile("examples.dat#running");
         if (datl) {
            for (n=0; datl[n].type==DAT_BITMAP; n++)
               ;
            if (n) {
               for (i=n-1; datl[i].type==DAT_BITMAP; i--) {
                  f = GetMem(t_frame, 1);
                  f->bmp = datl[i].dat;
                  f->next = fs;
                  fs = f;
                  if (end == NULL)
                     end = f;
               }
               end->next = fs;
               make_object_window(fs);
               end->next = NULL;
               free_anim(fs);
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
