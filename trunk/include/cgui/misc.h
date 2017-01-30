#ifndef MISC_H
#define MISC_H

#include "cguicfg.h"

#ifdef __cplusplus
  extern "C" {
#endif

CGUI_FUNC(int, DeleteTree, (const char *path));
CGUI_FUNC(int, MkDir, (const char *p));

#ifdef __cplusplus
  }
#endif

#endif
