/* Contains the operations needed for the language node in an
   intermediate file structure */

#include <string.h>
#include <stdio.h>
#include <allegro.h>
#include "cgui/mem.h"

#include "t_itree.h"
#include "t_isec.h"
#include "t_ilang.h"

typedef struct t_ilang {
   char *id;
   int n;
   struct t_isec **is;
} t_ilang;

extern int equal_ilang_name(struct t_ilang *il, char *lang)
{
   return strcmp(il->id, lang) == 0;
}

extern void destroy_ilang(struct t_ilang *il)
{
   int i;

   for (i = 0; i < il->n; i++)
      destroy_isection(il->is[i]);
   free(il->is);
   free(il->id);
   free(il);
}

extern void hook_isec(t_ilang *il, struct t_isec *is)
{
   il->n++;
   il->is = ResizeMem(struct t_isec*, il->is, il->n);
   il->is[il->n - 1] = is;
}

extern t_ilang *insert_ilang(struct t_inode *it, char *lang)
{
   t_ilang *il;

   il = get_ilang_ref(it, lang);
   if (il == NULL) {
      il = GetMem0(t_ilang, 1);
      il->id = MkString(lang);
      hook_ilang(it, il);
   }
   return il;
}

extern int ilang_eq(struct t_ilang *il, struct t_ilang *ref)
{
   int i, j, x, eq = 1;
   const char *name = "";

   if (il == ref)
      return eq;
   for (i = 0; i < ref->n; i++) {
      name = isection_name(ref->is[i]);
      for (j = 0; j < il->n; j++) {
         if (equal_isection_name(il->is[j], name)) {
            x = isec_eq(il->is[j], ref->is[i]);
            if (!x) {
               allegro_message("... language %s compared to %s\n",
                               il->id, ref->id);
            }
            eq &= x;
            break;
         }
      }
      if (j >= il->n) {
         allegro_message("Section %s in file %s in language %s\n"
                         "does not exist in language %s\n",
                         name, isec_file(ref->is[i]), ref->id, il->id);
         eq = 0;
         break;
      }
   }
   if (eq && il->n > ref->n) {
      eq = 0;
      ilang_eq(ref, il);
   }
   return eq;
}

extern void print_final_text_lang(FILE *f, const t_ilang *il)
{
   int i;

   qsort(il->is, il->n, sizeof(struct t_isec *), sort_isec);
   fprintf(f, "%s\n", il->id);
   for (i = 0; i < il->n; i++)
      print_final_text_section(f, il->is[i]);
}
