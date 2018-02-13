#ifndef CGUI_H
#define CGUI_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "cgui/cguicfg.h"

#ifdef __cplusplus
  extern "C" {
#endif

#ifdef DJGPP
#define nouse __attribute__((unused))
#else
#define nouse
#endif

#ifdef WIN32
#define inline __inline
#endif

#define CGUI_VERSION_MAJOR      2
#define CGUI_VERSION_MINOR      0
#define CGUI_VERSION_PATCH      5
#define CGUI_VERSION_STRING     "2.0.5"
#define CGUI_DATE               20151215    /* yyyymmdd */
#define CGUI_DATE_STRING "Dec 15, 2015"

/* Init */

#define CGUI_INIT_CODE           0
#define CGUI_INIT_LOAD           0
#define CGUI_INIT_WINDOWED       1
#define CGUI_INIT_FULLSCREEN     2
#define CGUI_INIT_KEEP_CURRENT   0

CGUI_FUNC(int, InitCguiEx, (int w, int h, int colour_depth, int *errno_ptr,
     CGUI_METHOD(int, atexit_ptr, (CGUI_METHOD(void, f, (void))))));
#define InitCguiLoadMode()  InitCguiEx(0, CGUI_INIT_LOAD, CGUI_INIT_CODE, &errno, (int (*)(void (*)(void)))atexit)
#define InitCguiFullscreenMode()  InitCguiEx(0, CGUI_INIT_FULLSCREEN, CGUI_INIT_CODE, &errno, (int (*)(void (*)(void)))atexit)
#define InitCguiWindowedMode()  InitCguiEx(0, CGUI_INIT_WINDOWED, CGUI_INIT_CODE, &errno, (int (*)(void (*)(void)))atexit)
#define InitCguiKeepCurrent()  InitCguiEx(0, CGUI_INIT_KEEP_CURRENT, CGUI_INIT_CODE, &errno, (int (*)(void (*)(void)))atexit)
#define InitCgui(w, h, bpp)  InitCguiEx(w, h, bpp, &errno, (int (*)(void (*)(void)))atexit)
CGUI_FUNC(void, DeInitCgui, (void));
CGUI_FUNC(void, CguiUseUTF8, (void));
CGUI_FUNC(int, CguiParseLabels, (int state));

CGUI_VAR(int , cgui_ver);
CGUI_VAR(int , cgui_rev);
CGUI_VAR(int , cgui_minor_rev);
CGUI_VAR(int , cgui_release_date);


/* DO NOT USE THESE! They are commands used by CGUI, use the
   direction-commands specified under "Window Objects" */
#define DIR_TOPLEFT           1
#define DIR_RIGHT             2
#define DIR_LEFT              4
#define DIR_DOWNLEFT          8
#define DIR_DOWN              0x10
#define AUTOINDICATOR         0x007fffff

/* The "direction commands", for general ise */
#define TOPLEFT         DIR_TOPLEFT,AUTOINDICATOR
#define RIGHT           DIR_RIGHT,AUTOINDICATOR
#define DOWNLEFT        DIR_DOWNLEFT,AUTOINDICATOR

#define LEFT            DIR_LEFT,AUTOINDICATOR
#define DOWN            DIR_DOWN,AUTOINDICATOR

#define ALIGNCENTRE           0x00800000
#define ALIGNBOTTOM           0x01000000
#define ALIGNRIGHT            0x02000000

#define HORIZONTAL            0x04000000
#define VERTICAL              0x08000000

#define FILLSPACE             0x10000000
#define EQUALWIDTH            0x20000000
#define EQUALHEIGHT           0x40000000

struct FONT;
struct BITMAP;
struct DATAFILE;

/* Windows */
#define ADAPTIVE        0,AUTOINDICATOR
#define FILLSCREEN      1,AUTOINDICATOR
CGUI_FUNC(int, MkDialogue, (int width, int height, const char *label, int attr));
#define W_SIBLING    (1<<0)        /* The attributes of a dialogue window  */
#define W_NOMOVE     (1<<2)
#define W_FLOATING   (1<<3)
#define W_TOP        (1<<4)
#define W_BOTTOM     (1<<5)
#define W_LEFT       (1<<6)
#define W_RIGHT      (1<<7)
#define W_CENTRE_H   (W_LEFT|W_RIGHT)
#define W_CENTRE_V   (W_TOP|W_BOTTOM)
#define W_CENTRE     (W_CENTRE_H|W_CENTRE_V)

CGUI_FUNC(void, DisplayWin, (void));
CGUI_FUNC(void, CloseWin, (void *dummy));
CGUI_FUNC(void, SetWindowPosition, (int x, int y));
CGUI_FUNC(void, GetWinInfo, (int id, int *x, int *y, int *width, int *height));
CGUI_FUNC(void, DesktopImage, (struct BITMAP *bmp));
CGUI_FUNC(int, CurrentWindow, (void));
CGUI_FUNC(void, SetOperatingWindow, (int winid));
CGUI_FUNC(int, Req, (const char *winheader, const char *format_text));
CGUI_PRINTFUNC(int, Request, (const char *title, int options, int width, const char *format, ...), 4, 5);
CGUI_FUNC(void, RedrawScreen, (void));
CGUI_FUNC(void, ScrMode, (CGUI_METHOD(void, CallBack, (void))));
CGUI_FUNC(int, MkProgressWindow, (const char *wlabel, const char *blabel, int w));
#define ID_DESKTOP cgui_desktop_id
CGUI_VAR(int , cgui_desktop_id);

/* Menus */
CGUI_FUNC(int, MkMenu, (const char *text, CGUI_METHOD(void, CallBack, (void *)), void *data));
CGUI_FUNC(int, MakeMenuBar, (void));
CGUI_FUNC(int, MkMenuBarItem, (const char *text, CGUI_METHOD(void, CallBack, (void *)), void *data));
CGUI_FUNC(void, EndMenuBar, (void));
CGUI_FUNC(int, MkScratchMenu, (int id, CGUI_METHOD(void, CallBack, (void *)), void *data));
CGUI_FUNC(int, MkSingleMenu, (int x, int y, const char *text, CGUI_METHOD(void, CallBack, (void *)), void *data));
CGUI_FUNC(int, MkMenuItem, (int sub, const char *text, const char *shortcut, CGUI_METHOD(void, CallBack, (void *)), void *data));
CGUI_FUNC(int, MkMenuRadio, (int *selvar, int n, ...));
CGUI_FUNC(int, MkMenuCheck, (int *checkvar, const char *text));
CGUI_FUNC(int, HookMenuClose, (CGUI_METHOD(void, CloseHook, (void *)), void *data));
CGUI_FUNC(int, MkGroove, (void));

/* Objects */
CGUI_FUNC(int, AddButton, (int x, int y, const char *label, CGUI_METHOD(void, CallBack, (void *data)), void *data));
enum cgui_widget_sates {CGUI_WIDGET_STATE_NORMAL, 
CGUI_WIDGET_STATE_OVER, 
CGUI_WIDGET_STATE_DOWN, 
CGUI_WIDGET_STATE_DOWN_BESIDE, 
CGUI_WIDGET_STATE_GRIPPED, 
CGUI_WIDGET_STATE_DRAGGING_OVER};
CGUI_FUNC(int, AddUserDefinedWidget, (int x, int y, int w, int h, CGUI_METHOD(void, Draw, (void *, struct BITMAP *bmp, enum cgui_widget_sates state)), 
          CGUI_METHOD(void, single_click, (void*)), void *data));
CGUI_FUNC(int, AddCheck, (int x, int y, const char *label, int *sel));
#define R_HORIZONTAL 0
#define R_VERTICAL   1
CGUI_FUNC(int, AddFlip, (int x, int y, const char *label, const char *const*strs, int *sel));
CGUI_FUNC(int, AddDropDown, (int x, int y, int width, const char *label, int *sel, const void *data, int n, CGUI_METHOD(void, CallBack, (const void *data, int i, char *s))));
CGUI_FUNC(int, AddDropDownS, (int x, int y, int width, const char *label, int *sel, const char * const *strs, int n));
CGUI_FUNC(int, AddDropDownD, (int x, int y, int width, const char *label, int *sel, const void *listdata, 
                        CGUI_METHOD(const void *, Iterate, (const void *listdata, const void *it, char *text))));
CGUI_VAR(int , cgui_drop_down_list_row_spacing);
CGUI_FUNC(int, MkRadioContainer, (int x, int y, int *var, int direction));
CGUI_FUNC(int, AddRadioButton, (const char *name));
CGUI_FUNC(void, EndRadioContainer, (void));
CGUI_FUNC(int, AddIcon, (int id, int x, int y, const char *iconname, CGUI_METHOD(void, CallBack, (void *)), void *data));
CGUI_FUNC(int, MkCanvas, (int x, int y, int width, int height, CGUI_METHOD(void, CallBack, (struct BITMAP * bmp, int x, int y, void *data)), void *data));
CGUI_FUNC(int*, GetRulerTabList, (int id, int *n));
CGUI_FUNC(int, FlipRulerTab, (int id, int x));
CGUI_FUNC(int, AddTabRuler, (int x, int y, int width, int height, int dx, int options,
           CGUI_METHOD(void, CallBack, (void *data, int x, int *tabs, int n)), void *data));
#define TR_SHOWPOS 1
CGUI_FUNC(int, UpdateProgressValue, (int id, int percent));
CGUI_FUNC(int, AddProgressBar, (int x, int y, int w, int h));
#define SL_SCALE           1
#define SL_LABEL           2
#define SL_STYLE1          4
#define SL_STYLE2          8
#define SL_STYLE3          0x20
CGUI_FUNC(int, AddSlider, (int x, int y, int length, int *ctrl, int start, int end, int option, int id));
CGUI_FUNC(int, AddSliderFloat, (int x, int y, int length, float *ctrl, float start, float end, int ndecimals, int option, int id));
CGUI_FUNC(int, HookSpinButtons, (int id, int *var, int delta1, int delta2, int minv, int maxv));
CGUI_FUNC(int, AddTag, (int x, int y, const char *tag));
CGUI_FUNC(int, AddStatusField, (int x, int y, int width, CGUI_METHOD(void, FormatFunc, (void *data, char *string)), void *data));
CGUI_FUNC(int, AddTextBox, (int x, int y, const char *text, int width, int nrows, int option));
CGUI_FUNC(int, TextboxHighlightSection, (int id, int bgcolor, int startrow, int startcolumn, int endrow, int endcolumn));
CGUI_FUNC(int, TextboxGetTopRow, (int id));
CGUI_FUNC(void, TextboxHighlighting, (int id, int bgcolor, int textcolor, int line_nr));
CGUI_FUNC(void, TextboxScrollDownOneLine, (int id));
CGUI_FUNC(const char*, TextboxGetHighlightedText, (int id));

#define TB_FRAMERAISE   0x80000000 /* text-box options */
#define TB_FRAMESINK    0x40000000
#define TB_PREFORMAT    0x20000000
#define TB_LINEFEED_    0x10000000
#define TB_FIXFONT      0x08000000
#define TB_WHITE        0x04000000
#define TB_TEXTFILE     0x02000000
#define TB_FOCUS_END    0x01000000
CGUI_FUNC(int, UpdateTextBoxText, (int id, const char *s));
CGUI_FUNC(int, AddEditBox, (int x, int y, int width, const char *label, int format, int string_buffer_size, void *data));
/* Edit-box optional stuff */
#define TERMINATE_EDIT 999
CGUI_FUNC(int, AttachComboProperty, (int id, int *sel, const char * const *strings, const int *n));
CGUI_FUNC(void, CguiEditBoxSetSelectionMode, (int mode));
CGUI_FUNC(void, GetEditData, (int *scan, int *ascii, int *offset));
CGUI_FUNC(void, SetEditData, (int scan, int ascii, int offset));
CGUI_FUNC(int, TabOnCR, (int id));
#ifdef FLONG
   #undef FLONG
#endif
#ifdef FSHORT
   #undef FSHORT
#endif
#define FBYTE     1             /* unsigned char  */
#define FSHORT    2             /* short  */
#define FINT      3             /* int  */
#define FLONG     4             /* long  */
#define FSTRING   5             /* char*   */
#define FPTRSTR   6             /* char**  */
#define FPOINTS   7
#define FBPOINTS  8
#define FFLOAT    9
#define FDOUBLE   10
#define FHEX1     11            /* 1-byte value  */
#define FHEX2     12            /* 2-byte value  */
#define FHEX4     13            /* 4-byte value  */
#define FOCT1     14            /* 1-digit view 4-byte value */
#define FOCT2     15            /* 2-digit view 4-byte value  */
#define FOCT3     16            /* 3-digit view 4-byte value  */
#define FOCT4     17            /* 4-digit view 4-byte value  */
#define FNAME     0x20
#define FBLANK0   0x40
#define FUNDEF    0x6000
#define B_UNDEF_VAL 0xFF
#define S_UNDEF_VAL 0x8000
#define I_UNDEF_VAL 0x80000000L
#define L_UNDEF_VAL 0x80000000L
#define P_UNDEF_VAL 0x8000
#define BP_UNDEF_VAL 0x80

/* Object facilities */
CGUI_FUNC(void, Refresh, (int id));
CGUI_FUNC(void, DeActivate, (int id));
CGUI_FUNC(void, Activate, (int id));
CGUI_FUNC(int, CguiLoadImage, (const char *filename, const char *imagename, int transp, int id));
CGUI_FUNC(int, InsertPoint, (int id));

/* Image types */
#define IMAGE_BMP          0
#define IMAGE_TRANS_BMP    1
#define IMAGE_RLE_SPRITE   2
#define IMAGE_CMP_SPRITE   3
CGUI_FUNC(int, RegisterImage, (void *data, const char *imagename, int type, int id));
CGUI_FUNC(const void *, GetRegisteredImage, (const char *name, int *type, int id));
#define LEFT_MOUSE      1 /* mouse button values in calls to below */
#define RIGHT_MOUSE     2
CGUI_FUNC(void, SetObjectSlidable, (int id, CGUI_METHOD(int, Slider, (int x, int y, void *src, int id, int reason)), int buttons, void *data));
CGUI_FUNC(void, SetObjectDouble, (int id, CGUI_METHOD(void, DoubleCall, (void *)), void *data, int button));
CGUI_FUNC(void, SetObjectGrippable, (int id, CGUI_METHOD(void *, Grip, (void *src, int id, int reason)), int flags, int buttons, void *data));
CGUI_FUNC(void, SetObjectDroppable, (int id, CGUI_METHOD(int, Drop, (void *dest, int id, void *src, int reason, int flags)), int flags, void *data));
#define DD_GRIPPED         0 /* drag-drop reason codes */
#define DD_UNGRIPPED       1
#define DD_SUCCESS         2
#define DD_OVER_GRIP       3
#define DD_OVER_DROP       4
#define DD_END_OVER_DROP   5
#define SL_OVER            6
#define SL_OVER_END        7
#define SL_STARTED         8
#define SL_PROGRESS        9
#define SL_STOPPED         10

CGUI_FUNC(int, ToolTipText, (int id, const char *text));
CGUI_FUNC(void, CguiSetToolTipDelay, (int delay));
CGUI_FUNC(void, CguiSetToolTipAnimation, (int step, int delay));

CGUI_FUNC(int, SetView, (int id, int flags));
#define SV_HIDE_LABEL                  1
#define SV_HIDE_ICON                   2
#define SV_NO_TOOLTIP                  4
#define SV_ONLY_BRIEF                  8
#define SV_ONLY_LONG                   16
#define SV_PREFERE_BRIEF               32
#define SV_PREFERE_LONG                0
CGUI_FUNC(int, RegisterDragFlag, (int flag));
CGUI_FUNC(int, UnRegisterDragFlag, (int flag));
CGUI_FUNC(int, SetMouseButtons, (int id, int buttons));
CGUI_FUNC(void, Remove, (int id));
CGUI_FUNC(void, Destroy, (int id));
CGUI_FUNC(int, GetPressedButton, (int id));
CGUI_FUNC(void, PointerOn, (int id));
CGUI_FUNC(void, ModifyHeader, (int id, char *newtag));
CGUI_FUNC(int, AddHandler, (int id, CGUI_METHOD(void, Handler, (void *data)), void *data));
CGUI_FUNC(void, Click, (int id));
CGUI_FUNC(int, GetObjectPosition, (int id, int *x, int *y, int *wx, int *wy));
CGUI_FUNC(int, GetObjectSize, (int id, int *w, int *h));
CGUI_FUNC(void, SetBlitLimit, (int x1, int y1, int x2, int y2));
CGUI_VAR(int , cgui_use_vsync);

/* Containers */
#define CT_BORDER       1
#define CT_OBJECT_TABLE 2
CGUI_FUNC(int, StartContainer, (int x, int y, int width, int height, const char *label, int options));
CGUI_FUNC(void, EndContainer, (void));
CGUI_FUNC(void, SelectContainer, (int id));
CGUI_FUNC(int, SetSpacing, (int id, int leftx, int xdist, int rightx, int topy, int ydist, int boty));
CGUI_FUNC(void, SetMeasure, (int leftx, int xdist, int rightx, int topy, int ydist, int boty));
CGUI_FUNC(void, SetDistance, (int xdist, int ydist));
CGUI_FUNC(void, ReBuildContainer, (int id));
CGUI_FUNC(void, EmptyContainer, (int id));

#define OFF 0
#define ON  1

/* List-boxes */
#define ROW_STRIKE            (1<<31)
#define ROW_UNDERLINE         (1<<30)
#define ROW_CHECK             (1<<29)
#define ROW_UNCHECK           (1<<28)
#define COL_RIGHT_ALIGN       (1<<27)
#define ROW_COLUMN_UNDERLINE  (1<<26)

CGUI_FUNC(int, AddList, (int x, int y, void *listdata, const int *n, int width, int events, CGUI_METHOD(int, TextFormatter, (void *, char *)), CGUI_METHOD(void, Action, (int id, void *)), int norows));
CGUI_FUNC(int, SetLinkedList, (int id, CGUI_METHOD(void *, NextCreater, (void *list, void *prev))));
CGUI_FUNC(int, SetIndexedList, (int id, CGUI_METHOD(void *, IndexCreater, (void *list, int i))));
CGUI_FUNC(int, RefreshListRow, (int id, int i));
CGUI_FUNC(void **, GetMarkedRows, (int listid, int *n));
CGUI_FUNC(int, BrowseToF, (int id, int i));
CGUI_FUNC(int, BrowseToL, (int id, int i));
CGUI_FUNC(int, BrowseTo, (int id, int i, int uncond));
CGUI_FUNC(int, SetListColumns, (int id, CGUI_METHOD(int, RowTextCreater, (void *rowdata, char *s, int colnr)), int *widths, int n, int options, char **labels, CGUI_METHOD(void, CallBack, (void *data, int id, int i)), void *data));
CGUI_FUNC(int, SetListGrippable, (int listid, CGUI_METHOD(int, Grip, (void *srcobj, int reason, void *srclist, int i)), int flags, int buttons));
CGUI_FUNC(int, SetListDroppable, (int listid, CGUI_METHOD(int, Drop, (void *destobj, int reason, void *srcobj, void *destlist, int i, int flags)), int flags));
CGUI_FUNC(int, SetListDoubleClick, (int listid, CGUI_METHOD(void, AppDouble, (int id, void *data, int i)), int button));
CGUI_FUNC(int, HookList, (int listid, void *listdata, int *n, int width, int events, CGUI_METHOD(int, TextFormatter, (void *, char *)), CGUI_METHOD(void, Action, (int, void *))));
CGUI_FUNC(int, SetDeleteHandler, (int listid, CGUI_METHOD(void, CallBack, (int rowid, void *object))));
CGUI_FUNC(int, SetInsertHandler, (int listid, CGUI_METHOD(void, CallBack, (void *list, int index))));
CGUI_FUNC(int, GetListIndex, (int id));
CGUI_FUNC(int, HookExit, (int id, CGUI_METHOD(void, ExitFun, (void *data)), void *data));
CGUI_FUNC(int, NotifyFocusMove, (int listid, CGUI_METHOD(void, CallBack, (int id, void *rowobject))));
#define TR_HIDE_ROOT    0x00000001
CGUI_FUNC(int, ListTreeView, (int listid, int width, CGUI_METHOD(int, IsLeaf, (void *rowobject)), int options));
CGUI_FUNC(int, ListTreeSetNodesExpandedState, (int listid, CGUI_METHOD(int, IsExpanded, (void *data))));
CGUI_FUNC(int, ListTreeSetNodeExpandedState, (int listid, int new_expanded_state, void *data));
CGUI_FUNC(int, InstallBelowListEndCallBack, (int listid, CGUI_METHOD(void, CallBack, (int id, void *data)), void *data));
CGUI_FUNC(int, CguiListBoxSetToolTip, (int listid, int mode, int options));
CGUI_FUNC(void, CguiListBoxRowSetBar, (int color, double percentage));
CGUI_FUNC(int, CguiListBoxRowGetClickedColumn, (int rowid));
CGUI_FUNC(void, CguiListBoxSetColumnSelection, (int listid, int state));

CGUI_VAR(int , cgui_list_no_multiple_row_selection);
CGUI_VAR(int, cgui_list_show_focused_row);
CGUI_VAR(struct FONT *, CGUI_list_font);
CGUI_VAR(struct FONT *, CGUI_list_row_font);
CGUI_VAR(int , CGUI_list_row_f_color);
CGUI_VAR(int , CGUI_list_row_b_color);
CGUI_VAR(int , CGUI_list_vspace);
CGUI_VAR(int , CGUI_list_fixfont);
CGUI_VAR(int , cgui_list_fix_digits);
/* Options to SetListColumns: */
#define LIST_COLUMNS_ADJUSTABLE                 (1 << 0)
#define LIST_COLUMNS_ADJUSTABLE_KEEP_BOX_WIDTH  (1 << 1)
#define LIST_COLUMNS_DELIMITER                  (1 << 2)

/* The browsing object. */
CGUI_FUNC(int, MkVerticalBrowser, (int x, int y, CGUI_METHOD(void, CallBack, (void *data)), void *data, int *viewpos));
CGUI_FUNC(int, MkHorizontalBrowser, (int x, int y, CGUI_METHOD(void, CallBack, (void *data)), void *data, int *viewpos));
CGUI_FUNC(int, NotifyBrowser, (int id, int step, int scrolled_area_length));
CGUI_FUNC(int, SetBrowserSize, (int id, int view_port_length, int browser_length));

/* File selector and file-manager */
CGUI_FUNC(int, RegisterFileType, (const char *ext, CGUI_METHOD(void, Handler, (void *data, char *path)), void *data, const char *aname, const char *icon, const char *label, CGUI_METHOD(void, Viewer, (void *privatedata, void *viewdata))));
CGUI_FUNC(const char *, FileSelect, (const char *masks, const char *rpath, int flags, const char *winheader, const char *buttonlabel));
CGUI_FUNC(void, FileManager, (const char *winheader, int flags));
CGUI_FUNC(char *, UnPackSelection, (char **flist));
#define FS_BROWSE_DAT                         (1<< 0)
#define FS_DISABLE_EDIT_DAT                   (1<< 1)
#define FS_WARN_EXISTING_FILE                 (1<< 2)
#define FS_FORBID_EXISTING_FILE               (1<< 3)
#define FS_REQUIRE_EXISTING_FILE              (1<< 4)
#define FS_SELECT_DIR                         (1<< 5)
#define FS_DISABLE_CREATE_DIR                 (1<< 6)
#define FS_DISABLE_DELETING                   (1<< 7)
#define FS_DISABLE_COPYING                    (1<< 8)
#define FS_SHOW_MENU                          (1<< 9)
#define FS_NO_SETTINGS_IN_CONFIG              (1<<10)
#define FS_MULTIPLE_SELECTION                 (1<<11)
#define FS_NO_DRAG_DROP                       (1<<12)
#define FS_DIRECT_SELECT_BY_DOUBLE_CLICK      (1<<13)
#define FS_SAVE_AS                            (1<<14)
#define FS_SHOW_DIR_TREE                      (1<<15)
#define FS_SHOW_DIRS_IN_FILE_VIEW             (1<<16)
#define FS_HIDE_LOCATION                      (1<<17)
#define FS_HIDE_UP_BUTTON                     (1<<18)
#define FS_HIDE_UP_DIRECTORY                  (1<<19)
#define FS_FILE_FILTER_IS_READ_ONLY           (1<<20)

#define FM_HIDE_FILE_VEW                      (1<<21)
#define FM_DO_NOT_SHOW_MENU                   (1<<22)
#define FM_NO_FLOATING                        (1<<23)
#define FM_BROWSE_DAT                         FS_BROWSE_DAT
#define FM_DISABLE_EDIT_DAT                   FS_DISABLE_EDIT_DAT
#define FM_DISABLE_CREATE_DIR                 FS_DISABLE_CREATE_DIR
#define FM_DISABLE_DELETING                   FS_DISABLE_DELETING
#define FM_DISABLE_COPYING                    FS_DISABLE_COPYING
#define FM_NO_SETTINGS_IN_CONFIG              FS_NO_SETTINGS_IN_CONFIG
#define FM_SHOW_DIR_TREE                      FM_SHOW_DIR_TREE
#define FM_SHOW_DIRS_IN_FILE_VIEW             FS_SHOW_DIRS_IN_FILE_VIEW
#define FM_HIDE_LOCATION                      FS_HIDE_LOCATION
#define FM_HIDE_UP_BUTTON                     FS_HIDE_UP_BUTTON
#define FM_HIDE_UP_DIRECTORY                  FS_HIDE_UP_DIRECTORY
CGUI_FUNC(void, CguiUseIcons, (const char *filename));

/* Tab windows */
CGUI_FUNC(int, CreateTabWindow, (int x, int y, int width, int height, int *status));
CGUI_FUNC(int, AddTab, (int id, CGUI_METHOD(void, callback, (void *data, int id)), void *data, const char *label));
CGUI_FUNC(void, HookLeaveTab, (CGUI_METHOD(void, callback, (void *data)), void *data));

/* Functions for all kind of objects */
CGUI_FUNC(int, SetFocusOn, (int id));
CGUI_FUNC(int, JoinTabChain, (int id));
CGUI_FUNC(int, GetCurrentFocus, (int id));

/* Fonts */
CGUI_FUNC(void, SetCguiFont, (struct FONT *f));
CGUI_FUNC(struct FONT *, GetCguiFont, (void));
CGUI_FUNC(struct FONT *, GetCguiFixFont, (void));
CGUI_FUNC(void, CguiSetBaseLine, (int base_line));

/* Event handling */
CGUI_FUNC(int, Invite, (int mask, void *data, char *text));
CGUI_FUNC(void *, Attend, (int mask));
CGUI_FUNC(void, ProcessEvents, (void));
CGUI_FUNC(void, StopProcessEvents, (void));
CGUI_FUNC(unsigned, GenEvent, (CGUI_METHOD(void, Handler, (void *)), void *msg, unsigned delay, int objid));
CGUI_FUNC(void, CguiEventIterateFunction, (CGUI_METHOD(int, Function, (void *)), void *));

CGUI_FUNC(int, KillEvent, (unsigned id));
CGUI_FUNC(void, FlushGenEvents, (void));
CGUI_FUNC(void, CguiYieldTimeslice, (int state));
CGUI_VAR(int , event_message_buffer_size);

/* Keyboard handling */
CGUI_FUNC(void, InstallKBHandler, (CGUI_METHOD(int, Handler, (void *data, int scan, int key)), void *data));
CGUI_FUNC(void *, UnInstallKBHandler, (CGUI_METHOD(int, Handler, (void *, int, int))));

/* Hot key handling */
CGUI_FUNC(int, SetHotKey, (int id, CGUI_METHOD(void, CallBack, (void *)), void *data, int scan, int ascii));
CGUI_FUNC(int, IsHotKey, (int scan, int ascii));
CGUI_FUNC(void, SimulateHotKeys, (int control, int key));
CGUI_FUNC(void, UseHotKeys, (char *s));
CGUI_FUNC(void, AutoHotKeys, (int mode));

CGUI_ARRAY(int ,cgui_colors);
typedef enum t_cgui_colors {
   CGUI_COLOR_DESKTOP,
   CGUI_COLOR_UNSELECTED_TAB,
   CGUI_COLOR_SELECTED_TAB,
   CGUI_COLOR_LIGHTENED_BORDER,
   CGUI_COLOR_HEAVY_LIGHTENED_BORDER,
   CGUI_COLOR_SHADOWED_BORDER,
   CGUI_COLOR_HEAVY_SHADOWED_BORDER,
   CGUI_COLOR_CONTAINER,
   CGUI_COLOR_WIDGET_BACKGROUND,
   CGUI_COLOR_LABEL,
   CGUI_COLOR_LABEL_FOCUS,
   CGUI_COLOR_LABEL_HIDDEN_FOCUS,
   CGUI_COLOR_LABEL_INACTIVE_1,
   CGUI_COLOR_LABEL_INACTIVE_2,
   CGUI_COLOR_BUTTON_FRAME_FOCUS,
   CGUI_COLOR_TEXT_CURSOR,
   CGUI_COLOR_DRAGGED_TEXT,
   CGUI_COLOR_LISTBOX_TEXT,
   CGUI_COLOR_LISTBOX_BACKGROUND,
   CGUI_COLOR_LISTBOX_FOCUS_TEXT,
   CGUI_COLOR_LISTBOX_FOCUS_BACKGROUND,
   CGUI_COLOR_LISTBOX_HIDDEN_FOCUS_BACKGROUND,
   CGUI_COLOR_LISTBOX_HIDDEN_FOCUS_TEXT,
   CGUI_COLOR_LISTBOX_ROW_DELIMITER,
   CGUI_COLOR_LISTBOX_COLUMN_DELIMITER,
   CGUI_COLOR_TOOL_TIP_BACKGROUND,
   CGUI_COLOR_TOOL_TIP_FRAME,
   CGUI_COLOR_TOOL_TIP_TEXT,
   CGUI_COLOR_TEXTBOX_TEXT,
   CGUI_COLOR_TEXTBOX_BACKGROUND,
   CGUI_COLOR_CONTAINER_LABEL,
   CGUI_COLOR_EDITBOX_BACKGROUND_MARK,
   CGUI_COLOR_EDITBOX_TEXT_MARK,
   CGUI_COLOR_EDITBOX_BACKGROUND_INACTIVE,
   CGUI_COLOR_EDITBOX_TEXT_INACTIVE,
   CGUI_COLOR_EDITBOX_BACKGROUND,
   CGUI_COLOR_EDITBOX_TEXT,
   CGUI_COLOR_STATUSFIELD_BACKGROUND,
   CGUI_COLOR_STATUSFIELD_TEXT,
   CGUI_COLOR_PROGRESSBAR,
   CGUI_COLOR_PROGRESSBAR_BACKGROUND,
   CGUI_COLOR_TITLE_FOCUS_BACKGROUND,
   CGUI_COLOR_TITLE_FOCUS_TEXT,
   CGUI_COLOR_TITLE_UNFOCUS_BACKGROUND,
   CGUI_COLOR_TITLE_UNFOCUS_TEXT,
   CGUI_COLOR_TREE_VIEW_BACKGROUND,
   CGUI_COLOR_TREE_CONTROL_BACKGROUND,
   CGUI_COLOR_TREE_CONTROL_OUTLINE,
   CGUI_COLOR_CHECKBOX_MARK_BACKGROUND,
   CGUI_COLOR_BROWSEBAR_BACKGROUND,
   CGUI_COLOR_BROWSEBAR_HANDLE_BACKGROUND,
   CGUI_COLOR_RESIZER_HANDLE,
   NR_OF_CGUI_COLORS} t_cgui_colors;

CGUI_FUNC(int, CguiSetColor, (int color_name, int r, int g, int b));
CGUI_FUNC(void, NameCase, (char *text));
CGUI_FUNC_DEPRECATED(void, Sound, (int freq, int duration));
CGUI_FUNC(int, AddClock, (int x, int y, int options));
#define CLOCK_SHOW_SECONDS 1
#define CLOCK_SHOW_MINUTES 2
#define CLOCK_NO_DIALOGUE  4
CGUI_FUNC(int, ToUpper, (int chr));
CGUI_FUNC(int, SaveDatafileObject, (const char *path, void *data, int type));
CGUI_FUNC(int, CreateNewDataFile, (const char *path, const char *fn, int pack, const char *pwd));
CGUI_PRINTFUNC(char*, msprintf, (const char *format, ...), 1, 2);

/* Cursor and mouse */
CGUI_FUNC(void, InstallCursor, (int cursor_no, struct BITMAP *sprite, int x, int y));
CGUI_FUNC(void, ShowPointer, (void));
CGUI_FUNC(void, HidePointer, (void));
CGUI_FUNC(void, PointerLocation, (int id, int *x, int *y));
CGUI_FUNC(struct BITMAP *, ObjectApearance, (int id));
CGUI_FUNC(void, OverlayPointer, (struct BITMAP *sprite, int x, int y));
CGUI_FUNC(void, RemoveOverlayPointer, (void));
CGUI_FUNC(void, MkTextPointer, (struct FONT *f, const char *text));
CGUI_FUNC(void, SelectCursor, (int cursor_no));
#define CURS_ILLEGAL 0
#define CURS_DRAGGABLE 1
#define CURS_DRAG_V 2
#define CURS_DRAG_H 3
#define CURS_CROSS 4
#define CURS_BUSY 5
#define CURS_DEFAULT 6
CGUI_FUNC(void, CguiSetMouseInput, (CGUI_METHOD(void, MouseInput, (int *x, int *y, int *z, int *buttons)), CGUI_METHOD(void, ForcePos, (int x, int y)), CGUI_METHOD(void, SetRange, (int x, int y, int w, int h))));
CGUI_FUNC(void, SetMousePos, (int x, int y));
CGUI_VAR(int , cgui_mouse_draw_in_interrupt);

/* Text facility */
CGUI_FUNC(const char *const*, LoadTexts, (const char *fn, const char *section, int *nr));
CGUI_FUNC(void, DestroyTexts, (void));
CGUI_FUNC(void, RegisterConversionHandler, (CGUI_METHOD(void, Handler, (void *, char *)), void *data, const char *name));
CGUI_FUNC(void, PrintFloatingConversion, (char *dest, const char *src));
CGUI_FUNC(const char *const*, LoadCompiledTexts, (const struct DATAFILE *dat, const char *section, int *nr));

/* Refreshing system */
CGUI_FUNC(void, RegisterRefresh, (int id, CGUI_METHOD(void, AppUpd, (int id, void *data, void *calldata, int reason)), void *data));
CGUI_FUNC(void, ConditionalRefresh, (void *calldata, int reason));

/* Re-sizing facility */
CGUI_FUNC(int, GetSizeOffset, (int id, int *x, int *y));
CGUI_FUNC(int, SetSizeOffset, (int id, int x, int y));
CGUI_FUNC(int, MakeStretchable, (int id, CGUI_METHOD(void, Notify, (void *)), void *data, int options));
#define NO_VERTICAL     1
#define NO_HORIZONTAL   2
CGUI_VAR(int , continous_update_resize);

/* obsolete */
#ifndef ERROR_IF_OBSOLETE
#define FRAMERAISE   TB_FRAMERAISE
#define FRAMESINK    TB_FRAMESINK
#define W_NOMODAL    W_FLOATING
#define W_CHILD      0
#endif

#ifdef __cplusplus
  }
#endif

#endif
