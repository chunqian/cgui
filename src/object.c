/* Module OBJECT
   Contains functions operating on basic (generic) objects */

#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <allegro.h>
#include "cgui.h"
#include "cgui/mem.h"

#include "window.h"
#include "node.h"
#include "object.h"
#include "msm.h"
#include "rectlist.h"
#include "graphini.h"
#include "obsizer.h"
#include "obbutton.h"
#include "setpos.h"
#include "font.h"
#include "id.h"
#include "refresh.h"
#include "event.h"
#include "cursor.h"
#include "tabchain.h"
#include "filedat.h"
#include "files.h"

#define INIT_RECT_SIZE 50
#define BORDER_WIDTH 2

#if ALLEGRO_SUB_VERSION == 0
#define textout_ex(bmp, f, s, x, y, col, bgcol) textout(bmp, f, s, x, y, col)
#define set_clip_rect set_clip
#endif

static int tool_tip_delay = 0;
static int tool_tip_animation_step = 0;
static int tool_tip_animation_delay = 20;

t_object *_win_root;
int _cgui_hot_key_line;
int _cgui_button1_height = 16;
int hold_object_blitting = 0;
static int blitxmin, blitymin, blitxmax, blitymax;
int cgui_use_vsync = 0;
static int blit_clip_x1, blit_clip_y1, blit_clip_x2, blit_clip_y2,
           blit_clip_request = 0;
static int _cgui_parse_labels = 1;

static int NextWinToTop(t_object *b);
static void GetNextWin(t_object *b);
static t_object *GetMainNode(t_object *b);
static void DefaultSetSize(t_object *b);
static void DefaultStretcher(t_object *, int, int);
static int DefaultChangeData(t_object *b nouse, void *data nouse,
                             int type nouse);
static void DefaultActivate(t_object *b);
static void DefaultDeActivate(t_object *b);
static void DefaultUnlinkObject(t_object *b);
static void DefaultFree(t_object *b);
static void DefaultSetView(t_object *b, int flags);
static void DefaultRemove(t_object *b);
static int DefaultAddHandler(t_object *b, void (*Handler) (void *data),
                             void *data);
static int FetchData(t_object *b nouse) {return 0;}
static void DefaultCatchImage(struct t_object *b);
static void DefaultPrepareNewScreenMode(struct t_object *b nouse){}
static void DefaultNewScreenMode(struct t_object *b nouse){}
static t_coord *InsertAreaStub(t_object *b, t_coord *rects, int *n, int *m);
t_typefun default_type_functions = {
   DefaultSetSize,
   DefaultStretcher,
   DefaultCatchImage,
   DefaultChangeData,
   DefaultUnlinkObject,
   DefaultFree,
   DefaultRemove,
   DefaultActivate,
   DefaultDeActivate,
   StubOb,
   DefaultRefresh,
   DefaultAddHandler,
   FetchData,
   StubOb,
   StubOb,
   StubOb,
   GetNextWin,
   NextWinToTop,
   GetMainNode,
   MakeObjectStretchable,
   SingleDoJoinTabChain,
   InsertAreaStub,
   DefaultGetSizeOffset,
   DefaultSetSizeOffset,
   DefaultSetView,
   DefaultPrepareNewScreenMode,
   DefaultNewScreenMode,
   StubOb,
   StubOb
};

static int AsciiToScan(int ascii)
{
   switch (ascii) {
      case 'A': return KEY_A;
      case 'B': return KEY_B;
      case 'C': return KEY_C;
      case 'D': return KEY_D;
      case 'E': return KEY_E;
      case 'F': return KEY_F;
      case 'G': return KEY_G;
      case 'H': return KEY_H;
      case 'I': return KEY_I;
      case 'J': return KEY_J;
      case 'K': return KEY_K;
      case 'L': return KEY_L;
      case 'M': return KEY_M;
      case 'N': return KEY_N;
      case 'O': return KEY_O;
      case 'P': return KEY_P;
      case 'Q': return KEY_Q;
      case 'R': return KEY_R;
      case 'S': return KEY_S;
      case 'T': return KEY_T;
      case 'U': return KEY_U;
      case 'V': return KEY_V;
      case 'W': return KEY_W;
      case 'X': return KEY_X;
      case 'Y': return KEY_Y;
      case 'Z': return KEY_Z;
      case '0': return KEY_0;
      case '1': return KEY_1;
      case '2': return KEY_2;
      case '3': return KEY_3;
      case '4': return KEY_4;
      case '5': return KEY_5;
      case '6': return KEY_6;
      case '7': return KEY_7;
      case '8': return KEY_8;
      case '9': return KEY_9;
      case '~': return KEY_TILDE;
      case '-': return KEY_MINUS;
      case '=': return KEY_EQUALS;
      case '{': return KEY_OPENBRACE;
      case '}': return KEY_CLOSEBRACE;
      case ':': return KEY_COLON;
      case '\'': return KEY_QUOTE;
      case '\\': return KEY_BACKSLASH;
      case ',': return KEY_COMMA;
      case '.': return KEY_STOP;
      case '/': return KEY_SLASH;
      case ' ': return KEY_SPACE;
      case '*': return KEY_ASTERISK;
      case '+': return KEY_PLUS_PAD;
      case '@': return KEY_AT;
      case '^': return KEY_CIRCUMFLEX;
      case '`': return KEY_BACKQUOTE;
      case ';': return KEY_SEMICOLON;
   }
   return 0;
}

/* This function blits the specified area from the local window buffer onto
   the double-buffer (the screen image). If the object is covered by other
   window(s) it will be properly clipped. When this has been done, the same
   area (covered with whatever) will be blitted onto screen.
   a - the area to blit from expressed in the local node (i.e. nd) coordinates
   sc - the area to blit to expressed in screen coordinates
   nd - the node to blit from

   Procedure: first partition the area into rectangles. The rectangle list
   will contain the rectangles that constitutes the visible area of a
   (a may be more or less covererd by windows) for each rectangle: set
   clipping, and blit the entire area into the double buffer. When ready:
   blit it all onto screen */
