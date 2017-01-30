#ifndef LANG_H
#define LANG_H

#include <stdio.h>

struct t_language;
struct t_section;
struct t_ifile;

extern void hook_section(struct t_section *sec, struct t_language *lang);
extern void destroy_language(struct t_language *lang);

extern void build_lang_header(struct t_language *lang, const char *fn);
extern void build_lang_intermediate(struct t_language *lang, struct t_ifile *imed);
extern void output_lang_header(FILE *f, struct t_language *lang);
extern int check_hot_keys_lang(struct t_language *lang);

#endif
