/* Module FILEBROW.C
   Contains the gui-part of the file browser. There are two api-functions,
   one for selecting file(s) and another for file-maninging. Both
   wraps a generic file-browser.
*/
#include <ctype.h>
#include <string.h>
#include <allegro.h>

#include "filebrow.h"
#include "cgui.h"
#include "cgui/mem.h"
#include "cgui/dw_conv.h"

#include "id.h"
#include "node.h"
#include "window.h"
#include "filereg.h"
#include "files.h"
#include "filedial.h"
#include "filedat.h"
#include "fileimpq.h"
#include "filebuff.h"
#include "filebrow.ht"
#include "datedit.h"
#include "event.h"
#include "memint.h"
#include "cguiinit.h"
#include "labldata.h"
#include "font.h"
#include "icondata.h"
#include <errno.h>

#ifdef ALLEGRO_UNIX
#define COL_LAB COLUMN_LABELS_U
#define SHOW_MODE SHOW_PERM
#else
#define COL_LAB COLUMN_LABELS
#define SHOW_MODE SHOW_ATTRIB
#endif

#define MAX_NR_OF_COLUMNS

#define FILE_SEL_EVENT_MASK 0x40000000
#define FB_GRIPFLAGS 0x80000000
#define DROP_TO_OPEN_DELAY 2000

#if ALLEGRO_SUB_VERSION == 0
#define textout_ex(bmp, f, buf, x, y, col, bgcol) textout(bmp, f, buf, x, y, col)
#define canonicalize_filename fix_filename_path
#endif

static int nr_open_managers = 0;

typedef struct t_grip t_grip;

struct t_grip {
   /* Pointer back to the browser object. */
   t_browser *br;

   /* The detination path (the path of the object over which the gripped object is held). */
   char *dest_path;

   /* The detination object (a pointer to the object over which the gripped object is held). */
   t_diskobj *dest_dobj;

   /* A list of the gripped objects. */
   t_diskobj **dobjs;
   int ndobjs;

   /* Flag indicating that there are only plain files (no directories) among the gripped objects. */
   int contains_plain_files;
};

struct t_browser {
   /* Pointer to the virtual root of the directory tree. */
   t_root *root;

   /* Pointer to the `dirinfo' object of the currently selected directory. */
   t_dirinfo *dirinfo;

   /* Path to the directory that is displayed for the moment */
   char *path;

   /* Restriction path. The start point of the browsing, and only browsing from this point and dowwards are allowed. */
   char *rpath;

   /* A pointer to the global copy buffer. */
   t_copy_buffer *copy_buffer;

   /* Currently selected files (without path) */
   char *selfile;
   char *finalselection;
   char *masks;

   /* A pointer to the copy-buffer window. */
   t_copy_buffer_window *copy_buffer_window;

   /* A flag indicating that the lodaing in progress concerns a directory that should be expanded in the tree, when loading has been finished. */
   int do_expand_node;

   int idlocation;
   int idm;
   int idsel;
   int idlist;
   int idtree;
   int idstatus;
   int idwin;
   int evid;
   int winx, winy;
   int buffer_open;
   int addmode;
   int show_image;
   int show_preview;
   int show_org;
   int show_time;
   int show_size;
   int show_mode;
   int show_hs;
   int flags;
   int column_index_to_data_type[NR_COLUMN_LABELS];
   int widths[NR_COLUMN_LABELS];
   int nr_of_columns;
   int width_st;
   int width_se;
   int width_pa;
   int width;
   int height;
   int width_tree;
   int height_tree;
   int sort;
   int sort_desc;
   int force_case;
   int browse_dat;
   int warn_exist;
   int forbid_exist;
   int require_exist;
   int select_dir;
   int save_mode;
   int allow_delete;
   int allow_copy;
   int edit_dat;
   int no_action;
   int multi_select;
   int load_settings;
   int save_pwd;

   /* Tells the current selected "list index" in the tree view. */
   int cur_tree_index;
   t_preview *preview_win;
   char *winheader;

   /* The column labels in the list. */
   const char *l[NR_COLUMN_LABELS];

   /* Just a pointer to the above */
   char **labels;
   char *section_id;

   /* Nr of disks in disklist */
   int nodisks;
   char **disklist;
   /* Index to selected disk */
   int diskind;

   /* If non-zero then also directories will be shown in the file list (not only files). */
   int show_dirs_in_file_list;

   /* If non-zero then the .. dir will not be shown in the file list. */
   int hide_up_dir;

   /* Used to indicate for the load call-back that this is the first load. */
   int initial;

   /* Pointer to a grip object used during a drag-and-drop operation, else NULL. */
   t_grip *gr;
};

static char *icon_name_mkdatadir = "#fb42_mkdatadir";
static char *icon_name_datadir = "#fb42_datadir";
static char *icon_name_mkdir = "#fb42_makedir";
static char *icon_name_up = "#fb42_updir";
static char *icon_name_uparrow = "#fb42_uparrow";
static char *icon_name_dir = "#fb42_isdirectory";
static char *icon_name_drive = "#fb42_drive";

static const char *const*txt;

static char *item_image = "Show-image";
static char *item_preview = "Show-preview";
static char *item_org = "Show-org";
static char *item_time = "Show-time";
static char *item_size = "Show-size";
static char *item_mode = "Show-mode";
static char *item_hs = "Show-hidden-system";
static char *item_case = "Force-case";
static char *item_bufop = "Show-buffer";
static char *item_addmode = "Add-mode";
static char *item_winx = "X-pos";
static char *item_winy = "Y-pos";
static char *item_width = "Width";
static char *item_height = "Height";
static char *item_width0 = "Width-0";
static char *item_width1 = "Width-1";
static char *item_width2 = "Width-2";
static char *item_width3 = "Width-3";
static char *item_width4 = "Width-4";
static char *item_width_tree = "Width-tree";
static char *item_height_tree = "Height-tree";
static char *item_sortcol = "Sorting-column";
static char *item_sortdesc = "Sorting-descending";
static char *item_width_st = "Width-status";
static char *item_width_se = "Width-select";
static char *item_width_pa = "Width-path";
static char *item_path = "Init-path";

static char *section_pwds = "DAT_PWD";
static char *item_pwd = "pwd";

static void InitColumnXRefTable(t_browser *br)
{
   int i=0, n=NR_COLUMN_LABELS;

   br->column_index_to_data_type[i++] = L_ICON;
   br->column_index_to_data_type[i++] = L_NAME;
   if (br->show_time)
      br->column_index_to_data_type[i++] = L_TIME;
   else
      br->column_index_to_data_type[--n] = L_TIME;
   if (br->show_size)
      br->column_index_to_data_type[i++] = L_SIZE;
   else
      br->column_index_to_data_type[--n] = L_SIZE;
   if (br->show_mode)
      br->column_index_to_data_type[i++] = L_ATTRIB;
   else
      br->column_index_to_data_type[--n] = L_ATTRIB;
}

static void SetLabels(t_browser *br)
{
   int i;

   i = 0;
   br->l[i++] = txt[COL_LAB+L_ICON];
   br->l[i++] = txt[COL_LAB+L_NAME];
   if (br->show_time)
      br->l[i++] = txt[COL_LAB+L_TIME];
   if (br->show_size)
      br->l[i++] = txt[COL_LAB+L_SIZE];
   if (br->show_mode)
      br->l[i++] = txt[COL_LAB+L_ATTRIB];
   br->labels = (char **)br->l; /* ### */
   br->nr_of_columns = i;
   InitColumnXRefTable(br);
}

static void LoadPasswordSettings(void)
{
   const char *pwd;
   char s[100];
   int i;

   i = 0;
   do {
      sprintf(s, "%s_%d", item_pwd, i++);
      pwd = get_config_string(section_pwds, s, "");
      if (*pwd)
         AddPassword(pwd);
   } while (*pwd);
}

#define BUF_SIZE 1000
static void LoadSettings(t_browser *br)
{
   const char *path;
   char buffer[BUF_SIZE];
   br->show_image = get_config_int(br->section_id, item_image, 0);
   br->show_preview = get_config_int(br->section_id, item_preview, 0);
   br->show_org = get_config_int(br->section_id, item_org, 0);
   br->show_time = get_config_int(br->section_id, item_time, 0);
   br->show_size = get_config_int(br->section_id, item_size, 0);
   br->show_mode = get_config_int(br->section_id, item_mode, 0);
   br->show_hs = get_config_int(br->section_id, item_hs, 0);
   br->force_case = get_config_int(br->section_id, item_case, 0);
   br->buffer_open = get_config_int(br->section_id, item_bufop, 0);
   br->addmode = get_config_int(br->section_id, item_addmode, 0);
   br->winx = get_config_int(br->section_id, item_winx, 0) * SCREEN_W / 10240;
   br->winy = get_config_int(br->section_id, item_winy, 0) * SCREEN_H / 10240;
   br->width = get_config_int(br->section_id, item_width, 0);
   br->height_tree = get_config_int(br->section_id, item_height_tree, 0);
   br->width_tree = get_config_int(br->section_id, item_width_tree, 0);
   br->height = get_config_int(br->section_id, item_height, 0);
   br->sort = get_config_int(br->section_id, item_sortcol, 1);
   br->sort_desc = get_config_int(br->section_id, item_sortdesc, 0);
   br->width_st = get_config_int(br->section_id, item_width_st, 0);
   br->width_se = get_config_int(br->section_id, item_width_se, 0);
   br->width_pa = get_config_int(br->section_id, item_width_pa, 0);
   br->widths[0] = get_config_int(br->section_id, item_width0, 22);
   br->widths[1] = get_config_int(br->section_id, item_width1, 150);
   br->widths[2] = get_config_int(br->section_id, item_width2, 100);
   br->widths[3] = get_config_int(br->section_id, item_width3, 60);
   br->widths[4] = get_config_int(br->section_id, item_width4, 20);
   path = get_config_string(br->section_id, item_path, NULL);
   if (path == NULL) {
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
      path = "c:\\";
#else
      path = getenv("HOME");
      if (path == NULL) {
         path = "/home";
      }
#endif
   }
   canonicalize_filename(buffer, path, BUF_SIZE);
   br->path = MkString(buffer);
   LoadPasswordSettings();
}