extern void BlitObject(t_coord *a, t_coord *sc, t_node *nd)
{
   int n, i, width, height;     /* sx,sy is object position in screen

                                   coordinates */
   t_coord *rects, *r;
   t_object *ow;

   if (nd->bmp == NULL)
      return;
   width = a->x2 - a->x1 + 1;
   height = a->y2 - a->y1 + 1;
   ow = nd->win->node->ob;
   rects = PartitionArea(ow, sc, &n);
   for (i = 0, r = rects; i < n; i++, r++) {
      set_clip_rect(cgui_bmp, r->x1, r->y1, r->x2, r->y2);
      blit(nd->bmp, cgui_bmp, a->x1, a->y1, sc->x1, sc->y1, width, height);
   }
   set_clip_rect(cgui_bmp, 0, 0, SCREEN_W - 1, SCREEN_H - 1);
   if (hold_object_blitting) {
      blitxmax = MAX(blitxmax, sc->x1 + width);
      blitymax = MAX(blitymax, sc->y1 + height);
      blitxmin = MIN(blitxmin, sc->x1);
      blitymin = MIN(blitymin, sc->y1);
   } else {
      if (cgui_use_vsync)
         vsync();
      scare_mouse_area(MAX(0,sc->x1), MAX(0,sc->y1), width+10, height+10);
      blit(cgui_bmp, screen, sc->x1, sc->y1, sc->x1, sc->y1, width, height);
      unscare_mouse();
   }
   Release(rects);
}

extern void StopBlitting(void)
{
   if (hold_object_blitting == 0) {
      blitxmin = SCREEN_W;
      blitymin = SCREEN_H;
      blitxmax = 0;
      blitymax = 0;
   }
   hold_object_blitting++;
}

extern void ContinueBlitting(void)
{
   hold_object_blitting--;
   if (hold_object_blitting == 0 && blitxmin < blitxmax && blitymin < blitymax) {
      if (cgui_use_vsync)
         vsync();
      scare_mouse_area(blitxmin, blitymin, blitxmax - blitxmin, blitymax - blitymin);
      blit(cgui_bmp, screen, blitxmin, blitymin, blitxmin, blitymin, blitxmax - blitxmin, blitymax - blitymin);
      unscare_mouse();
   }
}

extern void DrawImage(t_image *im, BITMAP *bmp, int x, int y)
{
   BITMAP *imbmp;

   if (im->bmp) {
      switch (im->type) {
      case IMAGE_BMP:
         imbmp = im->bmp;
         blit(imbmp, bmp, 0, 0, x, y, imbmp->w, imbmp->h);
         break;
      case IMAGE_TRANS_BMP:
         draw_sprite(bmp, im->bmp, x, y);
         break;
      case IMAGE_RLE_SPRITE:
         draw_rle_sprite(bmp, im->bmp, x, y);
         break;
      case IMAGE_CMP_SPRITE:
         draw_compiled_sprite(bmp, im->bmp, x, y);
         break;
      }
   }
}

extern t_image *GetImage(t_node *nd, const char *imagename)
{
   t_image *im;

   if (nd == NULL)
      return NULL;
   for (im = nd->image; im; im = im->next) {
      if (strcmp(imagename, im->name) == 0)
         return im;
   }
   return GetImage(nd->ob->parent, imagename);
}

static void ConditionalDestroyImageData(t_image *image)
{
   if (!image->callers_data) {
      switch (image->type) {
      case IMAGE_TRANS_BMP:
      case IMAGE_BMP:
         destroy_bitmap(image->bmp);
         break;
      case IMAGE_RLE_SPRITE:
         destroy_rle_sprite(image->bmp);
         break;
      case IMAGE_CMP_SPRITE:
         destroy_compiled_sprite(image->bmp);
         break;
      }
   }
}

static void DestroyImage(t_image *image)
{
   ConditionalDestroyImageData(image);
   Release(image->name);
   Release(image->filename);
   Release(image);
}

/* Destroys a list of images */
extern void DestroyImages(t_image *image)
{
   if (image) {
      DestroyImages(image->next);
      DestroyImage(image);
   }
}

static t_image *UnlinkImage(t_image *image, t_image *unl)
{
   if (image) {
      if (image == unl)
         return image->next;
      else {
         image->next = UnlinkImage(image->next, unl);
         return image;
      }
   }
   return NULL;
}

static t_image *SetImageData(t_image *im, void *data, int w, int h, int type)
{
   if (im->bmp)
      ConditionalDestroyImageData(im);
   im->bmp = data;
   im->transp = type == IMAGE_TRANS_BMP;
   im->type = type;
   im->w = w;
   im->h = h;
   return im;
}

static t_image *CreateImage(const char *imname, void *data, int w, int h,
       const char *fn, int type, int callers_data, int subseq)
{
   t_image *im;

   im = GetMem0(t_image, 1);
   if (im->filename)
      Release(im->filename);
   im->filename = MkString(fn);
   if (im->name)
      Release(im->name);
   im->name = MkString(imname);
   im->transp = type == IMAGE_TRANS_BMP;
   im->callers_data = callers_data;
   im->subseq = subseq;
   return SetImageData(im, data, w, h, type);
}

static void InsertImage(t_image *im, t_node *nd)
{
   im->next = nd->image;
   nd->image = im;
}

