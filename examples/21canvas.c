/* CGUI Example program showing how to use the canvas object. */
#include <allegro.h>
#include <string.h>
#include "cgui.h"
#include "cgui/mem.h"

#define PIXEL 0
#define FILL  1
#define LINE  2
#define RECTANGLE  3
#define NR_COLORS 10

typedef struct t_edit {
   int id;
   int idwin;
   int mode;
   int upd;
   int x, y;
   int idpos;
   int curx, cury;
   int r, g, b;
   int down;
   int colind;
   int palid;
   int slid;
   int colid;
   int inited;
   char *path;
   BITMAP *bmps[NR_COLORS];/* bitmap list used to show the colors */
   BITMAP *bmp2;           /* used as temp */
   BITMAP *cbmp;           /* the canvas' bitmap */
   BITMAP *sbmp;           /* the sub-bitmap to draw on */
   int colors[NR_COLORS];
} t_edit;

static void save_image(void *data);

static void check_save(t_edit *ed)
{
   if (ed->upd)
      if (Req("", "You have made changes to the image, do you want to save?| Yes | No ")==0)
         save_image(ed);
}

static void one_point_tool(t_edit *ed, int x, int y, void (*func)(BITMAP *, int, int, int))
{
   if (mouse_b) {
      if (!ed->down) {
         ed->x = x;
         ed->y = y;
         ed->down = 1;
      }
   } else if (ed->down) {
      ed->down = 0;
      if (ed->x == x && ed->y == y) {
         func(ed->sbmp, x, y, ed->colors[ed->colind]);
         if (ed->bmp2)
            blit(ed->sbmp, ed->bmp2, 0, 0, 0, 0, ed->sbmp->w, ed->sbmp->h);
         Refresh(ed->id);
         ed->upd = 1;
      }
   }
}

static void two_points_tool(t_edit *ed, int x, int y, void (*func)(BITMAP *, int, int, int, int, int))
{
   int wx, wy, ox, oy, x1, x2, y1, y2, dummy;
   if (mouse_b) {
      if (ed->down) {
         if (ed->bmp2) {
            blit(ed->bmp2, ed->sbmp, 0, 0, 0, 0, ed->sbmp->w, ed->sbmp->h);
            func(ed->sbmp, x, y, ed->x, ed->y, ed->colors[ed->colind]);
            Refresh(ed->id);
         }
      } else {
         ed->x = x;
         ed->y = y;
         ed->down = 1;
         GetWinInfo(ed->idwin, &wx, &wy, &dummy, &dummy);
         GetObjectPosition(ed->id, &dummy, &dummy, &ox, &oy);
         x1 = ox + wx;
         y1 = oy + wy;
         x2 = x1 + ed->sbmp->w - 1;
         y2 = y1 + ed->sbmp->h - 1;
         set_mouse_range(x1, y1, x2, y2);
      }
   } else if (ed->down) {
      ed->down = 0;
      ed->upd = 1;
      set_mouse_range(0, 0, screen->w, screen->h);
      if (ed->bmp2) {
         blit(ed->bmp2, ed->sbmp, 0, 0, 0, 0, ed->sbmp->w, ed->sbmp->h);
         func(ed->sbmp, x, y, ed->x, ed->y, ed->colors[ed->colind]);
         blit(ed->sbmp, ed->bmp2, 0, 0, 0, 0, ed->sbmp->w, ed->sbmp->h);
      }
      Refresh(ed->id);
   }
}

static void new_image(void *data)
{
   t_edit *ed = data;

   clear_to_color(ed->cbmp, makecol(255,255,255));
   if (ed->sbmp) {
      check_save(ed);
      destroy_bitmap(ed->sbmp);
      clear_to_color(ed->bmp2, makecol(255,255,255));
   }
   ed->sbmp = create_sub_bitmap(ed->cbmp, 0, 0, ed->cbmp->w, ed->cbmp->h);
}

static void canvas_event_handler(BITMAP *bmp, int x, int y, void *data)
{
   t_edit *ed = data;
   int w, h;

   if (y < 0) {
      ed->cbmp = bmp;
      /* This would not be sufficient if the canvas were made resizeble: */
      if (ed->inited) {
         w = ed->cbmp->w;
         h = ed->cbmp->h;
         destroy_bitmap(ed->sbmp);
         ed->sbmp = create_sub_bitmap(ed->cbmp, 0, 0, w, h);
         blit(ed->bmp2, ed->sbmp, 0, 0, 0, 0, bmp->w, bmp->h);
      } else {
         ed->inited = 1;
         new_image(ed);
         ed->bmp2 = create_bitmap(bmp->w, bmp->h);
         clear_to_color(ed->bmp2, makecol(255,255,255));
      }
   } else if (x < 0) {
   } else {
      switch (ed->mode) {
      case PIXEL:
         one_point_tool(ed, x, y, putpixel);
         break;
      case FILL:
         one_point_tool(ed, x, y, floodfill);
         break;
      case LINE:
         two_points_tool(ed, x, y, line);
         break;
      case RECTANGLE:
         two_points_tool(ed, x, y, rect);
         break;
      }
      ed->curx = x;
      ed->cury = y;
      Refresh(ed->idpos);
   }
}

