#ifdef DJGPP

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cgui/mem.h"
#include "cgui/clipwin.h"

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
extern int InsertIntoWinClip(char *str)
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
#else
#include "cgui.h"
extern int InsertIntoWinClip(char *str) {return str!=NULL;}
extern char *GetFromWinClip(void) {return NULL;}
extern int CheckWinClip(void) {return 0;}
#endif