static int CguiLoadImages(const char *fn, const char *name, int tr, t_node *nd)
{
   BITMAP *bmp = NULL, *org;
   RLE_SPRITE *rle;
   COMPILED_SPRITE *cmp;
   RGB pal[256];
   DATAFILE *df = NULL, *datl = NULL;
   int isdat, i, ok = 1;
   char *p, *delim, *end, *next;
   t_image *im;

   p = MkString(fn);
   fix_filename_slashes(p);
   delim = strchr(p, '#');
   if (delim)
      *delim = 0;
   isdat = stricmp(get_extension(p), "dat") == 0;
   if (isdat) {
      if (delim) {
         end = delim;
         do {
            next = end;
            end = EndOfFirstPart(next + 1);
         } while (*end);
         if (next != delim) {
            *delim = '#';
            *next = 0;
         }
         df = load_datafile_object(p, next + 1);
         if (df) {
            im = NULL;
            switch (df->type) {
            case DAT_FILE:
               datl = df->dat;
               df->dat = NULL;
               break;
            case DAT_BITMAP:
               bmp = df->dat;
               df->dat = NULL;
               im = CreateImage(next + 1, bmp, bmp->w, bmp->h, fn, tr, 0, 0);
               break;
            case DAT_RLE_SPRITE:
               rle = df->dat;
               df->dat = NULL;
               im = CreateImage(next + 1, rle, rle->w, rle->h, fn, IMAGE_RLE_SPRITE, 0, 0);
               break;
            case DAT_C_SPRITE:
               cmp = df->dat;
               df->dat = NULL;
               im = CreateImage(next + 1, cmp, cmp->w, cmp->h, fn, IMAGE_CMP_SPRITE, 0, 0);
               break;
            default:
               ok = 0;
               break;
            }
            if (im)
               InsertImage(im, nd);
            unload_datafile_object(df);
         }
      } else
         datl = load_datafile(fn);
      if (datl) {
         for (i=0; datl[i].type != DAT_END; i++) {
            if (datl[i].type == DAT_BITMAP || datl[i].type == DAT_RLE_SPRITE || datl[i].type == DAT_C_SPRITE) {
               name = get_datafile_property(datl + i, DAT_NAME);
               if (name && *name) {
                  im = NULL;
                  switch (datl[i].type) {
                  case DAT_BITMAP:
                     org = datl[i].dat;
                     bmp = create_bitmap(org->w, org->h);
                     if (bmp) {
                        blit(datl[i].dat, bmp, 0, 0, 0, 0, bmp->w, bmp->h);
                        im = CreateImage(name, bmp, bmp->w, bmp->h, fn, tr, 0, i);
                     }
                     break;
                  case DAT_RLE_SPRITE:
                     rle = datl[i].dat;
                     im = CreateImage(name, datl[i].dat, rle->w, rle->h, fn, IMAGE_RLE_SPRITE, 0, i);
                     datl[i].dat = NULL;
                     break;
                  case DAT_C_SPRITE:
                     cmp = datl[i].dat;
                     im = CreateImage(name, datl[i].dat, cmp->w, cmp->h, fn,  IMAGE_CMP_SPRITE, 0, i);
                     datl[i].dat = NULL;
                     break;
                  }
                  if (im)
                     InsertImage(im, nd);
               }
            }
         }
         if (datl)
            unload_datafile(datl);
      } else if (df == NULL)
         ok = 0;
   } else {
      bmp = load_bitmap(fn, pal);
      if (bmp) {
         im = CreateImage(name, bmp, bmp->w, bmp->h, fn, tr, 0, 0);
         InsertImage(im, nd);
      } else
         ok = 0;
   }
   Release(p);
   return ok;
}

/* Refreshes a list of images by destroying them and then re-loading them
   from disk */
extern void NewScreenModeImageList(t_image *im, t_node *nd)
{
   BITMAP *bmp;
   if (im) {
      NewScreenModeImageList(im->next, nd);
      if (im->callers_data) {
         InsertImage(im, nd);
      } else {
         if (*im->filename==0 && (im->type==IMAGE_BMP || im->type==IMAGE_TRANS_BMP)) {
            BITMAP *oldbmp;
            oldbmp = im->bmp;
            bmp = create_bitmap(oldbmp->w, oldbmp->h);
            blit(oldbmp, bmp, 0, 0, 0, 0, oldbmp->w, oldbmp->h);
            SetImageData(im, bmp, im->w, im->h, im->type);
            InsertImage(im, nd);
         } else if (*im->filename && !im->subseq) {
            CguiLoadImages(im->filename, im->name, im->type, nd);
         }
      }
   }
}

extern void NewScreenMode(void)
{
   _win_root->tf->NewScreenMode(_win_root);
}

extern void PrepareNewScreenMode(void)
{
   _win_root->tf->PrepareNewScreenMode(_win_root);
}

/* cd = callers data */
static int RegisterImage2(void *data, const char *imagename, int type, int id, int cd)
{
   t_node *nd;
   BITMAP *bmp;
   RLE_SPRITE *rle;
   COMPILED_SPRITE *cmp;
   int w, h;
   t_image *im;

   nd = GetNode(id);
   if (nd) {
      if (data) {
         im = GetImage(nd, imagename);
         switch (type) {
         case IMAGE_TRANS_BMP:
         case IMAGE_BMP:
            bmp = data;
            w = bmp->w;
            h = bmp->h;
            if (!cd) {
               data = create_bitmap(w, h);
               blit(bmp, data, 0, 0, 0, 0, w, h);
            }
            break;
         case IMAGE_RLE_SPRITE:
            rle = data;
            w = rle->w;
            h = rle->h;
            cd = 1;
            break;
         case IMAGE_CMP_SPRITE:
            cmp = data;
            w = cmp->w;
            h = cmp->h;
            cd = 1;
            break;
         default:
            return 0;
         }
         if (im) {
            if (im->callers_data == cd)
               SetImageData(im, data, w, h, type);
            else {
               nd->image = UnlinkImage(nd->image, im);
               DestroyImage(im);
               im = CreateImage(imagename, data, w, h, "", type, cd, 0);
               InsertImage(im, nd);
            }
         } else {
            im = CreateImage(imagename, data, w, h, "", type, cd, 0);
            InsertImage(im, nd);
         }
         return 1;
      } else {
         im = GetImage(nd, imagename);
         if (im) {
            nd->image = UnlinkImage(nd->image, im);
            DestroyImage(im);
         }
      }
   }
   return 0;
}

/* Registers an image `data' and keep a reference to that (callers data) */
extern int RegisterImage(void *data, const char *imagename, int type, int id)
{
   return RegisterImage2(data, imagename, type, id, 1);
}

