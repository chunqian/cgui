/* Module graphini.c
   Contains functions for setting up what is needed for cgui concerning
   graphics. Here you will also find the dialogu for letting the
   user chose graphics settings.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "object.h"
#include "mouse.h"
#include "cursor.h"
#include "memint.h"
#include "graphini.h"
#include "graphini.ht"
#include "icondata.h"
#include "labldata.h"
#include "cursdata.h"

#define NRBPP 5
#define NRSIMPLE 3
#define NRMULTI 5
#define MAXFAIL 10

#if ALLEGRO_SUB_VERSION > 0
#define text_mode(x)
#endif

int cgui_colors[NR_OF_CGUI_COLORS];
static int cgui_colors_pused[NR_OF_CGUI_COLORS];

static int set_failed;
static BITMAP *save_screen;
static GFX_MODE fail_mode;

typedef struct t_row {
   GFX_MODE *mode;
   struct t_scrmode *scrmode;
   int i;
} t_row;

typedef struct t_scrmode {
   GFX_MODE *cursel;
   int id_cursel;
   int tabsel;
   int nrows;
   GFX_MODE_LIST *glist;
   int glist_generated;
   void **dummy_texts;
   const char *const* txt;
   void (*CallBack) (void);
   t_row *rows;
} t_scrmode;

typedef struct t_pair {
   int w, h;
} t_pair;


#ifdef ALLEGRO_WITH_XWINDOWS
static int driver_mode = GFX_AUTODETECT_WINDOWED;
#else
static int driver_mode = GFX_AUTODETECT_FULLSCREEN;
#endif
static char *scrmod_section_name = "cgui:screen-res";
static char *cgui_dbid_w = "Width";
static char *cgui_dbid_h = "Height";
static char *cgui_dbid_cd = "Colour_depth";

BITMAP *cgui_bmp;

static int get_config_rgb(const char *varname, int r, int g, int b)
{
   const char *value;
   int color;

   value = get_config_string("cgui:colors", varname, NULL);
   if (value) {
      sscanf(value, "%d,%d,%d", &r, &g, &b);
   }
   color = makecol(r, g, b);
   return color;
}

#define DEFAULT_COLOR_GRAY   210,210,210
#define DEFAULT_COLOR_DGRAY  143,143,143
#define DEFAULT_COLOR_LGRAY  224,224,224
#define DEFAULT_COLOR_LBLUE  0,  200,255
#define DEFAULT_COLOR_BLUE   0,  0,  255
#define DEFAULT_COLOR_DBLUE  60, 80, 128
#define DEFAULT_COLOR_WHITE  255,255,255
#define DEFAULT_COLOR_BLACK  0,  0,  0

static void LoadWidgetComponentsColors(void)
{
   cgui_colors[CGUI_COLOR_DESKTOP] = get_config_rgb("CGUI_COLOR_DESKTOP", DEFAULT_COLOR_LBLUE);
   cgui_colors[CGUI_COLOR_UNSELECTED_TAB] = get_config_rgb("CGUI_COLOR_UNSELECTED_TAB", DEFAULT_COLOR_GRAY);
   cgui_colors[CGUI_COLOR_SELECTED_TAB] = get_config_rgb("CGUI_COLOR_SELECTED_TAB", DEFAULT_COLOR_GRAY);
   cgui_colors[CGUI_COLOR_LIGHTENED_BORDER] = get_config_rgb("CGUI_COLOR_LIGHTENED_BORDER", DEFAULT_COLOR_LGRAY);
   cgui_colors[CGUI_COLOR_HEAVY_LIGHTENED_BORDER] = get_config_rgb("CGUI_COLOR_HEAVY_LIGHTENED_BORDER", DEFAULT_COLOR_WHITE);
   cgui_colors[CGUI_COLOR_SHADOWED_BORDER] = get_config_rgb("CGUI_COLOR_SHADOWED_BORDER", DEFAULT_COLOR_DGRAY);
   cgui_colors[CGUI_COLOR_HEAVY_SHADOWED_BORDER] = get_config_rgb("CGUI_COLOR_HEAVY_SHADOWED_BORDER", DEFAULT_COLOR_BLACK);
   cgui_colors[CGUI_COLOR_CONTAINER] = get_config_rgb("CGUI_COLOR_CONTAINER", DEFAULT_COLOR_GRAY);
   cgui_colors[CGUI_COLOR_WIDGET_BACKGROUND] = get_config_rgb("CGUI_COLOR_WIDGET_BACKGROUND", DEFAULT_COLOR_GRAY);
   cgui_colors[CGUI_COLOR_LABEL] = get_config_rgb("CGUI_COLOR_LABEL", DEFAULT_COLOR_BLACK);
   cgui_colors[CGUI_COLOR_LABEL_FOCUS] = get_config_rgb("CGUI_COLOR_LABEL_FOCUS", DEFAULT_COLOR_LBLUE);
   cgui_colors[CGUI_COLOR_LABEL_HIDDEN_FOCUS] = get_config_rgb("CGUI_COLOR_LABEL_HIDDEN_FOCUS", DEFAULT_COLOR_LBLUE);
   cgui_colors[CGUI_COLOR_LABEL_INACTIVE_1] = get_config_rgb("CGUI_COLOR_LABEL_INACTIVE_1", DEFAULT_COLOR_LGRAY);
   cgui_colors[CGUI_COLOR_LABEL_INACTIVE_2] = get_config_rgb("CGUI_COLOR_LABEL_INACTIVE_2", DEFAULT_COLOR_DGRAY);
   cgui_colors[CGUI_COLOR_BUTTON_FRAME_FOCUS] = get_config_rgb("CGUI_COLOR_BUTTON_FRAME_FOCUS", DEFAULT_COLOR_LBLUE);
   cgui_colors[CGUI_COLOR_TEXT_CURSOR] = get_config_rgb("CGUI_COLOR_TEXT_CURSOR", DEFAULT_COLOR_BLACK);
   cgui_colors[CGUI_COLOR_DRAGGED_TEXT] = get_config_rgb("CGUI_COLOR_DRAGGED_TEXT", DEFAULT_COLOR_BLACK);
   cgui_colors[CGUI_COLOR_LISTBOX_TEXT] = get_config_rgb("CGUI_COLOR_LISTBOX_TEXT", DEFAULT_COLOR_BLACK);
   cgui_colors[CGUI_COLOR_LISTBOX_BACKGROUND] = get_config_rgb("CGUI_COLOR_LISTBOX_BACKGROUND", DEFAULT_COLOR_WHITE);
   cgui_colors[CGUI_COLOR_LISTBOX_FOCUS_TEXT] = get_config_rgb("CGUI_COLOR_LISTBOX_FOCUS_TEXT", DEFAULT_COLOR_WHITE);
   cgui_colors[CGUI_COLOR_LISTBOX_FOCUS_BACKGROUND] = get_config_rgb("CGUI_COLOR_LISTBOX_FOCUS_BACKGROUND", DEFAULT_COLOR_LBLUE);
   cgui_colors[CGUI_COLOR_LISTBOX_HIDDEN_FOCUS_BACKGROUND] = get_config_rgb("CGUI_COLOR_LISTBOX_HIDDEN_FOCUS_BACKGROUND", DEFAULT_COLOR_LBLUE);
   cgui_colors[CGUI_COLOR_LISTBOX_HIDDEN_FOCUS_TEXT] = get_config_rgb("CGUI_COLOR_LISTBOX_HIDDEN_FOCUS_TEXT", DEFAULT_COLOR_BLACK);
   cgui_colors[CGUI_COLOR_LISTBOX_ROW_DELIMITER] = get_config_rgb("CGUI_COLOR_LISTBOX_ROW_DELIMITER", DEFAULT_COLOR_WHITE);
   cgui_colors[CGUI_COLOR_LISTBOX_COLUMN_DELIMITER] = get_config_rgb("CGUI_COLOR_LISTBOX_COLUMN_DELIMITER", DEFAULT_COLOR_GRAY);
   cgui_colors[CGUI_COLOR_TOOL_TIP_BACKGROUND] = get_config_rgb("CGUI_COLOR_TOOL_TIP_BACKGROUND", DEFAULT_COLOR_WHITE);
   cgui_colors[CGUI_COLOR_TOOL_TIP_FRAME] = get_config_rgb("CGUI_COLOR_TOOL_TIP_FRAME", DEFAULT_COLOR_BLACK);
   cgui_colors[CGUI_COLOR_TOOL_TIP_TEXT] = get_config_rgb("CGUI_COLOR_TOOL_TIP_TEXT", DEFAULT_COLOR_BLACK);
   cgui_colors[CGUI_COLOR_TEXTBOX_TEXT] = get_config_rgb("CGUI_COLOR_TEXTBOX_TEXT", DEFAULT_COLOR_BLACK);
   cgui_colors[CGUI_COLOR_TEXTBOX_BACKGROUND] = get_config_rgb("CGUI_COLOR_TEXTBOX_BACKGROUND", DEFAULT_COLOR_GRAY);
   cgui_colors[CGUI_COLOR_CONTAINER_LABEL] = get_config_rgb("CGUI_COLOR_CONTAINER_LABEL", DEFAULT_COLOR_BLACK);
   cgui_colors[CGUI_COLOR_EDITBOX_BACKGROUND_MARK] = get_config_rgb("CGUI_COLOR_EDITBOX_BACKGROUND_MARK", DEFAULT_COLOR_LBLUE);
   cgui_colors[CGUI_COLOR_EDITBOX_TEXT_MARK] = get_config_rgb("CGUI_COLOR_EDITBOX_TEXT_MARK", DEFAULT_COLOR_WHITE);
   cgui_colors[CGUI_COLOR_EDITBOX_BACKGROUND_INACTIVE] = get_config_rgb("CGUI_COLOR_EDITBOX_BACKGROUND_INACTIVE", DEFAULT_COLOR_LGRAY);
   cgui_colors[CGUI_COLOR_EDITBOX_TEXT_INACTIVE] = get_config_rgb("CGUI_COLOR_EDITBOX_TEXT_INACTIVE", DEFAULT_COLOR_DGRAY);
   cgui_colors[CGUI_COLOR_EDITBOX_BACKGROUND] = get_config_rgb("CGUI_COLOR_EDITBOX_BACKGROUND", DEFAULT_COLOR_WHITE);
   cgui_colors[CGUI_COLOR_EDITBOX_TEXT] = get_config_rgb("CGUI_COLOR_EDITBOX_TEXT", DEFAULT_COLOR_BLACK);
   cgui_colors[CGUI_COLOR_STATUSFIELD_BACKGROUND] = get_config_rgb("CGUI_COLOR_STATUSFIELD_BACKGROUND", DEFAULT_COLOR_GRAY);
   cgui_colors[CGUI_COLOR_STATUSFIELD_TEXT] = get_config_rgb("CGUI_COLOR_STATUSFIELD_TEXT", DEFAULT_COLOR_BLACK);
   cgui_colors[CGUI_COLOR_PROGRESSBAR] = get_config_rgb("CGUI_COLOR_PROGRESSBAR", DEFAULT_COLOR_LBLUE);
   cgui_colors[CGUI_COLOR_PROGRESSBAR_BACKGROUND] = get_config_rgb("CGUI_COLOR_PROGRESSBAR_BACKGROUND", DEFAULT_COLOR_GRAY);
   cgui_colors[CGUI_COLOR_TITLE_FOCUS_BACKGROUND] = get_config_rgb("CGUI_COLOR_TITLE_FOCUS_BACKGROUND", DEFAULT_COLOR_DBLUE);
   cgui_colors[CGUI_COLOR_TITLE_FOCUS_TEXT] = get_config_rgb("CGUI_COLOR_TITLE_FOCUS_TEXT", DEFAULT_COLOR_WHITE);
   cgui_colors[CGUI_COLOR_TITLE_UNFOCUS_BACKGROUND] = get_config_rgb("CGUI_COLOR_TITLE_UNFOCUS_BACKGROUND", DEFAULT_COLOR_DGRAY);
   cgui_colors[CGUI_COLOR_TITLE_UNFOCUS_TEXT] = get_config_rgb("CGUI_COLOR_TITLE_UNFOCUS_TEXT", DEFAULT_COLOR_LGRAY);
   cgui_colors[CGUI_COLOR_TREE_VIEW_BACKGROUND] = get_config_rgb("CGUI_COLOR_TREE_VIEW_BACKGROUND", DEFAULT_COLOR_WHITE);
   cgui_colors[CGUI_COLOR_TREE_CONTROL_BACKGROUND] = get_config_rgb("CGUI_COLOR_TREE_CONTROL_BACKGROUND", DEFAULT_COLOR_WHITE);
   cgui_colors[CGUI_COLOR_TREE_CONTROL_OUTLINE] = get_config_rgb("CGUI_COLOR_TREE_CONTROL_OUTLINE", DEFAULT_COLOR_DGRAY);
   cgui_colors[CGUI_COLOR_CHECKBOX_MARK_BACKGROUND] = get_config_rgb("CGUI_COLOR_CHECKBOX_MARK_BACKGROUND", DEFAULT_COLOR_WHITE);
   cgui_colors[CGUI_COLOR_BROWSEBAR_BACKGROUND] = get_config_rgb("CGUI_COLOR_BROWSEBAR_BACKGROUND", DEFAULT_COLOR_GRAY);
   cgui_colors[CGUI_COLOR_BROWSEBAR_HANDLE_BACKGROUND] = get_config_rgb("CGUI_COLOR_BROWSEBAR_HANDLE_BACKGROUND", DEFAULT_COLOR_GRAY);
   cgui_colors[CGUI_COLOR_RESIZER_HANDLE] = get_config_rgb("CGUI_COLOR_RESIZER_HANDLE", DEFAULT_COLOR_DGRAY);
}

/* This array along with the below two functions is an attempt to keep the current colors when the
   user changed the screen mode. This may not work if paletted mode is used. */
