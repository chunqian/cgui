/* Module WINREQ.C
   This file contains functions for creating request windows A "request
   window" means that the window-creating function does not immediately
   return. It will wait until the user has made some selection in the window,
   then closes the window and return the selected value. During this time the
   event-processing will continue. This is simply solved by just invoking a
   new q processor. The interface for doing this is generalised to a
   "meeting". Inviting to a meeting will start a new event-q processor, which
   will pass the given data-pointer to anyone that attends the meeting with
   the right identification key. The attending one may do whatever with the
   data-pointer. One can only attend the meeting at top */

#include <allegro.h>
#include <string.h>

#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "object.h"
#include "obbutton.h"
#include "node.h"
#include "id.h"
#include "setpos.h"
#include "cguiinit.h"
#include "memint.h"
#include "winreq.h"

#define REQ_WIDTH    240

typedef struct t_requester {
   int no;
   struct t_requester *r;
} t_requester;

static void CancelReply(void *msg)
{
   t_requester *r = msg;
   int *no;

   no = Attend(0x80000000);
   *no = r->no;
   Release(r->r);
   _CguiCancel();
}

/* Not to be read... */
static void OutButtons(char *text, int window_width)
{
   char *p, *s, *ss;
   int n, i, available_width, init_width, button_width, id, bars;
   t_requester *r;
   t_object *b;
   ss = GetMem(char, strlen(text) + 1);

   for (p = text, n = 0; *p; p++) {
      if (*p == '|') {
         for (bars = 0; *p == '|'; p++)
            bars++;
         n += bars & 1;
      }
   }
   StartContainer(DOWNLEFT|ALIGNCENTRE, ADAPTIVE, "", 0);
   r = GetMem(t_requester, n);
   init_width = window_width - DEFAULT_WINFRAME - 2 - opwin->win->opnode->leftx - opwin->win->opnode->rightx;
   available_width = init_width;
   for (i = 0, p = text + 1; *p; i++) {
      s = ss;
      while (*p) {
         if (*p == '|') {
            for (bars = 0; *p == '|'; p++) {
               bars++;
               if ((bars & 1) == 0)
                  *s++ = '|';
            }
            if (bars & 1)
               break;
         }
         *s++ = *p++;
      }
      *s = 0;
      if (*ss) {
         r[i].no = i;
         r[i].r = r;

         /* Put first button at top left corner in the buttons container, and
         subsequent ones to the right (in a row). If the with of all buttons
         in that row exceeds the specified width, then start a new row. */
         if (i == 0) {
            id = AddButton(TOPLEFT | EQUALHEIGHT, ss, CancelReply, r + i);
         } else {
            id = AddButton(RIGHT | EQUALHEIGHT, ss, CancelReply, r + i);
         }
         b = GetObject(id);
         SetPushButtonSize(b);
         button_width = b->x2;
         available_width -= button_width;
         if (i > 0) {
            available_width -= opwin->win->opnode->xdist;
         }
         if (available_width < 0) {
            /* There was no horizontal space left for this button, start with
            a new row of buttons. */
            SetPosition(b, DOWNLEFT);
            available_width = init_width - button_width;
         }
      } else {
         i--;                   /* skip empty button */
      }
   }
   EndContainer();
   Release(ss);
}

/* Creates a standard dialog window, using the info found in 's'. This shuld be
   some (optional) leadning infor-text, used to create a text-box. After that
   text there shoul be at least one button specification, i.e. a text beginning
   with a '|' character. The buttons will all be put on a stright line at below
   of the info-text */
static int CreateRequster(const char *win_title, int options, int width, const char *s)
{
   char *p, *cp;
   int id;

   if (width <=0)
      width = REQ_WIDTH;
   cp = GetMem(char, strlen(s) + 1);
   strcpy(cp, s);
   p = cp;
   while (*p != '|' && *p)
      p++;
   if (*p) {
      *p = 0;
      id = MkDialogue(ADAPTIVE, win_title, options&W_CENTRE);
      if (id == 0) {
         Release(cp);
         return 0;
      }
      AddTextBox(TOPLEFT | FILLSPACE, cp, width, 0, TB_LINEFEED_);
      *p = '|';
      OutButtons(p, width);
      DisplayWin();
      Release(cp);
      return 1;
   }
   Release(cp);
   return 0;
}

