/* Generates a text file and a header file from a special format of source
   file. It is a tool to make programs language independent. See readme.txt
   and the cgui help for details. */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define USE_CONSOLE
#include <allegro.h>

#ifndef MAXPATH
#define MAXPATH 260
#endif

#include "cgui/mem.h"
#include "t_scan.h"
#include "t_parser.h"
#include "t_tree.h"
#include "t_header.h"
#include "t_imed.h"
#include "t_itree.h"

#if ALLEGRO_SUB_VERSION == 0

#define for_each_file_ex(filename, flags, reject_flags, callback, data) \
        ((void (*)(const char *, int, void (*)(const char *, int, void*), void*))for_each_file)(filename, flags, callback, data)
#define FOREACHFILE_CALLBACK_RETVAL
#define FOREACHFILE_CALLBACK_RETTYPE void
#define ALLFILES FA_RDONLY|FA_ARCH|FA_HIDDEN|FA_SYSTEM

#else

#define FOREACHFILE_CALLBACK_RETVAL 0
#define FOREACHFILE_CALLBACK_RETTYPE int
#define ALLFILES 0
#endif

typedef struct t_files {
   int n;
   char **fn;
} t_files;

static FOREACHFILE_CALLBACK_RETTYPE get_fn(const char *fn, int attrib, void *data)
{
   t_files *f = (t_files*)data;

   if ((attrib & FA_DIREC) == 0) {
      f->fn = ResizeMem(char*, f->fn, ++f->n);
      f->fn[f->n - 1] = MkString(fn);
   }
   return FOREACHFILE_CALLBACK_RETVAL;
}

static char *mk_fn(char *path, char *optpath, char *src, char *ext)
{
   if (*optpath) {
      append_filename(path, optpath, get_filename(src), MAXPATH);
      replace_extension(path, path, ext, MAXPATH);
   } else {
      replace_extension(path, src, ext, MAXPATH);
   }
   return path;
}