RGB saved_colors[NR_OF_CGUI_COLORS];
static void SaveColorsAsRGB(void)
{
   int i;
   for (i=0; i<NR_OF_CGUI_COLORS; i++) {
      saved_colors[i].r = getr(cgui_colors[i]);
      saved_colors[i].g = getg(cgui_colors[i]);
      saved_colors[i].b = getb(cgui_colors[i]);
   }
}

static void SetSavedRGBColors(void)
{
   int i;
   for (i=0; i<NR_OF_CGUI_COLORS; i++) {
      cgui_colors[i] = makecol(saved_colors[i].r, saved_colors[i].g, saved_colors[i].b);
   }
}

static void LoadScreenSettings(int *w, int *h, int *bpp)
{
   *w = get_config_int(scrmod_section_name, cgui_dbid_w, 1024);
   *h = get_config_int(scrmod_section_name, cgui_dbid_h, 768);
   *bpp = get_config_int(scrmod_section_name, cgui_dbid_cd, 32);
   if (*w < 320)
      *w = 320;
   if (*h < 240)
      *h = 240;
   if (*bpp < 8)
      *bpp = 8;
}

static void StoreScreenSettings(int w, int h, int bpp)
{
   set_config_int(scrmod_section_name, cgui_dbid_w, w);
   set_config_int(scrmod_section_name, cgui_dbid_h, h);
   set_config_int(scrmod_section_name, cgui_dbid_cd, bpp);
   flush_config_file();
}