static int required_buffer_size(const char *format, va_list ap)
{
   const char *p,*s;
   int x, Ls, ls, precision, field_width, done, len;

   len = strlen(format)+1;//+1 for terminating NULL
   p = format;
   while (*p) {
      if (*p == '%') {
         precision = 0;
         field_width = 0;
         x = 0;
         p++;
         if (*p == '%') {
            p++;
         } else {
            done = 0;
            do {
               switch (*p) {
               case '-':
               case '+':
               case ' ':
               case '#':
               case '0':
                  p++;
                  break;
               default:
                  done = 1;
                  break;
              }
           } while (!done);

           if (*p == '*') {
              p++;
              field_width = va_arg(ap, int);
              if (field_width < 0) {
                 field_width = -field_width;
              }
           } else if ((*p >= '0') && (*p <= '9')) {
              field_width = 0;
              do {
                 field_width *= 10;
                 field_width += *p - '0';
                 p++;
              } while ((*p >= '0') && (*p <= '9'));
           }

           if (*p == '.')
              p++;

           if (*p == '*') {
                p++;
                precision = va_arg(ap, int);
                if (precision < 0)
                   precision = 0;
           } else if ((*p >= '0') && (*p <= '9')) {
               precision = 0;
               do {
                  precision *= 10;
                  precision += *p - '0';
                  p++;
               } while ((*p >= '0') && (*p <= '9'));
           }

           done = 0;
           ls = 0;
           Ls = 0;
           do {
              switch (*p) {
              case 'h':
                 p++;
                 break;
              case 'l':
                 ls++;
                 p++;
                 break;
              case 'L':
                 Ls++;
                 p++;
                 break;
              default:
                 done = 1;
                 break;
              }
           } while (!done);

            switch (*p) {
            case 'c':
               va_arg(ap, int);
               x = 1;
               p++;
               break;
            case 'd':
            case 'i':
            case 'o':
            case 'u':
            case 'x':
            case 'X':
               /* Just take the longest possible but be careful about the arg */
               if (Ls || ls > 1) {
                  va_arg(ap, LONG_LONG);
                  x = 20;
               } else if (ls == 1) {
                  va_arg(ap, long);
                  x = 9;
               } else {
                  va_arg(ap, int);
                  x = 9;
               }
               p++;
               break;
            case 'D':
            case 'U':
               va_arg(ap, long);
               x = 9;
               p++;
               break;
            case 'e':
            case 'E':
            case 'f':
            case 'g':
            case 'G':
               if (ls || Ls) {
                  va_arg(ap, long double);
               } else {
                  va_arg(ap, double);
               }
               x = 20;
               p++;
               break;
            case 'n':
               va_arg(ap, int*);
               x = 0;
               p++;
               break;
            case 'p':
               va_arg(ap, void*);
               x = 18;
               p++;
               break;
            case 's':
            s=va_arg(ap, char *);
               if(s)
         		x = strlen(s);
            else
         		x=strlen("(null)");
               p++;
               break;
            default:
               /* Bad format */
               break;
            }
         }

         x -= 2; /* At least 2 chars for spec */

         /* These are overestimations */
         x++; /* For sign, in case ..*/
         x += precision;
         x += field_width;

         len += x;
      } else {
         p++;
      }
   }
   return len;
}

/* Application interface: */

extern char *msprintf(const char *format, ...)
{
   char *buf;
   int n;
   va_list ap;

   va_start(ap, format);
   n = required_buffer_size(format, ap);
   va_end(ap);
   buf = GetMem(char, n);
   va_start(ap, format);
   vsprintf(buf, format, ap);
   va_end(ap);
   return buf;
}

extern int Request(const char *title, int options, int width, const char *format, ...)
{
   char *buf;
   int n;
   va_list ap;
   int reply_number, ok = 0, was_started;

   va_start(ap, format);
   n = required_buffer_size(format, ap);
   va_end(ap);
   buf = GetMem(char, n);
   va_start(ap, format);
   vsprintf(buf, format, ap);
   va_end(ap);
   was_started = cgui_started;
   if (!cgui_started) {
      if (cgui_starting_in_progress)
         exit(0);
      InitCgui(0, 0, 0);
   }
   if (*buf)
      ok = CreateRequster(title, options, width, buf);
   SelectCursor(CURS_DEFAULT);
   if (!ok)
      ok = CreateRequster("Error", 0, 0, "Internal: No texts available|S~top");
   if (ok)
      Invite(0x80000000, &reply_number, "Request");
   if (!was_started)
      DeInitCgui();

   Release(buf);
   if (!ok)
      return 0;
   return reply_number;
}

extern int Req(const char *title, const char *s)
{
   return Request(title, 0, 0, s);
}
