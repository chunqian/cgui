/* CGUI Example program showing the usage of a sliders. */
#include <allegro.h>
#include "cgui.h"

static void shut_down(void *data)
{
   (void)data;
   StopProcessEvents();
}

static void show_code(void *data)
{
   int id;
   (void)data;

   MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
   id = AddTextBox(DOWNLEFT, "../examples/07slider.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
   MakeStretchable(id, NULL, NULL, 0);
   AddButton(DOWNLEFT, "Close", CloseWin, NULL);
   DisplayWin();
}

void make_object_window(void)
{
   static int val[]={1,1,1,1,1,1,1,0}, id;
   static float fval=3.14;

   MkDialogue(ADAPTIVE, "Sliders", 0);
   AddTextBox(DOWNLEFT, "This example shows how to make and use "
   "a slider object. There are some different styles and options to choose "
   "between.", 500, 0, TB_FRAMESINK|TB_LINEFEED_);

   AddTag(DOWNLEFT, "The default style:");
   AddSlider(RIGHT, 100, val+0, 1, 100, 0, 0);
   AddTag(RIGHT, "..here with a scale..");
   AddSlider(RIGHT, 100, val+1, 1, 100, SL_SCALE, 0);
   AddTag(RIGHT, "..and here also with lables");
   AddSlider(RIGHT, 100, val+2, 1, 100, SL_SCALE|SL_LABEL, 0);
   MkGroove();
   AddTag(DOWNLEFT, "There are 3 other styles that can be used");
   AddTag(DOWNLEFT, "Style1:");
   AddSlider(RIGHT, 100, val+3, 1, 100, SL_STYLE1|SL_SCALE|SL_LABEL, 0);
   AddTag(RIGHT, "Style2:");
   AddSlider(RIGHT, 100, val+4, 1, 100, SL_STYLE2|SL_SCALE|SL_LABEL, 0);
   AddTag(RIGHT, "Style3:");
   AddSlider(RIGHT, 100, val+5, 1, 100, SL_STYLE3|SL_SCALE|SL_LABEL, 0);
   MkGroove();
   AddTag(DOWNLEFT, "Like other CGUI-objects they can be made re-sizeable:");
   id = AddSlider(DOWNLEFT, 100, val+6, 1, 100, SL_SCALE|SL_LABEL, 0);
   MakeStretchable(id, NULL, NULL, NO_VERTICAL);
   MkGroove();
   AddTag(DOWNLEFT, "A good idea may be to hook it on an edit-box:");
   id = AddEditBox(DOWNLEFT, 40, "Current value:", FINT, 0, val+7);
   id = AddSlider(DOWNLEFT, 100, val+7, 100, 199, SL_SCALE|SL_LABEL, id);
   MakeStretchable(id, NULL, NULL, NO_VERTICAL);
   MkGroove();
   StartContainer(DOWNLEFT, ADAPTIVE, "Floating point version", CT_BORDER);
   id = AddEditBox(DOWNLEFT|ALIGNBOTTOM, 40, "Value:", FFLOAT, 0, &fval);
   AddSliderFloat(RIGHT, 100, &fval, 3.14, 9.81, 2, SL_SCALE|SL_LABEL, id);
   EndContainer();

   AddButton(DOWNLEFT, "\33E~xit", shut_down, NULL);
   AddButton(RIGHT, "Show code", show_code, NULL);
   DisplayWin();
}

int main(void)
{
   InitCgui(1024, 768, 15);
   make_object_window();
   ProcessEvents();
   return 0;
}
END_OF_MAIN()
