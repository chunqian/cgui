#include "cgui/mem.h"
#include <allegro.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "t_block.h"
#include "t_tree.h"

typedef struct t_hotkey {
   struct t_label *l;
   int row;
   char *s;
} t_hotkey;

typedef struct t_block {
   int nh;
   struct t_hotkey *hks;
   int nb;
   struct t_block **bl;
} t_block;

extern t_block *create_block(void)
{
   t_block *bl;

   bl = GetMem0(t_block, 1);
   return bl;
}

extern void destroy_block_tree(t_block *root)
{
   int i;

   free(root->hks);
   for (i=0; i<root->nb; i++)
      destroy_block_tree(root->bl[i]);
   free(root);
}

extern void insert_block_node(t_block *root, t_block *newnode)
{
   root->nb++;
   root->bl = ResizeMem(t_block*, root->bl, root->nb);
   root->bl[root->nb - 1] = newnode;
}

extern void insert_block_label(t_block *root, struct t_label *l, char *s, int row)
{
   root->nh++;
   root->hks = ResizeMem(t_hotkey, root->hks, root->nh);
   root->hks[root->nh - 1].l = l;
   root->hks[root->nh - 1].row = row;
   root->hks[root->nh - 1].s = s;
}

static int hot_keys_is_unique(t_block *root, int *keys)
{
   int i, ok = 1, lkeys[256], n = 0;
   char *p;
   t_hotkey *hk;

   for (i=0, hk = root->hks + i; i<root->nh; i++, hk++) {
      for (p = strchr(hk->s, '~'); p; p = strchr(p + 1, '~'))
         if (p[1] == '~')
            p++;
         else
            break;
      if (p) {
         p++;
         lkeys[n++] = (unsigned)toupper(*p);
         if (keys[(unsigned)toupper(*p)]++) {
            allegro_message("Hot-key %c in line %d has been used earlier "
                      "in the same scope\n",
                      *p, hk->row);
            ok = 0;
         }
      }
   }
   for (i=0; i<root->nb; i++)
      ok &= hot_keys_is_unique(root->bl[i], keys);
   for (i=0; i<n; i++)
      keys[lkeys[i]]--;
   return ok;
}

extern int check_hot_keys(t_block *root)
{
   int keys[256], ok = 1, i;
   t_hotkey *hk;
   char *p, *s, *prev = NULL;

   for (i=0, hk = root->hks + i; i<root->nh; i++, hk++) {
      memset(keys, 0, sizeof(int) * 256);
      prev = NULL;
      for (p = hk->s + strcspn(hk->s, "|"); ; p += strcspn(p, "|")) {
         if (*p && p[1] == '|') {
            p++;
         } else {
            if (prev) {
               for (s = strchr(prev, '~'); s && s < p; s = strchr(s + 1, '~')) {
                  if (s[1] == '~')
                     s++;
                  else
                     break;
               }
               if (s && s < p) {
                  s++;
                  if (keys[(unsigned)toupper(*s)]++) {
                     allegro_message("At line %d: hot-key '%c' is used "
                                     "as label\nby more than one button in "
                                     "a requester text.\n", hk->row, *s);
                     ok = 0;
                  }
               }
            }
            if (*p == 0)
               break;
            prev = p;
         }
         p++;
      }
   }
   for (i=0; i<root->nb; i++) {
      memset(keys, 0, sizeof(int) * 256);
      ok &= hot_keys_is_unique(root->bl[i], keys);
   }
   return ok;
}
