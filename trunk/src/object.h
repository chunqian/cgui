#ifndef OBJECT_H
#define OBJECT_H

struct t_coord;
struct t_object;
struct t_image;
struct BITMAP;
struct t_refresh_traversor;

/* These functions may differ between different type of objects - i.e. the */
/* push-buttons may have one set, and exitboxes another. */
typedef struct t_typefun {
   /* Creates the object, i.e. set the size of it */
   void (*SetSize) (struct t_object *);
   void (*Stretch) (struct t_object *, int, int);
   /* Re-sets image references if the images have been re-loaded */
   void (*CatchImage) (struct t_object *);
   /* Changes the data that is used to view the object. Can be used to change */
   /* the label text, or to animate. */
   int (*ChangeData) (struct t_object *, void *, int);
   /* Unlinks an object properly from the linked list and if necessary
      updates pointer of the parent */
   void (*Unlink) (struct t_object *);
   /* Free shall release all memory associated with the object, and destroy
      all references to the objects and return the id-nr allocated, but must
      not unlink object from the list. The freeing also applies to its
      possible descendants. */
   void (*Free) (struct t_object *);
   /* Remove shall Unlink object, Free and then update screen */
   void (*Remove) (struct t_object *);
   /* Set the "active" flag of the object */
   void (*Activate) (struct t_object *);
   /* Set the object to inactive */
   void (*DeActivate) (struct t_object *);
   /* Redraws the object on its bitmap - no update of the screen */
   void (*Draw) (struct t_object *);
   /* Re-draws and blits to screen */
   void (*Refresh) (struct t_object *);
   /* Installs an extra application handler */
   int (*AddHandler) (struct t_object *, void (*cb) (void *), void *);
   /* A function that fetches data set by the application. Can be used to
      transfer data in a secure way with use of hand-shaking */
   int (*FetchData)(struct t_object *);
   /* Will create the bitmaps after the build process is ready. All objects */
   /* must firts be in there proper positions to make it possible to create */
   /* bitmaps of the right size. This must be run from top to bottom, since a */
   /* node has a sub-bitmap in the bitmap of its parent. */
   /* This function also generate the rectangle list of the node, so it is */
   /* necessary that all objects has there dimensions and positions */
   /* determined. */
   void (*MakeBitmap) (struct t_object *);
   void (*DestroyBitmap) (struct t_object *);
   void (*MakeNodeRects) (struct t_object *);
   /* Returns the node of an object. A simple objects will return their */
   /* parent node. */
   /* Take focus is a function that takes over as the current active window. */
   void (*TakeFocus) (struct t_object *);
   /* Promotes window to be the top-window */
   int (*MoveWin) (struct t_object *);
   /* Some complex object contructs like list-boxes, containers etc. may be */
   /* built from several recursive nodes. If the 'id' returned to the */
   /* application referes to an inner-node, but a certain subsequent calls */
   /* from the application concerns the outermost node, then this function */
   /* will return the "top"-node in the hierachy of nodes. A typical object */
   /* is a "container" and a typical command is "make stretcher". */
   struct t_object *(*GetMainNode) (struct t_object *);
   int (*MakeStretchable) (struct t_object *, void (*)(void *), void *,
                           int);
   int (*DoJoinTabChain)(struct t_object *b);
   struct t_coord *(*InsertArea)(struct t_object *b, struct t_coord *rects, int *n, int *m);
   void (*GetSizeOffset)(struct t_object *b, int *w, int *h);
   void (*SetSizeOffset)(struct t_object *b, int w, int h);
   void (*SetView)(struct t_object *b, int flags);
   void (*PrepareNewScreenMode)(struct t_object *b);
   void (*NewScreenMode)(struct t_object *b);
   /* Type-specific functions for focusing: */
   void (*SetFocus) (struct t_object *b);
   void (*UnSetFocus) (struct t_object *b);
} t_typefun;

