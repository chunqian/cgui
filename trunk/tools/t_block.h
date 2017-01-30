#ifndef BLOCK_H
#define BLOCK_H

struct t_block;
struct t_label;

extern struct t_block *create_block(void);
extern void destroy_block_tree(struct t_block *root);
extern void insert_block_node(struct t_block *root, struct t_block *newnode);
extern void insert_block_label(struct t_block *root, struct t_label *l, char *hotkey, int row);
extern int check_hot_keys(struct t_block *root);

#endif