static void DestroyGeneratedModeList(GFX_MODE_LIST *glist)
{
   Release(glist->mode);
   Release(glist);
}

/* Generate a mode list just by chance. Can be used in case there is no
   real mode list present. */
static GFX_MODE_LIST *GetGFXModes(void)
{
   GFX_MODE_LIST *glist;
   int i, j=0, k, bpps[NRBPP]={8, 15, 16, 24, 32};
   t_pair sres[NRSIMPLE] = {{320,200}, {320,240}, {640,400}};
   t_pair mres[NRMULTI] = {{640,480}, {800,600},
                                       {1024,768}, {1280,1024},{1600,1200}};

   glist = GetMem0(GFX_MODE_LIST, 1);
   glist->mode = GetMem0(GFX_MODE, NRSIMPLE + NRMULTI*NRBPP + 10);
   for (i=0; i<NRSIMPLE; i++) {
      glist->mode[j].width = sres[i].w;
      glist->mode[j].height = sres[i].h;
      glist->mode[j++].bpp = 8;
   }
   for (i=0; i<NRMULTI; i++) {
      for (k=0; k<NRBPP; k++) {
         glist->mode[j].width = mres[i].w;
         glist->mode[j].height = mres[i].h;
         glist->mode[j++].bpp = bpps[k];
      }
   }
   glist->num_modes = j;
   return glist;
}