int main(int const n, char const * const *strings)
{
   struct t_ifile **imed;
   struct t_tokenlist *tl;
   struct t_node *t;
   struct t_inode *it;
   t_files *srcs;
   char htpath[MAXPATH] = "", itpath[MAXPATH] = "";
   char *usage = "Type mktext -h for help";
   char *help = "Usage: mktext [options] src_file1 [src_file2 ..]\n\n"
                "Makes a textfile and header-file(s) from source-file(s) of a certain format.\n"
                "See readme.txt for details.\n\n"
                "The text generation is always performed as two passes. The default mode is however\n"
                "to run both (compile + link). The compile pass checks for syntax correctness and\n"
                "prepares for the checking during the link pass.\n"
                "The link performs the final check (e.g. checking conversion code consistency across\n"
                "different languages) and generates a single output text-file\n\n"
                "Options:\n"
                " -o<PATH>\t<PATH> = The output file where to put the loadable texts (default is stdout)\n"
                " -c\tOnly compile, i.e. header files and intermediate files will be generated. The generated\n"
                "\tfile(s) will get their base names from the source file(s) and with the suffixes \n"
                "\t'.ht' and '.it' respectively.\n"
                " -l\tOnly link. The input files must be intermediate files, '.it', generated by a\n"
                "\tprevious command with a -c option.\n"
                " -nh\tNo header files generated.\n"
                " -p<PATH>\t<PATH> = path where to put the header-file(s). Default path is the direcory of\n"
                "\tthe source files.\n"
                " -i<PATH>\t<PATH> = path where to put the intermediate file(s). Default is current directory.\n"
                " -t\tJust test input files, no files are generated\n"
                " -h\tDisplay this inforamtion\n";

   const char *dname=NULL;
   char path[MAXPATH];
   int i, link = 1, compile = 1, gen_header = 1, test=0;

   allegro_init();
   for (i=1; i<n; i++) {
      if (*strings[i]=='-') {
         if (stricmp(strings[i] + 1, "l") == 0)
            compile = 0;
         else if (tolower(strings[i][1]) == 'c')
            link = 0;
         else if (tolower(strings[i][1]) == 't') {
            link = 0;
            compile = 0;
            test = 1;
         } else if (tolower(strings[i][1]) == 'p')
            strcpy(htpath, strings[i] + 2);
         else if (tolower(strings[i][1]) == 'i')
            strcpy(itpath, strings[i] + 2);
         else if (tolower(strings[i][1]) == 'o')
            dname = strings[i] + 2;
         else if (tolower(strings[i][1]) == 'n' && tolower(strings[i][2]) == 'h')
            gen_header = 0;
         else if (tolower(strings[i][1]) == 'h' || (tolower(strings[i][1]) == '-' && tolower(strings[i][1]) == 'h')) {
            allegro_message(help);
            return 0;
         } else {
            allegro_message("Unkown option to mktext (%s)\n%s\n", strings[i],
                            usage);
            return -1;
         }
      }
   }
   if (compile == 0 && link == 0 && !test)
      link = compile = 1;
   if (!compile && link) {
      if (*htpath) {
         allegro_message("Option -hXXX is ignored when -l is specified\n");
      } else if (*itpath) {
         allegro_message("Option -iXXX is ignored when -l is specified\n");
      } else if (gen_header == 0) {
         allegro_message("Option -nd is ignored when -l is specified\n");
      }
   }
   if (!gen_header && *htpath) {
      allegro_message("Use of option -hXXX is overridden by -nh\n");
   }
   srcs = GetMem0(t_files, 1);
   for (i=1; i<n; i++)
      if (*strings[i]!='-')
         for_each_file_ex(strings[i], ALLFILES, 0, get_fn, srcs);

   if (dname == NULL && link && !test) {
      allegro_message("No destination file specified\n%s\n", usage);
      free(srcs);
      return -1;
   }
   if (srcs->n == 0) {
      allegro_message("No source file specified\n%s\n", usage);
      free(srcs);
      return -1;
   }

   imed = GetMem0(struct t_ifile*, srcs->n);

   if (compile) {
      for (i=0; i < srcs->n; i++) {
         tl = create_token_list();
         if (scan_file(srcs->fn[i], tl) == 0) {
            allegro_message("Source-file %s was not found\n", srcs->fn[i]);
         }
         t = create_tree();
         parse(tl, t, srcs->fn[i]);
         destroy_token_list(tl);
         imed[i] = create_imed_file_image();
         build_intermediate(t, imed[i]);

         if (!link) {
            if (!print_intermediate_file(mk_fn(path, itpath, srcs->fn[i], "it"), imed[i])) {
               allegro_message("Failed to create intermediate file %s\n", path);
            }
         }

         build_header(t, srcs->fn[i]);
         if (gen_header) {
            if (!print_header_file(mk_fn(path, htpath, srcs->fn[i], "ht"), t)) {
               allegro_message("Failed to create header-file %s\n", path);
            }
         }
         check_hot_keys_in_file(t);
         destroy_tree(t);
      }
   } else if (test) {
      for (i=0; i < srcs->n; i++) {
         tl = create_token_list();
         if (scan_file(srcs->fn[i], tl) == 0) {
            allegro_message("Source-file %s was not found\n", srcs->fn[i]);
         }
         t = create_tree();
         parse(tl, t, srcs->fn[i]);
         destroy_token_list(tl);
         imed[i] = create_imed_file_image();
         build_intermediate(t, imed[i]);
         build_header(t, srcs->fn[i]);
         check_hot_keys_in_file(t);
         destroy_tree(t);
      }
      it = create_itree();
      for (i=0; i < srcs->n; i++) {
         if (parse_intermediate_data(imed[i], it) == 0) {
            allegro_message("Intermediate-file %s is corrupt\n", srcs->fn[i]);
         }
      }
      if (check_lang_equal(it, "<en>"))
         allegro_message("Check is OK\n");
      destroy_itree(it);
   } else {
      for (i=0; i < srcs->n; i++) {
         imed[i] = create_imed_file_image();
         if (load_intermediate_file(srcs->fn[i], imed[i]) == 0) {
            allegro_message("Intermediate-file %s was not found\n", srcs->fn[i]);
         }
      }
   }
   if (link) {
      it = create_itree();
      for (i=0; i < srcs->n; i++) {
         if (parse_intermediate_data(imed[i], it) == 0) {
            allegro_message("Intermediate-file %s is corrupt\n", srcs->fn[i]);
         }
      }
      check_lang_equal(it, "<en>");
      print_final_text(dname, it);
      destroy_itree(it);
      allegro_message("Done\n");
   }
   for (i=0; i < srcs->n; i++)
      destroy_imed_file_image(imed[i]);
   free(imed);
   for (i=0; i < srcs->n; i++)
      free(srcs->fn[i]);
   if (srcs->fn)
      free(srcs->fn);
   free(srcs);
   return 0;
}
END_OF_MAIN();
