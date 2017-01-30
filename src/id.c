/* Module ID.C
   Contains the function for managing identity numbers for the objects

   Id-numbers is necessary to make it safe the access functions even for
   applications that are not experianced. The alternative way is to refere to
   objects using pointers. Using dangling pointers for direct access may let
   the application error crach the system functions, which is no good. Using
   id-number will make the comand ignored. Other pointer errors made by the
   application programmer will crach in the application functions.

   The ids must always be returned when an object is deleted, so that it can
   be re-cycled. Since a normal application will work like a stack, we just
   have to keep track of the highest id-number, which is both processor and
   memory efficient.

   At start there wil be allocated place for 10000 ids which is a large
   number of window objects. If reaching the limit, the id-array will be
   extended with 10000 more. This extension may be repeated 25 times. If
   there whould really quarter a million of objects, the program would
   probably not be running any more because the memory for all other stuff
   would take all the memory.

   However there is a possibilty that the application don't behave in a
   "normal" manner, i.e. it always creates new windows before closign the
   old. In this case will may get a situation where nearly the entire
   ids-array is empty - those in use is at the top. It would be a pity to end
   with memory error due to this, so we now start to look for free entries
   from bottom. If the application doesn't change behaviour at this moment
   the allocated numers at top will soon be freed and go turn back the
   default mode where we don't need to serach.
*/

#include <string.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "id.h"

static struct t_object **ids;
static int n, maxid, chunksize = DEFAULT_CHUNK_SIZE, definite_size =
    DEFAULT_CHUNK_SIZE * 25;

static void ReleaseIds(void *data nouse)
{
   Release(ids);
   ids = NULL;
   n = maxid = 0;
}

extern int GetAnId(struct t_object *b)
{
   int i;

   if (n >= maxid) {
      if (maxid >= definite_size) {
         for (i = 0; i < n; i++) {
            if (ids[i] == NULL) {
               ids[i] = b;
               return i;
            }
         }
         return 0;              /* we can do nothing */
      } else {
         if (ids == NULL)
            HookCguiDeInit(ReleaseIds, ids);
         ids = ResizeMem(struct t_object *, ids, maxid + chunksize);
         memset(ids + maxid, 0, sizeof(struct t_object *) * chunksize);
         maxid += chunksize;
      }
   }
   ids[n] = b;
   return n++;
}

extern struct t_object *GetObject(int id)
{
   if ((unsigned) id < (unsigned) n)
      return ids[id];
   return NULL;
}

/* It will be most efficient to return objects in the same order as created
   to minimize the loop overhead The id-number should be proper (it must
   never be passed directly by the application) */
extern void ReturnId(int id)
{
   ids[id] = NULL;
   if (id == n - 1)
      for (; n && ids[n - 1] == NULL; n--);
}

/* Application interface */
extern void AdjustIdParam(int block_size, int maxlevel)
{
   chunksize = block_size;
   definite_size = maxlevel;
}
