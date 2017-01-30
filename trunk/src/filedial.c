/* Module FILEDIAL.C
   Contains the dialogue for inspecting/editing a file-object.
*/
#include <allegro.h>
#include <string.h>

#include "cgui.h"
#include "cgui/mem.h"

#include "datedit.h"
#include "filedial.h"
#include "filedial.ht"
#include "filebrow.h"
#include "filebuff.h"
#include "filereg.h"
#include "files.h"
#include "filedat.h"
#include "labldata.h"

#define MAXLEN 10000
#define NR_FMODES 12

#if (defined ALLEGRO_DOS) || (defined ALLEGRO_UNIX)
#ifndef CGUI_SCAN_DEPEND
#include <sys/stat.h>
#endif
#endif

#if ALLEGRO_SUB_VERSION > 0
#define text_mode(x)
#endif

#ifdef ALLEGRO_UNIX
 #define FILEMODE PERMISSIONS
#else
 #define FILEMODE ATTRIBUTE
   extern int strerror_r(int en,char *buf,int n);
#endif

typedef struct t_prop t_prop;
struct t_prop {
  char typestring[5];
  char *string;
};

typedef struct t_dial t_dial;
struct t_dial {
   char *orgname;          /* The original name of the object. Needs to be freed */
   t_dirinfo *dirinfo; /* Pointer to the list of objects that is currently viewed in the browser list */
   int attr_a;             /* The attribute flags (not valid for datafile objects) */
   int attr_h;
   int attr_s;
   int attr_r;
   int orgpack;            /* Original pack-state of an data-file (only valid for datafiles) */
   int pack;               /* The (eventually) updated pack-state of a datafile. */
   char *name;             /* The name of the object to edit. May be an object within a datafile */
   const char *path;       /* The path to the diretory where the edited object is. Callers data. May be within a datafile. */
   char *pwd;              /* Password, (dun mem) */
   char *pwdload;          /* Original password (at load time), dyn mem */
   int *save;              /* Pointer to callers data. Tells if the passwords are to be stored. User my modifiy this flag. */
   char *headername;       /* Name of header file (valid for datafiles only). Dyn. mem */
   char *prefix;           /* Prefix of macros for optional use when generating header files. Dyn. mem  */
   const char *typestring; /* The type of a file (file, dir, or datafile-type) */
   int tabsel;             /* Handle needed for tab-windows, can be re-used in future */
   int idpwdc;             /* id to the password-container (can be used for e.g. refresh) */
   int idfcont;            /* id to the main-part of dialogue (can be used for e.g. refresh) */
   int idlist;             /* id to the actual browser list (can be used for e.g. refresh) */
   int idplist;            /* id to the list of properties conatiner (can be used for e.g. refresh) */
   int idhname;            /* id to the edit-box for entering the header file name (can be used for e.g. refresh) */
   int type;               /* Datafile type of a data-file object */
   int imagetype;          /* The image type of `image' (bitmap/rle/compiled) */
   DATAFILE *dat;          /* Pointer to a loaded datafile (must be freed) */
   DATAFILE *datob;        /* Pointer to a the edited data-object (must NOT be freed) */
   FONT *font;             /* Pointer to a font object, found in a data-object */
   BITMAP *scaleim;        /* Pointer to a scaled version of `image'. Must bee freed */
   void *image;            /* Pointer to an image found in a data-object. Must NOT be freed */
   t_prop **props;         /* A list of properties in local format. Dyn mem. */
   int nprop;              /* ... number of such properites */
   int fmode[NR_FMODES];
   int st_mode;
   int modeid;
   int edid;
};

static const char *const*txt;

static void DelayedErrorMessage(void *data)
{
   char *info_text = data;

   Req("", info_text);
   Release(info_text);
}

static void generic_msg(const char *s, AL_CONST char *fmt, ...)
{
   va_list args;
   char *buf, *close = "|Close";

   buf = GetMem(char, 1000);
   va_start(args, fmt);
   uvszprintf(buf, 1000, fmt, args);
   va_end(args);
   if (*buf) {
      strcat(buf, s);
      strcat(buf, close);
      GenEvent(DelayedErrorMessage, buf, 0, 0);
   }
}

extern void datedit_msg(AL_CONST char *fmt, ...)
{
   va_list args;
return;
   va_start(args, fmt);
   generic_msg("(message)", fmt, args);
   va_end(args);
}

extern void datedit_startmsg(AL_CONST char *fmt, ...)
{
   va_list args;
return;
   va_start(args, fmt);
   generic_msg("(start)", fmt, args);
   va_end(args);
}

extern void datedit_endmsg(AL_CONST char *fmt, ...)
{
   va_list args;
return;
   va_start(args, fmt);
   generic_msg("(end)", fmt, args);
   va_end(args);
}

extern void datedit_error(AL_CONST char *fmt, ...)
{
   va_list args;

/*   if (errno == 1)*/
   return;
   va_start(args, fmt);
   generic_msg("(error)", fmt, args);
   va_end(args);
}

extern int datedit_ask(AL_CONST char *fmt, ...)
{
   va_list args;
   char *buf;

   buf = GetMem(char, 10000);
   va_start(args, fmt);
   vsprintf(buf, fmt, args);
   va_end(args);
   if (*buf) {
      strcat(buf, "| Yes | No ");
      GenEvent(DelayedErrorMessage, buf, 0, 0);
   }
   if (Req("", buf))
      return 'n';
   else
      return 'y';
}

