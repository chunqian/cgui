/* Contains the operations needed for the array node in a section node in a
   language node in a source tree */

#include "cgui/mem.h"
#include <string.h>
#include <stdio.h>
#include "t_tree.h"
#include "t_sec.h"
#include "t_arr.h"
#include "t_label.h"
#include "t_header.h"
#include "t_imed.h"

typedef struct t_array {
   int n;
   struct t_label **labels;
   char *name;
   char *endname;
   int row,endrow;
} t_array;

extern void destroy_array(t_array *arr)
{
   int i;

   for (i=0; i < arr->n; i++)
      destroy_label(arr->labels[i]);
   free(arr->labels);
   free(arr->name);
   free(arr->endname);
   free(arr);
}

extern t_array *insert_array(char *name, struct t_section *sec, int line)
{
   t_array *arr;

   arr = GetMem0(t_array, 1);
   hook_array(arr, sec);
   arr->name = MkString(name);
   arr->row = line;
   return arr;
}

extern struct t_label *insert_array_label(char *macro, char *string, void *data, int line,
                      struct t_block *bl)
{
   t_array *arr = data;

   arr->n++;
   arr->labels = ResizeMem(struct t_label*, arr->labels, arr->n);
   return arr->labels[arr->n - 1] = create_label(macro, string, line, bl);
}

extern void add_array_end(char *name, t_array *arr, int line)
{
   arr->endname = MkString(name);
   arr->endrow = line;
}

extern int build_array_header(struct t_header *hdr, t_array *arr, int macroval)
{
   int i;
   char row[1000];

   sprintf(row, "#define %s %d", arr->name, macroval);
   add_header_row(hdr, row);
   for (i=0; i < arr->n; i++)
      build_label_header(hdr, arr->labels[i], i);
   sprintf(row, "#define %s %d", arr->endname, arr->n);
   add_header_row(hdr, row);
   return arr->n;
}

extern void build_array_intermediate(struct t_ifile *imed, t_array *arr)
{
   int i;

   for (i=0; i < arr->n; i++)
      build_label_intermediate(imed, arr->labels[i]);
}

extern void build_header_array_intermediate(struct t_ifile *imed, t_array *arr)
{
   int i;

   add_intermediate_header_row(imed, arr->name, arr->row);
   for (i=0; i < arr->n; i++)
      build_header_intermediate(imed, arr->labels[i]);
   add_intermediate_header_row(imed, arr->endname, arr->endrow);
}