static void SelectNextLessMode(GFX_MODE_LIST *ml, int *w, int *h, int bpp)
{
   int i;
   GFX_MODE *modes;

   modes = ml->mode;
   for (i = ml->num_modes-1; i >= 0; i--) {
      if (modes[i].bpp == bpp && modes[i].width < *w && modes[i].height < *h) {
         *w = modes[i].width;
         *h = modes[i].height;
         return;
      }
   }
}

static GFX_MODE_LIST *SelectModesToTry(GFX_MODE_LIST *src, GFX_MODE *req)
{
   GFX_MODE_LIST *glist;
   GFX_MODE *modes, *smodes;
   int i, j=0, k, n, found, reqbpp;

   glist = GetMem0(GFX_MODE_LIST, 1);
   glist->mode = modes = GetMem0(GFX_MODE, src->num_modes);
   smodes = src->mode;
   for (i = src->num_modes-1; i >= 0; i--)
     if (smodes[i].width <= req->width)
        break;
   for (; i >= 0; i--)
      if (smodes[i].height <= req->height)
         break;
   if (req->bpp == 15)
      reqbpp = 16;
   else if (req->bpp == 24)
      reqbpp = 32;
   else
      reqbpp = req->bpp;
   for (; i >= 0; i--)
      if (smodes[i].bpp <= reqbpp)
         break;
   n = i;
   for (i=j=0; i<=n; i++) {
      if (smodes[i].bpp == 8) {
         modes[j] = smodes[i];
         j++;
      }
   }
   if (req->bpp == 8) {
      glist->num_modes = j;
      return glist;
   }
   switch (req->bpp) {
   case 15: /* 8,8,8... 16,15,16,15... <- */
      found = -1;
      for (i = 0; i <= n; ) {
         k = 0;
         if (smodes[i].bpp == 15) {
            /* Try to save until after next 16bpp encountered */
            if (found < 0) {
               found = i; /* Save it */
            } else {
               /* There was no 16 bpp since previous 15bpp, then we
                  must insert the previous one now */
               modes[j] = smodes[found];
               j++;
               found = i; /* Save the new one */
            }
            i++;
            if (i > n)
               break;
            k++;
         }
         if (smodes[i].bpp == 16) {
            modes[j] = smodes[i];
            j++;
            if (found >= 0) {
               /* There has been a 15 bpp mode saved, insert and empty buffer */
               modes[j] = smodes[found];
               j++;
               found = -1;
            }
            i++;
            k++;
         }
         if (k==0)
            i++;
      }
      glist->num_modes = j;
      return glist;
   case 16: /* 8,8,8... 15,16,15,16... <- */
      for (i = 0; i <= n; ) {
         k = 0;
         if (smodes[i].bpp == 15) {
            modes[j] = smodes[i];
            j++;
            i++;
            if (i > n)
               break;
            k++;
         }
         if (smodes[i].bpp == 16) {
            modes[j] = smodes[i];
            j++;
            i++;
            k++;
         }
         if (k==0)
            i++;
      }
      glist->num_modes = j;
      return glist;
   case 24: /* 8,8,8... 15,16,32,24,15,16,32,24... <- */
      found = -1;
      for (i = 0; i <= n; ) {
         k = 0;
         if (smodes[i].bpp == 15) {
            modes[j] = smodes[i];
            j++;
            i++;
            if (i > n)
               break;
            k++;
         }
         if (smodes[i].bpp == 16) {
            modes[j] = smodes[i];
            j++;
            i++;
            if (i > n)
               break;
            k++;
         }
         if (smodes[i].bpp == 24) {
            /* Try to hold it until after next 32bpp encountered */
            if (found < 0) {
               found = i; /* Save it */
            } else {
               /* There was no 32 bpp since previous 24bpp, then we
                  must insert the previous one now */
               modes[j] = smodes[found];
               j++;
               found = i; /* Save the new one */
            }
            i++;
            if (i > n)
               break;
            k++;
         }
         if (smodes[i].bpp == 32) {
            modes[j] = smodes[i];
            j++;
            if (found >= 0) {
               /* There has been a 24 bpp mode hold, insert and empty buffer */
               modes[j] = smodes[found];
               j++;
               found = -1;
            }
            i++;
            k++;
         }
         if (k==0)
            i++;
      }
      glist->num_modes = j;
      return glist;
   case 32: /* 8,8,8... 15,16,24,32,15,16,24,32... <- */
      for (i = 0; i <= n; ) {
         k = 0;
         if (smodes[i].bpp == 15) {
            modes[j] = smodes[i];
            j++;
            i++;
            if (i > n)
               break;
            k++;
         }
         if (smodes[i].bpp == 16) {
            modes[j] = smodes[i];
            j++;
            i++;
            if (i > n)
               break;
            k++;
         }
         if (smodes[i].bpp == 24) {
            modes[j] = smodes[i];
            j++;
            i++;
            if (i > n)
               break;
            k++;
         }
         if (smodes[i].bpp == 32) {
            modes[j] = smodes[i];
            j++;
            i++;
            k++;
         }
         if (k==0)
            i++;
      }
      glist->num_modes = j;
      return glist;
   }
   /* Default (unknown) */
   memmove(modes, smodes, sizeof(GFX_MODE)*src->num_modes);
   glist->num_modes = src->num_modes;
   return glist;
}