/* Registers a copy of the image `data'. No refernece is saved */
extern int RegisterImageCopy(void *data, const char *imagename, int type, int id)
{
   return RegisterImage2(data, imagename, type, id, 0);
}

extern const void *GetRegisteredImage(const char *name, int *type, int id)
{
   t_image *im;
   t_node *nd = GetNode(id);

   if (nd) {
      im = GetImage(nd, name);
      if (im) {
         *type = im->type;
         return im->bmp;
      }
   }
   return NULL;
}

extern int CguiLoadImage(const char *filename, const char *imagename, int transp,
                           int id)
{
   t_node *nd = GetNode(id);

   if (nd) {
      if (CguiLoadImages(filename, imagename, transp, nd))
         return nd->ob->id;
   }
   return -1;
}

/* This function creates a hot-key `c' (which is an uppercase letter) assuming that `b' is a labelled object,
   and that `c' exists somewhere in the label. */
extern int SetButtonHk(char c, t_object *b)
{
   char tmp, *label, *p;
   int i, scan, ascii;

   c = ToUpper(c);
   for (label = b->label, i = 0; *label; i++) {
      p = label;
      while (*label) {
         if (ToUpper(*label) == c) {
            b->hkrow = i;
            tmp = label[1];
            label[1] = 0;
            b->hklen = text_length(b->font, label);
            b->hkxoffs = text_length(b->font, p) - b->hklen;
            label[1] = tmp;
            ascii = (unsigned) (unsigned char) c;
            scan = AsciiToScan(ascii);
            InsertHotKey(b, scan, ascii);
            InsertHotKey(b, scan, 0);
            return 1;
         }
         label++;
      }
      label++;
   }
   return 0;
}

/* Inserts the label into the object and interpret the special escape charcters defined for labels in CGUI.
   - The label may be spread over more than one line
   - An image specification may be extracted from the label
   - A hot key specification may be extracted from the label. In that case also calculatets the position of
      of the hot key relative to the label.
   The rows in the final label are separated with string termination characters (i.e. zeros) while the actual
   string end is indicated by two zeros in sequence.
*/
extern void InsertLabel(t_object *b, const char *raw_label)
{
   int currow, hashk = 0, scan, ascii;
   char *s, *p, c, *rowstart, *label, *mem;

   label = mem = MkString(raw_label);
   /* One extra is needed for the extra "end of strings" */
   if (_cgui_parse_labels) {
      s = rowstart = b->label = GetMem0(char, strlen(raw_label) + 2);
      currow = 0;
      while (*label) {
         if ((unsigned) (unsigned char) *label == 27) {
            InsertHotKey(b, KEY_ESC, 27);
            label++;
         }
         if (*label == '#') {
            /* May be an image or a numeric hot key specification. */
            label++;
            if (*label == '#') {
               *s++ = *label++;
            } else {
               /* It is an image or a numeric hot key specification. */
               if (isdigit(*label)) {
               /* It is a numeric hot key specification. */
                  sscanf(label, "%d", &ascii);
                  for (label++; *label && *label != ';' && *label != ','; label++);
                  if (*label == ',') {
                     label++;
                     scan = ascii;
                     sscanf(label, "%d", &ascii);
                     for (; *label && *label != ';'; label++);
                  } else if (ascii==27) {
                     scan = KEY_ESC;
                  } else {
                     scan = AsciiToScan(ascii);
                  }

                  if (*label)
                     label++;
                  InsertHotKey(b, scan, ascii);
               } else {
                  /* It is an image specification. */
                  while (*label==' ')
                     label++;
                  for (p = label; *p && *p != ';'; p++);
                  c = *p;
                  *p = 0;
                  if (b->imname==NULL) {
                     b->imname = MkString(label);
                     b->im = GetImage(b->parent, label);
                  } else if (b->imname_inactive == NULL) {
                     b->imname_inactive = MkString(label);
                     b->im_inactive = GetImage(b->parent, label);
                  } else if (b->imname_focus == NULL) {
                     b->imname_focus = MkString(label);
                     b->im_focus = GetImage(b->parent, label);
                  }
                  *p = c;
                  label = p;
                  if (*label == ';')
                     label++;
               }
            }
         } else if (*label == '_') {
            /* May be a new line. */
            label++;
            if (*label == '_') {
               *s++ = *label++;
            } else {
               /* It's a new line. Terminate the current and start a new one. */
               *s++ = 0;
               rowstart = s;
               currow++;
            }
         } else if (*label == '~') {
            /* May be a hot key. */
            label++;
            if (*label == '~') {
               *s++ = *label++;
            } else {
               /* It is a hot key. */
               *s = *label++;
               ascii = toupper((unsigned) (unsigned char) *s);
               scan = AsciiToScan(ascii);
               InsertHotKey(b, scan, ascii);
               InsertHotKey(b, scan, 0);
               s[1] = 0;
               b->hklen = text_length(b->font, s);
               b->hkrow = currow;
               b->hkxoffs = text_length(b->font, rowstart) - b->hklen;
               s++;
               hashk = 1;
            }
         } else {
            *s++ = *label++;
         }
      }
      *s = 0;
      Release(mem);
   } else {
      b->label = GetMem0(char, strlen(raw_label) + 2);
      strcpy(b->label, raw_label);
   }
   if (!hashk)
      b->autohk = b->parent->win->autohk;
}

/* This function creates a list of sub-rectangles within the rectangle
   `sc'.
   - sc is the rectangular area that you want to display. It can be anything.
   - ow is the widow where it is.
   - The return value is a (possibly empty) list of rectangles, each of these
   representing a fraction of the sc in window ow that is visable (i.e. not
   hidden by other windows.
   - The value pointed to by `n' will be updated with the number of
   rectangles there are in the list.

   The point is to use this when e.g. an object is to be refreshed (and it
   is possibly more or less hidden) - The list tells how the object must be
   clipped.
   The algorthm used will not split into more rectangles than necessary and
   if there is a choice between making one rectangle high or another wide, it
   will always prefere making the wide one. However there is no concatenation
   of rectangles, so occationally there may occure non-optimal elements in
   the list. */
