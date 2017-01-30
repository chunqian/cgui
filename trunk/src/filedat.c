/* Module FILEDAT.C
   Contains operations on Allegro datafiles.
*/
#include <allegro.h>
#include <allegro/internal/aintern.h>
#include <stdio.h>
#include <string.h>

#include "cgui/dw_conv.h"

#include "datedit.h"
#include "fileimpq.h"
#include "cgui/mem.h"
#include "cgui.h"
#include "filedat.h"
#include "filebuff.h"
#include "files.h"

#ifndef ALLEGRO_DOS
#define Dos2Win(a, b)
#define Win2Dos(a, b)
#endif
#define EMPTY

#if ALLEGRO_SUB_VERSION > 0
#define text_mode(x)
#endif

static char **passwords;
static int npwd;

extern void TypeToString(char *s, int t)
{
   s[0] = (t>>24) & 0xff;
   s[1] = (t>>16) & 0xff;
   s[2] = (t>>8)  & 0xff;
   s[3] = (t>>0)  & 0xff;
   s[4] = 0;
}

extern int StringToType(const char *s)
{
   int t;

   t = datedit_clean_typename(s);
   return t;
/*
   int i;
   for (i = 0; datedit_object_info[i]->type != DAT_END; i++)
      if (datedit_object_info[i]->type == t)
         return t;
   return -1;*/
}

extern int CheckDataFile(const char *path, int *pack)
{
   char magic[4];
   FILE *fp;
   int pck, n;

   *pack = 0;
   fp = fopen(path, "r");
   if (fp) {
      n = fread(magic, 1, 4, fp);
      fclose(fp);
      if (n == 4) {
         if (magic[0] == (char)((F_NOPACK_MAGIC >> 24) & 0xff) && magic[1] == (char)((F_NOPACK_MAGIC >> 16) & 0xff) &&
            magic[2] == (char)((F_NOPACK_MAGIC >>  8) & 0xff) && magic[3] == (char)( F_NOPACK_MAGIC        & 0xff)) {

            return 1;
         } else if (magic[0] == (char)((F_PACK_MAGIC   >> 24) & 0xff) && magic[1] == (char)((F_PACK_MAGIC   >> 16) & 0xff) &&
                  magic[2] == (char)((F_PACK_MAGIC   >>  8) & 0xff) && magic[3] == (char)( F_PACK_MAGIC          & 0xff)) {
            *pack = 1;
            return 1;
         } else {
            if (CheckAnyPassword(path, &pck)) {
               *pack = pck;
               return 1;
            }
         }
      }
   }
   return 0;
}

extern void UpdateProperty(DATAFILE *dat, int type, const char *string)
{
   datedit_set_property(dat, type, string);
}

extern void RemoveProperty(DATAFILE *dat, const DATAFILE_PROPERTY *prop)
{
   datedit_set_property(dat, prop->type, "");
}

extern void AddProperty(DATAFILE *dat, const char *tstr, const char *string)
{
   datedit_set_property(dat, datedit_clean_typename(tstr), string);
}

extern void PropertyTypestring(char typestring[5], const DATAFILE_PROPERTY *prop)
{
   TypeToString(typestring, prop->type);
}

extern const char *PropertyString(const DATAFILE_PROPERTY *prop)
{
   return prop->dat;
}

extern const DATAFILE_PROPERTY *FirstProperty(const DATAFILE *datob)
{
   if (datob->prop->type == DAT_END)
      return NULL;
   else
      return datob->prop;
}

extern const DATAFILE_PROPERTY *NextProperty(const DATAFILE_PROPERTY *prop)
{
   prop++;
   if (prop->type == DAT_END)
      return NULL;
   else
      return prop;
}

extern const char *const *GetPasswordList(int *n)
{
   *n = npwd;
   return (const char *const *)passwords;
}

/* dobj is assumed to be a file in the file system. */
extern int CheckPassword(const char *path, struct t_diskobj *dobj, char *pwd)
{
   char *p, *name;
   int pack;
   DATAFILE *datl;

   name = MkString(DiskObjectName(dobj));
   Dos2Win(name, name);
   p = MergePathAndFile(path, name, 0); /* path/name should be a file system entry. */
   datl = LoadPossibleDataFile(p, pwd, &pack, NULL);
   Release(p);
   Release(name);
   if (datl) {
      unload_datafile(datl);
      return 1;
   } else
      return 0;
}

extern void CoreClearPasswords(void *data)
{
   int i;
   (void)data;

   for (i=0; i<npwd; i++)
      Release(passwords[i]);
   Release(passwords);
   passwords = NULL;
   npwd = 0;
}

extern void ReplacePassword(const char *oldpwd, const char *newpwd)
{
   int i,j;

   for (i=0; i<npwd; i++) {
      if (strcmp(passwords[i], oldpwd) == 0) {
         Release(passwords[i]);
         if (*newpwd) {
            for (j=0; j<npwd; j++)
               if (i != j && strcmp(passwords[j], newpwd) == 0)
                  break;
            if (j < npwd) { /* The new one is already there */
               passwords[i] = passwords[--npwd];
            } else {
               passwords[i] = MkString(newpwd);
            }
         } else {
            passwords[i] = passwords[--npwd];
         }
         break;
      }
   }
}

