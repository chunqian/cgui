#ifndef DW_CONV_H
#define DW_CONV_H

#include "cguicfg.h"

#ifdef __cplusplus
  extern "C" {
#endif

CGUI_FUNC(char *, Win2Dos, (const char *src, char *dest));
CGUI_FUNC(char *, Dos2Win, (const char *src, char *dest));

#ifdef __cplusplus
  }
#endif

#endif
