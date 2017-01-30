/* src/cursdata.h
 * 
 *  Converted datafile header for resource/cursors.dat .
 *  See src/cursdata.c for definitions.
 *  Do not hand edit.
 */

#ifndef ALLEGRO_H
#error You must include allegro.h
#endif

#ifndef SRC_CURSDATA_H
#define SRC_CURSDATA_H

#ifdef __cplusplus
extern "C" {
#endif



extern DATAFILE curs_cursors[];

#define CURS_ILLEGAL 0
extern BITMAP curs_illegal;

#define CURS_DRAGGABLE 1
extern BITMAP curs_draggable;

#define CURS_DRAG_V 2
extern BITMAP curs_drag_v;

#define CURS_DRAG_H 3
extern BITMAP curs_drag_h;

#define CURS_CROSS 4
extern BITMAP curs_cross;

#define CURS_BUSY 5
extern BITMAP curs_busy;

#define CURS_DEFAULT 6
extern BITMAP curs_default;

#define CURS_COUNT 7



#ifdef __cplusplus
}
#endif

#endif /* include guard */

/* end of src/cursdata.h */