/* adds the file-extnsion `ext' at end of the filename `s' (or replaces
   an existing extension). `s' myst be dunamic memory and (a possibly new)
   address to the updated string will be returned. */
static char *FixExtension(char *s, char *ext)
{
   char *e;

   s = ResizeMem(char, s, strlen(s) + strlen(ext) + 2);
   e = get_extension(s);
   if (e > s) {
     if (*(e-1) != '.')
        *e++ = '.';
   } else {
      e = s + strlen(s);
      *e++ = '.';
   }
   strcpy(e, ext);
   return s;
}

/* Frees memory allocated by the "edit-file" dialogue */
static void DestroyFileDialogue(void *data)
{
   t_dial *d = data;
   int i;

   i = GetFileListIndex(d->dirinfo, d->orgname);
   Release(d->name);
   Release(d->pwd);
   Release(d->pwdload);
   Release(d->headername);
   Release(d->prefix);
   Release(d->orgname);
   if (d->scaleim)
      destroy_bitmap(d->scaleim);
   if (d->dat)
      unload_datafile(d->dat);
   if (d->props) {
      for (i=0; i<d->nprop; i++) {
         Release(d->props[i]->string);
         Release(d->props[i]);
      }
      Release(d->props);
   }
   Release(d);
}

#ifdef ALLEGRO_UNIX
static void StModeToFlags(t_dial *d)
{
   int i=0;
   d->fmode[i++] = (d->st_mode & S_IRUSR)  != 0;
   d->fmode[i++] = (d->st_mode & S_IWUSR)  != 0;
   d->fmode[i++] = (d->st_mode & S_IXUSR)  != 0;

   d->fmode[i++] = (d->st_mode & S_IRGRP)  != 0;
   d->fmode[i++] = (d->st_mode & S_IWGRP)  != 0;
   d->fmode[i++] = (d->st_mode & S_IXGRP)  != 0;

   d->fmode[i++] = (d->st_mode & S_IROTH)  != 0;
   d->fmode[i++] = (d->st_mode & S_IWOTH)  != 0;
   d->fmode[i++] = (d->st_mode & S_IXOTH)  != 0;
}

static void FlagsToStMode(t_dial *d)
{
   int i=0;
   d->st_mode = 0;
   if (d->fmode[i++])
      d->st_mode |= S_IRUSR;
   if (d->fmode[i++])
      d->st_mode |= S_IWUSR;
   if (d->fmode[i++])
      d->st_mode |= S_IXUSR;
   if (d->fmode[i++])
      d->st_mode |= S_IRGRP;
   if (d->fmode[i++])
      d->st_mode |= S_IWGRP;
   if (d->fmode[i++])
      d->st_mode |= S_IXGRP;
   if (d->fmode[i++])
      d->st_mode |= S_IROTH;
   if (d->fmode[i++])
      d->st_mode |= S_IWOTH;
   if (d->fmode[i++])
      d->st_mode |= S_IXOTH;
}

static void DelayedEditPermissionHook(void *data)
{
   t_dial *d = data;

   StModeToFlags(d);
   Refresh(d->modeid);
}

static void EditPermissionHook(void *data)
{
   t_dial *d = data;

   GenEvent(DelayedEditPermissionHook, data, 0, d->edid);
}

static void CheckPermissionHook(void *data)
{
   t_dial *d = data;

   FlagsToStMode(d);
   Refresh(d->edid);
}

#endif