extern int IsPasswordInList(const char *pwd)
{
   int i;

   for (i=0; i<npwd; i++)
      if (strcmp(passwords[i], pwd) == 0)
         return 1;
   return 0;
}

extern void ClearPasswords(void)
{
   CoreClearPasswords(NULL);
   UnHookCguiDeInit(CoreClearPasswords, NULL);
}

extern void AddPassword(const char *pwd)
{
   if (passwords == NULL) {
      HookCguiDeInit(CoreClearPasswords, NULL);
   }
   if (IsPasswordInList(pwd))
      return;
   passwords = ResizeMem(char*, passwords, npwd + 1);
   passwords[npwd] = MkString(pwd);
   npwd++;
}

extern int CheckAnyPassword(const char *path, int *pack)
{
   DATAFILE *datl;
   int i;

   for (i = 0; i < npwd; i++) {
      datl = LoadPossibleDataFile(path, passwords[i], pack, NULL);
      if (datl) {
         unload_datafile(datl);
         return 1;
      }
   }
   return 0;
}

static void ReConvertProperties(DATAFILE *dat)
{
   #define MAX_PROPERTIES  1000
   DATAFILE_PROPERTY *prop;
   DATAFILE_PROPERTY tmp[MAX_PROPERTIES], *p;
   int i, n;

   for (; dat->type != DAT_END; dat++) {
      if (dat->prop) {
         for (i = n = 0; dat->prop[i].type != DAT_END;) {
            prop = dat->prop + i;
            if (need_uconvert(prop->dat, U_CURRENT, U_UTF8) && n < MAX_PROPERTIES) {
               /* store temporary the prop-info, remove it and restore after
                  wards, because we must not touch the memory allocated by
                  DAT. */
               p = tmp + n++;
               p->type = prop->type;
               p->dat = GetMem(char, uconvert_size(prop->dat, U_CURRENT, U_UTF8));
               if (prop->type == DAT_NAME) {
                  Dos2Win(prop->dat, prop->dat);
               }
               do_uconvert(prop->dat, U_CURRENT, p->dat, U_UTF8, -1);
               datedit_set_property(dat, prop->type, NULL);
            } else {
               i++;
            }
         }
         for (i = 0; i < n; i++) {
            datedit_set_property(dat, tmp[i].type, tmp[i].dat);
            Release(tmp[i].dat);
         }
      }
      if (dat->type == DAT_FILE)
         ReConvertProperties(dat->dat);
   }
}

static void ConvertProperties(DATAFILE *dat)
{
   #define MAX_PROPERTIES  1000
   DATAFILE_PROPERTY *prop;

   int i, n;

   for (; dat->type != DAT_END; dat++) {
      if (dat->prop) {
         for (i = n = 0; dat->prop[i].type != DAT_END; i++) {
            prop = dat->prop + i;
            if (prop->type == DAT_NAME && need_uconvert(prop->dat, U_CURRENT, U_UTF8) && n < MAX_PROPERTIES) {
               /* Both formats occupy the same amout of storage */
               Win2Dos(prop->dat, prop->dat);
               break; /* There is only one name */
            }
         }
      }
      if (dat->type == DAT_FILE)
         ConvertProperties(dat->dat);
   }
}

static void *MakeNewDataObject(int type)
{
   void *nd = NULL;
   int i;
   long size = 0;

   for (i=0; datedit_object_info[i]->type != DAT_END; i++) {
      if ((datedit_object_info[i]->type == type) && (datedit_object_info[i]->makenew)) {
         nd = datedit_object_info[i]->makenew(&size);
         break;
      }
   }
   return nd;
}

/* Store a datafile into the file system (not as a "sub-datafile"). If the
   pack-flag is negative the datafile will keep the packing state from the
   loading state.
   Note! The pointer dat may point to invalid memory after the call (an
   object containing administrative data may have been inserted/removed)
   Note! Name encoding is DOS not Win */
static int SaveDatafile2(DATAFILE **dat, const char *name, const char *password, int pack)
{
   const char *prop;
   char s[100];
   int i, found_packinfo = 0;
   DATAFILE *nd = NULL, *ndatob;
#if ALLEGRO_SUB_VERSION > 0
   DATEDIT_SAVE_DATAFILE_OPTIONS options={0,-1,0,0,0,0,0};
   int fixed_prop = 0;
#endif

   ReConvertProperties(*dat);
   if (pack >= 0) {
      for (i=0; (*dat)[i].type != DAT_END; i++) {
         if ((*dat)[i].type == DAT_INFO || (*dat)[i].type == DAT_ID('c','g','u','i')) {
            prop = get_datafile_property(*dat + i, DAT_PACK);
            if (prop && *prop) {
               sprintf(s, "%d", pack);
               datedit_set_property(*dat + i, DAT_PACK, s);
               found_packinfo = 1;
            } else if (found_packinfo)
               *dat = datedit_delete(*dat, i);
         }
      }
      if (!found_packinfo) {
         MakeNewDataObject(DAT_INFO);
         *dat = datedit_insert(*dat, &ndatob, "cgui_info", DAT_ID('c','g','u','i'), nd, 0);
         sprintf(s, "%d", pack);
         datedit_set_property(ndatob, DAT_PACK, s);
      }
   }
#if ALLEGRO_SUB_VERSION == 0
   return datedit_save_datafile(*dat, name, 0, pack, 0, 0, 0, password);
#else
   options.pack = pack;
   return datedit_save_datafile(*dat, name, &fixed_prop, &options, password);
#endif
}