/* The object structure. */
typedef struct t_object {
   int x1, y1, x2, y2;          /* The object location within the bitmap of
                                   the parent node. In case the parent node
                                   has no bitmap (i.e. the object is a window)
                                   then this is the location within the screen */
   int x, y;                    /* These are mouse-coordinates for the latest
                                   hit/grep/release */
   int rex, rey;                /* Size offset. */
   int id;                      /* A key identifying the object */
   int state;                   /* The state-varaible that indicates the
                                   current mouse state. The meaning of the
                                   state enumarations may vary between
                                   different object types (it is depending on
                                   which state machine that has been assigned
                                   to the object type. However the initial
                                   state must always be encoded to 0 */
   struct t_tabchain *tablink;  /* A reference to the tab-chain where the
                                   object is connected. In the case when a
                                   node-object is connected to the chain, all
                                   its children referes to the same link.
                                   NULL if there not joining the tab-chain */
   struct t_mousefocus *md;     /* A pointer to the mouse-data struct.
                                   Set to NULL initially. Can be used freely
                                   by the mouse-state-machines */
   struct t_node *parent;       /* Reference to the node where the object is
                                   located. */
   struct t_object *next;       /* Pointer next object within the same node,
                                   normally the subsequent issued one. NULL
                                   if last. */
   struct t_object *prev;       /* Pointer to previous object in the node,
                                   normally the one issued prior to the
                                   current. NULL if first. */
   void *appdata;               /* Data item for general use for the
                                   application. Some object types may wrap
                                   the application data into its own data
                                   structure and let appdata pointe to the
                                   latter, in which case the object is
                                   responsible for that. */
   struct t_node *node;         /* Pointer to the node-struct if the object
                                   is a node. Leafs are indicated with
                                   node == NULL */

   /* Below 4 tables of function pointer specifying the properties of the
      object. */

   /* `me' handles the mouse events (the mouse state machine) */
   struct t_mevent *me;
   /* This contains pointers to all functions that is common for a certain
      type of object */
   struct t_typefun *tf;
   /* This contains pointers to the functions that specifies the hot-key
      behaviour */
   struct t_hkfun *hkfunc;
   /* This contains pointers to the functions that specifies the tab-chain
      behaviour */
   struct t_tcfun *tcfun;

   /* Below a number of function-pointers to internal functions. The
      combination of functions pointed to does not follow the pattern of any
      of the functions tables above, and the number of permutations is quite
      large, hence they are not included in any functio-pointer table. */

   /* `EqualSize' will set the size of a sequence of objects to the size of
      the largest one in the sequence. A sequence is all objects in the same
      node, that has been issued sequentially, and with the same top or left
      coordintaes (depeneding on if the equalization will be done vertially
      or horizontally), and with the same EqualSize. */
      void (*EqualSize) (struct t_object *);
   /* `Position' will put the object into its specified position related to
      other objects or to the window. If parent window is self-sizeable this
      will be successively adjusted to be able to view all its objects. */
   void (*Position) (struct t_object *);
   /* `Pack' will make use of overflow space in som manner,
      often by extending the objects (FILL... parameter) or by adjusting
      object posiitons to distribute the space evenly between objects in a
      sequence */
   void (*Pack) (struct t_object *);
   /* `Align' will align the objects position ont the top and/or right
      edge) of its parent node. */
   void (*Align) (struct t_object *);
   /* `Adapt' will adapt the size of the parent node of "b" to
      ensure that it will be large enough to show entire b. */
   void (*Adapt) (struct t_object *parent, struct t_object *b);

   /* Application callbacks and data: */
   void *gripdata;         /* The data passed to the grip-function (and
                              to the drop-function as a represemtation of
                              the dropped object) */
   void *dropdata;         /* The data passed to the drop-function */
   void *slidedata;        /* The data passed to the slide-function */
   void *resizedata;       /* The data passed to the slide-function */
   void *data2;            /* The data passed to Action2 when double-clicked*/

   /*`Action' handles the "click" event (or corresponding key-press).*/
   void (*Action) (void *);
   /*`Action2' handles the "double-click" event (or corresp. key-press).*/
   void (*Action2) (void *);
   /* Called when the object gets grip-events */
   void *(*Grip) (void *src, int id, int reason);
   /* Called when the object gets drop-events */
   int (*Drop) (void *dest, int id, void *src, int reason, int flags);
   /* Called when the object gets slide-events */
   int (*Slide) (int x, int y, void *src, int id, int reason);
   /* Called when the object gets resize-events */
   int (*ReSize) (int x, int y, void *src, int id, int reason);
   /* Called when the object enters or leaves the "moues over" state */
   void (*Over)(struct t_object *b);
   /* Called when the object gets update-events, launched by the application*/
   void (*AppUpd) (int id, void *data, void *calldata, int reason);
   void *appupddata;     /* Data pointer passed to the above */
   /* Application call-back, called when object is destroyed. */
   void (*AppExit) (void *data);
   void *exitdata;              /* Data pointer passed to AppExit */
   int gflags;                  /* Grip-flags 32 bits */
   int dflags;                  /* Drop-flags 32 bits */
   int seqdir;                  /* The direction (verical or horizontal),
                                   related to previous object in same node */
   struct t_hotkey *hks;        /* List of hot-keys */
   int linked_event;            /* Indicates that there may be a link from
                                   some message(s) in the event-queue that
                                   has to be removed before the object is
                                   destroyed */
   /* A pointer to a refresh traversor that is currently visiting this widget, or NULL if there is none.*/
   struct t_refresh_traversor *visitor;
   unsigned char dire;          /* The "direction-command" of the object */
   unsigned char clicks;        /* number of clicks so far. */
   unsigned char exclude_add;   /* Tells if the object should be excluded
                                   from the "adding" */
   unsigned char exclude_rect;  /* Tells that object don't take the
                                   responsibility for the drawing of its area*/
   unsigned char autohk;        /* Tells if the object shall be assigned a
                                   hotkey before completed. */
   unsigned char hkxoffs;       /* The hotkey x-offset from text start */
   unsigned char hklen;         /* The hotkey underline length */
   unsigned char hkrow;         /* Tells on which row the underlined
                                   hotkey-letter is */
   unsigned char inactive;      /* Tells if the object is active/inactive */
   unsigned char click2;        /* The allowed buttons for double-click */
   unsigned char click;         /* The allowed buttons for click */
   unsigned char grip;          /* The allowed buttons for dragging */

   unsigned char slide;         /* The allowed buttons for sliding */
   unsigned char resize;        /* The allowed buttons for resizing. The
                                   variable also contains info about the
                                   directions (horizontal=0x80 vertical=0x40)*/
   unsigned char usedbutton;    /* The latest pressed mouse-button. */
   unsigned char hasfocus;      /* Indicates if the object is in focus */
   unsigned char view;          /* Flags telling the current viewing
                                   properties of the object */
   unsigned char action_by_mouse;/* Tells the hardware source of command a (i.e. call to Action) */
   int dx1, dy1, dx2, dy2;      /* These are offsets for the  mouse sensitive
                                   area of the object. Can be used if it is
                                   desired that the sensitive area of the
                                   object is not identical to the visible
                                   area (e.g. the label or frame may be
                                   excluded). */
   struct FONT *font;           /* The font used for displaying the optional
                                   label */
   struct t_image *im;          /* A pointer to an optional image to be
                                   displayed. */
   struct t_image *im_inactive, *im_focus; /* Pointers to images to be used when in other states than the "normal" state. */
   char *imname;                /* The name of an optional image. */
   char *imname_inactive, *imname_focus; /* Names of images to be used when in other states than the "normal" state. */
   char *label;                 /* A pointer to an optional label-text. This
                                   may have some special properties speciifed
                                   by `InsertLabel' */
   char *tooltip;               /* A pointer to an optional tool-tip text. */
   struct BITMAP *ttbmp;        /* A bitmap used temporarly by the tooltip */
   int ttx, tty;                /* Screen positions of the above bitmap when showed. */
   int tooltip_start;           /* The x-position to start drawing in case of animation. */
} t_object;

