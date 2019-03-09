/* Module OBEDBOX.C

   Contains functions for creating "text-edit boxes" A text-edit box is a an
   object that lets the user enter a text after selecting the editbox (with
   left mouse-button or hot-key). There is only one row of text and no
   scrolling in the box. The application does not need any call-back
   function. In most cases the edit-box is part of a dialogue box, and the
   resulting value may be examined when closing the box. However, in some
   cases the application may want to get a callback. If this is the case,
   such a call-backfunction mau be installed by a call to "AddHandler". The
   call-back function of an edit-box will be called each time the user
   presses a key (before the edit-box itself takes care of the key-press). */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <allegro.h>

#include "cgui.h"
#include "cgui/mem.h"

#include "obedbox.h"
#include "edbmenu.h"
#include "clipwin.h"
#include "combo.h"
#include "event.h"
#include "cguiinit.h"
#include "window.h"
#include "node.h"
#include "object.h"
#include "obbutton.h"
#include "mstates.h"
#include "id.h"
#include "cgui/halfform.h"
#include "cgui/clipwin.h"
#include "ndresize.h"
#include "tabchain.h"

#if ALLEGRO_SUB_VERSION == 0
#define textout_ex(bmp, f, s, x, y, col, bgcol) textout(bmp, f, s, x, y, col)
#define set_clip_rect set_clip
#endif

#define SIZE_OF_NUMBER_STRING 60

static int decimal_point;

struct t_editbox {
   void (*Action) (void *);
   void *data;
   void (*ComboAction) (t_combo_box *);
   t_combo_box *combo_box;
   int scan, ascii;
   char *curpos;
   long copy;
#ifdef ALLEGRO_UNIX
   clipX *xclip;
#endif
   /* The x-position of the text-cursor */
   int x;
   int width;
   int th;

   /* The string-index of the cursor (immediatly after cursor) */
   unsigned i;

   /* How to translate between item and text */
   unsigned format;

   /* The index in the text that starts the display */
   int vstart;

   /* Max number of bytes in the application's textbuffer in case of string. */
   int string_buffer_size;

   /* The data size of numerical values */
   int datasize;

   /* Special key action */
   int keyaction;

   /* Flag indicating that the user has started typing and so we shall ignore the FBLANK0 flag, which is for presentation only. */
   int usertyped;

   /* Flag (user option) indicating that we shall handle a KEY_ENTER as if it were a KEY_TAB. */
   int taboncr;

   /* Flag (user option) indicating that we shall pad a hexadecimal number with zeros to the left (up to the size of the number). */
   int pad_zeros;

   /* The user data item to to create formatted text from (and the reverse) */
   void *item;

   /* Pointer back to the object carrying */
   struct t_object *b;

   /* Pointer to the text to edit */
   char *text;

   /* Pointer to pointer to re-allocatable memory (FPTRSTR) */
   char **ptr;

   /* pointers marking start and end of selected part of text */
   char *mark1, *mark2;

   /* The edit-field has a private bitmap */
   struct BITMAP *bmp;

   /* A copy of the text to edit, will be used if user presses the esc-key */
   char *textcopy;

   int cursor;

   /* Event-id for the text-cursor toggler. */
   int blinkid;

   /* Event-id for the scolling-function */
   int scrollid;

   /* Flag indicating if scrolling is in progress */
   int scrolling;

   /* Flag indicating if editing is in progress, i.e. the edit-box is "in focus", and cursor is blinking */
   int edit_in_progress;

   /* Must be there to to know if the screen resolution has changed in which case it is necessary to re-make the bitmap */
   int curh, curw;

   /* A flag telling if rounding of float type decimals should be done. */
   int is_max_nr_of_decimals;

   /* The number of decimals to limit float types display to. */
   int max_nr_of_decimals;
};

static char *clipboard;
static int blinktime = 250;
static t_editbox *ed_progr, *ed_callback_progr;
static int persistent_selection = 1;

extern int GetHalf(char *s)
{
   while (*s) {
      if (*s == HALF_CHAR)
         return 1;
      s++;
   }
   return 0;
}

extern int SgnGetHalf(char *text)
{
   char *s = text;

   while (*s) {
      if (*s == '-')
         return -GetHalf(s);
      s++;
   }
   return GetHalf(text);
}

extern int ToUpper(int chr)
{
   if (chr & 0x80) {
      switch ((char) chr) {
      case 'å':
         chr = 'Å';
         break;
      case 'ä':
         chr = 'Ä';
         break;
      case 'ö':
         chr = 'Ö';
         break;
      case 'é':
         chr = 'É';
         break;
      case 'ü':
         chr = 'Ü';
         break;
         break;
      }
   } else {
      chr = toupper(chr);
   }
   return chr;
}

static int ToLower(int chr)
{
   if (chr & 0x80) {
      switch ((char) chr) {
      case 'Å':
         chr = 'å';
         break;
      case 'Ä':
         chr = 'ä';
         break;
      case 'Ö':
         chr = 'ö';
         break;
      case 'É':
         chr = 'é';
         break;
      case 'Ü':
         chr = 'ü';
         break;
      }
   } else {
      chr = tolower(chr);
   }
   return chr;
}

static int IsDelimiter(char chr)
{
   switch (chr) {
   case '-':
   case ' ':
   case '\t':
      return 1;
   }
   return 0;
}

void TimeAsDate(char *buf, time_t time)
{
   struct tm *tmtime;
   tmtime = gmtime(&time);
   strftime(buf, SIZE_OF_NUMBER_STRING, "%Y-%m-%d", tmtime);
}

static void FormatItem2Text(t_editbox *edb)
{
   char *text;
   char *p;
   char tmp[100];
   void *item;
   long lval = 0;
   long undef = 0;
   char buf[100];
   char *fmt;

   text = edb->text;
   item = edb->item;
   switch (edb->format) {
   case FBYTE:
      lval = (unsigned) *(unsigned char *) item;
      sprintf(text, "%d", (int) lval);
      undef = B_UNDEF_VAL;
      break;
   case FSHORT:
      lval = *(short *) item;
      sprintf(text, "%d", (short) lval);
      undef = S_UNDEF_VAL;
      break;
   case FINT:
      lval = *(int *) item;
      sprintf(text, "%d", (int) lval);
      undef = I_UNDEF_VAL;
      break;
   case FLONG:
      lval = *(long *) item;
      sprintf(text, "%ld", lval);
      undef = L_UNDEF_VAL;
      break;
   case FFLOAT:
      if (edb->is_max_nr_of_decimals) {
         sprintf(buf, "%%.%df", edb->max_nr_of_decimals);
         fmt = buf;
      } else {
         fmt ="%f";
      }
      sprintf(tmp, fmt, *(float *) item);
      tmp[edb->string_buffer_size-1] = 0;
      strcpy(text, tmp);
      break;
   case FDOUBLE:
      if (edb->max_nr_of_decimals) {
         sprintf(buf, "%%.%dlf", edb->max_nr_of_decimals);
         fmt = buf;
      } else {
         fmt ="%lf";
      }
      sprintf(tmp, fmt, *(double *) item);
      tmp[edb->string_buffer_size-1] = 0;
      strcpy(text, tmp);
      break;
   case FSTRING:
      if (edb->keyaction & NAMECASE)
         NameCase(text);
      break;
   case FPTRSTR:
      if (*edb->ptr == NULL)
         *edb->ptr = GetMem0(char, 1);
      edb->text = *edb->ptr;
      if (edb->keyaction & NAMECASE)
         NameCase(*edb->ptr);
      break;
   case FPOINTS:
      lval = *(int *) item;
      MakeHalf(text, lval, 0, 0);
      undef = P_UNDEF_VAL;
      break;
   case FBPOINTS:
      lval = (int) *(char *) item;
      MakeHalf(text, lval, 0, 0);
      undef = BP_UNDEF_VAL;
      break;
   case FHEX1:
      sprintf(text, "%x", (unsigned) *(unsigned char *) item);
      break;
   case FHEX2:
      sprintf(text, "%x", *(unsigned short *) item);
      break;
   case FHEX4:
      sprintf(text, "%lx", *(unsigned long *) item);
      break;
   case FOCT1:
      sprintf(text, "%o", *(unsigned int *) item);
   case FOCT2:
      sprintf(text, "%o", *(unsigned int *) item);
   case FOCT3:
      sprintf(text, "%o", *(unsigned int *) item);
   case FOCT4:
      sprintf(text, "%o", *(unsigned int *) item);
      break;
   case ISO8601_DATE:
      TimeAsDate(text, *(time_t *) item);
      break;
   }
   if ((edb->keyaction & BL0) && !edb->usertyped) {
      switch (edb->format) {
      case FBYTE:
      case FSHORT:
      case FINT:
      case FLONG:
      case FPOINTS:
      case FBPOINTS:
         if (*text == '0')
            *text = 0;
         break;
      case FDOUBLE:
      case FFLOAT:
         p = text;
         while (*p && (*p == '0' || *p == ',' || *p == '.'))
            ++p;
         if (*p == 0) {
            *text = 0;
         }
         break;
      }
   }
   if ((edb->keyaction & FUND) && (lval & undef) == undef) {
      strcpy(text, "?");
   }
   if (edb->format != FPTRSTR)
      text[edb->string_buffer_size-1] = 0;
}