/* Loads a datafile, and strips reduntant info-objects. If multiple
   info-objects, the first one containing pack-info will be kept. If
   password is a non-empty string the load will fail if the password
   is incorrect (or if there is no password).
   If the password is the empty string, then the currently
   stored list of passwords will be scanned for possible matches.
   If the file was successfully opened with a password from
   the list `pwdmatch' will point to that password, else to the empty string.
   Note! The string pointed to by `pwdmatch' will be invalid memory
   after `ClearPasswords' is called.
   `pwdmatch' can be NULL-pointer to ignore the auto-password detection to
   be returned.
   */
static DATAFILE *LoadDataFile2(const char *name, const char *password, char **pwdmatch)
{
   DATAFILE *dat;
   const char *prop;
   int i, found_packinfo = 0, global_error;

   if (pwdmatch)
      *pwdmatch = MkString("");
   global_error = *allegro_errno;
   dat = datedit_load_datafile(name, 1, password);
   if (*password == 0 && dat == NULL) {
      for (i = 0; i < npwd; i++) {
         dat = datedit_load_datafile(name, 1, passwords[i]);
         if (dat) {
            if (pwdmatch)
               *pwdmatch = MkString(passwords[i]);
            break;
         }
      }
   }
   if (dat) {
      *allegro_errno = global_error;
      /* Filter redundant info-objects */
      for (i=0; dat[i].type != DAT_END; ) {
         if (dat[i].type == DAT_INFO || dat[i].type == DAT_ID('c','g','u','i')) {
            if (found_packinfo) {
               dat = datedit_delete(dat, i);
            } else {
               prop = get_datafile_property(dat + i, DAT_PACK);
               if (prop == NULL || *prop == 0)
                  dat = datedit_delete(dat, i);
               else {
                  i++;
                  found_packinfo = 1;
               }
            }
         } else {
            i++;
         }
      }
      /* Convert property format to DOS-format (to conform to what is returned
         from the disk-system) */
      ConvertProperties(dat);
   }
   if (*allegro_errno == 1)
      *allegro_errno = 0;
   return dat;
}

extern int GetDataTypeList(const char *ext, t_typelist **ptl)
{
   int i, j, ntypes, nmatching = 0;
   t_typelist *tl;
   char *extlist, *e;

   for (ntypes = 0; datedit_grabber_info[ntypes]->type != DAT_END; ntypes++)
      ;
   if (ntypes > 0) {
      tl = GetMem0(t_typelist, ntypes + 1);
      for (i = 0; datedit_grabber_info[i]->type != DAT_END; i++) {
         tl[i].type = datedit_grabber_info[i]->type;
         if (datedit_grabber_info[i]->grab_ext && *datedit_grabber_info[i]->grab_ext) {
            extlist = MkString(datedit_grabber_info[i]->grab_ext);
            for (e = strtok(extlist, ";"); e; e = strtok(NULL, ";")) {
               if (stricmp(ext, e) == 0) {
                  tl[i].matchext = 1;
                  nmatching++;
               }
            }
            Release(extlist);
         }
      }
      for (j = 0; j < ntypes; j++) {
         for (i = 0; datedit_object_info[i]->type != DAT_END; i++) {
            if (datedit_object_info[i]->type == tl[j].type) {
               tl[j].desc = datedit_object_info[i]->desc;
               break;
            }
         }
      }
      for (j = 0; j < ntypes; j++)
         if (tl[j].type == DAT_DATA)
            break;
      if (j >= ntypes) {
         tl[ntypes].type = DAT_DATA;
         tl[ntypes].desc = "Binary data";
         j = ntypes;
      }
      if (nmatching == 1 && stricmp(ext, "txt") == 0)
         tl[j].matchext = 1;
      ntypes++;
      *ptl = tl;
   } else
      *ptl = NULL;
   return ntypes;
}

extern DATAFILE *FindNestedDatObj(DATAFILE *datl, char *dfp)
{
   DATAFILE *datob = NULL;
   char *end;
   int c;

   end = dfp;
   while (*end && datl) {
      end = EndOfFirstPart(dfp);
      c = *end;
      *end = 0;
      datob = find_datafile_object(datl, dfp);
      *end = c;
      if (*end)
         dfp = end + 1;
      if (datob) {
         if (datob->type == DAT_FILE) {
            datl = datob->dat;
         } else {
            datl = NULL;
         }
      } else {
         return NULL;
      }
   }
   if (*end) {
      return NULL;
   }
   return datob;
}

static int GetDatafileIndex(DATAFILE *datl, DATAFILE *datob)
{
   int i;

   for (i = 0; ; i++)
     if (datl + i == datob)
        return i;
   return -1;
}

