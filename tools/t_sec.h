#ifndef SEC_H
#define SEC_H

struct t_section;
struct t_array;
struct t_header;
struct t_ifile;

extern struct t_section *insert_section(char *name, struct t_language *lang, char *fn);
extern void destroy_section(struct t_section *sec);
extern void hook_array(struct t_array *a, struct t_section *sec);
extern void build_section_header(struct t_header *hdr, struct t_section *sec);
extern void build_section_intermediate(struct t_section *sec, struct t_ifile *imed);
extern struct t_block *block_root(struct t_section *sec);
extern int check_hot_keys_sec(struct t_section *sec);

#endif