/* Exported to the dialogue only */
extern void StorePasswordSettings(void)
{
   const char *const *pwds;
   char s[100];
   int n, i;

   pwds = GetPasswordList(&n);
   for (i = 0; i < n; i++) {
      sprintf(s, "%s_%d", item_pwd, i);
      set_config_string(section_pwds, s, pwds[i]);
   }
   sprintf(s, "%s_%d", item_pwd, i);
   set_config_string(section_pwds, s, "");
   flush_config_file();
}

static void StoreSettings(t_browser *br)
{
   set_config_int(br->section_id, item_image, br->show_image);
   set_config_int(br->section_id, item_preview, br->show_preview);
   set_config_int(br->section_id, item_org, br->show_org);
   set_config_int(br->section_id, item_time, br->show_time);
   set_config_int(br->section_id, item_size, br->show_size);
   set_config_int(br->section_id, item_mode, br->show_mode);
   set_config_int(br->section_id, item_hs, br->show_hs);
   set_config_int(br->section_id, item_case, br->force_case);
   set_config_int(br->section_id, item_bufop, br->buffer_open);
   set_config_int(br->section_id, item_addmode, br->addmode);
   set_config_int(br->section_id, item_winx, br->winx * 10240 / SCREEN_W);
   set_config_int(br->section_id, item_winy, br->winy * 10240 / SCREEN_H);
   set_config_int(br->section_id, item_width, br->width);
   set_config_int(br->section_id, item_height_tree, br->height_tree);
   set_config_int(br->section_id, item_width_tree, br->width_tree);
   set_config_int(br->section_id, item_height, br->height);
   set_config_int(br->section_id, item_width0, br->widths[0]);
   set_config_int(br->section_id, item_width1, br->widths[1]);
   set_config_int(br->section_id, item_width2, br->widths[2]);
   set_config_int(br->section_id, item_width3, br->widths[3]);
   set_config_int(br->section_id, item_width4, br->widths[4]);
   set_config_int(br->section_id, item_sortcol, br->sort);
   set_config_int(br->section_id, item_sortdesc, br->sort_desc);
   if (br->idstatus)
      set_config_int(br->section_id, item_width_st, br->width_st);
   if (br->idsel)
      set_config_int(br->section_id, item_width_se, br->width_se);
   set_config_int(br->section_id, item_width_pa, br->width_pa);
   set_config_string(br->section_id, item_path, br->path);
   flush_config_file();
}

static void RowFocusMoved(int id, void *data)
{
   t_diskobj *dobj = data;
   t_browser *br;
   (void)id;

   br = DiskObjectGetUserData(dobj);
   UpdatePreviewWindow(br->preview_win, dobj);
}

static void ChangePreview(void *data)
{
   t_browser *br = data;

   if (br->show_preview) {
      br->preview_win = OpenPreviewWindow(br->section_id, br->idwin, br);
      if (br->preview_win) {
         NotifyFocusMove(br->idlist, RowFocusMoved);
         SetFocusOn(br->idwin);
         SetFocusOn(br->idlist);
      }
   } else if (br->preview_win) {
      ClosePreviewWindow(br->preview_win, br->section_id);
      br->preview_win = NULL;
      NotifyFocusMove(br->idlist, NULL);
   }
}

static void SetInitialExpansions(t_dirinfo *dirinfo)
{
   t_diskobj *dobj;
   t_dirinfo *parent;

   dobj = DirInfoGetDiskObject(dirinfo);
   DiskObjectSetExpanded(dobj, 1);
   parent = GetParent(dobj);
   if (parent) {
      SetInitialExpansions(parent);
   }
}

static int IsExpanded(void *data)
{
   t_diskobj *dobj = data;
   return DiskObjectGetExpanded(dobj);
}

static void RefreshAll(t_browser *br)
{
   const char *path;

   if (!br->save_mode)
      *br->selfile = 0;
   if (br->idlocation) {
      path = DirInfoGetPath(br->dirinfo);
      if (path) {
         Release(br->path);
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
         if (path[1] == DEVICE_SEPARATOR && path[2] == 0) {
            char *ext_path;
            ext_path = msprintf("%s%c", path, OTHER_PATH_SEPARATOR);
            br->path = FixPath(ext_path);
            Release(ext_path);
         } else {
            br->path = FixPath(path);
         }
         br->diskind = GetDiskIndex(br->path, br->disklist, br->nodisks);
#else
         br->path = FixPath(path);
#endif
      }
   }
   DisplayWin();
}

static void LoadProgressCallBack(void *data, int reason, t_dirinfo *dirinfo)
{
   t_browser *br = data;
   t_diskobj *dobj;

   br->dirinfo = dirinfo; /* Might be NULL. */
   switch (reason) {
   case DL_END:
      CguiYieldTimeslice(1);
      if (br->show_preview && br->preview_win==NULL) {
         ChangePreview(br);
      }
      if (br->buffer_open && br->copy_buffer_window==NULL) {
         br->copy_buffer_window = CopyBufferOpenWindow(&br->buffer_open, &br->addmode);
      }
      if (br->idtree) {
         if (br->initial) {
            br->initial = 0;
            if (br->dirinfo) {
               SetInitialExpansions(br->dirinfo);
               ListTreeSetNodesExpandedState(br->idtree, IsExpanded);
            }
            BrowseTo(br->idtree, br->cur_tree_index, 1);
         } else if (br->do_expand_node) {
            br->do_expand_node = 0;
            dobj = DirInfoGetDiskObject(dirinfo);
            ListTreeSetNodeExpandedState(br->idtree, 1, dobj);
         }
      }
      RefreshAll(br);
      break;
   case DL_INTERVAL_ELAPSED:
      Refresh(br->idlist);
      break;
   default:
      break;
   }
}

extern void ReloadFileList(t_browser *br)
{
   if (br->evid) {
      _KillEventOfCgui(br->evid);
      br->evid = 0;
   }
   CguiYieldTimeslice(0);
   DirInfoReLoadDirectory(&br->dirinfo);
}

static void MkMultirowTextPointer(t_grip *gr)
{
   BITMAP *bmp;
   int w = 0, h, y, n, i;

   h = text_height(CGUI_list_font);
   for (i=0; i<gr->ndobjs; i++) {
      w = MAX(w, text_length(CGUI_list_font, DiskObjectName(gr->dobjs[i])));
   }
   if (gr->ndobjs > 1 && w > 0) {
      bmp = create_bitmap(w, h * gr->ndobjs);
      clear_to_color(bmp, bitmap_mask_color(screen));
      for (y=i=0; i<gr->ndobjs; i++, y+=h) {
         textout_ex(bmp, CGUI_list_font, DiskObjectName(gr->dobjs[i]), 0, y, cgui_colors[CGUI_COLOR_DRAGGED_TEXT], -1);
      }
      RemoveOverlayPointer();
      OverlayPointer(bmp, w / 2, h * gr->ndobjs / 2);
      destroy_bitmap(bmp);
   }
}

static void DestroyGripObject(t_browser *br)
{
   t_grip *gr;

   gr = br->gr;
   if (gr) {
      br->gr = NULL;
      Release(gr->dest_path);
      Release(gr);
   }
   br->gr = NULL;
}

static void AppendSelfile(t_browser *br, const char *fn)
{
   int x;

   x = strlen(br->selfile);
   br->selfile = ResizeMem(char, br->selfile, x + strlen(fn) + 8);
   if (*br->selfile) {
      if (*br->selfile != '"') {
         memmove(br->selfile + 1, br->selfile, x);
         *br->selfile = '"';
         x++;
         br->selfile[x++] = '"';
      }
      br->selfile[x++] = ';';
      br->selfile[x++] = '"';
      sprintf(br->selfile + x, "%s\"", fn);
   } else
      strcpy(br->selfile, fn);
}

static void PasteGrippedObjectsInSelectBox(t_browser *br, t_grip *gr)
{
   int i;
   if (gr) {
      *br->selfile = 0;
      for (i=0; i<gr->ndobjs; i++) {
         AppendSelfile(br, DiskObjectName(gr->dobjs[i]));
      }
   }
}

static void PasteGrippedObjects(t_diskobj *dest_dobj, t_grip *gr)
{
   t_dirinfo *dest_dirinfo;
   t_diskobj *dobj;
   t_copy_buffer *copy_buffer;
   int i;

   if (gr) {
      if (IsDirectoryObject(dest_dobj)) {
         dest_dirinfo = DiskObjectGetDirInfo(dest_dobj);
      } else {
         dest_dirinfo = GetParent(dest_dobj);
      }
      if (dest_dirinfo) {
         copy_buffer = CopyBufferCreate();
         for (i=0; i<gr->ndobjs; i++) {
            dobj = gr->dobjs[i];
            CopyBufferInsertEntry(copy_buffer, dobj, 0);
         }
         CopyBufferPaste(copy_buffer, dest_dirinfo);
         CopyBufferDestroy(copy_buffer);
      }
   }
}

