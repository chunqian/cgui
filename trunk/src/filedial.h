#ifndef FILEDIAL_H
#define FILEDIAL_H

typedef struct t_preview t_preview;
struct t_diskobj;
struct t_dirinfo;
struct t_browser;

extern void EditPropertiesDialog(struct t_dirinfo *dirinfo, struct t_diskobj *dobj, int *save, int idlist);
extern void CreateDirDialog(int idlist, struct t_dirinfo *dirinfo, int *save);
extern void CreateDatafileDialog(int idlist, struct t_dirinfo *dirinfo, int *save);
extern t_preview *OpenPreviewWindow(const char *section, int brwinid, struct t_browser *br);
extern void ClosePreviewWindow(t_preview *pw, const char *section);
extern void UpdatePreviewWindow(t_preview *pw, struct t_diskobj *dobj);

#endif
