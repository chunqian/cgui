#ifndef OBDROP_H
#define OBDROP_H

struct t_ddrow;
struct t_object;

typedef struct t_dropbox {
   void (*Action) (void *);     /* Optional user call back installed by AddHandler */
   void *actdata;               /* Data to be passed to the above call back. */
   int n;                       /* Max number of elements in the drop-down list */
   const int *np;               /* Pointer to max number of elements in the drop-down list */
   int width;                   /* The width in pixels. */
   int *sel;                    /* A pointer to an integer holding the index of the selected item. */
   struct t_object *b;          /* Pointer to the generic widget object. */
   void (*RowCallBack) (const void *listdata, int i, char *s); /* Pointer to a function called once per row in the drop down list. */
   const void *listdata;        /* Pointer to some data passed to the above. */
   const void *(*Iterate) (const void *listdata, const void *it, char *text); /* Alternative row call back in case of linked list. */
   /* A list of temporary row data. The memory is needed while the box is dropped down. */
   struct t_ddrow *garbage;
   int ngarbage;
} t_dropbox;

extern t_dropbox *CreateDropDownData(struct t_object *editbox, int *sel, const char * const *strings, const int *n);
extern void DropDown(t_dropbox *dd);

#endif

