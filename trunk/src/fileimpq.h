#ifndef FILEIMPQ_H
#define FILEIMPQ_H

struct t_diskobj;
extern void QueryImportData(const char *curname, int *rep, const char *fn, int **types,
                                int *nt, char **objname, int *bpp,
                                int *relpath, int always);
extern char *InquirePassword(const char *path, struct t_diskobj *dobj, int *save);

#endif