/* Confirms changes that was made in the "edit-file" dialogue */
static void ConfirmDial(void *data)
{
   t_dial *d = data;
   t_prop *prp;
   t_diskobj *dobj;
   char *newpath, *delim;
   int err, newpwd, i, removed;
   const DATAFILE_PROPERTY *prop;

   i = GetFileListIndex(d->dirinfo, d->orgname);
   if (i >= DirInfoGetNrOfDiskObjects(d->dirinfo)) {
      return;
   }
   dobj = DirInfoGetDiskObjectAt(d->dirinfo, i);
   if (DiskObjectIsDotDat(dobj))
      d->name = FixExtension(d->name, "dat");
   if (strcmp(d->name, d->orgname) != 0 && !DiskObjectIsDatafileObject(dobj))
      RenameDiskObject(d->name, dobj);
   if (DiskObjectIsDotDat(dobj) && d->dat) {
      newpwd = strcmp(d->pwd, d->pwdload);
      if (newpwd || d->pack != d->orgpack) {
         newpath = MergePathAndFile(d->path, d->name, 0); /* ".dat" is not in path here, it is in name. */
         SaveDatafile(d->dat, newpath, d->pwd, d->pack);
         Release(newpath);
      }
      if (newpwd) {
         if (*d->pwdload) {
            if (IsPasswordInList(d->pwd)) {
               if (Req("", txt[NEW_PWD_REMOVE_PREV_YES_NO]) == 0)
                  ReplacePassword(d->pwdload, "");
            } else if (*d->pwd == 0) {
               if (Req("", txt[NO_PWD_REMOVE_PREV_YES_NO]) == 0)
                  ReplacePassword(d->pwdload, "");
            } else {
               if (Req("", txt[NEW_PWD_ADD_REPLACE]) == 1)
                  ReplacePassword(d->pwdload, d->pwd);
               else
                  AddPassword(d->pwd);
            }
         } else {
            if (!IsPasswordInList(d->pwd))
               AddPassword(d->pwd);
         }
         if (*d->save)
            StorePasswordSettings();
      }
   }
   if (DiskObjectIsDatafileObject(dobj)) {
      do {
         removed = 0;
         for (prop=FirstProperty(d->datob); prop; ) {
            if (prop->type == DAT_NAME || prop->type == DAT_ORIG || prop->type == DAT_DATE)
               prop = NextProperty(prop);
            else {
               RemoveProperty(d->datob, prop);
               removed = 1;
               break;
            }
         }
      } while (removed);
      for (i = 0; i < d->nprop; i++) {
         prp = d->props[i];
         if (*prp->typestring && *prp->string)
            AddProperty(d->datob, prp->typestring, prp->string);
      }
      if (strcmp(d->name, d->orgname) != 0)
         UpdateProperty(d->datob, DAT_NAME, d->name);
      delim = strchr(d->path, '#');
      if (delim)
         *delim = 0;
      SaveDatafile(GetDataFile(d->dirinfo), d->path, d->pwd, d->pack);
   } else { /* The case of normal file system entry. */
      /* Handle the mode flags */
      newpath = MergePathAndFile(d->path, d->name, 0); /* Skip check, we already know. */
#ifdef ALLEGRO_UNIX
      err = AdjustPermissions(newpath, d->st_mode);
      if (err)
         Req("", txt[PERMISSIONS_SET_ERROR]);
#else
      {int attrib;
      attrib = DiskObjectMode(dobj);
      err = AdjustAttributes(newpath, attrib, d->attr_h, d->attr_r, d->attr_s, d->attr_a);
      if (err == -1) {
         char buf[200];
         strerror_r(errno, buf, 200);
         errno = 0;
         Request("", 0, 0, txt[ATTRIB_ERROR_CLOSE_NN], buf);
      } else if (err) {
         Req("", txt[ATTRIB_ERROR_UNKNOWN_CLOSE]);
      }
      }
#endif
      Release(newpath);
   }
   ConditionalRefresh(NULL, UPD_BROWSER_FLAG);
   i = GetFileListIndex(d->dirinfo, d->name);
   BrowseTo(d->idlist, i, 0);
   CloseWin(NULL);
}

static void CleanPwdList(void *data)
{
   int *id = data;

   ClearPasswords();
   DeActivate(*id);
}

static void AddDiskComponents(t_dial *d, t_diskobj *dobj)
{
   char s[MAXLEN];

   AddTextBox(DOWNLEFT|ALIGNRIGHT, d->typestring, 300, 0, TB_FRAMESINK);
   AddTag(LEFT, txt[FILETYPE]);
   sprintf(s, "%ld", DiskObjectSize(dobj));
   AddTextBox(DOWNLEFT|ALIGNRIGHT, s, 300, 0, TB_FRAMESINK);
   AddTag(LEFT, txt[SIZE]);
   DiskObjectTimeString(s, dobj);
   if (*s == 0)
      strcpy(s, txt[UNKNOWN]);
   AddTextBox(DOWNLEFT|ALIGNRIGHT, s, 300, 0, TB_FRAMESINK);
   AddTag(LEFT, txt[DATE]);
}

static void CreateHeader(void *data)
{
   t_dial *d = data;
   char *datname;
   int ok;

   datname = MergePathAndFile(d->path, d->name, 0); /* We're editing a ".dat" file here, path doesn't contain ".dat". */
   ok = datedit_save_header(d->dat, datname, d->headername, "CGUI's file browser using Allegro", d->prefix, 0);
   if (!ok)
      Req("", txt[FAILED_IN_HEADER_GENERATION_CLOSE]);
}

static void BrowseHeaderName(void *data)
{
   t_dial *d = data;
   const char *fn;
   fn = FileSelect("*.h", "", FS_MULTIPLE_SELECTION, txt[SELECT_HEADER_FILE], txt[SELECT]);
   if (*fn) {
      Release(d->headername);
      d->headername = MkString(fn);
      Refresh(d->idhname);
   }
}

static void UpdateAll(void *data)
{
   t_dial *d = data;
   int i;

   UpdateObjectsWithinDatafile(d->path, d->name);
   ConditionalRefresh(NULL, UPD_BROWSER_FLAG);
   i = GetFileListIndex(d->dirinfo, d->orgname);
   if (i >= DirInfoGetNrOfDiskObjects(d->dirinfo)) {
      return;
   }
   BrowseTo(d->idlist, i, 0);
   /* Re-generate the contents of the main part of the window (the data in
      the text boxes may have been updated, e.g. the date and the size) */
   EmptyContainer(d->idfcont);
   AddDiskComponents(d, DirInfoGetDiskObjectAt(d->dirinfo, i));
}

static int NeedScaling(int *maxw, int *maxh, int w, int h)
{
   *maxw = SCREEN_W - 100;
   *maxh = SCREEN_H - 200;
   if (w > *maxw || h > *maxh) {
      if (w/(double)*maxw > h/(double)*maxh)
         *maxh = (double)*maxw/w * h;
      else
         *maxw = (double)*maxh / h * w;
      return 1;
   }
   return 0;
}

