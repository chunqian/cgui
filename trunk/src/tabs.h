#ifndef TABS_H
#define TABS_H

struct t_object;

typedef struct t_tab {
   struct t_object *b;
   int selected;
   struct t_tabwin *tw;
   void (*callback) (void *data, int mb);
   void (*Free) (struct t_object *);
   void (*Unlink) (struct t_object *);
   void *data;
} t_tab;

extern void MakeTabWin(struct t_object *b);
#endif
