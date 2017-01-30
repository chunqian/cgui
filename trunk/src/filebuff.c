/* Module FILEBUFF.C
   This module contains functions for the copy-files buffer. Files may be
   insert into the copy buffer when needed and later all the contents of the
   buffer can be commanded to be pasted.
   When insertion is done the inserted object is indicated as either of
   "copy object" or move object. The buffer may contain a mix of these. The
   objects may be either of simple files, directories or datafile objects.
   Object marked as "move" may be removed from thier origin location when the
   paste command is preformed, as well as from the buffer.
   The current content of the buffer may be displayed in a new floating
   window.
*/
#include <allegro.h>
#include <stdio.h>
#include <string.h>
#include "datedit.h"

#include "cgui.h"
#include "cgui/mem.h"
#include "cgui/dw_conv.h"

#include "files.h"
#include "filedat.h"
#include "filebuff.h"
#include "filebuff.ht"
#include "fileimpq.h"
#include "labldata.h"

#ifndef ALLEGRO_DOS
#define Dos2Win(a, b)
#define Win2Dos(a, b)
#endif

typedef struct t_copyfile t_copyfile;

struct t_copy_buffer_window {
   int idbuf;
   int idbufwin;
   int *isopen;
};

struct t_copyfile {
   char *path;                  /* Pointer to the path of the copied file
                                   dynamic memory, can be freed with the
                                   t_copyfile object */
   char *file;                  /* Pointer to file-name. Refers to the latter
                                   part of the above */
   int size;
   int isdir;
   int is_in_dat;
   int is_dot_dat;
   int cut;
   int copied;
};

struct t_copy_buffer {
   int n;
   t_copyfile *cpf;
};

static t_copy_buffer *global_copy_buffer = NULL;

static const char *const*GetBufferTexts(void)
{
   static const char *dummies[SIZE_filebuff];
   static const char *const*bufftexts = NULL;
   int nr, i;

   bufftexts = LoadCompiledTexts(cgui_labels+CGUI_CGUILABL, "filebuff", &nr);
   if (nr < SIZE_filebuff) {
      for (i = 0; i < SIZE_filebuff; i++)
         dummies[i] = "??";
      bufftexts = dummies;
   }
   return bufftexts;
}

/* Destroys the copy buffer */
static void DestroyGlobalCopyBuffer(void *data)
{
   (void)data;
   if (global_copy_buffer) {
      CopyBufferDestroy(global_copy_buffer);
      global_copy_buffer = NULL;
   }
}

/* Filters all cut-files from the copy-buffer */
static void FilterCutFiles(t_copy_buffer *copy_buffer)
{
   int i, j;
   for (i = j = 0; i < copy_buffer->n; i++) {
      if (copy_buffer->cpf[i].cut) {
         Release(copy_buffer->cpf[i].path);
      } else
         copy_buffer->cpf[j++] = copy_buffer->cpf[i];
   }
   if (j == 0) {
      Release(copy_buffer->cpf);
      copy_buffer->cpf = NULL;
   }
   copy_buffer->n = j;
}

/* Empties the contents of the copy buffer */
extern void CopyBufferClean(t_copy_buffer *copy_buffer)
{
   t_copyfile *cp;
   int i;

   if (copy_buffer->n) {
      for (i = 0, cp = copy_buffer->cpf; i < copy_buffer->n; i++, cp++)
         Release(cp->path);
      Release(copy_buffer->cpf);
      copy_buffer->cpf = NULL;
      copy_buffer->n = 0;
   }
}

/* Rmeoves all duplicate names from the buffer */
extern void CopyBufferFilterDuplicates(t_copy_buffer *copy_buffer)
{
   int i, j, k;

   for (i = 0; i < copy_buffer->n; i++) {
      for (j = i + 1; j < copy_buffer->n; j++) {
         if (strcmp(copy_buffer->cpf[i].file, copy_buffer->cpf[j].file) == 0) {
            Release(copy_buffer->cpf[j].path);
            for (k = j + 1; k < copy_buffer->n; k++) {
               copy_buffer->cpf[k - 1] = copy_buffer->cpf[k];
            }
            copy_buffer->n--;
         }
      }
   }
}

static void SetCopyFile(t_copyfile *cp, const char *path, const char *file, int size, int isdir, int is_in_dat, int is_dot_dat, int cut)
{
   cp->path = MergePathAndFile(path, file, is_in_dat|is_dot_dat);
   cp->file = get_filename(cp->path);
   cp->size = size;
   cp->isdir = isdir;
   cp->cut = cut;
   cp->is_in_dat = is_in_dat;
   cp->is_dot_dat = is_dot_dat;
   cp->copied = 0;
}