static void ScaleBmp(t_dial *d)
{
   BITMAP *bmp;
   BITMAP *scaled;
   int maxw, maxh;

   bmp = d->image;
   if (bmp) {
      if (NeedScaling(&maxw, &maxh, bmp->w, bmp->h)) {
         scaled = create_bitmap(maxw, maxh);
         if (scaled) {
            stretch_blit(bmp, scaled, 0, 0, bmp->w, bmp->h, 0, 0, maxw, maxh);
            d->scaleim = scaled;
         }
      }
   }
}

static void ScaleRle(t_dial *d)
{
   BITMAP *bmp;
   RLE_SPRITE *rle;
   BITMAP *scaled;
   int maxw, maxh;

   rle = d->image;
   if (rle) {
      if (NeedScaling(&maxw, &maxh, rle->w, rle->h)) {
         bmp = create_bitmap(rle->w, rle->h);
         if (bmp) {
            draw_rle_sprite(bmp, rle, 0, 0);
            scaled = create_bitmap(maxw, maxh);
            if (scaled) {
               stretch_blit(bmp, scaled, 0, 0, bmp->w, bmp->h, 0, 0, maxw, maxh);
               d->scaleim = scaled;
            }
            destroy_bitmap(bmp);
         }
      }
   }
}

static void ModeContainerContents(t_dial *d)
{
   int id;

#ifdef ALLEGRO_UNIX
   int i, j, k;
   d->modeid = StartContainer(DOWNLEFT, ADAPTIVE, "", 0);
   d->edid = AddEditBox(TOPLEFT, 100, txt[MODE_MASK], FOCT4, 4, &d->st_mode);
   AddHandler(d->edid, EditPermissionHook, d);
   for (i=k=0; i<3; i++) {
      AddTag(DOWNLEFT, txt[USER_CAT+i]);
      for (j=0; j<3; j++, k++) {
         id = AddCheck(DOWNLEFT, txt[ACCESS_TYPE+j], &d->fmode[k]);
         AddHandler(id, CheckPermissionHook, d);
      }
   }
   EndContainer();
   JoinTabChain(d->modeid);
#else
   id = StartContainer(DOWNLEFT, ADAPTIVE, "", 0);
   AddCheck(DOWNLEFT, txt[READ_ONLY], &d->attr_r);
   AddCheck(DOWNLEFT, txt[HIDDEN], &d->attr_h);
   AddCheck(DOWNLEFT, txt[SYSTEM], &d->attr_s);
   AddCheck(DOWNLEFT, txt[ARCHIVE], &d->attr_a);
   EndContainer();
#ifdef ALLEGRO_DOS
   JoinTabChain(id);
#else
   DeActivate(id);
#endif
#endif
}

static void DataFileTab(void *data, int id)
{
   t_dial *d = data;
   int npwd;
   (void)id;

   StartContainer(TOPLEFT, ADAPTIVE, txt[PACKED], CT_BORDER);
   MkRadioContainer(TOPLEFT, &d->pack, R_HORIZONTAL|EQUALWIDTH);
   AddRadioButton(txt[NONE]);
   AddRadioButton(txt[INDIVIDUAL]);
   AddRadioButton(txt[GLOBAL]);
   EndRadioContainer();
   EndContainer();

   StartContainer(DOWNLEFT, ADAPTIVE, txt[PASSWORD], CT_BORDER);
   AddEditBox(TOPLEFT, 100, "", FPTRSTR, 0, &d->pwd);
   AddCheck(DOWNLEFT, txt[SAVE_PASSWORD], d->save);
   GetPasswordList(&npwd);
   if (npwd)
      d->idpwdc = AddButton(RIGHT, txt[CLEAN_PREV_PWD], CleanPwdList, &d->idpwdc);
   EndContainer();

   StartContainer(DOWNLEFT, ADAPTIVE, txt[HEADER_FILES], CT_BORDER);
   d->idhname = AddEditBox(TOPLEFT, 300, txt[HEADER_FILE_NAME], FPTRSTR, 0, &d->headername);
   AddButton(RIGHT, txt[BROWSE], BrowseHeaderName, d);
   AddEditBox(DOWNLEFT, 100, txt[MACRO_PREFIX], FPTRSTR, 0, &d->prefix);
   AddButton(DOWNLEFT, txt[DO_GENERATE], CreateHeader, d);
   EndContainer();

   AddButton(DOWNLEFT, txt[UPDATE_OBJECTS], UpdateAll, d);
}

static void NewProperty(void *data)
{
   t_dial *d = data;
   t_prop *prp;

   d->props = ResizeMem(t_prop*, d->props, ++d->nprop);
   d->props[d->nprop - 1] = prp = GetMem(t_prop, 1);
   *prp->typestring = 0;
   prp->string = MkString("");
   SelectContainer(d->idplist);
   AddEditBox(DOWNLEFT, 50, "", FSTRING, 4, prp->typestring);
   AddEditBox(RIGHT, 250, "", FPTRSTR, 0, &prp->string);
   DisplayWin();
}

