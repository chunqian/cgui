/* Module FILES.C
   Contains all the basic functions for the file browser.
*/
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <allegro.h>
#include <allegro/internal/aintern.h>
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_UNIX)
#ifndef CGUI_SCAN_DEPEND
#include <sys/stat.h>
#endif
#endif

#include "files.h"
#include "cgui.h"
#include "cgui/misc.h"
#include "cgui/mem.h"
#include "cgui/dw_conv.h"
#include "filedat.h"
#include "timer.h"

#ifndef MAXPATH
#define MAXPATH 1024
#endif

#if (ALLEGRO_SUB_VERSION < 2) || ((ALLEGRO_SUB_VERSION == 2) && (ALLEGRO_WIP_VERSION == 0))
#define file_size_ex file_size
#endif

#ifndef ALLEGRO_DOS
#define Dos2Win(a, b)
#define Win2Dos(a, b)
#endif

#define FILE_LOAD_EVENT_MASK 0x20000000

#ifndef ALLEGRO_UNIX
#define strtok_r(s, pat, buf) strtok(s, pat)
#endif

#define STATE_LOAD_INIT                    0
#define STATE_LOAD_DIRECTORIES             1
#define STATE_LOAD_DATAFILES               2
#define STATE_CHECK_DATAFILES              3
#define STATE_LOAD_FILES_OF_MASK           4
#define STATE_LOAD_MODE_FLAGS              5

#if ALLEGRO_SUB_VERSION == 0

#define for_each_file_ex(filename, flags, reject_flags, callback, data) \
        ((void (*)(const char *, int, void (*)(const char *, int, void*), void*))for_each_file)(filename, flags, callback, data)
#define FOREACHFILE_CALLBACK_RETVAL
#define FOREACHFILE_CALLBACK_RETTYPE void
#define ALLFILES FA_RDONLY|FA_ARCH|FA_SYSTEM|FA_HIDDEN|FA_DIREC
#define canonicalize_filename fix_filename_path

#else

#define FOREACHFILE_CALLBACK_RETVAL 0
#define FOREACHFILE_CALLBACK_RETTYPE int
#define ALLFILES 0
#endif

static char separator_string[2] = {OTHER_PATH_SEPARATOR, 0};

typedef struct t_loader t_loader;

struct t_diskobj {
   /* Indicates if this is an Allegro datafile (i.e. a real file in the file system) */
   int is_dot_dat;

   /* Indicates if this is an an Allegro datafile object (i.e. any object within a datafile) */
   int isindat;

   /* Indicates if this is a logical directory. It may either be a real file-system dir, or an Allegro data-file or an Allegro datafile object
      of type DAT_FILE. */
   int isdir;

   /* Indicates if this is the unique "up"-dir (../) */
   int updir;

   /* The name of the object as received from the file system. No full path. */
   char *name;

   /* A viewable translation of the name above, actualy needed only in case of DOS. */
   char *namev;

   /* The file name extension (pointer into the above string)*/
   char *ext;

   /* A pointer to the original name of a datafile object. Optional also for these. NULL when not used.
      NOTE: this pointer refers to the datafile-structure and must not be freed (unload datafile does this). */
   const char *orig;

   /* If set there is no relevant time is present. */
   int no_time;

   /* Modification time of the file in tm-format. */
   struct tm t;

   /* This size of the file, directory or datafile object. */
   long size;

   /* Valid for datafiles only. Indicate if it should be packed. */
   int pack;

   /* A pointer to the cgui-image-name-string of this file. In case there is no image or if the image is not loaded, it will b eNULL: */
   char *image_loaded;

   /* Indicates if we have already made an attempt to load an image (which occasionally may have failed). */
   int tried_loading;

   /* A pointer to the parent directory of this disk object. */
   t_dirinfo *parent;

   /* A pointer to directory info. NULL if this is a plain file or datafile object that is not a datafile itself. */
   t_dirinfo *dirinfo;

   /* Valid only in case of a datafile object. Tells the object type. */
   int datafile_object_type;

   /* A pointer to the data-object withing a data file associated with this disk object. Valid only if isindat is true. Must not be freed */
   DATAFILE *datob;

   /* In case the disk object is a datafile object and the type is `DAT_FONT' then `font will point to that font. */
   struct FONT *font;

   /* The file or directory attributes, or corresponding unix mode-flags. Not valid in case of datafile object. The attrib component is available
      on the unix platform (for simplicity, since allegro supports it), but is not used. */
#ifdef ALLEGRO_UNIX
   int st_mode;
   int inode;
#endif
   int attrib;

   /* An attribut that we serve the user with. Set to 0 if the user does not set it. */
   int expanded;

   /* Pointer to the root data of the tree. Valid in all disk objects. */
   t_root *root;
};

/* This is the information about a directory. Note that allegro datafiles are also considered directories. */
struct t_dirinfo {
   /* The path to the directory. The form is as received from the file system */
   char *path;

   /* True if this is a data-file in the disk system (not a sub-datafile within another) */
   int is_file_dot_dat;

   /* Non-NULL if this is a directory within a datafile. */
   DATAFILE *dat;

   /* In case of a datafile, this tells if the datafile is packed. */
   int pack;

   /* Valid if `is_file_dot_dat' is true, in which case it will point to the password string, else NULL. */
   char *pwd;

   /* The disk object that owns this dirinfo. */
   t_diskobj *dobj;

   /* The kids of this directory. */
   t_diskobj **dobjs;

   /* The number of kids. */
   int n;

   /* The parked (unused) kids that are hidden or system files.. */
   t_diskobj **hidden_system_files;

   /* The number of parked objects. */
   int nhidden_system_files;

   /* The number of kids that are directories. */
   int ndirs;

   /* An iterator that iterates over all disk objects. */
   int itall;

   /* An iterator that iterates over all directories. */
   int itdir;

   /* An iterator that iterates over all normal files (not directories). */
   int itfile;

   /* The size of this directory as the sum of the sizes of the kids. */
   int dirsize;

   /* Pointer to the root data of the tree. Valid in all disk objects. */
   t_root *root;

   /* Pointer to a data structure used while loading. NULL indicates that no lodaing is in progress. */
   t_loader *loader;

   /* Indicates if the current content is no longer up to date with the loading requirements (mask and dat loading). */
   int dirty;
};

struct t_root {
   /* Pointer to the root directory of the directory tree. */
   t_dirinfo *dirinfo;

   /* Pointer to the root directory of the directory tree. */
   t_diskobj *dobj;

   /* Call back. If set it will be called by a loader as a notification about progress (in case the loading is slow). */
   void (*CallBack)(void *data, int reason, t_dirinfo *dirinfo);

   /* User call-back data. Can also be retrieved using any disk object as handle. */
   void *data;

   /* The time between callback progress calls. */
   long call_back_interval;

   /* Indicates if hidden and system files should be ignored or not (0=ignore) when loading. */
   int load_hs;

   /* Indicates if candidiates to be datafiles (i.e. matching *.dat) should be attempted to be handled as directories
      or as normal disk files when loading. */
   int load_dat;

   /* Masks telling which patterns of filenames that shall be used when loading the files in a directory. */
   char *masks;

   /* Indicates which sorting criteria to be used when sorting files in a directory. */
   int sorting;

   /* Indicates if sorting should be decsending or ascending. */
   int descending;
};

struct t_loader {
   /* Pointer back to directory to load. */
   t_dirinfo *dirinfo;

   /* A data structure to interface al_find* functions. */
   struct al_ffblk info;

   /* The event id of the loading process. */
   int eventid;

   /* Buffer used by strtok to split the the masks string into pieces. */
   char *splitted_masks;

   /* Buffer used by strtok_r, make it thread safe. */
   char *strtok_buffer;

   unsigned long curtime;

   /* The state variable that keep track of which step of loading that is processed. */
   int step;

   /* If true there will be more loads when this is finished (indicates that appropriate code should be passed to call-back). */
   int last_dir_reason;
};

/* The file-collection is a simplified representation of disk object that is used during the paste process only. */
typedef struct t_filecoll t_filecoll;
typedef struct t_name t_name;

struct t_name {
   /* The name of the disk objekt. */
   char *name;

   /* Indicates if the object is a directory or not. */
   int isdirectory;
};

struct t_filecoll {
   /* A pointer to an array of collecton of disk objects. */
   t_name *f;

   /* Number of objects. */
   int n;
};

static void DestroyDiskObject(t_diskobj *dobj);
static void DestroyDirInfo(t_dirinfo *dirinfo);
static void MoveToNextStep(t_dirinfo *dirinfo);
static t_dirinfo *CreateDirInfo(t_diskobj *dobj, t_dirinfo *parent, int isindat);

/* --------- String operations related to the file system */

static int IsRoot(const char *path)
{
   if (path[0] == 0 || path[1] == 0)
      return 1;
   if (path[1] == DEVICE_SEPARATOR && path[2] == 0)
      return 1;
   if (path[1] == DEVICE_SEPARATOR && path[2] == OTHER_PATH_SEPARATOR && path[3] == 0)
      return 1;
   return 0;
}

static int IsDiskName(char *path)
{
   return *path == 0 || (path[2] == 0 && path[1] == DEVICE_SEPARATOR);
}

/* Returns dynamic memory */
extern char *FixPath(const char *path)
{
   int size, i;
   char *p;

   size = MAX(strlen(path) + 10, MAXPATH);
   p = GetMem(char, size);
   strcpy(p, path);
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
   if (*p == 0) return p;
#endif
   i = strlen(p) - 1;
   if (i >= 0 && p[i] == '.')
      put_backslash(p);
   canonicalize_filename(p, p, size);
   i = strlen(p) - 1;
   if (i > 1 && p[i] == OTHER_PATH_SEPARATOR && p[i-1] != DEVICE_SEPARATOR)
      p[i] = 0;
   return p;
}

static void RemoveFileDelimiter(char *path)
{
   int end;

   end = strlen(path) - 1;
   if (path[end] == OTHER_PATH_SEPARATOR || path[end] == '#')
      path[end] = 0;
}

extern char *EndOfFirstPart(char *path)
{
   return path + strcspn(path, separator_string);
}

