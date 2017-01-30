#ifndef TEXT_H
#define TEXT_H

#include <stdio.h>

struct t_ifile;
struct t_inode;

extern struct t_ifile *create_imed_file_image(void);
extern void destroy_imed_file_image(struct t_ifile *imed);
extern void add_intermediate_header_row(struct t_ifile *imed, char *label, int srcrow);
extern void add_intermediate_text_row(struct t_ifile *imed, char *row, int srcrow);
extern void add_intermediate_language_row(struct t_ifile *imed, char *id);
extern void add_intermediate_section_row(struct t_ifile *imed, char *sec, char *fn);
extern int print_intermediate_file(const char *fn, const struct t_ifile *imed);
extern int parse_intermediate_data(struct t_ifile *imed, struct t_inode *it);
extern int load_intermediate_file(const char *fn, struct t_ifile *imed);

#endif
