#ifndef FILES_H
#define FILES_H

struct DATAFILE;
struct FONT;
typedef struct t_diskobj t_diskobj;
typedef struct t_dirinfo t_dirinfo;
typedef struct t_root t_root;

/* In general (unless other is specified) all path strings, either these are
   inputs or values produced by the file-functions, have the cgui-printable
   character set of the path, not the character set received from the file
   system. On normal platforms this will of course be the same, but in
   win/dos the national characters may differ. */

/* Will append file to path using by default using the file systems path separator. If path_contain_dot_dat is true it is assumed that the path
   contains a valid Allegro ".dat"-file, optionally at the end (detected by not finding a "#" in the path). In the latter case '#' will be used
   as separator instead of the default. */
extern char *MergePathAndFile(const char *path, const char *file, int path_contain_dot_dat);

/* NOTE! returns a string with the character set of the file-system. */
extern char *CheckSelectionPath(const char *path, const char *file, int *isdir, int isdat);
extern void AppendFileDelimiter(char *path);
extern char *EndOfFirstPart(char *path);
extern char *FixPath(const char *path);

/* Disk unit handling in DOS/Win */
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
extern char **GetDiskList(int *nodisks, const char *rpath);
extern void DestroyDisklist(char **list, int n);
extern int GetDiskIndex(char *path, char **disklist, int n);
#endif

/* Direct file system operations. */
extern int MakeDirectory(t_dirinfo *dirinfo, const char *dir, int mkdatafile, int pack, const char *pwd);
extern int RemoveDiskObject(t_diskobj *dobj);
/* NOTE! requires a string with the character set of the file-system. */
extern int RemoveDiskObjectByName(const char *path, const char *filename, int isdat);
#ifdef ALLEGRO_UNIX
extern int AdjustPermissions(const char *path, int mode);
#else
extern int AdjustAttributes(const char *path, int prevattrib,
           int attr_h, int attr_r, int attr_s, int attr_a);
#endif
extern void RenameDiskObject(const char *newname, t_diskobj *dobj);
extern int PasteADirectory(t_dirinfo *dirinfo, const char *srcdir, const char *name, const char *destdir, int isaldat);
extern int PasteASingleFile(const char *srcdir, const char *filename, int size, const char *destdir, int check_over_write);
/* Returncodes from `PasteASingleFile': */
#define PASTE_OK          0
#define PASTE_CHECK_ERROR 1
#define PASTE_READ_ERROR  2
#define PASTE_WRITE_ERROR 3

/* Operations on t_diskobj data objects */
extern int IsDirectoryObject(t_diskobj *dobj);
extern t_dirinfo *GetGrandParent(t_diskobj *dobj);
extern t_dirinfo *GetParent(t_diskobj *dobj);

extern void *DiskObjectGetUserData(t_diskobj *dobj);
extern long DiskObjectSize(t_diskobj *dobj);
extern const char *DiskObjectGetImage(t_diskobj *dobj);
extern const char *DiskObjectName(t_diskobj *dobj);
extern const char *DiskObjectExt(t_diskobj *dobj);
/* NOTE DiskObjectModeString returns pointer to static memory. */
extern const char *DiskObjectModeString(t_diskobj *dobj);
extern void DiskObjectTimeString(char *s, t_diskobj *dobj);
extern int DiskObjectDatafileObjectType(t_diskobj *dobj);
extern const struct FONT *DiskObjectGetFont(t_diskobj *dobj);

/* Returns the directory info of the disk object `dobj' in case it is a directory (or datafile), else NULL. */
extern t_dirinfo *DiskObjectGetDirInfo(t_diskobj *dobj);

/* Returns true if `dobj' is a data file object (i.e. it is a part of an Allegro data file. */
extern void *DiskObjectDatafileObject(t_diskobj *dobj);

/* Returns true if `dobj' is the special "../" entry. */
extern int IsUpDir(t_diskobj *dobj);

extern int DiskObjectMode(t_diskobj *dobj);

/* Sets the exapnded state of a disk object. */
extern void DiskObjectSetExpanded(t_diskobj *dobj, int state);

/* Returns the exapnded state of a disk object. */
extern int DiskObjectGetExpanded(t_diskobj *dobj);

/* Returns true if `dobj' is in the current path of the tree. */
extern int DiskObjectIsInDirectoryPath(t_dirinfo *dirinfo, t_diskobj *dobj);

/* Returns true if `dobj' is an Allegro data file object (within a datafile). */
extern int DiskObjectIsDatafileObject(t_diskobj *dobj);

/* Returns true if `dobj' is an Allegro data file in the filessytem (i.e. a ".dat" file that has approved to be a datafile). */
extern int DiskObjectIsDotDat(t_diskobj *dobj);