static void CreateGripObject(t_diskobj *dobj, t_browser *br)
{
   t_grip *gr;
   void **seleced_rows;
   int nsel, i, included = 0;

   DestroyGripObject(br);
   gr = GetMem0(t_grip, 1);
   gr->br = br;
   br->gr = gr;

   seleced_rows = GetMarkedRows(br->idlist, &nsel);
   gr->dobjs = GetMem0(t_diskobj*, nsel+1);
   for (i = 0; i < nsel; i++) {
      gr->dobjs[i] = seleced_rows[i];
   }
   gr->ndobjs = nsel;
   for (i = 0; i < nsel; i++) {
      if (seleced_rows[i] == dobj) {
         included = 1;
         break;
      }
   }
   if (!included) {
      gr->dobjs[gr->ndobjs] = dobj;
      gr->ndobjs++;
   }
   for (i = 0; i < nsel; i++) {
      if (DiskObjectGetDirInfo(gr->dobjs[i]) == NULL) {
         gr->contains_plain_files = 1;
      }
   }
   MkMultirowTextPointer(gr);
//    BrowseTo(br->idlist, rowind, 1);
   Release(seleced_rows);
}

static void DestroyBrowseObject(void *data)
{
   t_browser *br = data;

#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
   DestroyDisklist(br->disklist, br->nodisks);
#endif
   DestroyDirectoryTree(br->root);
   if (br->finalselection)
      Release(br->finalselection);
   DestroyGripObject(br);
   Release(br->rpath);
   Release(br->masks);
   Release(br->path);
   Release(br->selfile);
   Release(br->winheader);
   Release(br);
}

static void CancelFileDialogue(t_browser *br)
{
   if (br->load_settings) {
      int x;
      GetSizeOffset(br->idlist, &br->width, &br->height);
      GetSizeOffset(br->idtree, &br->width_tree, &br->height_tree);
      GetSizeOffset(br->idstatus, &br->width_st, &x);
      GetSizeOffset(br->idsel,    &br->width_se, &x);
      GetSizeOffset(br->idlocation,      &br->width_pa, &x);
      GetWinInfo(br->idwin, &br->winx, &br->winy, &x, &x);
      StoreSettings(br);
   }
   if (br->preview_win) {
      ClosePreviewWindow(br->preview_win, br->section_id);
      br->preview_win = NULL;
      NotifyFocusMove(br->idlist, NULL);
   }
   CloseWin(NULL);
}

static void CancelBrowserWindow(void *data)
{
   t_browser *br = data;

   --nr_open_managers;
   CancelFileDialogue(br);
}

static void CancelFileSelection(void *data)
{
   t_browser *br = data;

   br->finalselection = MkString("");
   CancelFileDialogue(br);
   Attend(FILE_SEL_EVENT_MASK);
}

/* Exported from the lib. Extracts one file from a list of selected files. */
extern char *UnPackSelection(char **flist)
       {
   char *p, *s, *e;
   s = *flist;
   if (*s == '"') {
      s++;
      e = strchr(s, '"');
      if (e == NULL)
         e = s + strlen(s);
      p = GetMem(char, e - s + 1);
      strncpy(p, s, e - s);
      p[e - s] = 0;
      if (*e == '"')
         e++;
      if (*e == ';')
         e++;
      *flist = e;
   } else if (*s) {
      p = MkString(s);
      *flist += strlen(s);
   } else {
      p = MkString("");
   }
   return p;
}

/* The "Open"-button call-back */
static void DoSelectFile(void *data)
{
   t_browser *br = data;
   char *path,s[10000], *sel, *selfiles, *selfilesmem, *final = NULL;
   int ok, existing, isdir, len = 0, n = 0;

   selfiles = selfilesmem = MkString(br->selfile);
   ok = 1;
   do {
      sel = UnPackSelection(&selfiles);
      path = CheckSelectionPath(br->path, sel, &isdir, IsFileDotDat(br->dirinfo)|IsDataFileList(br->dirinfo));
      existing = path != NULL;
      if (br->forbid_exist) {
         if (existing) {
            sprintf(s, txt[NN_ALREADY_EXISTS_CLOSE], sel);
            Req("", s);
            ok = 0;
         }
      }
      if (ok && br->warn_exist) {
         if (existing) {
            sprintf(s, txt[NN_ALREADY_EXISTS_OK_CANCEL], sel);
            ok = !Req("", s);
         }
      }
      if (ok && br->require_exist) {
         if (!existing) {
            sprintf(s, txt[NN_NOT_EXISTS_RETRY_CLOSE], sel);
            Req("", s);
            ok = 0;
         }
      }
      if (ok && existing) {
         if (br->select_dir) {
            if (!isdir) {
               sprintf(s, txt[NN_IS_NO_DIRECTORY], sel);
               Req("", s);
               ok = 0;
            }
         } else {
            if (isdir) {
               sprintf(s, txt[NN_IS_A_DIRECTORY], sel);
               Req("", s);
               ok = 0;
            }
         }
      }
      if (path || ok) {
         if (path == NULL) {
            if (br->select_dir && strcmp(get_filename(br->path), sel)==0)
               path = MkString(br->path);
            else if (br->select_dir)
               path = MkString("");
            else
               path = MergePathAndFile(br->path, sel, IsFileDotDat(br->dirinfo)|IsDataFileList(br->dirinfo));
            Win2Dos(path, path);
         }
         if (path) { /* should always exist */
            n = len;
            len += strlen(path);
            final = ResizeMem(char, final, len + 4);
            if (n == 0 && *selfiles == 0)
               strcpy(final + n, path);
            else {
               if (n > 0) {
                  sprintf(final + n, ";\"%s\"", path);
                  len += 3;
               } else {
                  sprintf(final + n, "\"%s\"", path);
                  len += 2;
               }
            }
         }
      }
      Release(sel);
      Release(path);
   } while (ok && *selfiles);
   Release(selfilesmem);
   if (ok) {
      if (final)
         br->finalselection = final;
      else
         br->finalselection = MkString("");
      CancelFileDialogue(br);
      Attend(FILE_SEL_EVENT_MASK);
   } else
      Release(final);
}

#define OK_DELETE_FILE        0
#define STOP_DELETE_FILE      1
#define DONT_ASK_DELETE_FILE  2
static int DeleteAFile(t_diskobj *dobj, int multi, int ans, const char *tmptxt, const char *name)
{
   char *s;
   int status;

   s = GetMem(char, strlen(name) + strlen(tmptxt) + strlen(txt[YES_TO_ALL]) + 2);
   sprintf(s, tmptxt, name);
   if (multi)
      strcat(s, txt[YES_TO_ALL]);
   if (ans != DONT_ASK_DELETE_FILE)
      ans = Req("", s);
   if (ans != STOP_DELETE_FILE) {
      status = RemoveDiskObject(dobj);
      if (status != 1)
         Req("", txt[DELETE_FILE_ERROR_CONTINUE]);
   }
   Release(s);
   return ans;
}

static void DeleteFiles(void **seleced_rows, int nsel)
{
   const char *tmptxt, *name;
   t_diskobj *dobj = NULL;
   int i, ans = OK_DELETE_FILE;

   for (i = 0; i < nsel; i++) {
      dobj = seleced_rows[i];
      name = DiskObjectName(dobj);
      if (IsDirectoryObject(dobj))
         tmptxt = txt[SURE_YOU_WANT_TO_REMOVE_DIRECTORY_NN_OK_CANCEL];
      else
         tmptxt = txt[SURE_YOU_WANT_TO_REMOVE_FILE_NN_OK_CANCEL];
      ans = DeleteAFile(dobj, nsel > 1 && i < nsel - 1, ans, tmptxt, name);
   }
   ConditionalRefresh(NULL, UPD_BROWSER_FLAG);
}

/* Delete-key call-back */
static void DeleteFileListSelect(int rowid, void *data)
{
   t_diskobj *dobj = data;
   t_browser *br;
   void **seleced_rows;
   int nsel;
   (void)rowid;

   br = DiskObjectGetUserData(dobj);
   seleced_rows = GetMarkedRows(br->idlist, &nsel);
   DeleteFiles(seleced_rows, nsel);
   Release(seleced_rows);
}

static void DeleteFileMenuSelect(void *data)
{
   DeleteFiles(&data, 1);
}

static void PackSelection(t_browser *br, const char *fn, void **seleced_rows, int nsel)
{
   const char *name;
   t_diskobj *dobj;
   int i, forbidden_sel, loaded = 0;

   if (seleced_rows == NULL) {
      seleced_rows = GetMarkedRows(br->idlist, &nsel);
      loaded = 1;
   }
   *br->selfile = 0;
   AppendSelfile(br, fn);
   if (br->multi_select || *fn == 0) {
      if (!br->multi_select && nsel > 1)
         nsel = 1;
      for (i = 0; i < nsel; i++) {
         dobj = seleced_rows[i];
         forbidden_sel = IsDirectoryObject(dobj) != br->select_dir;
         if (IsUpDir(dobj) || forbidden_sel) {
            fn = NULL;
            *br->selfile = 0;
            break;
         }
         name = DiskObjectName(dobj);
         if (strcmp(fn, name) != 0)
            AppendSelfile(br, name);
      }
   }
   if (loaded)
      Release(seleced_rows);
}

static void SelectFiles(void *data)
{
   t_diskobj *dobj = data;
   t_browser *br;

   br = DiskObjectGetUserData(dobj);
   PackSelection(br, DiskObjectName(dobj), NULL, 0);
   if (br->idsel)
      Refresh(br->idsel);
}