extern int RemoveDatafileObjectByName(const char *dir, const char *filename)
{
   int i, status = 1;
   char *delim, *path, *fn, *pwd;
   DATAFILE *datmem, *datl, *prevdatob, *datob = NULL;

   path = MkString(dir);
   Win2Dos(path, path);
   fn = MkString(filename);
   Win2Dos(fn, fn);
   delim = strchr(path, '#');
   if (delim)
      *delim = 0;
   datl = datmem = LoadDataFile2(path, "", &pwd);
   if (datl) {
      if (delim) {
         datob = FindNestedDatObj(datl, delim + 1);
         if (datob && datob->type == DAT_FILE)
            datl = datob->dat;
         else
            status = -2;
      }
      prevdatob = datob;
      if (datl && status == 1)
         datob = find_datafile_object(datl, fn);
      if (datob == NULL)
         status = -2;
      if (status == 1) {
         i = GetDatafileIndex(datl, datob);
         if (prevdatob)
            prevdatob->dat = datedit_delete(datl, i);
         else
            datmem = datedit_delete(datmem, i);
         if (!SaveDatafile2(&datmem, path, pwd, -1))
            status = -2;
      }
      unload_datafile(datmem);
   } else {
      status = -2;
   }
   Release(path);
   Release(fn);
   return status;
}

/* Assumes that `filename' is a data-file object. If destdir is a directory that data-file object will be exported to that directory. If it is adatafile or
   a datafile object, then it will just be insertet into thatdatafile.`ReplaceExisting' is a function that will be called if a file to exportalready exists.
   It is passed the filename and shall return 0 if replacingmust not be done, else non-0.Return value of `PasteADatafileObject' is 0 on success, 1 if error,
   and-1 if the user rejects overwriting (i.e. if `ReplaceExistingRequest' haswas called and returned 0).*/
extern int PasteADatafileObject(const char *srcdir, const char *filename, const char *destdir, int dest_is_dat, DATAFILE **datorg,
                                int (*ReplaceExistingRequest)(const char *fn))
{
   char *delim, *delimd, *expname, *dfp, *dfpd, *ddir, *sdir, *fn, *pwd;
   const char *orgname, *tmp;
   DATAFILE_PROPERTY *prop;
   int error = 0, ok = 1, date_set = 0, i;
   void *v;
   DATAFILE *datl, *datob = NULL, *datmem, *datmemd = NULL, *newd = NULL;
   DATAFILE *db = NULL, *datld, *d;

   ddir = MkString(destdir);
   Win2Dos(ddir, ddir);
   sdir = MkString(srcdir);
   Win2Dos(sdir, sdir);
   fn = MkString(filename);
   Win2Dos(fn, fn);
   delim = strchr(sdir, '#');
   if (delim) {
      *delim = 0;
      dfp = delim + 1;
   } else
      dfp = "";
   datl = datmem = LoadDataFile2(sdir, "", NULL);
   if (datl) {
      if (*dfp) {
         datob = FindNestedDatObj(datl, dfp);
         if (datob && datob->type == DAT_FILE)
            datl = datob->dat;
         else
            error = 1;
      }
      if (datl && !error)
         datob = find_datafile_object(datl, fn);
      error |= datob == NULL;
      if (!error) {
         if (dest_is_dat) { /* dest is an allegro data-file: just insert */
            delimd = strchr(ddir, '#');
            if (delimd) {
               *delimd = 0;
               dfpd = delimd + 1;
            } else
               dfpd = "";
            /* If caller has not loaded the dest datfile (normally that has been
               done), then we must load it now. If it is loaded and the object is
               on the top level, then we also need to load it to not corrupt
               callers data */
            if (*datorg)
               datld = *datorg;
            else
               datld = datmemd = LoadDataFile2(ddir, "", &pwd);
            if (datld) {
               if (*dfpd) {
                  db = FindNestedDatObj(datld, dfpd);
                  if (db && db->type == DAT_FILE)
                     datld = db->dat;
                  else
                     error = 1;
               }
               if (!error) {
                  d = find_datafile_object(datld, fn);
                  if (d) {
                     ok = ReplaceExistingRequest(fn);
                     if (ok) {
                        i = GetDatafileIndex(datld, d);
                        datld = datedit_delete(datld, i);
                     } else
                        error = -1;
                  }
                  if (ok) {
                     datld = datedit_insert(datld, &newd, fn, datob->type, datob->dat, datob->size);
                     if (datld) {
                        if (*dfpd == 0) { /* top datafile reallocated */
                           if (*datorg)
                              *datorg = datld;
                           else
                              datmemd = datld;
                        } else
                           db->dat = datld;
                     } else
                        error = 1;
                  }
               }
               if (newd && !error) {
                  v = newd->dat;
                  for (prop = datob->prop; prop && prop->type != DAT_END; prop++) {
                     datedit_set_property(newd, prop->type, prop->dat);
                     if (prop->type == DAT_DATE)
                        date_set = 1;
                  }
                  if (!date_set)
                     datedit_set_property(newd, DAT_DATE, datedit_ftime2asc(time(NULL)));
                  if (*datorg)
                     error = !SaveDatafile2(datorg, ddir, pwd, -1);
                  else
                     error = !SaveDatafile2(&datmemd, ddir, pwd, -1);
                  for (d = datld; d->type != DAT_END; d++) {
                     if (d->dat == v) {
                        d->dat = NULL;
                        break;
                     }
                  }
               } else if (ok)
                  error = 1;
            } else
               error = 1;
            if (datmemd)
               unload_datafile(datmemd);
            if (delimd)
               *delimd = '#';
         } else { /* dest is a file-system directory: export */
            tmp = get_datafile_property(datob, DAT_ORIG);
            orgname = get_filename(tmp);
            if (*orgname == 0)
               orgname = fn;
            expname = MergePathAndFile(ddir, orgname, 0); /* 0 means don't check (we know that ddir already continas a #) */
            if (exists(expname)) {
               ok = ReplaceExistingRequest(fn);
               if (!ok)
                  error = -1;
            }
            if (ok) {
               error = !datedit_export(datob, expname);
               text_mode(-1);
            }
            Release(expname);
         }
      } else
         error = 1;
      unload_datafile(datmem);
   }
   Release(fn);
   Release(sdir);
   Release(ddir);
   return error;
}

