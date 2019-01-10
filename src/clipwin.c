#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cgui/mem.h"
#include "cgui.h"
#include "cgui/clipwin.h"
#include "allegro.h"

#ifdef DJGPP
#ifndef CGUI_SCAN_DEPEND
#include <dpmi.h>
#include <sys/movedata.h>
#endif
#define RM_OFF(m) ( (m) & 0x0f )
#define RM_SEG(m) ( ((m)>>4) & 0xffff )
#define MSW(dw) ( (dw) >> 16 )
#define LSW(dw) ( (dw) & 0xffff )

static unsigned AllocDosMem(int *size)
{
   unsigned dosmem;
   __dpmi_regs r;

   if (*size > 0x100000)        /* "640k will be enough in all the future
                                   ..." */
      *size = 0x100000;
   do {
      /* set dos function */
      r.h.ah = 0x48;
      /* dos mem size is allocated as number of paragraphs */
      r.x.bx = *size / 16;
      if (*size & 0xF)
         r.x.bx++;
      __dpmi_int(0x21, &r);
      if ((r.x.flags & 1) == 0)
         break;
      *size -= 1000;
   } while (1);                 /* while size is too large */
   dosmem = (unsigned long) r.x.ax * 16;
   return dosmem;
}

static void FreeDosMem(unsigned dosmem)
{
   __dpmi_regs r;

   r.h.ah = 0x49;
   r.x.es = dosmem / 16;
   __dpmi_int(0x21, &r);
}

/* Opens Windows Clip-board, i.e.check if clipboard is locked by another user
 */
static int OpenWinClip(void)
{
   __dpmi_regs r;

   r.x.ax = 0x1701;
   __dpmi_int(0x2F, &r);
   return r.x.ax;
}

/* Close Windows Clipboard */
static void CloseWinClip(void)
{
   __dpmi_regs r;

   r.x.ax = 0x1708;
   __dpmi_int(0x2F, &r);
}

/* Get size of clipboard content, function 1704h, texttype: DX=1 Return
   value: DX:AX is LSW */
static int WinClipTextLen(void)
{
   __dpmi_regs r;

   r.x.ax = 0x1704;
   r.x.dx = 1;
   __dpmi_int(0x2F, &r);
   return ((unsigned) r.x.dx << 16) + r.x.ax;
}

/* Get the current text-content from the Windows Clipboard. Returns a pointer
   to allocated memory containing the text. If failed or if clipboard is
   empty, a NULL-pointer will be returned. Due to DOS limitations, the
   maximum buffer to copy is 640k and in practice often much less. NOTE!
   caller is responsible to free the allocated memory */
extern char *GetFromWinClip(void)
{
   char *str = NULL;
   unsigned dosmem;
   int len, newsize;
   __dpmi_regs r;

   if (!OpenWinClip())
      return NULL;
   len = WinClipTextLen();
   if (len) {
      newsize = len;
      dosmem = AllocDosMem(&newsize);
      /* must find a way to reduce the clipboard size if not enough dosmem */
      if (dosmem && len == newsize) {
         str = GetMem(char, len + 1);

         if (str) {
            /* move from clipboard to DOS-memory, function 1705h, text: dx=1
               rm-address in ES:BX */
            r.x.es = RM_SEG(dosmem);
            r.x.bx = RM_OFF(dosmem);
            r.x.ax = 0x1705;
            r.x.dx = 1;
            __dpmi_int(0x2F, &r);
            /* copy the data from the DOS memory to the p.m. string */
            dosmemget(dosmem, len, str);
            str[len] = 0;       /* if size is reduced, the data may not be
                                   0-terminated */
         }
         FreeDosMem(dosmem);
      }
   }
   CloseWinClip();
   return str;
}

/* Inserts the string str into the Windows clipboard, by way of some DOS
   memory. If DOS-memory is not large enough to receive str, as much as
   possible will be copied. Returns 1 on sucess, otherwise 0 */
