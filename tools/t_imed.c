/* Contains all operations needed for handling of the intermediate file
   format. This includes parsing the intermediate file. Actual parsing is
   not necessary since the input is a generated file, and in addition consist
   of a rather simple structure. The "parsing" will genererat a "tree"-ish
   structure. */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <allegro.h>

#ifndef MAXPATH
#define MAXPATH 260
#endif

#include "cgui/mem.h"
#include "t_ilang.h"
#include "t_isec.h"
#include "t_istr.h"
#include "t_imed.h"

#define MAXROWLEN 100000

/* An image of the intermediate-file */
typedef struct t_ifile {
   int n;
   char **rows;
} t_ifile;

extern t_ifile *create_imed_file_image(void)
{
   t_ifile *imed;

   imed = GetMem0(t_ifile, 1);
   return imed;
}

extern void destroy_imed_file_image(t_ifile *imed)
{
   int i;

   for (i=0; i < imed->n; i++)
      free(imed->rows[i]);
   free(imed->rows);
   free(imed);
}

static void add_intermediate_row(t_ifile *imed, char *row)
{
   imed->n++;
   imed->rows = ResizeMem(char*, imed->rows, imed->n);
   imed->rows[imed->n - 1] = MkString(row);
}

extern void add_intermediate_section_row(t_ifile *imed, char *sec, char *fn)
{
   char tmp[MAXROWLEN];

   sprintf(tmp, "[%s][%s]", fn, sec);
   add_intermediate_row(imed, tmp);
}

extern void add_intermediate_language_row(t_ifile *imed, char *id)
{
   char tmp[MAXROWLEN];

   sprintf(tmp, "<%s>", id);
   add_intermediate_row(imed, tmp);
}

extern void add_intermediate_header_row(t_ifile *imed, char *label, int srcrow)
{
   char tmp[MAXROWLEN];

   sprintf(tmp, " %d#%s", srcrow, label);
   add_intermediate_row(imed, tmp);
}

extern void add_intermediate_text_row(t_ifile *imed, char *string, int srcrow)
{
   char tmp[MAXROWLEN], *s, *p, *percent;

   p = tmp;
   p += sprintf(p, "#%d#{", srcrow);
   for (s = string; *s; s++) {
      if (*s == '%') {
         s++;
         if (*s == '<') {
            while (*s != '>')
               s++;
         } else if (*s != '%' && *s!= '&') {
            *p++ = '%';
            if (*s == '-' || *s == '+' || *s == ' ' || *s == '0' || *s == '#')
               s++;
            if (*s == '*')
               s++;
            else
               while (isdigit(*s))
                  s++;
            if (*s == '.') {
               if (*s == '*')
                  s++;
               else
                  while (isdigit(*s))
                     s++;
            }
            if ((*s == 'h' || *s == 'l') && s[1] &&
                (s[1] == 'd' || s[1] == 'i' || s[1] == 'o' ||
                 s[1] == 'x' || s[1] == 'u')) {
               p += sprintf(p, "%c", *s);
               s++;
               p += sprintf(p, "%c", *s);
            } else if (*s == 'L' && s[1] &&
                (s[1] == 'e' || s[1] == 'f' || s[1] == 'g' ||
                 s[1] == 'E' || s[1] == 'G')) {
               p += sprintf(p, "%c", *s);
               s++;
               p += sprintf(p, "%c", *s);
            } else if (*s == 'd' || *s == 'i' || *s == 'f' || *s == 'e' ||
                  *s == 'c' || *s == 's' || *s == 'u' || *s == 'o' ||
                  *s == 'x' || *s == 'g' || *s == 'p' || *s == 'n' ||
                  *s == 'E' || *s == 'G' || *s == 'X' || *s == '.') {
               p += sprintf(p, "%c", *s);
            } else {
               printf("Unterminated conversion specification in line %d:"
                            "\n%s\n", srcrow, string);
            }
         } else if (*s == 0) { /* syntax error in format string */
            printf("Unterminated conversion specification in line %d:"
                            "\n%s\n", srcrow, string);
            break;
         }
      }
   }
   percent = string;
   while ((percent=strstr(percent, "%%"))) {
      strcpy(percent, percent+1);
   }
   p += sprintf(p, "}%s", string);
   add_intermediate_row(imed, tmp);
}

extern int print_intermediate_file(const char *fn, const t_ifile *imed)
{
   int i;
   FILE *f;

   f = fopen(fn, "wt");
   if (f) {
      for (i=0; i < imed->n; i++) {
         fprintf(f, "%s\n", imed->rows[i]);
      }
      fclose(f);
      return 1;
   }
   return 0;
}

extern int load_intermediate_file(const char *fn, t_ifile *imed)
{
   FILE *f;
   char s[MAXROWLEN];

   f = fopen(fn, "rt");
   if (f) {
      do {
         if (fgets(s, MAXROWLEN, f) == NULL)
            break;
         s[strlen(s) - 1] = 0;
         add_intermediate_row(imed, s);
      } while (!feof(f));
      fclose(f);
      return 1;
   } else
      return 0;
}

static char *scanfn(char *fn, char *srcline)
{
   char *p;

   for (p = srcline; *p && *p != ']'; p++, fn++)
      *fn = *p;
   *fn = 0;
   while (*p && *p != '[')
      p++;
   return p;
}

static char *scan_rownr(int *rn, char *srcline)
{
   char *p, *end;
   int x = 1;

   for (end = srcline; *end && !isdigit(*end); end++)
      ;
   for (end = srcline; *end && isdigit(*end); end++)
      ;
   *rn = 0;
   for (p = end - 1; p >= srcline && isdigit(*p); p--, x *= 10)
      *rn += (*p - '0') * x;
   while (*end && *end != '#')
      end++;
   if (*end)
      end++;
   return end;
}

extern int parse_intermediate_data(t_ifile *imed, struct t_inode *it)
{
   struct t_ilang *il;
   struct t_isec *is;
   char fn[MAXPATH], *next;
   int i, rn;

   for (i=0; i < imed->n; ) {
      if (*imed->rows[i] != '<')
         return 0;
      il = insert_ilang(it, imed->rows[i++]);
      for (; i < imed->n && *imed->rows[i] == '['; ) {
         next = scanfn(fn, imed->rows[i++] + 1);
         is = insert_isec(il, next, fn);
         for (; i < imed->n && *imed->rows[i] == ' '; i++) {
            next = scan_rownr(&rn, imed->rows[i] + 1);
            add_imacro(is, next, rn);
         }
         for (; i < imed->n && *imed->rows[i] == '#'; i++) {
            next = scan_rownr(&rn, imed->rows[i] + 1);
            add_istring(is, next, rn);
         }
      }
   }
   return 1;
}