/* Returns a dynamically allocated string that represents the shortest
   possible relative path to src from dest */
static char *CalculateRelativePath(const char *src, const char *dest, const char *dp)
{
   const char *s, *d, *lastsep, *lasts;
   char *r, up[]={'.','.',OTHER_PATH_SEPARATOR,0};
   int i, len, n;

   s = src;
   d = dest;
   /* Only needed for dos file systems */
   while (*s && *s!=OTHER_PATH_SEPARATOR && *s==*d) {
      s++;
      d++;
   }
   if (*s == *d) {
      for (lasts = s, lastsep = d; *d && *s==*d; s++, d++) {
         if (*d==OTHER_PATH_SEPARATOR) {
            lastsep = d;
            lasts = s;
         }
      }
      for (n=0, d = lastsep; *d; d++) {
         if (*d==OTHER_PATH_SEPARATOR) {
            n++;
         }
      }
      if (*dp) {
         n++;
         for (d = dp; *d && *d != '#'; d++)
            if (*d==OTHER_PATH_SEPARATOR)
               n++;
      }
      n--;
      len = strlen(lasts) + 3*n + 1;
      r = GetMem(char, len);
      *r = 0;
      for (i=0; i<n; i++)
         strcat(r, up);
      strcat(r, lasts+1);
   } else {  /* Only possible on dos file systems */
      r = MkString(src);
   }
   return r;
}

static int GrabNew(DATAFILE **dat, const char *src, char *relpath, const char *objname, int type, int bpp, int userelpath, const char *dpath)
{
   DATAFILE *tmp, *datob;
   char *s;
#if ALLEGRO_SUB_VERSION == 0
   char stype[5];
   (void)userelpath;
   (void)dpath;
   TypeToString(stype, type);
   tmp = datedit_grabnew(*dat, src, objname, stype, bpp, -1, -1, -1, -1);
#else
   DATEDIT_GRAB_PARAMETERS params;
   params.datafile = dpath;
   params.filename = src;
   params.name = objname;
   params.type = type;
   params.x = 0;
   params.y = 0;
   params.w = 0;
   params.h = 0;
   params.colordepth = bpp;
   params.relative = userelpath;
   tmp = datedit_grabnew(*dat, &params);
#endif
   if (tmp) {
      *dat = tmp;
      datob = find_datafile_object(tmp, objname);
      if (datob) {
         s = MkString(relpath);
         Dos2Win(s, s);
         datedit_set_property(datob, DAT_ORIG, s);
         Release(s);
      }
   }
   return tmp == NULL;
}

/* Win encoding. Checks if the imported object is already there */
static DATAFILE *FindMatchingDatafileObject(DATAFILE *datl, const char *spath)
{
   DATAFILE *datob;

   for (datob = datl; datob && datob->type!=DAT_END; datob++)
      if (strcmp(spath, get_datafile_property(datob, DAT_ORIG)) == 0)
         return datob;
   return  NULL;
}

static int ReImportObject(DATAFILE *datob, const char *spath, const char *curname, int userelpath, const char *dpath)
{
   int bpp = 0, w = 0, h = 0;
   BITMAP *bmp;

   if (datob->type == DAT_BITMAP || datob->type == DAT_XC_SPRITE || datob->type == DAT_C_SPRITE || datob->type == DAT_FONT) {
      bmp = datob->dat;
      bpp = bmp->vtable->color_depth;
      w = bmp->w;
      h = bmp->h;
   }
#if ALLEGRO_SUB_VERSION == 0
   {
      char stype[5];
      (void)userelpath;
      (void)dpath;
      TypeToString(stype, datob->type);
      return datedit_grabreplace(datob, spath, curname, stype, bpp, 0, 0, w, h);
   }
#else
   {
      DATEDIT_GRAB_PARAMETERS params;
      params.datafile = dpath;
      params.filename = spath;
      params.name = curname;
      params.type = datob->type;
      params.x = 0;
      params.y = 0;
      params.w = w;
      params.h = h;
      params.colordepth = bpp;
      params.relative = userelpath;
      return datedit_grabreplace(datob, &params);
   }
#endif
}

