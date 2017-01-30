/* Contains the operations needed for the label node somewhere a source
   tree */

#include <stdio.h>
#include "cgui/mem.h"
#include <string.h>
#include "t_tree.h"
#include "t_label.h"
#include "t_header.h"
#include "t_imed.h"
#include "t_block.h"

typedef struct t_label {
   char *macro;
   char *string;
   int line; /* source line nr */
} t_label;

extern void destroy_label(t_label *l)
{
   free(l->macro);
   free(l->string);
   free(l);
}

extern t_label *create_label(char *macro, char *string, int line, struct t_block *bl)
{
   t_label *l;

   l = GetMem0(t_label, 1);
   l->macro = MkString(macro);
   l->string = MkString(string);
   l->line = line;
   insert_block_label(bl, l, l->string, line);
   return l;
}

extern void concat_string(char *name, t_label *l)
{
   l->string = ResizeMem(char, l->string, strlen(name) + strlen(l->string) + 1);
   strcat(l->string, name);
}

extern void build_label_header(struct t_header *hdr, t_label *l, int i)
{
   char row[1000];

   if (l->macro[0] == '_' && l->macro[1] == 0)
      return;
   sprintf(row, "#define %s %d", l->macro, i);
   add_header_row(hdr, row);
}

extern void build_label_intermediate(struct t_ifile *imed, t_label *l)
{
   add_intermediate_text_row(imed, l->string, l->line);
}

extern void build_header_intermediate(struct t_ifile *imed, t_label *l)
{
   add_intermediate_header_row(imed, l->macro, l->line);
}
