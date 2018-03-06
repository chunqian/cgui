/* Module OBTEXT.C
   Contains functions for creating a textobject. A text object is a simple
   object displaying text. The text will be broken into rows of a length that
   fit the specified width. */

#include <string.h>
#include <allegro.h>
#include <ctype.h>

#include "cgui.h"
#include "cgui/mem.h"

#include "obtext.h"
#include "mouse.h"
#include "mstates.h"
#include "window.h"
#include "node.h"
#include "event.h"
#include "list.h"
#include "font.h"
#include "object.h"
#include "obtext.ht"
#include "labldata.h"
#include "id.h"

#define CHUNK_SIZE 1000
#define ISSPACE(c) ((c)==' ' || (c)=='\n' || (c)=='\r' || (c)=='\t')

#if ALLEGRO_SUB_VERSION == 0
#define textout_ex(bmp, f, s, x, y, col, bgcol) textout(bmp, f, s, x, y, col)
#define set_clip_rect set_clip
#endif

#if (ALLEGRO_SUB_VERSION < 2) || ((ALLEGRO_SUB_VERSION == 2) && (ALLEGRO_WIP_VERSION == 0))
#define file_size_ex file_size
#endif

static int fetching_in_progress = 0;
static const char *new_string;

typedef struct t_hilightsection {
   int bgrowstart;
   int bgcolumnstart;
   int bgrowend;
   int bgcolumnend;
   int bgcolor;
} t_hilightsection;

typedef struct t_textob {
   struct t_typefun *tf;
   int width;
   int frame;
   int preformat;
   int lf_;
   int tabw;
   char **page;
   char *s;
   int browsed;
   int n;
   int preset_height;
   int rh;
   int fixfont;
   int load;
   struct t_object *b;
   int sti;
   int pos;
   int background_color;
   RGB rgb;
   int focus_end;
   int br;
   int dirty;

   /* For the highlighting. */
   int hilite;
   int hrow;
   int hilite_background_color;
   int hilite_text_color;

   /* For scrolling with the mouse wheel. */
   int over_event_id;
   int prevz;

   /* Section highlightings. */
   int nr_hilights;
   t_hilightsection *hilights;

   void (*app_call_back) (void *data);
   void *appdata;

} t_textob;

static void TbPrepareNewScreenMode(t_object *b)
{
   t_textob *to;

   to = b->appdata;
   to->rgb.r = getr(to->background_color);
   to->rgb.g = getg(to->background_color);
   to->rgb.b = getb(to->background_color);
}

static void TbNewScreenMode(t_object *b)
{
   t_textob *to;

   to = b->appdata;
   to->background_color = makecol(to->rgb.r, to->rgb.r, to->rgb.r);
}

/* GetRow This function tells how long part of the string s that fits into a
   specified "width". The substring will end between two words if possible.
   It can be used when creating pages of rows from a textstring that may be
   longer than the available width to draw on. s - a string containing any
   text width - specifies the maximum width of the substring

   Return value: a pointer that tells where the end of the string should be
   to fit into a row of the specified width. Note! This implies that the
   character pointed to may be one of the following: - a blank - the string
   terminator (the entire string s fits into one row) - any character (error
   case: the first word in s is longer than the specified row). */
extern char *GetRow(FONT *f, char *s, int width)
{
   char *p, tmp;
   int len, i, h;

   len = text_length(f, s);
   i = strlen(s);
   h = i >> 1;
   if (len > width) {
      while (h) {
         if (len > width)
            i -= h;
         else
            i += h;
         tmp = s[i];
         s[i] = 0;
         len = text_length(f, s);
         s[i] = tmp;
         h >>= 1;
      }
      while (len > width && i) {
         i--;
         tmp = s[i];
         s[i] = 0;
         len = text_length(f, s);
         s[i] = tmp;
      }
      for (p = s + i; p > s; p--)
         if (*p == ' ')
            break;
      if (p == s) {
         if (i <= 0)
            i = 1;
         p = s + i;
      }
   } else {
      if (i > 0)
         return s + i;
      return s + 1;
   }
   if (p<=s)
      return s+1;
   return p;
}

static char *GetRowLineFeed(FONT *f, char *s, int width)
{
   char *p, *q;

   p = strchr(s, '\n');
   if (p) {
      *p = 0;
      if (text_length(f, s) < width) {
         *p = '\n';
         return p;
      }
      q = GetRow(f, s, width);
      *p = '\n';
      return q;
   } else
      return GetRow(f, s, width);
}

