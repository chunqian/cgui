/* Module CLOCK.C
   Contains the functions for creating a periodically updated watch */

#include <string.h>
#include <allegro.h>

#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "obheader.h"
#include "font.h"
#include "id.h"
#include "labldata.h"
#include "clock.ht"
#include "event.h"
#ifdef DJGPP
#ifndef CGUI_SCAN_DEPEND
#include <dos.h>
#endif
#endif

#define UPD_CLOCK_FLAG  0x10000000

typedef struct t_clock {
   int evid;
   struct tm t;
   char s[10];
   int dispsecs;
} t_clock;

typedef struct t_clockv {
   int id;
   char *s;
   t_clock *cl;
} t_clockv;

typedef struct t_cldial {
   t_clockv *cv;
   struct tm t;
   int dispsecs;
} t_cldial;

static char *section_id = "Cgui-Clock";
static char *item_show_sec = "Show-sec";

static void UpdateWatch(int id nouse, void *data, void *calldata nouse, int reason)
{
   t_clockv *cv = data;

   if (UPD_CLOCK_FLAG & reason)
      Refresh(cv->id);
}

static void TimeUpdate(void *data)
{
   t_clock *cl = data;
   time_t nt = 0;
   struct tm t;
   int chg;

   time(&nt);
   t = *localtime(&nt);
   if (cl->dispsecs)
      chg = cl->t.tm_sec != t.tm_sec;
   else
      chg = cl->t.tm_min != t.tm_min;
   if (chg) {
      cl->t = t;
      if (cl->dispsecs)
         sprintf(cl->s, "%2d:%02d:%02d", (int) cl->t.tm_hour,
                       (int) cl->t.tm_min, (int) cl->t.tm_sec);
      else
         sprintf(cl->s, "%2d:%02d", (int) cl->t.tm_hour, (int) cl->t.tm_min);
      ConditionalRefresh(NULL, UPD_CLOCK_FLAG);
   }
   cl->evid = _GenEventOfCgui(TimeUpdate, cl, 1000, 0);
}

static void Time2Text(void *data, char *s)
{
   t_clockv *cv = data;

   strcpy(s, cv->s);
}

static void ExitWatch(void *data)
{
   t_clockv *cv = data;

   Release(cv);
}

static void CancelDialogueWatch(void *data)
{
   t_cldial *cld = data;

   Release(cld);
   CloseWin(NULL);
}

#ifdef DJGPP
static void SetNewTime(struct tm *nt)
{
   struct time ti;

   ti.ti_min = nt->tm_min;
   ti.ti_sec = nt->tm_sec;
   ti.ti_hour = nt->tm_hour;
   ti.ti_hund = 0;
   settime(&ti);
}
#else
#define SetNewTime(x)
#endif

static void ViewChecked(void *data)
{
   t_cldial *cld = data;
   t_clockv *cv;

   cv = cld->cv;
   cv->cl->dispsecs = cld->dispsecs;
   set_config_int(section_id, item_show_sec, cv->cl->dispsecs);
   flush_config_file();
   cv->cl->t.tm_sec = cv->cl->t.tm_min = -1;
   _KillEventOfCgui(cv->cl->evid);
   cv->cl->evid = 0;
   TimeUpdate(cv->cl);
}

static void ApplyNewTime(void *data)
{
   t_cldial *cld = data;
   t_clockv *cv;

   cv = cld->cv;
   SetNewTime(&cld->t);
   cv->cl->t.tm_sec = cv->cl->t.tm_min = -1;
   _KillEventOfCgui(cv->cl->evid);
   cv->cl->evid = 0;
   TimeUpdate(cv->cl);
}

static void DialogueWatch(void *data)
{
   t_clockv *cv = data;
   t_cldial *cld;
   int id, nr;
   const char *const*txt;

   txt = LoadCompiledTexts(cgui_labels+CGUI_CGUILABL, "clock", &nr);
   if (nr < SIZE_clock)
      return;
   cld = GetMem0(t_cldial, 1);
   cld->t = cv->cl->t;
   cld->cv = cv;
   cld->dispsecs = cv->cl->dispsecs;
   MkDialogue(ADAPTIVE, txt[ADJUST_CLOCK_SETTINGS], 0);
   id = StartContainer(TOPLEFT, ADAPTIVE, "", 0);
   AddEditBox(RIGHT, 30, "", FINT, 2, &cld->t.tm_hour);
   AddEditBox(RIGHT, 30, "", FINT, 2, &cld->t.tm_min);
   AddEditBox(RIGHT, 30, "", FINT, 2, &cld->t.tm_sec);
   AddButton(RIGHT, txt[APPLY], ApplyNewTime, cld);
   EndContainer();
#ifndef DJGPP
   DeActivate(id);
#endif
   id = AddCheck(DOWNLEFT, txt[SHOW_SECONDS], &cld->dispsecs);
   AddHandler(id, ViewChecked, cld);
   AddButton(DOWNLEFT, txt[CLOSE], CancelDialogueWatch, cld);
   DisplayWin();
}

static void StopWatch(void *data)
{
   t_clock *cl = data;
   _KillEventOfCgui(cl->evid);
   cl->evid = 0;
   Release(cl);
}

extern int AddClock(int x, int y, int options)
{
   static t_clock *cl = NULL;
   t_clockv *cv;
   t_object *b;

   if (cl == NULL) {
      cl = GetMem0(t_clock, 1);
      cl->evid = _GenEventOfCgui(TimeUpdate, cl, 0, 0);
      cl->t.tm_sec = cl->t.tm_min = -1;
      HookCguiDeInit(StopWatch, cl);
   }

   cl->dispsecs = get_config_int(section_id, item_show_sec, 0);
   cv = GetMem0(t_clockv, 1);
   cv->cl = cl;
   cv->s = cl->s;
   cv->id = AddStatusField(x, y, text_length(_cgui_prop_font, "28:88:88"),
                             Time2Text, cv);
   b = GetObject(cv->id);
   if ((options & CLOCK_NO_DIALOGUE) == 0) {
      b->tf->Activate(b);
      b->click = LEFT_MOUSE|RIGHT_MOUSE;
      AddHandler(cv->id, DialogueWatch, cv);
   }
   HookExit(cv->id, ExitWatch, cv);
   RegisterRefresh(cv->id, UpdateWatch, cv);
   return cv->id;
}
