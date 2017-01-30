#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>

struct t_header;

extern struct t_header *create_header(const char *fn);
extern void destroy_header(struct t_header *hdr);
extern void print_header(FILE *f, struct t_header *hdr);
extern void add_header_row(struct t_header *hdr, char *row);

#endif
