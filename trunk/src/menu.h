#ifndef MENU_H
#define MENU_H

struct t_menuitem;
struct t_menu;
struct t_object;
struct t_node;
struct t_window;

typedef struct t_menu {
   struct t_window *topwin;     /* Reference to the top-most window (the
                                   first dropped down menu in a hierarchy) */
   struct t_node *nd;           /* Reference to the node of the menu (the
                                   inner node, where the menu-items live) */
   struct t_menuitem *down;     /* Reference to the menu-item that has opened
                                   a sub-menu. NULL if there is no sub-menu
                                   open */
   struct t_menu *up;           /* Reference to the menu from which this
                                   was opened. NULL if this is the first. */
   int imw1, imw2;              /* These will be used during the process of
                                   building the menu. It is used by the
                                   menu-items to find out which is the width
                                   of the widest icon. */
   void (*Free) (struct t_object *); /* Default free function */
} t_menu;

extern struct t_menu *MenuDropDown(void (*CallBack)(void*), void *data, struct t_window *,
       struct t_menu *prevm, struct t_menuitem *mi);
extern void CloseCurrentMenu(void *data);
extern void PositionWindow(int x, int y1, int y2, struct t_object *b);
extern int CloseOtherSubs(struct t_menu *menu, struct t_menuitem *req);
extern void CloseAllMenus(void *data);

extern struct t_menu *current_open_menu;

#endif
