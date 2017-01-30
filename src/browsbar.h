#ifndef BROWSBAR_H
#define BROWSBAR_H

#define BROWSERWIDTH 17

struct t_browsebar;

extern struct t_browsebar *CreateVerticalBrowseBar(int x, int y,
      int *pos, int *id, void (*CallBack) (void *data), void *data);
extern struct t_browsebar *CreateHorizontalBrowseBar(int x, int y,
      int *pos, int *id, void (*CallBack) (void *data), void *data);
extern int NotifyBrowseBar(struct t_browsebar *bb, int scrolled_area_length);
extern void SetBrowseBarSize(struct t_browsebar *bb, int oblen, int bblen);

#endif