static char *GetPreformatRow(char *s)
{

   while (*s && *s != '\n' && *s != '\r')
      s++;
   return s;
}

/* DestroyPage This function returns the memory of one text-page created by
   the function MakePage. Parameter page: a pointer to the page */
extern void DestroyPage(char **page)
{
   char **p;

   if (page) {
      for (p = page; *p; Release(*p++));
      Release(page);
   }
}

static void FreeTO(t_object * b)
{
   t_textob *to;
   void (*Free) (t_object *);

   to = b->appdata;
   Free = to->tf->Free;
   DestroyPage(to->page);
   Release(to->s);
   if (to->hilights) {
      Release(to->hilights);
   }
   Release(to);
   Free(b);
}

/* MakePage This function returns an array of string pointers to rows that
   fits into the specified page width. DestroyPage can be used to release
   the memory when no longer needed.
   A NULL-pointer is used to indicate the end of string pointers.
   s - a string containing any text
   width - specifies the width (in pixels) that the text must to fit to.
   NB! return value i NULL if no s is an empty string! */
extern char **MakePage(FONT *f, char *s, int width, int pf, int lf, int *n)
{
   char **p = NULL, *end, *r;
   int rows, len;

   for (rows = 0; *s; s = end) {
      p = ResizeMem(char *, p, ++rows);
      if (pf)
         end = GetPreformatRow(s);
      else if (lf)
         end = GetRowLineFeed(f, s, width);
      else
         end = GetRow(f, s, width);
      len = end - s;
      p[rows - 1] = r = GetMem(char, len + 1);
      strncpy(r, s, len);
      r[len] = 0;
      if (pf) {
         if (*end) {
            if ((end[1] == '\r' || end[1] == '\n') && end[1] != *end)
               end++;
            end++;
         }
      } else if (lf) {
         if (*end == '\n')
            end++;
         while (*end == ' ')
            end++;
      } else {
         while (ISSPACE(*end))
            end++;
      }
   }
   *n = rows;
   rows++;
   p = ResizeMem(char *, p, rows);
   p[rows - 1] = NULL;
   return p;
}

static void DrawPage(t_object *b)
{
   t_textob *to;
   char **p;
   int y, clip_x2, clip_y2, fixw = 0, highlightcolor = -1, text_color, row_index;
   BITMAP *bmp;
   int x1, y1, x2, y2;
   int bgrowstart, bgrowend, bgcolumnstart, bgcolumnend;
   char *prow;
   int nchars;
   int saved_char;
   int i;
   t_hilightsection *hilightsection;

   bmp = b->parent->bmp;
   if (bmp == NULL)
      return;
   to = b->appdata;

   rectfill(bmp, b->x1, b->y1, b->x2, b->y2, to->background_color);
   if (to->frame == TB_FRAMESINK)
      DrawFrameSink(bmp, b->x1, b->y1, b->x2, b->y2);
   else if (to->frame == (int)TB_FRAMERAISE)
      DrawFrameRaise(bmp, b->x1, b->y1, b->x2, b->y2);
   clip_x2 = bmp->cr;
   clip_y2 = bmp->cb;
   if (to->fixfont)
      fixw = text_length(b->font, "W");
   /* 3 pixels for  the frame */
   set_clip_rect(bmp, bmp->cl, bmp->ct, b->x2 - 3, b->y2 - 3);
   if (to->hilite) {
      /* Mark the background of a certain row (old feature kept for compability). */
      rectfill(bmp, b->x1+1, b->y1+(to->hrow*to->rh)+3, b->x2, b->y1+(to->hrow*to->rh+to->rh)+2, to->hilite_background_color);
   }

   for (i = 0; i < to->nr_hilights; ++i) {
      /* Mark one section of the text. Start and end points are
         expressed in number of characters, so we need to compute the 
         coordinates from that, which is dependent on the current font. */
      hilightsection = to->hilights + i;
      bgrowstart = hilightsection->bgrowstart;
      bgrowend = hilightsection->bgrowend;
      bgcolumnstart = hilightsection->bgcolumnstart;
      bgcolumnend = hilightsection->bgcolumnend;
      if (bgrowstart < to->n) {
         prow = to->page[bgrowstart];
         nchars = strlen(prow);
         if (bgcolumnstart > nchars) {
            bgcolumnstart = nchars;
         }
         /* Compute the length of the text before the start position in the 
            string to get the start x-coordinate. */
         saved_char = prow[bgcolumnstart];
         prow[bgcolumnstart] = 0;
         x1 = b->x1 + text_length(b->font, prow) + 1;
         prow[bgcolumnstart] = saved_char;
         /* Draw the rows of the section to mark one by one.
            The end point may be either on the start row or any later row. 
            We need to preset the start y-coordinate to the row before the
            start row in order to include the frame offset. */
         y1 = b->y1 + (bgrowstart - 1 - to->sti) * to->rh + 3;
         while (bgrowstart < to->n) {
            /* Compute the start and end y-coordinate using that we know the 
               height of rows. */
            y1 += to->rh;
            y2 = y1 + to->rh - 1;
            if (bgrowstart == bgrowend) {
               /* The current row is where the end point exists. */

               /* Limit the end of the high light to the end of the text. */
               prow = to->page[bgrowend];
               nchars = strlen(prow);
               if (bgcolumnend > nchars) {
                  bgcolumnend = nchars;
               }

              /* Compute the length of the text before the end position in the
                 string to get the end x-coordinate. */
               saved_char = prow[bgcolumnend];
               prow[bgcolumnend] = 0;
               x2 = b->x1 + text_length(b->font, prow) + 1;
               prow[bgcolumnend] = saved_char;
               /* Break the iteration after the last portion of the section is marked. */
               bgrowstart = to->n;
            } else {
               x2 = b->x2;
            }
            rectfill(bmp, x1, y1, x2, y2, hilightsection->bgcolor);
            x1 = b->x1 + 1;
            ++bgrowstart;
         }
      }
   }

   row_index = 0;
   for (y = b->y1 + 3, p = to->page + to->sti; *p && y < b->y2; p++, y += to->rh) {
      if (row_index == to->hrow && to->hilite) {
         text_color = to->hilite_text_color;
      } else {
         text_color = cgui_colors[CGUI_COLOR_TEXTBOX_TEXT];
      }
      if (to->fixfont) {
         FixTextOut(bmp, b->font, *p, b->x1 + 2, y, text_color, to->background_color, fixw);
      } else {
         textout_ex(bmp, b->font, *p, b->x1 + 2, y, text_color, -1);
      }
      row_index++;
   }
   set_clip_rect(bmp, bmp->cl, bmp->ct, clip_x2, clip_y2);
   if (to->dirty) {
      to->dirty = 0;
      Refresh(to->br);
   }
}

