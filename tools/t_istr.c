/* Contains the operations needed for the string node in a section node in a
   language node in an intermediate file structure */

#include <stdio.h>
#include <string.h>
#include <allegro.h>
#include "cgui/mem.h"

#include "t_isec.h"
#include "t_istr.h"

typedef struct t_istring {
   char *str;
   int nf;
   unsigned int *fmt;
   int nr;
   int srcrow;
} t_istring;

extern void destroy_istring(t_istring *str)
{
   free(str->str);
   free(str->fmt);
   free(str);
}

extern void add_istring(struct t_isec *is, char *s, int rn)
{
   t_istring *str;
   char *p, *actual_string;
   int i;

   str = GetMem0(t_istring, 1);
   str->srcrow = rn;
   for (p = s + 1, str->nf = 0; *p && *p != '}'; p++) {
      if (*p == '%')
         str->nf++;
   }
   if (str->nf > 0)
      str->fmt = GetMem0(unsigned int, str->nf);
   for (s++, i = 0; *s && *s != '}'; )
      if (*s++ == '%')
         str->fmt[i++] = *s;
   if (*s == '}')
      s++;
   actual_string = s;
   for (; *s; s++) {
      if (*s == '|') {
         if (s[1] && s[1] == '|')
            s++;
         else
            str->nr++;
      }
   }
   str->str = MkString(actual_string);
   hook_istring(is, str);
}

extern int istring_eq(t_istring *str, t_istring *ref)
{
   int i, n;

   if (str->nf < ref->nf)
      n = str->nf;
   else
      n = ref->nf;

   for (i = 0; i < n; i++) {
     if (str->fmt[i] != ref->fmt[i]) {
        allegro_message("Conversion specification %d differs\n"
                        " Strings:\nLine %d %s\nLine %d %s\n", i + 1,
                        str->srcrow, str->str,
                        ref->srcrow, ref->str);
        return 0;

     }
   }
   if (str->nf != ref->nf) {
      allegro_message("Different number of conversion specifications detected."
                      " Strings:\nLine %d %s\nLine %d %s\n",
                      str->srcrow, str->str,
                      ref->srcrow, ref->str);
      return 0;
   }

   if (str->nr != ref->nr) {
      allegro_message("Warning! Different number of request-buttons detected."
                      " Strings:\nLine %d %s\nLine %d %s\n",
                      str->srcrow, str->str,
                      ref->srcrow, ref->str);
   }
   return 1;
}

extern void print_final_text_string(FILE *f, t_istring *str)
{
   char *p;

   p = strchr(str->str, '%');
   if (p) {
      p++;
      if (*p == '&') {
         *p = 0;
         fprintf(f, "\"%s", str->str);
         *p++ = '&';
         fprintf(f, "%s\n", p);
         return;
      }
   }
   fprintf(f, "\"%s\n", str->str);
}