/* Tries to find the best possible resolution less than the passed one.
   Returns 1 on success else 0. */
static int RetryOtherModes(int w, int h, int bpp)
{
   int i;
   GFX_MODE_LIST *glist = NULL, *tmp;
   GFX_MODE *mode;

   /* Save error */
   set_failed++;
   fail_mode.width = w;
   fail_mode.height = h;
   fail_mode.bpp = bpp;
   if (gfx_driver) {
      tmp = get_gfx_mode_list(gfx_driver->id);
      if (tmp) {
         glist = SelectModesToTry(tmp, &fail_mode);
         destroy_gfx_mode_list(tmp);
      }
   }
   if (glist == NULL) {
      tmp =  GetGFXModes();
      glist = SelectModesToTry(tmp, &fail_mode);
      DestroyGeneratedModeList(tmp);
   }
   for (i=glist->num_modes-2; i>=0; i--) {
      mode = glist->mode + i;
      set_color_depth(mode->bpp);
      request_refresh_rate(70);
      if (set_gfx_mode(driver_mode, mode->width, mode->height, 0, 0) == 0)
         break;
   }
   DestroyGeneratedModeList(glist);
   return i >= 0;
}

static void SetScreenMode(int w, int h, int bpp, int regenerate)
{
   int error, prev_w = 0, prev_h = 0, prev_bpp = 0, mx, my, mz;
   int cgui_rgb_r_shift=0, cgui_rgb_g_shift=0, cgui_rgb_b_shift=0;

   set_failed = 0;
   if (screen) {
      prev_w = SCREEN_W;
      prev_h = SCREEN_H;
      prev_bpp = bitmap_color_depth(screen);
   }
   _CguiMousePos(&mx, &my, &mz);
   if (prev_w ==w && prev_h ==h && prev_bpp == bpp)
      return;
   if (cgui_started)
      PrepareNewScreenMode();
   SaveColorsAsRGB();
   request_refresh_rate(70);
   if (cgui_started) {
      switch (prev_bpp) {
      case 15:
         cgui_rgb_r_shift = _rgb_r_shift_15;
         cgui_rgb_g_shift = _rgb_g_shift_15;
         cgui_rgb_b_shift = _rgb_b_shift_15;
         break;
      case 16:
         cgui_rgb_r_shift = _rgb_r_shift_16;
         cgui_rgb_g_shift = _rgb_g_shift_16;
         cgui_rgb_b_shift = _rgb_b_shift_16;
         break;
      case 24:
         cgui_rgb_r_shift = _rgb_r_shift_24;
         cgui_rgb_g_shift = _rgb_g_shift_24;
         cgui_rgb_b_shift = _rgb_b_shift_24;
         break;
      case 32:
         cgui_rgb_r_shift = _rgb_r_shift_32;
         cgui_rgb_g_shift = _rgb_g_shift_32;
         cgui_rgb_b_shift = _rgb_b_shift_32;
         break;
      }
   }
   set_color_depth(bpp);
   error = set_gfx_mode(driver_mode, w, h, 0, 0);
   if (error) {
      if (!RetryOtherModes(w, h, bpp)) {
         allegro_message("Unable to set any graphics mode - the graphic card is unknown"
                     "\nor useless.\n"
                     "Maybe you have no VESA driver installed?\n");
         exit(0);
      }
   }
   if (cgui_started) {
      switch (prev_bpp) {
      case 15:
         _rgb_r_shift_15 = cgui_rgb_r_shift;
         _rgb_g_shift_15 = cgui_rgb_g_shift;
         _rgb_b_shift_15 = cgui_rgb_b_shift;
         break;
      case 16:
         _rgb_r_shift_16 = cgui_rgb_r_shift;
         _rgb_g_shift_16 = cgui_rgb_g_shift;
         _rgb_b_shift_16 = cgui_rgb_b_shift;
         break;
      case 24:
         _rgb_r_shift_24 = cgui_rgb_r_shift;
         _rgb_g_shift_24 = cgui_rgb_g_shift;
         _rgb_b_shift_24 = cgui_rgb_b_shift;
         break;
      case 32:
         _rgb_r_shift_32 = cgui_rgb_r_shift;
         _rgb_g_shift_32 = cgui_rgb_g_shift;
         _rgb_b_shift_32 = cgui_rgb_b_shift;
         break;
      }
   }
   SetSavedRGBColors();
   if (cgui_started) {
      if (prev_bpp != bitmap_color_depth(screen)) {
         NewScreenMode();
      }
      InitCursor();
      _CguiForceMousePos(mx, my);
      if (cgui_bmp)
         destroy_bitmap(cgui_bmp);
      cgui_bmp = create_bitmap(SCREEN_W, SCREEN_H);
      RemakeWindows(prev_w, prev_h);
   }
}

