#ifndef ITREE_H
#define ITREE_H

#include <stdio.h>

struct t_inode;
struct t_ilang;

extern struct t_inode *create_itree(void);
extern void destroy_itree(struct t_inode *);
extern void hook_ilang(struct t_inode *it, struct t_ilang *il);
extern struct t_ilang *get_ilang_ref(struct t_inode *it, char *lang);
extern int check_lang_equal(struct t_inode *it, char *reflang);
extern int print_final_text(const char *fn, const struct t_inode *it);

#endif
