#ifndef ISTRING_H
#define ISTRING_H
#include <stdio.h>

struct t_istring;
struct t_isec;

extern void add_istring(struct t_isec *is, char *s, int rn);
extern void destroy_istring(struct t_istring *str);
extern int istring_eq(struct t_istring *str, struct t_istring *ref);
extern void print_final_text_string(FILE *f, struct t_istring *str);

#endif