extern const char *DiskObjectDatSource(t_diskobj *dobj);
extern int DiskObjectIsPacked(t_diskobj *dobj);
/* NOTE: the paths to `DiskObjectCreateDat' and from `DiskObjectGetFullPathDos' shall be/are DOS character set in case of DOS platform. */
extern t_diskobj *DiskObjectCreateDat(const char *name, int attrib, t_dirinfo *dirinfo, long size, int type, struct DATAFILE *dat,
                                      const struct tm *t, const char *orig);

/* Returns the full path of dobj. */
extern char *DiskObjectGetFullPathDos(t_diskobj *dobj);

/* Returns the path to the directory of `dobj'. */
extern char *DiskObjectGetLocation(t_diskobj *dobj);

/* Returns true if `dobj' is a disk object within a datafile. */
extern int DiskObjectIsInDatafile(t_diskobj *dobj);

/* Operations on t_dirinfo data objects */
#define DO_SORT_TEXT    0
#define DO_SORT_NAME    1
#define DO_SORT_TIME    2
#define DO_SORT_SIZE    3
#define DO_SORT_ATTRIB  4

extern int GetFileListIndex(t_dirinfo *dirinfo, const char *name);

/* Returns true if dirinfo represents a ".dat"-datafile (i.e. the list in it is a list of datafile-objects at top level */
extern int IsFileDotDat(t_dirinfo *dirinfo);

/* Returns true if dirinfo represents a ".dat"-datafile or an object that is a of type DAT_FILE. In either case this means that iterating dirinfo gets
   datafile objects. */
extern int IsDataFileList(t_dirinfo *dirinfo);

/* Returns the DATAFILE object that this dirinfo has loaded. Valid if `IsDataFileList' is true, else it returns NULL. */
extern struct DATAFILE *GetDataFile(t_dirinfo *dirinfo);

/* Returns the full path of the directory represented by `dirinfo'. */
extern const char *DirInfoGetPath(t_dirinfo *dirinfo);

/* Returns the size of the directory represented by `dirinfo', as the sum of the sizes of its file entries. */
extern int DirInfoGetTotalSize(t_dirinfo *dirinfo);

/* Returns 1 if it is known that there are subdirectories in `dirinfo' else 0. I.e. the directory is loaded and there was at least one directory when
   last loaded and the chace is not dirty. */
extern int DirInfoHasSubdirs(t_dirinfo *dirinfo);

/* Returns the number sub directories. */
extern int DirInfoGetNrOfSubdirs(t_dirinfo *dirinfo);

/* Returns the number of file entries within the directory represented by `dirinfo'. */
extern int DirInfoGetNrOfDiskObjects(t_dirinfo *dirinfo);

/* Returns the file entry object of this directory (i.e. of `dirinfo'). */
extern t_diskobj *DirInfoGetDiskObject(t_dirinfo *dirinfo);

/* Accessor to the file entry at index 'i' (according to the current sorting) in a directory (all loaded entries, including subdirectories). */
extern t_diskobj *DirInfoGetDiskObjectAt(t_dirinfo *dirinfo, int i);

/* Iterator functions over the loaded entries in a directory. NOTE: iterations can't be nested over the same directory since there is one
   entry-iterator in each dirinfo. */
extern t_diskobj *DirInfoGetFirstDiskObject(t_dirinfo *dirinfo);
extern t_diskobj *DirInfoGetNextDiskObject(t_dirinfo *dirinfo);

/* Iterator functions over the loaded files in a directory. NOTE: iterations can't be nested over the same directory since there is one
   file-iterator in each dirinfo. */
extern t_diskobj *DirInfoGetFirstFile(t_dirinfo *dirinfo);
extern t_diskobj *DirInfoGetNextFile(t_dirinfo *dirinfo);

/* Iterator functions over the loaded subdirectories in a directory. NOTE: iterations can't be nested over the same directory since there is one
   directory-iterator in each dirinfo. The parameter `dirinfo' should for both functions be a pointer to the directory to iterate. The return value
   is a pointer to a sub-directory. */
extern t_diskobj *DiskObjectGetFirstDirectory(t_diskobj *dobj);
extern t_diskobj *DiskObjectGetNextDirectory(t_diskobj *dobj);

/* Objects are sorted according to the current sorting. When the current sorting is changed all cached directories will be resorted. So
   this function should not be needed ... */
extern void SortDiskObjects(t_dirinfo *dirinfo);

/* Create a virtual root data structure. The root directory of the file system will however not be loaded. If `LoadProgressCallBack' is not NULL it
   will be called with `data' with time intervals of `callback_time_interval' during subsequent loads. The second parameter should be one of
   the DL_* specified below. The call back will be called also for some certain check points of the loads as indicated by the macro name.
   The `data' pointer can be obtained also by `DiskObjectGetUserData'. The `dirinfo' pointer will point to the directory that is loaded. */
