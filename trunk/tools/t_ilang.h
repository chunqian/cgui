#ifndef ILANG_H
#define ILANG_H

#include <stdio.h>

struct t_ilang;
struct t_inode;
struct t_isec;

extern void destroy_ilang(struct t_ilang *il);
extern struct t_ilang *insert_ilang(struct t_inode *it, char *lang);
extern int equal_ilang_name(struct t_ilang *il, char *lang);
extern void hook_isec(struct t_ilang *il, struct t_isec *is);
extern int ilang_eq(struct t_ilang *il, struct t_ilang *ref);
extern void print_final_text_lang(FILE *f, const struct t_ilang *il);

#endif