extern char *MergePathAndFile(const char *path, const char *file, int path_contain_dot_dat)
{
   char *s;

   if (path_contain_dot_dat && strchr(path, '#') == NULL) {
      s = msprintf("%s#%s", path, file);
   } else {
      s = msprintf("%s%c%s", path, OTHER_PATH_SEPARATOR, file);
   }
   return s;
}

extern void AppendFileDelimiter(char *path)
{
   if (IsDiskName(path))
      strcat(path, separator_string);
   else
      put_backslash(path);
}

/* Checks if path/file or path#file is valid path if so returns a pointer to dynamic memory that contains that path. Caller is responsible for
   freeing mem. NOTE! The returned string will have the native disk system character set. */
extern char *CheckSelectionPath(const char *path, const char *file, int *isdir, int path_contain_dot_dat)
{
   char *p, *datdel;
   int aret;

   p = MergePathAndFile(path, file, path_contain_dot_dat);
   datdel = strchr(p, '#');
   if (datdel)
      *datdel = 0;
   Win2Dos(p, p);
   if (datdel)
      *datdel = '#';
   if (file_exists(p, FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_DIREC|FA_ARCH, &aret)) {
      *isdir = (aret & FA_DIREC) != 0;
      return p;
   }
   *isdir = 0;
   Release(p);
   p = MergePathAndFile(path, file, path_contain_dot_dat);
   Win2Dos(p, p);
   if (exists(p))
      return p;
   Release(p);
   return NULL;
}

/* --------- Disk unit handling in DOS/Win */

static FOREACHFILE_CALLBACK_RETTYPE DeleteCallback(const char *fn, int attrib, void *data)
{
   char *s;
   (void)data;

   if (attrib & FA_DIREC) {
      s = get_filename(fn);
      if (strcmp(s, "..") == 0 || strcmp(s, ".") == 0)
         return FOREACHFILE_CALLBACK_RETVAL;
      DeleteTree(fn);
   } else {
      delete_file(fn);
   }
   return FOREACHFILE_CALLBACK_RETVAL;
}

extern int DeleteTree(const char *path)
{
   char *filename;
   int error;

   filename = MergePathAndFile(path, "*", 0); /* We only operates on file system entires. */
   fix_filename_slashes(filename);
   for_each_file_ex(filename, ALLFILES, 0, DeleteCallback, NULL);
   strcpy(filename, path);
   RemoveFileDelimiter(filename);
#ifdef ALLEGRO_MSVC
   error=-1;
#else
   error = rmdir(filename);
#endif
   Release(filename);
   return error == 0;
}

#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
extern int GetDiskIndex(char *path, char **disklist, int n)
{
   int i, c;

   if (path[0] == 0 || path[1] == 0 || path[1] != ':')
      return -1;
   c = toupper(*path);
   for (i = 0; i < n; i++)
      if (*disklist[i] == c)
         return i;
   return -1;
}

extern void DestroyDisklist(char **list, int n)
{
   int i;
   for (i = 0; i < n; i++)
      Release(list[i]);
   Release(list);
}

#define MAXDISKS 26
static char disk_units[MAXDISKS][3];
static int n_disks;
extern char **GetDiskList(int *nodisks, const char *rpath)
{
   char **disklist = NULL;
   char buf[MAXPATH];
   int i;
   int drive;
   unsigned long drives_mask;

   if (n_disks == 0) {
      if (rpath && *rpath && rpath[1]==':') {
         sprintf(disk_units[n_disks++], "%c:", toupper(*rpath));
      } else {
         for (drive = 1, drives_mask = _getdrives(); drives_mask != 0; ++drive, drives_mask >>= 1) {
            /* Check if the drive is valid */
            if (drives_mask & 1 != 0) {
               /* Get the current working directory path of the drive */
               if (_getdcwd(drive, buf, sizeof(buf)) != 0) {
                  /* Copy only the drive prefix from the path */
                  strncpy(disk_units[n_disks], buf, 2);
                  disk_units[n_disks][0] = toupper(disk_units[n_disks][0]);
                  ++n_disks;
               }
            }
         }
      }
   }
   *nodisks = n_disks;
   disklist = GetMem(char *, n_disks);
   for (i = 0; i < n_disks; i++)
      disklist[i] = MkString(disk_units[i]);
   return disklist;
}
#endif

/* --------- Direct file system operations. */
extern int MkDir(const char *p)
{
   int error = 1;
#ifdef ALLEGRO_DOS
   char *cp,*fn;
   cp = MkString(p);
   fn = get_filename(cp);
   Win2Dos(fn, fn);
   error = mkdir(cp, S_IREAD|S_IWRITE|S_IEXEC);
   Release(cp);
#else
 #ifdef ALLEGRO_UNIX
   error = mkdir(p, S_IREAD|S_IWRITE|S_IEXEC);
 #else
  #ifdef ALLEGRO_WINDOWS
   error = mkdir(p);
  #endif
 #endif
#endif
   return error;
}

extern int MakeDirectory(t_dirinfo *destdirinfo, const char *destdirname, int mkdatafile, int pack, const char *pwd)
{
   int error = 0;
   char *newpath, *p;

   if (destdirinfo==NULL) {
      ; /* determine from the path if it is a (sub-)datafile or a directory */
   } else if (destdirinfo->dat) { /* This is within a datafile. */
      p = MkString(destdirinfo->path);
      Dos2Win(p, p);
      newpath = MergePathAndFile(p, destdirname, 1);
      CreateSubDataFile(newpath, &destdirinfo->dat); /* Needs win-encoding */
      Release(p);
      Release(newpath);
   } else if (mkdatafile) { /* This is within a datafile. */
      p = MkString(destdirinfo->path);
      Dos2Win(p, p);
      CreateNewDataFile(p, destdirname, pack, pwd);
      Release(p);
   } else { /* Normal file system entry. */
      if (get_filename(destdirname) == destdirname)
         p = MergePathAndFile(destdirinfo->path, destdirname, 0);
      else
         p = MkString(destdirname);
      MkDir(p);
      Release(p);
   }
   return error;
}

static void DestroyFileCollection(t_filecoll *fc)
{
   int i;

   for (i = 0; i < fc->n; i++)
      Release(fc->f[i].name);
   Release(fc->f);
   Release(fc);
}

static t_filecoll *CreateFileCollection(void)
{
   t_filecoll *fc;

   fc = GetMem0(t_filecoll, 1);
   return fc;
}

static FOREACHFILE_CALLBACK_RETTYPE FilePasteCallback(const char *fn, int attrib, void *data)
{
   t_filecoll *fc = data;

   if ((attrib & FA_DIREC) && *fn && (fn[strlen(fn)-1] == '.'))
      return FOREACHFILE_CALLBACK_RETVAL;
   fc->n++;
   fc->f = ResizeMem(t_name, fc->f, fc->n);
   fc->f[fc->n - 1].name = MkString(fn);
   fc->f[fc->n - 1].isdirectory = attrib & FA_DIREC;
   return FOREACHFILE_CALLBACK_RETVAL;
}

#ifdef ALLEGRO_UNIX
extern int AdjustPermissions(const char *path, int mode)
{
   return chmod(path, mode);
}
#else
extern int AdjustAttributes(const char *path, int attrib, int attr_h, int attr_r, int attr_s, int attr_a)
{
   char *p, *q, *att;
   int err = 0;

   att = GetMem(char, strlen(path) + 100);
   sprintf(att, "attrib +");
   p = q = att + strlen(att);
   if (attr_h && !(attrib&FA_HIDDEN))
      *p++ = 'H';
   if (attr_r && !(attrib&FA_RDONLY))
      *p++ = 'R';
   if (attr_s && !(attrib&FA_SYSTEM))
      *p++ = 'S';
   if (attr_a && !(attrib&FA_ARCH))
      *p++ = 'A';
   if (p[-1] == '+')
      p[-1] = '-';
   else {
      *p++ = ' ';
      *p++ = '-';
   }
   if (!attr_h && (attrib&FA_HIDDEN))
      *p++ = 'H';
   if (!attr_r && (attrib&FA_RDONLY))
      *p++ = 'R';
   if (!attr_s && (attrib&FA_SYSTEM))
      *p++ = 'S';
   if (!attr_a && (attrib&FA_ARCH))
      *p++ = 'A';
   if (p[-1] == '-')
      p -= 2;
   if (p > q) { /* Don't do this if not necessary - it may fail */
      *p++ = ' ';
      strcpy(p, path);
      Win2Dos(p, p);
      strcat(p, " > nul");
#ifdef ALLEGRO_DOS
      err = system(att);
#else
/* ShellExecute(??); */
#endif
   }
   Release(att);
   return err;
}
#endif

/* Assumes that `filename' is a single disk-file at location `srcdir' it will paste that file at `destdir' if that is a normal disk directory,
   otherwise (i.e. `destdir' is a datafile or a datafile-object) it will be imported to location `destdir' in that datfile (if destdir can
   either be the root datafile, a single object in which case it can be a FILE or not).
   If the check-parameter is 0 the file will overwrite an existing file with the same name (without checking).
   Return values:
   PASTE_OK          - Successful operation
   PASTE_CHECK_ERROR - The disk object does already exist
   PASTE_READ_ERROR  - Could not read the file
   PASTE_WRITE_ERROR - Could not write to the destination
   */
extern int PasteASingleFile(const char *srcdir, const char *filename, int size, const char *destdir, int check)
{
   char *content, *dpath, *spath;

   int n, c, error = PASTE_OK;
   PACKFILE *src, *dest;

   dpath = MergePathAndFile(destdir, filename, 0);
   spath = MergePathAndFile(srcdir, filename, 0);
   Win2Dos(spath, spath);
   Win2Dos(dpath, dpath);
   if (size == 0)
      size = file_size_ex(spath);
   if (check && exists(dpath)) {
      Release(dpath);
      Release(spath);
      return PASTE_CHECK_ERROR;
   }
   if (size)
      content = (char*)malloc(size);
   else
      content = NULL;
   /* In case we got enough memory then we try a possibly faster copying method */
   if (content) {
      src = pack_fopen(spath, "rb");
      if (src) {
         n = pack_fread(content, size, src);
         pack_fclose(src);
         if (n != size) {
            error = PASTE_READ_ERROR;
         } else {
            dest = pack_fopen(dpath, "wb");
            if (dest) {
               n = pack_fwrite(content, size, dest);
               pack_fclose(dest);
               if (n != size) {
                  error = PASTE_WRITE_ERROR;
               }
            } else {
               error = PASTE_WRITE_ERROR;
            }
         }
      } else {
         error = PASTE_READ_ERROR;
      }
      Release(content);
   } else {
      src = pack_fopen(spath, "rb");
      if (src) {
         dest = pack_fopen(dpath, "wb");
         if (dest) {
            for (c = pack_getc(src); c != EOF; c = pack_getc(src)) {
               c = pack_putc(c, dest);
               if (c == EOF) {
                  error = PASTE_WRITE_ERROR;
               }
            }
            pack_fclose(dest);
         } else {
            error = PASTE_WRITE_ERROR;
         }
         pack_fclose(src);
      } else {
         error = PASTE_READ_ERROR;
      }
   }
   Release(spath);
   Release(dpath);
   return error;
}