static void InsertMarkedFilesIntoBuffer(t_browser *br, int cut)
{
   t_diskobj *dobj;
   int i, n;
   void **datas;

   if (!br->addmode)
      CopyBufferClean(br->copy_buffer);

   datas = GetMarkedRows(br->idlist, &n);
   for (i = 0; i < n; i++) {
      dobj = datas[i];
      if (dobj) {
         br = DiskObjectGetUserData(dobj);
         CopyBufferInsertEntry(br->copy_buffer, dobj, cut);
      }
   }
   Release(datas);
   CopyBufferFilterDuplicates(br->copy_buffer);
   ConditionalRefresh(NULL, UPD_BUFFER_FLAG);
}

static void HandleCopyMenuSelection(void *data)
{
   t_diskobj *dobj = data;
   t_browser *br;

   br = DiskObjectGetUserData(dobj);
   InsertMarkedFilesIntoBuffer(br, 0);
}

static void HandleCutMenuSelection(void *data)
{
   t_diskobj *dobj = data;
   t_browser *br;

   br = DiskObjectGetUserData(dobj);
   InsertMarkedFilesIntoBuffer(br, 1);
}

static void PasteKeyCommand(void *data)
{
   t_browser *br = data;
   CopyBufferPaste(br->copy_buffer, br->dirinfo);
   ConditionalRefresh(NULL, UPD_BUFFER_FLAG|UPD_BROWSER_FLAG);
}

static void HandlePasteMenuSelection(void *data)
{
   t_diskobj *dest_dobj = data;
   t_dirinfo *dest_dirinfo;
   t_browser *br;

   br = DiskObjectGetUserData(dest_dobj);
   if (IsDirectoryObject(dest_dobj)) {
      dest_dirinfo = DiskObjectGetDirInfo(dest_dobj);
   } else {
      dest_dirinfo = GetParent(dest_dobj);
   }
   CopyBufferPaste(br->copy_buffer, dest_dirinfo);
   ConditionalRefresh(NULL, UPD_BUFFER_FLAG|UPD_BROWSER_FLAG);
}

static void ActionWrapper(void *data)
{
   t_diskobj *dobj = data;
   t_browser *br;
   void (*Action)(void *, char*);
   char *path;
   const char *ext;
   int isdir, type;

   br = DiskObjectGetUserData(dobj);
   type = DiskObjectDatafileObjectType(dobj);
   ext = DiskObjectExt(dobj);
   Action = GetRegAction(ext, type);
   if (Action) {
      path = CheckSelectionPath(br->path, DiskObjectName(dobj), &isdir, IsFileDotDat(br->dirinfo)|IsDataFileList(br->dirinfo));
      Action(GetRegData(ext, type), path);
      Release(path);
   }
}

static void EditPropertiesDialogWrapper(void *data)
{
   t_diskobj *dobj = data;
   t_browser *br;

   br = DiskObjectGetUserData(dobj);
   EditPropertiesDialog(br->dirinfo, dobj, &br->save_pwd, br->idlist);
}

static void EnterPasswordWrapper(void *data)
{
   t_diskobj *dobj = data;
   t_browser *br;
   char *pwd;

   br = DiskObjectGetUserData(dobj);
   pwd = InquirePassword(br->path, dobj, &br->save_pwd);
   if (pwd) {
      AddPassword(pwd);
      Release(pwd);
      if (br->save_pwd)
         StorePasswordSettings();
      CguiYieldTimeslice(0);
      ReloadFileList(br);
   }
}

static void CreateDirDialogWrapper(void *data)
{
   t_browser *br = data;
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
   if (RootGetDirInfo(br->root) == br->dirinfo)
      return;
#endif
   CreateDirDialog(br->idlist, br->dirinfo, &br->save_pwd);
}

static void CreateDatafileDialogWrapper(void *data)
{
   t_browser *br = data;
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
   if (RootGetDirInfo(br->root) == br->dirinfo)
      return;
#endif
   if (!br->edit_dat && IsDataFileList(br->dirinfo))
      return;
   CreateDatafileDialog(br->idlist, br->dirinfo, &br->save_pwd);
}

/* This is a call-back function for a drop down a "context menu". It will create the content of the menu, which is the current
   allowed operations on disk entries. All allowed operations are listed in the menu. Operations that are not possible due to the
   current state or view are listed but deactivated (like e.g. "paste" if there is nothing copied). */
static void DropQuickMenu(void *data)
{
   t_diskobj *dobj = data;
   t_diskobj *d;
   t_browser *br;
   const char *s = NULL, *name, *ext;
   char *p;
   int id;
   void **seleced_rows;
   int i, nsel, isupdir, isdir, type;

   br = DiskObjectGetUserData(dobj);
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
   if (RootGetDirInfo(br->root) == br->dirinfo)
      return;
#endif
   seleced_rows = GetMarkedRows(br->idlist, &nsel);
   isupdir = strcmp(DiskObjectName(dobj), "..") == 0;
   isdir = IsDirectoryObject(dobj);
   name = DiskObjectName(dobj);
   id = MkMenuItem(0, txt[EDIT_FILE], "Ctrl-E", EditPropertiesDialogWrapper, dobj);
   if (isupdir || nsel > 1) { /* Don't know which one to edit. */
      DeActivate(id);
   }
   if (br->allow_copy && br->allow_delete) {
      id = MkMenuItem(0, txt[CUT_FILE], "Ctrl-X", HandleCutMenuSelection, dobj);
      if (isupdir) {
         DeActivate(id);
      }
   }
   if (br->allow_copy) {
      id = MkMenuItem(0, txt[PASTE_FILE], "Ctrl-V", HandlePasteMenuSelection, dobj);
      if (CopyBufferIsEmpty(br->copy_buffer) || nsel > 1) {
         DeActivate(id);
      } else {
         p = MergePathAndFile(br->path, DiskObjectName(dobj), IsFileDotDat(br->dirinfo)|IsDataFileList(br->dirinfo));
         if (CopyBufferEntryExists(br->copy_buffer, p)) {
            DeActivate(id);
         }
         Release(p);
      }
      id = MkMenuItem(0, txt[COPY_FILE], "Ctrl-C", HandleCopyMenuSelection, dobj);
      if (isupdir) {
         DeActivate(id);
      }
   }
   if ((br->flags & FS_DISABLE_CREATE_DIR) == 0) {
      MkMenuItem(0, txt[CREATE_DIRECTORY], "Ctrl-N", CreateDirDialogWrapper, br);
   }
   if ((br->flags & FS_DISABLE_CREATE_DIR) == 0 && br->edit_dat) {
      MkMenuItem(0, txt[CREATE_DATAFILE], "Ctrl-D", CreateDatafileDialogWrapper, br);
   }

   if (br->allow_delete) {
      id = MkMenuItem(0, txt[DELETE_FILE], "Del", DeleteFileMenuSelect, dobj);
      if (isupdir) {
         DeActivate(id);
      }
   }

   if (br->no_action) {
      id = MkMenuItem(0, txt[SELECT], "", SelectFiles, dobj);
      if (br->multi_select) {
         if (br->select_dir) {
            for (i = 0; i < nsel; i++) {
               if (!IsDirectoryObject(seleced_rows[i])) {
                  DeActivate(id);
                  break;
               }
            }
         } else {
            for (i = 0; i < nsel; i++) {
               d = seleced_rows[i];
               if (IsDirectoryObject(d)) {
                  DeActivate(id);
                  break;
               }
            }
         }
      } else if (nsel > 1) {
         DeActivate(id);
      }
   } else {
      type = DiskObjectDatafileObjectType(dobj);
      ext = DiskObjectExt(dobj);
      if (IsRegistered(ext, type)) {
         s = GetRegActionName(ext, type);
         if (s == NULL)
            s = txt[FILE_ACTION];
         id = MkMenuItem(0, s, "", ActionWrapper, dobj);
         if (nsel > 1 || IsDirectoryObject(dobj))
            DeActivate(id);
      }
   }
   if (br->edit_dat && !isdir && stricmp(get_extension(name),"dat") == 0) {
      MkMenuItem(0, txt[ENTER_PASSWORD], "", EnterPasswordWrapper, dobj);
   }
   Release(seleced_rows);
}

static void PasteBelowEndCallBack(void *data)
{
   t_browser *br = data;
   CopyBufferPaste(br->copy_buffer, br->dirinfo);
   ConditionalRefresh(NULL, UPD_BUFFER_FLAG|UPD_BROWSER_FLAG);
}

static void DropQuickMenuForEmptyRow(void *data)
{
   t_browser *br = data;
   int id;

#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
   if (RootGetDirInfo(br->root) == br->dirinfo)
      return;
#endif
   if (br->allow_copy) {
      id = MkMenuItem(0, txt[PASTE_FILE], "Ctrl-V", PasteBelowEndCallBack, br);
      if (CopyBufferIsEmpty(br->copy_buffer)) {
         DeActivate(id);
      }
   }
   if ((br->flags & FS_DISABLE_CREATE_DIR) == 0) {
      MkMenuItem(0, txt[CREATE_DIRECTORY], "Ctrl-N", CreateDirDialogWrapper, br);
   }
   if ((br->flags & FS_DISABLE_CREATE_DIR) == 0 && br->edit_dat) {
      MkMenuItem(0, txt[CREATE_DATAFILE], "Ctrl-D", CreateDatafileDialogWrapper, br);
   }
}

/* Tree-row call-back. */
static void HandleTreeItemClicked(int id, void *data)
{
   t_diskobj *dobj = data;
   t_browser *br;

   if (GetPressedButton(id) == RIGHT_MOUSE) {
      MkScratchMenu(0, DropQuickMenu, dobj);
   } else {
      br = DiskObjectGetUserData(dobj);
      CguiYieldTimeslice(0);
      br->dirinfo = DirInfoLoadDirectoryFromDiskObject(dobj);
      br->cur_tree_index = GetListIndex(id);
      if (br->select_dir) {
         Release(br->selfile);
         br->selfile = MkString(DiskObjectName(dobj));
         Refresh(br->idsel);
      }
   }
}

