/* src/cursdata.c
 * 
 *  Source for data encoded from resource/cursors.dat .
 *  Do not hand edit.
 */

#include <allegro.h>
#include <allegro/internal/aintern.h>



static DATAFILE_PROPERTY curs_illegal_prop[] = {
    { "11-08-2002, 20:24", DAT_ID('D', 'A', 'T', 'E') },
    { "8", DAT_ID('H', 'S', '_', 'X') },
    { "8", DAT_ID('H', 'S', '_', 'Y') },
    { "illegal", DAT_ID('N', 'A', 'M', 'E') },
    { "stop.pcx", DAT_ID('O', 'R', 'I', 'G') },
    { 0, DAT_END }
};

static unsigned char curs_illegal_lines[] = 
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f\0\x1f\0\x1f\0\x1f\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f\0\x1f\0\0|\0|\0|\0|\x1f\0\x1f\0\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f\0\0|\0|\0|\0|\0|\0|\0|\0|\x1f\0\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f\0\x1f\0\x1f\0\0|\0|\0|\0|\0|\0|\0|\0|\x1f\0\x1f|\x1f|"
"\x1f|\x1f\0\0|\0|\x1f\0\x1f\0\0|\0|\0|\0|\0|\0|\0|\0|\x1f\0\x1f|"
"\x1f|\x1f\0\0|\0|\0|\x1f\0\x1f\0\0|\0|\0|\0|\0|\0|\0|\x1f\0\x1f|"
"\x1f\0\0|\0|\0|\0|\0|\x1f\0\x1f\0\0|\0|\0|\0|\0|\0|\0|\x1f\0"
"\x1f\0\0|\0|\0|\0|\0|\0|\x1f\0\x1f\0\0|\0|\0|\0|\0|\0|\x1f\0"
"\x1f\0\0|\0|\0|\0|\0|\0|\0|\x1f\0\x1f\0\0|\0|\0|\0|\0|\x1f\0"
"\x1f\0\0|\0|\0|\0|\0|\0|\0|\0|\x1f\0\x1f\0\0|\0|\0|\0|\x1f\0"
"\x1f|\x1f\0\0|\0|\0|\0|\0|\0|\0|\0|\x1f\0\x1f\0\0|\0|\x1f\0\x1f|"
"\x1f|\x1f\0\0|\0|\0|\0|\0|\0|\0|\0|\0|\x1f\0\x1f\0\0|\x1f\0\x1f|"
"\x1f|\x1f|\x1f\0\0|\0|\0|\0|\0|\0|\0|\0|\0|\x1f\0\x1f\0\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f\0\0|\0|\0|\0|\0|\0|\0|\0|\x1f\0\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f\0\x1f\0\0|\0|\0|\0|\x1f\0\x1f\0\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f\0\x1f\0\x1f\0\x1f\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
;

struct { int w, h; int clip; int cl, cr, ct, cb;
                GFX_VTABLE *vtable; void *write_bank; void *read_bank;
                void *dat; unsigned long id; void *extra;
                int x_ofs; int y_ofs; int seg; unsigned char *line[17]; } curs_illegal = {
    16, 16, /* width, height */
    0, 0, 0, 0, 0, /* clip */
    (GFX_VTABLE *)15, /* bpp */
    0, 0, /* write_bank, read_bank */
    curs_illegal_lines, /* data */
    0, 0, 0, 0, 0,
    { /* line[] array */
      curs_illegal_lines + 0,
      curs_illegal_lines + 32,
      curs_illegal_lines + 64,
      curs_illegal_lines + 96,
      curs_illegal_lines + 128,
      curs_illegal_lines + 160,
      curs_illegal_lines + 192,
      curs_illegal_lines + 224,
      curs_illegal_lines + 256,
      curs_illegal_lines + 288,
      curs_illegal_lines + 320,
      curs_illegal_lines + 352,
      curs_illegal_lines + 384,
      curs_illegal_lines + 416,
      curs_illegal_lines + 448,
      curs_illegal_lines + 480,
      0 }
};



