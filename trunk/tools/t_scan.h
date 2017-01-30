#ifndef SCANNER_H
#define SCANNER_H

struct t_tokenlist;
struct t_token;

extern void destroy_token_list(struct t_tokenlist *tl);
extern struct t_tokenlist *create_token_list(void);
extern int scan_file(const char *name, struct t_tokenlist *tl);
extern struct t_token *token_k(int k, const struct t_tokenlist *tl);
extern struct t_token *next_token(const struct t_tokenlist *tl);
extern struct t_token *first_token(struct t_tokenlist *tl);
extern char *token_name(const struct t_token *t);
extern int token_type(const struct t_token *t);
extern int token_row(const struct t_token *t);
extern void match(struct t_tokenlist *tl, int type);

#define DONE    0
#define START   1
#define END     2
#define ID      3
#define STRING  5
#define LANGUAGE 6
#define NUM      8
#define ERROR    9
#define LT      '<'
#define GT      '>'
#define EQ      '='
#define LB      '['
#define RB      ']'
#define BLOCKSTART '{'
#define BLOCKEND   '}'

#endif