static time_t StringToTime(const char *text)
{
   struct tm tm = {0};
   sscanf(text, "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
   tm.tm_isdst = -1;
   tm.tm_mon--;
   tm.tm_mday++;
   if (tm.tm_year >= 0 && tm.tm_year < 100) {
    tm.tm_year += 2000;
   }
   tm.tm_year -= 1900;
   return mktime(&tm);
}

static int IsStringDate(const char *text)
{
   struct tm tm = {0};
   int n = 0;
   sscanf(text, "%d-%d-%d %n", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &n);
   return n == 0 || text[n] != 0;
}

static void DecodeText(t_editbox *edb)
{
   int tmp = 0;
   unsigned long tmpl;
   float f;
   double d;

   switch (edb->format) {
   case FBYTE:
      sscanf(edb->text, "%d", &tmp);
      if ((edb->keyaction & FUND) && *edb->text == '?')
         tmp = B_UNDEF_VAL;
      *(unsigned char *) edb->item = tmp;
      break;
   case FSHORT:
      sscanf(edb->text, "%d", &tmp);
      if ((edb->keyaction & FUND) && *edb->text == '?')
         tmp = S_UNDEF_VAL;
      *(short *) edb->item = tmp;
      break;
   case FINT:
      sscanf(edb->text, "%d", &tmp);
      if ((edb->keyaction & FUND) && *edb->text == '?')
         tmp = I_UNDEF_VAL;
      *(int *) edb->item = tmp;
      break;
   case FLONG:
      if ((edb->keyaction & FUND) && *edb->text == '?')
         tmpl = L_UNDEF_VAL;
      sscanf(edb->text, "%ld", &tmpl);
      *(long *) edb->item = tmpl;
      break;
   case FFLOAT:
      sscanf(edb->text, "%f", &f);
      *(float *) edb->item = f;
      break;
   case FDOUBLE:
      sscanf(edb->text, "%lf", &d);
      *(double *) edb->item = d;
      break;
      /* case FSTRING: No handling for strings case FPTRSTR: No handling for
         string-pointers break; */
   case FPOINTS:
      sscanf(edb->text, "%d", &tmp);
      if ((edb->keyaction & FUND) && *edb->text == '?')
         tmp = P_UNDEF_VAL;
      *(int *) edb->item =
          (tmp << 1) +
          (((tmp << 1) < 0) ? -GetHalf(edb->text) : tmp ?
           GetHalf(edb->text) : SgnGetHalf(edb->text));
      break;
   case FBPOINTS:
      sscanf(edb->text, "%d", &tmp);
      if ((edb->keyaction & FUND) && *edb->text == '?')
         tmp = BP_UNDEF_VAL;
      *(unsigned char *) edb->item =
          (tmp << 1) +
          (((tmp << 1) < 0) ? -GetHalf(edb->text) : GetHalf(edb->text));
      break;
   case FHEX1:
      sscanf(edb->text, "%x", &tmp);
      *(unsigned char *) edb->item = tmp;
      break;
   case FHEX2:
      sscanf(edb->text, "%x", &tmp);
      *(unsigned short *) edb->item = tmp;
      break;
   case FHEX4:
      sscanf(edb->text, "%lx", &tmpl);
      *(unsigned long *) edb->item = tmpl;
      break;
   case FOCT1:
      sscanf(edb->text, "%o", &tmp);
      *(unsigned int *) edb->item = tmp;
      break;
   case FOCT2:
      sscanf(edb->text, "%o", &tmp);
      *(unsigned int *) edb->item = tmp;
      break;
   case FOCT3:
      sscanf(edb->text, "%o", &tmp);
      *(unsigned int *) edb->item = tmp;
      break;
   case FOCT4:
      sscanf(edb->text, "%o", &tmp);
      *(unsigned int *) edb->item = tmp;
      break;
   case ISO8601_DATE:
      *(time_t*)edb->item = StringToTime(edb->text);
      break;
   }
}

static void IncrementItem(t_editbox *edb)
{
   switch (edb->format) {
   case FBYTE:
   case FBPOINTS:
   case FHEX1:
      (*(unsigned char *)edb->item)++;
      break;
   case FSHORT:
   case FHEX2:
      (*(short*)edb->item)++;
      break;
   case FINT:
   case FPOINTS:
   case FOCT1:
   case FOCT2:
   case FOCT3:
   case FOCT4:
      (*(int*)edb->item)++;
      break;
   case FLONG:
   case FHEX4:
      (*(long*)edb->item)++;
      break;
   }
}

static void DecrementItem(t_editbox *edb)
{
   switch (edb->format) {
   case FBYTE:
   case FBPOINTS:
   case FHEX1:
      (*(unsigned char *)edb->item)--;
      break;
   case FSHORT:
   case FHEX2:
      (*(short*)edb->item)--;
      break;
   case FINT:
   case FPOINTS:
   case FOCT1:
   case FOCT2:
   case FOCT3:
   case FOCT4:
      (*(int*)edb->item)--;
      break;
   case FLONG:
   case FHEX4:
      (*(long*)edb->item)--;
      break;
   }
}

static void RestoreItem(t_editbox *edb)
{
   void *data;
   switch (edb->format) {
   case FBYTE:
      *(unsigned char *) edb->item = edb->copy;
      break;
   case FSHORT:
      *(short *) edb->item = edb->copy;
      break;
   case FINT:
      *(int *) edb->item = edb->copy;
      break;
   case FLONG:
      *(long *) edb->item = edb->copy;
      break;
   case FFLOAT:
      data = &edb->copy;
      *(float *)edb->item = *(float *)data;
      break;
   case FDOUBLE:
      data = &edb->copy;
      *(double *)edb->item = *(double *)data;
      break;
   case FSTRING:
      strcpy(edb->item, edb->textcopy);
      break;
   case FPTRSTR:
      /* *edb->ptr = edb->text = ResizeMem(*edb->ptr,
         strlen(edb->textcopy)+1); strcpy(*edb->ptr, edb->textcopy); */
      break;
   case FPOINTS:
      *(int *) edb->item = edb->copy;
      break;
   case FBPOINTS:
      *(unsigned char *) edb->item = edb->copy;
      break;
   case FHEX1:
      *(unsigned char *) edb->item = edb->copy;
      break;
   case FHEX2:
      *(unsigned short *) edb->item = edb->copy;
      break;
   case FHEX4:
      *(unsigned long *) edb->item = edb->copy;
      break;
   case FOCT1:
   case FOCT2:
   case FOCT3:
   case FOCT4:
      *(unsigned int *) edb->item = edb->copy;
      break;
   case ISO8601_DATE:
      *(time_t *) edb->item = edb->copy;
      break;
   }
}


static void SaveCopy(t_editbox *edb)
{
   void *data;
   switch (edb->format) {
   case FBYTE:
      edb->copy = *(unsigned char *) edb->item;
      break;
   case FSHORT:
      edb->copy = *(short *) edb->item;
      break;
   case FINT:
      edb->copy = *(int *) edb->item;
      break;
   case FLONG:
      edb->copy = *(long *) edb->item;
      break;
   case FFLOAT:
      data = &edb->copy;
      *(float*)data = *(float *)edb->item;
      break;
   case FDOUBLE:
      data = &edb->copy;
      *(double*)data = *(double *)edb->item;
      break;
   case FSTRING:
      edb->textcopy = ResizeMem(char, edb->textcopy, strlen(edb->item) + 1);
      strcpy(edb->textcopy, edb->item);
      break;
   case FPTRSTR:
      edb->textcopy = ResizeMem(char, edb->textcopy, strlen(*edb->ptr) + 1);
      strcpy(edb->textcopy, *edb->ptr);
      break;
   case FPOINTS:
      edb->copy = *(int *) edb->item;
      break;
   case FBPOINTS:
      edb->copy = *(unsigned char *) edb->item;
      break;
   case FHEX1:
      edb->copy = *(unsigned char *) edb->item;
      break;
   case FHEX2:
      edb->copy = *(unsigned short *) edb->item;
      break;
   case FHEX4:
      edb->copy = *(unsigned long *) edb->item;
      break;
   case FOCT1:
   case FOCT2:
   case FOCT3:
   case FOCT4:
      edb->copy = *(unsigned int *) edb->item;
      break;
   case ISO8601_DATE:
      edb->copy = *(time_t *) edb->item;
      break;
   }
}

static void TextCursorBlink(void *data)
{
   t_editbox *edb = data;

   edb->cursor ^= 1;
   edb->b->tf->Refresh(edb->b);
   edb->blinkid = _GenEventOfCgui(TextCursorBlink, edb, blinktime, edb->b->id);
}

static void TerminateEditing(t_editbox *edb)
{
   if (edb->format == ISO8601_DATE) {
      if (IsStringDate(edb->text)) {
         *(time_t*)edb->item = edb->copy;
         TimeAsDate(edb->text, *(time_t *)edb->item);
      }
   }

   UnInstallKBHandler(EditBoxKeyboardCallback);
   edb->b->x = edb->x + edb->b->x1 + edb->b->dx1 + 1;
   edb->edit_in_progress = 0;
   edb->cursor = 0;
   edb->vstart = 0;
   edb->curpos = edb->text;
   ed_progr = NULL;
   _KillEventOfCgui(edb->blinkid);
   edb->blinkid = 0;
   edb->b->tf->Refresh(edb->b);
#ifdef ALLEGRO_UNIX
   if (edb->xclip) {
      close_X_clipboard(edb->xclip);
      edb->xclip = NULL;
   }
#endif
}

/* This function makes an update of the complete edit field by a call to the
   Refresh. First a name-case conversion must be done (if required), and the
   text will be evalueted. A patch of the "keyaction" value is done to avoid
   0 to be printed when empty string and the string represents a numeral. */
static void RedrawText(t_editbox *edb)
{
   t_object *b;
   int keyaction;

   b = edb->b;
   if (edb->keyaction & NAMECASE)
      NameCase(edb->text);
   DecodeText(edb);
   edb->usertyped = 1;
   b->tf->Refresh(b);
   edb->usertyped = 0;
}

/* This function controls the scrolling in the edit-box. This means: Using
   the current focus in the string determines which point in the string
   that shall be the start point to draw in the editbox. A position for the
   text-cursor is also calcualted. Components "vstart" and "x" is updated, by
   use of "curpos". */
static void SetTextStart(t_editbox *edb)
{
   char tmp, *s, *text;
   t_object *b;
   int maxx;

   text = edb->text;
   s = edb->curpos;
   b = edb->b;
   if (edb->bmp) {
      maxx = edb->bmp->w - 1;
      if (s - text <= edb->vstart) {
         edb->vstart = s - text;
         tmp = *s;
         *s = 0;
         edb->x = text_length(b->font, text + edb->vstart);
         *s = tmp;
      } else {
         do {
            tmp = *s;
            *s = 0;
            edb->x = text_length(b->font, text + edb->vstart);
            *s = tmp;
            if (edb->x > maxx && text[edb->vstart])
               edb->vstart++;
            else
               break;
         } while (1);
      }
   } else {
      edb->vstart = edb->x = 0;
   }
}

static char *PrevWord(char *s, char *text)
{
   if (s > text) {
      s--;
      while (s > text && *s == ' ')
         s--;
      while (s > text && *s != ' ')
         s--;
      if (s > text)
         s++;
   }
   return s;
}

static char *NextWord(char *s)
{
   if (*s) {
      while (*s && *s != ' ')
         s++;
      while (*s && *s == ' ')
         s++;
   }
   return s;
}

/* If text is marked: inserts text (replace) into the cgui clipboard If no
   text marked return 0 */
static int InsertIntoClipBoard(t_editbox *edb)
{
   char *s1, *s2;
   int n;

   if (edb->mark1 == edb->mark2)
      return 0;
   if (edb->mark1 > edb->mark2) {
      s1 = edb->mark2;
      s2 = edb->mark1;
   } else {
      s1 = edb->mark1;
      s2 = edb->mark2;
   }
   n = s2 - s1;
   clipboard = ResizeMem(char, clipboard, n + 1);

   strncpy(clipboard, s1, n);
   clipboard[n] = 0;
   return 1;
}

static char *Paste(t_editbox *edb, char *instxt)
{
   int n, n2, size;
   char *text, *s, *p;

   text = edb->text;
   s = edb->curpos;
   if (instxt) {
      n = strlen(instxt);
      n2 = s - text;
      if (edb->format == FPTRSTR) { /* unlimited */
         size = n + strlen(*edb->ptr) + 1;
         *edb->ptr = edb->text = text = ResizeMem(char, *edb->ptr, size);

         s = text + n2;
         /* copy rest of sting after c.p. to end of new memory */
         memmove(text + size - 1 - strlen(s), s, strlen(s) + 1);
         /* insert clipboard at current position */
         memmove(s, instxt, strlen(instxt));
         edb->curpos = s;
      } else {
         p = GetMem(char, n + edb->string_buffer_size + 1);

         strncpy(p, text, n2);
         strcpy(p + n2, instxt);
         strcat(p, s);
         p[edb->string_buffer_size-1] = 0;
         strncpy(text, p, edb->string_buffer_size);
         text[edb->string_buffer_size-1] = 0;
         Release(p);
      }
      edb->mark1 = edb->mark2 = NULL;
   }
   return s;
}

/** Cut a prefix from a string
   \pre s1 < s2
   \param s1 The beginning of the string
   \param s2 the end of the prefix (the first character not removed)  */
static void CutSubstring(char *s1, char *s2)
{
   memmove(s1, s2, strlen(s2)+1);
}

/* Deletes marked text */
static char *DeleteMarked(t_editbox *edb)
{
   char *s1, *s2;

   if (edb->mark1) {
      if (edb->mark1 > edb->mark2) {
         s1 = edb->mark2;
         s2 = edb->mark1;
      } else {
         s1 = edb->mark1;
         s2 = edb->mark2;
      }
      edb->mark1 = edb->mark2 = NULL;
      if (s1 != s2) {
         CutSubstring(s1, s2);
         edb->curpos = s1;
      }
   }
   return edb->curpos;
}

static char *ProcessPrintableLetter(t_editbox *edb)
{
   char *writepos_ptr, *text, buf[10];
   unsigned char half = HALF_CHAR;
   int ascii, nochar, newchars, ok, writepos_index, i;
   char date_buf[100];

   text = edb->text;
   writepos_ptr = edb->curpos;
   nochar = strlen(text);
   ascii = edb->ascii;
   ok = 0;
   switch (edb->format) {
   case FBYTE:
   case FSHORT:
   case FINT:
   case FLONG:
      if ((ascii == '-' || ascii == '+') && text == writepos_ptr)
         ok = 1;
      else if ((edb->keyaction & FUND) && (ascii == '?' && text == writepos_ptr))
         ok = 1;
      else if (isdigit(ascii))
         ok = 1;
      break;
   case FFLOAT:
   case FDOUBLE:
      if (isdigit(ascii) || ascii == decimal_point || (ascii == '-' && text == writepos_ptr))
         ok = 1;
      break;
   case FHEX1:
   case FHEX2:
   case FHEX4:
      if (isxdigit(ascii))
         ok = 1;
      break;
   case FOCT1:
   case FOCT2:
   case FOCT3:
   case FOCT4:
      if (ascii >= '0' && ascii <= '7')
         ok = 1;
      break;
   case FPOINTS:
   case FBPOINTS:
      if (isdigit(ascii)) {
         if (*writepos_ptr != '-' && (text == writepos_ptr || *(writepos_ptr - 1) != half))
            ok = 1;
      } else if (ascii == half) {
         /* accept if last, and the one to left is not half */
         if (*writepos_ptr == 0 && (text == writepos_ptr || *(writepos_ptr - 1) != half))
            ok = 1;
      } else if (ascii == '-' && text == writepos_ptr && *writepos_ptr != '-')
         ok = 1;
      else if ((edb->keyaction & FUND) && (ascii == '?' && text == writepos_ptr))
         ok = 1;
      break;
   case FSTRING:
   case FPTRSTR:
      ok = 1;
      break;
   case ISO8601_DATE:
      if (isdigit(ascii)) {
         ok = 1;
      }
      break;
   }
   if (ok) {
      if (_cgui_utf8_format_is_requested) {
         newchars = usetc(buf, ascii);
      } else {
         *buf = ascii;
         newchars = 1;
      }
      if (edb->format == FPTRSTR) {
         /* We need to make place for more letters. */
         writepos_index = writepos_ptr - text;
         *edb->ptr = edb->text = text = ResizeMem(char, text, nochar + newchars + 1);
         writepos_ptr = text + writepos_index;
      }
      if (((nochar < edb->string_buffer_size-newchars) || (edb->format == FPTRSTR)) && ok) {
         if (*writepos_ptr) {
            memmove(writepos_ptr + newchars, writepos_ptr, strlen(writepos_ptr) + 1);
         } else {
            /* Write position is at the end of the string. */
            writepos_ptr[newchars] = 0;
         }
         for (i=0; i<newchars; i++) {
            *writepos_ptr++ = buf[i];
         }
      }
   } else {
      ; /* No error handling. */
   }
   edb->curpos = writepos_ptr;
   edb->mark1 = edb->mark2 = NULL;
   return writepos_ptr;
}

static char *GetSelection(t_editbox *edb)
{
   char *s1;
   char *s2;
   char *selection;

   if (edb->mark1 == edb->mark2) {
      selection = NULL;
   } else {
      if (edb->mark1 > edb->mark2) {
         s1 = edb->mark2;
         s2 = edb->mark1;
      } else {
         s1 = edb->mark1;
         s2 = edb->mark2;
      }
      selection = GetMem0(char, s2 - s1 + 1);
      strncpy(selection, s1, s2 - s1);
   }
   return selection;
}
/* return true if fail */
static int CopyToGlobalBuffer(t_editbox *edb)
{
   char *selection;
   int ok;
   if ((selection = GetSelection(edb))) {
      ok = InsertIntoSystemsClipboard(selection);
      free(selection);
   } else {
      ok = 0;
   }
   return ok;
}

/* return true if fail */
static int CopyFromWinClip(t_editbox *edb)
{
   char *s = NULL;

#ifdef DJGPP
   s = GetFromWinClip();
#else
   #ifdef ALLEGRO_UNIX
   s = get_X_clipboard(edb->xclip);
   #else
   if (clipboard)
      s = MkString(clipboard);
   #endif
#endif
   if (s) {
      Paste(edb, s);
      Release(s);
      return 0;
   } else
      return 1;
}

/* Processes the key-press as found in edb->ascii and edb->scan considering also the allegro key array
   and the KB_NUMLOCK_FLAG flag from key_shifts (seems that the latter is not possible to obtain in
   any other way).
   Tries to detect it is a editing command key or a just typing a letter. In case of command, handled
   immediately, while the processing of printable letters is passed on to a helper function.
   In general we try to mimic what we beleve is some kind of "standard windowed environment behavior"
   when interpreting commands. */
static int ProcessKeyPress(t_editbox *edb)
{
   char *input_position;
   char *cut_selection_end;
   char *text;
   int ascii, scan, error = 0;
   int input_position_at_end;
   int input_position_at_beginning;
   int accept_keypress = 1;
   int keypad_printable_key = 0;
   int main_keyboard_printable_key = 0;
   int unhandled_modifiers;
   int numlock;
   int shift;
   int ctrl;

   text = edb->text;
   ascii = edb->ascii;
   scan = edb->scan;
   input_position = edb->curpos;
   input_position_at_end = input_position >= text + strlen(text);
   input_position_at_beginning = input_position <= text;
   numlock = key_shifts & KB_NUMLOCK_FLAG;
   shift = key[KEY_LSHIFT] || key[KEY_RSHIFT];
   ctrl = key[KEY_LCONTROL] || key[KEY_RCONTROL];
   unhandled_modifiers = key[KEY_LWIN] || key[KEY_RWIN] || key[KEY_MENU];

   if (!unhandled_modifiers) {
      if (key[KEY_ALT]) {
         accept_keypress = 0;
      } else {
         accept_keypress = 1;
         if (numlock) {
            /* Then the below keys should be interpreted as printable letters, because we don't handle any
               combinations with keypad-keys and alt or ctrl. */
            switch (scan) {
            case KEY_DEL_PAD:
            case KEY_0_PAD:
            case KEY_1_PAD:
            case KEY_2_PAD:
            case KEY_3_PAD:
            case KEY_4_PAD:
            case KEY_5_PAD:
            case KEY_6_PAD:
            case KEY_7_PAD:
            case KEY_8_PAD:
            case KEY_9_PAD:
               keypad_printable_key = 1;
               break;
            }
         }
         if (!keypad_printable_key) {
            switch (scan) {
            case KEY_BACKSPACE:
               if (!input_position_at_beginning) {
                  if (!persistent_selection && edb->mark1 && edb->mark2 && (edb->mark1 != edb->mark2)) {
                     input_position = DeleteMarked(edb);
                  }
                  /* Input might have moved to the beginning. */
                  input_position_at_beginning = input_position <= text;
                  if (!input_position_at_beginning) {
                     if (shift && ctrl) {
                     } else if (ctrl) {
                        cut_selection_end = input_position;
                        input_position = PrevWord(input_position, text);
                        CutSubstring(input_position, cut_selection_end);
                     } else {
                        if (edb->format == ISO8601_DATE && input_position[-1] == '-') {
                           /* Don't allow deleting a - . */
                        } else {
                           input_position--;
                           CutSubstring(input_position, input_position + 1);
                        }
                     }
                  }
               }
               break;

            case KEY_LEFT:
            case KEY_4_PAD:
               if (input_position_at_beginning && !persistent_selection) {
                  if (!shift) {
                     edb->mark1 = edb->mark2 = NULL;
                  }
               } else {
                  if (shift && ctrl) {
                     /* Ctrl-shift-left arrow: extend selection to include prev word */
                     if (edb->mark1 == NULL) {
                        edb->mark1 = input_position;
                     }
                     input_position = PrevWord(input_position, text);
                     edb->mark2 = input_position;
                  } else if (shift) {
                     /* Shift-Left-arrow: extend selection to include prev letter */
                     if (edb->mark1 == NULL) {
                        edb->mark1 = input_position;
                     }
                     input_position--;
                     edb->mark2 = input_position;
                  } else if (ctrl) {
                     /* Ctrl-left: Move to prev word */
                     input_position = PrevWord(input_position, text);
                     edb->mark1 = edb->mark2 = NULL;
                  } else {
                     /* Left: move one letter to the left */
                     edb->mark1 = edb->mark2 = NULL;
                     input_position--;
                  }
               }
               break;

            case KEY_RIGHT:
            case KEY_6_PAD:
               if (input_position_at_end && !persistent_selection) {
                  if (!shift) {
                     edb->mark1 = edb->mark2 = NULL;
                  }
               } else {
                  if (shift && ctrl) {
                     /* Ctrl-shift-right arrow: extend selection to include next word */
                     if (edb->mark1 == NULL) {
                        edb->mark1 = input_position;
                     }
                     input_position = NextWord(input_position);
                     edb->mark2 = input_position;
                  } else if (shift) {
                     /* Shift-Right-arrow: extend selection to include next letter */
                     if (edb->mark1 == NULL) {
                        edb->mark1 = input_position;
                     }
                     input_position++;
                     edb->mark2 = input_position;
                  } else if (ctrl) {
                     /* Ctrl-right: Move to next word */
                     input_position = NextWord(input_position);
                     edb->mark1 = edb->mark2 = NULL;
                  } else {
                     /* Right: move one letter to the right */
                     edb->mark1 = edb->mark2 = NULL;
                     input_position++;
                  }
               }
               break;

            case KEY_END:
            case KEY_1_PAD:
               if (input_position_at_end && !persistent_selection) {
                  if (!shift) {
                     edb->mark1 = edb->mark2 = NULL;
                  }
               } else {
                  if (shift && ctrl) {
                  } else if (shift) {
                     /* Shift-End: extend selection to include rest of the line */
                     if (*input_position) {
                        if (edb->mark1 == NULL) {
                           edb->mark1 = input_position;
                        }
                        input_position += strlen(input_position);
                        edb->mark2 = input_position;
                     }
                  } else if (ctrl) {
                  } else {
                     /* Left: move to the end */
                     edb->mark1 = edb->mark2 = NULL;
                     input_position += strlen(input_position);
                  }
               }
               break;

            case KEY_HOME:
            case KEY_7_PAD:
               if (input_position_at_beginning && !persistent_selection) {
                  if (!shift) {
                     edb->mark1 = edb->mark2 = NULL;
                  }
               } else {
                  if (shift && ctrl) {
                  } else if (shift) {
                     /* Shift-Home: extend selection to include all from the beginning of the line */
                     if (edb->mark1 == NULL) {
                        edb->mark1 = input_position;
                     }
                     input_position = text;
                     edb->mark2 = input_position;
                  } else if (ctrl) {
                  } else {
                     /* Left: move to the start position */
                     edb->mark1 = edb->mark2 = NULL;
                     input_position = text;
                  }
               }
               break;

            case KEY_UP:
               if (shift && ctrl) {
               } else if (shift) {
               } else if (ctrl) {
               } else {
                  /* Up: increment if it is an integer item else ignore */
                  edb->mark1 = edb->mark2 = NULL;
                  IncrementItem(edb);
                  FormatItem2Text(edb);
               }
               break;

            case KEY_DOWN:
               if (shift && ctrl) {
               } else if (shift) {
               } else if (ctrl) {
               } else {
                  /* Down: decrement if it is an integer item else ignore */
                  edb->mark1 = edb->mark2 = NULL;
                  DecrementItem(edb);
                  FormatItem2Text(edb);
               }
               break;

            case KEY_INSERT:
               if (shift && ctrl) {
               } else if (shift) {
                  /* Shift-Insert: paste from local buffer */
                  if (!persistent_selection) {
                     input_position = DeleteMarked(edb);
                  }
                  edb->mark1 = edb->mark2 = NULL;
                  input_position = Paste(edb, clipboard);
                  text = edb->text;
               } else if (ctrl) {
                  /* Ctrl-insert: copy selection to local buffer */
                  InsertIntoClipBoard(edb);
               } else {
               }
               break;

            case KEY_DEL_PAD:
            case KEY_DEL:
               if (input_position_at_end && !persistent_selection) {
                  input_position = DeleteMarked(edb);
               } else {
                  if (!persistent_selection && edb->mark1 && edb->mark2 && (edb->mark1 != edb->mark2)) {
                     input_position = DeleteMarked(edb);
                  }
                  /* Might be at the end now */
                  input_position_at_end = input_position >= text + strlen(text);
                  if (!input_position_at_end) {
                     if (shift && ctrl) {
                     } else if (shift) {
                        /* Shift-delete: Cut to local buffer */
                        if (InsertIntoClipBoard(edb)) {
                           input_position = DeleteMarked(edb);
                        } else {
                           error = 1;
                        }
                     } else if (ctrl) {
                        /* Ctrl-delete: Delete next word. */
                        CutSubstring(input_position, NextWord(input_position));
                     } else {
                        if (edb->format == ISO8601_DATE && *input_position == '-') {
                           /* Don't allow deleting a - . */
                        } else {
                           CutSubstring(input_position, input_position + 1);
                        }
                     }
                  }
               }
               break;

            case KEY_TAB:
               edb->mark1 = edb->mark2 = NULL;
               accept_keypress = 0;
               break;

            case KEY_ENTER:
            case KEY_ENTER_PAD:
               edb->mark1 = edb->mark2 = NULL;
               if ((edb->keyaction & CR_TAB) && scan != TERMINATE_EDIT) {
                  simulate_keypress((KEY_TAB<<8)|'\t');
               } else {
                  TerminateEditing(edb);
               }
               break;

            case KEY_ESC:
               RestoreItem(edb);
               edb->mark1 = edb->mark2 = NULL;
               TerminateEditing(edb);
               break;

            default:
               /* Might be a key corresponding to a printable letter. */
               if (ctrl) {
                  /* Handle all possible ctrl-printable letter (which is not printable, but a command. */
                  switch (ascii) {
                  case KEY_C:
                     /* Ctrl-C: copy selection to Windows clipboard */
                     if (!CopyToGlobalBuffer(edb)) {
                        error = 1;
                     }
                     break;
                  case KEY_V:
                     /* Ctrl-V: paste from Windows clipboard */
                     if (!persistent_selection) {
                        input_position = DeleteMarked(edb);
                     }
                     edb->mark1 = edb->mark2 = NULL;
                     error = CopyFromWinClip(edb);
                     input_position = edb->curpos;
                     break;
                  case KEY_X:
                     /* Ctrl-X: cut selection to Windows clipboard */
                     if (CopyToGlobalBuffer(edb)) {
                        input_position = DeleteMarked(edb);
                     } else {
                        error = 1;
                     }
                     break;
                  }
               }
               if (ascii >= 32) {
                  main_keyboard_printable_key = 1;
               }
               break;
            }
         }
         if (main_keyboard_printable_key || keypad_printable_key) {
            /* Normal case: Insert the corrsponding letter into the string.
               If either of no modifier, shift- or altgr modifier then it's for sure a "normal key" if the ascii is >= 32
               and we can safely issue a letter. */
            if (!persistent_selection) {
               input_position = DeleteMarked(edb);
            }
            edb->mark1 = edb->mark2 = NULL;
            input_position = ProcessPrintableLetter(edb);
         }
      }
   }
   if (error) {
      ; /* No error handling. */
   }
   edb->curpos = input_position;
   SetTextStart(edb);
   RedrawText(edb);
   return accept_keypress;
}

extern int EditBoxKeyboardCallback(void *data, int scan, int ascii)
{
   t_editbox *edb = data;

   edb->scan = scan;
   edb->ascii = ascii;
   if (edb->ComboAction && (scan == KEY_DOWN || scan == KEY_UP)) {
      TerminateEditing(edb);
      edb->ComboAction(edb->combo_box);
      edb->b->Action(edb);
   }
   if (edb->Action && ed_callback_progr == NULL) {
      ed_callback_progr = edb;
      edb->Action(edb->data);
      ed_callback_progr = NULL;
   }
   return ProcessKeyPress(edb);
}

/* This function uses the mouse cursor x-position (found in the object) to
   calculate a position within the string and also converts to local edit-
   box coordinate */
static void SetCurrentPosition(t_editbox *edb)
{
   t_object *b;
   char *s, *p;
   int x, tl, x1, x2;

   b = edb->b;
   s = edb->text + edb->vstart;
   /* adjust to local edit bitmap coordinates */
   x = b->x - (b->x1 + b->dx1 + 1);
   if (b->usedbutton == 0) /* indicates initial state - move cursor to end */
      x = text_length(b->font, s);

   /* Search for possible positions between letters to find the one that is
      as close as possible to the actual cursor position */
   x1 = x2 = tl = text_length(b->font, s);   /* Start at right end of text */
   if (x >= tl) {
      x = tl;
      p = s + strlen(s);
   } else {
      for (p = s + strlen(s); x1 > x && p > s;) {
         x2 = x1;
         x1 = tl - text_length(b->font, --p);
      }
      if (x2 - x < x - x1) {
         x = x2;
         p++;
      } else {
         x = x1;
      }
   }

   /* Store the text cursor position in edit-box coordinates */
   edb->x = x;
   edb->curpos = p;
}

static void StartEdit(t_editbox *edb)
{
   if (ed_progr) {
      TerminateEditing(ed_progr);
      SetCurrentPosition(edb);
      TextCursorBlink(edb);
   }
   ed_progr = edb;
   FormatItem2Text(edb);
   edb->edit_in_progress = 1;
   SaveCopy(edb);
   InstallKBHandler(EditBoxKeyboardCallback, edb);
   if (edb->curpos == NULL) {
      edb->curpos = edb->text;
   }
#ifdef ALLEGRO_UNIX
   if (edb->xclip == NULL) {
      edb->xclip = init_X_clipboard();
   }
#endif
}

static void EditMenu(void *data)
{
   t_editbox *edb = data;
   int isclipped, ismarked;

   isclipped = clipboard && *clipboard;
   ismarked = edb->mark1 != edb->mark2;
   CreateEditBoxMenu(edb, isclipped, ismarked);
   if (edb->curpos == NULL) {
      edb->curpos = edb->text;
   }
}

static void Action(void *data)
{
   t_editbox *edb = data;

   if (edb->b->usedbutton == RIGHT_MOUSE) {
      MkScratchMenu(edb->b->id, EditMenu, edb);
#ifdef ALLEGRO_UNIX
   } else if (edb->b->usedbutton == MID_MOUSE) {
      char *s = get_X_primary(edb->xclip);
      if (*s) {
         Paste(edb, s);
         Release(s);
      }
#endif
   } else if (!edb->edit_in_progress) {
      TextCursorBlink(edb);
      SetCurrentPosition(edb);
      StartEdit(edb);
   }
}

extern void DrawLeftSidedImage(t_object *b, int x1, int x2, int offset)
{
   int y1, y2, yt, col, diff, imy1, h, xh, yh, xh2;
   BITMAP *bmp;

   bmp = b->parent->bmp;
   y1 = b->y1;
   y2 = b->y2;
   yt = TEXTY(b, 0);
   if (b->inactive)
      col = cgui_colors[CGUI_COLOR_LABEL_INACTIVE_1];
   else if (b->hasfocus) {
      if (b->hasfocus == 2)
         col = cgui_colors[CGUI_COLOR_LABEL_HIDDEN_FOCUS];
      else
         col = cgui_colors[CGUI_COLOR_LABEL_FOCUS];
   } else
      col = cgui_colors[CGUI_COLOR_LABEL];

   if (bmp) {
      rectfill(bmp, x1, y1, x2, y2, cgui_colors[CGUI_COLOR_WIDGET_BACKGROUND]);
      x1 += offset;
      if (b->im) {
         diff = (y2 - y1 + 1) - b->im->h;
         imy1 = y1 + diff/2 + offset;
         if (diff > 0)
            diff = 0;
         h = b->im->h + diff;
         set_clip_rect(bmp, x1, y1, x2, y2);
         DrawImage(b->im, bmp, x1, imy1 + diff);
         set_clip_rect(bmp, 0, 0, bmp->w - 1, bmp->h - 1);
         x1 += b->im->w + 1;
      }
      if (b->inactive) {
         textout_ex(bmp, b->font, b->label, x1 + 1, yt + offset + 1, cgui_colors[CGUI_COLOR_LABEL_INACTIVE_2], -1);
      }
      textout_ex(bmp, b->font, b->label, x1, yt + offset, col, -1);
      if (b->hklen) {
         xh = x1 + b->hkxoffs;
         xh2 = xh + b->hklen - 1;
         yh = _cgui_hot_key_line + yt + offset;
         hline(bmp, xh, yh, xh2, col);
      }
   }
}

/* Draws all static parts of an selectable box (e.g. an edit-box or drop-down
   box). This includes the frame, the focus-highlight and the label-parts. */
extern void DrawBoxFrame(t_object *b)
{
   int x1, x2, y1, y2, col;
   t_node *nd;

   nd = b->parent;
   x1 = b->x1 + b->dx1 - INPUTBORDER;
   x2 = b->x2;
   y1 = b->y1;
   y2 = b->y2;

   DrawLeftSidedImage(b, b->x1, x1, 0);

   hline(nd->bmp, x1, y1, x2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   hline(nd->bmp, x1 + 1, y1 + 1, x2 - 1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);

   vline(nd->bmp, x1, y1 + 1, y2, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);
   vline(nd->bmp, x1 + 1, y1 + 2, y2 - 1, cgui_colors[CGUI_COLOR_SHADOWED_BORDER]);

   hline(nd->bmp, x1 + 1, y2, x2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   hline(nd->bmp, x1 + 2, y2 - 1, x2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);

   vline(nd->bmp, x2, y1 + 1, y2 - 2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   vline(nd->bmp, x2 - 1, y1 + 2, y2 - 2, cgui_colors[CGUI_COLOR_LIGHTENED_BORDER]);
   if (b->hasfocus) {
      if (b->hasfocus == 2)
         col = cgui_colors[CGUI_COLOR_LABEL_HIDDEN_FOCUS];
      else
         col = cgui_colors[CGUI_COLOR_BUTTON_FRAME_FOCUS];
   } else
      col = cgui_colors[CGUI_COLOR_EDITBOX_BACKGROUND];
   rect(nd->bmp, x1 + 2, y1 + 2, x2 - 2, y2 - 2, col);
}

static void DrawEditField(void *event)
{
   t_object *b = event;
   t_editbox *edb = b->appdata;
   int select_startx = -1, select_endx = -1, curs_col;
   int drawcol, bcol, x2, y1, y2, l1, l2;
   char *tmp, c, *text, *s1, *s2;

   if (edb->bmp == NULL)
      /* This may be the case e.g. if the editbox is placed outside the node */
      return;
   if (b->inactive) {
      drawcol = cgui_colors[CGUI_COLOR_EDITBOX_TEXT_INACTIVE];
      bcol = cgui_colors[CGUI_COLOR_EDITBOX_BACKGROUND_INACTIVE];
   } else {
      drawcol = cgui_colors[CGUI_COLOR_EDITBOX_TEXT];
      bcol = cgui_colors[CGUI_COLOR_EDITBOX_BACKGROUND];
   }
   if (!edb->edit_in_progress)
      FormatItem2Text(edb);

   text = edb->text + edb->vstart;
   s1 = edb->mark1;
   s2 = edb->mark2;
   x2 = edb->bmp->w - 1;
   y2 = edb->bmp->h - 1;
   y1 = (edb->bmp->h - text_height(b->font)) / 2;
   clear_to_color(edb->bmp, bcol);
   if (s1 && s2) {
      if (s1 > s2) {
         tmp = s2;
         s2 = s1;
         s1 = tmp;
      }
      if (s1 != s2) {
         if (s1 > text) {
            c = *s1;
            *s1 = 0;
            l1 = text_length(b->font, text);
            *s1 = c;
         } else {
            l1 = 0;
            s1 = text;
         }
         c = *s2;
         *s2 = 0;
         l2 = text_length(b->font, s1);
         *s2 = c;
         select_startx = 1 + l1;
         select_endx = l1 + l2;
         rectfill(edb->bmp, select_startx, 0, select_endx, y2, cgui_colors[CGUI_COLOR_EDITBOX_BACKGROUND_MARK]);
      }
   }
   textout_ex(edb->bmp, b->font, text, 1, y1, drawcol, -1);

   if (edb->cursor) {
      if (edb->x >= select_startx && edb->x <= select_endx ) {
         curs_col = cgui_colors[CGUI_COLOR_EDITBOX_TEXT_MARK];
      } else {
         curs_col = drawcol;
      }
      vline(edb->bmp, edb->x, y1, y1 + text_height(b->font) - 2, curs_col);
   }
}

static void DrawEdBox(t_object * b)
{
   if (b->parent->bmp == NULL)
      return;
   DrawBoxFrame(b);
   DrawEditField(b);
}

extern void SetBoxSize(t_object *b, int width)
{
   if (b->dire)
      b->x1 = b->y1 = 0;
   /* 1 space between text and frame (for the focus highlight) */
   b->dx1 = text_length(b->font, b->label) + INPUTBORDER + 1;
   if (b->im)
      b->dx1 += b->im->w + 1;
   b->y2 = b->y1 + _cgui_button1_height - 1 + b->rey;
   b->dy1 = INPUTBORDER;
   b->dx2 = INPUTBORDER;
   b->dy2 = INPUTBORDER;
   b->x2 = b->x1 + b->dx1 + width + INPUTBORDER + b->rex;
}

static void SetSize(t_object * b)
{
   t_editbox *edb;

   edb = b->appdata;
   SetBoxSize(b, edb->width);
}

static void MakeEdit(t_object * b, int format, int string_buffer_size, void *item, int width)
{
   t_editbox *edb;

   edb = GetMem0(t_editbox, 1);
   edb->width = width;
   edb->keyaction = TAB_TAB;
   if (format & FBLANK0) {
      edb->keyaction |= BL0;
      format &= ~FBLANK0;
   }
   if (format & FUNDEF) {
      edb->keyaction |= FUND;
      format &= ~FUNDEF;
   }
   if (format & FNAME) {
      edb->keyaction |= NAMECASE;
      format &= ~FNAME;
   }
   edb->format = format;
   edb->item = item;
   edb->b = b;
   if (format == FSTRING) {
      edb->string_buffer_size = string_buffer_size;
      edb->text = edb->item;
   } else if (format == FPTRSTR) {
      edb->string_buffer_size = 0;               /* arbitrary */
      edb->ptr = edb->item;
      if (*edb->ptr == NULL)
         *edb->ptr = edb->text = GetMem0(char, 1);
      edb->text = *edb->ptr;
   } else {
      edb->string_buffer_size = SIZE_OF_NUMBER_STRING;
      edb->textcopy = GetMem0(char, SIZE_OF_NUMBER_STRING+2);  /* will be enough to contain any number */
      edb->text = edb->textcopy;
   }
   b->appdata = edb;
   b->Action = Action;
}

static int AddEditBoxHandler(t_object * b, void (*Handler) (void *data),
                             void *data)
{
   t_editbox *edb;

   edb = b->appdata;
   edb->Action = Handler;
   edb->data = data;
   return 1;
}


static BITMAP *MakeEditBitmap(t_object * b)
{
   t_node *nd;
   BITMAP *bmp = NULL;
   int x, y, width, height;

   nd = b->parent;
   x = b->x1 + b->dx1 + 1;
   y = b->y1 + TEXTOFFSETY;
   width = (b->x2 - b->dx2 - 1) - (b->x1 + b->dx1 + 1) + 1;
   height = (b->y2 - b->dy2 - 1) - (b->y1 + b->dy1 + 1) + 1;
   if (nd && nd->bmp && x < nd->bmp->w && y < nd->bmp->h && width > 0 &&
          height > 0)
      bmp = create_sub_bitmap(nd->bmp, x, y, width, height);
   return bmp;
}

static void MakeEdBitmap(t_object * b)
{
   t_editbox *edb;

   edb = b->appdata;
   if (edb->bmp && (bitmap_color_depth(edb->bmp) != bitmap_color_depth(screen)
      || SCREEN_H != edb->curh || SCREEN_W != edb->curw)) {
      /* SCREEN_H != edb->curh || SCREEN_W != edb->curw || b->x2-b->x1+1 !=
         edb->bmp->w || b->y2-b->y1+1 != edb->bmp->h)) { */
      destroy_bitmap(edb->bmp);
   }
   edb->bmp = MakeEditBitmap(b);
}

static void CleanupEdit(void *data nouse)
{
   if (clipboard) {
      Release(clipboard);
      clipboard = NULL;
   }
}

static void FreeEdit(t_object *b)
{
   t_editbox *edb;

   edb = b->appdata;
   if (ed_progr && ed_progr == edb) {
      TerminateEditing(ed_progr);
   }
   destroy_bitmap(edb->bmp);
   Release(edb->textcopy);
   XtendedFree(b);
}

static void EditSetFocus(t_object *b)
{
   t_editbox *edb;

   if (b->parent->bmp == NULL)
      return;
   edb = b->appdata;
   if (edb->edit_in_progress) {
      _KillEventOfCgui(edb->blinkid);
      edb->blinkid = 0;
   }
   if (edb->Action) {
      edb->Action(edb->data);
   }
   FormatItem2Text(edb);
   SetCurrentPosition(edb);
   SetTextStart(edb);
   TextCursorBlink(edb);
   StartEdit(edb);
}

static void EditUnSetFocus(t_object *b)
{
   t_editbox *edb;

   edb = b->appdata;
   if (edb->Action) {
      edb->Action(edb->data);
   }
   TerminateEditing(b->appdata);
}

static void StopScrollEditor(t_editbox *edb)
{
   if (edb->scrolling) {
      _KillEventOfCgui(edb->scrollid);
      edb->scrollid = 0;
      edb->scrolling = 0;
   }
}

static void ScrollEdit(void *);
static void NextScroll(int diff, t_editbox *edb)
{
   int delay;

   if (diff < 10)
      diff = 10;
   delay = 2000 / diff;
   edb->scrollid = _GenEventOfCgui(ScrollEdit, edb, delay, edb->b->id);
}

static void ScrollEdit(void *data)
{
   t_editbox *edb = data;
   t_object *b;
   int start, end;

   b = edb->b;
   end = b->x2 - b->dx2 - 1;
   start = b->x1 + b->dx1 + 1;
   if (b->x > end) {
      if (*edb->curpos) {
         edb->curpos++;
         edb->mark2 = edb->curpos;
         SetTextStart(edb);
         b->tf->Refresh(b);
      }
      NextScroll(b->x - end, edb);
   } else if (b->x < start) {
      if (edb->curpos > edb->text) {
         edb->curpos--;
         edb->mark2 = edb->curpos;
         SetTextStart(edb);
         b->tf->Refresh(b);
      }
      NextScroll(start - b->x, edb);
   } else
      edb->scrolling = 0;
}

static void StartScrollEditor(t_editbox *edb)
{
   if (!edb->scrolling) {
      edb->scrolling = 1;
      ScrollEdit(edb);
   }
}

static int InsertIntoBuffer(t_editbox *edb)
{
   char *selection;
   int ok;
   selection = GetSelection(edb);
   ok = InsertIntoPrimaryBuffer(selection);
   free(selection);
   return ok;
}

static int Slide(int x nouse, int y nouse, void *src, int id nouse,
                 int reason)
{
   t_object *b = src;
   t_editbox *edb;

   edb = b->appdata;
   switch (reason) {
   case SL_STARTED:
      b->tcfun->MoveFocusTo(b);
      SetCurrentPosition(edb);
      edb->mark1 = edb->mark2 = edb->curpos;
      SetTextStart(edb);
      break;
   case SL_PROGRESS:
      if (b->x > b->x2 - b->dx2 - 1) {
         StartScrollEditor(edb);
      } else if (b->x < b->x1 + b->dx1 + 1) {
         StartScrollEditor(edb);
      } else {
         StopScrollEditor(edb);
         SetCurrentPosition(edb);
         SetTextStart(edb);
         if (edb->mark2 != edb->curpos) {
            edb->mark2 = edb->curpos;
            b->tf->Refresh(b);
         }
      }
      InsertIntoBuffer(edb);
      break;
   case SL_STOPPED:
      StopScrollEditor(edb);
      if (edb->mark2 != edb->curpos) {
         edb->mark2 = edb->curpos;
         b->tf->Refresh(b);
      } else {
         StartEdit(edb);
      }
      break;
   default:
      return 1;
   }
   return 1;
}

extern void CguiInitEditBox(void)
{
   char buf[10] = {0};
   sprintf(buf, "%0.1lf", 0.0);
   decimal_point = buf[1];
}

/* Application interface: */
extern void NameCase(char *text)
{
   *text = ToUpper(*text);
   while (*text) {
      if (IsDelimiter(*text++))
         *text = ToUpper(*text);
      else
         *text = ToLower(*text);
   }
}

extern void GetEditData(int *scan, int *ascii, int *offset)
{
   if (ed_callback_progr) {
      *scan = ed_callback_progr->scan;
      *ascii = ed_callback_progr->ascii;
      *offset = ed_callback_progr->curpos - ed_callback_progr->text;
   } else {
      *scan = 0;
      *ascii = 0;
      *offset = 0;
   }
}

extern void SetEditData(int scan, int ascii, int offset)
{
   if (ed_callback_progr) {
      ed_callback_progr->scan = scan;
      ed_callback_progr->ascii = ascii;
      if (offset < ed_callback_progr->string_buffer_size-1) {
         ed_callback_progr->curpos = ed_callback_progr->text + offset;
      }
   }
}

extern int TabOnCR(int id)
{
   t_editbox *edb;
   t_object *b;

   b = GetObject(id);
   if (b && b->Action == Action) {
      edb = b->appdata;
      edb->keyaction |= CR_TAB;
      return 1;
   }
   return 0;
}

extern void EditBoxUpdateString(t_editbox *edb, const char *string)
{
   if (edb->format == FPTRSTR) {
      *edb->ptr = edb->text = ResizeMem(char, edb->text, strlen(string) + 1);
   }
   strncpy(edb->text, string, edb->string_buffer_size);
}

extern t_editbox *EditBoxAttachComboProperty(int id, void (*ComboAction)(t_combo_box *), t_combo_box *combo_box)
{
   t_editbox *edb;
   t_object *b;

   b = GetObject(id);
   if (b && b->Action == Action) {
      edb = b->appdata;
      edb->ComboAction = ComboAction;
      edb->combo_box = combo_box;
   } else {
      edb = NULL;
   }
   return edb;
}

static void EditBoxRefresh(struct t_object *b)
{
   t_editbox *edb;

   edb = b->appdata;
   if (edb->edit_in_progress)
      if (edb->format == FPTRSTR && edb->text != *edb->ptr)
         edb->text = *edb->ptr;
   DefaultRefresh(b);
}

extern void CguiEditBoxSetSelectionMode(int mode)
{
   persistent_selection = !mode;
}

extern int CguiEditBoxSelectAll(int id)
{
   t_editbox *edb;
   t_object *b;

   b = GetObject(id);
   if (b && b->Action == Action) {
      edb = b->appdata;
      edb->mark1 = edb->text;
      edb->mark2 = edb->text + strlen(edb->text);
      return 1;
   }
   return 0;
}

extern int CguiEditBoxSetFloatDecimals(int id, int max_nr_of_decimals)
{
   t_editbox *edb;
   t_object *b;

   b = GetObject(id);
   if (b && b->Action == Action) {
      edb = b->appdata;
      if (edb->format == FFLOAT || edb->format == FDOUBLE) {
         edb->is_max_nr_of_decimals = 1;
         edb->max_nr_of_decimals = max_nr_of_decimals;
         return 1;
      }
   }
   return 0;
}

extern int AddEditBox(int x, int y, int width, const char *label, int format,
                      int string_buffer_size, void *data)
{
   t_object *b;
   static t_typefun tf;
   static int virgin = 1;

   b = CreateObject(x, y, opwin->win->opnode);
   InsertLabel(b, label);
   b->click = LEFT_MOUSE|RIGHT_MOUSE|MID_MOUSE;
   MakeEdit(b, format, string_buffer_size, data, width);
   SetObjectSlidable(b->id, Slide, LEFT_MOUSE, b);
   if (b->tablink == NULL)
      b->tf->DoJoinTabChain(b);
   if (virgin) {
      virgin = 0;
      tf = default_type_functions;
      tf.Draw = DrawEdBox;
      tf.AddHandler = AddEditBoxHandler;
      tf.Free = FreeEdit;
      tf.SetSize = SetSize;
      tf.MakeBitmap = MakeEdBitmap;
      tf.MakeStretchable = MakeNodeStretchable;
      tf.Refresh = EditBoxRefresh;
      tf.SetFocus = EditSetFocus;
      tf.UnSetFocus = EditUnSetFocus;
      HookCguiDeInit(CleanupEdit, NULL);
   }
   b->tf = &tf;
   return b->id;
}

