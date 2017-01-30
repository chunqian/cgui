#ifndef CLIPWIN_H
#define CLIPWIN_H

#include "cguicfg.h"

#ifdef __cplusplus
  extern "C" {
#endif

CGUI_FUNC(int, CheckWinClip, (void));
CGUI_FUNC(char*, GetFromWinClip, (void));
CGUI_FUNC(int, InsertIntoWinClip, (char *p));

#ifdef __cplusplus
  }
#endif

#endif
