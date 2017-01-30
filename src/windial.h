#ifndef WINDIAL_H
#define WINDIAL_H

typedef struct t_windial t_windial;

struct t_windial {
   void (*Cancel) (void *);     /* Pointer to the Cancel function (optional) */
   void (*Ok) (void *);         /* Pointer to the Ok function (optional) */
   void *data;                  /* The data pointer to be passed to Cancel
                                   and/or OK-pointers. */
   int helpref;
};

#endif
