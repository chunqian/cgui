#ifndef DATEFORM_H
#define DATEFORM_H

#include "cguicfg.h"

#ifdef __cplusplus
  extern "C" {
#endif

struct tm;

/*
   Converts a date specified as a string into a date-struct. It will use the
   format specification specified by `format'. This is supposed to contain
   component letters with or without delimiters. The component letters, are
   defined in formdate.t (for english the letters are "YMD". 
   Anything in the format string that is not one of these three letters, is
   regarded as a delimiter. Blanks are ignored in the source string as well
   as in the format string. If "format" is NULL or empty string the default
   format specification is used instead. This is loaded from the current
   environment the first time it is used, and may later be changed and
   stored, see below. Returns 0 if the format string is not consistent or 
   if the scanned string does not match the format, otherwise non-zero. */
CGUI_FUNC(int, String2Date, (struct tm *d, const char *s, const char *format));

/*
   Converts a date specified as a date-struct into a string. It will use the
   format specification specified by `format' or the default format
   specification if NULL or empty string, see String2Date for more details
   about format strings. Returns 0 if the format string is not consistent,
   otherwise non-zero. */
CGUI_FUNC(int, Date2String, (struct tm *d, char *s, const char *format));

/*
   Checks if a date in a date-struct is correct. */
CGUI_FUNC(int, CheckDate, (struct tm *d, const char *format));

/*
   Set the default date format to "format". */
CGUI_FUNC(void, SetDateForm, (const char *format));

#ifdef __cplusplus
  }
#endif

#endif
