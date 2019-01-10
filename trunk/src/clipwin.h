#ifndef CLIPWIN_H_INCLUDED
#define CLIPWIN_H_INCLUDED

#ifdef ALLEGRO_UNIX
typedef struct clipX clipX;

void close_X_clipboard(clipX *xclip);

char *get_X_clipboard(clipX *xclip);
char *get_X_primary(clipX *xclip);

clipX * init_X_clipboard(void);
#endif // ALLEGRO_UNIX
int InsertIntoSystemsClipboard(const char *selection);
int InsertIntoPrimaryBuffer(const char *selection);

#endif // CLIPWIN_H_INCLUDED