extern int InsertIntoSystemsClipboard(char *str)
{
   __dpmi_regs r;
   unsigned dosmem;
   int size;
   char c;

   if (!OpenWinClip())
      return 0;
   size = strlen(str) + 1;
   dosmem = AllocDosMem(&size);
   if (dosmem) {
      /* copy string to DOS-memory */
      c = str[size - 1];
      str[size - 1] = 0;        /* if size is reduced we must also terminate
                                   the string */
      dosmemput(str, size, dosmem);
      str[size - 1] = c;
      /* copy from DOS-memory to clipboard, function 1700h, rm-address in
         ES:BX, size in SI:CX */
      r.x.ax = 0x1703;
      r.x.dx = 1;
      r.x.es = RM_SEG(dosmem);
      r.x.bx = RM_OFF(dosmem);
      r.x.si = MSW(size);
      r.x.cx = LSW(size);
      __dpmi_int(0x2F, &r);
      FreeDosMem(dosmem);
      if (r.x.ax == 0) {        /* some error */
         CloseWinClip();
         return 0;
      }
   }
   CloseWinClip();
   return 1;
}

/* Returns 0 if if windows clipboard is not available. Return value 1 means
   clipboard available but currently empty from text Return value 3 means
   clipboard available and contains text */
extern int CheckWinClip(void)
{
   __dpmi_regs r;

   /* Function 1700h - identify windows clipboard */
   r.x.ax = 0x1700;
   __dpmi_int(0x2F, &r);
   if (r.x.ax == 0x1700)
      return 0;
   else if (OpenWinClip()) {
      if (WinClipTextLen())
         return 3;
      else
         return 1;
      CloseWinClip();
   } else
      return 0;
}
#endif
#ifdef ALLEGRO_UNIX
#include <limits.h>
#include <X11/Xlib.h>

typedef struct clipX {
  Display *display;
  Window window;
} clipX;

void close_X_clipboard(clipX *xclip)
{
  XDestroyWindow(xclip->display, xclip->window);
  XCloseDisplay(xclip->display);
  free(xclip);
}

static char *get_X_copy_buffer(clipX *xclip, const char *buffer)
{
   char *result;
   char *s;
   unsigned long ressize, restail;
   int resbits;
   Atom bufid = XInternAtom(xclip->display, buffer, False);
   Atom fmtid = XInternAtom(xclip->display, "UTF8_STRING", False);
   Atom propid = XInternAtom(xclip->display, "XSEL_DATA", False);
   Atom incrid = XInternAtom(xclip->display, "INCR", False);
   XEvent event;

   XConvertSelection(xclip->display, bufid, fmtid, propid, xclip->window, CurrentTime);
   do {
      XNextEvent(xclip->display, &event);
   } while (event.type != SelectionNotify || event.xselection.selection != bufid);

   if (event.xselection.property) {
      XGetWindowProperty(xclip->display, xclip->window, propid, 0, LONG_MAX/4, False, AnyPropertyType,
      &fmtid, &resbits, &ressize, &restail, (unsigned char**)&result);

      if (fmtid != incrid) {
         s = strdup(result);
      } else {
         s = strdup("");
      }
      XFree(result);
   } else {
      s = strdup("");
   }
   return s;
}

char *get_X_clipboard(clipX *xclip)
{
   return get_X_copy_buffer(xclip, "CLIPBOARD");
}

char *get_X_primary(clipX *xclip)
{
   return get_X_copy_buffer(xclip, "PRIMARY");
}

clipX * init_X_clipboard(void)
{
   clipX *xclip = calloc(sizeof(clipX), 1);
   xclip->display = XOpenDisplay(NULL);
   unsigned long color = BlackPixel(xclip->display, DefaultScreen(xclip->display));
   xclip->window = XCreateSimpleWindow(xclip->display, DefaultRootWindow(xclip->display), 0,0, 1,1, 0, color, color);
   return xclip;
}

int InsertIntoSystemsClipboard(const char *selection)
{
   char *cmd;
   int ok = 0;
   if (selection) {
      cmd = msprintf("echo -n \"%s\" | xsel -b &", selection);
      ok = !system(cmd);
      free(cmd);
   }
   return ok;
}

int InsertIntoPrimaryBuffer(const char *selection)
{
   char *cmd;
   int ok = 0;
   if (selection) {
      cmd = msprintf("echo -n \"%s\" | xsel -p &", selection);
      ok = !system(cmd);
      free(cmd);
   }
   return ok;
}

#else
#include "cgui.h"

extern int InsertIntoSystemsClipboard(char *str) {return str!=NULL;}
extern int InsertIntoPrimaryBuffer(const char *selection) {return selection != NULL;}
extern char *GetFromWinClip(void) {return NULL;}
extern int CheckWinClip(void) {return 0;}
#endif