/* The source `name' is assumed to be a directory. The entire directory tree will be created into the destination dir. If destination dir is a datafile
   or a datafile object within a datafile, then the disk-directory will copied into the datafile representing the directories by data-fileobject of
   type FILE. */
extern int PasteADirectory(t_dirinfo *destdirinfo, const char *srcdir, const char *srcname, const char *destdir, int isaldat)
{
   int i, error, len;
   char *dpath, *spath, *fn;
   t_filecoll *fc;

   dpath = MergePathAndFile(destdir, srcname, isaldat);
   if (destdirinfo && destdirinfo->dat)
      error = CreateSubDataFile(dpath, &destdirinfo->dat); /* Needs win-encoding */
   else
      error = MakeDirectory(destdirinfo, dpath, 0, 0, ""); /* Needs win-encoding */
   if (!error) {
      fc = CreateFileCollection();
      len = strlen(srcdir) + strlen(srcname) + 10;
      spath = GetMem(char, len);
      strcpy(spath, srcdir);
      append_filename(spath, spath, srcname, len);
      Win2Dos(spath, spath);
      append_filename(spath, spath, "*", len);
      for_each_file_ex(spath, ALLFILES, 0, FilePasteCallback, fc);
      for (i = 0; i < fc->n; i++) {
         Dos2Win(fc->f[i].name, fc->f[i].name);
         fn = get_filename(fc->f[i].name);
         if (fn > fc->f[i].name)
            fn[-1] = 0;
         if (fc->f[i].isdirectory) {
            error |= PasteADirectory(destdirinfo, fc->f[i].name, fn, dpath, isaldat);
         } else if (isaldat) {
            error = ImportSingleFileIntoDataFile(fc->f[i].name, fn, dpath);
         } else {
            error |= PasteASingleFile(fc->f[i].name, fn, 0, dpath, 0);
         }
      }
      Release(spath);
      DestroyFileCollection(fc);
   }
   Release(dpath);
   return error;
}

/*  Returns 1 if OK. If failed it returns 0 when a directory, -1 when a
    simple file and -2 when an internal data-file error. */
extern int RemoveDiskObjectByName(const char *dir, const char *filename, int isindat)
{
   int ex, attrib, status = 1;
   char *p;

   if (isindat) {
      status = RemoveDatafileObjectByName(dir, filename);
   } else {
      p = MergePathAndFile(dir, filename, 0);
      Win2Dos(p, p);
      ex = file_exists(p, FA_DIREC|FA_RDONLY|FA_ARCH|FA_HIDDEN|FA_SYSTEM, &attrib);
      if (ex == 0)
         status = -1;
      if (status == 1) {
         if (attrib & FA_DIREC) {
            status = DeleteTree(p);
         } else {
            if (delete_file(p)) {
               status = -1;
            } else {
               status = 1;
            }
         }
      }
      Release(p);
   }
   return status;
}

extern int RemoveDiskObject(t_diskobj *dobj)
{
   char *p, *fn;
   int status;

   p = MkString(dobj->parent->path);
   fn = MkString(dobj->name);
   Dos2Win(p, p);
   Dos2Win(fn, fn);
   status = RemoveDiskObjectByName(p, fn, IsDataFileList(dobj->parent));
   Release(p);
   Release(fn);
   return status;
}

extern void RenameDiskObject(const char *newname, struct t_diskobj *dobj)
{
   char *orgpath, *newpath, *name;
   t_dirinfo *dirinfo;

   dirinfo = dobj->parent;
   if (IsDataFileList(dirinfo)) {
      RenameDatfileObject(dobj->datob, dirinfo->path, &dirinfo->dat, newname, dirinfo->pack, dirinfo->pwd);
   } else {
      orgpath = MergePathAndFile(dirinfo->path, dobj->name, 0);
      newpath = MergePathAndFile(dirinfo->path, newname, 0);
      name = get_filename(newpath);
      Win2Dos(name, name);
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
      if (stricmp(name, dobj->name) == 0) {
         /* If names differs only in case, then DOS will refuse to change the
            name, so we have to first change to some completely different name
            and then rename a second time to the requested one */
         const char *tmp = tmpnam(NULL);
         if (tmp) {
            rename(orgpath, tmp);
            rename(tmp, newpath);
         }
      } else
#endif
         rename(orgpath, newpath);
      Release(orgpath);
      Release(newpath);
   }
}

/* --------- Operations on t_diskobj data objects */

static int CompareFileExtensionStrings(const void *p1, const void *p2)
{
   const t_diskobj *dobj1 = *(const t_diskobj **)p1;
   const t_diskobj *dobj2 = *(const t_diskobj **)p2;
   int dir;

   dir = dobj2->updir - dobj1->updir;
   if (dir == 0) {
      dir = (dobj2->isdir) - (dobj1->isdir);
      if (dir==0)
         return stricmp(dobj1->ext, dobj2->ext);
      else
         return dir;
   } else
      return dir;
}

static int CompareFileExtensionStringsDescending(const void *p1, const void *p2)
{
   const t_diskobj *dobj1 = *(const t_diskobj **)p1;
   const t_diskobj *dobj2 = *(const t_diskobj **)p2;
   int dir;

   dir = dobj2->updir - dobj1->updir;
   if (dir == 0) {
      dir = (dobj2->isdir) - (dobj1->isdir);
      if (dir==0)
         return stricmp(dobj2->ext, dobj1->ext);
      else
         return dir;
   } else
      return dir;
}

static int CompareFileNames(const void *p1, const void *p2)
{
   const t_diskobj *dobj1 = *(const t_diskobj **)p1;
   const t_diskobj *dobj2 = *(const t_diskobj **)p2;
   int dir;

   dir = dobj2->updir - dobj1->updir;
   if (dir == 0) {
      dir = (dobj2->isdir) - (dobj1->isdir);
      if (dir == 0)
         return stricmp(dobj1->namev, dobj2->namev);
      else
         return dir;
   } else
      return dir;
}

static int CompareFileNamesDescending(const void *p1, const void *p2)
{
   const t_diskobj *dobj1 = *(const t_diskobj **)p1;
   const t_diskobj *dobj2 = *(const t_diskobj **)p2;
   int dir;

   dir = dobj2->updir - dobj1->updir;
   if (dir == 0) {
      dir = (dobj2->isdir) - (dobj1->isdir);
      if (dir==0)
         return stricmp(dobj2->namev, dobj1->namev);
      else
         return dir;
   } else
      return dir;
}

static int CompareFileTimes(const void *p1, const void *p2)
{
   const t_diskobj *dobj1 = *(const t_diskobj **)p1;
   const t_diskobj *dobj2 = *(const t_diskobj **)p2;
   int dir, x;

   dir = dobj2->updir - dobj1->updir;
   if (dir == 0) {
      dir = (dobj2->isdir) - (dobj1->isdir);
      if (dir==0)
         return (x = dobj2->t.tm_year - dobj1->t.tm_year) ? x :
                (x = dobj2->t.tm_mon - dobj1->t.tm_mon) ? x :
                (x = dobj2->t.tm_mday - dobj1->t.tm_mday) ? x :
                (x = dobj2->t.tm_hour - dobj1->t.tm_hour) ? x :
                dobj2->t.tm_min - dobj1->t.tm_min;
      else
         return dir;
   } else
      return dir;
}

static int CompareFileTimesDescending(const void *p1, const void *p2)
{
   const t_diskobj *dobj1 = *(const t_diskobj **)p1;
   const t_diskobj *dobj2 = *(const t_diskobj **)p2;
   int dir,x;

   dir = dobj2->updir - dobj1->updir;
   if (dir == 0) {
      dir = (dobj2->isdir) - (dobj1->isdir);
      if (dir==0)
         return (x = dobj1->t.tm_year - dobj2->t.tm_year) ? x :
                (x = dobj1->t.tm_mon - dobj2->t.tm_mon) ? x :
                (x = dobj1->t.tm_mday - dobj2->t.tm_mday) ? x :
                (x = dobj1->t.tm_hour - dobj2->t.tm_hour) ? x :
                dobj1->t.tm_min - dobj2->t.tm_min;
      else
         return dir;
   } else
      return dir;
}

static int CompareFileSizes(const void *p1, const void *p2)
{
   const t_diskobj *dobj1 = *(const t_diskobj **)p1;
   const t_diskobj *dobj2 = *(const t_diskobj **)p2;
   int dir;

   dir = dobj2->updir - dobj1->updir;
   if (dir == 0) {
      dir = (dobj2->isdir) - (dobj1->isdir);
      if (dir==0)
         return dobj2->size - dobj1->size;
      else
         return dir;
   } else
      return dir;
}

static int CompareFileSizesDescending(const void *p1, const void *p2)
{
   const t_diskobj *dobj1 = *(const t_diskobj **)p1;
   const t_diskobj *dobj2 = *(const t_diskobj **)p2;
   int dir;

   dir = dobj2->updir - dobj1->updir;
   if (dir == 0) {
      dir = (dobj2->isdir) - (dobj1->isdir);
      if (dir==0)
         return dobj1->size - dobj2->size;
      else
         return dir;
   } else
      return dir;
}

#ifdef ALLEGRO_UNIX
static int CompareFileModes(const void *p1, const void *p2)
{
   const t_diskobj *dobj1 = *(const t_diskobj **)p1;
   const t_diskobj *dobj2 = *(const t_diskobj **)p2;
   int dir;

   dir = dobj2->updir - dobj1->updir;
   if (dir == 0) {
      dir = (dobj2->isdir) - (dobj1->isdir);
      if (dir==0)
         return dobj2->st_mode - dobj1->st_mode;
      else
         return dir;
   } else
      return dir;
}

