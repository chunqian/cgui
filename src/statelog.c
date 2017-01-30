#include "statelog.h"

#ifdef STATE_LOGG_ON
#include <allegro.h>
#include "../src/id.h"
#include "../src/msm.h"
#include "../src/object.h"
#include "../src/mouse.h"
#include "cgui/mem.h"
#include "cgui.h"
#define MAXLOG 10000

static char *state_log[MAXLOG];
char *mouse_state_logg_reason;
static int nlog;
static int logid;

static int state_logg_printer(void *data, char *s)
{
   strcpy(s, data);
   return 0;
}

static void refresh_state_logg(void *data nouse)
{
   BrowseToL(logid, nlog - 1);
}

static void reset_state_logg(void *data nouse)
{
   nlog = 0;
   BrowseToL(logid, 0);
}

static void *state_logg_index(void *list, int i)
{
   char **s = list;

   return s[i];
}

extern void create_mouse_state_logg_window(void)
{
   MkDialogue(ADAPTIVE, "State logg", W_FLOATING);
   AddButton(TOPLEFT, "Update list (F1)", refresh_state_logg, NULL);
   SetHotKey(0, refresh_state_logg, NULL, KEY_F1, 0);
   AddButton(RIGHT, "Clean list (F2)", reset_state_logg, NULL);
   SetHotKey(0, reset_state_logg, NULL, KEY_F2, 0);
   logid = AddList(DOWNLEFT, state_log, &nlog, 500, 0, state_logg_printer,
                             NULL, 25);
   SetIndexedList(logid, state_logg_index);
   DisplayWin();
}

extern void MouseStateTransitionDone(t_object * b, int over, int mb,
                                     t_mousedata *m nouse)
{
   static int first = 1;
   char *reason;

   reason = mouse_state_logg_reason;
   if (nlog < MAXLOG) {
      if (first == 1) {
         int i;

         first = 0;
         for (i = 0; i < MAXLOG; i++)
            state_log[i] = GetMem0(char, 100);
      }
      if (msmf1.isset) {
         if (msmf1.b == NULL)
            sprintf(state_log[nlog], "%s isset=%x error - no object",
                    reason, msmf1.isset);
         else if (msmf1.b->label && *msmf1.b->label)
            sprintf(state_log[nlog],
                    "%s isset=%x state=%d over=%d down=%d \"%s\"",
                    reason, msmf1.isset, msmf1.b->state, over, mb,
                    msmf1.b->label);
         else
            sprintf(state_log[nlog],
                    "%s isset=%x state=%d over=%d down=%d id=%d (no label)(pr)",
                    reason, msmf1.isset, msmf1.b->state, over, mb,
                    msmf1.b->id);
         if (nlog > 0
             && strncmp(state_log[nlog], state_log[nlog - 1],
                        strlen(state_log[nlog])) == 0) {
            int n = 0;

            sscanf(state_log[nlog - 1] + strlen(state_log[nlog]), "(%d)",
                   &n);
            sprintf(state_log[nlog - 1] + strlen(state_log[nlog]), "(%d)",
                    n + 1);
         } else
            nlog++;
      }
      if (msmf2.isset) {
         if (msmf2.b == NULL)
            sprintf(state_log[nlog], "*isset=%x error - no object",
                    msmf2.isset);
         else if (msmf2.b->label && *msmf2.b->label)
            sprintf(state_log[nlog],
                    "*isset=%x state=%d over=%d down=%d \"%s\"",
                    msmf2.isset, msmf2.b->state, over, mb, msmf2.b->label);
         else
            sprintf(state_log[nlog],
                    "*isset=%x state=%d over=%d down=%d id=%d",
                    msmf2.isset, msmf2.b->state, over, mb, msmf2.b->id);
         if (nlog > 0
             && strncmp(state_log[nlog], state_log[nlog - 1],
                        strlen(state_log[nlog])) == 0) {
            int n = 0;

            sscanf(state_log[nlog - 1] + strlen(state_log[nlog]), "(%d)",
                   &n);
            sprintf(state_log[nlog - 1] + strlen(state_log[nlog]), "(%d)",
                    n + 1);
         } else
            nlog++;
      }
      if (msmf1.isset == 0 && msmf2.isset == 0) {
         if (msmf1.b == NULL) {
            b = GetObject(msmf1.id);
            if (b) {
               if (b->label)
                  sprintf(state_log[nlog],
                          "%s isset=%x (state=%d %s)id=%d down=%d", reason,
                          msmf1.isset, b->state, b->label, msmf1.id, m->mb);
               else
                  sprintf(state_log[nlog], "%s isset=%x (state=%d)id=%d down=%d",
                          reason, msmf1.isset, b->state, msmf1.id, m->mb);
            } else
               sprintf(state_log[nlog], "%s isset=%x id=%d", reason,
                       msmf1.isset, msmf1.id);
         } else if (msmf1.b->label && b->label)
            sprintf(state_log[nlog],
                    "%s isset=%x state=%d over=%d down=%d \"%s\"(m) \"%s\"(pr)",
                    reason, msmf1.isset, msmf1.b->state, over, mb,
                    msmf1.b->label, b->label);
         else
            sprintf(state_log[nlog],
                    "%s isset=%x state=%d over=%d down=%d id=%d (no label)(pr)",
                    reason, msmf1.isset, msmf1.b->state, over, mb,
                    msmf1.b->id);
         if (nlog > 0
             && strncmp(state_log[nlog], state_log[nlog - 1],
                        strlen(state_log[nlog])) == 0) {
            int n = 0;

            sscanf(state_log[nlog - 1] + strlen(state_log[nlog]), "(%d)",
                   &n);
            sprintf(state_log[nlog - 1] + strlen(state_log[nlog]), "(%d)",
                    n + 1);
         } else {
            nlog++;
         }
      }
   }
}

void printlog(int x, char *s)
{char buf[100];
 static int y;
 if (s)
   sprintf(buf, "%d %s", x, s);
 else
   sprintf(buf, "%d", x);
 rectfill(screen, 400, y, 800, y+9, makecol(255, 255, 255));
 textout_ex(screen, font, buf, 400, y, makecol(0,0,0), -1);
 y+=9;
 rest(2000);
}
extern void printlog(int , char *);
//printlog(1, NULL);


#endif