/* List-row call-back. */
static void HandleListRowClick(int id, void *data)
{
   t_diskobj *dobj = data;
   t_diskobj *d;
   t_browser *br;
   int i, nrsel, nrseldir;
   void **selobj;

   br = DiskObjectGetUserData(dobj);
   if (GetPressedButton(id) == RIGHT_MOUSE) {
      /* It's ok whith "mutiple marked including directories" here, the decision of acceptance must be done by the choice of the menu */
      MkScratchMenu(0, DropQuickMenu, dobj);
   } else {
      selobj = GetMarkedRows(br->idlist, &nrsel);
      for (i=nrseldir=0; i<nrsel; i++) {
         d = selobj[i];
         if (IsDirectoryObject(d))
            nrseldir++;
      }
      if (IsDirectoryObject(dobj) && nrsel == 1 && nrseldir == 1) {
         CguiYieldTimeslice(0);
         br->dirinfo = DirInfoLoadDirectoryFromDiskObject(dobj);
         if (br->select_dir) {
            Release(br->selfile);
            br->selfile = MkString(DiskObjectName(dobj));
            Refresh(br->idsel);
         }
      } else {
         /* It is an ordnary file */
         if (br->select_dir && nrsel == nrseldir)
            PackSelection(br, DiskObjectName(dobj), selobj, nrsel);
         if (br->no_action) {
            if (!br->select_dir) {
               PackSelection(br, DiskObjectName(dobj), selobj, nrsel);
               if (br->idsel)
                  Refresh(br->idsel);
            }
         } else if (nrsel == 1)
            ActionWrapper(dobj);
      }
      Release(selobj);
   }
}

static void BelowListEndCallBack(int id, void *data)
{
   if (GetPressedButton(id) == RIGHT_MOUSE) {
      MkScratchMenu(0, DropQuickMenuForEmptyRow, data);
   }
}

static void DoubleClickHandler(int id, void *data, int i)
{
   t_diskobj *dobj = data;
   t_browser *br;
   (void)i;

   br = DiskObjectGetUserData(dobj);
   if (GetPressedButton(id) == LEFT_MOUSE) {
      HandleListRowClick(id, dobj);
      if (br->selfile && *br->selfile && *br->selfile != '"')
         DoSelectFile(br);
   }
}

static void MoveUp(void *data)
{
   t_browser *br = data;
   t_diskobj *dobj;

   br->do_expand_node = 1;
   br->dirinfo = DirInfoGetUpDirectory(br->dirinfo);
   if (br->select_dir) {
      Release(br->selfile);
      dobj = DirInfoGetDiskObject(br->dirinfo);
      br->selfile = MkString(DiskObjectName(dobj));
      Refresh(br->idsel);
   }
}

static int IsLeaf(void *data)
{
   t_diskobj *dobj = data;
   t_dirinfo *dirinfo;

   dirinfo = DiskObjectGetDirInfo(dobj);
   return !DirInfoHasSubdirs(dirinfo);
}

static int CreateTreeRow(void *data, char *s)
{
   t_diskobj *dobj = data;;

#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
   if (GetGrandParent(dobj) == NULL) {
      sprintf(s, "%s;%s", icon_name_drive, DiskObjectName(dobj));
   } else
#endif
   if (DiskObjectIsDotDat(dobj) || DiskObjectIsDatafileObject(dobj)) {
      sprintf(s, "%s;%s", icon_name_datadir, DiskObjectName(dobj));
   } else {
      sprintf(s, "%s;%s", icon_name_dir, DiskObjectName(dobj));
   }
   return 0;
}

static int CreateListRow(void *data, char *s, int colnr)
{
   t_diskobj *dobj = data;
   static const char *name;
   const char *org, *imname, *icon;
   t_browser *br;
   int df_type;
   long size;
   const FONT *f = NULL;

   br = DiskObjectGetUserData(dobj);
   *s = 0;
   switch (br->column_index_to_data_type[colnr]) {
   case 0: /* This column is always present, used for minor visualisation */
      cgui_list_fix_digits = 0;
      CGUI_list_fixfont = 0;
      CGUI_list_row_font = _cgui_prop_font;
      name = DiskObjectName(dobj);
      if (br->show_image) {
         imname = DiskObjectGetImage(dobj);
         f = DiskObjectGetFont(dobj);
      } else
         imname = "";
      if (IsDirectoryObject(dobj)) { /* Prio=1: Directories has "built in" icons */
         if (IsUpDir(dobj))
            strcpy(s, icon_name_uparrow);
         else if (DiskObjectIsDotDat(dobj) || DiskObjectIsDatafileObject(dobj))
            strcpy(s, icon_name_datadir);
         else
            strcpy(s, icon_name_dir);
      } else if (*imname) { /* Prio=2: Content display is requested and present */
         strcpy(s, imname);
      } else if (f) { /* Prio=3: Content is present as a font */
         CGUI_list_row_font = (FONT*)f;
         strcpy(s, "ABCDEFGHIJK");
      } else { /* Prio=3: There is an icon registered for current extension */
         if (DiskObjectIsDatafileObject(dobj)) { /* Datafile object */
            df_type = DiskObjectDatafileObjectType(dobj);
            icon = GetRegIcon(NULL, df_type);
            if (icon)
               strcpy(s, icon);
         } else {                               /* Disk file */
            icon = GetRegIcon(DiskObjectExt(dobj), 0);
            if (icon)
               strcpy(s, icon);
         }
      }
      break;
   case 1:
      org = DiskObjectDatSource(dobj);
      if (org == NULL) {
         if (*name)
            strcpy(s, name);
         else
            strcpy(s, txt[NO_NAME]);
      } else if (br->show_org)
         sprintf(s, "%s (%s)", name, org);
      else
         strcpy(s, name);
      if (br->force_case)
         NameCase(s);
      break;
   case 2:
      DiskObjectTimeString(s, dobj);
      cgui_list_fix_digits = 1;
      CGUI_list_fixfont = 0;
      return COL_RIGHT_ALIGN;
   case 3:
      if (!IsDirectoryObject(dobj) || DiskObjectIsDotDat(dobj) || DiskObjectIsDatafileObject(dobj)) {
         size = DiskObjectSize(dobj);
         cgui_list_fix_digits = 0;
         CGUI_list_fixfont = 0;
         if (size >= 1000)
            sprintf(s, "%ld", size/1000);
         else if (size >= 100)
            sprintf(s, "%0.1f", size/1000.0);
         else if (size >= 10)
            sprintf(s, "%0.2f", size/1000.0);
         else
            sprintf(s, "%0.3f", size/1000.0);
         return COL_RIGHT_ALIGN;
      }
      break;
   case 4:
      cgui_list_fix_digits = 0;
      CGUI_list_row_font = GetCguiFixFont();
      strcpy(s, DiskObjectModeString(dobj));
   }
   return 0;
}

/* Returns a dobj pointer to a subdirectory of the current directory. */
static void *IterateDirectories(void *data, void *prev)
{
   t_diskobj *dobj = data;

   if (prev) {
      return DiskObjectGetNextDirectory(dobj);
   } else {
      return DiskObjectGetFirstDirectory(dobj);
   }
}

static void *IterateDiskObjects(void *data, void *prev)
{
   t_browser *br = data;
   if (br->dirinfo == NULL) return NULL;
   if (br->show_dirs_in_file_list) {
      if (prev) {
         return DirInfoGetNextDiskObject(br->dirinfo);
      } else {
         if (br->hide_up_dir) {
            DirInfoGetFirstDiskObject(br->dirinfo);
            return DirInfoGetNextDiskObject(br->dirinfo);
         } else {
            return DirInfoGetFirstDiskObject(br->dirinfo);
         }
      }
   } else {
      if (prev) {
         return DirInfoGetNextFile(br->dirinfo);
      } else {
         return DirInfoGetFirstFile(br->dirinfo);
      }
   }
}

static void SetNewMask(void *data)
{
   t_browser *br = data;
   SetLoadMasks(br->root, br->masks);
   CguiYieldTimeslice(0);
   ReloadFileList(br);
}

/* If editing is in progress and the enter-key is pressed, it will call the reload directory-function. This is done in a way safe for a callback of
   an edit-box. */
static void MaskEdited(void *data)
{
   t_browser *br = data;
   int scan, ascii, curpos;

   GetEditData(&scan, &ascii, &curpos);
   if (scan == KEY_ENTER && ascii == '\r')
      _GenEventOfCgui(SetNewMask, br, 0, br->idwin);
}

static void SetNewLocation(void *data)
{
   t_browser *br = data;
   CguiYieldTimeslice(0);
   br->do_expand_node = 1;
   br->dirinfo = DirInfoLoadDirectoryFromPath(br->root, br->path);
}

static void LocationEdited(void *data)
{
   t_browser *br = data;
   int scan, ascii, curpos;

   GetEditData(&scan, &ascii, &curpos);
   if (scan == KEY_ENTER && ascii == '\r')
      _GenEventOfCgui(SetNewLocation, br, 0, br->idwin);
}

#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
static void ChangeDrive(void *data)
{
   t_browser *br = data;

   br->path = ResizeMem(char, br->path, strlen(br->disklist[br->diskind]) + 2);
   strcpy(br->path, br->disklist[br->diskind]);
   AppendFileDelimiter(br->path);
   CguiYieldTimeslice(0);
   DirInfoLoadDirectoryFromPath(br->root, br->path);
}

static void LPDirletter(const void *data, int i, char *s)
{
   const char *const*dirinfo = data;

   strcpy(s, dirinfo[i]);
}
#endif