static int CompareFileModesDescending(const void *p1, const void *p2)
{
   const t_diskobj *dobj1 = *(const t_diskobj **)p1;
   const t_diskobj *dobj2 = *(const t_diskobj **)p2;
   int dir;

   dir = dobj2->updir - dobj1->updir;
   if (dir == 0) {
      dir = (dobj2->isdir) - (dobj1->isdir);
      if (dir==0)
         return dobj1->st_mode - dobj2->st_mode;
      else
         return dir;
   } else
      return dir;
}
#else
static int CompareFileModes(const void *p1, const void *p2)
{
   const t_diskobj *dobj1 = *(const t_diskobj **)p1;
   const t_diskobj *dobj2 = *(const t_diskobj **)p2;
   int dir;

   dir = dobj2->updir - dobj1->updir;
   if (dir == 0) {
      dir = (dobj2->isdir) - (dobj1->isdir);
      if (dir==0)
         return dobj2->attrib - dobj1->attrib;
      else
         return dir;
   } else
      return dir;
}

static int CompareFileModesDescending(const void *p1, const void *p2)
{
   const t_diskobj *dobj1 = *(const t_diskobj **)p1;
   const t_diskobj *dobj2 = *(const t_diskobj **)p2;
   int dir;

   dir = dobj2->updir - dobj1->updir;
   if (dir == 0) {
      dir = (dobj2->isdir) - (dobj1->isdir);
      if (dir==0)
         return dobj1->attrib - dobj2->attrib;
      else
         return dir;
   } else
      return dir;
}
#endif
extern void SortDiskObjects(t_dirinfo *dirinfo)
{
   int descending;
   descending = dirinfo->root->descending;
   switch (dirinfo->root->sorting) {
   case DO_SORT_TEXT:
      if (descending)
         qsort(dirinfo->dobjs, dirinfo->n, sizeof(t_diskobj*), CompareFileExtensionStringsDescending);
      else
         qsort(dirinfo->dobjs, dirinfo->n, sizeof(t_diskobj*), CompareFileExtensionStrings);
      break;
   case DO_SORT_NAME:
      if (descending)
         qsort(dirinfo->dobjs, dirinfo->n, sizeof(t_diskobj*), CompareFileNamesDescending);
      else
         qsort(dirinfo->dobjs, dirinfo->n, sizeof(t_diskobj*), CompareFileNames);
      break;
   case DO_SORT_TIME:
      if (descending)
         qsort(dirinfo->dobjs, dirinfo->n, sizeof(t_diskobj*), CompareFileTimesDescending);
      else
         qsort(dirinfo->dobjs, dirinfo->n, sizeof(t_diskobj*), CompareFileTimes);
      break;
   case DO_SORT_SIZE:
      if (descending)
         qsort(dirinfo->dobjs, dirinfo->n, sizeof(t_diskobj*), CompareFileSizesDescending);
      else
         qsort(dirinfo->dobjs, dirinfo->n, sizeof(t_diskobj*), CompareFileSizes);
      break;
   case DO_SORT_ATTRIB:
      if (descending)
         qsort(dirinfo->dobjs, dirinfo->n, sizeof(t_diskobj*), CompareFileModesDescending);
      else
         qsort(dirinfo->dobjs, dirinfo->n, sizeof(t_diskobj*), CompareFileModes);
      break;
   }
}

#define DOBJ_LESS(d1, d2) strcmp(d1->name, d2->name) < 0
#define DOBJ_GT(d1, d2) strcmp(d1->name, d2->name) > 0

typedef struct t_btree t_btree;
struct t_btree {
   t_btree *left, *right;
   t_diskobj *dobj;
};

/* This function filters duplicate files. Duplicates may occur because multiple masks are used (specially because then handling of
   Allegro data files requires all *.dat-files), and some files may match more than one of the masks.
   The alorith is to just put the objects into a set to get rid of duplicates. The set is represented as a binary tree.  */
static void FilterDiskObjects(t_dirinfo *dirinfo)
{
   t_btree *filter, *it, *nodebuf, *nd;
   t_root *root;
   int i, j;

   root = dirinfo->root;
   if ((root->load_dat || strchr(root->masks, ';')) && dirinfo->n  > 0) {
      nodebuf = GetMem0(t_btree, dirinfo->n);
      i = 0;
      j = 0;
      /* Create the first node. */
      nd = nodebuf + j;
      j++;
      nd->dobj = dirinfo->dobjs[i];
      i++;
      /* 'filter' is the root of the set (bintree). */
      filter = nd;
      while (i < dirinfo->n) {
         /* Create next node. */
         nd = nodebuf + j;
         j++;
         nd->dobj = dirinfo->dobjs[i];
         /* Find where to insert it. */
         it = filter;
         while (1) {
            if (DOBJ_GT(nd->dobj, it->dobj)) {
               if (it->left) {
                  /* Move on to next level in the tree. */
                  it = it->left;
               } else {
                  /* Insert and move on to next node to insert. */
                  it->left = nd;
                  i++;
                  break;
               }
            } else if (DOBJ_LESS(nd->dobj, it->dobj)) {
               if (it->right) {
                  /* Move on to next level in the tree. */
                  it = it->right;
               } else {
                  /* Insert and move on to next node to insert. */
                  it->right = nd;
                  i++;
                  break;
               }
            } else { /* Then it is the same. */
               /* A duplicate: leave the one that is there already (possibly marked as dat-file) and skip the current, then move on to next
                  node to insert. */
               dirinfo->dobjs[i] = dirinfo->dobjs[--dirinfo->n];
               if (nd->dobj->is_dot_dat) {
                  dirinfo->ndirs--;
               }
               DestroyDiskObject(nd->dobj);
               break;
            }
         }
      }
      Release(nodebuf);
   }
}

extern int IsDirectoryObject(t_diskobj *dobj)
{
   return dobj->isdir;
}

extern t_dirinfo *GetGrandParent(t_diskobj *dobj)
{
   if (dobj->parent) {
      return dobj->parent->dobj->parent;
   }
   return NULL;
}

extern t_dirinfo *GetParent(t_diskobj *dobj)
{
   return dobj->parent;
}

extern const char *DiskObjectName(t_diskobj *dobj)
{
   return dobj->namev;
}

extern const char *DiskObjectExt(t_diskobj *dobj)
{
   return dobj->ext;
}

extern long DiskObjectSize(t_diskobj *dobj)
{
   return dobj->size;
}

extern int DiskObjectMode(struct t_diskobj *dobj)
{
   return dobj->attrib;
}

/* NOTE DiskObjectAttribString returns pointer to static memory. */
#ifdef ALLEGRO_UNIX
extern const char *DiskObjectModeString(t_diskobj *dobj)
{
   static char perm[20];
   char *s;

   if (dobj->parent->dat || dobj->updir)
      return "";
   s = perm;
   *s++ = ' ';
   if (dobj->st_mode & S_IRUSR)
      *s++ = 'r';
   else
      *s++ = '-';
   if (dobj->st_mode & S_IWUSR)
      *s++ = 'w';
   else
      *s++ = '-';
   if (dobj->st_mode & S_IXUSR)
      *s++ = 'x';
   else
      *s++ = '-';
   if (dobj->st_mode & S_IRGRP)
      *s++ = 'r';
   else
      *s++ = '-';
   if (dobj->st_mode & S_IWGRP)
      *s++ = 'w';
   else
      *s++ = '-';
   if (dobj->st_mode & S_IXGRP)
      *s++ = 'x';
   else
      *s++ = '-';
   if (dobj->st_mode & S_IROTH)
      *s++ = 'r';
   else
      *s++ = '-';
   if (dobj->st_mode & S_IWOTH)
      *s++ = 'w';
   else
      *s++ = '-';
   if (dobj->st_mode & S_IXOTH)
      *s++ = 'x';
   else
      *s++ = '-';
   *s = 0;
   return perm;
}
#else
extern const char *DiskObjectModeString(t_diskobj *dobj)
{
   static char attr[10];
   char *s;

   s = attr;
   *s++ = ' ';
   if (dobj->attrib & FA_RDONLY)
      *s++ = 'R';
   if (dobj->attrib & FA_HIDDEN)
      *s++ = 'H';
   if (dobj->attrib & FA_SYSTEM)
      *s++ = 'S';
   if (dobj->attrib & FA_ARCH)
      *s++ = 'A';
   *s = 0;
   return attr;
}
#endif

extern int DiskObjectDatafileObjectType(struct t_diskobj *dobj)
{
   return dobj->datafile_object_type;
}

extern int DiskObjectIsDatafileObject(struct t_diskobj *dobj)
{
   return dobj->isindat;
}

extern void *DiskObjectDatafileObject(struct t_diskobj *dobj)
{
   return dobj->datob;
}

extern int DiskObjectIsDotDat(struct t_diskobj *dobj)
{
   return dobj->is_dot_dat;
}

extern int IsUpDir(struct t_diskobj *dobj)
{
   return dobj->updir;
}

extern void DiskObjectTimeString(char *s, struct t_diskobj *dobj)
{
   if (dobj->no_time) {
      *s = 0;
   } else {
      sprintf(s, "%d-%02d-%02d %02d:%02d", dobj->t.tm_year+1900,dobj->t.tm_mon+1, dobj->t.tm_mday, dobj->t.tm_hour,dobj->t.tm_min);
   }
}

/* Returns NULL if no original name is present */
extern const char *DiskObjectDatSource(struct t_diskobj *dobj)
{
   return dobj->orig;
}

extern int DiskObjectIsPacked(struct t_diskobj *dobj)
{
   return dobj->pack;
}

extern const struct FONT *DiskObjectGetFont(struct t_diskobj *dobj)
{
   return dobj->font;
}

extern void DiskObjectSetExpanded(t_diskobj *dobj, int state)
{
   dobj->expanded = state;
}

extern int DiskObjectGetExpanded(t_diskobj *dobj)
{
   return dobj->expanded;
}

/* If the disk object has a visual representation, then an image will be
   created and registered, and the name which the image was registered under
   will be returned, else the empty string will be returned.
   Subsequent calls will use the same registration, for memory and performance
   improvements. */
