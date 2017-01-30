#ifndef  MEM_H
#define  MEM_H

#include <stdlib.h>
#include "cguicfg.h"

#ifdef __cplusplus
  extern "C" {
#endif

CGUI_FUNC(void, HookCguiDeInit, (CGUI_METHOD(void, DeInitFunc, (void*)), void *data));
CGUI_FUNC(void, UnHookCguiDeInit, (CGUI_METHOD(void, DeInitFunc, (void*)), void *data));

#ifdef FORTIFY
/* In test-mode we want getting correct line-numbers from fortify so the
   wrappers are omitted (during test it's also rather better to let the
   program crasch because of dereferencing of a NULL pointer, than just exit)
*/
#define MkString(str) strdup(str)
#define GetMem(type, nelem) (type*)malloc((nelem)*sizeof(type))
#define GetMem0(type, nelem) (type*)calloc(nelem, sizeof(type))
#define ResizeMem(type, p, nelem) (type*)realloc(p, (nelem)*sizeof(type))
#define cgui_malloc(size) malloc(size)
#define cgui_calloc(size) calloc(1, size)
#define cgui_realloc(oldmem, newsize) realloc(oldmem, newsize)
#define Release(x) free(x)
#include "../fortify.h"

#else

CGUI_FUNC(char *, MkString, (const char *string));

/* This explicit type-cast forces type-checking to be done on the lvalue in
   the assignment */
#define GetMem(type, nelem) (type*)cgui_malloc((nelem)*sizeof(type))
#define GetMem0(type, nelem) (type*)cgui_calloc((nelem)*sizeof(type))
#define ResizeMem(type, p, nelem) (type*)cgui_realloc(p, (nelem)*sizeof(type))

/* These corresponds to the "x-functions" (i.e. they exit on fail); they are
   needed for the macros above (but they can of course be used with direct
   calls if necessary). */
CGUI_FUNC(void *, cgui_malloc, (size_t size));
CGUI_FUNC(void *, cgui_calloc, (size_t size));
CGUI_FUNC(void *, cgui_realloc, (void *oldmem, size_t newsize));
#endif

/* Currently there is no use for this wrapper */
/* #define Release(p) free(p) can't be a macro if shared library */
CGUI_FUNC(void, Release, (void *));

#ifdef __cplusplus
  }
#endif

#endif