static int UpdateObjectList(DATAFILE *datl, const char *dpath)
{
   DATAFILE *datob;
   const char *orgpath, *oname;
   int error = 0;

   for (datob = datl; datob && datob->type!=DAT_END; datob++) {
      if (datob->type == DAT_FILE) {
         error |= UpdateObjectList(datob->dat, dpath);
      } else {
         orgpath = get_datafile_property(datob, DAT_ORIG);
         oname = get_datafile_property(datob, DAT_NAME);
         if (*orgpath)
            error |= ReImportObject(datob, orgpath, oname, 1, dpath);
         else
            error |= 1;
      }
   }
   return !error;
}

extern int UpdateObjectsWithinDatafile(const char *path, const char *name)
{
   char *dpath, *pwd;
   int ok = 0;
   DATAFILE *datl;

   dpath = MergePathAndFile(path, name, 0); /* 0 means don't check (we know that ddir already continas a #) */
   datl = LoadDataFile2(dpath, "", &pwd);
   if (datl) {
      ok = UpdateObjectList(datl, dpath);
      ok |= SaveDatafile2(&datl, dpath, pwd, -1);
      unload_datafile(datl);
      ok = 1;
   }
   Release(dpath);
   return ok;
}

/* This function imports a single file into a datafile. `destdir' may be the path to a nested DAT_FILE within a datafile. */
extern int ImportSingleFileIntoDataFile(const char *srcdir, const char *filename, const char *destdir)
{
   int error = 0, *types, nt, i, j, chgdestr = 0, rep=0, bpp;
   static int userelpath=1;
   char *dpath, *spath, stype[5], *objname=NULL, *pwd;
   const char *curname = NULL;
   char *delim, *dp, *stem = NULL, *relpath, *usepath;
   DATAFILE *datl, *datmem, *datob = NULL, *mdatob, *grabnewdat;
   AL_METHOD(void, destroyc, (void *)) = NULL;
   AL_METHOD(void, destroyxc, (void *)) = NULL;

   spath = MergePathAndFile(srcdir, filename, 0); /* 0 means don't check (we know that ddir already continas a #) */
   dpath = MkString(destdir);
   Win2Dos(dpath, dpath);
   delim = strchr(dpath, '#');
   if (delim) {
      *delim = 0;
      dp = delim + 1;
   } else
      dp = "";
   datl = datmem = LoadDataFile2(dpath, "", &pwd);
   if (*dp) {
      datob = FindNestedDatObj(datl, dp);
      if (datob == NULL || datob->type != DAT_FILE)
         error = 1;
   }
   if (!error) {
      relpath = CalculateRelativePath(spath, dpath, dp);
      if (datob) {
         mdatob = FindMatchingDatafileObject(datob->dat, relpath);
         if (mdatob == NULL)
            mdatob = FindMatchingDatafileObject(datob->dat, spath);
      } else {
         mdatob = FindMatchingDatafileObject(datmem, relpath);
         if (mdatob == NULL)
            mdatob = FindMatchingDatafileObject(datmem, spath);
      }
      if (mdatob)
         curname = get_datafile_property(mdatob, DAT_NAME);
      QueryImportData(curname, &rep, filename, &types, &nt, &objname, &bpp, &userelpath, 0);
      if (userelpath)
         usepath = relpath;
      else
         usepath = spath;
      if (rep && curname) {
         ReImportObject(mdatob, usepath, curname, 1, destdir);
      } else if (nt > 0) {
         Win2Dos(objname, objname);
         Win2Dos(spath, spath);
         Win2Dos(relpath, relpath);
         if (nt > 1) {
            stem = objname;
            objname = GetMem(char, strlen(stem) + 10);
         }
         for (i = 0; i < nt; i++) {
            if (types[i] == DAT_C_SPRITE || types[i] == DAT_C_SPRITE)
               chgdestr = 1;
            if (types[i] == DAT_FONT)
               set_color_depth(8);
            if (nt > 1) {
               TypeToString(stype, types[i]);
               sprintf(objname, "%s_%s", stem, stype);
            }
            if (datob) {
               grabnewdat = datob->dat;
               error = GrabNew(&grabnewdat, spath, usepath, objname, types[i], bpp, 1, dpath);
            } else {
               error = GrabNew(&datmem, spath, usepath, objname, types[i], bpp, 1, dpath);
            }
            if (types[i] == DAT_FONT)
               set_color_depth(screen->vtable->color_depth);
         }
         Release(types);
         if (stem)
            Release(stem);
      }
      error = !SaveDatafile2(&datmem, dpath, pwd, -1);
      for (i=0; i<32; i++) {
         if (_datafile_type[i].type == DAT_C_SPRITE) {
            if (chgdestr) {
               destroyc = _datafile_type[i].destroy;
               _datafile_type[i].destroy = (AL_METHOD(void, EMPTY ,(void *)))destroy_bitmap;
            }
            break;
         }
      }
      for (j=0; j<32; j++) {
         if (_datafile_type[j].type == DAT_XC_SPRITE) {
            if (chgdestr) {
               destroyxc = _datafile_type[j].destroy;
               _datafile_type[j].destroy = (AL_METHOD(void, EMPTY ,(void *)))destroy_bitmap;
            }
            break;
         }
      }
      unload_datafile(datmem);
      if (destroyc)
         _datafile_type[i].destroy = destroyc;
      if (destroyxc)
         _datafile_type[j].destroy = destroyxc;
      if (objname)
         Release(objname);
      Release(relpath);
   }
   Release(dpath);
   Release(spath);
   return error;
}