extern const char *DiskObjectGetImage(struct t_diskobj *dobj)
{
   char *fn;
   int status, ok;

   if (dobj->image_loaded == NULL && !dobj->tried_loading) {
      ok = 0;
      dobj->tried_loading = 1;
      if (dobj->isindat) {
         if (dobj->datob) {
            status = RegisterDatafileImage(dobj->datob, dobj->namev);
            ok = status == 1;
         }
      } else if (!dobj->is_dot_dat && !dobj->isdir) { /* Not a directory (they already have images). */
         fn = MergePathAndFile(dobj->parent->path, dobj->name, 0);
         status = CguiLoadImage(fn, dobj->namev, IMAGE_TRANS_BMP, 0);
         ok = status == 0;
         errno = 0;
         Release(fn);
      }
      if (ok) {
         dobj->image_loaded = GetMem(char, strlen(dobj->namev) + 3);
         *dobj->image_loaded++ = '#';
         *dobj->image_loaded++ = ' ';
         strcpy(dobj->image_loaded, dobj->namev);
         dobj->image_loaded -= 2;
      }
   }
   if (dobj->image_loaded)
      return dobj->image_loaded;
   else
      return "";
}

extern void *DiskObjectGetUserData(t_diskobj *dobj)
{
   return dobj->root->data;
}

extern int DiskObjectIsInDirectoryPath(t_dirinfo *dirinfo, t_diskobj *dobj)
{
   if (dirinfo->dobj == dobj)
      return 1;
   if (dirinfo->dobj->parent)
      return DiskObjectIsInDirectoryPath(dirinfo->dobj->parent, dobj);
   else
      return 0;
}

/* --------- Operations on t_dirinfo data objects */

static void AddHiddenFileEntry(t_dirinfo *parent, t_diskobj *kid)
{
   if (parent) {
      parent->nhidden_system_files++;
      parent->hidden_system_files = ResizeMem(t_diskobj*, parent->hidden_system_files, parent->nhidden_system_files);
      parent->hidden_system_files[parent->nhidden_system_files - 1] = kid;
   }
}

static void AddFileEntry(t_dirinfo *parent, t_diskobj *kid)
{
   if (parent) {
      parent->n++;
      parent->dobjs = ResizeMem(t_diskobj*, parent->dobjs, parent->n);
      parent->dobjs[parent->n - 1] = kid;
   }
}

static t_diskobj *DiskObjectCreate(t_dirinfo *parent, const char *name, int attrib, time_t time, long size, int isindat)
{
   t_diskobj *dobj;

   dobj = GetMem0(t_diskobj, 1);
   dobj->parent = parent;
   if (parent)
      dobj->root = parent->root;
   dobj->name = MkString(name);
   dobj->namev = MkString(name);
   Dos2Win(dobj->namev, dobj->namev);
   dobj->ext = get_extension(dobj->namev);
   if (stricmp(dobj->ext, "dat") == 0)
      dobj->is_dot_dat = dobj->isdir = 1;
   else
      dobj->isdir = (attrib & FA_DIREC) != 0;
   dobj->attrib = attrib;
   dobj->t = *localtime(&time);
   dobj->size = size;
   dobj->isindat = isindat;
   if (dobj->is_dot_dat && parent) {
      parent->ndirs++;
   }
   dobj->updir = dobj->isdir && name[0] == '.' && name[1] == '.' && name[2] == 0;
   if (dobj->isdir) {
      dobj->dirinfo = CreateDirInfo(dobj, parent, isindat);
   }
   return dobj;
}

extern t_diskobj *DiskObjectCreateDat(const char *name, int attrib, t_dirinfo *parent, long size, int type, DATAFILE *dat, const struct tm *t,
                                      const char *orig)
{
   t_diskobj *dobj;
   dobj = DiskObjectCreate(parent, name, attrib, 0, size, 1);
   AddFileEntry(parent, dobj);
   dobj->datob = dat;
   dobj->datafile_object_type = type;
   if (type == DAT_FILE)
      dobj->isdir = 1;
   if (type == DAT_FONT)
      dobj->font = dat->dat;
   if (dobj->isdir && dobj->parent) {
      dobj->parent->ndirs++;
   }
   if (t==NULL)
      dobj->no_time = 1;
   else
      dobj->t = *t;
   dobj->orig = orig;
   return dobj;
}

static void CheckNextDataFile(void *data)
{
   t_diskobj **dobjp=data;
   t_diskobj *dobj;
   t_dirinfo *dirinfo;
   char *path;

   /* In case we might have advanced beyond the last we need to use the prev to get pointer to the directory. There will always be at least
      one object before the first data file (the .. directory). */
   dirinfo = dobjp[-1]->parent;
   if (dobjp < dirinfo->dobjs + dirinfo->n) {
      dobj = *dobjp;
      path = MergePathAndFile(dirinfo->path, dobj->name, 0);
      dobj->isdir = dobj->is_dot_dat = CheckDataFile(path, &dobj->pack);
      if (dobj->isdir) {
         dobj->dirinfo = CreateDirInfo(dobj, dirinfo, 0);
      }
      Release(path);
      dirinfo->loader->eventid = GenEvent(CheckNextDataFile, dobjp + 1, 0, 0);
   } else {
      MoveToNextStep(dirinfo);
   }
}

static t_loader *CreateLoader(t_dirinfo *dirinfo, int last_dir_reason)
{
   t_loader *loader;

   loader = GetMem0(t_loader, 1);
   loader->dirinfo = dirinfo;
   loader->last_dir_reason = last_dir_reason;
   return loader;
}

static void DestroyLoader(t_loader *loader)
{
   Release(loader->splitted_masks);
   Release(loader);
}

static void FinishLoading(t_dirinfo *dirinfo)
{
   t_loader *loader;
   t_root *root;

   loader = dirinfo->loader;
   root = dirinfo->root;
   if (root->CallBack)
      root->CallBack(root->data, DL_LOAD_FINISHED, dirinfo);
   FilterDiskObjects(dirinfo);
   SortDiskObjects(dirinfo);
   if (root->CallBack) {
      root->CallBack(root->data, loader->last_dir_reason, dirinfo);
   }
   DestroyLoader(loader);
   dirinfo->loader = NULL;
}

static void StopLoader(t_loader *loader)
{
   KillEvent(loader->eventid);
}

static void CheckProgressCallBack(t_dirinfo *dirinfo)
{
   t_loader *loader;
   t_root *root;

   loader = dirinfo->loader;
   root = dirinfo->root;
   if (root->CallBack && GetTicks() - loader->curtime >= root->call_back_interval) {
      loader->curtime = GetTicks();
      root->CallBack(root->data, DL_INTERVAL_ELAPSED, dirinfo);
   }
}

#ifdef ALLEGRO_UNIX
static void CollectModes(void *data)
{
   t_diskobj **dobjp=data;
   t_diskobj *dobj;
   t_dirinfo *dirinfo;
   struct stat st;
   char *path;
   int index, hindex;

   dobj = *dobjp;
   dirinfo = dobj->parent;
   path = MergePathAndFile(dirinfo->path, dobj->name, 0);
   stat(path, &st);
   dobj->st_mode = st.st_mode;
   dobj->inode = st.st_ino;
   Release(path);
   CheckProgressCallBack(dirinfo);
   dobjp++;
   index = dobjp - dirinfo->dobjs;
   hindex = dobjp - dirinfo->hidden_system_files;
   if ((index > 0 && index < dirinfo->n) || (hindex > 0 && hindex < dirinfo->nhidden_system_files)) {
      /* If next is within the range of either array, then process it. */
      dirinfo->loader->eventid = GenEvent(CollectModes, dobjp, 0, 0);
   } else if (index == dirinfo->n && dirinfo->nhidden_system_files > 0) {
      /* Else if we were processing the normal files and there are some hidden to process then go on with the hidden ones. */
      CollectModes(dirinfo->hidden_system_files);
   } else {
      /* Else nothing more to do with loading modes, move on to the next loading state. */
      MoveToNextStep(dirinfo);
   }
}

/* In case of unix we also want to pick the mode flags. First the normal "disk objects" then the "parked" (i.e. hidden). */
static void GetModeFlags(t_dirinfo *dirinfo)
{
   if (dirinfo->n > 0) {
      CollectModes(dirinfo->dobjs);
   } else if (dirinfo->nhidden_system_files > 0) {
      CollectModes(dirinfo->hidden_system_files);
   }
}
#endif
int dela=0;
/* Loads all entries of a directory using al_findnext (assuming al_findfirst has been called). If the current mode don't include system/hidden files,
   such files will be loaded and "parked". It works in a recursive manner (recursion over the event queue). */
static void LoadRestOfFiles(void *data)
{
   t_dirinfo *dirinfo=data;
   t_root *root;
   t_loader *loader;
   struct al_ffblk *info;
   t_diskobj *dobj;

   root = dirinfo->root;
   loader = dirinfo->loader;
   info = &loader->info;
   if (info->name[0] == '.' && info->name[1] == 0) {
   } else {
      if (loader->step == STATE_LOAD_DIRECTORIES && (info->attrib & FA_DIREC) == 0) {
         ;
      } else {
         dobj = DiskObjectCreate(dirinfo, info->name, info->attrib, info->time, info->size, 0);
         if (root->load_hs || !(info->attrib & (FA_HIDDEN|FA_SYSTEM))) {
            AddFileEntry(dirinfo, dobj);
         } else {
            AddHiddenFileEntry(dirinfo, dobj);
         }
         CheckProgressCallBack(dirinfo);
      }
   }
   if (al_findnext(info)) {
      al_findclose(info);
      MoveToNextStep(dirinfo);
   } else {
      dirinfo->loader->eventid = GenEvent(LoadRestOfFiles, dirinfo, dela, 0);
   }
}

/* Initialize for loading files (non-directories) of a certain mask, and pass on to the general function for lodaing entries of one directory. */
static void LoadFilesOfMask(t_dirinfo *dirinfo, char *current_mask)
{
   t_root *root;
   t_loader *loader;
   struct al_ffblk *info;
   char *pattern;

   root = dirinfo->root;
   loader = dirinfo->loader;
   info = &loader->info;
   pattern = MergePathAndFile(dirinfo->path, current_mask, 0);
   if (al_findfirst(pattern, info, FA_HIDDEN|FA_SYSTEM|FA_RDONLY|FA_ARCH)) {
      al_findclose(info);
      MoveToNextStep(dirinfo);
   } else {
      LoadRestOfFiles(dirinfo);
   }
   Release(pattern);
}

