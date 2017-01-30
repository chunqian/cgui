/* Module HALFFORM.C
   This file contains functions for converting from double-int representation
   to the « character
*/

#include <string.h>
#include <stdio.h>

#include "cgui/halfform.h"

/* Converts to a long right value */
extern char *LongHalf(char *s, int x, int decipoint)
{
   char half[20];

   if (decipoint)
      strcpy(half, ".5");
   else
      strcpy(half, HALF_STRING);
   if (x & 1) {
      if (x == 1) {
         strcpy(s, half);
      } else {
         if (x < 0) {
            if (x == -1)
               sprintf(s, "-%s", half);
            else
               sprintf(s, "%d%s", (x >> 1) + 1, half);
         } else
            sprintf(s, "%d%s", x >> 1, half);
      }
   } else
      sprintf(s, "%d", x >> 1);
   return s;
}

/* Converts to varible size. If value is 0 it will only print the 0 on demand
 */
extern char *MakeHalf(char *s, int x, int printzero, int decipoint)
{
   char half[20];

   if (decipoint) {
      if (printzero)
         strcpy(half, "0.5");
      else
         strcpy(half, ".5");
   } else
      strcpy(half, HALF_STRING);
   switch (x) {
   case 1:
      strcpy(s, half);
      break;
   case 0:
      if (printzero)
         strcpy(s, "0");        /* do print a 0; */
      else
         *s = 0;
      break;
   case -1:
      sprintf(s, "-%s", half);
      break;
   default:
      LongHalf(s, x, decipoint);
      break;
   }
   return s;
}