extern int RegisterDatafileImage(DATAFILE *datob, const char *name)
{
   switch (datob->type) {
   case DAT_BITMAP:
   case DAT_C_SPRITE:
      RegisterImage(datob->dat, name, IMAGE_TRANS_BMP, 0);
      break;
   case DAT_RLE_SPRITE:
      RegisterImage(datob->dat, name, IMAGE_RLE_SPRITE, 0);
      break;
   default:
      return 0;
   }
   return 1;
}

extern void UsePalette(struct t_diskobj *dobj)
{
   DATAFILE *dat;
   RGB *pal;

   dat = DiskObjectDatafileObject(dobj);
   if (dat && dat->dat) {
      pal = dat->dat;
      set_palette(pal);
   }
}

extern BITMAP *MakePaletteImage(struct t_diskobj *dobj)
{
   BITMAP *bmp = NULL;
   DATAFILE *dat;
   RGB *pal;
   int i, x, y;

   dat = DiskObjectDatafileObject(dobj);
   if (dat && dat->dat) {
      pal = dat->dat;
      bmp = create_bitmap(128, 128);
      if (bmp) {
         for (i=x=0; x<128; x+=8) {
            for (y=0; y<128; y+=8) {
               rectfill(bmp, x, y, x+7, y+7, makecol(255*pal[i].r/63, 255*pal[i].g/63, 255*pal[i].b/63));
               i++;
            }
         }
      }
   }
   return bmp;
}

extern void RenameDatfileObject(DATAFILE *datob, const char *path, DATAFILE **datl, const char *newname, int pack, const char *pwd)
{
   char *delim, *p, *dn;
   int error = 0;

   if (datob) {
      p = MkString(path);
      Win2Dos(p, p);
      dn = MkString(newname);
      Win2Dos(dn, dn);
      datedit_set_property(datob, DAT_NAME, dn);
      delim = strchr(p, '#');
      if (delim)
         *delim = 0;
      SaveDatafile2(datl, p, pwd, pack);
      Release(p);
      Release(dn);
   } else {
      error = 1;
   }
}

extern int CreateSubDataFile(const char *fullpath, DATAFILE **pdat)
{
   int error = 0;
   long size = 0;
   void **vp;
   char *delim, *tmpname, *p, *pwd = "";
   DATAFILE *datob = NULL, *nd = NULL, *ndatob, *datmem = NULL, **pdatl = NULL;

   nd = MakeNewDataObject(DAT_FILE);
   p = MkString(fullpath);
   Win2Dos(p, p);
   if (nd) {
      delim = strchr(p, '#');
      if (delim) {
         *delim++ = 0;
         if (*pdat == NULL) {
            datmem = LoadDataFile2(p, "", &pwd);
            pdat = &datmem;
         }
         tmpname = get_filename(delim);
         if (*tmpname) {
            if (tmpname > delim) {
               tmpname[-1] = 0;
               if (*pdat) {
                  datob = FindNestedDatObj(*pdat, delim);
                  if (datob && datob->type == DAT_FILE) {
                     vp = &datob->dat;
                     pdatl = (DATAFILE**)vp;
                  } else
                     error = 1;
               } else
                  pdatl = pdat;
            } else
               pdatl = pdat;
            if (*pdatl && !error) {
               *pdatl = datedit_insert(*pdatl, &ndatob, tmpname, DAT_FILE, nd, size);
               datedit_set_property(ndatob, DAT_DATE, datedit_ftime2asc(time(NULL)));
               error = !SaveDatafile2(pdat, p, pwd, -1);
            } else
               error = 1;
         }
         if (datmem)
            unload_datafile(datmem);
      }
   } else {
      error = 1;
   }
   Release(p);
   return error;
}

/* Creates a new data-file named `fn' in the directory path on the hard disk */
extern int CreateNewDataFile(const char *path, const char *fn, int pack, const char *pwd)
{
   int error = 0;
   char *newpath;
   DATAFILE *nd;

   nd = MakeNewDataObject(DAT_FILE);
   if (nd) {
      newpath = MergePathAndFile(path, fn, 0); /* path/name should be a file system entry. */
      Win2Dos(newpath, newpath);
      error = !SaveDatafile2(&nd, newpath, pwd, pack);
      unload_datafile(nd);
      Release(newpath);
   }
   return nd != NULL;
}

extern int SaveDatafile(DATAFILE *dat, const char *path, const char *password, int pack)
{
   char *p;
   int status;

   p = MkString(path);
   Win2Dos(p, p);
   status = SaveDatafile2(&dat, p, password, pack);
   Release(p);
   return status;
}

extern DATAFILE *LoadDataFile(const char *path, const char *password, int *pack, char **pwdret)
{
   char *p;
   const char *prop;
   DATAFILE *datl;
   int i;

   p = MkString(path);
   Win2Dos(p, p);
   datl = LoadDataFile2(p, password, pwdret);
   if (datl) {
      for (i = 0; datl[i].type != DAT_END; i++) {
         if (datl[i].type == DAT_INFO || datl[i].type == DAT_ID('c','g','u','i')) {
            prop = get_datafile_property(datl + i, DAT_PACK);
            if (prop && *prop) {
               sscanf(prop, "%d", pack);
               break;
            }
         }
      }
   }
   Release(p);
   return datl;
}

