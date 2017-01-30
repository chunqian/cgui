/* Module timer.c
   Has the responsibility to initialize and remove the timer properly. */

#include <allegro.h>
#if ALLEGRO_VERSION == 3 && ALLEGRO_SUB_VERSION == 12
#include <src/internal.h>
#else
#include <allegro/internal/aintern.h>
#endif

#include "cgui.h"
#include "cgui/mem.h"
#include "timer.h"

unsigned long _cgui_tick_counter = 0;
static int timer_pre_installed = 0;

static void TimerHandler(void)
{
   _cgui_tick_counter++;
}
END_OF_STATIC_FUNCTION(TimerHandler);

static void DeInitTimerInternal(void *data nouse)
{
   remove_int(TimerHandler);
   _cgui_tick_counter = 0;
   if (!timer_pre_installed)
      remove_timer();
}

extern void InitTimer(void)
{
   int virgin = 1;

   if (virgin) {
      virgin = 0;
      LOCK_FUNCTION(TimerHandler);
      LOCK_VARIABLE(_cgui_tick_counter);
   }
   timer_pre_installed = _timer_installed;
   if (!timer_pre_installed)
      install_timer();
   install_int(TimerHandler, 1);
   HookCguiDeInit(DeInitTimerInternal, NULL);
}
