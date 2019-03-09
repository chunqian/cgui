#ifndef TABWIN_H
#define TABWIN_H

typedef struct t_tabwin {
   struct t_node *top;          /* the top node - containing the other */
   struct t_node *app;          /* the node containing the actual window
                                   objects (for the */
   /* application) */
   struct t_node *head;         /* the header part containing the row of tabs */
   int tabh;
   int new_tab_selected;
   int orig_fixsize;
   int *index;
   void (*Free) (struct t_object *);
   void (*SetSize) (struct t_object *);
} t_tabwin;

#define TABSPACE 5

extern struct t_object *GetTab(struct t_tabwin *tw);
extern void SetTabIndex(struct t_tabwin *tw, struct t_object *b);
extern void ReMakeTabWin(struct t_tabwin *tw);

#endif