static int FormatMode(GFX_MODE *mode, char *s)
{
   sprintf(s, "%d x %d, %d bpp", mode->width, mode->height, mode->bpp);
   return 0;
}

static void FormatFailed(void *data, char *s)
{
   t_scrmode *scrmode=data;

   if (set_failed) {
      FormatMode(&fail_mode, s);
      if (set_failed > 1)
         sprintf(s+strlen(s), " (%d %s)", set_failed-1, scrmode->txt[MORE_TRIES]);
   } else
      *s = 0;
}

static void *IndexCreater(void *listdata, int i)
{
   t_scrmode *scrmode=listdata;
   return scrmode->rows + i;
}

static int FormatRow(void *data, char *s)
{
   t_row *r=data;
   return FormatMode(r->mode, s);
}

static void SelMode(int id nouse, void *data)
{
   t_row *r = data;
   r->scrmode->cursel = r->mode;
   Refresh(r->scrmode->id_cursel);
}

static void FormatSelection(void *data, char *s)
{
   t_scrmode *scrmode=data;
   if (scrmode->cursel)
      FormatMode(scrmode->cursel, s);
   else
      *s = 0;
}

static void FormatCurrent(void *data nouse, char *s)
{
   if (screen)
      sprintf(s, "%d x %d, %d bpp", SCREEN_W, SCREEN_H, bitmap_color_depth(screen));
}

static int GetRequiredRows(t_scrmode *scrmode)
{
   int n;
   n = (SCREEN_H - 140) /
                  (text_height(CGUI_list_font) + CGUI_list_vspace);
   if (scrmode->glist->num_modes < n)
      n = scrmode->glist->num_modes;
   return n;
}

static void ApplySelection(void *data)
{
   t_scrmode *scrmode = data;
   int prev_mx, prev_my, prev_height, prev_width, prevbpp, mz;
   GFX_MODE *m;

   m = scrmode->cursel;
   if (m==NULL) {
      Req("", "You didn't choose any mode from the list. You have to do that first!|OK");
      return;
   }
   _CguiMousePos(&prev_mx, &prev_my, &mz);
   prev_width = SCREEN_W;
   prev_height = SCREEN_H;
   prevbpp = bitmap_color_depth(screen);
   SetScreenMode(m->width, m->height, m->bpp, 1);
   if (prev_width && prev_height)
      _CguiForceMousePos(prev_mx * SCREEN_W / prev_width, prev_my * SCREEN_H / prev_height);
   StoreScreenSettings(SCREEN_W, SCREEN_H, bitmap_color_depth(screen));
   DisplayWin();
   if (scrmode->CallBack)
      scrmode->CallBack();
}

