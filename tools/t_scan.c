/* Scans a text source file, generating a token list, as intermediate
   reprsentation of the source file */

#include <stdio.h>
#include <allegro.h>
#include "cgui/mem.h"
#include <string.h>
#include <ctype.h>

#include "t_parser.h"
#include "t_scan.h"

#if (ALLEGRO_SUB_VERSION < 2) || ((ALLEGRO_SUB_VERSION == 2) && (ALLEGRO_WIP_VERSION == 0))
#define file_size_ex file_size
#endif

typedef struct t_token {
   int type;
   char *name;
   int line_nr;
} t_token;

typedef struct t_tokenlist {
   int n;
   t_token *t;
   t_token *cur;
   const char *fn;
} t_tokenlist;

typedef struct t_lex {
   int tok;
   char *pat;
} t_lex;

#define NRLEXEMES 3
t_lex lex[NRLEXEMES] = {{LANGUAGE, "language"}, {START, "start"}, {END, "end"}};

extern t_tokenlist *create_token_list(void)
{
   t_tokenlist *tl;

   tl = GetMem0(t_tokenlist, 1);
   return tl;
}

extern void destroy_token_list(t_tokenlist *tl)
{
   int i;

   for (i = 0; i < tl->n; i++)
      if (tl->t[i].name)
         free(tl->t[i].name);
   free(tl->t);
   free(tl);
}

static char *revert_token(int type)
{
   switch (type) {
   case DONE:
      return "end of file";
   case START:
      return "start";
   case END:
      return "end";
   case ID:
      return "identifier";
   case STRING:
      return "string";
   case LANGUAGE:
      return "language";
   case ERROR:
      return "unknown pattern";
   case NUM:
      return "numeric value";
   case LT:
      return "<";
   case GT:
      return ">";
   case EQ:
      return "=";
   case LB:
      return "[";
   case RB:
      return "]";
   default:
      return "internal error!";
   }
}

extern void match(t_tokenlist *tl, int type)
{
   unsigned i;

   if (token_type(next_token(tl)) == type)
      tl->cur++;
   else {
      allegro_message(
              "Syntax error in line %d in file %s. Expected \"%s\" but\n"
              "found %s \"%s\"",
              tl->cur->line_nr, tl->fn, revert_token(type),
              revert_token(tl->cur->type),
              tl->cur->name?tl->cur->name:"");
      exit(-1);
   }
}

extern t_token *first_token(t_tokenlist *tl)
{
   tl->cur = tl->t;
   return tl->t;
}

extern t_token *next_token(const t_tokenlist *tl)
{
   return tl->cur;
}

extern struct t_token *token_k(int k, const struct t_tokenlist *tl)
{
   return tl->cur + k - 1;
}

extern char *token_name(const t_token *t)
{
   return t->name;
}

extern int token_type(const t_token *t)
{
   return t->type;
}

extern int token_row(const t_token *t)
{
   return t->line_nr;
}

static void insert_token(t_tokenlist *tl, int type, char *name, int line_nr)
{
   tl->n++;
   tl->t = ResizeMem(t_token, tl->t, tl->n);
   tl->t[tl->n - 1].line_nr = line_nr;
   tl->t[tl->n - 1].name = MkString(name);
   tl->t[tl->n - 1].type = type;
}

extern int scan_file(const char *fn, t_tokenlist *tl)
{
   FILE *f;
   char *s, tmps[10000];
   int n, i, j, len, line_nr = 1;

   tl->fn = fn;
   f = fopen(fn, "rb");
   if (f) {
      n = file_size_ex(fn);
      s = GetMem(char, n + 1);
      s[n] = 0;
      fread(s, n, 1, f);
      fclose(f);
      i = 0;
      for (i=0; i<n; ) {
         /* skip space */
         for (; i < n && isspace(s[i]); i++)
            if (s[i] == '\n')
               line_nr++;
         switch (s[i]) {
         case '<':
         case '>':
         case '=':
         case '[':
         case ']':
         case '{':
         case '}':
            insert_token(tl, s[i], "", line_nr);
            i++;
            break;
         default:
            if (s[i] == '/' && s[i + 1] == '*') {
               for (i += 2; i + 1 < n && !(s[i] == '*' && s[i + 1] == '/'); i++)
                  ;
               i += 2;
            } else if (s[i] == '/' && s[i + 1] == '/') {
               for (; i < n && s[i] != '\n'; i++)
                  ;
            } else if (s[i] == '"') {
               for (i++, j = 0; i < n && s[i] != '"'; i++, j++) {
                  if (s[i] == '\\')
                     i++;
                  tmps[j] = s[i];
               }
               if (s[i] == '"') {
                  tmps[j] = 0;
                  insert_token(tl, STRING, tmps, line_nr);
                  i++;
               }
            } else if (s[i]) {
               for (j = 0; j < NRLEXEMES; j++) {
                  len = strlen(lex[j].pat);
                  if (strncmp(s + i, lex[j].pat, len) == 0) {
                     if (!(s[i + len] == '_' || isalnum(s[i + len]))) {
                        insert_token(tl, lex[j].tok, "", line_nr);
                        i += len;
                        break;
                     }
                  }
               }
               if (j >= NRLEXEMES) {
                  if (s[i] == '_' || isalpha(s[i])) {
                     for (j = 0; i < n && (isalnum(s[i]) || s[i] == '_'); i++, j++)
                        tmps[j] = s[i];
                     tmps[j] = 0;
                     insert_token(tl, ID, tmps, line_nr);
                  } else if (isdigit(s[i])) {
                     for (j = 0; i < n && isdigit(s[i]); i++, j++)
                        tmps[j] = s[i];
                     tmps[j] = 0;
                     insert_token(tl, NUM, tmps, line_nr);
                  } else {
                     tmps[0] = s[i];
                     tmps[1] = 0;
                     insert_token(tl, ERROR, tmps, line_nr);
                     i++;
                  }
               }
            }
         }
      }
      free(s);
      insert_token(tl, DONE, "", line_nr);
      return 1;
   }
   insert_token(tl, DONE, "", line_nr);
   return 0;
}
