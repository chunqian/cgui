#ifndef MENUITEM_H
#define MENUITEM_H

struct t_menu;
struct t_object;

typedef struct t_menuitem {
   struct t_menu *menu;         /* Reference the menu in which this item
                                   exists. */
   int issub;                   /* Flag telling if this item is
                                   able to drop a new sub-menu. */
   void (*CallBack) (void *);   /* Application call-back. If issub is true
                                   this will be called when time for
                                   dropping down the sub-menu. If not
                                   it will be called when the user selects
                                   the item (it is an "action-item") */
   void *data;                  /* Application data (corresponding to the
                                   above call-back). */
   void (*AdditionalCB) (void *);/* Addittional call-back for the application.
                                    Can be used with `AddHandler' */
   void *adddata;               /* Application data (for the above */
   struct t_menu *submenu;      /* Reference to an open sub-menu. NULL when
                                   not open. */
   struct t_object *b;          /* Reference to the object of the menu-item */
   int seqno;                   /* Sequence number within a group of
                                   radiobuttons. */
   int *indicateval;            /* Pointer to the radio- or check value
                                   (application data). This will be NULL if the
                                   menu-item is not a radio- or check item. */
   char *sc;                    /* Short-cut text */
   void (*Free) (struct t_object *); /* Default free function */
} t_menuitem;

extern void ResetCatcher(struct t_menuitem *m);
extern struct t_object *MkMenuObject(const char *text, const char *shortcut);
extern struct t_object *MkMenuObjectTop(int x, int y, const char *text);
extern void InitMenuItems(void);
extern void DrawMenuItem(struct t_object *b);
#define MenuItemFrameIn(bmp, x1, y1, x2, y2) \
      hline(bmp, x1, y1,   x2,   cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);      \
      vline(bmp, x1, y1+1, y2-1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);      \
      hline(bmp, x1, y2,   x2,   cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);   \
      vline(bmp, x2, y1+1, y2-1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);

#define MenuItemFrameOut(bmp, x1, y1, x2, y2)   \
      hline(bmp, x1, y1,   x2,   cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);      \
      vline(bmp, x1, y1+1, y2-1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);      \
      hline(bmp, x1, y2,   x2,   cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);         \
      vline(bmp, x2, y1+1, y2-1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);        \
      rect(bmp, x1,  y1, x2, y2, cgui_colors[CGUI_COLOR_WIDGET_BACKGROUND]);
#define MENU_LABEL_SPACE   8
#define MENU_DROP_ARROW_WIDTH 16

#include "object.h"
extern t_typefun default_menu_item_type_functions;

#endif
