/* Contains all operations needed for the header object. The header object is
   supposed to contain all data from the source file that is needed for
   generating a header file from it. */

#include <stdio.h>
#include "cgui/mem.h"
#include <allegro.h>
#include <string.h>

#include "t_header.h"

#ifndef MAXPATH
#define MAXPATH 260
#endif

#define VER1 1
#define VER2 1

typedef struct t_header {
   int n;
   char **rows;
   const char *fn;
} t_header;

extern t_header *create_header(const char *fn)
{
   t_header *hdr;

   hdr = GetMem0(t_header, 1);
   hdr->fn = fn;
   return hdr;
}

extern void destroy_header(t_header *hdr)
{
   int i;

   for (i=0; i < hdr->n; i++)
      free(hdr->rows[i]);
   free(hdr->rows);
   free(hdr);
}

extern void add_header_row(t_header *hdr, char *row)
{
   hdr->n++;
   hdr->rows = ResizeMem(char*, hdr->rows, hdr->n);
   hdr->rows[hdr->n - 1] = MkString(row);
}

static int print_macros(FILE *f, t_header *hdr)
{
   int i;

   for (i=0; i < hdr->n; i++)
      fprintf(f, "%s\n", hdr->rows[i]);
   return hdr->n;
}

extern void print_header(FILE *f, t_header *hdr)
{
   char nameid[MAXPATH], fname[MAXPATH], *ext;

   fprintf(f, "/* Generated header-file <generator=mktext.%d.%d> */\n", VER1, VER2);
   strcpy(fname, get_filename(hdr->fn));
   ext = get_extension(fname);
   if (*ext--)
      *ext = 0;
   strupr(fname);
   sprintf(nameid, "%s_HT", fname);
   fprintf(f, "#ifndef %s\n", nameid);
   fprintf(f, "#define %s\n", nameid);

   print_macros(f, hdr);
   fprintf(f, "#endif\n");
}
