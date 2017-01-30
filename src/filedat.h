#ifndef FILEDAT_H
#define FILEDAT_H

struct DATAFILE;
struct DATAFILE_PROPERTY;
struct t_dirinfo;
struct t_diskobj;

typedef struct t_typelist {
   const char *desc;
   int type;
   int matchext;
} t_typelist;

extern int PasteADatafileObject(const char *srcdir, const char *filename, const char *destdir, int isaldat, struct DATAFILE **datorg,
                                int (*ReplaceExisting)(const char *fn));
extern int ImportSingleFileIntoDataFile(const char *srcdir, const char *filename, const char *destdir);
extern void TypeToString(char *s, int t);
extern int CheckDataFile(const char *path, int *pack);
extern int GetDataTypeList(const char *ext, t_typelist **ptl);
extern int StringToType(const char *s);
extern int RemoveDatafileObjectByName(const char *dir, const char *filename);
extern int SaveDatafile(struct DATAFILE *dat, const char *name, const char *password, int pack);
extern struct DATAFILE *LoadDataFile(const char *name, const char *password, int *pack, char **pwdret);
extern int CreateSubDataFile(const char *fullpath, struct DATAFILE **pdat);
extern struct DATAFILE *FindNestedDatObj(struct  DATAFILE *datlist, char *subpath);
extern void RenameDatfileObject(struct DATAFILE *datob, const char *path, struct DATAFILE **datl, const char *newname, int pack, const char *pwd);
extern struct DATAFILE *LoadPossibleDataFile(const char *path, const char *password, int *pack, char **pwdret);
extern void GenerateDatafileList(struct t_dirinfo *dirinfo, const char *path, struct DATAFILE *datl, int *isdatafile);
extern int UpdateObjectsWithinDatafile(const char *path, const char *name);
extern int RegisterDatafileImage(struct DATAFILE *datob, const char *name);
extern struct BITMAP *MakePaletteImage(struct t_diskobj *dobj);
extern void UsePalette(struct t_diskobj *dobj);

extern const char *const *GetPasswordList(int *n);
extern int CheckPassword(const char *path, struct t_diskobj *dobj, char *pwd);
extern void ClearPasswords(void);
extern void AddPassword(const char *pwd);
extern void ReplacePassword(const char *oldpwd, const char *newpwd);
extern int IsPasswordInList(const char *pwd);
extern int CheckAnyPassword(const char *path, int *pack);

extern void PropertyTypestring(char *typestring, const struct DATAFILE_PROPERTY *prop);
extern const char *PropertyString(const struct DATAFILE_PROPERTY *prop);
extern void UpdateProperty(struct DATAFILE *dat, int type, const char *string);
extern void RemoveProperty(struct DATAFILE *dat, const struct DATAFILE_PROPERTY *prop);
extern void AddProperty(struct DATAFILE *dat, const char *tstring, const char *string);
extern const struct DATAFILE_PROPERTY *FirstProperty(const struct DATAFILE *datob);
extern const struct DATAFILE_PROPERTY *NextProperty(const struct DATAFILE_PROPERTY *prop);

#endif