extern t_coord *PartitionArea(t_object *ow, t_coord *sc, int *n)
{
   t_coord *rects;
   t_object *p;

   *n = 1;
   rects = GetMem(t_coord, INIT_RECT_SIZE);
   *rects = *sc;
   for (p = FirstWin(); p && p != ow; p = NextWin(p))
      rects = _RectList(p->x1, p->y1, p->x2, p->y2, rects, n, INIT_RECT_SIZE);
   return rects;
}

extern void Stub(void *data nouse)
{
}

extern void StubOb(t_object *b nouse)
{
}

static t_coord *InsertAreaStub(t_object *b nouse, t_coord *rects, int *n nouse, int *m nouse)
{
   return rects;
}

static void DefaultStretcher(t_object *b, int dx, int dy)
{
   b->rex += dx;
   b->rey += dy;
}

static void DefaultSetSize(t_object *b)
{
   if (b->dire)
      b->x1 = b->y1 = b->x2 = b->y2 = 0;
}

static void DefaultCatchImage(struct t_object *b)
{
   if (b->imname)
      b->im = GetImage(b->parent, b->imname);
}

extern void DefaultGetSizeOffset(t_object *b, int *w, int *h)
{
   *w = b->rex;
   *h = b->rey;
}

extern void DefaultSetSizeOffset(t_object *b, int w, int h)
{
   b->rex = w;
   b->rey = h;
}

static int DefaultChangeData(t_object *b nouse, void *data nouse,
                             int type nouse)
{
   return 0;
}

static int DefaultAddHandler(t_object *b nouse,
           void (*Handler) (void *data) nouse, void *data nouse)
{
   return 0;
}

/* This function will be invoced when the object has to be individually
   updated, e.g. when hit by the mouse or by demand update by the
   application. During draw of the entire screen the update-routine of the
   object will be called instead (to avoid redundant screen-blits). */
extern void DefaultRefresh(t_object *b)
{
   t_coord sc, a;
   t_object *ow;  /* object-window (contains absolute screen coordinates) */
   t_node *nd;

   nd = b->parent;
   if (nd && nd->win && nd->bmp) {
      ow = nd->win->node->ob;
      b->tf->Draw(b); /* draw on window's local buffer */

      /* express the area of the object in parent coordinate system */
      if (blit_clip_request) {
         /* The application requests a less area of blitting */
         a.x1 = b->x1 + blit_clip_x1;
         a.y1 = b->y1 + blit_clip_y1;
         a.x2 = b->x1 + blit_clip_x2;
         a.y2 = b->y1 + blit_clip_y2;
         blit_clip_request = 0;
      } else {
         a.x1 = b->x1;
         a.y1 = b->y1;
         a.x2 = b->x2;
         a.y2 = b->y2;
      }
      /* express the area of the object in screen coordinate system */
      sc.x1 = ow->x1 + nd->wx + a.x1;
      sc.y1 = ow->y1 + nd->wy + a.y1;
      sc.x2 = ow->x1 + nd->wx + a.x2;
      sc.y2 = ow->y1 + nd->wy + a.y2;
      BlitObject(&a, &sc, nd);
   }
}

extern void SetBlitLimit(int x1, int y1, int x2, int y2)
{
   blit_clip_x1 = x1;
   blit_clip_y1 = y1;
   blit_clip_x2 = x2;
   blit_clip_y2 = y2;
   blit_clip_request = 1;
}

static void DefaultSetView(t_object *b, int flags)
{
   b->view = flags;
}

static void DefaultFree(t_object * b)
{
   t_hotkey *hk, *tmp;

   for (hk = b->hks; hk;) {
      tmp = hk;
      hk = hk->next;
      Release(tmp);
   }
   if (b->tablink && b->parent)
      UnlinkFromTabChain(b->parent->win, b->tablink, b);
   if (b->tooltip)
      Release(b->tooltip);
   if (b->ttbmp)
      destroy_bitmap(b->ttbmp);
   if (b->md)
      StopMouseEvent(b->md);
   if (b->AppExit)
      _GenEventOfCgui(b->AppExit, b->exitdata, 0, 0);
   if (b->linked_event)
      DestroyLinkedEvents(b->id);
   if (b->label)
      Release(b->label);
   if (b->imname)
      Release(b->imname);
   if (b->visitor)
      StopRefreshTraversing(b->visitor);
   ReturnId(b->id);
   Release(b);
}

extern void Event2ObjectLink(int id)
{
   t_object *b;

   b = GetObject(id);
   if (b) {
      b->linked_event++;
   }
}

extern void XtendedFree(t_object * b)
{
   if (b->appdata)
      Release(b->appdata);
   DefaultFree(b);
}

/* Default handler: just pass it on to the next one */
static int NextWinToTop(t_object * b)
{
   if (b->prev) {
      return b->prev->tf->MoveWin(b->prev);
   } else
      return 0;
}

static t_object *GetMainNode(t_object * b)
{
   return b;
}

/* Default handler: just pass it on to the next one */
static void GetNextWin(t_object * b)
{
   if (b->prev)
      b->prev->tf->TakeFocus(b->prev);
}

static void DefaultActivate(t_object * b)
{
   b->inactive = 0;
}

static void DefaultDeActivate(t_object *b)
{
   b->inactive = 1;
   if (b->tablink)
      b->tcfun->NotifyInactivation(b->tablink, b);
}

/* Makes a plain unlink of the object */
static void DefaultUnlinkObject(t_object * b)
{
   if (b->parent) {
      if (b->parent->ip == b) {
         if (b->prev)
            b->parent->ip = b->prev;
         else
            b->parent->ip = b->next;
      }
      if (b->prev)
         b->prev->next = b->next;
      else
         b->parent->firstob = b->next;
      if (b->next)
         b->next->prev = b->prev;
      else
         b->parent->last = b->prev;
   }
}