static void SwitchSorting(void *data, int id, int colnr)
{
   t_browser *br = data;
   int sort_crit;
   (void)id;

   if (br->evid) {
      _KillEventOfCgui(br->evid);
      br->evid = 0;
   }
   sort_crit = br->column_index_to_data_type[colnr];
   br->labels = (char **)br->l; /* ### */
   if (br->sort == sort_crit) {
      br->sort_desc ^= 1;
   } else {
      br->sort_desc = 0;
      br->sort = sort_crit;
   }
   SetLoadSorting(br->root, br->sort, br->sort_desc);
   Refresh(br->idlist);
}

static void Updater(void *data)
{
   t_browser *br = data;

   Refresh(br->idlist);
}

static void SelectUpdate(void *data)
{
   t_browser *br = data;

   Refresh(br->idlist);
}

static void OpenNewBrowser(void *data)
{
   t_browser *br = data;

   FileManager(NULL, br->flags);
}

static void BufferOpenChecked(void *data)
{
   t_browser *br = data;

   if (br->buffer_open) {
      br->copy_buffer_window = CopyBufferOpenWindow(&br->buffer_open, &br->addmode);
   } else {
      if (br->copy_buffer_window) {
         CopyBufferCloseWindow(br->copy_buffer_window);
         br->copy_buffer_window = NULL;
      }
   }
}

static void ChangeFileListColumns(t_browser *br)
{
   int i, saved_widths[NR_COLUMN_LABELS];

   /* Find the current location of the requested type. */
   for (i=0; i<NR_COLUMN_LABELS; i++) {
      saved_widths[br->column_index_to_data_type[i]] = br->widths[i];
   }
   SetLabels(br);
   for (i=0; i<NR_COLUMN_LABELS; i++) {
      br->widths[i] = saved_widths[br->column_index_to_data_type[i]];
   }
   SetListColumns(br->idlist, CreateListRow, br->widths, br->nr_of_columns, LIST_COLUMNS_ADJUSTABLE, br->labels, SwitchSorting, br);
   DisplayWin();
}

static void ShowTimeMenuItemSelected(void *data)
{
   ChangeFileListColumns(data);
}

static void ShowSizeMenuItemSelected(void *data)
{
   ChangeFileListColumns(data);
}

static void ShowModeFlagsMenuItemSelected(void *data)
{
   ChangeFileListColumns(data);
}

static void ShowHiddenFilesMenuItemSelected(void *data)
{
   t_browser *br = data;
   SetLoadHiddenSystem(br->root, br->show_hs);
   DisplayWin();
}

static void SelectOptions(void *data)
{
   t_browser *br = data;
   int id;

   id = MkMenuCheck(&br->show_preview, txt[SHOW_PREVIEW_WINDOW]);
   AddHandler(id, ChangePreview, br);
   id = MkMenuCheck(&br->show_image, txt[SHOW_IMAGE]);
   AddHandler(id, Updater, br);
   if (br->browse_dat) {
      id = MkMenuCheck(&br->show_org, txt[SHOW_ORIGINAL]);
      AddHandler(id, Updater, br);
   }
   id = MkMenuCheck(&br->show_time, txt[SHOW_TIME]);
   AddHandler(id, ShowTimeMenuItemSelected, br);
   id = MkMenuCheck(&br->show_size, txt[SHOW_SIZE]);
   AddHandler(id, ShowSizeMenuItemSelected, br);
   id = MkMenuCheck(&br->show_mode, txt[SHOW_MODE]);
   AddHandler(id, ShowModeFlagsMenuItemSelected, br);
   id = MkMenuCheck(&br->show_hs, txt[SHOW_HS]);
   AddHandler(id, ShowHiddenFilesMenuItemSelected, br);
   id = MkMenuCheck(&br->force_case, txt[FORCE_CASE]);
   AddHandler(id, SelectUpdate, br);
   MkMenuItem(0, txt[OPEN_NEW_BROWSER], "", OpenNewBrowser, br);
   id = MkMenuCheck(&br->buffer_open, txt[SHOW_BUFFER]);
   AddHandler(id, BufferOpenChecked, br);
}

static const char *const*MakeDummyStrings(void)
{
   int i;
   char **t;

   t = GetMem(char*, SIZE_filebrow);
   for (i=0; i<SIZE_filebrow; i++)
      t[i] = "??";
   return (const char *const*) t;
}

static void CopyKeyCommand(void *data)
{
   t_browser *br = data;
   InsertMarkedFilesIntoBuffer(br, 0);
}

static void MoveKeyCommand(void *data)
{
   t_browser *br = data;
   InsertMarkedFilesIntoBuffer(br, 1);
}

static int HandleGrip(void *srcobj, int reason, void *srclist, int rowind)
{
   t_diskobj *dobj = srcobj;
   t_browser *br;
   (void)srclist;
   (void)rowind;

   if (dobj == NULL)
      return 0;
   switch (reason) {
   case DD_OVER_GRIP:
      if (IsUpDir(dobj))
         return 0;
      break;
   case DD_GRIPPED:
      br = DiskObjectGetUserData(dobj);
      CreateGripObject(dobj, br);
      break;
   case DD_UNGRIPPED:
      break;
   case DD_SUCCESS:
      break;
   default:
      return 0;
   }
   return 1;
}

/* If grip is still existing, then open directory. */
static void HandleGrippedHoldOverTimeOut(void *data)
{
   t_browser *br = data;
   t_grip *gr;

   gr = br->gr;
   br->evid = 0;
   if (gr) {
      if (!br->save_mode)
         *br->selfile = 0;
      Release(br->path);
      br->path = FixPath(gr->dest_path);
      CguiYieldTimeslice(0);
      br->dirinfo = DirInfoLoadDirectoryFromDiskObject(gr->dest_dobj);
   }
}

/* If grip is still existing, then open directory. */
static void HandleGrippedHoldOverTreeTimeOut(void *data)
{
   t_browser *br = data;
   t_grip *gr;

   gr = br->gr;
   br->evid = 0;
   if (gr) {
      if (!br->save_mode)
         *br->selfile = 0;
      Release(br->path);
      br->path = FixPath(gr->dest_path);
      CguiYieldTimeslice(0);
      br->do_expand_node = 1;
      br->dirinfo = DirInfoLoadDirectoryFromDiskObject(gr->dest_dobj);
   }
}

static int Drop(void *destobj, int reason, void *srcobj, void *destlist, int i, int flags)
{
   t_diskobj *dest_dobj = destobj;
   t_browser *br = destlist;
   (void)srcobj;
   (void)i;
   (void)flags;

   if (dest_dobj == NULL)
      return 1;
   switch (reason) {
   case DD_OVER_DROP:
      if (IsDirectoryObject(dest_dobj)) {
         if (br->gr) {
            Release(br->gr->dest_path);
            if (IsDirectoryObject(destobj)) {
               br->gr->dest_dobj = dest_dobj;
               br->gr->dest_path = DiskObjectGetFullPathDos(dest_dobj);
            } else {
               br->gr->dest_path = NULL;
            }
            if (br->gr->dest_path && br->evid == 0)
               br->evid = _GenEventOfCgui(HandleGrippedHoldOverTimeOut, br, DROP_TO_OPEN_DELAY, br->idwin);
         }
         return 1;
      }
      break;
   case DD_END_OVER_DROP:
      if (br->evid) {
         _KillEventOfCgui(br->evid);
         br->evid = 0;
      }
      break;
   case DD_SUCCESS:
      PasteGrippedObjects(dest_dobj, br->gr);
      DestroyGripObject(br);
      ConditionalRefresh(NULL, UPD_BUFFER_FLAG|UPD_BROWSER_FLAG);
      break;
   }
   return 1;
}

static int HandleDropInTree(void *destobj, int reason, void *srcobj, void *destlist, int i, int flags)
{
   t_diskobj *dest_dobj = destobj;
   t_dirinfo *dest_dirinfo;
   t_browser *br;
   (void)srcobj;
   (void)destlist;
   (void)i;
   (void)flags;

   if (dest_dobj == NULL)
      return 1;
   dest_dirinfo = DiskObjectGetDirInfo(dest_dobj);
   br = DiskObjectGetUserData(dest_dobj);
   switch (reason) {
   case DD_OVER_DROP:
      if (br->gr) {
         Release(br->gr->dest_path);
         br->gr->dest_path = DiskObjectGetFullPathDos(dest_dobj);
         if (br->gr->dest_path && br->evid == 0) {
            br->gr->dest_dobj = dest_dobj;
            br->evid = _GenEventOfCgui(HandleGrippedHoldOverTreeTimeOut, br, DROP_TO_OPEN_DELAY, br->idwin);
         }
      }
      break;
   case DD_END_OVER_DROP:
      if (br->evid) {
         _KillEventOfCgui(br->evid);
         br->evid = 0;
      }
      break;
   case DD_SUCCESS:
      PasteGrippedObjects(dest_dobj, br->gr);
      DestroyGripObject(br);
      ConditionalRefresh(NULL, UPD_BUFFER_FLAG|UPD_BROWSER_FLAG);
      break;
   }
   return 1;
}

static int DropSelectBox(void *dest, int id, void *src, int reason, int flags)
{
   t_browser *br = dest;
   t_grip *gr;
   int ok;
   (void)src;
   (void)flags;

   gr = br->gr;
   if (gr == NULL)
      return 0;
   switch (reason) {
   case DD_OVER_DROP:
      if (br->select_dir && gr->contains_plain_files) {
         ok = 0;
      } else {
         ok = 1;
      }
      if (ok) {
         SetFocusOn(id);
         simulate_keypress((TERMINATE_EDIT<<8)|'\r');
      }
      return ok;
   case DD_SUCCESS:
      PasteGrippedObjectsInSelectBox(br, br->gr);
      DestroyGripObject(br);
      Refresh(br->idsel);
      return 1;
   }
   return 0;
}

