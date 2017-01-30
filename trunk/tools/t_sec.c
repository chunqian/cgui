/* Contains the operations needed for the section node in a language node in
   a source tree */

#include "cgui/mem.h"
#include <string.h>
#include "t_lang.h"
#include "t_sec.h"
#include "t_arr.h"
#include "t_label.h"
#include "t_imed.h"
#include "t_block.h"
#include "t_header.h"

typedef struct t_section {
   int nl;
   struct t_label **labels;
   int na;
   struct t_array **arr;
   char *id;
   char *fn;
   struct t_block *bl;
} t_section;

extern void destroy_section(t_section *sec)
{
   int i;

   destroy_block_tree(sec->bl);
   for (i=0; i < sec->nl; i++)
      destroy_label(sec->labels[i]);
   free(sec->labels);
   for (i=0; i < sec->na; i++)
      destroy_array(sec->arr[i]);
   free(sec->arr);
   free(sec->id);
   free(sec->fn);
   free(sec);
}

extern t_section *insert_section(char *name, struct t_language *lang, char *fn)
{
   t_section *sec;

   sec = GetMem0(t_section, 1);
   hook_section(sec, lang);
   sec->id = MkString(name);
   sec->fn = MkString(fn);
   sec->bl = create_block();
   return sec;
}

extern void hook_array(struct t_array *a, t_section *sec)
{
   sec->na++;
   sec->arr = ResizeMem(struct t_array*, sec->arr, sec->na);
   sec->arr[sec->na - 1] = a;
}

extern struct t_label *insert_sec_label(char *macro, char *string, void *data, int line,
              struct t_block *bl)
{
   t_section *sec = data;

   sec->nl++;
   sec->labels = ResizeMem(struct t_label*, sec->labels, sec->nl);
   return sec->labels[sec->nl - 1] = create_label(macro, string, line, bl);
}

extern struct t_block *block_root(t_section *sec)
{
   return sec->bl;
}

extern int check_hot_keys_sec(t_section *sec)
{
   return check_hot_keys(sec->bl);
}

extern void build_section_header(struct t_header *hdr, t_section *sec)
{
   int i, j;
   char row[10000];

   for (i=0; i < sec->nl; i++)
      build_label_header(hdr, sec->labels[i], i);
   for (j=0; j < sec->na; j++)
      i += build_array_header(hdr, sec->arr[j], i);
   sprintf(row, "#define SIZE_%s %d\n", sec->id, i);
   add_header_row(hdr, row);
}

extern void build_section_intermediate(t_section *sec, struct t_ifile *imed)
{
   int i;

   add_intermediate_section_row(imed, sec->id, sec->fn);
   for (i=0; i < sec->nl; i++)
      build_header_intermediate(imed, sec->labels[i]);
   for (i=0; i < sec->na; i++)
      build_header_array_intermediate(imed, sec->arr[i]);
   for (i=0; i < sec->nl; i++)
      build_label_intermediate(imed, sec->labels[i]);
   for (i=0; i < sec->na; i++)
      build_array_intermediate(imed, sec->arr[i]);
}