static DATAFILE_PROPERTY curs_draggable_prop[] = {
    { "11-08-2002, 20:24", DAT_ID('D', 'A', 'T', 'E') },
    { "8", DAT_ID('H', 'S', '_', 'X') },
    { "8", DAT_ID('H', 'S', '_', 'Y') },
    { "draggable", DAT_ID('N', 'A', 'M', 'E') },
    { "hand.pcx", DAT_ID('O', 'R', 'I', 'G') },
    { 0, DAT_END }
};

static unsigned char curs_draggable_lines[] = 
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\0\0\0\0\x1f|\0\0\xff\x7f\xff\x7f\0\0\0\0\0\0\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\0\0\xff\x7f\xff\x7f\0\0\0\0\xff\x7f\xff\x7f\0\0\xff\x7f\xff\x7f\0\0\x1f|\x1f|\x1f|"
"\x1f|\x1f|\0\0\xff\x7f\xff\x7f\0\0\0\0\xff\x7f\xff\x7f\0\0\xff\x7f\xff\x7f\0\0\x1f|\0\0\x1f|"
"\x1f|\x1f|\x1f|\0\0\xff\x7f\xff\x7f\0\0\xff\x7f\xff\x7f\0\0\xff\x7f\xff\x7f\0\0\0\0\xff\x7f\0\0"
"\x1f|\x1f|\x1f|\0\0\xff\x7f\xff\x7f\0\0\xff\x7f\xff\x7f\0\0\xff\x7f\xff\x7f\0\0\xff\x7f\xff\x7f\0\0"
"\x1f|\0\0\0\0\x1f|\0\0\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\0\0\xff\x7f\xff\x7f\0\0"
"\0\0\xff\x7f\xff\x7f\0\0\0\0\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\0\0"
"\0\0\xff\x7f\xff\x7f\xff\x7f\0\0\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\0\0\x1f|"
"\x1f|\0\0\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\0\0\x1f|"
"\x1f|\x1f|\0\0\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\0\0\x1f|"
"\x1f|\x1f|\0\0\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\0\0\x1f|\x1f|"
"\x1f|\x1f|\x1f|\0\0\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\0\0\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\0\0\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\0\0\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\0\0\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\xff\x7f\0\0\x1f|\x1f|\x1f|"
;

struct { int w, h; int clip; int cl, cr, ct, cb;
                GFX_VTABLE *vtable; void *write_bank; void *read_bank;
                void *dat; unsigned long id; void *extra;
                int x_ofs; int y_ofs; int seg; unsigned char *line[17]; } curs_draggable = {
    16, 16, /* width, height */
    0, 0, 0, 0, 0, /* clip */
    (GFX_VTABLE *)15, /* bpp */
    0, 0, /* write_bank, read_bank */
    curs_draggable_lines, /* data */
    0, 0, 0, 0, 0,
    { /* line[] array */
      curs_draggable_lines + 0,
      curs_draggable_lines + 32,
      curs_draggable_lines + 64,
      curs_draggable_lines + 96,
      curs_draggable_lines + 128,
      curs_draggable_lines + 160,
      curs_draggable_lines + 192,
      curs_draggable_lines + 224,
      curs_draggable_lines + 256,
      curs_draggable_lines + 288,
      curs_draggable_lines + 320,
      curs_draggable_lines + 352,
      curs_draggable_lines + 384,
      curs_draggable_lines + 416,
      curs_draggable_lines + 448,
      curs_draggable_lines + 480,
      0 }
};



static DATAFILE_PROPERTY curs_drag_v_prop[] = {
    { "11-08-2002, 20:24", DAT_ID('D', 'A', 'T', 'E') },
    { "8", DAT_ID('H', 'S', '_', 'X') },
    { "8", DAT_ID('H', 'S', '_', 'Y') },
    { "drag_v", DAT_ID('N', 'A', 'M', 'E') },
    { "dragv.pcx", DAT_ID('O', 'R', 'I', 'G') },
    { 0, DAT_END }
};