static void DataPropertiesTab(void *data, int id)
{
   t_dial *d = data;
   t_prop *prp;
   int i;
   (void)id;

   d->idplist = StartContainer(DOWNLEFT, ADAPTIVE, "", 0);
   for (i = 0; i < d->nprop; i++) {
      prp = d->props[i];
      AddEditBox(DOWNLEFT, 50, "", FSTRING, 4, &prp->typestring);
      AddEditBox(RIGHT, 250, "", FPTRSTR, 0, &prp->string);
   }
   EndContainer();
   AddButton(DOWNLEFT, txt[NEW_PROPERTY], NewProperty, d);
}

static void ViewFullSizeImage(void *data)
{
   t_dial *d = data;
   int id;

   id = MkDialogue(ADAPTIVE, NULL, 0);
   RegisterImage(d->image, "fullsized", d->imagetype, id);
   AddTag(TOPLEFT, "#fullsized");
   DisplayWin();
   SetHotKey(id, CloseWin, NULL, KEY_ESC, 27);
}

static void MakeEmbeddedImage(t_dial *d, const char *name)
{
   if (d->scaleim) {
      AddTag(DOWNLEFT, txt[SCALED_IMAGE]);
      AddTag(RIGHT, name);
      AddButton(RIGHT, txt[VIEW_IMAGE], ViewFullSizeImage, d);
   } else
      AddTag(DOWNLEFT, name);
}

static void DataObjectTab(void *data, int id)
{
   t_dial *d = data;
   const char *p;
   FONT *f;
   int i;
   (void)id;

   i = GetFileListIndex(d->dirinfo, d->orgname);
   if (i >= DirInfoGetNrOfDiskObjects(d->dirinfo))
      return;
   p = DiskObjectDatSource(DirInfoGetDiskObjectAt(d->dirinfo, i));
   if (p == NULL)
      p = txt[UNKNOWN];
   AddTextBox(DOWNLEFT|ALIGNRIGHT, p, 300, 0, TB_FRAMESINK);
   AddTag(LEFT, txt[DAT_SOURCE]);
   switch (d->type) {
   case DAT_BITMAP:
   case DAT_C_SPRITE:
   case DAT_RLE_SPRITE:
      MakeEmbeddedImage(d, "#filedial");
      break;
   case DAT_FONT:
      f = GetCguiFont();
      SetCguiFont(d->font);
      AddTag(DOWNLEFT, "ABCDEFGHIJKLMNOPQRSTUVXYZ");
      AddTag(DOWNLEFT, "abcdefghijklmnopqrstuvxyz");
      AddTag(DOWNLEFT, "0123456789");
      SetCguiFont(f);
      break;
   }

}

static void ImageTab(void *data, int id)
{
   t_dial *d = data;
   (void)id;

   MakeEmbeddedImage(d, "#filedial");
}

static void ModeTab(void *data, int id)
{
   t_dial *d = data;
   (void)id;

   ModeContainerContents(d);
}