static void color_adjusted(void *data)
{
   t_edit *ed = data;

   ed->colors[ed->colind] = makecol(ed->r, ed->g, ed->b);
   clear_to_color(ed->bmps[ed->colind], ed->colors[ed->colind]);
   Refresh(ed->colid);
}

static void new_color_selection(void *data)
{
   t_edit *ed = data;

   ed->r = getr(ed->colors[ed->colind]);
   ed->g = getg(ed->colors[ed->colind]);
   ed->b = getb(ed->colors[ed->colind]);
   Refresh(ed->palid);
}

static void destroy_palette_bitmaps(t_edit *ed)
{
   int i;
   for (i=0; i<NR_COLORS; i++) {
      if (ed->bmps[i]) {
         destroy_bitmap(ed->bmps[i]);
         ed->bmps[i] = NULL;
      }
   }
}

static void close_palette(void *data)
{
   t_edit *ed = data;
   destroy_palette_bitmaps(ed);
   ed->palid = 0;
   CloseWin(NULL);
}

static void open_palette(void *data)
{
   t_edit *ed = data;
   char s[10];
   int i, id;

   if (ed->palid) {
      SetFocusOn(ed->palid);
   } else {
      ed->palid = MkDialogue(ADAPTIVE, "Palette", W_FLOATING);

      ed->slid = StartContainer(TOPLEFT, ADAPTIVE, "", 0);
      id = AddEditBox(DOWNLEFT|ALIGNRIGHT, 30, "", FINT, 0, &ed->r);
      id = AddSlider(LEFT, 100, &ed->r, 0, 255, SL_SCALE|SL_LABEL, id);
      AddHandler(id, color_adjusted, ed);
      AddTag(LEFT, "Red:");

      id = AddEditBox(DOWNLEFT|ALIGNRIGHT, 30, "", FINT, 0, &ed->g);
      id = AddSlider(LEFT, 100, &ed->g, 0, 255, SL_SCALE|SL_LABEL, id);
      AddHandler(id, color_adjusted, ed);
      AddTag(LEFT, "Green:");

      id = AddEditBox(DOWNLEFT|ALIGNRIGHT, 30, "", FINT, 0, &ed->b);
      id = AddSlider(LEFT, 100, &ed->b, 0, 255, SL_SCALE|SL_LABEL, id);
      AddHandler(id, color_adjusted, ed);
      AddTag(LEFT, "Blue:");
      EndContainer();

      ed->colid = MkRadioContainer(DOWNLEFT, &ed->colind, R_HORIZONTAL);
      for (i=0; i<NR_COLORS; i++) {
         sprintf(s, "#c%d", i);
         ed->bmps[i] = create_bitmap(20,20);
         if (ed->bmps[i]) {
            clear_to_color(ed->bmps[i], ed->colors[i]);
            RegisterImage(ed->bmps[i], s+1, IMAGE_BMP, ed->palid);
            id = AddRadioButton(s);
            AddHandler(id, new_color_selection, ed);
         }
      }
      EndRadioContainer();
      AddButton(DOWNLEFT, "Close", close_palette, ed);
      DisplayWin();
   }
}

static void save_image_as(void *data)
{
   t_edit *ed = data;
   const char *path;

   path = FileSelect("*.pcx;*.bmp;*.tga", "", FS_BROWSE_DAT|FS_SHOW_MENU|FS_WARN_EXISTING_FILE|FS_SAVE_AS, "Save image as", "Save");
   if (*path) {
      if (ed->path)
         Release(ed->path);
      ed->path = MkString(path);
      save_image(ed);
   }
}

static void save_image(void *data)
{
   t_edit *ed = data;
   int error;

   if (ed->path == NULL)
      save_image_as(ed);
   else if (ed->sbmp->w > 0 && ed->sbmp->h > 0) {
      if (strchr(ed->path, '#'))
         error = !SaveDatafileObject(ed->path, ed->sbmp, DAT_BITMAP);
      else
         error = save_bitmap(ed->path, ed->sbmp, NULL);
      if (error)
         Req("", "Saving failed| OK ");
      else
         ed->upd = 0;
   }
}