static void SetSize(t_object *b)
{
   int h, width, height;
   char **p;
   t_textob *to;

   to = b->appdata;
   h = text_height(b->font);

   if (to->page)
      DestroyPage(to->page);
   p = to->page = MakePage(b->font, to->s, to->width + b->rex - 4,
                to->preformat, to->lf_, &to->n);
   if (b->dire)
      b->x1 = b->y1 = 0;
   b->x2 = b->x1 + to->width - 1 + 4 + b->rex;
   if (to->browsed) { /* A browsable box */
      b->y2 = b->y1 + to->preset_height + 6 - 1 + b->rey;
      h = b->y2 - b->y1 + 1;
      if (to->focus_end) {
         to->sti = to->n - h / text_height(b->font); /* start index */
         if (to->sti < 0)
            to->sti = 0;
      } else
         to->sti = (double)to->pos / to->rh + 0.5;
      SetBrowserSize(to->br, h-6, h);
      NotifyBrowser(to->br, to->rh, to->n*to->rh);
   } else {
      b->y2 = b->y1 + to->n * h - 1 + 6 + b->rey;
      if (b->y2 < b->y1)
         b->y2 = b->y1;
      width = b->parent->ob->x2 - b->parent->ob->x1 + 1;
      height = b->parent->ob->y2 - b->parent->ob->y1 + 1;
      if (b->x2 > width - 1 + DEFAULT_WINFRAME) {
         width = b->x2 + 1 + DEFAULT_WINFRAME;
         b->parent->ob->x2 = width + b->parent->ob->x1 - 1;
      }
      if (b->y2 > height - 1 + DEFAULT_WINFRAME) {
         height = b->y2 + 1 + DEFAULT_WINFRAME;
         b->parent->ob->y2 = height + b->parent->ob->y1 - 1;
      }
   }
}

