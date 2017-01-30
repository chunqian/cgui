/* Contains the operations needed for the section node in a language node in
   an intermediate file structure */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <allegro.h>
#include "cgui/mem.h"

#include "t_isec.h"
#include "t_ilang.h"
#include "t_istr.h"

typedef struct t_imac {
   char *name;
   int srcrow;
} t_imac;

typedef struct t_isec {
   char *name;
   char *fn;
   int nm;
   t_imac *macros;
   int ns;
   struct t_istring **str;
} t_isec;

extern void destroy_isection(t_isec *is)
{
   int i;

   for (i = 0; i < is->nm; i++)
      free(is->macros[i].name);
   for (i = 0; i < is->ns; i++)
      free(is->str[i]);
   free(is->macros);
   free(is->name);
   free(is->fn);
   free(is);
}

extern t_isec *insert_isec(struct t_ilang *il, char *secname, char *fn)
{
   struct t_isec *is;

   is = GetMem0(t_isec, 1);
   is->name = MkString(secname);
   is->fn = MkString(fn);
   hook_isec(il, is);
   return is;
}

extern char *isec_file(t_isec *is)
{
   return is->fn;
}

extern void add_imacro(t_isec *is, char *macrodef, int srcrow)
{
   is->nm++;
   is->macros = ResizeMem(t_imac, is->macros, is->nm);
   is->macros[is->nm - 1].name = MkString(macrodef);
   is->macros[is->nm - 1].srcrow = srcrow;
}

extern void hook_istring(t_isec *is, struct t_istring *str)
{
   is->ns++;
   is->str = ResizeMem(struct t_istring*, is->str, is->ns);
   is->str[is->ns - 1] = str;
}

extern const char *isection_name(t_isec *is)
{
   return is->name;
}

extern int equal_isection_name(struct t_isec *is, const char *name)
{
   return strcmp(is->name, name) == 0;
}

extern int isec_eq(t_isec *is, t_isec *ref)
{
   int i, n;

   if (ref->nm < is->nm)
      n = ref->nm;
   else
      n = is->nm;

   for (i = 0; i < n; i++) {
      if (strcmp(ref->macros[i].name, is->macros[i].name) != 0) {
         allegro_message("%s in line %d in file %s\n"
                         "differs from\n"
                         "%s in line %d in file %s\n"
                         "in section %s\n",
                         ref->macros[i].name, ref->macros[i].srcrow, ref->fn,
                         is->macros[i].name, is->macros[i].srcrow, is->fn,
                         is->name);
         return 0;
      }
   }
   if (ref->nm < is->nm) {
      allegro_message("Section %s after %d in %s contains excess stuff\n",
                      is->name, is->macros[i-1].srcrow, is->fn);
      return 0;
   }
   if (ref->nm > is->nm) {
      allegro_message("Section %s in %s lacks data after %d\n",
                      is->name, is->fn, is->macros[i-1].srcrow);
      return 0;
   }

   /* if noone has damaged the file there can not be any diference in
      string number when the macro numbers are the same */
   if (ref->ns < is->ns)
      n = ref->ns;
   else
      n = is->ns;
   for (i = 0; i < n; i++) {
      if (!istring_eq(is->str[i], ref->str[i])) {
         allegro_message("... in section %s in %s\n", is->name, is->fn);
         return 0;
      }
   }
   return 1;
}

extern int sort_isec(const void *s1, const void *s2)
{
   t_isec *const*is1 = s1, *const*is2 = s2;

   return strcmp((*is1)->name, (*is2)->name);
}

extern void print_final_text_section(FILE *f, t_isec *is)
{
   int i;
   char s[1000];

   strcpy(s, is->name);
   if (*s)
      s[strlen(s) - 1] = 0;
   fprintf(f, "%s\n", s);
   for (i = 0; i < is->ns; i++)
      print_final_text_string(f, is->str[i]);
}