/* Loads files using the first of possibly multiple masks, by initializing buffers and calling a function for getting one mask. */
static void LoadFilesOfFirstMask(t_dirinfo *dirinfo)
{
   t_loader *loader;
   t_root *root;

   root = dirinfo->root;
   loader = dirinfo->loader;
   loader->splitted_masks = MkString(root->masks);
   LoadFilesOfMask(dirinfo, strtok_r(loader->splitted_masks, ";", &loader->strtok_buffer));
}

/* Initialize for loading subdirectories and pass on to general function for lodaing entries of one directory. */
static void LoadDirectories(t_dirinfo *dirinfo)
{
   t_loader *loader;
   char *pattern;
   struct al_ffblk *info;

   loader = dirinfo->loader;
   info = &loader->info;
   pattern = MergePathAndFile(dirinfo->path, "*", 0);
   if (al_findfirst(pattern, info, FA_DIREC|FA_HIDDEN|FA_SYSTEM|FA_RDONLY|FA_ARCH)) {
      al_findclose(info);
      MoveToNextStep(dirinfo);
   } else {
      LoadRestOfFiles(dirinfo);
   }
   Release(pattern);
}

/* Will move on to next state (or to next mask) and load the files of that. If the current load mode doesn't include system/hidden files, then
   such files will be loaded  and parked. The loading process works in a recursive manner (recursion over the event queue). */
static void MoveToNextStep(t_dirinfo *dirinfo)
{
   char *next_mask;
   t_loader *loader;
   t_root *root;

   loader = dirinfo->loader;
   root = dirinfo->root;
   switch (loader->step) {
   case STATE_LOAD_INIT:
      loader->step = STATE_LOAD_DIRECTORIES;
      loader->curtime = GetTicks();
      LoadDirectories(dirinfo);
      if (root->CallBack)
         root->CallBack(root->data, DL_LOAD_STARTED, dirinfo);
      break;
   case STATE_LOAD_DIRECTORIES:
      dirinfo->ndirs = dirinfo->n;
      if (root->CallBack)
         root->CallBack(root->data, DL_DIRECTORIES_FINISHED, dirinfo);
      if (root->load_dat) {
         loader->step = STATE_LOAD_DATAFILES;
         LoadFilesOfMask(dirinfo, "*.dat");
      } else {
         loader->step = STATE_LOAD_FILES_OF_MASK;
         LoadFilesOfFirstMask(dirinfo);
      }
      break;
   case STATE_LOAD_DATAFILES:
      if (root->CallBack)
         root->CallBack(root->data, DL_DATAFILES_LOADED, dirinfo);
      if (dirinfo->n > dirinfo->ndirs) {
         loader->step = STATE_CHECK_DATAFILES;
         CheckNextDataFile(dirinfo->dobjs + dirinfo->ndirs);
      } else {
         loader->step = STATE_LOAD_FILES_OF_MASK;
         LoadFilesOfFirstMask(dirinfo);
      }
      break;
   case STATE_CHECK_DATAFILES:
      if (root->CallBack)
         root->CallBack(root->data, DL_DATAFILES_CHECKED, dirinfo);
      loader->step = STATE_LOAD_FILES_OF_MASK;
      LoadFilesOfFirstMask(dirinfo);
      break;
   case STATE_LOAD_FILES_OF_MASK:
      next_mask = strtok_r(NULL, ";", &loader->strtok_buffer);
      if (next_mask) {
         LoadFilesOfMask(dirinfo, next_mask);
      } else {
#ifdef ALLEGRO_UNIX
         if (root->CallBack)
            root->CallBack(root->data, DL_FILES_LOADED, dirinfo);
         loader->step = STATE_LOAD_MODE_FLAGS;
         GetModeFlags(dirinfo);
#else
         FinishLoading(dirinfo);
         Attend(FILE_LOAD_EVENT_MASK);
#endif
      }
      break;
   case STATE_LOAD_MODE_FLAGS:
      FinishLoading(dirinfo);
      Attend(FILE_LOAD_EVENT_MASK);
      break;
   }
}

static t_dirinfo *CreateDirInfo(t_diskobj *dobj, t_dirinfo *parent, int isindat)
{
   t_dirinfo *dirinfo;

   dirinfo = GetMem0(t_dirinfo, 1);
   dirinfo->dobj = dobj;
   dirinfo->root = dobj->root;
   if (parent) {
      if (isindat) {
         dirinfo->path = msprintf("%s#%s", parent->path, dobj->name);
      } else {
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
         if (parent->path[0] == 0 && parent->dobj->parent == NULL) {
            dirinfo->path = msprintf("%s", dobj->name);
         } else
#endif
         if (parent->path[strlen(parent->path)-1] == OTHER_PATH_SEPARATOR) {
            dirinfo->path = msprintf("%s%s", parent->path, dobj->name);
         } else {
            dirinfo->path = msprintf("%s%c%s", parent->path, OTHER_PATH_SEPARATOR, dobj->name);
         }
      }
   } else {
      /* name is empty here */
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
      dirinfo->path = msprintf("");
#else
      dirinfo->path = msprintf("%c", OTHER_PATH_SEPARATOR);
#endif
   }
   dirinfo->dirty = 1;
   return dirinfo;
}

extern t_root *CreateRoot(void (*LoadProgressCallBack)(void *data, int reason, t_dirinfo *dirinfo), void *data, int callback_time_interval)
{
   t_root *root;

   root = GetMem0(t_root, 1);
   root->CallBack = LoadProgressCallBack;
   root->data =data;
   root->call_back_interval = callback_time_interval;
   /* Create a virtual object to the root. */
   root->dobj = DiskObjectCreate(NULL, "", FA_DIREC, 0, 0, 0);
   root->dobj->root = root;
   root->dirinfo = root->dobj->dirinfo;
   root->dirinfo->root = root;
   return root;
}

extern t_dirinfo *RootGetDirInfo(t_root *root)
{
   if (root)
      return root->dirinfo;
   return NULL;
}

static void DestroyDiskObject(t_diskobj *dobj)
{
   if (dobj->dirinfo) {
      DestroyDirInfo(dobj->dirinfo);
   }
   if (dobj->image_loaded) {
      RegisterImage(NULL, dobj->namev, 0, 0);
      Release(dobj->image_loaded);
   }
   Release(dobj->name);
   Release(dobj->namev);
   Release(dobj);
}

/* Clean the dirinfo struct from stuff related to the currently loaded disk data, to prepare for a new load. */
static void UnloadDirInfo(t_dirinfo *dirinfo)
{
   int i;
   for (i = 0; i < dirinfo->n; i++) {
      DestroyDiskObject(dirinfo->dobjs[i]);
   }
   Release(dirinfo->dobjs);
   dirinfo->n = 0;
   dirinfo->dobjs = NULL;

   for (i = 0; i < dirinfo->nhidden_system_files; i++) {
      DestroyDiskObject(dirinfo->hidden_system_files[i]);
   }
   Release(dirinfo->hidden_system_files);
   dirinfo->nhidden_system_files = 0;
   dirinfo->hidden_system_files = NULL;

   if (dirinfo->dat) {
      unload_datafile(dirinfo->dat);
   }
   dirinfo->ndirs = 0;
   dirinfo->dirsize = 0;
   if (dirinfo->loader) {
      StopLoader(dirinfo->loader);
   }
}

extern void DestroyDirectoryTree(t_root *root)
{
   DestroyDiskObject(root->dobj);
   Release(root->masks);
   Release(root);
}

static void DestroyDirInfo(t_dirinfo *dirinfo)
{
   if (dirinfo) {
      Release(dirinfo->path);
      Release(dirinfo->pwd);
      UnloadDirInfo(dirinfo);
      Release(dirinfo);
   }
}

extern int IsFileDotDat(t_dirinfo *dirinfo)
{
   return dirinfo->is_file_dot_dat;
}

/* Returns the datafile if there is any otherwise NULL */
extern struct DATAFILE *GetDataFile(t_dirinfo *dirinfo)
{
   return dirinfo->dat;
}

/* Returns true if dirinfo contains a list of datafile-objects. */
extern int IsDataFileList(t_dirinfo *dirinfo)
{
   return dirinfo->dat != NULL;
}

/* Returns a pointer to the full path of the directory. */
extern const char *DirInfoGetPath(t_dirinfo *dirinfo)
{
   if (dirinfo) {
      return dirinfo->path;
   }
   return NULL;
}

/* Because of potentially slow loading (the directory might contain an arbitrary number of objects), the lodaing is executed in a recursive style
   using an event queue. The loading goes in the following steps:
   - Directories (all directories matching the mask "*").
   - Datafiles (optional loading of normal files which matches the mask "*.dat").
   - Check datafiles (optional, checks if the "*.dat" files are really data-files).
   - Non-directories (load normal disk files matching the mask).
   -  ... (there may an arbitrary number of masks)
   - Mode flags (unix only)
   Each of these steps ends with closing the necessary activities of the current step and initializes for next step. Within each step there
   is a "recursion" over an event queue, e.g. get one file and put the same function into the queue. This will act as backgroud loading, make
   a graphical interface free to handle other events "simultaneously". Also a call-back can be installed to get calls at certain time intervals. */
static int LoadFileSystemObjects(t_dirinfo *dirinfo, int last_dir_reason)
{
   t_root *root;
   int aret, ok=0;
   root = dirinfo->root;
   if (IsRoot(dirinfo->path) || (file_exists(dirinfo->path, FA_DIREC|FA_HIDDEN|FA_SYSTEM|FA_RDONLY|FA_ARCH, &aret) && (aret & FA_DIREC))) {
      dirinfo->loader = CreateLoader(dirinfo, last_dir_reason);
      MoveToNextStep(dirinfo);
      dirinfo->dirty = 0;
      ok = 1;
   }
   return ok;
}

static void ReSortTree(t_dirinfo *dirinfo)
{
   int i;
   if (dirinfo) {
      SortDiskObjects(dirinfo);
      for (i=0; i<dirinfo->n; i++) {
         ReSortTree(dirinfo->dobjs[i]->dirinfo);
      }
   }
}

