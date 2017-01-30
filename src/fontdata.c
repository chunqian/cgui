/* src/fontdata.c
 * 
 *  Source for data encoded from resource/fonts.dat .
 *  Do not hand edit.
 */

#include <allegro.h>
#include <allegro/internal/aintern.h>



static DATAFILE_PROPERTY _cgui_font_prop[] = {
    { "9-27-2002, 23:26", DAT_ID('D', 'A', 'T', 'E') },
    { "font", DAT_ID('N', 'A', 'M', 'E') },
    { "../../../../../../../home/krille/programming/sf/cg"
      "ui/resource/font.pcx", DAT_ID('O', 'R', 'I', 'G') },
    { 0, DAT_END }
};

static struct { short w,h; char data[13]; } _cgui_font_glyph32 = {
    6, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph33 = {
    3, 13, /* width, height */
    "\0\0@@@@@@@\0@\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph34 = {
    6, 13, /* width, height */
    "\0\0HHH\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph35 = {
    8, 13, /* width, height */
    "\0\0$$~$$$~$$\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph36 = {
    7, 13, /* width, height */
    "\0\0\x10""8TP0\x18\x14T8\x10\0"
};

static struct { short w,h; char data[26]; } _cgui_font_glyph37 = {
    9, 13, /* width, height */
    "\0\0\0\0""0\0I\0""2\0\x4\0\b\0\x10\0&\0I\0\x6\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph38 = {
    7, 13, /* width, height */
    "\0\0 PP  THH4\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph39 = {
    3, 13, /* width, height */
    "\0\0@@@\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph40 = {
    4, 13, /* width, height */
    "\0\0 @@@@@@@@@ "
};

static struct { short w,h; char data[13]; } _cgui_font_glyph41 = {
    4, 13, /* width, height */
    "\0\0@         @"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph42 = {
    5, 13, /* width, height */
    "\0\0\0P P\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph43 = {
    7, 13, /* width, height */
    "\0\0\0\0\0\x10\x10|\x10\x10\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph44 = {
    4, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0 @\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph45 = {
    4, 13, /* width, height */
    "\0\0\0\0\0\0\0`\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph46 = {
    3, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0@\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph47 = {
    6, 13, /* width, height */
    "\0\0\b\b\b\x10\x10  @@\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph48 = {
    7, 13, /* width, height */
    "\0\0""8DDDDDDD8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph49 = {
    5, 13, /* width, height */
    "\0\0\x10p\x10\x10\x10\x10\x10\x10\x10\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph50 = {
    7, 13, /* width, height */
    "\0\0""8D\x4\x4\b\x10 @|\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph51 = {
    7, 13, /* width, height */
    "\0\0""8D\x4\x4\x18\x4\x4""D8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph52 = {
    7, 13, /* width, height */
    "\0\0\b\x18\x18((H|\b\b\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph53 = {
    7, 13, /* width, height */
    "\0\0|@@xD\x4\x4""D8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph54 = {
    7, 13, /* width, height */
    "\0\0""8D@@xDDD8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph55 = {
    7, 13, /* width, height */
    "\0\0|\x4\b\b\x10\x10   \0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph56 = {
    7, 13, /* width, height */
    "\0\0""8DDD8DDD8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph57 = {
    7, 13, /* width, height */
    "\0\0""8DDD<\x4\x4""D8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph58 = {
    3, 13, /* width, height */
    "\0\0\0\0\0@\0\0\0\0@\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph59 = {
    4, 13, /* width, height */
    "\0\0\0\0\0 \0\0\0\0 @\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph60 = {
    6, 13, /* width, height */
    "\0\0\0\0\b\x10 @ \x10\b\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph61 = {
    7, 13, /* width, height */
    "\0\0\0\0\0\0|\0|\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph62 = {
    6, 13, /* width, height */
    "\0\0\0\0@ \x10\b\x10 @\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph63 = {
    7, 13, /* width, height */
    "\0\0""8D\x4\x4\b\x10\x10\0\x10\0\0"
};

static struct { short w,h; char data[26]; } _cgui_font_glyph64 = {
    12, 13, /* width, height */
    "\0\0\0\0\xf\0""0\xc0 @G I I F\xe0 \0""0\0\xf\x80\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph65 = {
    8, 13, /* width, height */
    "\0\0\x10\x10((DD|\x82\x82\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph66 = {
    7, 13, /* width, height */
    "\0\0xDDDxDDDx\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph67 = {
    8, 13, /* width, height */
    "\0\0<B@@@@@B<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph68 = {
    8, 13, /* width, height */
    "\0\0xDBBBBBDx\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph69 = {
    7, 13, /* width, height */
    "\0\0|@@@x@@@|\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph70 = {
    7, 13, /* width, height */
    "\0\0|@@@x@@@@\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph71 = {
    8, 13, /* width, height */
    "\0\0<B@@NBBF:\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph72 = {
    8, 13, /* width, height */
    "\0\0""BBBB~BBBB\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph73 = {
    3, 13, /* width, height */
    "\0\0@@@@@@@@@\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph74 = {
    5, 13, /* width, height */
    "\0\0\x10\x10\x10\x10\x10\x10\x90\x90`\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph75 = {
    8, 13, /* width, height */
    "\0\0""DHP``PHDB\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph76 = {
    7, 13, /* width, height */
    "\0\0@@@@@@@@|\0\0"
};

static struct { short w,h; char data[26]; } _cgui_font_glyph77 = {
    9, 13, /* width, height */
    "\0\0\0\0""A\0""A\0""c\0""c\0U\0U\0I\0I\0""A\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph78 = {
    8, 13, /* width, height */
    "\0\0""BbbRRJFFB\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph79 = {
    8, 13, /* width, height */
    "\0\0<BBBBBBB<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph80 = {
    8, 13, /* width, height */
    "\0\0|BBB|@@@@\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph81 = {
    8, 13, /* width, height */
    "\0\0<BBBBBJF<\x2\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph82 = {
    8, 13, /* width, height */
    "\0\0|BBB|BBBB\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph83 = {
    7, 13, /* width, height */
    "\0\0""8D@@8\x4\x4""D8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph84 = {
    7, 13, /* width, height */
    "\0\0|\x10\x10\x10\x10\x10\x10\x10\x10\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph85 = {
    8, 13, /* width, height */
    "\0\0""BBBBBBBB<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph86 = {
    8, 13, /* width, height */
    "\0\0\x82\x82""DDD((\x10\x10\0\0"
};

static struct { short w,h; char data[26]; } _cgui_font_glyph87 = {
    12, 13, /* width, height */
    "\0\0\0\0\x80 \x80 D@D@D@*\x80*\x80\x11\0\x11\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph88 = {
    8, 13, /* width, height */
    "\0\0\x82\x82""D(\x10(D\x82\x82\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph89 = {
    8, 13, /* width, height */
    "\0\0\x82\x82""D(\x10\x10\x10\x10\x10\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph90 = {
    8, 13, /* width, height */
    "\0\0\xfe\x2\x4\b\x10 @\x80\xfe\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph91 = {
    4, 13, /* width, height */
    "\0\0`@@@@@@@@@`"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph92 = {
    6, 13, /* width, height */
    "\0\0@@@  \x10\x10\b\b\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph93 = {
    4, 13, /* width, height */
    "\0\0`         `"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph94 = {
    7, 13, /* width, height */
    "\0\x10(D\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph95 = {
    7, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0\0\0\xfc"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph96 = {
    4, 13, /* width, height */
    "\0\0@ \0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph97 = {
    7, 13, /* width, height */
    "\0\0\0\0\0""8\x4<DD<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph98 = {
    7, 13, /* width, height */
    "\0\0@@@xDDDDx\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph99 = {
    7, 13, /* width, height */
    "\0\0\0\0\0""8D@@D8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph100 = {
    7, 13, /* width, height */
    "\0\0\x4\x4\x4<DDDD<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph101 = {
    7, 13, /* width, height */
    "\0\0\0\0\0""8D|@D8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph102 = {
    4, 13, /* width, height */
    "\0\0 @@`@@@@@\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph103 = {
    7, 13, /* width, height */
    "\0\0\0\0\0<DDDD<\x4x"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph104 = {
    7, 13, /* width, height */
    "\0\0@@@XdDDDD\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph105 = {
    3, 13, /* width, height */
    "\0\0@\0\0@@@@@@\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph106 = {
    3, 13, /* width, height */
    "\0\0@\0\0@@@@@@@@"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph107 = {
    7, 13, /* width, height */
    "\0\0@@@HP`PHD\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph108 = {
    3, 13, /* width, height */
    "\0\0@@@@@@@@@\0\0"
};

static struct { short w,h; char data[26]; } _cgui_font_glyph109 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0v\0I\0I\0I\0I\0I\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph110 = {
    7, 13, /* width, height */
    "\0\0\0\0\0XdDDDD\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph111 = {
    7, 13, /* width, height */
    "\0\0\0\0\0""8DDDD8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph112 = {
    7, 13, /* width, height */
    "\0\0\0\0\0xDDDDx@@"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph113 = {
    7, 13, /* width, height */
    "\0\0\0\0\0<DDDD<\x4\x4"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph114 = {
    4, 13, /* width, height */
    "\0\0\0\0\0`@@@@@\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph115 = {
    6, 13, /* width, height */
    "\0\0\0\0\0""0H \x10H0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph116 = {
    4, 13, /* width, height */
    "\0\0\0@@`@@@@ \0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph117 = {
    7, 13, /* width, height */
    "\0\0\0\0\0""DDDDL4\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph118 = {
    7, 13, /* width, height */
    "\0\0\0\0\0""DD((\x10\x10\0\0"
};

static struct { short w,h; char data[26]; } _cgui_font_glyph119 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0I\0I\0U\0U\0\"\0\"\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph120 = {
    6, 13, /* width, height */
    "\0\0\0\0\0HH00HH\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph121 = {
    6, 13, /* width, height */
    "\0\0\0\0\0HHHH0  \xc0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph122 = {
    6, 13, /* width, height */
    "\0\0\0\0\0x\b\x10 @x\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph123 = {
    5, 13, /* width, height */
    "\0\x10    @    \x10\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph124 = {
    3, 13, /* width, height */
    "\0\0@@@@@@@@@@\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph125 = {
    5, 13, /* width, height */
    "\0@    \x10    @\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph126 = {
    8, 13, /* width, height */
    "\0\0\0""2L\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph127 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph128 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph129 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph130 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph131 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph132 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph133 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph134 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph135 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph136 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph137 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph138 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph139 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph140 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph141 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph142 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph143 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph144 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph145 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph146 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph147 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph148 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph149 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph150 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph151 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph152 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph153 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph154 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph155 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph156 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph157 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph158 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph159 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph160 = {
    1, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph161 = {
    3, 13, /* width, height */
    "\0\0\0\0@\0@@@@@@@"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph162 = {
    7, 13, /* width, height */
    "\0\0\0\0\b8D@@D8 \0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph163 = {
    7, 13, /* width, height */
    "\0\0""0H@ p @@|\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph164 = {
    7, 13, /* width, height */
    "\0\0""D8DDD8D\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph165 = {
    7, 13, /* width, height */
    "\0\0""DDD(|\x10|\x10\x10\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph166 = {
    3, 13, /* width, height */
    "\0\0@@@@@\0@@@@@"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph167 = {
    7, 13, /* width, height */
    "\0\0""8D PH$\x14\bD8\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph168 = {
    4, 13, /* width, height */
    "\0\0\0\xa0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_font_glyph169 = {
    10, 13, /* width, height */
    "\0\0\0\0\x1e\0!\0L\x80P\x80L\x80!\0\x1e\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph170 = {
    5, 13, /* width, height */
    "\0\0`0Pp\0p\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph171 = {
    7, 13, /* width, height */
    "\0\0\0\0\0\0\x14(P(\x14\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph172 = {
    7, 13, /* width, height */
    "\0\0\0\0\0\0\0|\x4\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph173 = {
    4, 13, /* width, height */
    "\0\0\0\0\0\0`\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_font_glyph174 = {
    9, 13, /* width, height */
    "\0\0\0\0\x1c\0\"\0]\0Y\0U\0\"\0\x1c\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph175 = {
    7, 13, /* width, height */
    "\0\xfc\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph176 = {
    5, 13, /* width, height */
    "\0\0 P \0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph177 = {
    7, 13, /* width, height */
    "\0\0\0\0\x10\x10|\x10\x10\0|\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph178 = {
    4, 13, /* width, height */
    "\0\0\xc0 @\x80\xe0\0\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph179 = {
    4, 13, /* width, height */
    "\0\0\xc0 @ \xc0\0\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph180 = {
    4, 13, /* width, height */
    "\0\0 @\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph181 = {
    7, 13, /* width, height */
    "\0\0\0\0\0HHHHHt@@"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph182 = {
    7, 13, /* width, height */
    "\0\0<|||<\x14\x14\x14\x14\x14\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph183 = {
    4, 13, /* width, height */
    "\0\0\0`````````\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph184 = {
    4, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0\0 `"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph185 = {
    4, 13, /* width, height */
    "\0\0 `   \0\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph186 = {
    5, 13, /* width, height */
    "\0\0pPPp\0p\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph187 = {
    7, 13, /* width, height */
    "\0\0\0\0\0\0P(\x14(P\0\0"
};

static struct { short w,h; char data[26]; } _cgui_font_glyph188 = {
    9, 13, /* width, height */
    "\0\0\0\0 \0""a\0\"\0$\0)\0\x13\0%\0G\0\x1\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_font_glyph189 = {
    9, 13, /* width, height */
    "\0\0\0\0 \0""a\0\"\0$\0.\0\x11\0\"\0""D\0\a\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_font_glyph190 = {
    9, 13, /* width, height */
    "\0\0\0\0`\0\x11\0\"\0\x14\0i\0\x13\0%\0G\0\x1\0\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph191 = {
    7, 13, /* width, height */
    "\0\0\0\0\x10\0\x10\x10 @@D8"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph192 = {
    8, 13, /* width, height */
    " \x10\x10\x10((DD|\x82\x82\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph193 = {
    8, 13, /* width, height */
    "\b\x10\x10\x10((DD|\x82\x82\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph194 = {
    8, 13, /* width, height */
    "\x10(\0\x10((DD|\x82\x82\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph195 = {
    8, 13, /* width, height */
    "4X\x10\x10((DD|\x82\x82\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph196 = {
    8, 13, /* width, height */
    "D\0\x10\x10((DD|\x82\x82\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph197 = {
    8, 13, /* width, height */
    "\x10(\x10\x10((DD|\x82\x82\0\0"
};

static struct { short w,h; char data[26]; } _cgui_font_glyph198 = {
    10, 13, /* width, height */
    "\0\0\0\0\x1f\x80\x18\0(\0(\0O\0x\0H\0\x88\0\x8f\x80\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph199 = {
    8, 13, /* width, height */
    "\0\0<B@@@@@B<\b\x18"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph200 = {
    7, 13, /* width, height */
    " \x10|@@@x@@@|\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph201 = {
    7, 13, /* width, height */
    "\b\x10|@@@x@@@|\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph202 = {
    7, 13, /* width, height */
    "\x10(|@@@x@@@|\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph203 = {
    7, 13, /* width, height */
    "D\0|@@@x@@@|\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph204 = {
    3, 13, /* width, height */
    "\x80@\0@@@@@@@@\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph205 = {
    4, 13, /* width, height */
    " @\0@@@@@@@@\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph206 = {
    4, 13, /* width, height */
    "@\xa0\0@@@@@@@@\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph207 = {
    4, 13, /* width, height */
    "\xa0\0@@@@@@@@@\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph208 = {
    8, 13, /* width, height */
    "\0\0xDBB\xf2""BBDx\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph209 = {
    8, 13, /* width, height */
    "2LBbbRRJFFB\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph210 = {
    8, 13, /* width, height */
    "\x10\b<BBBBBBB<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph211 = {
    8, 13, /* width, height */
    "\b\x10<BBBBBBB<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph212 = {
    8, 13, /* width, height */
    "\x18$<BBBBBBB<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph213 = {
    8, 13, /* width, height */
    "2L<BBBBBBB<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph214 = {
    8, 13, /* width, height */
    "$\0<BBBBBBB<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph215 = {
    7, 13, /* width, height */
    "\0\0\0\0\0""D(\x10(D\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph216 = {
    8, 13, /* width, height */
    "\0\0>FJJRRbb|\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph217 = {
    8, 13, /* width, height */
    "\x10\bBBBBBBBB<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph218 = {
    8, 13, /* width, height */
    "\b\x10""BBBBBBBB<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph219 = {
    8, 13, /* width, height */
    "\x18$BBBBBBBB<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph220 = {
    8, 13, /* width, height */
    "$\0""BBBBBBBB<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph221 = {
    8, 13, /* width, height */
    "\b\x10\x82\x82""D(\x10\x10\x10\x10\x10\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph222 = {
    8, 13, /* width, height */
    "\0\0@@|BBB|@@\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph223 = {
    7, 13, /* width, height */
    "\0\0""0HHHXDDDX\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph224 = {
    7, 13, /* width, height */
    "\0\0 \x10\0""8\x4<DD<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph225 = {
    7, 13, /* width, height */
    "\0\0\b\x10\0""8\x4<DD<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph226 = {
    7, 13, /* width, height */
    "\0\0\x10(\0""8\x4<DD<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph227 = {
    7, 13, /* width, height */
    "\0\0""4X\0""8\x4<DD<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph228 = {
    7, 13, /* width, height */
    "\0\0$\0\0""8\x4<DD<\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph229 = {
    7, 13, /* width, height */
    "\0\x10(\x10\0""8\x4<DD<\0\0"
};

static struct { short w,h; char data[26]; } _cgui_font_glyph230 = {
    11, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0;\x80\x4@?\xc0""D\0""D@;\x80\0\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph231 = {
    7, 13, /* width, height */
    "\0\0\0\0\0""8D@@D8\x10""0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph232 = {
    7, 13, /* width, height */
    "\0\0 \x10\0""8D|@D8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph233 = {
    7, 13, /* width, height */
    "\0\0\b\x10\0""8D|@D8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph234 = {
    7, 13, /* width, height */
    "\0\0\x10(\0""8D|@D8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph235 = {
    7, 13, /* width, height */
    "\0\0""D\0\0""8D|@D8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph236 = {
    3, 13, /* width, height */
    "\0\0\x80@\0@@@@@@\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph237 = {
    5, 13, /* width, height */
    "\0\0\x10 \0      \0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph238 = {
    5, 13, /* width, height */
    "\0\0 P\0      \0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph239 = {
    5, 13, /* width, height */
    "\0\0P\0\0      \0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph240 = {
    7, 13, /* width, height */
    "\0\0(\x10(\b<DDD8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph241 = {
    7, 13, /* width, height */
    "\0\0""4X\0XdDDDD\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph242 = {
    7, 13, /* width, height */
    "\0\0 \x10\0""8DDDD8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph243 = {
    7, 13, /* width, height */
    "\0\0\b\x10\0""8DDDD8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph244 = {
    7, 13, /* width, height */
    "\0\0\x10(\0""8DDDD8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph245 = {
    7, 13, /* width, height */
    "\0\0""4X\0""8DDDD8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph246 = {
    7, 13, /* width, height */
    "\0\0""D\0\0""8DDDD8\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph247 = {
    7, 13, /* width, height */
    "\0\0\0\0\0\x10\0|\0\x10\0\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph248 = {
    7, 13, /* width, height */
    "\0\0\0\0\0<LTTdx\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph249 = {
    7, 13, /* width, height */
    "\0\0 \x10\0""DDDDL4\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph250 = {
    7, 13, /* width, height */
    "\0\0\b\x10\0""DDDDL4\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph251 = {
    7, 13, /* width, height */
    "\0\0\x10(\0""DDDDL4\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph252 = {
    7, 13, /* width, height */
    "\0\0""D\0\0""DDDDL4\0\0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph253 = {
    6, 13, /* width, height */
    "\0\0\x10 \0HHHH0  \xc0"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph254 = {
    7, 13, /* width, height */
    "\0\0@@@xDDDDx@@"
};

static struct { short w,h; char data[13]; } _cgui_font_glyph255 = {
    6, 13, /* width, height */
    "\0\0H\0\0HHHH0  \xc0"
};

static FONT_GLYPH* _cgui_font_range0_glyphs[] = {
    (FONT_GLYPH *)&_cgui_font_glyph32,
    (FONT_GLYPH *)&_cgui_font_glyph33,
    (FONT_GLYPH *)&_cgui_font_glyph34,
    (FONT_GLYPH *)&_cgui_font_glyph35,
    (FONT_GLYPH *)&_cgui_font_glyph36,
    (FONT_GLYPH *)&_cgui_font_glyph37,
    (FONT_GLYPH *)&_cgui_font_glyph38,
    (FONT_GLYPH *)&_cgui_font_glyph39,
    (FONT_GLYPH *)&_cgui_font_glyph40,
    (FONT_GLYPH *)&_cgui_font_glyph41,
    (FONT_GLYPH *)&_cgui_font_glyph42,
    (FONT_GLYPH *)&_cgui_font_glyph43,
    (FONT_GLYPH *)&_cgui_font_glyph44,
    (FONT_GLYPH *)&_cgui_font_glyph45,
    (FONT_GLYPH *)&_cgui_font_glyph46,
    (FONT_GLYPH *)&_cgui_font_glyph47,
    (FONT_GLYPH *)&_cgui_font_glyph48,
    (FONT_GLYPH *)&_cgui_font_glyph49,
    (FONT_GLYPH *)&_cgui_font_glyph50,
    (FONT_GLYPH *)&_cgui_font_glyph51,
    (FONT_GLYPH *)&_cgui_font_glyph52,
    (FONT_GLYPH *)&_cgui_font_glyph53,
    (FONT_GLYPH *)&_cgui_font_glyph54,
    (FONT_GLYPH *)&_cgui_font_glyph55,
    (FONT_GLYPH *)&_cgui_font_glyph56,
    (FONT_GLYPH *)&_cgui_font_glyph57,
    (FONT_GLYPH *)&_cgui_font_glyph58,
    (FONT_GLYPH *)&_cgui_font_glyph59,
    (FONT_GLYPH *)&_cgui_font_glyph60,
    (FONT_GLYPH *)&_cgui_font_glyph61,
    (FONT_GLYPH *)&_cgui_font_glyph62,
    (FONT_GLYPH *)&_cgui_font_glyph63,
    (FONT_GLYPH *)&_cgui_font_glyph64,
    (FONT_GLYPH *)&_cgui_font_glyph65,
    (FONT_GLYPH *)&_cgui_font_glyph66,
    (FONT_GLYPH *)&_cgui_font_glyph67,
    (FONT_GLYPH *)&_cgui_font_glyph68,
    (FONT_GLYPH *)&_cgui_font_glyph69,
    (FONT_GLYPH *)&_cgui_font_glyph70,
    (FONT_GLYPH *)&_cgui_font_glyph71,
    (FONT_GLYPH *)&_cgui_font_glyph72,
    (FONT_GLYPH *)&_cgui_font_glyph73,
    (FONT_GLYPH *)&_cgui_font_glyph74,
    (FONT_GLYPH *)&_cgui_font_glyph75,
    (FONT_GLYPH *)&_cgui_font_glyph76,
    (FONT_GLYPH *)&_cgui_font_glyph77,
    (FONT_GLYPH *)&_cgui_font_glyph78,
    (FONT_GLYPH *)&_cgui_font_glyph79,
    (FONT_GLYPH *)&_cgui_font_glyph80,
    (FONT_GLYPH *)&_cgui_font_glyph81,
    (FONT_GLYPH *)&_cgui_font_glyph82,
    (FONT_GLYPH *)&_cgui_font_glyph83,
    (FONT_GLYPH *)&_cgui_font_glyph84,
    (FONT_GLYPH *)&_cgui_font_glyph85,
    (FONT_GLYPH *)&_cgui_font_glyph86,
    (FONT_GLYPH *)&_cgui_font_glyph87,
    (FONT_GLYPH *)&_cgui_font_glyph88,
    (FONT_GLYPH *)&_cgui_font_glyph89,
    (FONT_GLYPH *)&_cgui_font_glyph90,
    (FONT_GLYPH *)&_cgui_font_glyph91,
    (FONT_GLYPH *)&_cgui_font_glyph92,
    (FONT_GLYPH *)&_cgui_font_glyph93,
    (FONT_GLYPH *)&_cgui_font_glyph94,
    (FONT_GLYPH *)&_cgui_font_glyph95,
    (FONT_GLYPH *)&_cgui_font_glyph96,
    (FONT_GLYPH *)&_cgui_font_glyph97,
    (FONT_GLYPH *)&_cgui_font_glyph98,
    (FONT_GLYPH *)&_cgui_font_glyph99,
    (FONT_GLYPH *)&_cgui_font_glyph100,
    (FONT_GLYPH *)&_cgui_font_glyph101,
    (FONT_GLYPH *)&_cgui_font_glyph102,
    (FONT_GLYPH *)&_cgui_font_glyph103,
    (FONT_GLYPH *)&_cgui_font_glyph104,
    (FONT_GLYPH *)&_cgui_font_glyph105,
    (FONT_GLYPH *)&_cgui_font_glyph106,
    (FONT_GLYPH *)&_cgui_font_glyph107,
    (FONT_GLYPH *)&_cgui_font_glyph108,
    (FONT_GLYPH *)&_cgui_font_glyph109,
    (FONT_GLYPH *)&_cgui_font_glyph110,
    (FONT_GLYPH *)&_cgui_font_glyph111,
    (FONT_GLYPH *)&_cgui_font_glyph112,
    (FONT_GLYPH *)&_cgui_font_glyph113,
    (FONT_GLYPH *)&_cgui_font_glyph114,
    (FONT_GLYPH *)&_cgui_font_glyph115,
    (FONT_GLYPH *)&_cgui_font_glyph116,
    (FONT_GLYPH *)&_cgui_font_glyph117,
    (FONT_GLYPH *)&_cgui_font_glyph118,
    (FONT_GLYPH *)&_cgui_font_glyph119,
    (FONT_GLYPH *)&_cgui_font_glyph120,
    (FONT_GLYPH *)&_cgui_font_glyph121,
    (FONT_GLYPH *)&_cgui_font_glyph122,
    (FONT_GLYPH *)&_cgui_font_glyph123,
    (FONT_GLYPH *)&_cgui_font_glyph124,
    (FONT_GLYPH *)&_cgui_font_glyph125,
    (FONT_GLYPH *)&_cgui_font_glyph126,
    (FONT_GLYPH *)&_cgui_font_glyph127,
    (FONT_GLYPH *)&_cgui_font_glyph128,
    (FONT_GLYPH *)&_cgui_font_glyph129,
    (FONT_GLYPH *)&_cgui_font_glyph130,
    (FONT_GLYPH *)&_cgui_font_glyph131,
    (FONT_GLYPH *)&_cgui_font_glyph132,
    (FONT_GLYPH *)&_cgui_font_glyph133,
    (FONT_GLYPH *)&_cgui_font_glyph134,
    (FONT_GLYPH *)&_cgui_font_glyph135,
    (FONT_GLYPH *)&_cgui_font_glyph136,
    (FONT_GLYPH *)&_cgui_font_glyph137,
    (FONT_GLYPH *)&_cgui_font_glyph138,
    (FONT_GLYPH *)&_cgui_font_glyph139,
    (FONT_GLYPH *)&_cgui_font_glyph140,
    (FONT_GLYPH *)&_cgui_font_glyph141,
    (FONT_GLYPH *)&_cgui_font_glyph142,
    (FONT_GLYPH *)&_cgui_font_glyph143,
    (FONT_GLYPH *)&_cgui_font_glyph144,
    (FONT_GLYPH *)&_cgui_font_glyph145,
    (FONT_GLYPH *)&_cgui_font_glyph146,
    (FONT_GLYPH *)&_cgui_font_glyph147,
    (FONT_GLYPH *)&_cgui_font_glyph148,
    (FONT_GLYPH *)&_cgui_font_glyph149,
    (FONT_GLYPH *)&_cgui_font_glyph150,
    (FONT_GLYPH *)&_cgui_font_glyph151,
    (FONT_GLYPH *)&_cgui_font_glyph152,
    (FONT_GLYPH *)&_cgui_font_glyph153,
    (FONT_GLYPH *)&_cgui_font_glyph154,
    (FONT_GLYPH *)&_cgui_font_glyph155,
    (FONT_GLYPH *)&_cgui_font_glyph156,
    (FONT_GLYPH *)&_cgui_font_glyph157,
    (FONT_GLYPH *)&_cgui_font_glyph158,
    (FONT_GLYPH *)&_cgui_font_glyph159,
    (FONT_GLYPH *)&_cgui_font_glyph160,
    (FONT_GLYPH *)&_cgui_font_glyph161,
    (FONT_GLYPH *)&_cgui_font_glyph162,
    (FONT_GLYPH *)&_cgui_font_glyph163,
    (FONT_GLYPH *)&_cgui_font_glyph164,
    (FONT_GLYPH *)&_cgui_font_glyph165,
    (FONT_GLYPH *)&_cgui_font_glyph166,
    (FONT_GLYPH *)&_cgui_font_glyph167,
    (FONT_GLYPH *)&_cgui_font_glyph168,
    (FONT_GLYPH *)&_cgui_font_glyph169,
    (FONT_GLYPH *)&_cgui_font_glyph170,
    (FONT_GLYPH *)&_cgui_font_glyph171,
    (FONT_GLYPH *)&_cgui_font_glyph172,
    (FONT_GLYPH *)&_cgui_font_glyph173,
    (FONT_GLYPH *)&_cgui_font_glyph174,
    (FONT_GLYPH *)&_cgui_font_glyph175,
    (FONT_GLYPH *)&_cgui_font_glyph176,
    (FONT_GLYPH *)&_cgui_font_glyph177,
    (FONT_GLYPH *)&_cgui_font_glyph178,
    (FONT_GLYPH *)&_cgui_font_glyph179,
    (FONT_GLYPH *)&_cgui_font_glyph180,
    (FONT_GLYPH *)&_cgui_font_glyph181,
    (FONT_GLYPH *)&_cgui_font_glyph182,
    (FONT_GLYPH *)&_cgui_font_glyph183,
    (FONT_GLYPH *)&_cgui_font_glyph184,
    (FONT_GLYPH *)&_cgui_font_glyph185,
    (FONT_GLYPH *)&_cgui_font_glyph186,
    (FONT_GLYPH *)&_cgui_font_glyph187,
    (FONT_GLYPH *)&_cgui_font_glyph188,
    (FONT_GLYPH *)&_cgui_font_glyph189,
    (FONT_GLYPH *)&_cgui_font_glyph190,
    (FONT_GLYPH *)&_cgui_font_glyph191,
    (FONT_GLYPH *)&_cgui_font_glyph192,
    (FONT_GLYPH *)&_cgui_font_glyph193,
    (FONT_GLYPH *)&_cgui_font_glyph194,
    (FONT_GLYPH *)&_cgui_font_glyph195,
    (FONT_GLYPH *)&_cgui_font_glyph196,
    (FONT_GLYPH *)&_cgui_font_glyph197,
    (FONT_GLYPH *)&_cgui_font_glyph198,
    (FONT_GLYPH *)&_cgui_font_glyph199,
    (FONT_GLYPH *)&_cgui_font_glyph200,
    (FONT_GLYPH *)&_cgui_font_glyph201,
    (FONT_GLYPH *)&_cgui_font_glyph202,
    (FONT_GLYPH *)&_cgui_font_glyph203,
    (FONT_GLYPH *)&_cgui_font_glyph204,
    (FONT_GLYPH *)&_cgui_font_glyph205,
    (FONT_GLYPH *)&_cgui_font_glyph206,
    (FONT_GLYPH *)&_cgui_font_glyph207,
    (FONT_GLYPH *)&_cgui_font_glyph208,
    (FONT_GLYPH *)&_cgui_font_glyph209,
    (FONT_GLYPH *)&_cgui_font_glyph210,
    (FONT_GLYPH *)&_cgui_font_glyph211,
    (FONT_GLYPH *)&_cgui_font_glyph212,
    (FONT_GLYPH *)&_cgui_font_glyph213,
    (FONT_GLYPH *)&_cgui_font_glyph214,
    (FONT_GLYPH *)&_cgui_font_glyph215,
    (FONT_GLYPH *)&_cgui_font_glyph216,
    (FONT_GLYPH *)&_cgui_font_glyph217,
    (FONT_GLYPH *)&_cgui_font_glyph218,
    (FONT_GLYPH *)&_cgui_font_glyph219,
    (FONT_GLYPH *)&_cgui_font_glyph220,
    (FONT_GLYPH *)&_cgui_font_glyph221,
    (FONT_GLYPH *)&_cgui_font_glyph222,
    (FONT_GLYPH *)&_cgui_font_glyph223,
    (FONT_GLYPH *)&_cgui_font_glyph224,
    (FONT_GLYPH *)&_cgui_font_glyph225,
    (FONT_GLYPH *)&_cgui_font_glyph226,
    (FONT_GLYPH *)&_cgui_font_glyph227,
    (FONT_GLYPH *)&_cgui_font_glyph228,
    (FONT_GLYPH *)&_cgui_font_glyph229,
    (FONT_GLYPH *)&_cgui_font_glyph230,
    (FONT_GLYPH *)&_cgui_font_glyph231,
    (FONT_GLYPH *)&_cgui_font_glyph232,
    (FONT_GLYPH *)&_cgui_font_glyph233,
    (FONT_GLYPH *)&_cgui_font_glyph234,
    (FONT_GLYPH *)&_cgui_font_glyph235,
    (FONT_GLYPH *)&_cgui_font_glyph236,
    (FONT_GLYPH *)&_cgui_font_glyph237,
    (FONT_GLYPH *)&_cgui_font_glyph238,
    (FONT_GLYPH *)&_cgui_font_glyph239,
    (FONT_GLYPH *)&_cgui_font_glyph240,
    (FONT_GLYPH *)&_cgui_font_glyph241,
    (FONT_GLYPH *)&_cgui_font_glyph242,
    (FONT_GLYPH *)&_cgui_font_glyph243,
    (FONT_GLYPH *)&_cgui_font_glyph244,
    (FONT_GLYPH *)&_cgui_font_glyph245,
    (FONT_GLYPH *)&_cgui_font_glyph246,
    (FONT_GLYPH *)&_cgui_font_glyph247,
    (FONT_GLYPH *)&_cgui_font_glyph248,
    (FONT_GLYPH *)&_cgui_font_glyph249,
    (FONT_GLYPH *)&_cgui_font_glyph250,
    (FONT_GLYPH *)&_cgui_font_glyph251,
    (FONT_GLYPH *)&_cgui_font_glyph252,
    (FONT_GLYPH *)&_cgui_font_glyph253,
    (FONT_GLYPH *)&_cgui_font_glyph254,
    (FONT_GLYPH *)&_cgui_font_glyph255,
    0
};

static FONT_MONO_DATA _cgui_font_range0 = {
    32, 256, /* begin, end */
    _cgui_font_range0_glyphs,
    0
};

static FONT _cgui_font = {
    &_cgui_font_range0,
    13, /* height */
    (FONT_VTABLE *)0 /* color flag */
};



static DATAFILE_PROPERTY _cgui_fix_font_prop[] = {
    { "8-02-2007, 17:15", DAT_ID('D', 'A', 'T', 'E') },
    { "fix_font", DAT_ID('N', 'A', 'M', 'E') },
    { "/home/krille/programming/sf/cgui16/cgui/resource/f"
      "ix_font.bmp", DAT_ID('O', 'R', 'I', 'G') },
    { 0, DAT_END }
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph32 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph33 = {
    9, 13, /* width, height */
    "\0\0\0\0\x10\0\x10\0\x10\0\x10\0\x10\0\x10\0\0\0\x10\0\x10\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph34 = {
    9, 13, /* width, height */
    "\0\0$\0$\0$\0$\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph35 = {
    9, 13, /* width, height */
    "\0\0\0\0\x14\0\x14\0\x14\0>\0\x14\0\x14\0>\0\x14\0\x14\0\x14\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph36 = {
    9, 13, /* width, height */
    "\b\0\b\0\x1e\0\"\0 \0""0\0\f\0\x2\0\"\0<\0\b\0\b\0\b\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph37 = {
    9, 13, /* width, height */
    "\0\0`\0\x90\0\x90\0s\0\f\0""0\0\xcc\0\x12\0\x12\0\f\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph38 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\x1c\0 \0 \0 \0T\0H\0L\0""2\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph39 = {
    9, 13, /* width, height */
    "\0\0\0\0\x10\0\x10\0\x10\0\x10\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph40 = {
    9, 13, /* width, height */
    "\0\0\x4\0\b\0\b\0\x10\0\x10\0\x10\0\x10\0\x10\0\x10\0\b\0\b\0\x4\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph41 = {
    9, 13, /* width, height */
    "\0\0 \0\x10\0\x10\0\b\0\b\0\b\0\b\0\b\0\b\0\x10\0\x10\0 \0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph42 = {
    9, 13, /* width, height */
    "\0\0\0\0\x10\0\x10\0|\0\x10\0(\0""D\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph43 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\b\0\b\0\b\0\x7f\0\b\0\b\0\b\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph44 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x18\0\x18\0""0\0 \0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph45 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0\0\0~\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph46 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x18\0\x18\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph47 = {
    9, 13, /* width, height */
    "\0\0\x2\0\x4\0\x4\0\b\0\b\0\x10\0\x10\0 \0 \0@\0@\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph48 = {
    9, 13, /* width, height */
    "\0\0\x18\0$\0""B\0""B\0""B\0""B\0""B\0""B\0$\0\x18\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph49 = {
    9, 13, /* width, height */
    "\0\0\x10\0""0\0P\0\x10\0\x10\0\x10\0\x10\0\x10\0\x10\0|\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph50 = {
    9, 13, /* width, height */
    "\0\0""8\0""D\0""D\0\x4\0\b\0\x10\0 \0@\0""D\0|\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph51 = {
    9, 13, /* width, height */
    "\0\0\x1c\0\"\0\x2\0\x2\0\f\0\x2\0\x2\0\x2\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph52 = {
    9, 13, /* width, height */
    "\0\0\f\0\x14\0\x14\0$\0$\0""D\0""D\0~\0\x4\0\xe\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph53 = {
    9, 13, /* width, height */
    "\0\0>\0 \0 \0 \0<\0\x2\0\x2\0\x2\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph54 = {
    9, 13, /* width, height */
    "\0\0\x1c\0 \0@\0@\0\\\0""b\0""B\0""B\0\"\0\x1c\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph55 = {
    9, 13, /* width, height */
    "\0\0~\0""B\0\x2\0\x4\0\x4\0\x4\0\b\0\b\0\b\0\b\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph56 = {
    9, 13, /* width, height */
    "\0\0\x1c\0\"\0\"\0\"\0\x1c\0\"\0\"\0\"\0\"\0\x1c\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph57 = {
    9, 13, /* width, height */
    "\0\0""8\0""D\0""B\0""B\0""F\0:\0\x2\0\x2\0\x4\0""8\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph58 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\x18\0\x18\0\0\0\0\0\0\0\x18\0\x18\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph59 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\x18\0\x18\0\0\0\0\0\0\0\x18\0\x18\0""0\0 \0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph60 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\x6\0\x18\0`\0\x80\0`\0\x18\0\x6\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph61 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0\x7f\0\0\0\x7f\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph62 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0`\0\x18\0\x6\0\x1\0\x6\0\x18\0`\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph63 = {
    9, 13, /* width, height */
    "\0\0\0\0""8\0""D\0\x4\0\x4\0\x18\0\x10\0\0\0\x10\0\x10\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph64 = {
    9, 13, /* width, height */
    "\0\0\0\0\x1c\0\"\0""B\0N\0R\0R\0O\0@\0 \0\x1c\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph65 = {
    9, 13, /* width, height */
    "\0\0\0\0p\0\x10\0(\0(\0""D\0|\0\x82\0\x82\0\xef\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph66 = {
    9, 13, /* width, height */
    "\0\0\0\0\xfc\0""B\0""B\0""B\0|\0""B\0""B\0""B\0\xfc\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph67 = {
    9, 13, /* width, height */
    "\0\0\0\0\x1d\0#\0""A\0@\0@\0@\0@\0!\0\x1e\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph68 = {
    9, 13, /* width, height */
    "\0\0\0\0\xfc\0""B\0""A\0""A\0""A\0""A\0""A\0""B\0\xfc\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph69 = {
    9, 13, /* width, height */
    "\0\0\0\0\x7f\0!\0!\0$\0<\0$\0!\0!\0\x7f\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph70 = {
    9, 13, /* width, height */
    "\0\0\0\0\x7f\0!\0!\0$\0<\0$\0 \0 \0x\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph71 = {
    9, 13, /* width, height */
    "\0\0\0\0:\0""F\0\x82\0\x80\0\x80\0\x8f\0\x82\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph72 = {
    9, 13, /* width, height */
    "\0\0\0\0\xe7\0""B\0""B\0""B\0~\0""B\0""B\0""B\0\xe7\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph73 = {
    9, 13, /* width, height */
    "\0\0\0\0>\0\b\0\b\0\b\0\b\0\b\0\b\0\b\0>\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph74 = {
    9, 13, /* width, height */
    "\0\0\0\0\x1f\0\x4\0\x4\0\x4\0\x4\0""D\0""D\0""D\0""8\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph75 = {
    9, 13, /* width, height */
    "\0\0\0\0\xee\0""D\0H\0P\0p\0H\0""D\0""D\0\xe3\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph76 = {
    9, 13, /* width, height */
    "\0\0\0\0\xf8\0 \0 \0 \0 \0!\0!\0!\0\xff\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph77 = {
    9, 13, /* width, height */
    "\0\0\0\0\xe3\x80""c\0U\0U\0I\0I\0""A\0""A\0\xe3\x80\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph78 = {
    9, 13, /* width, height */
    "\0\0\0\0\xe7\0""b\0R\0R\0J\0J\0""F\0""F\0\xe2\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph79 = {
    9, 13, /* width, height */
    "\0\0\0\0<\0""B\0\x81\0\x81\0\x81\0\x81\0\x81\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph80 = {
    9, 13, /* width, height */
    "\0\0\0\0~\0!\0!\0!\0!\0>\0 \0 \0x\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph81 = {
    9, 13, /* width, height */
    "\0\0\0\0<\0""B\0\x81\0\x81\0\x81\0\x81\0\x81\0""B\0<\0""1\0^\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph82 = {
    9, 13, /* width, height */
    "\0\0\0\0\xfc\0""B\0""B\0""B\0""D\0x\0""D\0""B\0\xe1\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph83 = {
    9, 13, /* width, height */
    "\0\0\0\0:\0""F\0""B\0@\0<\0\x2\0""B\0""b\0\\\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph84 = {
    9, 13, /* width, height */
    "\0\0\0\0\x7f\0I\0I\0\b\0\b\0\b\0\b\0\b\0>\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph85 = {
    9, 13, /* width, height */
    "\0\0\0\0\xe7\0""B\0""B\0""B\0""B\0""B\0""B\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph86 = {
    9, 13, /* width, height */
    "\0\0\0\0\xe3\x80""A\0""A\0\"\0\"\0\x14\0\x14\0\b\0\b\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph87 = {
    9, 13, /* width, height */
    "\0\0\0\0\xe3\x80""A\0I\0I\0U\0U\0\"\0\"\0\"\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph88 = {
    9, 13, /* width, height */
    "\0\0\0\0\xe7\0""B\0$\0$\0\x18\0$\0$\0""B\0\xe7\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph89 = {
    9, 13, /* width, height */
    "\0\0\0\0\xee\0""D\0""D\0(\0(\0\x10\0\x10\0\x10\0|\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph90 = {
    9, 13, /* width, height */
    "\0\0\0\0~\0""B\0""D\0\b\0\x10\0\x10\0\"\0""B\0~\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph91 = {
    9, 13, /* width, height */
    "\0\0\x1c\0\x10\0\x10\0\x10\0\x10\0\x10\0\x10\0\x10\0\x10\0\x10\0\x10\0\x1c\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph92 = {
    9, 13, /* width, height */
    "\0\0@\0 \0 \0\x10\0\x10\0\b\0\b\0\x4\0\x4\0\x2\0\x2\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph93 = {
    9, 13, /* width, height */
    "\0\0""8\0\b\0\b\0\b\0\b\0\b\0\b\0\b\0\b\0\b\0\b\0""8\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph94 = {
    9, 13, /* width, height */
    "\0\0\0\0\b\0\x14\0\x14\0\"\0\"\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph95 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph96 = {
    9, 13, /* width, height */
    "\0\0""0\0\f\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph97 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0<\0""B\0\x2\0>\0""B\0""B\0=\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph98 = {
    9, 13, /* width, height */
    "\0\0\xc0\0@\0@\0\\\0""b\0""A\0""A\0""A\0""b\0\xdc\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph99 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\x1d\0#\0""A\0@\0@\0!\0\x1e\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph100 = {
    9, 13, /* width, height */
    "\0\0\x6\0\x2\0\x2\0:\0""F\0\x82\0\x82\0\x82\0""F\0;\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph101 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\x1c\0\"\0""A\0\x7f\0@\0!\0\x1e\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph102 = {
    9, 13, /* width, height */
    "\0\0\xf\0\x10\0\x10\0~\0\x10\0\x10\0\x10\0\x10\0\x10\0|\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph103 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0;\0""F\0\x82\0\x82\0\x82\0""F\0:\0\x4\0x\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph104 = {
    9, 13, /* width, height */
    "\0\0\xc0\0@\0@\0\\\0""b\0""B\0""B\0""B\0""B\0\xe7\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph105 = {
    9, 13, /* width, height */
    "\0\0\b\0\b\0\0\0""8\0\b\0\b\0\b\0\b\0\b\0>\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph106 = {
    9, 13, /* width, height */
    "\0\0\x4\0\x4\0\0\0|\0\x4\0\x4\0\x4\0\x4\0\x4\0\x4\0\b\0p\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph107 = {
    9, 13, /* width, height */
    "\0\0`\0 \0 \0'\0$\0(\0""0\0(\0$\0g\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph108 = {
    9, 13, /* width, height */
    "\0\0""8\0\b\0\b\0\b\0\b\0\b\0\b\0\b\0\b\0>\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph109 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\xdb\0m\0I\0I\0I\0I\0\xed\x80\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph110 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\xdc\0""b\0""B\0""B\0""B\0""B\0\xe7\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph111 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0<\0""B\0\x81\0\x81\0\x81\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph112 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\xdc\0""b\0""A\0""A\0""A\0""b\0\\\0@\0\xf0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph113 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0;\0""F\0\x82\0\x82\0\x82\0""F\0:\0\x2\0\xf\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph114 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0""f\0)\0""0\0 \0 \0 \0|\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph115 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0>\0""B\0@\0<\0\x2\0""B\0|\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph116 = {
    9, 13, /* width, height */
    "\0\0\0\0 \0 \0\xfc\0 \0 \0 \0 \0\"\0\x1c\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph117 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\xc6\0""B\0""B\0""B\0""B\0""F\0;\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph118 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\xe7\0""B\0""B\0$\0$\0\x18\0\x18\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph119 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\xe3\x80""A\0I\0I\0*\0""6\0\"\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph120 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\xee\0""D\0(\0\x10\0(\0""D\0\xee\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph121 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\xe7\0""B\0""B\0$\0$\0\x18\0\b\0\x10\0x\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph122 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0>\0\"\0\x4\0\b\0\x10\0\"\0>\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph123 = {
    9, 13, /* width, height */
    "\0\0\b\0\x10\0\x10\0\x10\0\x10\0 \0\x10\0\x10\0\x10\0\x10\0\b\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph124 = {
    9, 13, /* width, height */
    "\0\0\0\0\x10\0\x10\0\x10\0\x10\0\x10\0\x10\0\x10\0\x10\0\x10\0\x10\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph125 = {
    9, 13, /* width, height */
    "\0\0 \0\x10\0\x10\0\x10\0\x10\0\b\0\x10\0\x10\0\x10\0\x10\0 \0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph126 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0\0\0""2\0L\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph127 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph128 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph129 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph130 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph131 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph132 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph133 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph134 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph135 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph136 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph137 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph138 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph139 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph140 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph141 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph142 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph143 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph144 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph145 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph146 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph147 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph148 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph149 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph150 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph151 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph152 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph153 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph154 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph155 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph156 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph157 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph158 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph159 = {
    9, 13, /* width, height */
    "\0\0\0\0\xaa\0\0\0\x82\0\0\0\x82\0\0\0\x82\0\0\0\xaa\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph160 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph161 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\b\0\b\0\0\0\b\0\b\0\b\0\b\0\b\0\b\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph162 = {
    9, 13, /* width, height */
    "\0\0\0\0\b\0\b\0\x1e\0\"\0 \0\"\0\x1c\0\b\0\b\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph163 = {
    9, 13, /* width, height */
    "\0\0\0\0\f\0\x12\0\x10\0\x10\0<\0\x10\0\x10\0!\0~\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph164 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0Z\0$\0""B\0""B\0$\0Z\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph165 = {
    9, 13, /* width, height */
    "\0\0\0\0w\0\"\0\"\0\x14\0>\0\b\0>\0\b\0\x1c\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph166 = {
    9, 13, /* width, height */
    "\0\0\0\0\x10\0\x10\0\x10\0\x10\0\0\0\0\0\x10\0\x10\0\x10\0\x10\0\x10\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph167 = {
    9, 13, /* width, height */
    "\0\0\0\0\x1e\0\"\0 \0x\0""D\0\"\0\x1e\0\x4\0""D\0x\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph168 = {
    9, 13, /* width, height */
    "\0\0\0\0""6\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph169 = {
    9, 13, /* width, height */
    "\0\0\0\0<\0""B\0\x99\0\xa5\0\xa1\0\xa5\0\x99\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph170 = {
    9, 13, /* width, height */
    "\0\0\0\0""0\0\b\0""8\0,\0\0\0<\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph171 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\x11\0\"\0""D\0\xcc\0""D\0\"\0\x11\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph172 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0\x7f\0\x1\0\x1\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph173 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0\0\0~\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph174 = {
    9, 13, /* width, height */
    "\0\0\0\0<\0""B\0\xb9\0\xa5\0\xb9\0\xa9\0\xa5\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph175 = {
    9, 13, /* width, height */
    "\0\0\0\0<\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph176 = {
    9, 13, /* width, height */
    "\0\0\x18\0$\0$\0\x18\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph177 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\b\0\b\0\x7f\0\b\0\b\0\0\0\x7f\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph178 = {
    9, 13, /* width, height */
    "\0\0\x18\0$\0\x4\0\b\0\x10\0<\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph179 = {
    9, 13, /* width, height */
    "\0\0\x18\0$\0\x18\0\x4\0$\0\x18\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph180 = {
    9, 13, /* width, height */
    "\0\0\f\0""0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph181 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\xc6\0""B\0""B\0""B\0""B\0""F\0{\0@\0@\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph182 = {
    9, 13, /* width, height */
    "\0\0\0\0?\0J\0J\0J\0:\0\n\0\n\0\n\0\n\0\x1f\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph183 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0\0\0\x18\0\x18\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph184 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x10\0\b\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph185 = {
    9, 13, /* width, height */
    "\0\0\b\0\x18\0\b\0\b\0\b\0\x1c\0\0\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph186 = {
    9, 13, /* width, height */
    "\0\0\0\0\x18\0$\0$\0\x18\0\0\0<\0\0\0\0\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph187 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\x88\0""D\0\"\0""3\0\"\0""D\0\x88\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph188 = {
    9, 13, /* width, height */
    "\0\0\x80\0\x82\0\x84\0\x88\0\x89\0\xd3\0%\0)\0O\x80\x81\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph189 = {
    9, 13, /* width, height */
    "\0\0\x80\0\x82\0\x84\0\x88\0\x8b\0\xd4\x80 \x80!\0""B\0\x87\x80\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph190 = {
    9, 13, /* width, height */
    "\0\0\xc0\0\"\0\xc4\0(\0)\0\xd3\0%\0)\0O\x80\x81\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph191 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\b\0\b\0\0\0\b\0\x18\0 \0 \0\"\0\x1c\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph192 = {
    9, 13, /* width, height */
    "0\0\f\0""8\0\b\0\x14\0\x14\0\"\0>\0""A\0""A\0\xf7\x80\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph193 = {
    9, 13, /* width, height */
    "\f\0""0\0""8\0\b\0\x14\0\x14\0\"\0>\0""A\0""A\0\xf7\x80\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph194 = {
    9, 13, /* width, height */
    "\b\0\x14\0""8\0\b\0\x14\0\x14\0\"\0>\0""A\0""A\0\xf7\x80\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph195 = {
    9, 13, /* width, height */
    "\x1a\0,\0""8\0\b\0\x14\0\x14\0\"\0>\0""A\0""A\0\xf7\x80\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph196 = {
    9, 13, /* width, height */
    "\0\0""6\0""8\0\b\0\x14\0\x14\0\"\0>\0""A\0""A\0\xf7\x80\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph197 = {
    9, 13, /* width, height */
    "\x18\0$\0""8\0\b\0\x14\0\x14\0\"\0>\0""A\0""A\0\xf7\x80\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph198 = {
    9, 13, /* width, height */
    "\0\0\0\0?\0\x19\0(\0)\0O\0y\0\x88\0\x89\0\xdf\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph199 = {
    9, 13, /* width, height */
    "\0\0\0\0\x1d\0#\0""A\0@\0@\0@\0@\0!\0\x1e\0\x4\0\x1c\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph200 = {
    9, 13, /* width, height */
    "0\0\f\0\x7f\0!\0!\0$\0<\0$\0!\0!\0\x7f\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph201 = {
    9, 13, /* width, height */
    "\x6\0\x18\0\x7f\0!\0!\0$\0<\0$\0!\0!\0\x7f\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph202 = {
    9, 13, /* width, height */
    "\b\0\x14\0\x7f\0!\0!\0$\0<\0$\0!\0!\0\x7f\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph203 = {
    9, 13, /* width, height */
    "\0\0""6\0\x7f\0!\0!\0$\0<\0$\0!\0!\0\x7f\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph204 = {
    9, 13, /* width, height */
    "0\0\f\0>\0\b\0\b\0\b\0\b\0\b\0\b\0\b\0>\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph205 = {
    9, 13, /* width, height */
    "\x6\0\x18\0>\0\b\0\b\0\b\0\b\0\b\0\b\0\b\0>\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph206 = {
    9, 13, /* width, height */
    "\b\0\x14\0>\0\b\0\b\0\b\0\b\0\b\0\b\0\b\0>\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph207 = {
    9, 13, /* width, height */
    "\0\0""6\0>\0\b\0\b\0\b\0\b\0\b\0\b\0\b\0>\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph208 = {
    9, 13, /* width, height */
    "\0\0\0\0\xfc\0""B\0""A\0""A\0\xf1\0""A\0""A\0""B\0\xfc\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph209 = {
    9, 13, /* width, height */
    "\x1a\0,\0\xe7\0""b\0R\0R\0J\0J\0""F\0""F\0\xe2\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph210 = {
    9, 13, /* width, height */
    "0\0\f\0<\0""B\0\x81\0\x81\0\x81\0\x81\0\x81\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph211 = {
    9, 13, /* width, height */
    "\f\0""0\0<\0""B\0\x81\0\x81\0\x81\0\x81\0\x81\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph212 = {
    9, 13, /* width, height */
    "\x10\0(\0<\0""B\0\x81\0\x81\0\x81\0\x81\0\x81\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph213 = {
    9, 13, /* width, height */
    "\x1a\0,\0<\0""B\0\x81\0\x81\0\x81\0\x81\0\x81\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph214 = {
    9, 13, /* width, height */
    "\0\0""f\0<\0""B\0\x81\0\x81\0\x81\0\x81\0\x81\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph215 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0""A\0\"\0\x14\0\b\0\x14\0\"\0""A\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph216 = {
    9, 13, /* width, height */
    "\0\0\0\0=\0""B\0\x85\0\x89\0\x91\0\xa1\0""A\0\xc2\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph217 = {
    9, 13, /* width, height */
    "0\0\f\0\xe7\0""B\0""B\0""B\0""B\0""B\0""B\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph218 = {
    9, 13, /* width, height */
    "\f\0""0\0\xe7\0""B\0""B\0""B\0""B\0""B\0""B\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph219 = {
    9, 13, /* width, height */
    "\x10\0(\0\xe7\0""B\0""B\0""B\0""B\0""B\0""B\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph220 = {
    9, 13, /* width, height */
    "\0\0""f\0\xe7\0""B\0""B\0""B\0""B\0""B\0""B\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph221 = {
    9, 13, /* width, height */
    "\x6\0\x18\0w\0\"\0\"\0\x14\0\x14\0\b\0\b\0\b\0>\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph222 = {
    9, 13, /* width, height */
    "\0\0\0\0\xe0\0@\0|\0""B\0""B\0""B\0|\0@\0\xe0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph223 = {
    9, 13, /* width, height */
    "\0\0\0\0""8\0""D\0""D\0X\0""D\0""B\0""B\0R\0\xcc\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph224 = {
    9, 13, /* width, height */
    "\0\0""0\0\f\0\0\0<\0""B\0\x2\0>\0""B\0""F\0;\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph225 = {
    9, 13, /* width, height */
    "\0\0\f\0""0\0\0\0<\0""B\0\x2\0>\0""B\0""F\0;\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph226 = {
    9, 13, /* width, height */
    "\0\0\b\0\x14\0\0\0<\0""B\0\x2\0>\0""B\0""F\0;\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph227 = {
    9, 13, /* width, height */
    "\0\0\x1a\0,\0\0\0<\0""B\0\x2\0>\0""B\0""F\0;\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph228 = {
    9, 13, /* width, height */
    "\0\0\0\0""6\0\0\0<\0""B\0\x2\0>\0""B\0""F\0;\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph229 = {
    9, 13, /* width, height */
    "\0\0\x18\0$\0\x18\0<\0""B\0\x2\0>\0""B\0""F\0;\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph230 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0v\0\x89\0\t\0\x7f\0\x88\0\x89\0v\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph231 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0\x1d\0#\0""A\0@\0@\0!\0\x1e\0\x4\0\x1c\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph232 = {
    9, 13, /* width, height */
    "\0\0""0\0\f\0\0\0\x1c\0\"\0""A\0\x7f\0@\0!\0\x1e\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph233 = {
    9, 13, /* width, height */
    "\0\0\x6\0\x18\0\0\0\x1c\0\"\0""A\0\x7f\0@\0!\0\x1e\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph234 = {
    9, 13, /* width, height */
    "\0\0\b\0\x14\0\0\0\x1c\0\"\0""A\0\x7f\0@\0!\0\x1e\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph235 = {
    9, 13, /* width, height */
    "\0\0\0\0""6\0\0\0\x1c\0\"\0""A\0\x7f\0@\0!\0\x1e\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph236 = {
    9, 13, /* width, height */
    "\0\0""0\0\f\0\0\0""8\0\b\0\b\0\b\0\b\0\b\0>\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph237 = {
    9, 13, /* width, height */
    "\0\0\f\0""0\0\0\0""8\0\b\0\b\0\b\0\b\0\b\0>\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph238 = {
    9, 13, /* width, height */
    "\0\0\b\0\x14\0\0\0""8\0\b\0\b\0\b\0\b\0\b\0>\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph239 = {
    9, 13, /* width, height */
    "\0\0\0\0""6\0\0\0""8\0\b\0\b\0\b\0\b\0\b\0>\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph240 = {
    9, 13, /* width, height */
    "\xe4\0\x18\0(\0""D\0<\0""B\0\x81\0\x81\0\x81\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph241 = {
    9, 13, /* width, height */
    "\0\0\x1a\0,\0\0\0\xdc\0""b\0""B\0""B\0""B\0""B\0\xe7\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph242 = {
    9, 13, /* width, height */
    "\0\0""0\0\f\0\0\0<\0""B\0\x81\0\x81\0\x81\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph243 = {
    9, 13, /* width, height */
    "\0\0\f\0""0\0\0\0<\0""B\0\x81\0\x81\0\x81\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph244 = {
    9, 13, /* width, height */
    "\0\0\x10\0(\0\0\0<\0""B\0\x81\0\x81\0\x81\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph245 = {
    9, 13, /* width, height */
    "\0\0\x1a\0,\0\0\0<\0""B\0\x81\0\x81\0\x81\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph246 = {
    9, 13, /* width, height */
    "\0\0\0\0l\0\0\0<\0""B\0\x81\0\x81\0\x81\0""B\0<\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph247 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\x18\0\x18\0\0\0\xff\0\0\0\x18\0\x18\0\0\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph248 = {
    9, 13, /* width, height */
    "\0\0\0\0\0\0\0\0=\0""F\0\x89\0\x91\0\xa1\0""B\0\xbc\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph249 = {
    9, 13, /* width, height */
    "\0\0""0\0\f\0\0\0\xc6\0""B\0""B\0""B\0""B\0""F\0;\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph250 = {
    9, 13, /* width, height */
    "\0\0\f\0""0\0\0\0\xc6\0""B\0""B\0""B\0""B\0""F\0;\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph251 = {
    9, 13, /* width, height */
    "\0\0\x10\0(\0\0\0\xc6\0""B\0""B\0""B\0""B\0""F\0;\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph252 = {
    9, 13, /* width, height */
    "\0\0\0\0l\0\0\0\xc6\0""B\0""B\0""B\0""B\0""F\0;\0\0\0\0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph253 = {
    9, 13, /* width, height */
    "\0\0\x6\0\x18\0\0\0\xe7\0""B\0""B\0$\0$\0\x18\0\b\0\x10\0x\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph254 = {
    9, 13, /* width, height */
    "\0\0\0\0\xc0\0@\0\\\0""b\0""A\0""A\0""A\0""b\0\\\0@\0\xf0\0"
};

static struct { short w,h; char data[26]; } _cgui_fix_font_glyph255 = {
    9, 13, /* width, height */
    "\0\0\0\0""6\0\0\0\xe7\0""B\0""B\0$\0$\0\x18\0\b\0\x10\0x\0"
};

static FONT_GLYPH* _cgui_fix_font_range0_glyphs[] = {
    (FONT_GLYPH *)&_cgui_fix_font_glyph32,
    (FONT_GLYPH *)&_cgui_fix_font_glyph33,
    (FONT_GLYPH *)&_cgui_fix_font_glyph34,
    (FONT_GLYPH *)&_cgui_fix_font_glyph35,
    (FONT_GLYPH *)&_cgui_fix_font_glyph36,
    (FONT_GLYPH *)&_cgui_fix_font_glyph37,
    (FONT_GLYPH *)&_cgui_fix_font_glyph38,
    (FONT_GLYPH *)&_cgui_fix_font_glyph39,
    (FONT_GLYPH *)&_cgui_fix_font_glyph40,
    (FONT_GLYPH *)&_cgui_fix_font_glyph41,
    (FONT_GLYPH *)&_cgui_fix_font_glyph42,
    (FONT_GLYPH *)&_cgui_fix_font_glyph43,
    (FONT_GLYPH *)&_cgui_fix_font_glyph44,
    (FONT_GLYPH *)&_cgui_fix_font_glyph45,
    (FONT_GLYPH *)&_cgui_fix_font_glyph46,
    (FONT_GLYPH *)&_cgui_fix_font_glyph47,
    (FONT_GLYPH *)&_cgui_fix_font_glyph48,
    (FONT_GLYPH *)&_cgui_fix_font_glyph49,
    (FONT_GLYPH *)&_cgui_fix_font_glyph50,
    (FONT_GLYPH *)&_cgui_fix_font_glyph51,
    (FONT_GLYPH *)&_cgui_fix_font_glyph52,
    (FONT_GLYPH *)&_cgui_fix_font_glyph53,
    (FONT_GLYPH *)&_cgui_fix_font_glyph54,
    (FONT_GLYPH *)&_cgui_fix_font_glyph55,
    (FONT_GLYPH *)&_cgui_fix_font_glyph56,
    (FONT_GLYPH *)&_cgui_fix_font_glyph57,
    (FONT_GLYPH *)&_cgui_fix_font_glyph58,
    (FONT_GLYPH *)&_cgui_fix_font_glyph59,
    (FONT_GLYPH *)&_cgui_fix_font_glyph60,
    (FONT_GLYPH *)&_cgui_fix_font_glyph61,
    (FONT_GLYPH *)&_cgui_fix_font_glyph62,
    (FONT_GLYPH *)&_cgui_fix_font_glyph63,
    (FONT_GLYPH *)&_cgui_fix_font_glyph64,
    (FONT_GLYPH *)&_cgui_fix_font_glyph65,
    (FONT_GLYPH *)&_cgui_fix_font_glyph66,
    (FONT_GLYPH *)&_cgui_fix_font_glyph67,
    (FONT_GLYPH *)&_cgui_fix_font_glyph68,
    (FONT_GLYPH *)&_cgui_fix_font_glyph69,
    (FONT_GLYPH *)&_cgui_fix_font_glyph70,
    (FONT_GLYPH *)&_cgui_fix_font_glyph71,
    (FONT_GLYPH *)&_cgui_fix_font_glyph72,
    (FONT_GLYPH *)&_cgui_fix_font_glyph73,
    (FONT_GLYPH *)&_cgui_fix_font_glyph74,
    (FONT_GLYPH *)&_cgui_fix_font_glyph75,
    (FONT_GLYPH *)&_cgui_fix_font_glyph76,
    (FONT_GLYPH *)&_cgui_fix_font_glyph77,
    (FONT_GLYPH *)&_cgui_fix_font_glyph78,
    (FONT_GLYPH *)&_cgui_fix_font_glyph79,
    (FONT_GLYPH *)&_cgui_fix_font_glyph80,
    (FONT_GLYPH *)&_cgui_fix_font_glyph81,
    (FONT_GLYPH *)&_cgui_fix_font_glyph82,
    (FONT_GLYPH *)&_cgui_fix_font_glyph83,
    (FONT_GLYPH *)&_cgui_fix_font_glyph84,
    (FONT_GLYPH *)&_cgui_fix_font_glyph85,
    (FONT_GLYPH *)&_cgui_fix_font_glyph86,
    (FONT_GLYPH *)&_cgui_fix_font_glyph87,
    (FONT_GLYPH *)&_cgui_fix_font_glyph88,
    (FONT_GLYPH *)&_cgui_fix_font_glyph89,
    (FONT_GLYPH *)&_cgui_fix_font_glyph90,
    (FONT_GLYPH *)&_cgui_fix_font_glyph91,
    (FONT_GLYPH *)&_cgui_fix_font_glyph92,
    (FONT_GLYPH *)&_cgui_fix_font_glyph93,
    (FONT_GLYPH *)&_cgui_fix_font_glyph94,
    (FONT_GLYPH *)&_cgui_fix_font_glyph95,
    (FONT_GLYPH *)&_cgui_fix_font_glyph96,
    (FONT_GLYPH *)&_cgui_fix_font_glyph97,
    (FONT_GLYPH *)&_cgui_fix_font_glyph98,
    (FONT_GLYPH *)&_cgui_fix_font_glyph99,
    (FONT_GLYPH *)&_cgui_fix_font_glyph100,
    (FONT_GLYPH *)&_cgui_fix_font_glyph101,
    (FONT_GLYPH *)&_cgui_fix_font_glyph102,
    (FONT_GLYPH *)&_cgui_fix_font_glyph103,
    (FONT_GLYPH *)&_cgui_fix_font_glyph104,
    (FONT_GLYPH *)&_cgui_fix_font_glyph105,
    (FONT_GLYPH *)&_cgui_fix_font_glyph106,
    (FONT_GLYPH *)&_cgui_fix_font_glyph107,
    (FONT_GLYPH *)&_cgui_fix_font_glyph108,
    (FONT_GLYPH *)&_cgui_fix_font_glyph109,
    (FONT_GLYPH *)&_cgui_fix_font_glyph110,
    (FONT_GLYPH *)&_cgui_fix_font_glyph111,
    (FONT_GLYPH *)&_cgui_fix_font_glyph112,
    (FONT_GLYPH *)&_cgui_fix_font_glyph113,
    (FONT_GLYPH *)&_cgui_fix_font_glyph114,
    (FONT_GLYPH *)&_cgui_fix_font_glyph115,
    (FONT_GLYPH *)&_cgui_fix_font_glyph116,
    (FONT_GLYPH *)&_cgui_fix_font_glyph117,
    (FONT_GLYPH *)&_cgui_fix_font_glyph118,
    (FONT_GLYPH *)&_cgui_fix_font_glyph119,
    (FONT_GLYPH *)&_cgui_fix_font_glyph120,
    (FONT_GLYPH *)&_cgui_fix_font_glyph121,
    (FONT_GLYPH *)&_cgui_fix_font_glyph122,
    (FONT_GLYPH *)&_cgui_fix_font_glyph123,
    (FONT_GLYPH *)&_cgui_fix_font_glyph124,
    (FONT_GLYPH *)&_cgui_fix_font_glyph125,
    (FONT_GLYPH *)&_cgui_fix_font_glyph126,
    (FONT_GLYPH *)&_cgui_fix_font_glyph127,
    (FONT_GLYPH *)&_cgui_fix_font_glyph128,
    (FONT_GLYPH *)&_cgui_fix_font_glyph129,
    (FONT_GLYPH *)&_cgui_fix_font_glyph130,
    (FONT_GLYPH *)&_cgui_fix_font_glyph131,
    (FONT_GLYPH *)&_cgui_fix_font_glyph132,
    (FONT_GLYPH *)&_cgui_fix_font_glyph133,
    (FONT_GLYPH *)&_cgui_fix_font_glyph134,
    (FONT_GLYPH *)&_cgui_fix_font_glyph135,
    (FONT_GLYPH *)&_cgui_fix_font_glyph136,
    (FONT_GLYPH *)&_cgui_fix_font_glyph137,
    (FONT_GLYPH *)&_cgui_fix_font_glyph138,
    (FONT_GLYPH *)&_cgui_fix_font_glyph139,
    (FONT_GLYPH *)&_cgui_fix_font_glyph140,
    (FONT_GLYPH *)&_cgui_fix_font_glyph141,
    (FONT_GLYPH *)&_cgui_fix_font_glyph142,
    (FONT_GLYPH *)&_cgui_fix_font_glyph143,
    (FONT_GLYPH *)&_cgui_fix_font_glyph144,
    (FONT_GLYPH *)&_cgui_fix_font_glyph145,
    (FONT_GLYPH *)&_cgui_fix_font_glyph146,
    (FONT_GLYPH *)&_cgui_fix_font_glyph147,
    (FONT_GLYPH *)&_cgui_fix_font_glyph148,
    (FONT_GLYPH *)&_cgui_fix_font_glyph149,
    (FONT_GLYPH *)&_cgui_fix_font_glyph150,
    (FONT_GLYPH *)&_cgui_fix_font_glyph151,
    (FONT_GLYPH *)&_cgui_fix_font_glyph152,
    (FONT_GLYPH *)&_cgui_fix_font_glyph153,
    (FONT_GLYPH *)&_cgui_fix_font_glyph154,
    (FONT_GLYPH *)&_cgui_fix_font_glyph155,
    (FONT_GLYPH *)&_cgui_fix_font_glyph156,
    (FONT_GLYPH *)&_cgui_fix_font_glyph157,
    (FONT_GLYPH *)&_cgui_fix_font_glyph158,
    (FONT_GLYPH *)&_cgui_fix_font_glyph159,
    (FONT_GLYPH *)&_cgui_fix_font_glyph160,
    (FONT_GLYPH *)&_cgui_fix_font_glyph161,
    (FONT_GLYPH *)&_cgui_fix_font_glyph162,
    (FONT_GLYPH *)&_cgui_fix_font_glyph163,
    (FONT_GLYPH *)&_cgui_fix_font_glyph164,
    (FONT_GLYPH *)&_cgui_fix_font_glyph165,
    (FONT_GLYPH *)&_cgui_fix_font_glyph166,
    (FONT_GLYPH *)&_cgui_fix_font_glyph167,
    (FONT_GLYPH *)&_cgui_fix_font_glyph168,
    (FONT_GLYPH *)&_cgui_fix_font_glyph169,
    (FONT_GLYPH *)&_cgui_fix_font_glyph170,
    (FONT_GLYPH *)&_cgui_fix_font_glyph171,
    (FONT_GLYPH *)&_cgui_fix_font_glyph172,
    (FONT_GLYPH *)&_cgui_fix_font_glyph173,
    (FONT_GLYPH *)&_cgui_fix_font_glyph174,
    (FONT_GLYPH *)&_cgui_fix_font_glyph175,
    (FONT_GLYPH *)&_cgui_fix_font_glyph176,
    (FONT_GLYPH *)&_cgui_fix_font_glyph177,
    (FONT_GLYPH *)&_cgui_fix_font_glyph178,
    (FONT_GLYPH *)&_cgui_fix_font_glyph179,
    (FONT_GLYPH *)&_cgui_fix_font_glyph180,
    (FONT_GLYPH *)&_cgui_fix_font_glyph181,
    (FONT_GLYPH *)&_cgui_fix_font_glyph182,
    (FONT_GLYPH *)&_cgui_fix_font_glyph183,
    (FONT_GLYPH *)&_cgui_fix_font_glyph184,
    (FONT_GLYPH *)&_cgui_fix_font_glyph185,
    (FONT_GLYPH *)&_cgui_fix_font_glyph186,
    (FONT_GLYPH *)&_cgui_fix_font_glyph187,
    (FONT_GLYPH *)&_cgui_fix_font_glyph188,
    (FONT_GLYPH *)&_cgui_fix_font_glyph189,
    (FONT_GLYPH *)&_cgui_fix_font_glyph190,
    (FONT_GLYPH *)&_cgui_fix_font_glyph191,
    (FONT_GLYPH *)&_cgui_fix_font_glyph192,
    (FONT_GLYPH *)&_cgui_fix_font_glyph193,
    (FONT_GLYPH *)&_cgui_fix_font_glyph194,
    (FONT_GLYPH *)&_cgui_fix_font_glyph195,
    (FONT_GLYPH *)&_cgui_fix_font_glyph196,
    (FONT_GLYPH *)&_cgui_fix_font_glyph197,
    (FONT_GLYPH *)&_cgui_fix_font_glyph198,
    (FONT_GLYPH *)&_cgui_fix_font_glyph199,
    (FONT_GLYPH *)&_cgui_fix_font_glyph200,
    (FONT_GLYPH *)&_cgui_fix_font_glyph201,
    (FONT_GLYPH *)&_cgui_fix_font_glyph202,
    (FONT_GLYPH *)&_cgui_fix_font_glyph203,
    (FONT_GLYPH *)&_cgui_fix_font_glyph204,
    (FONT_GLYPH *)&_cgui_fix_font_glyph205,
    (FONT_GLYPH *)&_cgui_fix_font_glyph206,
    (FONT_GLYPH *)&_cgui_fix_font_glyph207,
    (FONT_GLYPH *)&_cgui_fix_font_glyph208,
    (FONT_GLYPH *)&_cgui_fix_font_glyph209,
    (FONT_GLYPH *)&_cgui_fix_font_glyph210,
    (FONT_GLYPH *)&_cgui_fix_font_glyph211,
    (FONT_GLYPH *)&_cgui_fix_font_glyph212,
    (FONT_GLYPH *)&_cgui_fix_font_glyph213,
    (FONT_GLYPH *)&_cgui_fix_font_glyph214,
    (FONT_GLYPH *)&_cgui_fix_font_glyph215,
    (FONT_GLYPH *)&_cgui_fix_font_glyph216,
    (FONT_GLYPH *)&_cgui_fix_font_glyph217,
    (FONT_GLYPH *)&_cgui_fix_font_glyph218,
    (FONT_GLYPH *)&_cgui_fix_font_glyph219,
    (FONT_GLYPH *)&_cgui_fix_font_glyph220,
    (FONT_GLYPH *)&_cgui_fix_font_glyph221,
    (FONT_GLYPH *)&_cgui_fix_font_glyph222,
    (FONT_GLYPH *)&_cgui_fix_font_glyph223,
    (FONT_GLYPH *)&_cgui_fix_font_glyph224,
    (FONT_GLYPH *)&_cgui_fix_font_glyph225,
    (FONT_GLYPH *)&_cgui_fix_font_glyph226,
    (FONT_GLYPH *)&_cgui_fix_font_glyph227,
    (FONT_GLYPH *)&_cgui_fix_font_glyph228,
    (FONT_GLYPH *)&_cgui_fix_font_glyph229,
    (FONT_GLYPH *)&_cgui_fix_font_glyph230,
    (FONT_GLYPH *)&_cgui_fix_font_glyph231,
    (FONT_GLYPH *)&_cgui_fix_font_glyph232,
    (FONT_GLYPH *)&_cgui_fix_font_glyph233,
    (FONT_GLYPH *)&_cgui_fix_font_glyph234,
    (FONT_GLYPH *)&_cgui_fix_font_glyph235,
    (FONT_GLYPH *)&_cgui_fix_font_glyph236,
    (FONT_GLYPH *)&_cgui_fix_font_glyph237,
    (FONT_GLYPH *)&_cgui_fix_font_glyph238,
    (FONT_GLYPH *)&_cgui_fix_font_glyph239,
    (FONT_GLYPH *)&_cgui_fix_font_glyph240,
    (FONT_GLYPH *)&_cgui_fix_font_glyph241,
    (FONT_GLYPH *)&_cgui_fix_font_glyph242,
    (FONT_GLYPH *)&_cgui_fix_font_glyph243,
    (FONT_GLYPH *)&_cgui_fix_font_glyph244,
    (FONT_GLYPH *)&_cgui_fix_font_glyph245,
    (FONT_GLYPH *)&_cgui_fix_font_glyph246,
    (FONT_GLYPH *)&_cgui_fix_font_glyph247,
    (FONT_GLYPH *)&_cgui_fix_font_glyph248,
    (FONT_GLYPH *)&_cgui_fix_font_glyph249,
    (FONT_GLYPH *)&_cgui_fix_font_glyph250,
    (FONT_GLYPH *)&_cgui_fix_font_glyph251,
    (FONT_GLYPH *)&_cgui_fix_font_glyph252,
    (FONT_GLYPH *)&_cgui_fix_font_glyph253,
    (FONT_GLYPH *)&_cgui_fix_font_glyph254,
    (FONT_GLYPH *)&_cgui_fix_font_glyph255,
    0
};

static FONT_MONO_DATA _cgui_fix_font_range0 = {
    32, 256, /* begin, end */
    _cgui_fix_font_range0_glyphs,
    0
};

static FONT _cgui_fix_font = {
    &_cgui_fix_font_range0,
    13, /* height */
    (FONT_VTABLE *)0 /* color flag */
};



DATAFILE _cgui_fonts_datafile[] = {
    { &_cgui_font, DAT_ID('F', 'O', 'N', 'T'), 3990, _cgui_font_prop },
    { &_cgui_fix_font, DAT_ID('F', 'O', 'N', 'T'), 6733, _cgui_fix_font_prop },
    { 0, DAT_END, 0, 0 }
};



#ifdef ALLEGRO_USE_CONSTRUCTOR
CONSTRUCTOR_FUNCTION(static void _construct_me(void));
static void _construct_me(void)
{
    _construct_datafile(_cgui_fonts_datafile);
}
#endif



/* end of src/fontdata.c */



