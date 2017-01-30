#ifndef OBSIZER_H
#define OBSIZER_H

typedef struct t_stretch {
   struct t_object *b;
   void (*Notify) (void *);
   void *data;
   int options;
   int dir;
   struct t_object *v, *h, *d;
} t_stretch;

extern void FreeResizer(void *data);
extern int MakeObjectStretchable(struct t_object *b,
                                 void (*Notify) (void *), void *data,

                                 int options);

#endif