/* The "edit-file" dialogue. */
extern void EditPropertiesDialog(t_dirinfo *dirinfo, t_diskobj *dobj, int *save, int idlist)
{
   t_dial *d;
   t_prop *prp;
   int attrib, indat, nr, id;
   const char *filetypestring, *ext, *imname = NULL;
   char *mem = NULL, stype[10], *name, *pwdret, *delim;
   DATAFILE *datob = NULL, *datl;
   const DATAFILE_PROPERTY *prop;
   void *viewdata = NULL;

   txt = LoadCompiledTexts(cgui_labels+CGUI_CGUILABL, "filedial", &nr);
   if (nr < SIZE_filedial)
      return;
   d = GetMem0(t_dial, 1);
   d->orgname = MkString(DiskObjectName(dobj));
   indat = DiskObjectIsDatafileObject(dobj);
   attrib = DiskObjectMode(dobj);
#ifdef ALLEGRO_UNIX
   {char *fpath;
   struct stat st;
   if (!indat) { /* Edinting a normal file system entry, optionally a ".dat"-file. */
      fpath = MergePathAndFile(DirInfoGetPath(dirinfo), DiskObjectName(dobj), 0); /* Skip check, no ".dat" in path. */
      stat(fpath, &st);
      Release(fpath);
      d->st_mode = st.st_mode & (S_IRWXU|S_IRWXG|S_IRWXO);
      StModeToFlags(d);
   }
   }
#else
   d->attr_r = (attrib & FA_RDONLY) != 0;
   d->attr_h = (attrib & FA_HIDDEN) != 0;
   d->attr_a = (attrib & FA_ARCH) != 0;
   d->attr_s = (attrib & FA_SYSTEM) != 0;
#endif
   d->path = DirInfoGetPath(dirinfo);
   d->save = save;
   d->idlist = idlist;
   d->dirinfo = dirinfo;
   d->name = MkString(DiskObjectName(dobj));
   d->headername = MkString("");
   d->prefix = MkString("");

   if (indat) {
      /* Get the data file object */
      datl = GetDataFile(dirinfo);
      datob = find_datafile_object(datl, d->name);
      if (datob == NULL) {
         name = MergePathAndFile(d->path, d->name, indat);
         delim = strchr(name, '#');
         if (delim)
            datob = FindNestedDatObj(datl, delim + 1);
         Release(name);
      }
      if (datob) {
         d->datob = datob;
         viewdata = datob->dat;
         /* Extract properties from data file object */
         for (prop=FirstProperty(datob); prop; prop=NextProperty(prop)) {
            if (prop->type == DAT_NAME || prop->type == DAT_ORIG || prop->type == DAT_DATE)
               ;
            else {
               d->props = ResizeMem(t_prop*, d->props, ++d->nprop);
               d->props[d->nprop - 1] = prp = GetMem(t_prop, 1);
               PropertyTypestring(prp->typestring, prop);
               prp->string = MkString(PropertyString(prop));
            }
         }
      }
      d->type = DiskObjectDatafileObjectType(dobj);
      switch (d->type) {
      case 0:
         filetypestring = txt[UNKNOWN];
         break;
      case DAT_DATA:
         filetypestring = txt[ALLEGRO_BINARY];
         break;
      case DAT_MIDI:
         filetypestring = txt[ALLEGRO_MIDI];
         break;
      case DAT_PATCH:
         filetypestring = txt[ALLEGRO_MIDI_PAT];
         break;
      case DAT_FLI:
         filetypestring = txt[ALLEGRO_ANIMATION];
         break;
      case DAT_RLE_SPRITE:
         filetypestring = txt[ALLEGRO_RLE];
         d->imagetype = IMAGE_RLE_SPRITE;
         d->image = viewdata;
         if (viewdata)
            ScaleRle(d);
         break;
      case DAT_C_SPRITE:
         filetypestring = txt[ALLEGRO_CMP];
         d->imagetype = IMAGE_BMP;
         d->image = viewdata;
         if (viewdata)
            ScaleBmp(d);
         break;
      case DAT_XC_SPRITE:
         filetypestring = txt[ALLEGRO_CMPX];
         break;
      case DAT_PALETTE:
         filetypestring = txt[ALLEGRO_PAL];
         break;
      case DAT_FILE:
         filetypestring = txt[ALLEGRO_SUBDIR];
         break;
      case DAT_SAMPLE:
         filetypestring = txt[ALLEGRO_SOUND];
         break;
      case DAT_BITMAP:
         filetypestring = txt[ALLEGRO_BITMAP];
         d->imagetype = IMAGE_TRANS_BMP;
         d->image = viewdata;
         if (viewdata)
            ScaleBmp(d);
         break;
      case DAT_FONT:
         filetypestring = txt[ALLEGRO_FONT];
         d->font = viewdata;
         break;
      default:
         filetypestring = txt[ALLEGRO_USER];
         break;
      }
      TypeToString(stype, d->type);
      mem = msprintf("(%s) %s", stype, filetypestring);
      filetypestring = mem;
   } else if (attrib & FA_DIREC) {
      filetypestring = txt[DIRECTORY_TEXT];
   } else if (DiskObjectIsDotDat(dobj)) {
      filetypestring = txt[ALLEGRO_DAT];
      TypeToString(stype, DAT_FILE);
      mem = msprintf("(%s) %s", stype, filetypestring);
      filetypestring = mem;
      name = MergePathAndFile(DirInfoGetPath(dirinfo), d->name, 0); /* ".dat" is not in path, it is in name. */
      d->dat = LoadDataFile(name, "", &d->pack, &pwdret);
      if (pwdret == NULL)
         pwdret = "";
      d->pwdload = MkString(pwdret);
      d->pwd = MkString(pwdret);
      d->orgpack = d->pack;
      Release(name);
   } else {
      ext = DiskObjectExt(dobj);
      filetypestring = GetRegLabel(ext, 0);
      if (filetypestring == NULL)
         filetypestring = txt[FILE_TEXT];
      imname = DiskObjectGetImage(dobj);
      if (imname) {
         d->image = (void*)GetRegisteredImage(imname+2, &d->imagetype, idlist);
         if (d->image) {
            if (d->imagetype == IMAGE_RLE_SPRITE)
               ScaleRle(d);
            else
               ScaleBmp(d);
         }
      }
   }
   d->typestring = filetypestring;

   id = MkDialogue(ADAPTIVE, txt[EDIT_DISK_OBJECT], 0);
   HookExit(id, DestroyFileDialogue, d);
   if (d->scaleim)
      RegisterImage(d->scaleim, "filedial", IMAGE_BMP, id);
   else if (d->image)
      RegisterImage(d->image, "filedial", d->imagetype, id);

   AddEditBox(TOPLEFT|ALIGNRIGHT, 300, txt[NAME], FPTRSTR, 0, &d->name);
   d->idfcont = StartContainer(DOWNLEFT|ALIGNRIGHT, ADAPTIVE, "", 0);
   AddDiskComponents(d, dobj);
   EndContainer();

   if (DiskObjectIsDotDat(dobj)) {
      id = CreateTabWindow(DOWNLEFT|ALIGNRIGHT, ADAPTIVE, &d->tabsel);
      AddTab(id, DataFileTab, d, txt[DATAFILE_PROPERTIES]);
      AddTab(id, ModeTab, d, txt[FILEMODE]);
   } else if (indat) {
      id = CreateTabWindow(DOWNLEFT, ADAPTIVE, &d->tabsel);
      AddTab(id, DataObjectTab, d, txt[DATAFILE_OBJECT]);
      AddTab(id, DataPropertiesTab, d, txt[PROPERTY_LIST]);
   } else if (imname) {
      id = CreateTabWindow(DOWNLEFT|ALIGNRIGHT, ADAPTIVE, &d->tabsel);
      AddTab(id, ImageTab, d, txt[IMAGE]);
      AddTab(id, ModeTab, d, txt[FILEMODE]);
   } else {
      StartContainer(DOWNLEFT|ALIGNRIGHT, ADAPTIVE, txt[FILEMODE], CT_BORDER);
      ModeContainerContents(d);
      EndContainer();
   }
   AddButton(DOWNLEFT, txt[LABEL_OK], ConfirmDial, d);
   AddButton(RIGHT, txt[LABEL_CANCEL], CloseWin, NULL);
   DisplayWin();

   if (mem)
      Release(mem);
}