static void StatusUpdate(void *data, char *s)
{
   t_browser *br = data;
   int dirtext, filetext, unittext, ndir, nob, size;

   ndir = DirInfoGetNrOfSubdirs(br->dirinfo) - 1; /* -1 because we don't want to count the dir entry ".." */
   size = DirInfoGetTotalSize(br->dirinfo);
   nob = DirInfoGetNrOfDiskObjects(br->dirinfo) - 1; /* -1 because we don't want to count the dir entry ".." */
   if (ndir == 1)
      dirtext = FB_TXT_DIR;
   else
      dirtext = FB_TXT_DIRS;
   if (nob == 1)
      filetext = FB_TXT_FILE;
   else
      filetext = FB_TXT_FILES;
   if (size > 1000000)
      unittext = FB_TEXT_MB;
   else if (size > 1000)
      unittext = FB_TEXT_MB;
   else
      unittext = FB_TEXT_BYTE;

   if (size > 10000000)
      sprintf(s, "%d %s %d %s %d %s", ndir, txt[dirtext], nob - ndir, txt[filetext], size/1000000, txt[unittext]);
   else if (size > 1000000)
      sprintf(s, "%d %s %d %s %0.1f %s", ndir, txt[dirtext], nob - ndir, txt[filetext], size/1000000.0, txt[unittext]);
   else if (size > 1000)
      sprintf(s, "%d %s %d %s %d %s", ndir, txt[dirtext], nob - ndir, txt[filetext], size/1000, txt[unittext]);
   else
      sprintf(s, "%d %s %d %s %d %s", ndir, txt[dirtext], nob - ndir, txt[filetext], size/1000, txt[unittext]);
}

static void UpdList(int id, void *data, void *calldata, int reason)
{
   t_browser *br = data;
   (void)id;
   (void)calldata;

   if (UPD_BROWSER_FLAG & reason) {
      CguiYieldTimeslice(0);
      ReloadFileList(br);
   }
}

static void FileBrowserNewScreenMode(t_object *b)
{
   t_browser *br;
   br = b->appdata;
   CguiYieldTimeslice(0);
   ReloadFileList(br);
   default_node_type_functions.NewScreenMode(b);
}

static t_browser *GenericFileBrowser(const char *masks, const char *rpath, int flags, const char *winheader, const char *buttonlabel,
                                     void (*CallBack) (void *data), void (*CancelFunc) (void *data), char *section_id, int fm)
{
   t_browser *br;
   t_dirinfo *dirinfo;
   t_diskobj *dobj;
   int id, nr, winflag, prev_multi_select;
   FONT *f;
   t_object *b;
   static t_typefun tf;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      datedit_init();
      tf = default_window_type_functions;
      tf.NewScreenMode = FileBrowserNewScreenMode;
   }
   RegisterDragFlag(FB_GRIPFLAGS);

   /* Load all text that will be used in this module */
   txt = LoadCompiledTexts(cgui_labels+CGUI_CGUILABL, "filebrow", &nr);

   if (nr < SIZE_filebrow)
      txt = MakeDummyStrings();

   /* Check if default text-labels must be used */
   if (buttonlabel == NULL)
      buttonlabel = txt[DEFAULT_BUTLABEL];
   if (!*winheader)
      winheader = txt[DEFAULT_WINLABEL];

   /* If current font is too small, set a min height of the row-heights */
   f = GetCguiFont();
   if (f) {
      if (text_height(f) + CGUI_list_vspace < 16)
         CGUI_list_vspace = 16 - text_height(f);
   }

   /* Init the browser object with calling options */
   br = GetMem0(t_browser, 1);
   br->copy_buffer = CopyBufferGetGlobal();
   br->section_id = section_id;
   br->flags = flags;
   br->browse_dat = flags & FS_BROWSE_DAT;
   br->edit_dat = (flags & FS_DISABLE_EDIT_DAT) == 0;
   br->warn_exist = flags & FS_WARN_EXISTING_FILE;
   br->forbid_exist = flags & FS_FORBID_EXISTING_FILE;
   br->require_exist = flags & FS_REQUIRE_EXISTING_FILE;
   br->select_dir = (flags & FS_SELECT_DIR) != 0;
   br->allow_delete = (flags & FS_DISABLE_DELETING) == 0;
   br->allow_copy = (flags & FS_DISABLE_COPYING) == 0;
   br->multi_select = flags & FS_MULTIPLE_SELECTION;
   br->load_settings = (flags & FS_NO_SETTINGS_IN_CONFIG) == 0;
   br->save_mode = (flags & FS_SAVE_AS);
   br->hide_up_dir = (flags & FS_HIDE_UP_DIRECTORY);
   br->show_dirs_in_file_list = flags & FS_SHOW_DIRS_IN_FILE_VIEW || !(flags & FS_SHOW_DIR_TREE);
   if (br->select_dir) {
      br->warn_exist = 0;
      br->forbid_exist = 0;
      br->multi_select = 0;
      br->save_mode = 0;
   }
   br->masks = MkString(masks);
   br->selfile = MkString("");
   br->winheader = MkString(winheader);
   if (flags &FM_NO_FLOATING)
      winflag = 0;
   else
      winflag = W_FLOATING;
   if (fm && nr_open_managers > 0)
      winflag |= W_SIBLING;

   /* Init the browser object with options from previous session (cfg-file) */
   if (br->load_settings) {
      LoadSettings(br);
   } else {
      br->path = msprintf("%c", OTHER_PATH_SEPARATOR);
   }
   if (rpath && *rpath) {
      br->rpath = FixPath(rpath);
      Release(br->path);
      br->path = MkString(br->rpath);
   }

   /* "Disk-units" are not relevant on other systems than dos/win. Currently
      no other systems are supported, but hopefully there will be ...*/
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
   br->disklist = GetDiskList(&br->nodisks, br->rpath);
#endif
   /* Disk index is depending on the path (which is now maybe justified) */
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
   br->diskind = GetDiskIndex(br->path, br->disklist, br->nodisks);
#endif

   /* ---------- Set up the file loader */
   br->root = CreateRoot(LoadProgressCallBack, br, 500);
   SetLoadHiddenSystem(br->root, br->show_hs);
   SetLoadDat(br->root, br->browse_dat);
   SetLoadMasks(br->root, br->masks);
   SetLoadSorting(br->root, br->sort, br->sort_desc);
   /* ---------- END set up the file loader */

   /* ---------  Set up the dialogue */
   br->idwin = MkDialogue(ADAPTIVE, winheader, winflag);
   b = GetObject(br->idwin);
   b->tf = &tf;
   b->appdata = br;

   if (flags & FS_SHOW_MENU)
      MkSingleMenu(DOWNLEFT, txt[SETTINGS], SelectOptions, br);
   if ((flags & FS_DISABLE_CREATE_DIR) == 0) {
      id = AddButton(RIGHT, icon_name_mkdir, CreateDirDialogWrapper, br);
      ToolTipText(id, txt[CREATE_DIRECTORY]);
   }
   if ((flags & FS_DISABLE_CREATE_DIR) == 0 && br->edit_dat) {
      id = AddButton(RIGHT, icon_name_mkdatadir, CreateDatafileDialogWrapper, br);
      ToolTipText(id, txt[CREATE_DATAFILE]);
   }
   if (!(flags&FS_HIDE_UP_BUTTON)) {
      AddButton(RIGHT, icon_name_up, MoveUp, br);
   }
   if (!(flags&FS_HIDE_LOCATION)) {
      br->idlocation = AddEditBox(DOWNLEFT, 300, txt[LOCATION], FPTRSTR, 0, &br->path);
      MakeStretchable(br->idlocation, NULL, NULL, NO_VERTICAL);
      SetSizeOffset(br->idlocation, br->width_pa, 0);
      AddHandler(br->idlocation, LocationEdited, br);
   }

   /* -------- Begin ---- Main view --------- */
   prev_multi_select = cgui_list_no_multiple_row_selection;
   StartContainer(DOWNLEFT, ADAPTIVE, "", 0);

   /* -------- The tree view --------- */
   if (flags & FS_SHOW_DIR_TREE) {
      cgui_list_no_multiple_row_selection = 1;
      dirinfo = RootGetDirInfo(br->root);
      dobj = DirInfoGetDiskObject(dirinfo);
      br->idtree = AddList(DOWNLEFT, dobj, NULL, 200, LEFT_MOUSE|RIGHT_MOUSE, CreateTreeRow, HandleTreeItemClicked, 3);
      CguiListBoxSetToolTip(br->idtree, 1, 0);
      SetLinkedList(br->idtree, IterateDirectories);
      MakeStretchable(br->idtree, NULL, NULL, 0);
      ListTreeView(br->idtree, 17, IsLeaf, TR_HIDE_ROOT);
      br->initial = 1;
      if (br->height_tree > SCREEN_H - 200)
         br->height_tree = SCREEN_H - 200;
      SetSizeOffset(br->idtree, br->width_tree, br->height_tree);
      if ((flags & FS_NO_DRAG_DROP) == 0 && (flags & FS_DIRECT_SELECT_BY_DOUBLE_CLICK) == 0) {
         SetListGrippable(br->idtree, HandleGrip, FB_GRIPFLAGS, LEFT_MOUSE);
         SetListDroppable(br->idtree, HandleDropInTree, FB_GRIPFLAGS);
      }
   }
   /* -------- END ---- the tree view --------- */

   /* -------- The actual list for file display and selection --------- */
   if (!(flags & FM_HIDE_FILE_VEW)) {
      cgui_list_no_multiple_row_selection = br->multi_select==0;
      br->idlist = AddList(RIGHT, br, NULL, 50, LEFT_MOUSE | RIGHT_MOUSE, NULL, HandleListRowClick, 3);
      CguiListBoxSetToolTip(br->idlist, 1, 0);
      SetLinkedList(br->idlist, IterateDiskObjects);
      /* Attatch some useful properties of the list (stretching and partition into columns. */
      MakeStretchable(br->idlist, NULL, NULL, NO_HORIZONTAL);
      SetLabels(br);
      SetListColumns(br->idlist, CreateListRow, br->widths, br->nr_of_columns, LIST_COLUMNS_ADJUSTABLE, br->labels, SwitchSorting, br);
      /* Optional deleting of files */
      if (br->allow_delete)
         SetDeleteHandler(br->idlist, DeleteFileListSelect);
      /* Drag-n-drop only for file the manger */
      if ((flags & FS_NO_DRAG_DROP) == 0 && (flags & FS_DIRECT_SELECT_BY_DOUBLE_CLICK) == 0) {
         SetListGrippable(br->idlist, HandleGrip, FB_GRIPFLAGS, LEFT_MOUSE);
         SetListDroppable(br->idlist, Drop, FB_GRIPFLAGS);
      }
      if (flags & FS_DIRECT_SELECT_BY_DOUBLE_CLICK) {
         SetListDoubleClick(br->idlist, DoubleClickHandler, LEFT_MOUSE);
      }
      /* Restore the list size from previous session: */
      if (br->height > SCREEN_H - 184)
         br->height = SCREEN_H - 184;
      SetSizeOffset(br->idlist, br->width, br->height);
      InstallBelowListEndCallBack(br->idlist, BelowListEndCallBack, br);
   }
   /* -------- END ---- actual list for file display and selection --------- */
   EndContainer();
   cgui_list_no_multiple_row_selection = prev_multi_select;
   /* -------- END ---- Main view --------- */

   /* Once again: Different disk-drives has only mening in dos/win */