static void ActivateHiddenSystemFiles(t_dirinfo *dirinfo)
{
   int i, j;

   j = dirinfo->n;
   dirinfo->n += dirinfo->nhidden_system_files;
   if (dirinfo->n > 0) {
      dirinfo->dobjs = ResizeMem(t_diskobj*, dirinfo->dobjs, dirinfo->n);
      for (i=0; i<dirinfo->nhidden_system_files; i++, j++) {
         dirinfo->dobjs[j] = dirinfo->hidden_system_files[i];
      }
      Release(dirinfo->hidden_system_files);
      dirinfo->hidden_system_files = NULL;
      dirinfo->nhidden_system_files = 0;
   }
}

#ifdef ALLEGRO_UNIX
   #define IS_HIDDEN(d) (*(d)->name == '.')
#else
   #define IS_HIDDEN(d) ((d)->attrib & (FA_HIDDEN|FA_SYSTEM))
#endif
static void ParkHiddenSystemFiles(t_dirinfo *dirinfo)
{
   int i, j, k;

   if (dirinfo->n > 0) {
      dirinfo->hidden_system_files = GetMem0(t_diskobj*, dirinfo->n); /* Be generous... */
      j = 0;
      k = 0;
      for (i=0; i<dirinfo->n; i++) {
         if (IS_HIDDEN(dirinfo->dobjs[i])) {
            dirinfo->hidden_system_files[j++] = dirinfo->dobjs[i];
         } else {
            dirinfo->dobjs[k++] = dirinfo->dobjs[i];
         }
      }
      dirinfo->nhidden_system_files = j;
      dirinfo->n = k;
      if (j == 0) {
         Release(dirinfo->hidden_system_files);
         dirinfo->hidden_system_files = NULL;
      }
   }
}

static void RearrangeHiddenSystemFiles(t_dirinfo *dirinfo)
{
   int i;

   if (dirinfo) {
      if (dirinfo->root->load_hs) {
         ActivateHiddenSystemFiles(dirinfo);
      } else {
         ParkHiddenSystemFiles(dirinfo);
      }
      for (i=0; i<dirinfo->n; i++) {
         RearrangeHiddenSystemFiles(dirinfo->dobjs[i]->dirinfo);
      }
   }
}

static void MakeCacheDirty(t_dirinfo *dirinfo)
{
   int i;
   if (dirinfo) {
      dirinfo->dirty = 1;
      for (i=0; i<dirinfo->n; i++) {
         MakeCacheDirty(dirinfo->dobjs[i]->dirinfo);
      }
      for (i=0; i<dirinfo->nhidden_system_files; i++) {
         MakeCacheDirty(dirinfo->hidden_system_files[i]->dirinfo);
      }
   }
}

extern void SetLoadHiddenSystem(t_root *root, int mode)
{
   root->load_hs = mode;
   RearrangeHiddenSystemFiles(root->dirinfo);
}

extern void SetLoadDat(t_root *root, int mode)
{
   root->load_dat = mode;
   MakeCacheDirty(root->dirinfo);
}

extern void SetLoadMasks(t_root *root, const char *masks)
{
   Release(root->masks);
   if (masks && *masks) {
      root->masks = MkString(masks);
   } else {
      root->masks = MkString("*");
   }
   MakeCacheDirty(root->dirinfo);
}

extern void SetLoadSorting(t_root *root, int sorting, int descending)
{
   root->sorting = sorting;
   root->descending = descending;
   ReSortTree(root->dirinfo);
}

/* Reloads the directory already loaded in `*dirinfo'. This can be useful in case the contents of the disk might have been changed. In case the directory
   that `*dirinfo' represents does not exist any longer, it will be updated with the closest existing ancestor, and the return value will be 0, else 1. */
static int DirInfoReLoadDirectory2(t_dirinfo **dirinfop, int last_dir_reason)
{
   int is_file_dot_dat, ok=0;
   t_dirinfo *dirinfo, *datroot;

   if (dirinfop && *dirinfop) {
      dirinfo = *dirinfop;
      is_file_dot_dat = dirinfo->dobj->is_dot_dat;
      UnloadDirInfo(dirinfo);
      if (is_file_dot_dat) {
         /* We don't need the pack and pwd parameters, that info is already in the dirinfo struct. */
         Release(dirinfo->pwd);
         dirinfo->pwd = NULL;
         dirinfo->dat = LoadPossibleDataFile(dirinfo->path, "", &dirinfo->pack, &dirinfo->pwd);
         if (dirinfo->dat) {
            GenerateDatafileList(dirinfo, dirinfo->path, dirinfo->dat, &dirinfo->is_file_dot_dat);
            dirinfo->dirty = 0;
            ok = 1;
            if (dirinfo->root->CallBack) {
               dirinfo->root->CallBack(dirinfo->root->data, last_dir_reason, dirinfo);
            }
         }
         Attend(FILE_LOAD_EVENT_MASK);
      } else if (dirinfo->dobj->isindat) {
         /* If we are in a datafile, the actual datafile must first be loaded. */
         datroot = dirinfo->dobj->parent;
         while (datroot && datroot->dat == NULL) {
            datroot = datroot->dobj->parent;
         }
         if (datroot) {
            dirinfo = *dirinfop = DirInfoLoadDirectoryFromPath(dirinfo->root, dirinfo->path);
            ok = 1;
            if (dirinfo->root->CallBack) {
               dirinfo->root->CallBack(dirinfo->root->data, last_dir_reason, dirinfo);
            }
         }
      } else {
         ok = LoadFileSystemObjects(dirinfo, last_dir_reason);
      }
      if (!ok) {
         if (dirinfo->dobj->parent) {
            *dirinfop = DirInfoLoadDirectoryFromPath(dirinfo->root, dirinfo->dobj->parent->path);
         } else {
            /* This should not be possible. */
            *dirinfop = NULL;
         }
      }
   }
   return ok;
}

/* Will return a pointer to the directory info of the direcory associated with `dobj'. In case `dobj' is not a directory it will return a pointer to
   the closest ancestor. */
static t_dirinfo *DirInfoLoadDirectoryFromDiskObject2(t_diskobj *dobj, int last_dir_reason)
{
   t_dirinfo *dirinfo;

   dirinfo = dobj->dirinfo;
   if (dirinfo) {
      if (dirinfo->dirty) {
         DirInfoReLoadDirectory2(&dirinfo, last_dir_reason);
      } else {
         if (dirinfo->root->CallBack) {
            dirinfo->root->CallBack(dirinfo->root->data, last_dir_reason, dirinfo);
         }
      }
   } else {
      if (dobj->parent) {
         dirinfo = DirInfoLoadDirectoryFromPath(dobj->root, dobj->parent->path);
      } else {
         /* This should not be possible. */
         dirinfo = NULL;
      }
   }
   return dirinfo;
}

/* Returns a pointer to the dirinfo object of the passed dobj `current_root'. That dirinfo should have its path set, but the forest of kids not loaded.
   So, the task is to load the file list of the directory that this dirinfo represents (or, in case apllicable, the correspnding datafile), and
   recursively load the kid as indicated by `rest_of_path'. In order to carry this out we need to extract the first part of the path, which may
   be separated from the rest in some different manners: in case of a normal disk endtry it is the usual path separator of the current platform,
   or it may be the datafile separator in case the current root is a disk data file.
   In case of error it tries to make some error handling, i.e. give up and return the pointer of the parent's dirinfo. I haven't tested
   this sice I haven't succeeded in creating an error case. However the idea is that it should always return a proper pointer, but reading
   the code without its context, indicates that also a NULL pointer may be returned. */
