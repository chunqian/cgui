#ifndef LABEL_H
#define LABEL_H

struct t_label;
struct t_header;
struct t_ifile;
struct t_block;

extern struct t_label *create_label(char *macro, char *string, int line, struct t_block *bl);
extern void destroy_label(struct t_label *l);
extern void concat_string(char *name, struct t_label *label);
extern void build_label_header(struct t_header *hdr, struct t_label *l, int i);
extern void build_label_intermediate(struct t_ifile *imed, struct t_label *l);
extern void build_header_intermediate(struct t_ifile *imed, struct t_label *l);

#endif