static unsigned char curs_drag_v_lines[] = 
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\0\0\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\0\0\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\0\0\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\0\0\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
;

struct { int w, h; int clip; int cl, cr, ct, cb;
                GFX_VTABLE *vtable; void *write_bank; void *read_bank;
                void *dat; unsigned long id; void *extra;
                int x_ofs; int y_ofs; int seg; unsigned char *line[17]; } curs_drag_v = {
    16, 16, /* width, height */
    0, 0, 0, 0, 0, /* clip */
    (GFX_VTABLE *)15, /* bpp */
    0, 0, /* write_bank, read_bank */
    curs_drag_v_lines, /* data */
    0, 0, 0, 0, 0,
    { /* line[] array */
      curs_drag_v_lines + 0,
      curs_drag_v_lines + 32,
      curs_drag_v_lines + 64,
      curs_drag_v_lines + 96,
      curs_drag_v_lines + 128,
      curs_drag_v_lines + 160,
      curs_drag_v_lines + 192,
      curs_drag_v_lines + 224,
      curs_drag_v_lines + 256,
      curs_drag_v_lines + 288,
      curs_drag_v_lines + 320,
      curs_drag_v_lines + 352,
      curs_drag_v_lines + 384,
      curs_drag_v_lines + 416,
      curs_drag_v_lines + 448,
      curs_drag_v_lines + 480,
      0 }
};



static DATAFILE_PROPERTY curs_drag_h_prop[] = {
    { "11-08-2002, 20:24", DAT_ID('D', 'A', 'T', 'E') },
    { "8", DAT_ID('H', 'S', '_', 'X') },
    { "8", DAT_ID('H', 'S', '_', 'Y') },
    { "drag_h", DAT_ID('N', 'A', 'M', 'E') },
    { "dragh.pcx", DAT_ID('O', 'R', 'I', 'G') },
    { 0, DAT_END }
};

static unsigned char curs_drag_h_lines[] = 
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\x1f|"
"\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\0\0\x1f|\x1f|"
"\x1f|\0\0\0\0\0\0\0\0\0\0\0\0\x1f|\x1f|\0\0\0\0\0\0\0\0\0\0\0\0\x1f|"
"\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\0\0\x1f|\x1f|"
"\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
;

struct { int w, h; int clip; int cl, cr, ct, cb;
                GFX_VTABLE *vtable; void *write_bank; void *read_bank;
                void *dat; unsigned long id; void *extra;
                int x_ofs; int y_ofs; int seg; unsigned char *line[17]; } curs_drag_h = {
    16, 16, /* width, height */
    0, 0, 0, 0, 0, /* clip */
    (GFX_VTABLE *)15, /* bpp */
    0, 0, /* write_bank, read_bank */
    curs_drag_h_lines, /* data */
    0, 0, 0, 0, 0,
    { /* line[] array */
      curs_drag_h_lines + 0,
      curs_drag_h_lines + 32,
      curs_drag_h_lines + 64,
      curs_drag_h_lines + 96,
      curs_drag_h_lines + 128,
      curs_drag_h_lines + 160,
      curs_drag_h_lines + 192,
      curs_drag_h_lines + 224,
      curs_drag_h_lines + 256,
      curs_drag_h_lines + 288,
      curs_drag_h_lines + 320,
      curs_drag_h_lines + 352,
      curs_drag_h_lines + 384,
      curs_drag_h_lines + 416,
      curs_drag_h_lines + 448,
      curs_drag_h_lines + 480,
      0 }
};



