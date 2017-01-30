#ifndef  NDRESIZE_H
#define  NDRESIZE_H

struct t_object;
extern int MakeNodeStretchable(struct t_object *b, void (*Notify) (void *),
                               void *data, int options);
extern int SetStretchMin(int id, int w, int h);

#endif