static void load_image(void *data)
{
   t_edit *ed = data;
   PALETTE pal;
   const char *path;
   char *fn, *datname;
   int c = 0;
   BITMAP *bmp = NULL;
   DATAFILE *datob = NULL;

   check_save(ed);
   if (ed->bmp2 == NULL)
      return;
   path = FileSelect("*.pcx;*.bmp;*.tga", "", FS_BROWSE_DAT|FS_SHOW_MENU, "Load image", "Load");
   if (*path) {
      if (ed->path)
         Release(ed->path);
      ed->path = MkString(path);
      datname = strchr(ed->path, '#');
      if (datname) {
         fn = get_filename(datname+1);
         c = fn[-1];
         fn[-1] = 0;
         datob = load_datafile_object(ed->path, fn);
         if (datob) {
            if (c)
               fn[-1] = c;
            if (datob->type == DAT_BITMAP)
               bmp = datob->dat;
         }
      } else
         bmp = load_bitmap(ed->path, pal);
      if (bmp) {
         if (ed->sbmp)
            destroy_bitmap(ed->sbmp);
         clear_to_color(ed->cbmp, makecol(255,255,255));
         ed->sbmp = create_sub_bitmap(ed->cbmp, 0, 0, bmp->w, bmp->h);
         if (ed->sbmp)
            blit(bmp, ed->sbmp, 0, 0, 0, 0, bmp->w, bmp->h);
         blit(ed->cbmp, ed->bmp2, 0, 0, 0, 0, ed->cbmp->w, ed->cbmp->h);
         hline(ed->cbmp, 0, bmp->h, bmp->w + 1, makecol(0,0,0));
         vline(ed->cbmp, bmp->w, 0, bmp->h, makecol(0,0,0));
         Refresh(ed->id);
         ed->upd = 0;
      } else {
         Req("", "The image couldn't be loaded| OK ");
      }
      if (datob)
         unload_datafile_object(datob);
      else if (bmp)
         destroy_bitmap(bmp);
   }
}

static void quit(void *data)
{
   t_edit *ed = data;
   check_save(ed);
   StopProcessEvents();
}

static void show_code(void *data nouse)
{
   int id;
   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/21canvas.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "~Close", CloseWin, NULL);
   DisplayWin();
}

static void show_info(void *data nouse)
{
   MkDialogue(ADAPTIVE, "Example information", W_FLOATING);
   AddTextBox(DOWNLEFT, "This is not an image editor. It is an example that "
   "shows a likely way to use the \"canvas-object\". You can compare this "
   "to some of the games 2-6, which uses it in a slightly different way._"
   "1) Load some image_2) Select some tool from the menu_3) Choose some colour "
   "from the palette after opening the palette window._4) Use left button._"
   "5) Select \"Save as\" from the menu._6) Create a datafile and open it._"
   "7) Enter a name for your image and save._8) Select load from the menu "
   "again to verify that it was saved! Isn't it great?_ _So this was also an "
   "example of usage of the file-selector." , 400, 0, TB_FRAMESINK|TB_LINEFEED_);
   AddButton(DOWNLEFT, "~Close", CloseWin, NULL);
   DisplayWin();
}

static void edit_menu(void *data)
{
   t_edit *ed = data;
   MkMenuRadio(&ed->mode, 4, "Pixel", "Fill", "Line", "Rectangle");
}

static void file_menu(void *data)
{
   MkMenuItem(0, "New",  "Ctrl-N", new_image, data);
   MkMenuItem(0, "Load", "Ctrl-L", load_image, data);
   MkMenuItem(0, "Save", "Ctrl-S", save_image, data);
   MkMenuItem(0, "Save as", "",    save_image_as, data);
   MkMenuItem(0, "Exit", "Ctrl-Q", quit, data);
}

static void windows_menu(void *data)
{
   MkMenuItem(0, "Open palette", "", open_palette, data);
   MkMenuItem(0, "Show code", "", show_code, NULL);
   MkMenuItem(0, "Example info", "", show_info, NULL);
}

int main(void)
{
   t_edit *edit;
   int i;

   edit = GetMem0(t_edit, 1);
   InitCgui(1024, 768, 15);
   edit->idwin = MkDialogue(FILLSCREEN, "Canvas usage", 0);
   MakeMenuBar();
   MkMenuBarItem("File", file_menu, edit);
   MkMenuBarItem("Edit", edit_menu, edit);
   MkMenuBarItem("Windows", windows_menu, edit);
   EndMenuBar();
   edit->idpos = StartContainer(DOWNLEFT, ADAPTIVE, "", 0);
   AddEditBox(DOWNLEFT, 30, "x", FINT, 3, &edit->x);
   AddEditBox(RIGHT, 30, "y", FINT, 3, &edit->y);
   AddEditBox(RIGHT, 30, "x", FINT, 3, &edit->curx);
   AddEditBox(RIGHT, 30, "y", FINT, 3, &edit->cury);
   EndContainer();
   edit->id = MkCanvas(DOWNLEFT | FILLSPACE | VERTICAL | HORIZONTAL, 200, 100,
                canvas_event_handler, edit);
   DisplayWin();
   SetHotKey(0, quit, edit, KEY_Q, KEY_Q);
   SetHotKey(0, save_image, edit, KEY_S, KEY_S);
   SetHotKey(0, load_image, edit, KEY_L, KEY_L);
   ProcessEvents();
   if (edit->bmp2)
      destroy_bitmap(edit->bmp2);
   if (edit->sbmp)
      destroy_bitmap(edit->sbmp);
   destroy_palette_bitmaps(edit);
   if (edit->path)
      Release(edit->path);
   Release(edit);
   return 0;
}
END_OF_MAIN()
