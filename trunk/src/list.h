#ifndef LIST_H
#define LIST_H

struct t_listchain;
struct t_object;
struct FONT;
struct BITMAP;

/* a future application function ?? */
extern int ListShowFocus(int listid, int showfocus);
extern void InitList(void);
extern void FixTextOut(struct BITMAP *bmp, struct FONT *font, const char *s, int x, int y, int color, int bcolor, int fixwidth);

/* A reference to the list object that contains the row */

#endif