extern int SaveDatafileObject(const char *path, void *data, int type)
{
   DATAFILE *datl, *datob, *datm = NULL, *datf = NULL, *newd = NULL;
   int pack, ok = 1, i, c, psize;
   char *pwdret, *p, *on, *datpath;
   BITMAP *bmp;

   p = MkString(path);
   Dos2Win(p, p);
   datpath = strchr(p, '#');
   if (datpath) {
      *datpath++ = 0;
      datm = LoadDataFile(p, "", &pack, &pwdret);
      if (datm) {
         on = get_filename(datpath);
         if (on == datpath) {
            datl = datm;
         } else { /* In a nested data file */
            c = on[-1];
            on[-1] = 0;
            datf = FindNestedDatObj(datm, datpath);
            if (datf && datf->type == DAT_FILE) {
               datl = datf->dat;
            } else {
               ok = 0;
               datl = NULL;
            }
            on[-1] = c;
         }
         switch (screen->vtable->color_depth) {
         case 8:
            psize = 1;
            break;
         case 15:
         case 16:
            psize = 2;
            break;
         case 24:
            psize = 3;
            break;
         case 32:
            psize = 4;
            break;
         default:
            ok = psize = 0;
         }
         if (ok) {
            datob = find_datafile_object(datl, on);
            if (datob) {
               i = GetDatafileIndex(datl, datob);
               datl = datedit_delete(datl, i);
               if (datl) {
                  if (datf)
                     datf->dat = datl;
                  else
                     datm = datl;
               }
            }
            bmp = data;
            datl = datedit_insert(datl, &newd, on, type, data, 870);
            if (newd == NULL)
               ok = 0;
            if (datl) {
               if (datf)
                  datf->dat = datl;
               else
                  datm = datl;
            } else
               ok = 0;
         }
         if (ok) {
            datedit_set_property(newd, DAT_DATE, datedit_ftime2asc(time(NULL)));
            ok = SaveDatafile2(&datm, p, pwdret, -1);
            newd->dat = NULL;
         }
         unload_datafile(datm);
      }
   }
   Release(p);
   return ok;
}

extern DATAFILE *LoadPossibleDataFile(const char *path, const char *password, int *pack, char **pwdret)
{
   DATAFILE *datl = NULL;
   char *datdel, *p;

   p = MkString(path);
   datdel = strchr(p, '#');
   if (datdel) {
      *datdel = 0;
      datl = LoadDataFile(p, password, pack, pwdret);
   } else {
      Win2Dos(p, p);
      if (exists(p)) {
         datl = LoadDataFile(p, password, pack, pwdret);
      }
   }
   Release(p);
   return datl;
}

static void SetDataFileEntry(t_dirinfo *dirinfo, DATAFILE *dat)
{
   const char *s, *fn, *loc;
   int attrib;
   struct tm t;

   memset(&t, 0, sizeof(struct tm));
   fn = get_datafile_property(dat, DAT_NAME);
   if (fn == NULL)
      fn = "";
   loc = get_datafile_property(dat, DAT_ORIG);
   if (loc && *loc == 0)
      loc = NULL;
   s = get_datafile_property(dat, DAT_DATE);
   if (*s) {
      sscanf(s, "%d-%d-%d, %d:%d", &t.tm_mon, &t.tm_mday, &t.tm_year, &t.tm_hour, &t.tm_min);
      t.tm_mon--;
      t.tm_year -= 1900;
   }
   if (dat->type==DAT_FILE)
      attrib = FA_DIREC;
   else
      attrib = 0;
   DiskObjectCreateDat(fn, attrib, dirinfo, dat->size, dat->type, dat, &t, loc);
}

static int MakeDatafileList(DATAFILE *datl, t_dirinfo *dirinfo, char *delim)
{
   int i, ok = 1;
   DATAFILE *datob;

   if (delim) {
      datob = FindNestedDatObj(datl, delim + 1);
      if (datob && datob->type == DAT_FILE) {
         datl = datob->dat;
      } else {
         ok = 0;
      }
   }
   if (ok && datl) {
      for (i=0; datl[i].type != DAT_END; i++) {
         if (datl[i].type == DAT_FILE) {
            SetDataFileEntry(dirinfo, datl + i);
         }
      }
      for (i=0; datl[i].type != DAT_END; i++) {
         if (datl[i].type != DAT_FILE && datl[i].type != DAT_INFO) {
            SetDataFileEntry(dirinfo, datl + i);
         }
      }
   }
   return ok;
}

extern void GenerateDatafileList(t_dirinfo *dirinfo, const char *path, DATAFILE *datl, int *isdatafile)
{
   int ok;
   char *p, *delim;

   p = MkString(path);
   Win2Dos(p, p);
   delim = strchr(p, '#');
   *isdatafile = datl != NULL && delim == NULL;
//   DiskObjectCreateDat("..", FA_DIREC, dirinfo, 0, DAT_FILE, NULL, NULL, NULL);
   ok = MakeDatafileList(datl, dirinfo, delim);
   if (!ok) {
      /* Shouldn't be possible - the path has already been checked. */
      *allegro_errno = -1;
   }
   Release(p);
}
