/* Contains the operations needed for a source tree */

#include "cgui/mem.h"
#include "t_tree.h"
#include "t_tree2.h"
#include "t_lang.h"

typedef struct t_node {
   int n;
   struct t_language **lang;
} t_node;

extern void destroy_tree(t_node *t)
{
   int i;

   for (i=0; i < t->n; i++)
      destroy_language(t->lang[i]);
   free(t->lang);
   free(t);
}

extern t_node *create_tree()
{
   t_node *t;

   t = GetMem0(t_node, 1);
   return t;
}

extern void build_header(t_node *t, const char *fn)
{
   int i;

   for (i = 0; i < t->n; i++)
      build_lang_header(t->lang[i], fn);
}

extern void build_intermediate(t_node *t, struct t_ifile *ifil)
{
   int i;

   for (i=0; i < t->n; i++)
      build_lang_intermediate(t->lang[i], ifil);
}

extern int print_header_file(const char *fn, const struct t_node *t)
{
   FILE *f;

   f = fopen(fn, "wt");
   if (f) {
      if (t->n)
         output_lang_header(f, t->lang[0]);
      fclose(f);
      return 1;
   }
   return 0;
}

extern void hook_language(struct t_language *lang, struct t_node *t)
{
   t->n++;
   t->lang = ResizeMem(struct t_language*, t->lang, t->n);
   t->lang[t->n - 1] = lang;
}

extern int check_hot_keys_in_file(t_node *t)
{
   int i, ok = 1;

   for (i=0; i < t->n; i++)
      ok &= check_hot_keys_lang(t->lang[i]);
   return ok;
}
