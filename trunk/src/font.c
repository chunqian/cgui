/* Module font.c
   Do some handlig of fonts that are needed in cgui.

   Calculates the baseline of the font. This may be useful to know e.g. when
   underlining text (for short-cuts in labels)
*/

#include <allegro.h>
#include <allegro/internal/aintern.h>

#include "cgui.h"
#include "cgui/mem.h"
#include "object.h"
#include "font.h"
#include "memint.h"
#include "fontdata.h"

#if ALLEGRO_SUB_VERSION == 0
#define textout_ex(bmp, f, s, x, y, col, bgcol) textout(bmp, f, s, x, y, col)
#endif

FONT *_cgui_prop_font;
static FONT *cgui_fix_font;
static FONT *orgfont;

extern int GetBaseLine(FONT *f)
{
   BITMAP *bmp;
   int x, y, w, h, base;

   if (f == NULL)
      return 5;
   w = text_length(f, "E");
   h = text_height(f);
   if (h <=0 || w <= 0)
      return 0;
   bmp = create_bitmap(w, h);
   if (bmp == NULL)
      return 0;
   clear_to_color(bmp, 0);
   textout_ex(bmp, f, "E", 0, 0, 255, -1);
   for (y = bmp->h - 1; y >= 0; y--) {
      for (x = 0; x < bmp->w; x++) {
         if (getpixel(bmp, x, y)) {
            destroy_bitmap(bmp);
            return y + 1;
         }
      }
   }
   base = bmp->h - 3;
   destroy_bitmap(bmp);
   return base;
}

extern void InitFont(void)
{
   static int virgin = 1;
   if (cgui_started)
      return;
   if (virgin) {
      virgin = 0;
      fixup_datafile(_cgui_fonts_datafile);
   }
   _cgui_prop_font = _cgui_fonts_datafile[_CGUI_FONT].dat;
   cgui_fix_font = _cgui_fonts_datafile[_CGUI_FIX_FONT].dat;
   orgfont = _cgui_prop_font;
   SetCguiFont(_cgui_prop_font);
}

extern FONT *GetCguiFont(void)
{
   if (!cgui_started)
      InitFont();
   return _cgui_prop_font;
}

extern FONT *GetCguiFixFont(void)
{
   return cgui_fix_font;
}

extern void SetCguiFont(FONT *f)
{
   int base_line;

   if (f == NULL) {
      _cgui_prop_font = orgfont;
   } else {
      _cgui_prop_font = f;
   }
   base_line = GetBaseLine(_cgui_prop_font);
   _cgui_hot_key_line = base_line + 2;
   _cgui_button1_height = text_height(_cgui_prop_font) + 2 * TEXTOFFSETY + 1;
}

extern void CguiSetBaseLine(int base_line)
{
   _cgui_hot_key_line = base_line + 1;
}