static t_dirinfo *DirInfoLoadDirectoryFromPath2(t_diskobj *current_root, char *rest_of_path, int *any_dirty)
{
   t_dirinfo *dirinfo=NULL, *datroot;
   t_diskobj *kid=NULL;
   int i, last_dir_reason;
   char *p, *dat_path, *kidname, *restofpath="", *new_root_path;
   char saved_delim=OTHER_PATH_SEPARATOR; /* Initialize to stop compiler complaining. */

   kidname = rest_of_path;
   p = strchr(rest_of_path, OTHER_PATH_SEPARATOR);
   dat_path = strchr(rest_of_path, '#');

   /* Any of the two delimiters may or may not occure, we just want the fist one (if there is any). */
   if (p==NULL && dat_path==NULL) {
      restofpath = "";
   } else if (p==NULL) {
      p = dat_path;
   } else if (dat_path==NULL) {
      ;
   } else {
      /* Both delimiters are present - take the first */
      if (dat_path < p) {
         p = dat_path;
      } else {
         ;
      }
   }
   if (p) {
      /* There is some delimiter (of either type), and p points to the first one, but we don't know which type o delimiter. */
      saved_delim = *p;
      *p = 0;
      restofpath = p + 1;
      while (*restofpath==OTHER_PATH_SEPARATOR) {
         /* In case there are excess delimiters we just wind them. */
         restofpath++;
      }
   }
   if (current_root->dirinfo == NULL) {
      /* Error case. */
      if (current_root->parent)
         dirinfo = current_root->parent;
   } else {
      if (current_root->dirinfo->dirty) {
         *any_dirty = 1;
         if (*rest_of_path)
            last_dir_reason = DL_DIRECTORY_END;
         else
            last_dir_reason = DL_END;
         /* Load either a datafile or the file list of the directory, or a drive list. This is not recursive functions. */
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
         if (current_root->parent == NULL) {
            int nodisks;
            char **disklist = GetDiskList(&nodisks, "");
            t_diskobj *dobj;
            for (i=0; i<nodisks; i++) {
               dobj = DiskObjectCreate(current_root->dirinfo, disklist[i], FA_DIREC, 0, 0, 0);
               AddFileEntry(current_root->dirinfo, dobj);
            }
            dirinfo = current_root->dirinfo;
            dirinfo->dirty = 0;
            dirinfo->ndirs = dirinfo->n;
         } else
#endif
         if (current_root->isindat) {
            datroot = current_root->parent;
            while (datroot && datroot->dat == NULL) {
               datroot = datroot->dobj->parent;
            }
            dirinfo = current_root->dirinfo;
            UnloadDirInfo(dirinfo);
            GenerateDatafileList(dirinfo, dirinfo->path, datroot->dat, &dirinfo->is_file_dot_dat);
         } else {
            dirinfo = DirInfoLoadDirectoryFromDiskObject2(current_root, last_dir_reason);
         }
         if (dirinfo == NULL) {
            if (current_root->parent) {
               return current_root->parent;
            }
         }
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
         if (current_root->parent)
#endif
         if (!dirinfo->is_file_dot_dat && !dirinfo->dobj->isindat) {
            Invite(FILE_LOAD_EVENT_MASK, 0, "File loader");
         }
      }
      /* current_root->dirinfo is always valid here. */
      if (current_root->dirinfo->path[strlen(current_root->dirinfo->path)-1] == OTHER_PATH_SEPARATOR) {
         new_root_path = msprintf("%s%s", current_root->dirinfo->path, kidname);
      } else {
         new_root_path = MergePathAndFile(current_root->dirinfo->path, kidname, 0);
      }
      if (*kidname) {
         for (i=0; i<current_root->dirinfo->n; i++) {
            kid = current_root->dirinfo->dobjs[i];
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
            if (stricmp(kid->name, kidname) == 0) {
#else
            if (strcmp(kid->name, kidname) == 0) {
#endif
               break;
            }
         }
         if (p) {
            *p = saved_delim;
         }
         /* If path is proper, we should have found the directory, but in case not we stop recursion here. */
         if (i<current_root->dirinfo->n) {
            dirinfo = DirInfoLoadDirectoryFromPath2(kid, restofpath, any_dirty);
            if (dirinfo == NULL) {
               dirinfo = current_root->dirinfo;
            }
         }
      } else {
         if (p) {
            *p = saved_delim;
         }
         dirinfo = current_root->dirinfo;
      }
      Release(new_root_path);
   }
   return dirinfo;
}

/* Will return a pointer to the directory info of the direcory associated with `dobj'. In case `dobj' is not a directory it will return a pointer to
   the closest ancestor. */
extern t_dirinfo *DirInfoLoadDirectoryFromDiskObject(t_diskobj *dobj)
{
   return DirInfoLoadDirectoryFromDiskObject2(dobj, DL_END);
}

/* Returns a pointer to the directory info from the directory with path `path'. In case the info is already loaded into the cached directory tree,
   this will be re-used, else it will load and cache all the directories along the path. It will lways return a valid pointer. */
extern t_dirinfo *DirInfoLoadDirectoryFromPath(t_root *root, const char *path)
{
   t_dirinfo *dirinfo;
   char *p, *fixed_up_path;
   int any_dirty=0;

   p = MkString(path);
   Win2Dos(p, p);
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
   {
      int nodisks, i;
      char **disklist = GetDiskList(&nodisks, "");
      if (strlen(p) < 3 || p[1]!=DEVICE_SEPARATOR || p[2]!=OTHER_PATH_SEPARATOR) {
         *p = 0;
      }
      for (i=0; i<nodisks; i++) {
         if (toupper(*p) == toupper(*disklist[i]))
            break;
      }
      if (i == nodisks) {
         /* The requested path start with a non-existent unit. Change to any legal path. */
         for (i=0; i<nodisks; i++) {
            if ('C' == toupper(*disklist[i]))
               break;
         }
         if (i == nodisks) {
            if (nodisks >= 2) {
               Release(p);
               p = MkString(disklist[1]);
            } else if (nodisks >= 1) {
               Release(p);
               p = MkString(disklist[0]);
            } else {
               /* Don't know what to do. */
            }
         } else {
            Release(p);
            p = msprintf("C%c%c", DEVICE_SEPARATOR, OTHER_PATH_SEPARATOR);
         }
      }
   }
   fixed_up_path = p;
#else
   for (fixed_up_path=p; *fixed_up_path==OTHER_PATH_SEPARATOR; fixed_up_path++)
      ;
#endif
   dirinfo = DirInfoLoadDirectoryFromPath2(root->dobj, fixed_up_path, &any_dirty);
#ifdef ALLEGRO_UNIX
   if (dirinfo==NULL) {
      dirinfo = DirInfoLoadDirectoryFromPath2(root->dobj, "", &any_dirty);
   }
#endif
   Release(p);
   if (dirinfo->is_file_dot_dat || dirinfo->dobj->isindat) {
      dirinfo->root->CallBack(dirinfo->root->data, DL_END, dirinfo);
   }
   if (!any_dirty) {
      if (root->CallBack) {
         root->CallBack(root->data, DL_END, dirinfo);
      }
   }
   return dirinfo;
}

extern int DirInfoReLoadDirectory(t_dirinfo **dirinfop)
{
   return DirInfoReLoadDirectory2(dirinfop, DL_END);
}

/* Returns the directory info of the parent directory to `dirinfo' or NULL if this is the root of the directory tree. If the cache is fresh it will
   use it. If reloading is necessary and it is discovered that the parent does not exist any longer then it will return a pointer to the closest
   ancestor. */
extern t_dirinfo *DirInfoGetUpDirectory(t_dirinfo *dirinfo)
{
   t_dirinfo *newdirinfo = NULL;
   t_root *root;

   if (dirinfo) {
      root = dirinfo->root;
      if (dirinfo && dirinfo->dobj->parent) {
         newdirinfo = dirinfo->dobj->parent;
         if (newdirinfo->dirty) {
            DirInfoReLoadDirectory2(&newdirinfo, DL_END);
         } else {
            if (root->CallBack)
               root->CallBack(root->data, DL_END, newdirinfo);
         }
      }
   }
   if (newdirinfo==NULL) {
      newdirinfo = dirinfo;
      root = dirinfo->root;
      if (root->CallBack)
         root->CallBack(root->data, DL_END, newdirinfo);
   }
   return newdirinfo;
}


extern int GetFileListIndex(t_dirinfo *dirinfo, const char *name)
{
   int i;

   for (i = 0; i < dirinfo->n; i++) {
      if (strcmp(dirinfo->dobjs[i]->namev, name) == 0) {
         break;
      }
   }
   return i;
}

extern char *DiskObjectGetFullPathDos(t_diskobj *dobj)
{
   if (dobj && dobj->parent) {
      return MergePathAndFile(dobj->parent->path, dobj->name, IsFileDotDat(dobj->parent)|IsDataFileList(dobj->parent));
   } else {
      return NULL;
   }
}

extern int DiskObjectIsInDatafile(t_diskobj *dobj)
{
   if (dobj->parent) {
      return IsDataFileList(dobj->parent);
   } else {
      return 0;
   }
}

extern char *DiskObjectGetLocation(t_diskobj *dobj)
{
   if (dobj->parent) {
      return dobj->parent->path;
   } else {
      return NULL;
   }
}

extern int DirInfoGetTotalSize(t_dirinfo *dirinfo)
{
   if (dirinfo == NULL)
      return 0;
   return dirinfo->dirsize;
}

extern int DirInfoHasSubdirs(t_dirinfo *dirinfo)
{
   if (dirinfo == NULL || dirinfo->dirty)
      return 0;
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
   if (dirinfo->dobj->parent == NULL)
      return dirinfo->ndirs > 0;
#endif
   return dirinfo->ndirs > 1; /* The ".." is not a sub dir */
}

extern int DirInfoGetNrOfSubdirs(t_dirinfo *dirinfo)
{
   if (dirinfo == NULL)
      return 0;
   return dirinfo->ndirs;
}

extern int DirInfoGetNrOfDiskObjects(t_dirinfo *dirinfo)
{
   if (dirinfo == NULL)
      return 0;
   return dirinfo->n;
}

extern t_diskobj *DirInfoGetDiskObject(t_dirinfo *dirinfo)
{
   if (dirinfo) {
      return dirinfo->dobj;
   }
   return NULL;
}

extern t_diskobj *DirInfoGetDiskObjectAt(t_dirinfo *dirinfo, int i)
{
   if (dirinfo && i < dirinfo->n) {
      return dirinfo->dobjs[i];
   }
   return NULL;
}

extern t_diskobj *DirInfoGetFirstDiskObject(t_dirinfo *dirinfo)
{
   if (dirinfo) {
      dirinfo->itall = -1;
      return DirInfoGetNextDiskObject(dirinfo);
   }
   return NULL;
}

extern t_diskobj *DirInfoGetNextDiskObject(t_dirinfo *dirinfo)
{
   if (dirinfo) {
      dirinfo->itall++;
      if (dirinfo->itall < dirinfo->n) {
         return dirinfo->dobjs[dirinfo->itall];
      }
   }
   return NULL;
}

extern t_diskobj *DiskObjectGetFirstDirectory(t_diskobj *dobj)
{
   if (dobj && dobj->dirinfo) {
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
      if (dobj->parent == NULL)
         dobj->dirinfo->itdir = -1;
      else
#endif
      dobj->dirinfo->itdir = 0;
      return DiskObjectGetNextDirectory(dobj);
   }
   return NULL;
}

extern t_diskobj *DiskObjectGetNextDirectory(t_diskobj *dobj)
{
   if (dobj && dobj->dirinfo) {
      dobj->dirinfo->itdir++;
      if (dobj->dirinfo->itdir < dobj->dirinfo->ndirs) {
         return dobj->dirinfo->dobjs[dobj->dirinfo->itdir];
      }
   }
   return NULL;
}

extern t_diskobj *DirInfoGetFirstFile(t_dirinfo *dirinfo)
{
   if (dirinfo) {
      dirinfo->itfile = dirinfo->ndirs - 1;
      return DirInfoGetNextFile(dirinfo);
   }
   return NULL;
}

extern t_diskobj *DirInfoGetNextFile(t_dirinfo *dirinfo)
{
   if (dirinfo) {
      dirinfo->itfile++;
      if (dirinfo->itfile < dirinfo->n) {
         return dirinfo->dobjs[dirinfo->itfile];
      }
   }
   return NULL;
}

extern t_dirinfo *DiskObjectGetDirInfo(t_diskobj *dobj)
{
   return dobj->dirinfo;
}
/*
void print_file_tree(t_dirinfo *dirinfo)
{
   int i;
   if (dirinfo) {
      printf("%s\n",dirinfo->dobj->name);
      for (i = 0; i < dirinfo->n; i++)
         print_file_tree(dirinfo->dobjs[i]->dirinfo);
   }
}*/
