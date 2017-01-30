#ifndef TREE_H
#define TREE_H

struct t_node;
struct t_label;
struct t_language;
struct t_section;
struct t_array;
struct t_ifile;
struct t_block;

extern struct t_node *create_tree(void);
extern void destroy_tree(struct t_node *t);
extern struct t_language *insert_language(char *name, struct t_node *tree);
extern struct t_label *insert_array_label(char *macro, char *string, void *data, int line, struct t_block *bl);
extern struct t_array *insert_array(char *name, struct t_section *sec, int line);
extern struct t_label *insert_sec_label(char *macro, char *string, void *data, int line, struct t_block *bl);
extern void add_array_end(char *name, struct t_array *arr, int line);
extern void concat_string(char *name, struct t_label *label);
extern struct t_block *create_block(void);
extern void insert_block_node(struct t_block *root, struct t_block *newnode);

extern int print_header_file(const char *fn, const struct t_node *t);

extern void build_header(struct t_node *tree, const char *fn);
extern void build_intermediate(struct t_node *t, struct t_ifile *ifil);

extern int check_hot_keys_in_file(struct t_node *t);

#endif