extern struct t_object *_win_root;    /* pointer to the origin object,
                                         generated at startup */
extern int _cgui_hot_key_line;
extern int _cgui_button1_height; /* standard height for "1-row text"-objects */
extern struct t_object *CreateObject(int x, int y, struct t_node *nd);
extern void InsertLabel(struct t_object *b, const char *label);
extern void StopBlitting(void);
extern void ContinueBlitting(void);
extern struct t_coord *PartitionArea(struct t_object *b, struct t_coord *r0,
                                                     int *n);
extern void XtendedFree(struct t_object *b);
extern void Stub(void *data);
extern void StubOb(struct t_object *b);
extern void MoveObjectInTree(struct t_object *b1, struct t_object *b2);
extern int SetButtonHk(char c, struct t_object *b);
extern void BlitObject(struct t_coord *a, struct t_coord *r0,
                       struct t_node *nd);
extern void MoveNodeInTree(struct t_node *nd, struct t_object *b);
extern void DestroyImages(struct t_image *image);
extern void Event2ObjectLink(int objid);
extern struct t_image *GetImage(struct t_node *nd, const char *imagename);
extern void DrawImage(struct t_image *im, struct BITMAP *bmp, int x, int y);
extern void DefaultSetSizeOffset(struct t_object *b, int w, int h);
extern void DefaultGetSizeOffset(struct t_object *b, int *w, int *h);
extern void DefaultRefresh(struct t_object *b);
extern void DelayedCallBack(void (*Action)(void*), void *data);

extern void PrepareNewScreenMode(void);
extern void NewScreenMode(void);
extern void NewScreenModeImageList(struct t_image *im, struct t_node *nd);

extern int RegisterImageCopy(void *data, const char *imagename, int type, int id);

extern void HideToolTip(t_object *b);
extern void ShowToolTipAt(t_object *b, const char *s, int nr_of_rows, int width, int x, int y);
extern void ClearToolTip(t_object *b);

extern void ObjectRefreshEnter(t_object *b, struct t_refresh_traversor *visitor);
extern void ObjectRefreshVisit(t_object *b, void *calldata, int reason);
extern void ObjectRefreshLeave(t_object *b);

extern int hold_object_blitting;

extern t_typefun default_type_functions;

#define OB_CHGTEXT 1
#define OB_CHGICON 2
#define OB_CHGVALUE 3

#define TEXTOFFSETY 3
#define TEXTOFFSETX 4

/* Default values for distance between objects, and for the distance between */
/* objects and the window border */
#define DEFAULT_YDIST 2
#define DEFAULT_XDIST 2
#define TEXTY(b, dy) ((b->y1 + b->y2 - text_height(b->font)) >> 1) + dy

#endif
