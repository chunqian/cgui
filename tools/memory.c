/* Module MEMORY
   Contains interface-functions for memory handling */

#include <allegro.h>
#include <string.h>

#include "cgui/mem.h"

#include "../src/memint.h"

#ifdef FORTIFY
#include "../include/fortify2.h"
#endif

#if ALLEGRO_SUB_VERSION == 0
#define textout_ex(bmp, f, s, x, y, col, bgcol) textout(bmp, f, s, x, y, col)
#endif

typedef struct t_deinit_chain {
   void (*DeInitFunc)(void*);
   void *data;
} t_deinit_chain;

static t_deinit_chain *deinit_chain = NULL;
static int nr_deinits = 0;
int cgui_started = 0;

#ifndef FORTIFY
static void MemErr(void)
{
   clear_to_color(screen, makecol(255,255,255));
   textout_ex(screen, font, "Fatal error:Out of memory. The program will terminate", SCREEN_W/2, SCREEN_H/2, makecol(0,0,0), -1);
   rest(2000);
   exit(0);
}
#endif

extern void DeInitChain(void)
{
   int i;
   static int ishere = 0;

   if (ishere)
      return;
   ishere = 1;
   if (deinit_chain) {
      for (i=nr_deinits-1; i>=0; i--) {
         deinit_chain[i].DeInitFunc(deinit_chain[i].data);
      }
      Release(deinit_chain);
      deinit_chain = NULL;
      nr_deinits = 0;
   }
#ifdef FORTIFY
   fortify_exit();
#endif
   ishere = 0;
}

extern void HookCguiDeInit(void (*DeInitFunc)(void*), void *data)
{
   deinit_chain = ResizeMem(t_deinit_chain, deinit_chain, ++nr_deinits);
   deinit_chain[nr_deinits-1].DeInitFunc = DeInitFunc;
   deinit_chain[nr_deinits-1].data = data;
}

extern void UnHookCguiDeInit(void (*DeInitFunc)(void*), void *data)
{
   int i;
   static int deinit_in_progress = 0;
   if (!deinit_in_progress) {
      deinit_in_progress = 1;
      for (i=0; i<nr_deinits; i++) {
         if (deinit_chain[i].DeInitFunc==DeInitFunc && deinit_chain[i].data==data) {
            for (i++; i<nr_deinits; i++)
               deinit_chain[i-1] = deinit_chain[i];
            nr_deinits--;
         }
      }
      deinit_in_progress = 0;
   }
}

extern void RegisterCleanupFunc(int (*atexit_ptr)(void (*func)(void)))
{
   static int done = 0;

#ifdef FORTIFY
   if (!cgui_started)
      fortify_init();
#endif
   if (!done) {
      atexit_ptr(DeInitChain);
      done = 1;
   }
}

#ifndef FORTIFY
#include <malloc.h>

extern char *MkString(const char *string)
{
   char *ns = strdup(string);

   if (ns == NULL && string != NULL)
      MemErr();
   return ns;
}

extern void *cgui_malloc(size_t size)
{
   void *mem;

   mem = malloc(size);
   if (mem == NULL && size)
      MemErr();
   return mem;
}

extern void *cgui_realloc(void *oldmem, size_t newsize)
{
   oldmem = realloc(oldmem, newsize);
   if (oldmem == NULL && newsize != 0)
      MemErr();
   return oldmem;
}

extern void *cgui_calloc(size_t size)
{
   void *mem;

   mem = calloc(1, size);
   if (mem == NULL && size)
      MemErr();
   return mem;
}
#endif

extern void Release(void *data)
{
   free(data);
}

extern void InitMemory(int (*atexit_ptr)(void (*func)(void)))
{
   RegisterCleanupFunc(atexit_ptr);
}
