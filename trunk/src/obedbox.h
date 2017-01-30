#ifndef OBEDBOX_H
#define OBEDBOX_H

#define ON 1
#define OFF 0
#define TOGGLE 2

#define TAB_TAB   (1<<0)
#define CR_TAB    (1<<1)
#define TAB_END   (1<<2)
#define BL0       (1<<3)
#define NAMECASE  (1<<4)
#define FUND      (1<<5)


#define INPUTBORDER 2

struct t_object;
extern void DrawBoxFrame(struct t_object *b);
extern void SetBoxSize(struct t_object *b, int width);
extern void DrawLeftSidedImage(struct t_object *b, int x1, int x2, int offset);
extern int EditBoxKeyboardCallback(void *data, int scan, int key);

#endif
