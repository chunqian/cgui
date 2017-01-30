/* Module HOTKEY
   Contains functions for the hot-keys
*/

#include <ctype.h>
#include <allegro.h>

#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "hotkey.h"
#include "hotkeyau.h"
#include "tabchain.h"
#include "id.h"

#define isTab(scan, key) ((scan==0)&(key=='\t'))
#define isBackTab(scan, key) ((scan==0)&(key==126))

static int hold;

static int SimpleTakeHotKey(t_object *b, int scan, int ascii);
static t_object *SimpleIsHotKey(t_object *b, int scan, int ascii);

t_hkfun default_hotkey_functions = {
   SimpleIsHotKey,
   GetLabel,
   SimpleTakeHotKey
};

extern void InsertHotKey(t_object *b, int scan, int ascii)
{
   t_hotkey *hk;

   hk = GetMem(t_hotkey, 1);
   hk->scan = scan;
   hk->key = ToUpper(ascii);
   hk->next = b->hks;
   b->hks = hk;
}

static t_object *CheckHotKey(t_object *b, int scan, int ascii)
{
   t_hotkey *hk;

   if (b->inactive)
      return NULL;
   for (hk = b->hks; hk; hk = hk->next) {
      if (hk->scan == scan && (char) hk->key == ascii)
         return b;
   }
   return NULL;
}

static t_object *SimpleIsHotKey(t_object *b, int scan, int ascii)
{
   return CheckHotKey(b, scan, ascii);
}

static int SimpleTakeHotKey(t_object *b, int scan, int ascii)
{
   b = CheckHotKey(b, scan, ascii);
   if (b) {
      b->tcfun->MoveFocusTo(b);
      b->usedbutton = LEFT_MOUSE;
      b->action_by_mouse = 0;
      b->Action(b->appdata);
      return 1;
   }
   return 0;
}

/* HotKeyHandler
   This is the function that will be installed at starup as the default
   recipient of key-presses passed by the interrupt-callback over the event
   queue.
   First it will call the tab-chain if it wants the key-press. If not it will
   be passed on to the object tree, and each object will be asked if it
   has this key attached. This implementation has the effect that if two
   objects has attatched the same key, the first one will get it.
   */
extern int HotKeyHandler(void *data nouse, int scan, int ascii)
{
   int tmp;
   t_object *b;

   if (fwin && TabChainNotifyKeyPress(fwin->win, scan, ascii))
      return 1;
   ascii = ToUpper(ascii);
   if (fwin)
      if (fwin->ob->hkfunc->TakeHotKey(fwin->ob, scan, ascii))
         return 1;
   if (_win_root) {
      for (b = _win_root->node->firstob; b; b = b->next)
         if (b->node == NULL)
         /* Only check for simple objects, but we must not recurse the tree */
            if (b->hkfunc->TakeHotKey(b, scan, ascii))
               return 1;
   }
   return 1;
}

/* This function passes a hotkey to any of its kids - until any of them catches the hot-key. If non of them does, it will return false. */
extern int NodeTakeHotKey(t_object *b0, int scan, int ascii)
{
   t_node *nd;
   t_object *b;

   nd = b0->node;
   for (b = nd->last; b; b = b->prev)
      if (!IsWindow(b))
         if (b->hkfunc->TakeHotKey(b, scan, ascii))
            return 1;              /* stop when recipient was found */
   return 0;
}

/* This function asks any of its kids if it has the specified hot key - until any of them returns non-NULL. If non of them does, it will return NULL. */
extern t_object *NodeIsHotKey(t_object *b0, int scan, int ascii)
{
   t_node *nd;
   t_object *b, *ret;

   nd = b0->node;
   for (b = nd->last; b; b = b->prev) {
      if (!IsWindow(b)) {
         ret = b->hkfunc->IsHotKey(b, scan, ascii);
         if (ret)
            return ret;
      }
   }
   return NULL;
}

/* This function will temporary inhibit the hot-key processing. The processor
   will be still left on the key-processor stack. It may seem nicer to instead
   remove the processor, but don't do it! When starting it again it will pe
   put on to of the stack which is not certainly its position when stopping
   it. One may whish to stop the hot-key processing while a mouse-operation is
   in progress (e.g. klick is half-done = button down) to avoid interfernce. */
extern void StopHotKeys(void)
{
   /* hold = 1; */
}

/* This function will re-activate the hot-key processor if previously stopped
 */
extern void StartHotKeys(void)
{
   hold = 0;
}

/* Application interface: */

extern int IsHotKey(int scan, int ascii)
{
   if (fwin) {
      return fwin->ob->hkfunc->IsHotKey(fwin->ob, scan, ascii) != NULL;
   }
   return 0;
}

extern void SimulateHotKeys(int scan, int ascii)
{
   HotKeyHandler(NULL, scan, ascii);
}

static int StubMoveFocusTo(t_object *b nouse)
{
   return 1;
}

extern int SetHotKey(int id, void (*CallBack) (void *), void *data, int scan, int ascii)
{
   t_object *b;
   t_node *nd;
   static int virgin = 1;
   static t_tcfun tcfun;

   if (virgin) {
      virgin = 0;
      tcfun = default_tabchain_functions;
      tcfun.MoveFocusTo = StubMoveFocusTo;
   }
   nd = GetNode(id);
   if (nd) {
      b = CreateObject(0, 0, nd);
      b->Action = CallBack;
      b->appdata = data;
      b->exclude_rect = 1;
      b->exclude_add = 1;
      b->tcfun = &tcfun;
      InsertHotKey(b, scan, ascii);
      return b->id;
   }
   return 0;
}
