#ifndef OBBUTTON_H
#define OBBUTTON_H

#include "object.h"

struct t_object;
struct BITMAP;

extern void ButtonInFrame(struct BITMAP *bmp, int x1, int y1, int x2, int y2);
extern void ButtonFrame(struct BITMAP *bmp, int x1, int y1, int x2, int y2, int indic);

#define FillText(b, disp) rectfill(b->parent->bmp, b->x1+2, b->y1+2, b->x2-2+disp, b->y2-2+disp, cgui_colors[CGUI_COLOR_WIDGET_BACKGROUND])
#define ButtonTextC(dx, dy, b, s) ButtonText1(dx, dy, b, 1, s)
#define ButtonTextL(dx, dy, b, s) ButtonText1(dx, dy, b, 0, s)
extern void ButtonText1(int dx, int dy, struct t_object *b, int center,
                        char *s);
extern void ButtonText(int disp, int x1, int y1, int x2, int y2,
                       struct t_object *b, struct BITMAP * bmp);
extern void SetPushButtonSize(struct t_object *b);
extern void DisplayBitmap(t_object * b, int disp);
extern t_object *KernalAddButton(int x, int y, const char *label,
                               void (*cb) (void *), void *data);

typedef struct t_image {
   void *bmp;
   char *name;
   char *filename;
   int transp;
   int type;
   int w;
   int h;
   int callers_data;
   int subseq;
   struct t_image *next;
} t_image;

#define BUTTONBORDER 2
#define TEXTCENTERX(b, dx) b->x1 + dx + \
                       ((b->x2 - b->x1 - text_length(b->font, b->label)) >> 1) + 1

#endif
