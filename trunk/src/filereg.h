#ifndef FILEREG_H
#define FILEREG_H

typedef void (*t_actionfun)(void *, char*);
typedef void (*t_viewfun)(void *, void *);

extern int IsRegistered(const char *s, int type);
extern t_actionfun GetRegAction(const char *s, int type);
extern char *GetRegActionName(const char *s, int type);
extern char *GetRegIcon(const char *s, int type);
extern char *GetRegLabel(const char *s, int type);
extern void *GetRegData(const char *s, int type);
extern t_viewfun GetRegViewer(const char *s, int type);

#endif