static void CopyPreformattedText(t_textob *to, const char *s)
{
   int x = 0, n = 0, space, nsp, i;
   char *d;

   space = strlen(s) + CHUNK_SIZE;
   d = GetMem(char, space);
   while (*s) {
      if (*s == '\t') {
         nsp = to->tabw - (x % to->tabw);
         if (n + nsp + 1 > space) {
            space += CHUNK_SIZE;
            d = ResizeMem(char, d, space);
         }
         for (i = 0; i < nsp; i++)
            d[n++] = ' ';
         x += nsp;
         s++;
      } else {
         if (*s == '\n' || *s == '\r')
            x = 0;
         else
            x++;
         d[n++] = *s++;
         if (n + 1 > space) {
            space += CHUNK_SIZE;
            d = ResizeMem(char, d, space);
         }
      }
   }
   d[n] = 0;
   to->s = d;
}

static void CopyText(t_textob *to, const char *s)
{
   char *d;

   d = to->s = GetMem(char, strlen(s) + 1);
   while (ISSPACE(*s))
      s++;
   while (*s) {
      if (ISSPACE(*s)) {
         while (ISSPACE(*s))
            s++;
         if (*s)
            *d++ = ' ';
      }
      if (*s) {
         if (*s == '_' && to->lf_) {
            s++;
            if (*s == '_') {
               *d++ = *s++;
            } else {
               *d++ = '\n';
            }
         } else {
            *d++ = *s++;
         }
      }
   }
   *d = 0;
}

static void DelayedErrorMessage(void *data)
{
   char *s = data;

   Req("", s);
   Release(s);
}

static void GenerateTextLayout(t_object *b, const char *new_string)
{
   t_textob *to;
   int h;
   int size;
   char *mem = NULL;
   PACKFILE *fp;

   to = b->appdata;
   DestroyPage(to->page);
   Release(to->s);

   if (to->load && *new_string) {
      fp = pack_fopen(new_string, "rt");
      if (fp) {
         size = file_size_ex(new_string);
         new_string = mem = GetMem(char, size + 1);
         pack_fread(mem, size, fp);
         pack_fclose(fp);
         mem[size] = 0;
      } else {
         const char *const*txt;
         int nr;
         txt = LoadCompiledTexts(cgui_labels+CGUI_CGUILABL, "obtext", &nr);
         if (nr == SIZE_obtext) {
            char *tmp = GetMem(char, strlen(new_string) + strlen(txt[ERROR_OPENING_FILE_NN_OK]) + 1);
            sprintf(tmp, txt[ERROR_OPENING_FILE_NN_OK], new_string);
            _GenEventOfCgui(DelayedErrorMessage, tmp, 0, 0);
         }
      }
   }
   if (to->preformat)
      CopyPreformattedText(to, new_string);
   else
      CopyText(to, new_string);
   if (mem)
      Release(mem);
   to->page = MakePage(b->font, to->s, to->width + b->rex - 4, to->preformat, to->lf_, &to->n);
   h = b->y2 - b->y1 + 1;
   if (to->focus_end) {
      to->sti = to->n - h / text_height(b->font); /* start index */
      if (to->sti < 0)
         to->sti = 0;
   } else {
      to->sti = 0;
   }
   to->pos = to->sti*to->rh;
   if (to->browsed) {
      SetBrowserSize(to->br, h-6, h);
      NotifyBrowser(to->br, to->rh, to->n*to->rh);
   }
}

static int FetchTextBoxText(t_object *b)
{
   t_textob *to;

   to = b->appdata;
   if (fetching_in_progress) {
      fetching_in_progress = 0;
      GenerateTextLayout(b, new_string);
      to->dirty = 1;
      return 1;
   }
   return 0;
}

static void BrCallBack(void *data)
{
   t_textob *to = data;

   to->sti = to->pos / to->rh;
   to->b->tf->Refresh(to->b);
}

static int TextMoveFocusToNextSubObject(t_object *nf nouse, t_object *sf, int scan, int ascii)
{
   t_textob *to;
   int pagesize;
   (void)ascii;

   to = sf->appdata;
   switch (scan) {
   case KEY_DOWN:
      if (to->sti < to->n - 1) {
         to->sti++;
         to->b->tf->Refresh(to->b);
         to->pos = to->sti*to->rh;
         NotifyBrowser(to->br, to->rh, to->n*to->rh);
         Refresh(to->br);
      }
      break;
   case KEY_UP:
      if (to->sti > 0) {
         to->sti--;
         to->b->tf->Refresh(to->b);
         to->pos = to->sti*to->rh;
         NotifyBrowser(to->br, to->rh, to->n*to->rh);
         Refresh(to->br);
      }
      break;
   case KEY_PGUP:
      pagesize = (sf->y2 - sf->y1 + sf->rey) / text_height(sf->font);
      to->sti -= pagesize;
      if (to->sti < 0)
         to->sti = 0;
      to->b->tf->Refresh(to->b);
      to->pos = to->sti*to->rh;
      NotifyBrowser(to->br, to->rh, to->n*to->rh);
      Refresh(to->br);
      break;
   case KEY_PGDN:
      pagesize = (sf->y2 - sf->y1 + sf->rey) / text_height(sf->font);
      to->sti += pagesize;
      if (to->sti >= to->n)
         to->sti = to->n - 1;
      to->b->tf->Refresh(to->b);
      to->pos = to->sti*to->rh;
      NotifyBrowser(to->br, to->rh, to->n*to->rh);
      Refresh(to->br);
      break;
   default:
      return 0;
   }
   return 1;
}

