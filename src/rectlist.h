#ifndef RECTLIST_H
#define RECTLIST_H

typedef struct t_coord {
   int x1;
   int y1;
   int x2;
   int y2;
} t_coord;

extern t_coord *_RectList(int nx1, int ny1, int nx2, int ny2,
                          t_coord * rects, int *m, int init);

#endif
