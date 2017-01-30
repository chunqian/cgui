#ifndef SCROLL_H
#define SCROLL_H

#define SC_VERTICAL 0
#define SC_HORIZONTAL 1

struct t_scroll;
extern int NotifyScroller(struct t_scroll *scr, int active);
extern void SetScrollSize(struct t_scroll *scr, int len);
extern struct t_scroll *CreateScrollDownButton(int x, int y,
                                 void (*CallBack) (void *data), void *data);
extern struct t_scroll *CreateScrollUpButton(int x, int y,
                                 void (*CallBack) (void *data), void *data);
extern struct t_scroll *CreateScrollLeftButton(int x, int y,
                                 void (*CallBack) (void *data), void *data);
extern struct t_scroll *CreateScrollRightButton(int x, int y,
                                 void (*CallBack) (void *data), void *data);

#endif
