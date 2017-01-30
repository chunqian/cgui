/* Contains the operations needed for the "tree"-ish intermediate file
   representerion */

#include "cgui/mem.h"
#include "t_itree.h"
#include "t_ilang.h"

typedef struct t_inode {
   int n;
   struct t_ilang **il;
} t_inode;

extern void destroy_itree(t_inode *it)
{
   int i;

   for (i = 0; i < it->n; i++)
      destroy_ilang(it->il[i]);
}

extern t_inode *create_itree(void)
{
   t_inode *it;

   it = GetMem0(t_inode, 1);
   return it;
}

extern struct t_ilang *get_ilang_ref(struct t_inode *it, char *lang)
{
   int i;

   for (i = 0; i < it->n; i++)
      if (equal_ilang_name(it->il[i], lang))
         return it->il[i];
   return NULL;
}

extern void hook_ilang(t_inode *it, struct t_ilang *il)
{
   it->n++;
   it->il = ResizeMem(struct t_ilang*, it->il, it->n);
   it->il[it->n - 1] = il;
}

extern int check_lang_equal(t_inode *it, char *reflang)
{
   int i, ref;

   if (it->n <= 0)
      return 1;
   for (ref = 0; ref < it->n; ref++)
      if (equal_ilang_name(it->il[ref], reflang))
         break;
   if (ref >= it->n)
      ref = 0;

   for (i = 0; i < it->n; i++)
      if (!ilang_eq(it->il[i], it->il[ref]))
         return 0;
   return 1;
}

extern int print_final_text(const char *fn, const t_inode *it)
{
   int i;
   FILE *f;

   f = fopen(fn, "wt");
   if (f) {
      for (i = 0; i < it->n; i++)
         print_final_text_lang(f, it->il[i]);
      fclose(f);
      return 1;
   }
   return 0;
}