/* Insert a new file into the copy buffer. The up-dir is not allowed. */
extern void CopyBufferInsertEntry(t_copy_buffer *copy_buffer, struct t_diskobj *dobj, int cut)
{
   t_copyfile *cp;
   const char *path;

   if (strcmp(DiskObjectName(dobj), "..") != 0) {
      copy_buffer->n++;
      copy_buffer->cpf = ResizeMem(t_copyfile, copy_buffer->cpf, copy_buffer->n);
      cp = copy_buffer->cpf + copy_buffer->n - 1;
      path = DiskObjectGetLocation(dobj);
      if (path) {
         SetCopyFile(cp, path, DiskObjectName(dobj), DiskObjectSize(dobj), IsDirectoryObject(dobj), DiskObjectIsInDatafile(dobj),
                  DiskObjectIsDotDat(dobj), cut);
      }
   }
}

/* Returns true if buffer is empty. */
extern int CopyBufferIsEmpty(t_copy_buffer *copy_buffer)
{
   return copy_buffer->n == 0;
}

/* Checks if some file(s) in the buffer matches "path" */
extern int CopyBufferEntryExists(t_copy_buffer *copy_buffer, const char *path)
{
   t_copyfile *cf;
   int i;

   for (i = 0, cf = copy_buffer->cpf; i < copy_buffer->n; i++, cf++)
      if (strcmp(path, cf->path) == 0)
         return 1;
   return 0;
}

static void PasteSingleFileWrapper(const char *srcdir, const char *filename, t_copyfile *cf, const char *destdir, int check)
{
   int retcode;
   const char *const*txt;

   txt = GetBufferTexts();
   retcode = PasteASingleFile(srcdir, filename, cf->size, destdir, check);
   switch (retcode) {
   case PASTE_OK:
      cf->copied = 1;
      break;
   case PASTE_READ_ERROR:
      Req("", txt[FILE_READ_ERROR_CLOSE]);
      break;
   case PASTE_WRITE_ERROR:
      Req("", txt[FILE_STORE_ERROR_CLOSE]);
      break;
   case PASTE_CHECK_ERROR:
      if (Req("", txt[SPECIFIED_FILE_ALREADY_EXISTS_OK_CANCEL]) == 0) {
         PasteSingleFileWrapper(srcdir, filename, cf, destdir, 0);
      }
      break;
   }
}

static int ReplaceExistingRequest(const char *fn)
{
   const char *const*txt;
   txt = GetBufferTexts();
   return Request("", 0, 0, txt[SPECIFIED_FILE_ALREADY_EXISTS_OK_CANCEL], fn) == 0;

}

#ifndef ALLEGRO_UNIX
int strerror_r(int en, char *buf, int n)
{
   strncpy(buf, sys_errlist[en], n);
   return 0;
}
#endif

static void PasteSingleObject(t_dirinfo *destdirinfo, t_copyfile *cf, const char *destpath, int dest_is_dot_dat,
                              int dest_is_datafile_object, const char *const*txt)
{
   char *fn, *sd, *spath, *datmarker;
   int error, status, txtind=0, ok;

   sd = MkString(cf->path);
   fn = get_filename(sd);
   datmarker = strchr(fn, '#');
   if (datmarker) {
      /* cf->path="/foo/bar.dat#my_dir/my_image"  =>  sd="/foo/bar.dat" and fn="my_dir/my_image" */
      *datmarker = 0;
      fn = datmarker + 1;
   } else if (fn > sd) {
      /* cf->path="/foo/bar.dat"  =>  sd="/foo" and fn="bar.dat"
         or cf->path="/foo/bar"   =>  sd="/foo" and fn="bar" where bar is a directory. */
      fn[-1] = 0;
   }
   if (cf->is_in_dat) { /* source is a datafile object */
      DATAFILE *nil=NULL;
      error = PasteADatafileObject(sd, fn, destpath, dest_is_dot_dat|dest_is_datafile_object, &nil, ReplaceExistingRequest);
      cf->copied = error == 0;
   } else if (cf->is_dot_dat) { /* source is a ".dat"-file */
      if (dest_is_dot_dat | dest_is_datafile_object) {
         error = ImportSingleFileIntoDataFile(sd, fn, destpath);
         cf->copied = error == 0;
      } else {
         PasteSingleFileWrapper(sd, fn, cf, destpath, 1);
         error = cf->copied == 0;
      }
   } else if (cf->isdir) { /* source is a directory tree */
      error = PasteADirectory(destdirinfo, sd, fn, destpath, dest_is_dot_dat|dest_is_datafile_object);
      cf->copied = error == 0;
   } else { /* source is a single disk-file */
      if (dest_is_dot_dat | dest_is_datafile_object) { /* dest is a datafile/datafile object */
         error = ImportSingleFileIntoDataFile(sd, fn, destpath);
         cf->copied = error == 0;
      } else {
         PasteSingleFileWrapper(sd, fn, cf, destpath, 1);
         error = cf->copied == 0;
      }
   }
   /* If this object was a cut-and-paste and the copy part was successful, then we shall also remove if from the source. */
   if (cf->cut && cf->copied) {
      status = RemoveDiskObjectByName(sd, fn, cf->is_in_dat);
      switch (status) {
      case 0:
         txtind = REMOVE_DIRECTORY_NN_FAILED_NN_CLOSE;
         ok = 0;
         break;
      case -1:
         txtind = REMOVE_FILE_NN_FAILED_NN_CLOSE;
         ok = 0;
         break;
      case -2:
         txtind = REMOVE_DATAFILE_NN_FAILED_NN_CLOSE;
         ok = 0;
         break;
      default:
         ok = 1;
         break;
      }
      if (!ok) {
         char buf[200];
         strerror_r(errno, buf, 200);
         errno = 0;
         spath = MergePathAndFile(sd, fn, cf->is_dot_dat|cf->is_in_dat);
         Dos2Win(spath, spath);
         Request("", 0, 0, txt[txtind], spath, buf);
         Release(spath);
      }
   }
   Release(sd);
}