extern t_root *CreateRoot(void (*LoadProgressCallBack)(void *data, int reason, t_dirinfo *dirinfo), void *data, int callback_time_interval);

/* Returns a pointer the dirinfo data structure of the file system root. */
extern t_dirinfo *RootGetDirInfo(t_root *root);

/* Destroys the root and its directory tree. Note that not only the `root' pointer will be invalid after the call, also all pointer returned by the
   below "Loaders" do. Use it when you are done with everything. */
extern void DestroyDirectoryTree(t_root *root);

/* Below are 4 "loader" functions. They load the files from a directory (or may use the cache in case it is available) and can be used in various
   contexts. Common for all is that the callback installed by `CreateRoot' will get a dirinfo passed. */

/* Returns a pointer to the dirinfo structure representing the the directory at path `path'. In case the directory info is already loaded into
   the cached directory tree, this will be re-used, else it will load it from disk and cache all the directories along the path. It will always
   return a valid pointer. The `dirinfo' pointer passed to the call-back will be the directory that is currently loading when the call-back is
   called. Note that this may vary between calls as the loading along the path progresses. It the specified path is not a valid directory the
   closest existing ancestor will be loaded instead. */
extern t_dirinfo *DirInfoLoadDirectoryFromPath(t_root *root, const char *path);

/* Reloads the directory from disk (the cache is not used) that the dirinfo structure pointed to by `*dirinfop' represents. This can be useful in
   case the contents of the disk might have been changed. In case the directory of the dirinfo structure pointed to by `*dirinfop' does not
   exist any longer, then the pointer pointed to by dirinfop will be updated and set to point to a dirinfo structure that represents the closest
   existing ancestor directory, and the return value will be 0, else 1. */
extern int DirInfoReLoadDirectory(t_dirinfo **dirinfop);

/* Returns the directory info of the parent directory to `dirinfo' or, in case it is the root, it will return `dirinfo'. If the cache is fresh it will
   use it. If reloading is necessary because the cache is not fresh, then it will call DirInfoReLoadDirectory, thus a NULL pointer as return value is
   possible. */
extern t_dirinfo *DirInfoGetUpDirectory(t_dirinfo *dirinfo);

/* Will return a pointer to the directory info of the direcory associated with `dobj'. In case `dobj' is not a directory it will return a pointer to
   the closest ancestor. */
extern t_dirinfo *DirInfoLoadDirectoryFromDiskObject(t_diskobj *dobj);

/* The following are reason parameters to the progress call-back that will be called during loading, if not NULL. */
#define DL_INTERVAL_ELAPSED     0 /* Call reason is that one time interval has elapsed. */
#define DL_LOAD_STARTED         1 /* Call reason is that loading has just started (usually the file list will be empty). */
#define DL_DIRECTORIES_FINISHED 2 /* Call reason is that the directories portion of the entries has been loaded. */
#define DL_DATAFILES_LOADED     3 /* Call reason is that all potential datafiles has been loaded (omitted if datafiles are not accepted). */
#define DL_DATAFILES_CHECKED    4 /* Call reason is that all datafiles has been verified (omitted if datafiles are not accepted). */
#define DL_FILES_LOADED         5 /* Call reason is that all normal files (non-directories) has been loaded (only for unix, flags are still to be loaded). */
#define DL_LOAD_FINISHED        6 /* Call reason is that all disk activity has been finished (but not yet sorted). */
#define DL_DIRECTORY_END        7 /* Call reason is that everything is finished in the current directory, but there are more directories to be loaded. */
#define DL_END                  8 /* Call reason is that everything is finished. */

/* Set the mode for subsequent file loading. If `mode' is non-0 all files will be loaded, else all except hidden files and system files will be
   loaded. In case of unix "hidden" means files starting with '.'. Reloading is not necessary. */
extern void SetLoadHiddenSystem(t_root *root, int mode);

/* Set the mode for subsequent file loading. If `mode' is non-0 all dat-files will be loaded in subsequent loads, else not. To make them available
   in existing directories these need to be reloaded (the cache will be marked as outdated). */
extern void SetLoadDat(t_root *root, int mode);

/* Set the masks to be used in subsequent file loading. `masks' should point to a semi-colon separated list of masks, each fulfilling the criterias
   set by allegros directory loading functions (i.e. currently those of file system). To make a new file list available in existing directories these
   need to be reloaded (the cache will be marked as outdated). */
extern void SetLoadMasks(t_root *root, const char *masks);

/* Set the sorting used in subsequent file loading. Reloading is not necessary. */
extern void SetLoadSorting(t_root *root, int sorting, int descending);

#endif