typedef struct t_add
{
   t_dirinfo *dirinfo;
   char *name;
   int mkdatafile;
   int idp;
   int idlist;
   int idpwd;
   int idpwds;
   int idpwdc;
   int idcont;
   int pack;
   int *save;
   char *pwd;
} t_add;


static void CancelAdd(void *data)
{
   t_add *a = data;

   Release(a->name);
   if (a->pwd)
      Release(a->pwd);
   Release(a);
   CloseWin(NULL);
}

static void ConfirmAdd(void *data)
{
   t_add *a = data;
   t_dirinfo *dirinfo;
   int error;

   dirinfo = a->dirinfo;
   if (a->mkdatafile)
      a->name = FixExtension(a->name, "dat");

   error = MakeDirectory(dirinfo, a->name, a->mkdatafile, a->pack, a->pwd);
   if (error) {
      Req("", txt[SPECIFIED_DIRECTORY_NOT_CREATED_CLOSE]);
      return;
   }
   if (*a->pwd && !IsPasswordInList(a->pwd)) {
      AddPassword(a->pwd);
      if (*a->save)
         StorePasswordSettings();
   }
   ConditionalRefresh(NULL, UPD_BROWSER_FLAG);
   BrowseTo(a->idlist, GetFileListIndex(dirinfo, a->name), 0);
   CancelAdd(a);
}

/* The "create-directory" dialogue (directory here might be a datafile or a datafile object of type DAT_FILE). The dialog simply let the user
   enter the name of the new directory. In case of a datafile (.dat-file) the attributs for datafiles will be available too (i.e. packing
   and password). */
static void DirectoryDialog(int idlist, t_dirinfo *dirinfo, int *save, t_add *a, int label)
{
   int nr, npwd;

   txt = LoadCompiledTexts(cgui_labels+CGUI_CGUILABL, "filedial", &nr);
   if (nr < SIZE_filedial)
      return;
   a->idlist = idlist;
   a->dirinfo = dirinfo;
   a->save = save;
   a->name = MkString("");
   a->pwd = MkString("");
   MkDialogue(ADAPTIVE, txt[label], 0);
   AddEditBox(TOPLEFT, 300, txt[NAME], FPTRSTR, 0, &a->name);
   if (a->mkdatafile) {
      a->idcont = StartContainer(DOWNLEFT, ADAPTIVE, txt[DATAFILE_PROPERTIES], CT_BORDER);
      AddTag(DOWNLEFT, txt[PACKED]);
      a->idp = MkRadioContainer(RIGHT, &a->pack, R_HORIZONTAL);
      AddRadioButton(txt[NONE]);
      AddRadioButton(txt[INDIVIDUAL]);
      AddRadioButton(txt[GLOBAL]);
      EndRadioContainer();
      a->idpwd = AddEditBox(DOWNLEFT, 100, txt[PASSWORD], FPTRSTR, 0, &a->pwd);
      a->idpwds = AddCheck(DOWNLEFT, txt[SAVE_PASSWORD], save);
      GetPasswordList(&npwd);
      if (npwd)
         a->idpwdc = AddButton(RIGHT, txt[CLEAN_PREV_PWD], CleanPwdList, &a->idpwdc);
      EndContainer();
   }
   AddButton(DOWNLEFT, txt[LABEL_OK], ConfirmAdd, a);
   AddButton(RIGHT, txt[LABEL_CANCEL], CancelAdd, a);
   DisplayWin();
}

extern void CreateDirDialog(int idlist, t_dirinfo *dirinfo, int *save)
{
   t_add *a;

   a = GetMem0(t_add, 1);
   DirectoryDialog(idlist, dirinfo, save, a, CREATE_DIRECTORY);
}

extern void CreateDatafileDialog(int idlist, t_dirinfo *dirinfo, int *save)
{
   t_add *a;
   int label;

   a = GetMem0(t_add, 1);
   if (IsDataFileList(dirinfo)) {
      label = CREATE_SUBDATAFILE;
   } else {
      a->mkdatafile = 1;
      label = CREATE_DATAFILE;
   }
   DirectoryDialog(idlist, dirinfo, save, a, label);
}

struct t_preview {
   int winid;
   int contid;
   int reg;
   int text;
   int brwinid;
   BITMAP *palbmp;
   t_diskobj *dobj;
   t_browser *br;
};

static void DestroyPreviewWin(void *data)
{
   t_preview *pw = data;

   if (pw->palbmp)
      destroy_bitmap(pw->palbmp);
   Release(pw);
}