/* Replaces `b' with (the object of) node `nd' in the object tree.
   `b' will be inserted as the last child of `nd'. */
extern void MoveNodeInTree(t_node *nd, t_object *b)
{
   if (b->parent) {
      nd->ob->tf->Unlink(nd->ob);
      nd->ob->prev = b->prev;
      nd->ob->next = b->next;
      nd->ob->parent = b->parent;
      if (b->prev)
         b->prev->next = nd->ob;
      else
         b->parent->firstob = nd->ob;
      if (b->next)
         b->next->prev = nd->ob;
      else
         b->parent->last = nd->ob;
      if (b->parent->ip == b)
         b->parent->ip = nd->ob;
      if (nd->ip == nd->last)
         nd->ip = b;
      if (nd->last)
         nd->last->next = b;
      else
         nd->firstob = b;
      b->prev = nd->last;
      b->next = NULL;
      nd->last = b;
      b->parent = nd;
   }
}

/* Moves an object in the object tree. b1 will be removed from its current
   location and inserted immediately after the object b2.
   Note! Use with care! Moving simple objects that has been create with use
   of a "direction cammand" may behave unpredicatable in later updates, if the
   node/window has not been re-built first.
   The objects must exist but can be in arbitrary locations in the tree. */
extern void MoveObjectInTree(t_object *b1, t_object *b2)
{
   t_node *par_node;

   if (b1 == b2)
      return;
   if (b1 == NULL)
      return;
   par_node = b2->parent;
   if (par_node == NULL)
      return;
   b1->tf->Unlink(b1);
   b1->next = b2->next;
   b2->next = b1;
   if (b1->next)
      b1->next->prev = b1;
   else
      par_node->last = b1;
   b1->prev = b2;
}

/* Removes the object and updates the screen (i.e. grey-out the area where
   the object was). This is done by re-building the rectangles of the node
   that contains b. I.e. this requires that `b' is contained in a node
   that maintains a rectangle list (and this should always be the case). */
static void DefaultRemove(t_object *b)
{
   t_coord sc, a;
   t_object *ow;  /* object-window */
   t_node *nd;

   nd = b->parent;
   if (nd && nd->win && nd->bmp) {
      ow = nd->win->node->ob;
         /* express the area of the object in parent coordinate system */
      a.x1 = b->x1;
      a.y1 = b->y1;
      a.x2 = b->x2;
      a.y2 = b->y2;
      sc = a;
      sc.x1 += ow->x1 + nd->wx;
      sc.y1 += ow->y1 + nd->wy;
      sc.x2 += ow->x1 + nd->wx;
      sc.y2 += ow->y1 + nd->wy;
   }
   b->tf->Unlink(b);
   b->tf->Free(b);
   if (nd && nd->win) {
      nd->ob->tf->MakeNodeRects(nd->ob);
      /* It is necessary to draw the entire parent node to refresh the
         area where the object used to live */
      nd->ob->tf->Draw(nd->ob);
      /* Only blit the former object area */
      BlitObject(&a, &sc, nd);
   }
}

static void GetToolTipPostition(t_object *b, int *x, int *y, int w, int h)
{
   int x1, y1, y2;
   t_node *nd;
   t_object *ow;

   nd = b->parent;
   if (nd && nd->win) {
      ow = nd->win->node->ob;
      /* get the position in screen coordinate system */
      x1 = ow->x1 + nd->wx + b->x1;
      y1 = ow->y1 + nd->wy + b->y1;
      y2 = ow->y1 + nd->wy + b->y2;
      if (x1 + w < SCREEN_W)
         *x = x1;
      else
         *x = SCREEN_W - w;
      if (y2 + h < SCREEN_H)
         *y = y2;
      else if (y1 - h >= 0)
         *y = y1 - h;
      else
         *y = 0;
   } else
      *x = *y = 0;
}

extern void ClearToolTip(t_object *b)
{
   Release(b->tooltip);
   b->tooltip = NULL;
}

static void DrawToolTip(t_object *b, int srcx, int srcy, int x, int y)
{
   int width, height;
   BITMAP *bmp;

   width = b->ttbmp->w;
   height = b->ttbmp->h;
   bmp = b->ttbmp;
   blit(bmp, cgui_bmp, srcx, srcy, x, y, width, height);
   scare_mouse_area(MAX(0,x), MAX(0,y), width, height);
   blit(bmp, screen, srcx, srcy, x, y, width, height);
   unscare_mouse();
}

static void OutputToolTip(void *data)
{
   t_object *b=data;

   if (b->ttbmp) {
      if (tool_tip_animation_step) {
         b->tooltip_start -= tool_tip_animation_step;
         if (b->tooltip_start < 0) {
            b->tooltip_start = 0;
         }
         DrawToolTip(b, b->tooltip_start, 0, b->ttx, b->tty);
         if (b->tooltip_start > 0) {
            _GenEventOfCgui(OutputToolTip, b, tool_tip_animation_delay, b->id);
         }
      } else {
         DrawToolTip(b, 0, 0, b->ttx, b->tty);
      }
   }
}

extern void ShowToolTipAt(t_object *b, const char *s, int nr_of_rows, int width, int x, int y)
{
   int height;
   const char *p;

   b->ttx = x;
   b->tty = y;
   height = nr_of_rows * text_height(b->font) + 2*BORDER_WIDTH;
   if (b->ttbmp) {
      destroy_bitmap(b->ttbmp);
   }
   b->ttbmp = create_bitmap(width, height);
   clear_to_color(b->ttbmp, cgui_colors[CGUI_COLOR_TOOL_TIP_BACKGROUND]);
   rect(b->ttbmp, 0, 0, b->ttbmp->w - 1, b->ttbmp->h - 1, cgui_colors[CGUI_COLOR_TOOL_TIP_FRAME]);
   y = BORDER_WIDTH;
   for (p = s; *p; p += strlen(p) + 1) {
      textout_ex(b->ttbmp, b->font, p, BORDER_WIDTH, y, cgui_colors[CGUI_COLOR_TOOL_TIP_TEXT], -1);
      y += text_height(b->font);
   }
   if (tool_tip_animation_step) {
      b->tooltip_start = width;
   }
   _GenEventOfCgui(OutputToolTip, b, tool_tip_delay, b->id);
}