/* Perform "paste" on the entire buffer to `dest_dirinfo', i.e. moves all files marked "cut" and copies the rest. The moved files will also
   be removed from the copybuffer. */
extern void CopyBufferPaste(t_copy_buffer *copy_buffer, t_dirinfo *dest_dirinfo)
{
   int dest_is_dot_dat;   /* Destination is an allegro ".dat" file (a file in the file-system). */
   int dest_is_datafile_object; /* Destination is an data file object. */
   int i;
   char *destpath;
   const char *const*txt;

   destpath = MkString(DirInfoGetPath(dest_dirinfo));
   dest_is_dot_dat = IsFileDotDat(dest_dirinfo); /* dest is the top datafile (a disk file) */
   dest_is_datafile_object = IsDataFileList(dest_dirinfo);
   if (destpath) {
      txt = GetBufferTexts();
      for (i = 0; i < copy_buffer->n; i++) {
         PasteSingleObject(dest_dirinfo, copy_buffer->cpf+i, destpath, dest_is_dot_dat, dest_is_datafile_object, txt);
      }
      FilterCutFiles(copy_buffer);
      Release(destpath);
   }
}

/* Create a new copy buffer and returns apointer to it. */
extern t_copy_buffer *CopyBufferCreate(void)
{
   t_copy_buffer *copy_buffer;
   copy_buffer = GetMem0(t_copy_buffer, 1);
   return copy_buffer;
}

/* Initilazes the global copy-buffer and returns a pointer to it. */
extern t_copy_buffer *CopyBufferGetGlobal(void)
{
   if (global_copy_buffer == NULL) {
      global_copy_buffer = CopyBufferCreate();
      HookCguiDeInit(DestroyGlobalCopyBuffer, global_copy_buffer);
   }
   return global_copy_buffer;
}

extern void CopyBufferDestroy(t_copy_buffer *copy_buffer)
{
   CopyBufferClean(copy_buffer);
   Release(copy_buffer);
}

/* The buffer-window functions */
static int BufferRow(void *rowdata, char *s)
{
   t_copyfile *cp = rowdata;

   sprintf(s, "%s", cp->file);
   if (cp->cut)
      return ROW_STRIKE;
   return 0;
}

static void *IndexCreater(void *listdata, int i)
{
   t_copy_buffer *buf = listdata;
   return buf->cpf + i;
}

static void UpdBufferList(int id, void *data, void *calldata, int reason)
{
   t_copy_buffer_window *bw = data;
   (void)id;
   (void)calldata;
   if (reason & UPD_BUFFER_FLAG)
      Refresh(bw->idbuf);
}

static void CleanBuffer(void *data)
{
   t_copy_buffer *copy_buffer=data;
   CopyBufferClean(copy_buffer);
   ConditionalRefresh(NULL, UPD_BUFFER_FLAG);
}

static void DestroyBufferWindow(void *data)
{
   t_copy_buffer_window *bw = data;
   Release(bw);
}

extern void CopyBufferCloseWindow(t_copy_buffer_window *bw)
{
   Remove(bw->idbufwin);
}

static void DoCloseBufferWindow(void *data)
{
   t_copy_buffer_window *bw = data;
   *bw->isopen = 0;
   CopyBufferCloseWindow(bw);
}

extern struct t_copy_buffer_window *CopyBufferOpenWindow(int *isopen, int *addmode)
{
   t_copy_buffer_window *bw;
   const char *const*txt;

   *isopen = 1;
   txt = GetBufferTexts();
   bw = GetMem(t_copy_buffer_window, 1);
   bw->isopen = isopen;
   bw->idbufwin = MkDialogue(ADAPTIVE, txt[SELECTION_BUFFER], W_FLOATING);
   RegisterRefresh(bw->idbufwin, UpdBufferList, bw);
   bw->idbuf = AddList(DOWNLEFT, global_copy_buffer, &global_copy_buffer->n, 200, 0, BufferRow, NULL, 10);
   SetIndexedList(bw->idbuf, IndexCreater);
   AddButton(DOWNLEFT, txt[CLOSE], DoCloseBufferWindow, bw);
   AddButton(RIGHT | ALIGNRIGHT, txt[CLEAN], CleanBuffer, global_copy_buffer);
   AddCheck(LEFT, txt[ADDMODE], addmode);
   DisplayWin();
   HookExit(bw->idbufwin, DestroyBufferWindow, bw);
   return bw;
}