static void Stretch(t_object *b, int dx, int dy)
{
   int h;
   t_textob *to;

   to = b->appdata;
   b->rex += dx;
   b->rey += dy;
   h = b->y2 + b->rey - b->y1 + 1;
   SetBrowserSize(to->br, h, h);
}

static void HandleMouseWheel(void *data)
{
   t_object *b=data;
   t_textob *to;
   int x, y, z, diff;

   to = b->appdata;
   _CguiMousePos(&x, &y, &z);
   diff = z - to->prevz;
   if (diff != 0) {
      to->prevz = z;
      to->sti -= diff;
      if (to->sti > to->n - 1) {
         to->sti = to->n - 1;
      } else if (to->sti < 0) {
         to->sti = 0;
      }
      Refresh(b->id);
      to->pos = to->sti*to->rh;
      NotifyBrowser(to->br, to->rh, to->n*to->rh);
      Refresh(to->br);
   }
   to->over_event_id = _GenEventOfCgui(HandleMouseWheel, b, 100, b->id);
}

static void OverTextBox(t_object *b)
{
   t_textob *to;
   int mx, my, mz;

   to = b->appdata;
   Refresh(b->id);
   if (b->state == MS_OVER) {
      _CguiMousePos(&mx, &my, &mz);
      to->prevz = mz;
      HandleMouseWheel(b);
   } else {
      _KillEventOfCgui(to->over_event_id);
   }
}

static void HandlerWapper(void *data)
{
   t_textob *to = data;
   to->app_call_back(to->appdata);
}

static int TBAddHandler(t_object *b, void (*Handler) (void *data), void *data)
{
   t_textob *to;
   b->Action = HandlerWapper;
   to = b->appdata;
   to->app_call_back = Handler;
   to->appdata = data;
   return 1;
}

/* Application interface: */

extern int AddTextBox(int x, int y, const char *s, int w, int n, int option)
{
   t_textob *to;
   t_object *b;
   struct t_node *nd = NULL;
   static t_typefun tf;
   static t_tcfun tc;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      tf = default_type_functions;
      tf.Free = FreeTO;
      tf.Draw = DrawPage;
      tf.SetSize = SetSize;
      tf.AddHandler = TBAddHandler;
      tf.Stretch = Stretch;
      tf.FetchData = FetchTextBoxText;
      tf.NewScreenMode = TbNewScreenMode;
      tf.PrepareNewScreenMode = TbPrepareNewScreenMode;
      tc = default_master_tc_functions;
      tc.MoveFocusToNextSubObject = TextMoveFocusToNextSubObject;
   }
   if (n) { /* Prepare for browser */
      nd = MkNode(opwin->win->opnode, x, y, ADAPTIVE, StubOb);
      b = CreateObject(TOPLEFT, nd);
   } else
      b = CreateObject(x, y, opwin->win->opnode);
   to = GetMem0(t_textob, 1);
   to->tf = b->tf;
   to->frame = option & (TB_FRAMERAISE | TB_FRAMESINK);
   if (option & TB_WHITE) {
      to->background_color = makecol(255, 255, 255);
   } else {
      to->background_color = cgui_colors[CGUI_COLOR_TEXTBOX_BACKGROUND];
   }
   to->preformat = option & TB_PREFORMAT;
   to->lf_ = option & TB_LINEFEED_;
   to->load = option & TB_TEXTFILE;
   to->focus_end = option & TB_FOCUS_END;
   if (option & 0xffff)
      to->tabw = option & 0xffff;
   else
      to->tabw = 8;
   to->fixfont = option & TB_FIXFONT;
   if (to->fixfont)
      b->font = GetCguiFixFont();
   to->width = w;
   to->rh = text_height(b->font);
   to->preset_height = to->rh * n;
   to->browsed = n;
   to->b = b;
   b->tf = &tf;
   b->Action = Stub;
   b->appdata = to;
   GenerateTextLayout(b, s);
   if (to->browsed) {
      b->Over = OverTextBox;
      to->br = MkVerticalBrowser(RIGHT|FILLSPACE|VERTICAL, BrCallBack, to, &to->pos);
      nd->ob->tf->DoJoinTabChain(nd->ob);
      nd->ob->tcfun = &tc;
      CloseNode();
   }
   return b->id;
}

