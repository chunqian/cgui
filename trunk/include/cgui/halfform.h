#ifndef HALFFORM_H
#define HALFFORM_H

#include "cguicfg.h"

#ifdef __cplusplus
  extern "C" {
#endif

/* Sometimes real data is of a category that has values with a resolution of */
/* 0.5. Then it may be conveniant to store them in integers whith twice its */
/* real value. */
/* Here are some functions converting such numers from text or to text with */
/* the 0.5 fraction as the «-character in the text representation. */
CGUI_FUNC(char*, MakeHalf, (char *s, int x, int printzero, int decipoint));

/* LongHalf If x is odd the «-character will be added to the right. If x is 1
   there will be no 0-character to the left of the «-character. If x is 0 s
   will contain 0. Return value: s */
CGUI_FUNC(char*, LongHalf, (char *s, int x, int decipoint));

/* Acronyms for the half-character. */
#define HALF_CHAR '\xbd'
#define HALF_STRING "\xbd"
#define MINUS_HALF_STRING "-\xbd"

#ifdef __cplusplus
  }
#endif

#endif