static void InfoTab(void *data, int id nouse)
{
   t_scrmode *scrmode=data;

   AddTag(TOPLEFT, scrmode->txt[GRAPHICS_DRIVER]);
   AddTag(RIGHT, gfx_driver->name);
   AddTag(DOWN, gfx_driver->desc);
   AddTag(DOWNLEFT, scrmode->txt[CURRENT_MODE]);
   AddStatusField(RIGHT, 200, FormatCurrent, scrmode);
   if (set_failed) {
      AddTag(DOWNLEFT, scrmode->txt[FAILING_MODE]);
      AddStatusField(RIGHT, 200, FormatFailed, scrmode);
   }
   if (scrmode->glist_generated)
      AddTag(DOWNLEFT, scrmode->txt[FAILED_TO_DETECT_MODES]);
}

static void ListTab(void *data, int dummy nouse)
{
   t_scrmode *scrmode=data;
   int id;

   scrmode->nrows = GetRequiredRows(scrmode);
   AddTag(DOWNLEFT, scrmode->txt[SELECTED_MODE]);
   scrmode->id_cursel = AddStatusField(RIGHT, 200, FormatSelection, scrmode);
   AddButton(RIGHT, scrmode->txt[APPLY_MODE], ApplySelection, scrmode);
   StartContainer(DOWNLEFT, ADAPTIVE, scrmode->txt[AVAILABLE_MODES], CT_BORDER);
   id = AddList(TOPLEFT, scrmode, &scrmode->glist->num_modes, 200, LEFT_MOUSE, FormatRow, SelMode, scrmode->nrows);
   SetIndexedList(id, IndexCreater);
   EndContainer();
}

static void CloseDialogue(void *data)
{
   t_scrmode *scrmode = data;

   if (scrmode->glist_generated)
      DestroyGeneratedModeList(scrmode->glist);
   else
      destroy_gfx_mode_list(scrmode->glist);
   if (scrmode->dummy_texts)
      Release((void*)scrmode->dummy_texts);
   if (scrmode->rows)
      Release(scrmode->rows);
   Release(scrmode);
   CloseWin(NULL);
}

extern void ScrMode(void (*CallBack) (void))
{
   int i, nr, id;
   t_scrmode *scrmode;
   GFX_MODE_LIST *glist = NULL;
   const char *const*txt;

   /* Initialize dialogue data */
   scrmode = GetMem0(t_scrmode, 1);
   scrmode->CallBack = CallBack;
   txt = LoadCompiledTexts(cgui_labels+CGUI_CGUILABL, "graphini", &nr);
   if (nr < SIZE_graphini) {
      scrmode->dummy_texts = GetMem0(void*, SIZE_graphini);
      for (i=0; i<SIZE_graphini-1; i++)
         scrmode->dummy_texts[i] = "No label";
      scrmode->dummy_texts[i] = "\33Close";
      txt = (const char *const*)scrmode->dummy_texts;
      Req("Error", "File  `cgui.dat#cguilabl' is missing|\33Close");
   }
   scrmode->txt = txt;
   if (gfx_driver)
      glist =  get_gfx_mode_list(gfx_driver->id);
   if (glist == NULL) {
      glist =  scrmode->glist = GetGFXModes();
      scrmode->glist_generated = 1;
   }
   scrmode->glist = glist;
   scrmode->rows = GetMem0(t_row, glist->num_modes);
   for (i=0; i<glist->num_modes; i++) {
      scrmode->rows[i].scrmode = scrmode;
      scrmode->rows[i].mode = glist->mode + i;
   }

   /* Make the dialogue */
   MkDialogue(ADAPTIVE, txt[SCREEN_RESOLUTION], 0);
   AddButton(DOWNLEFT, txt[CLOSE_LABEL], CloseDialogue, scrmode);
   id = CreateTabWindow(DOWNLEFT, ADAPTIVE, &scrmode->tabsel);
   AddTab(id, ListTab, scrmode, txt[SETTING_TAB]);
   AddTab(id, InfoTab, scrmode, txt[INFO_TAB]);
   DisplayWin();
}

