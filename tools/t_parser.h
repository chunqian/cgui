#ifndef PARSER_H
#define PARSER_H

struct t_node;
struct t_tokenlist;

extern int parse(struct t_tokenlist *tl, struct t_node *tree, char *fn);

#endif
