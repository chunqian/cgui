/* Parsing a list of tokens from a text source-file. Uses k-lookahed,
   because I'm tolazy to make left-factoring. Generates a kind of "tree"
   representing the source-file. */

#include <stdio.h>
#include "t_tree.h"
#include "t_scan.h"
#include "t_parser.h"
#include "t_sec.h"

static void get_array_end(struct t_tokenlist *tl, struct t_array *arr)
{
   struct t_token *t;

   match(tl, LT);
   match(tl, END);
   match(tl, EQ);
   t = next_token(tl);
   match(tl, ID);
   match(tl, GT);
   add_array_end(token_name(t), arr, token_row(t));
}

static void get_labels(struct t_tokenlist *tl, void *labelkeeper,
       struct t_label* (*insert)(char *m, char *s, void *d, int line, struct t_block *bl),
       struct t_block *bl)
{
   struct t_token *t, *t1, *t2;
   struct t_label *label;

   for (t1 = next_token(tl); token_type(t1) == ID; t1 = next_token(tl)) {
      match(tl, ID);
      t2 = next_token(tl);
      match(tl, STRING);
      label = insert(token_name(t1), token_name(t2), labelkeeper, token_row(t1), bl);
      for (t = next_token(tl); token_type(t) == STRING; t = next_token(tl)) {
         concat_string(token_name(t), label);
         match(tl, STRING);
      }
   }
}

static struct t_array *get_array_name(struct t_tokenlist *tl, struct t_section *sec)
{
   struct t_token *t;

   match(tl, LT);
   match(tl, START);
   match(tl, EQ);
   t = next_token(tl);
   match(tl, ID);
   match(tl, GT);
   return insert_array(token_name(t), sec, token_row(t));
}

static void get_label_array(struct t_tokenlist *tl, struct t_section *sec, struct t_block *bl)
{
   struct t_array *arr;

   arr = get_array_name(tl, sec);
   get_labels(tl, arr, insert_array_label, bl);
   get_array_end(tl, arr);
}

static void get_block(struct t_tokenlist *tl, struct t_section *sec, struct t_block *root)
{
   struct t_token *t;
   struct t_block *bl;
   int end;

   match(tl, BLOCKSTART);
   bl = create_block();
   insert_block_node(root, bl);
   for (t = next_token(tl), end = 0; !end; t = next_token(tl)) {
      switch (token_type(t)) {
         case LT:
            get_label_array(tl, sec, bl);
            break;
         case ID:
            get_labels(tl, sec, insert_sec_label, bl);
            break;
         case BLOCKSTART:
            get_block(tl, sec, bl);
            break;
         default:
            end = 1;
            break;
      }
   }
   match(tl, BLOCKEND);
}

static struct t_language *get_language(struct t_tokenlist *tl, struct t_node *tree)
{
   struct t_token *t;

   match(tl, LT);
   match(tl, LANGUAGE);
   match(tl, EQ);
   t = next_token(tl);
   match(tl, ID);
   match(tl, GT);
   return insert_language(token_name(t), tree);
}

static struct t_section *get_section(struct t_tokenlist *tl, struct t_language *lang, char *fn)
{
   struct t_token *t;

   match(tl, LB);
   t = next_token(tl);
   match(tl, ID);
   match(tl, RB);
   return insert_section(token_name(t), lang, fn);
}

extern int parse(struct t_tokenlist *tl, struct t_node *tree, char *fn)
{
   struct t_language *lang = NULL;
   struct t_section *sec = NULL;
   struct t_token *t;
   int end_seq;

   for (t = first_token(tl); token_type(t) != DONE; t = next_token(tl)) {
      if (token_type(t) == LT || lang == NULL)
         lang = get_language(tl, tree);
      if (token_type(next_token(tl)) == LB || sec == NULL)
         sec = get_section(tl, lang, fn);
      for (t = next_token(tl), end_seq = 0; !end_seq; t = next_token(tl)) {
         switch (token_type(t)) {
         case LT:
            if (token_type(token_k(2, tl)) == LANGUAGE)
               end_seq = 1;
            else
               get_label_array(tl, sec, block_root(sec));
            break;
         case ID:
            get_labels(tl, sec, insert_sec_label, block_root(sec));
            break;
         case LB:
         case DONE:
            end_seq = 1;
            break;
         case BLOCKSTART:
            get_block(tl, sec, block_root(sec));
            break;
         default:
            match(tl, ID);
            end_seq = 1;
            break;
         }
      }
   }
   return token_type(t) == DONE;
}
