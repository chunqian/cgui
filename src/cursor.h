#ifndef CURSOR_H
#define CURSOR_H

struct t_object;
extern void InitCursor(void);
extern int current_selected_cursor;
extern void RemovePointerFromWindow(struct t_object *b);
extern void DrawPointerOnWindow(struct t_object *b, int x, int y);
extern int GetSelectedCursorNo(void);

#endif