/* If graphics were initialized before the first initialisation of cgui the
   screen content will be restored to what is was by that time, but the
   screen mode will not..
*/
static void DeInitGraphInternal(void *data nouse)
{
   if (save_screen) {
      blit(save_screen, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
      destroy_bitmap(save_screen);
      save_screen = NULL;
   }
   if (cgui_bmp) {
      destroy_bitmap(cgui_bmp);
      cgui_bmp = NULL;
   }
}

/* - No graphics mode detected: Default behaving.
   - Cgui is alreaday running. The call is assumed to be a request to
     change the screen settings. If you prefere to change the graphics
     mode by use of set_gfx_mode/set_color_depht you must call InitCgui
     to inform cgui about the changes (cgui needs to adapt its bitmaps
     to the new mode) and the paramters must be either 0:s or the values
     of the recently set mode.
     In either case InitCgui will adapt its bitmaps to the screen
     settings.
   - The graphics mode has been set (by Allegro's set_gfx_mode), and cgui
     is not initialized.
     Only if the parameters are non-zero and differs from the current
     settings these will be changed.<br>
     In either case the screen will be used as desktop.*/
#define CGUI_INIT_CODE           0
#define CGUI_INIT_LOAD           0
#define CGUI_INIT_WINDOWED       1
#define CGUI_INIT_FULLSCREEN     2
#define CGUI_INIT_KEEP_CURRENT   0
#define CGUI_KEEP_CURRENT_MODE   0,CGUI_INIT_KEEP_CURRENT,CGUI_INIT_CODE
#define CGUI_WINDOWED_MODE       0,CGUI_INIT_WINDOWED,CGUI_INIT_CODE
#define CGUI_FULLSCREEN_MODE     0,CGUI_INIT_FULLSCREEN,CGUI_INIT_CODE
#define CGUI_LOAD_MODE           0,CGUI_INIT_LOAD,CGUI_INIT_CODE
extern void InitGraph(int w, int h, int bpp)
{
/*

     Only if all parameters are non-zero and differs from the current
     screen mode, this will be changed, else the screen mode is not
     affected.  In either case CGUI will take a screen-shot an use this
     as desktop image.
*/
   BITMAP *bmp;
   int do_set_palette = 0, prev_depth = 0, dt_bpp, dt_width, dt_height;
   static int virgin = 1;
   GFX_MODE_LIST *glist=NULL;

   if (bpp == CGUI_INIT_CODE && h != CGUI_INIT_LOAD && (screen == NULL || cgui_started)) {
      dt_bpp = desktop_color_depth();
      if (get_desktop_resolution(&dt_width, &dt_height) < 0 || dt_bpp < 8) {
         w = 0;
         h = CGUI_INIT_LOAD;
         bpp = CGUI_INIT_CODE;
      } else {
         bpp = dt_bpp;
         if (h == CGUI_INIT_WINDOWED) {
            w = dt_width;
            h = dt_height;
            /* Try to find next less resolution, or there will be no window */
            driver_mode = GFX_AUTODETECT_WINDOWED;
            if (gfx_driver) {
               glist = get_gfx_mode_list(gfx_driver->id);
               if (glist) {
                  SelectNextLessMode(glist, &w, &h, bpp);
                  destroy_gfx_mode_list(glist);
               }
            }
            if (glist == NULL) {
               glist =  GetGFXModes();
               SelectNextLessMode(glist, &w, &h, bpp);
               DestroyGeneratedModeList(glist);
            }
         } else {
            w = dt_width;
            h = dt_height;
         }
      }
   }
   if (screen)
      prev_depth = bitmap_color_depth(screen);
   if (screen == NULL) {
      if (bpp == CGUI_INIT_CODE && h == CGUI_INIT_LOAD)
         LoadScreenSettings(&w, &h, &bpp);
      SetScreenMode(w, h, bpp, 0);
      do_set_palette = 1;
   } else if (cgui_started) {
      if (w != SCREEN_W || h != SCREEN_H || bpp != bitmap_color_depth(screen)) {
         SetScreenMode(w, h, bpp, 1);
      } /* else, called with current settings...*/
   } else { /* graphic mode is set by user and we havn't inited yet, maybe
               user requestes a new mode */
      save_screen = create_bitmap(SCREEN_W, SCREEN_H);
      if (save_screen)
         blit(screen, save_screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
      if (bpp == CGUI_INIT_CODE && h == CGUI_INIT_KEEP_CURRENT) {
         ;
      } else {
         if (w != SCREEN_W || h != SCREEN_H || bpp != bitmap_color_depth(screen))
            SetScreenMode(w, h, bpp, 0);
      }
      if (save_screen && bitmap_color_depth(screen) != bitmap_color_depth(save_screen)) {
         /* bitmap must be adjusted */
         bmp = create_bitmap(save_screen->w, save_screen->h);
         if (bmp)
            blit(save_screen, bmp, 0, 0, 0, 0, save_screen->w, save_screen->h);
         destroy_bitmap(save_screen);
         save_screen = bmp;
      }
      if (save_screen)
         DesktopImage(save_screen);
   }
   if (!cgui_started) {
      text_mode(-1);
      cgui_bmp = create_bitmap(SCREEN_W, SCREEN_H);
      HookCguiDeInit(DeInitGraphInternal, NULL);
      LoadWidgetComponentsColors();
   }
   if (virgin) {
      virgin = 0;
      fixup_datafile(__cgui_icons);
   }
}

extern int CguiSetColor(int color_name, int r, int g, int b)
{
   int prevcolor = -1;
   if (color_name < NR_OF_CGUI_COLORS) {
      prevcolor = cgui_colors[color_name];
      cgui_colors[color_name] = makecol(r, g, b);
   }
   return prevcolor;
}

extern void CguiPushColor(int color_name, int r, int g, int b)
{
   if (color_name < NR_OF_CGUI_COLORS) {
      cgui_colors_pused[color_name] = cgui_colors[color_name];
      cgui_colors[color_name] = makecol(r, g, b);
   }
}

extern void CguiPopColor(int color_name)
{
   cgui_colors[color_name] = cgui_colors_pused[color_name];
}