#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
   if ((flags & FS_SHOW_DIR_TREE) == 0) {
      /* This is redundant in case the entire file tree is viewed. It is not even necessary in case of flat
      view, but it might be unintuitive to move up one level above the disk unit root to find the disk list. */
      br->idm = AddDropDown(DOWNLEFT, 0, txt[DEVICE], &br->diskind, br->disklist, br->nodisks, LPDirletter);
      AddHandler(br->idm, ChangeDrive, br);
      /* The file mask(s) */
      id = AddEditBox(RIGHT, 200, txt[SHOW], FPTRSTR, 0, &br->masks);
   } else {
      /* The file mask(s) */
      id = AddEditBox(DOWNLEFT, 200, txt[SHOW], FPTRSTR, 0, &br->masks);
   }
#else
   /* The file mask(s) */
   id = AddEditBox(DOWNLEFT, 200, txt[SHOW], FPTRSTR, 0, &br->masks);
#endif
   AddHandler(id, MaskEdited, br); /* Catch changes immediately */
   if (flags & FS_FILE_FILTER_IS_READ_ONLY) {
      DeActivate(id);
   }

   /* If there is a call-back there is a possibility to select a file. */
   if (CallBack) {
      br->idsel = AddEditBox(DOWNLEFT, 200, txt[SELECTED_FILE], FPTRSTR, 0, &br->selfile);
      MakeStretchable(br->idsel, NULL, NULL, NO_VERTICAL);
      SetSizeOffset(br->idsel, br->width_se, 0);
      SetObjectDroppable(br->idsel, DropSelectBox, FB_GRIPFLAGS, br);
      if (!*buttonlabel)
         buttonlabel = txt[DEFAULT_BUTLABEL];
      AddButton(RIGHT, buttonlabel, CallBack, br);
   }

   AddButton(DOWNLEFT, txt[LABEL_CANCEL], CancelFunc, br);

   /* Set up shor-cuts for editing. Optional restrictions will be notified
      by the concerned call-back */
   SetHotKey(br->idwin, CopyKeyCommand, br, KEY_C, KEY_C);
   SetHotKey(br->idwin, MoveKeyCommand, br, KEY_X, KEY_X);
   SetHotKey(br->idwin, PasteKeyCommand, br, KEY_V, KEY_V);

   /* Makes it possible to catch update signals from various sources. */
   RegisterRefresh(br->idlist, UpdList, br);

   /* The status field and the clock is not a valid issue for simple
      file-selections, but may be important for a file manager */
   if (fm) {
      br->idstatus = AddStatusField(RIGHT, 200, StatusUpdate, br);
      Activate(br->idstatus);
      MakeStretchable(br->idstatus, NULL, NULL, NO_VERTICAL);
      SetSizeOffset(br->idstatus, br->width_st, 0);
      AddClock(RIGHT|ALIGNRIGHT, 0);
   } else {
      SetHotKey(br->idwin, CancelFileSelection, br, KEY_ESC, 27);
   }

   /* Let subsequent opened file-managers be placed in random positions */
   if (nr_open_managers > 0 && fm)
      ;
   else
      SetWindowPosition(br->winx, br->winy);

   /* Note: Show the window before loading the directory. */
   DisplayWin();

   /*  This may take som time, but the event system processes in the background. */
   CguiYieldTimeslice(0); /* This will a speed up with a factor of 50-100. */
   br->dirinfo = DirInfoLoadDirectoryFromPath(br->root, br->path);
   return br;
}

extern const char *FileSelect(const char *masks, const char *rpath, int flags, const char *winheader, const char *buttonlabel)
{
   t_browser *br;
   int was_started;
   char *finalselection;

   was_started = cgui_started;
   if (!cgui_started) {
      if (cgui_starting_in_progress)
         exit(0);
      InitCgui(0, 0, 0);
   }
   if (!(flags & FS_SELECT_DIR)) {
      /* The selector is useless without the fileview, except in case of selecting a directory. */
      flags &= ~FM_HIDE_FILE_VEW;
   }
   flags |= FM_NO_FLOATING;
   br = GenericFileBrowser(masks, rpath, flags, winheader, buttonlabel, DoSelectFile, CancelFileSelection, "File-selector", 0);
   br->no_action = 1;
   Invite(FILE_SEL_EVENT_MASK, br, "File-selector");
   _GenEventOfCgui(DestroyBrowseObject, br, 0, 0);
   finalselection = br->finalselection;
   if (!was_started) {
      br->finalselection = NULL;
      DeInitCgui();
      ExecuteAllPendingEvents();
   }
   return finalselection;
}

extern void FileManager(const char *winheader, int flags)
{
   t_browser *br;
   int f, nr;

   f = flags;
   if ((flags & FM_DO_NOT_SHOW_MENU) == 0)
      f |= FS_SHOW_MENU;
   f &= ~FS_DIRECT_SELECT_BY_DOUBLE_CLICK;
   if (winheader) {
      if (*winheader)
         br = GenericFileBrowser("*", "", f, winheader, NULL, NULL, CancelBrowserWindow, "File-manager", 1);
      else
         br = GenericFileBrowser("*", "", f, " ", NULL, NULL, CancelBrowserWindow, "File-manager", 1);
   } else {
      txt = LoadCompiledTexts(cgui_labels+CGUI_CGUILABL, "filebrow", &nr);
      if (nr < SIZE_filebrow)
         txt = MakeDummyStrings();
      br = GenericFileBrowser("*", "", f, txt[MANAGER_WINLABEL], NULL, NULL, CancelBrowserWindow, "File-manager", 1);
   }
   br->flags = flags;
   HookExit(br->idwin, DestroyBrowseObject, br);
   nr_open_managers++;
}

extern char **FileBrowserGetGrippedFileNames(int id)
{
}

extern void InitFileBrowser(void)
{
   /* Load all icons to be used (hook on to the window for releasing them). */
   RegisterImageCopy(&__cgui_mkdatadir, icon_name_mkdatadir+1, IMAGE_TRANS_BMP, 0);
   RegisterImageCopy(&__cgui_datadir, icon_name_datadir+1, IMAGE_TRANS_BMP, 0);
   RegisterImageCopy(&__cgui_makedir, icon_name_mkdir+1, IMAGE_TRANS_BMP, 0);
   RegisterImageCopy(&__cgui_updir, icon_name_up+1, IMAGE_TRANS_BMP, 0);
   RegisterImageCopy(&__cgui_uparrow, icon_name_uparrow+1, IMAGE_TRANS_BMP, 0);
   RegisterImageCopy(&__cgui_isdirectory, icon_name_dir+1, IMAGE_TRANS_BMP, 0);
   RegisterImageCopy(&__cgui_drive, icon_name_drive+1, IMAGE_TRANS_BMP, 0);
}

static void LoadASingleIcon(const char *filename, const char *objname)
{
   DATAFILE *df;

   df = load_datafile_object(filename, objname+6);
   if (df) {
      RegisterImageCopy(df->dat, objname+1, IMAGE_TRANS_BMP, 0);
      unload_datafile_object(df);
   }
}

extern void CguiUseIcons(const char *filename)
{
   LoadASingleIcon(filename, icon_name_mkdatadir);
   LoadASingleIcon(filename, icon_name_datadir);
   LoadASingleIcon(filename, icon_name_mkdir);
   LoadASingleIcon(filename, icon_name_up);
   LoadASingleIcon(filename, icon_name_uparrow);
   LoadASingleIcon(filename, icon_name_dir);
   LoadASingleIcon(filename, icon_name_drive);
}
