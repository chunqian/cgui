#ifndef OBTEXT_H
#define OBTEXT_H

#define DrawFrameSink(bmp, x1, y1, x2, y2)   \
   hline(bmp, x1, y1,   x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]),           \
   vline(bmp, x1, y1+1, y2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]),           \
   hline(bmp, x1+1, y2, x2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]),        \
   vline(bmp, x2, y1+1, y2-1, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER])

#define DrawFrameRaise(bmp, x1, y1, x2, y2)  \
   hline(bmp, x1, y1,   x2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]),           \
   vline(bmp, x1, y1+1, y2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]),           \
   hline(bmp, x1+1, y2, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]),        \
   vline(bmp, x2, y1+1, y2-1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER])

struct t_typefun;
struct FONT;

extern void DestroyPage(char **page);
extern char **MakePage(struct FONT *f, char *s, int w, int preformat, int lf, int *n);
extern char *GetRow(struct FONT *f, char *s, int width);

#endif
