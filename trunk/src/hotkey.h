#ifndef HOTKEY_H
#define HOTKEY_H

typedef struct t_hotkey {
   unsigned char scan;          /* The scankey that creates the event */
   unsigned char key;           /* The key that creates the event */
   struct t_hotkey *next;
} t_hotkey;

struct t_object;

typedef struct t_hkfun {
   /* IsHotKey determines if the tree `b' is the owner of the scan/ascii hot-key. */
   struct t_object *(*IsHotKey) (struct t_object *b, int scan, int ascii);

   /* GetLabel is used to recurse the object tree for runtime assignement of hot-keys. */
   void (*GetLabel) (struct t_object *b);

   /* TakeHotKey invites the tree `b'  to take the key press scan/ascii. */
   int (*TakeHotKey) (struct t_object *b, int scan, int ascii);
} t_hkfun;

extern t_hkfun default_hotkey_functions;
extern void InsertHotKey(struct t_object *b, int scan, int key);
extern int NodeTakeHotKey(struct t_object *b, int scan, int key);
extern struct t_object *NodeIsHotKey(struct t_object *b, int scan, int key);
extern int HotKeyHandler(void *data, int scan, int key);
extern void StopHotKeys(void);
extern void StartHotKeys(void);

#endif