static void UsePaletteWrapper(void *data)
{
   t_preview *pw = data;

   UsePalette(pw->dobj);
   ReloadFileList(pw->br);
}

static char *item_pre_x = "x-preview";
static char *item_pre_y = "y-preview";
extern void ClosePreviewWindow(t_preview *pw, const char *section)
{
   int x0, y0, x, y, w, h;

   GetWinInfo(pw->brwinid, &x0, &y0, &w, &h);
   GetWinInfo(pw->winid, &x, &y, &w, &h);
   set_config_int(section, item_pre_x, x - x0);
   set_config_int(section, item_pre_y, y - y0);
   flush_config_file();
   Remove(pw->winid);
}

extern t_preview *OpenPreviewWindow(const char *section, int brwinid, t_browser *br)
{
   int nr, x, y, w, h;
   t_preview *pw;

   pw = GetMem0(t_preview, 1);
   txt = LoadCompiledTexts(cgui_labels+CGUI_CGUILABL, "filedial", &nr);
   if (nr < SIZE_filedial)
      return NULL;
   GetWinInfo(CurrentWindow(), &x, &y, &w, &h);
   pw->winid = MkDialogue(ADAPTIVE, NULL, W_FLOATING);
   pw->contid = StartContainer(TOPLEFT, ADAPTIVE, "", 0);
   pw->brwinid = brwinid;
   pw->br = br;
   EndContainer();
   x += get_config_int(section, item_pre_x, 0);
   y += get_config_int(section, item_pre_y, 0);
   if (y < 0)
      y = 0;
   if (x < -50)
      x = -50;
   SetWindowPosition(x, y);
   DisplayWin();
   HookExit(pw->winid, DestroyPreviewWin, pw);
   return pw;
}

extern void UpdatePreviewWindow(t_preview *pw, t_diskobj *dobj)
{
   int indat, type = 0;
   const char *imname = "", *ext;
   char *name;
   const char *font_preview_text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_ _abcdefghijklmnopqrstuvwxyz_ _1234567890";
   const FONT *vf;
   FONT *f;
   DATAFILE *dat;
   BITMAP *bmp;
   t_viewfun vfun;

   indat = DiskObjectIsDatafileObject(dobj);
   if (indat) {
      type = DiskObjectDatafileObjectType(dobj);
      switch (type) {
      case DAT_DATA: /* Try text */
         break;
      case DAT_MIDI:
         break;
      case DAT_PATCH:
         break;
      case DAT_FLI: /* Animation */
         break;
      case DAT_PALETTE:
         break;
      case DAT_FILE:
         break;
      case DAT_SAMPLE:
         break;
      case DAT_RLE_SPRITE:
      case DAT_C_SPRITE:
      case DAT_XC_SPRITE:
      case DAT_BITMAP:
         imname = DiskObjectGetImage(dobj);
         break;
      case DAT_FONT: /* Show some text */
         break;
      default:
         break;
      }
   } else {
      imname = DiskObjectGetImage(dobj);
   }
   if (*imname) {
      SelectContainer(pw->contid);
      EmptyContainer(pw->contid);
      AddTag(TOPLEFT, imname);
      pw->reg = 1;
      pw->text = 0;
      Refresh(pw->winid);
   } else if (type == DAT_FONT) {
      SelectContainer(pw->contid);
      EmptyContainer(pw->contid);
      vf = DiskObjectGetFont(dobj);
      if (vf) {
         f = GetCguiFont();
         SetCguiFont((FONT*)vf);
         AddTextBox(TOPLEFT, font_preview_text, 200, 0, TB_LINEFEED_);
         SetCguiFont(f);
      }
      Refresh(pw->winid);
      pw->text = 0;
   } else if (type == DAT_PALETTE) {
      SelectContainer(pw->contid);
      EmptyContainer(pw->contid);
      bmp = MakePaletteImage(dobj);
      if (bmp) {
         RegisterImage(bmp, "palette", IMAGE_BMP, pw->contid);
         AddTag(TOPLEFT, "#palette");
         pw->dobj = dobj;
         AddButton(DOWNLEFT, txt[USE_PALETTE], UsePaletteWrapper, pw);
         if (pw->palbmp)
            destroy_bitmap(pw->palbmp);
         pw->palbmp = bmp;
      }
      Refresh(pw->winid);
      pw->text = 0;
   } else {
      if (indat)
         ext = "";
      else
         ext = DiskObjectExt(dobj);
      vfun = GetRegViewer(ext, type);
      if (vfun) {
         SelectContainer(pw->contid);
         EmptyContainer(pw->contid);
         if (indat) {
            dat = DiskObjectDatafileObject(dobj);
            if (dat) {
               vfun(GetRegData(ext, type), dat);
               pw->text = 0;
            }
         } else {
            name = DiskObjectGetFullPathDos(dobj);
            vfun(GetRegData(ext, type), name);
            pw->text = 0;
            Release(name);
         }
         Refresh(pw->winid);
      } else if (pw->text == 0) {
         SelectContainer(pw->contid);
         EmptyContainer(pw->contid);
         AddTag(TOPLEFT, txt[UNIMPLEMENTED_VIEWER]);
         pw->text = 1;
         Refresh(pw->winid);
      }
   }
}
