/* Module REFRESH
   Contains functions for the refresh system */

#include "cgui.h"
#include "cgui/mem.h"

#include "node.h"
#include "id.h"
#include "refresh.h"

struct t_refresh_traversor {
   int stopped;
};

/* Recurse the entire object tree in a top down manner, but break the recursion in case a stop has been
   requested (this indicates that the currently visited object and its descendants are no longer existing). */
static void TraverseObjectTree(t_object *b, t_refresh_traversor *visitor, void *calldata, int reason)
{
   t_object *child;

   ObjectRefreshEnter(b, visitor);
   ObjectRefreshVisit(b, calldata, reason);
   if (!visitor->stopped) {
      ObjectRefreshLeave(b);
      if (b->node) {
         for (child = b->node->firstob; child && !visitor->stopped; child = child->next) {
            TraverseObjectTree(child, visitor, calldata, reason);
         }
      }
   }
}

extern void StopRefreshTraversing(t_refresh_traversor *visitor)
{
   visitor->stopped = 1;
}

extern void ConditionalRefresh(void *calldata, int reason)
{
   t_refresh_traversor *visitor;

   visitor = GetMem0(t_refresh_traversor, 1);
   StopBlitting();
   TraverseObjectTree(_win_root, visitor, calldata, reason);
   ContinueBlitting();
   free(visitor);
}

extern void RegisterRefresh(int id, void (*AppUpd) (int id, void *data, void *calldata, int reason), void *data)
{
   t_object *b = GetObject(id);

   if (b) {
      b->AppUpd = AppUpd;
      b->appupddata = data;
   }
}