extern int UpdateTextBoxText(int id, const char *s)
{
   t_object *b;
   int ok = 0;

   b = GetObject(id);
   if (b) {
      if (b->tf->Draw == DrawPage) {
         fetching_in_progress = 1;
         new_string = s;
         b->tf->FetchData(b);
         if (fetching_in_progress) {
            fetching_in_progress = 0;
         } else {
            ok = 1;
         }
      }
   }
   return ok;
}

extern void TextboxHighlighting(int id, int hilite_background_color, int hilite_text_color, int line_nr)
{
   t_object *b;
   t_textob *to;

   b = GetObject(id);
   if (b) {
      if (b->tf->Draw == DrawPage) {
         to = b->appdata;
         to->hilite = 1;
         to->hrow = line_nr;
         to->hilite_text_color = hilite_text_color;
         to->hilite_background_color = hilite_background_color;
      }
   }
}

extern int AddTextboxHighlightSection(int id, int bgcolor, int startrow, int startcolumn, int endrow, int endcolumn)
{
   t_object *b;
   t_textob *to;
   t_hilightsection *hilightsection;
   int ok = 0;

   b = GetObject(id);
   if (b) {
      if (b->tf->Draw == DrawPage) {
         ok = 1;
         to = b->appdata;
         ++to->nr_hilights;
         to->hilights = ResizeMem(t_hilightsection, to->hilights, to->nr_hilights);
         hilightsection = to->hilights + to->nr_hilights -1;
         hilightsection->bgrowstart = startrow;
         hilightsection->bgcolumnstart = startcolumn;
         hilightsection->bgrowend = endrow;
         hilightsection->bgcolumnend = endcolumn;
         hilightsection->bgcolor = bgcolor;
      }
   }
   return ok;
}

extern int ClearTextboxHighlightSections(int id)
{
   t_object *b;
   t_textob *to;
   int ok = 0;

   b = GetObject(id);
   if (b) {
      if (b->tf->Draw == DrawPage) {
         ok = 1;
         to = b->appdata;
         if (to->nr_hilights > 0) {
            Release(to->hilights);
            to->nr_hilights = 0;
            to->hilights = NULL;
         }
      }
   }
   return ok;
}

extern int ClearLastTextboxHighlightSection(int id)
{
   t_object *b;
   t_textob *to;
   int ok = 0;

   b = GetObject(id);
   if (b) {
      if (b->tf->Draw == DrawPage) {
         ok = 1;
         to = b->appdata;
         if (to->nr_hilights > 0) {
            --to->nr_hilights;
         }
      }
   }
   return ok;
}

extern int TextboxGetTopRow(int id)
{
   t_object *b;
   t_textob *to;

   b = GetObject(id);
   if (b) {
      if (b->tf->Draw == DrawPage) {
         to = b->appdata;
         return to->sti;
      }
   }
   return -1;
}

extern const char *TextboxGetHighlightedText(int id)
{
   t_object *b;
   t_textob *to;
   int i;
   b = GetObject(id);
   if (b) {
      if (b->tf->Draw == DrawPage) {
         to = b->appdata;
         i = to->sti + to->hrow;
         if (i<to->n) {
            return to->page[i];
         }
      }
   }
   return NULL;
}

extern void TextboxScrollDownOneLine(int id)
{
   t_object *b;
   t_textob *to;

   b = GetObject(id);
   if (b) {
      if (b->tf->Draw == DrawPage) {
         to = b->appdata;
         if (to->sti < to->n - 1) {
            to->sti++;
            to->b->tf->Refresh(to->b);
            to->pos = to->sti*to->rh;
            NotifyBrowser(to->br, to->rh, to->n*to->rh);
            Refresh(to->br);
         }
      }
   }
}

/* Obsolete */
extern int MkTextObject(int x, int y, const char *s, int w,
                                      int sp nouse, int option)
{
   return AddTextBox(x, y, s, w, 0, option);
}
