/* Contains the operations needed for the language node in a source tree */

#include <stdio.h>
#include "cgui/mem.h"
#include <string.h>
#include "t_tree.h"
#include "t_tree2.h"
#include "t_lang.h"
#include "t_sec.h"
#include "t_imed.h"
#include "t_header.h"

typedef struct t_language {
   int n;
   struct t_section **sec;
   char *id;
   struct t_header *hdr;
} t_language;

extern t_language *insert_language(char *name, struct t_node *t)
{
   t_language  *lang;

   lang = GetMem0(t_language, 1);
   hook_language(lang, t);
   lang->id = MkString(name);
   return lang;
}

extern void destroy_language(t_language *lang)
{
   int i;

   for (i=0; i < lang->n; i++)
      destroy_section(lang->sec[i]);
   destroy_header(lang->hdr);
   free(lang->sec);
   free(lang->id);
   free(lang);
}

extern void build_lang_header(t_language *lang, const char *fn)
{
   int i;

   lang->hdr = create_header(fn);
   for (i=0; i < lang->n; i++)
      build_section_header(lang->hdr, lang->sec[i]);
}

extern void build_lang_intermediate(t_language *lang, struct t_ifile *imed)
{
   int i;

   add_intermediate_language_row(imed, lang->id);
   for (i=0; i < lang->n; i++)
      build_section_intermediate(lang->sec[i], imed);
}

extern void output_lang_header(FILE *f, t_language *lang)
{
   print_header(f, lang->hdr);
}

extern void hook_section(struct t_section *sec, struct t_language *lang)
{
   lang->n++;
   lang->sec = ResizeMem(struct t_section*, lang->sec, lang->n);
   lang->sec[lang->n - 1] = sec;
}

extern int check_hot_keys_lang(t_language *lang)
{
   int i, ok = 1;

   for (i=0; i < lang->n; i++)
      ok &= check_hot_keys_sec(lang->sec[i]);
   return ok;
}