static DATAFILE_PROPERTY curs_cross_prop[] = {
    { "11-08-2002, 20:24", DAT_ID('D', 'A', 'T', 'E') },
    { "8", DAT_ID('H', 'S', '_', 'X') },
    { "8", DAT_ID('H', 'S', '_', 'Y') },
    { "cross", DAT_ID('N', 'A', 'M', 'E') },
    { "cross.pcx", DAT_ID('O', 'R', 'I', 'G') },
    { 0, DAT_END }
};

static unsigned char curs_cross_lines[] = 
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
;

struct { int w, h; int clip; int cl, cr, ct, cb;
                GFX_VTABLE *vtable; void *write_bank; void *read_bank;
                void *dat; unsigned long id; void *extra;
                int x_ofs; int y_ofs; int seg; unsigned char *line[17]; } curs_cross = {
    16, 16, /* width, height */
    0, 0, 0, 0, 0, /* clip */
    (GFX_VTABLE *)15, /* bpp */
    0, 0, /* write_bank, read_bank */
    curs_cross_lines, /* data */
    0, 0, 0, 0, 0,
    { /* line[] array */
      curs_cross_lines + 0,
      curs_cross_lines + 32,
      curs_cross_lines + 64,
      curs_cross_lines + 96,
      curs_cross_lines + 128,
      curs_cross_lines + 160,
      curs_cross_lines + 192,
      curs_cross_lines + 224,
      curs_cross_lines + 256,
      curs_cross_lines + 288,
      curs_cross_lines + 320,
      curs_cross_lines + 352,
      curs_cross_lines + 384,
      curs_cross_lines + 416,
      curs_cross_lines + 448,
      curs_cross_lines + 480,
      0 }
};



static DATAFILE_PROPERTY curs_busy_prop[] = {
    { "11-08-2002, 20:24", DAT_ID('D', 'A', 'T', 'E') },
    { "8", DAT_ID('H', 'S', '_', 'X') },
    { "8", DAT_ID('H', 'S', '_', 'Y') },
    { "busy", DAT_ID('N', 'A', 'M', 'E') },
    { "busy.pcx", DAT_ID('O', 'R', 'I', 'G') },
    { 0, DAT_END }
};

static unsigned char curs_busy_lines[] = 
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\0\0\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\0\0"
"\x1f|\x1f|\x1f|\x1f|\0\0\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\0\0"
"\x1f|\x1f|\x1f|\x1f|\0\0\0\0\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\0\0\0\0"
"\x1f|\x1f|\x1f|\0\0\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\0\0\0\0\0\0\x1f|"
"\x1f|\x1f|\x1f|\0\0\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\x9cs\0\0\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\0\0\0\0\0\0\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\0\0\x9cs\x9cs\x9cs\x9cs\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\0\0\x9cs\x9cs\x9cs\x9cs\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\0\0\0\0\0\0\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\0\0\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\0\0\x9cs\x9cs\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\0\0\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
;

struct { int w, h; int clip; int cl, cr, ct, cb;
                GFX_VTABLE *vtable; void *write_bank; void *read_bank;
                void *dat; unsigned long id; void *extra;
                int x_ofs; int y_ofs; int seg; unsigned char *line[17]; } curs_busy = {
    16, 16, /* width, height */
    0, 0, 0, 0, 0, /* clip */
    (GFX_VTABLE *)15, /* bpp */
    0, 0, /* write_bank, read_bank */
    curs_busy_lines, /* data */
    0, 0, 0, 0, 0,
    { /* line[] array */
      curs_busy_lines + 0,
      curs_busy_lines + 32,
      curs_busy_lines + 64,
      curs_busy_lines + 96,
      curs_busy_lines + 128,
      curs_busy_lines + 160,
      curs_busy_lines + 192,
      curs_busy_lines + 224,
      curs_busy_lines + 256,
      curs_busy_lines + 288,
      curs_busy_lines + 320,
      curs_busy_lines + 352,
      curs_busy_lines + 384,
      curs_busy_lines + 416,
      curs_busy_lines + 448,
      curs_busy_lines + 480,
      0 }
};