static void ShowToolTip(t_object *b, const char *s)
{
   int nr_of_rows, height, width, x, y;
   const char *p;

   width = 0;
   nr_of_rows = 0;
   for (p = s; *p; p += strlen(p) + 1) {
      width = MAX(width, text_length(b->font, p));
      nr_of_rows++;
   }
   width += 2*BORDER_WIDTH;
   height = nr_of_rows * text_height(b->font) + 2*BORDER_WIDTH;
   GetToolTipPostition(b, &x, &y, width, height);
   ShowToolTipAt(b, s, nr_of_rows, width, x, y);
}

extern void HideToolTip(t_object *b)
{
   if (b->ttbmp) {
      RefreshScreen(b->ttx, b->tty, b->ttx + b->ttbmp->w, b->tty + b->ttbmp->h);
      destroy_bitmap(b->ttbmp);
      b->ttbmp = NULL;
   }
}

static void DefaultOver(t_object *b)
{
   Refresh(b->id);
   if (b->view & SV_NO_TOOLTIP)
      return;
   if (b->state == MS_OVER) {
      switch (b->view & (SV_ONLY_BRIEF|SV_PREFERE_BRIEF|SV_ONLY_LONG|SV_PREFERE_LONG)) {
      case SV_ONLY_BRIEF:
         if ((b->view & SV_HIDE_LABEL) && b->label && *b->label)
            ShowToolTip(b, b->label);
         break;
      case SV_PREFERE_BRIEF:
         if ((b->view & SV_HIDE_LABEL) && b->label && *b->label)
            ShowToolTip(b, b->label);
         else if (b->tooltip)
            ShowToolTip(b, b->tooltip);
         break;
      case SV_ONLY_LONG:
         if (b->tooltip)
            ShowToolTip(b, b->tooltip);
         break;
      default:
         if (b->tooltip)
            ShowToolTip(b, b->tooltip);
         else if ((b->view & SV_HIDE_LABEL) && b->label && *b->label)
            ShowToolTip(b, b->label);
         break;
      }
   } else {
         HideToolTip(b);
   }
}

/* Creates one generic object. Data components are initialized with
   reasonable values and pointers are correctly initialized. Functions
   pointers are set to dummy functions or to functions performing safe
   actions. */
extern t_object *CreateObject(int x, int y, t_node *nd)
{
   t_object *b;
   static t_mevent me;
   static int virgin = 1;

   if (virgin) {
      virgin = 0;
      SetDefaultStateMachine(&me);
   }
   b = GetMem0(t_object, 1);
   b->me = &me;
   b->tf = &default_type_functions;
   if (nd && nd->ob->tablink) {
      b->tcfun = &default_slave_tc_functions;
      b->tablink = nd->ob->tablink;
      if (SubFocusOfLink(b->tablink) == NULL) {
         SetSubFocusOfLink(b->tablink, b);
      }
   } else
      b->tcfun = &default_tabchain_functions;
   b->hkfunc = &default_hotkey_functions;
   b->id = GetAnId(b);
   b->font = _cgui_prop_font;
   b->click = LEFT_MOUSE;
   b->Action = Stub;
   b->Action2 = Stub;
   b->Adapt = NoAdapt;
   b->Over = DefaultOver;

   if (nd) {
      /* link it into the parent tree-node */
      if (nd->ip == NULL)
         nd->ip = nd->last;
      if (nd->ip == NULL) {
         nd->firstob = b;
         nd->last = b;
      } else {
         b->prev = nd->ip;
         b->next = nd->ip->next;
         if (b->next)
            b->next->prev = b;
         else
            nd->last = b;
         b->prev->next = b;
      }
      nd->ip = b;
      b->parent = nd;
   }
   /* Transform the coordinates (or rather eventual `direction commands') to
      a set of functions to be used during the process of arranging the
      positions and sizes of the objects. */
   SetPosition(b, x, y);
   return b;
}

extern int InsertPoint(int id)
{
   t_object *b;
   t_node *nd;
   int prevpoint=1;

   b = GetObject(id);
   if (b == NULL)
      return 0;
   nd = b->parent;
   if (nd == NULL)
      return 0;
   if (opwin && opwin->win->opnode->ip)
      prevpoint = opwin->win->opnode->ip->id;
   SelectContainer(nd->ob->id);
   nd->ip = b;
   return prevpoint;
}

/* Application interface: */

extern int AddHandler(int id, void (*Handler) (void *), void *data)
{
   t_object *b = GetObject(id);

   if (b)
      return b->tf->AddHandler(b, Handler, data);
   return 0;
}

/* Removes the object and updates the screen. */
extern void Remove(int id)
{
   t_object *b;

   b = GetObject(id);
   if (b && b->parent) {
      b = b->tf->GetMainNode(b);
      b->tf->Remove(b);
   }
}

/* As Remove, but no update */
extern void Destroy(int id)
{
   t_object *b;

   b = GetObject(id);
   if (b) {
      b->tf->Unlink(b);
      b->tf->Free(b);
   }
}

extern int GetPressedButton(int id)
{
   t_object *b = GetObject(id);

   if (b)
      return b->usedbutton;
   return 0;
}

static void NodeSetSpacing(t_node *nd, int left, int h, int right, int top, int v, int bottom)
{
   nd->leftx = left;
   nd->xdist = h;
   nd->rightx = right;
   nd->topy = top;
   nd->dy = v;
   nd->boty = bottom;
}

extern void ObjectRefreshEnter(t_object *b, t_refresh_traversor *visitor)
{
   b->visitor = visitor;
}

extern void ObjectRefreshVisit(t_object *b, void *calldata, int reason)
{
   if (b->AppUpd)
      b->AppUpd(b->id, b->appupddata, calldata, reason);
}

