#ifndef NODE_H
#define NODE_H

#include "hotkey.h"
#include "object.h"

/* The node stucture contains info about an internal node of the */
/* object tree. One type of node is a window node, another a menu-dropdown */
/* and a third is a list (actually all these consists of several nodes). */
typedef struct t_node {
   int wx, wy;                  /* Position of the node in the coordinte
                                   system of the window (this is not
                                   necessarily the parent node) */
   int fixsize;                 /* Indicates that the node has fixed size */
   struct t_object *ob;         /* pointer to the actual object of the node
                                   This object ob is part of its the parent
                                   node. */
   void *data;                  /* Pointer to misc. data used by the specific
                                   node */
   struct t_object *firstob;    /* Pointer to the start of the list of
                                   objects within the node (the kids) */
   struct t_object *last;       /* Pointer to the end of the list of objects
                                   within the node (the kids) */
   struct t_object *ip;         /* Pointer to the current insertion point.
                                   next object (kid) that will be created,
                                   will be inserted after the object pointed
                                   to by `ip'. */
   struct t_coord *r;           /* The rectangle list to fill spaces in the
                                   node not used by any object. */
   int nr;                      /* Number of rectangles in r */
   struct BITMAP *bmp;          /* A sub-bitmap on the parent window's
                                   bitmap If the node is the node of a
                                   window this is instead a bitmap. */
   struct t_window *win;        /* Pointer to window of the tree where the
                                   node exists. */
   int leftx, xdist, rightx, topy, dy, boty; /* The distance between objects
                                   when positioned with use of the
                                   "direction commands" */
   int width, height;           /* Initial values of size (only used when
                                   fix-size node) */
   /* This flag tells the freeer that a callback is in progress (the called */
   /* function is allowed to release the object tree or a part of it). In
      such */
   /* a case the free function must wait and release the memory later on */
   /* immediately. The callback processing in turn must be interrupted */
   /* immediately */
   int processing_callback;
   /* Copies of the original object-pointers */
   /* The pointer in the object-struct should normally be replaced by
      recursive functions. Sometimes the originals will be useful to have. */
   void (*Position) (struct t_object *);
   void (*SetSize) (struct t_object *);
   void (*Pack) (struct t_object *);
   void (*EqualSize) (struct t_object *);
   struct t_typefun *tf;
   struct t_mevent *me;
   /* A function that draws the shape of the node (usually some kind of
      frame) */
   void (*Draw) (struct t_object *);
   void (*AppUpd) (int id, void *data, void *calldata, int reason);
   void *appupddata;
   int type;
   struct t_image *image; /* A list of images associated to this node, to be
                             used by any descendant */
} t_node;

extern t_typefun xtended_node_type_functions, default_node_type_functions;
extern t_hkfun default_node_hotkey_functions;
extern t_node *fwin, *opwin;

struct t_mevent;

extern void SizeParentNode(struct t_object *b);
extern void CleanNode(t_node * node);
extern t_node *MkNode(t_node * parent, int x, int y, int width, int height,
                      void (*DrawSw) (struct t_object *));
extern t_node *GetNode(int id);
extern void CloseNode(void);
extern void DisplayNode(t_node * nd);
extern void MakeNodeRects(t_node * node, struct t_coord *r0);
extern void SetNodeStateMachine(struct t_mevent * node_me);
extern void SetFixSizeNode(t_node * nd, int width, int height);
extern void SetAdaptiveNode(t_node *nd);
extern void NoAdapt(struct t_object *b0, struct t_object *b);
extern void InitNode(void);

#define NODETYPE_DEFAULT   0
#define NODETYPE_WINDOW    1
#define NODETYPE_TAB       2
#define NODETYPE_LIST      3
#define NODETYPE_CONT      4

#endif
