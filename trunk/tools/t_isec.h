#ifndef ISEC_H
#define ISEC_H
#include <stdio.h>

struct t_isec;
struct t_ilang;
struct t_istring;

extern struct t_isec *insert_isec(struct t_ilang *il, char *secname, char *fn);
extern void destroy_isection(struct t_isec *is);
extern void add_imacro(struct t_isec *is, char *macrodef, int srcrow);
extern void hook_istring(struct t_isec *is, struct t_istring *str);

extern const char *isection_name(struct t_isec *is);
extern int equal_isection_name(struct t_isec *is, const char *name);
extern int isec_eq(struct t_isec *is, struct t_isec *ref);
extern void print_final_text_section(FILE *f, struct t_isec *is);
extern char *isec_file(struct t_isec *is);
extern int sort_isec(const void *s1, const void *s2);

#endif