static DATAFILE_PROPERTY curs_default_prop[] = {
    { "11-08-2002, 20:24", DAT_ID('D', 'A', 'T', 'E') },
    { "0", DAT_ID('H', 'S', '_', 'X') },
    { "0", DAT_ID('H', 'S', '_', 'Y') },
    { "default", DAT_ID('N', 'A', 'M', 'E') },
    { "arrow.pcx", DAT_ID('O', 'R', 'I', 'G') },
    { 0, DAT_END }
};

static unsigned char curs_default_lines[] = 
"\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\0\0\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\0\0\x18""c\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\0\0\x9cs\x18""c\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\0\0\x9cs\x18""c\x18""c\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\0\0\x9cs\x9cs\x18""c\x18""c\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\0\0\x9cs\x9cs\x18""c\x18""c\x18""c\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\0\0\x9cs\x9cs\x9cs\x18""c\x18""c\x18""c\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\0\0\x9cs\x9cs\x9cs\x18""c\x18""c\x18""c\x18""c\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\0\0\x9cs\x9cs\x9cs\0\0\0\0\0\0\0\0\0\0\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\0\0\x9cs\x9cs\0\0\x9cs\x18""c\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\0\0\x9cs\0\0\0\0\x9cs\x9cs\x18""c\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\0\0\0\0\x1f|\0\0\x9cs\x9cs\x18""c\x18""c\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\0\0\x1f|\x1f|\0\0\x9cs\0\0\0\0\0\0\0\0\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\0\0\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
"\x1f|\x1f|\x1f|\0\0\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|\x1f|"
;

struct { int w, h; int clip; int cl, cr, ct, cb;
                GFX_VTABLE *vtable; void *write_bank; void *read_bank;
                void *dat; unsigned long id; void *extra;
                int x_ofs; int y_ofs; int seg; unsigned char *line[17]; } curs_default = {
    16, 16, /* width, height */
    0, 0, 0, 0, 0, /* clip */
    (GFX_VTABLE *)15, /* bpp */
    0, 0, /* write_bank, read_bank */
    curs_default_lines, /* data */
    0, 0, 0, 0, 0,
    { /* line[] array */
      curs_default_lines + 0,
      curs_default_lines + 32,
      curs_default_lines + 64,
      curs_default_lines + 96,
      curs_default_lines + 128,
      curs_default_lines + 160,
      curs_default_lines + 192,
      curs_default_lines + 224,
      curs_default_lines + 256,
      curs_default_lines + 288,
      curs_default_lines + 320,
      curs_default_lines + 352,
      curs_default_lines + 384,
      curs_default_lines + 416,
      curs_default_lines + 448,
      curs_default_lines + 480,
      0 }
};



DATAFILE curs_cursors[] = {
    { &curs_illegal, DAT_ID('B', 'M', 'P', ' '), 518, curs_illegal_prop },
    { &curs_draggable, DAT_ID('B', 'M', 'P', ' '), 518, curs_draggable_prop },
    { &curs_drag_v, DAT_ID('B', 'M', 'P', ' '), 518, curs_drag_v_prop },
    { &curs_drag_h, DAT_ID('B', 'M', 'P', ' '), 518, curs_drag_h_prop },
    { &curs_cross, DAT_ID('B', 'M', 'P', ' '), 518, curs_cross_prop },
    { &curs_busy, DAT_ID('B', 'M', 'P', ' '), 518, curs_busy_prop },
    { &curs_default, DAT_ID('B', 'M', 'P', ' '), 518, curs_default_prop },
    { 0, DAT_END, 0, 0 }
};



#ifdef ALLEGRO_USE_CONSTRUCTOR
CONSTRUCTOR_FUNCTION(static void _construct_me(void));
static void _construct_me(void)
{
    _construct_datafile(curs_cursors);
}
#endif



/* end of src/cursdata.c */



