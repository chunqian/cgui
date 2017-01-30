/* Module CGKEYBRD.C

   This module contains the functions for keyboard handler.

   The keyboard system works like this: The key-presses will always reach the
   system (the "KB_isr_callback" will be invoked during the interrupt,
   inserting a message to the wrapper "KB_handler_wrapper" which transforms
   the data format to a conveniant form before transfering the key-press to
   the current. If there is no keyboard handler present the key-press will be
   dropped. At system start the hot-key handler will be installed and thus
   receive the keypresses. If an edit-box gets focus (user clicked on it),
   then the edit-box itself will install a new handler on top of the hot-key
   handler. When the edit-box closes it will uninstall the handler and future
   key-presses will go to the hot-key handler. Kb-handlers may be defined for
   any purpose, currently there are kb-handlers for hot-keys, edit-box and
   barcode.

   A Kb-handler may decide to pass keypresses on to other handlers on the
   kb-handler stack. This makes it possible e.g. to let some hot-keys override
   editing. */

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "cgkeybrd.h"
#include "memint.h"
#include "event.h"
#include "hotkey.h"
#include "cguiinit.h"

typedef struct t_kbhnd {
   /* The call back possibly taking care of a key press. */
   int (*Handler) (void *data, int scancode, int keycode);

   /* Private data for the call back */
   void *data;

   /* A pointer to next call back. */
   struct t_kbhnd *next;
} t_kbhnd;

static t_kbhnd *keyboard_handlers;

static void LaunchKeypressHandler(void)
{
   unsigned int keycode, x;
   int scan;
   t_kbhnd *kb, *next;

   keycode = ureadkey(&scan);
   for (kb = keyboard_handlers; kb; kb = next) {
      /* We must remember that the callback is allowed to uninstall itself */
      next = kb->next;
      if (kb->Handler) {        /* properly installed? */
         if (kb->Handler(kb->data, scan, keycode))   /* accepts key? */
            break;
      }
   }
}

static void ReleaseKbHandlers(t_kbhnd * kb)
{
   if (kb) {
      ReleaseKbHandlers(kb->next);
      Release(kb);
      keyboard_handlers = NULL;
   }
}

static void DeInitKeyboardInternal(void *data nouse)
{
   ReleaseKbHandlers(keyboard_handlers);
}

static void PollKeyboard(void *data)
{
   _GenEventOfCgui(PollKeyboard, data, 10, 0);
   if (keypressed()) {
      LaunchKeypressHandler();
   }
}

extern void CguiInitKeyboard(void)
{
   if (!cgui_started) {
      if (install_keyboard() == 0) {
         /* set the hot-key handler to the default handler */
         if (!keyboard_handlers)
            InstallKBHandler(HotKeyHandler, NULL);
      }
      HookCguiDeInit(DeInitKeyboardInternal, NULL);
      PollKeyboard(NULL);
   }
}

/* Application interface: This function installs a call-back function, i.e.
   the function "Handler" will be the recipient of all subsequent
   key-presses. */
extern void InstallKBHandler(int (*Handler) (void *data, int scancode, int keycode), void *data)
{
   t_kbhnd *kb;

   kb = GetMem0(t_kbhnd, 1);
   kb->next = keyboard_handlers;
   kb->data = data;
   kb->Handler = Handler;
   keyboard_handlers = kb;
}

/* This function uninstalls a Handler previously installed by
   "InstallKBHandler" */
extern void *UnInstallKBHandler(int (*Handler) (void *, int, int))
{
   t_kbhnd *kb, *prev;
   void *data;

   kb = keyboard_handlers;
   if (kb) {
      if (kb->Handler == Handler) {
         keyboard_handlers = kb->next;
         data = kb->data;
         Release(kb);
         return data;
      } else {
         prev = kb;
         for (kb = kb->next; kb && kb->Handler != Handler; kb = kb->next)
            prev = kb;
         if (kb) {
            prev->next = kb->next;
            data = kb->data;
            Release(kb);
            return data;
         }
      }
   }
   return NULL;
}