extern void ObjectRefreshLeave(t_object *b)
{
   b->visitor = NULL;
}

/* Sets the distance used when positioning objects with the
   "direction-commands". This will be used as the distance beteen objects as
   well as between objects and the window-borders. The setting will take
   effect in the operating node */
extern void SetDistance(int xdist, int ydist)
{
   SetMeasure(xdist, xdist, xdist, ydist, ydist, ydist);
}

extern void SetMeasure(int left, int h, int right, int top, int v, int bottom)
{
   if (opwin && opwin->win && opwin->win->opnode) {
      NodeSetSpacing(opwin->win->opnode, left, h, right, top, v, bottom);
   }
}

extern int SetSpacing(int id, int left, int h, int right, int top, int v, int bottom)
{
   t_node *nd;

   nd = GetNode(id);
   if (nd) {
      NodeSetSpacing(nd, left, h, right, top, v, bottom);
      return 1;
   }
   return 0;
}

/* Position the mouse-pointer above the object */
extern void PointerOn(int id)
{
   t_object *b = GetObject(id);
   int halfw, halfh;

   if (b) {
      halfw = (b->x2 - b->x1) >> 1;
      halfh = (b->y2 - b->y1) >> 1;
      /* SetPos needs coordinates in current window's coorinate system */
      SetMousePos(b->x1 + b->parent->wx + halfw +
                  b->parent->win->node->ob->x1,
                  b->y1 + b->parent->wy + halfh +
                  b->parent->win->node->ob->y1);
   }
}

extern void Refresh(int id)
{
   t_object *b = GetObject(id);

   if (b && b->parent && IsWindowCompleated())
      b->tf->Refresh(b);
}

extern void DeActivate(int id)
{
   t_object *b, *obm, *obs;

   b = GetObject(id);
   if (b && id) {
      b->tf->DeActivate(b);
      if (IsWindowCompleated() && b->parent && b->parent->win) {
         /* If the object occasionally is in focus it should be unfocused */
         GetTabChainFocus(b->parent->win, &obm, &obs);
         if (obm && obs == b) {
            if (obs == obm) {
               TabChainNotifyKeyPress(b->parent->win, KEY_TAB, 0);
            } else {
               if (!MoveFocusToNextSubObject(obm, obs, KEY_LEFT, 0)) {
                  if (MoveFocusToNextSubObject(obm, obs, KEY_DOWN, 0))
                     TabChainNotifyKeyPress(b->parent->win, KEY_TAB, 0);
               }
            }
         }
      }
   }
}

extern void Activate(int id)
{
   t_object *b = GetObject(id);

   if (b)
      b->tf->Activate(b);
}

extern void Click(int id)
{
   t_object *b = GetObject(id);

   if (b) {
      b->tcfun->MoveFocusTo(b);
      b->Action(b->appdata);
   }
}

extern int GetSizeOffset(int id, int *w, int *h)
{
   t_object *b = GetObject(id);

   if (b) {
      b->tf->GetSizeOffset(b, w, h);
      return 1;
   }
   return 0;
}

extern int SetSizeOffset(int id, int w, int h)
{
   t_object *b = GetObject(id);

   if (b) {
      b->tf->SetSizeOffset(b, w, h);
      return 1;
   }
   return 0;
}

extern int HookExit(int id, void (*ExitFun) (void *data), void *data)
{
   t_object *b = GetObject(id);

   if (b) {
      b->AppExit = ExitFun;
      b->exitdata = data;
      return 1;
   }
   return 0;
}

extern int SetMouseButtons(int id, int buttons)
{
   t_object *b = GetObject(id);

   if (b) {
      b->click = buttons;
      return 1;
   }
   return 0;
}

extern int GetObjectPosition(int id, int *x, int *y, int *wx, int *wy)
{
   t_object *b = GetObject(id);

   if (b) {
      *x = b->x1;
      *y = b->y1;
      if (b->parent) {
         *wx = b->x1 + b->parent->wx;
         *wy = b->y1 + b->parent->wy;
      }
      return 1;
   }
   return 0;
}

extern int GetObjectSize(int id, int *w, int *h)
{
   t_object *b = GetObject(id);

   if (b) {
      *w = b->x2 - b->x1 + 1;
      *h = b->y2 - b->y1 + 1;
      return 1;
   }
   return 0;
}

extern int ToolTipText(int id, const char *text)
{
   t_object *b;
   char *s;

   b = GetObject(id);
   if (b) {
      ClearToolTip(b);
      b->tooltip = s = GetMem(char, strlen(text) + 2);
      while (*text) {
         if (*text == '_' && _cgui_parse_labels) {
            text++;
            if (*text == '_')
               *s++ = *text++;
            else
               *s++ = 0;
         } else {
            *s++ = *text++;
         }
      }
      *s++ = 0;
      *s = 0;
      return 1;
   }
   return 0;
}

extern void CguiSetToolTipDelay(int delay)
{
   tool_tip_delay = delay;
}

extern void CguiSetToolTipAnimation(int step, int delay)
{
   tool_tip_animation_step = step;
   tool_tip_animation_delay = delay;
}

extern int SetView(int id, int flags)
{
   t_object *b = GetObject(id);

   if (b) {
      b->tf->SetView(b, flags);
      return 1;
   }
   return 0;
}

typedef struct t_dcallback {
   void (*CallBack)(void*);
   void *data;
} t_dcallback;

static void CallBackWrapper(void *data)
{
   t_dcallback *dc = data;

   dc->CallBack(dc->data);
   Release(dc);
}

extern void DelayedCallBack(void (*CallBack)(void*), void *data)
{
   t_dcallback *dc;

   dc = GetMem(t_dcallback, 1);
   dc->CallBack = CallBack;
   dc->data = data;
   _GenEventOfCgui(CallBackWrapper, dc, 0, 0);
}

extern int CguiParseLabels(int state)
{
   int prev_state;
   prev_state = _cgui_parse_labels;
   _cgui_parse_labels = state;
   return prev_state;
}
