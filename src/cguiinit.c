/* Module CGUIINIT.C
   All necessary initialization for cgui starts here */
#include <string.h>
#include <time.h>

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "memint.h"
#include "font.h"
#include "filebrow.h"
#include "cursor.h"
#include "node.h"
#include "msm.h"
#include "mouse.h"
#include "graphini.h"
#include "window.h"
#include "menuitem.h"
#include "winscr.h"
#include "cgkeybrd.h"
#include "list.h"
#include "timer.h"
#include "cguiinit.h"
#include "event.h"

#include "statelog.h"
#ifdef STATE_LOGG_ON
#include "mouse.h"
#endif

#if ALLEGRO_SUB_VERSION == 0
#define textout_ex(bmp, f, buf, x, y, col, bgcol) textout(bmp, f, buf, x, y, col)
#endif

/* The default is ASCII */
int _cgui_utf8_format_is_requested = 0;

int cgui_starting_in_progress = 0;
int cgui_ver = CGUI_VERSION_MAJOR,
    cgui_rev = CGUI_VERSION_MINOR,
    cgui_minor_rev = CGUI_VERSION_PATCH,
    cgui_release_date = CGUI_DATE;

extern void StartUpError(char *s1, char *s2)
{
   char s[1000];

   if (cgui_started) {
      sprintf(s, "%s %s|OK", s1, s2);
      Req("Message", s);
   } else {
      sprintf(s, "%s\n%s\nPress a key to continue\n", s1, s2);
      switch (cgui_starting_in_progress) {
      case 0:
         puts(s);
         getchar();
         break;
      case 1:
         puts(s);
         readkey();
         getchar();
         break;
      case 2:
         textout_ex(screen, font, s, 10, 10, 255, -1);
         readkey();
         getchar();
         break;
      }
   }
}

static void DeInitCguiIntern(void *data nouse)
{
   cgui_started = 0;
   flush_config_file();
}

#if ALLEGRO_SUB_VERSION > 0
static void DelayedCloseButtonCallback(void *data)
{
   (void)data;
   exit(0);
}

static void CloseButtonCallback(void)
{
   SafeInsertMessage(DelayedCloseButtonCallback, NULL);
}
END_OF_STATIC_FUNCTION(CloseButtonCallback);
#endif

/* Application interface: */

extern void CguiUseUTF8(void)
{
   set_uformat(U_UTF8);
   _cgui_utf8_format_is_requested = 1;
}

extern void DeInitCgui(void)
{
   DeInitChain();
}

extern int InitCguiEx(int w, int h, int cd, int *errno_ptr, int (*atexit_ptr)(void (*)(void)))
{
   if (!_cgui_utf8_format_is_requested) {
      /* To be backward compatible. */
      set_uformat(U_ASCII);
   }
   if (system_driver == NULL) {
      if (install_allegro(SYSTEM_AUTODETECT, errno_ptr, atexit_ptr) != 0) {
         printf("Error installing allegro\n");
         exit(-1);
      }
   }
   InitMemory(atexit_ptr);
   cgui_starting_in_progress = 1;
   InitTimer();
   CguiInitKeyboard();
   CguiInitEditBox();
   InitGraph(w, h, cd);
   InitFont();
   cgui_starting_in_progress = 2;
   InitTabChain();
   InitNode();
   InitList();
   InitWindows();
   InitMouse();
   InitCursor();
   InitMenuItems();
   InitMouseStateMachine();
   if (!cgui_started)
      HookCguiDeInit(DeInitCguiIntern, NULL);
   cgui_started = 1;
   MkDesktop();
   CREATE_MOUSE_STATE_LOGG_WINDOW();
   InitFileBrowser();
#if ALLEGRO_SUB_VERSION > 0
   LOCK_FUNCTION(CloseButtonCallback);
   set_close_button_callback(CloseButtonCallback);
#endif
   cgui_starting_in_progress = 0;
   return 1;
}
