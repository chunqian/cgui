#ifndef OBHEADER_H
#define OBHEADER_H

/* Draws specified label properly clipped within a frame of 3 pixels */
/* It make use of the fcol and bcol to draw */
struct BITMAP;
extern void DrawThinFrame(struct BITMAP *bmp, int x1, int y1, int x2, int y2);
extern int MkWinHeader(const char *label);
#define HEADERFRAME 3
#define HEADERSPACE 1

#endif
