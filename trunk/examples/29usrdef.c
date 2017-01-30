/* CGUI Example program showing how to use the user defined widget.
*/

#include <allegro.h>
#include "cgui.h"

static int container_id;

static void shut_down(void *data)
{
   (void)data;
   CloseWin(NULL);
   StopProcessEvents();
}

static void make_widgets(char **text);
static void change(void *data)
{
   char **string = data;
   CloseWin(NULL);
   SelectContainer(container_id);
   EmptyContainer(container_id);
   make_widgets(string);
   ReBuildContainer(container_id);
}

static void user_defiend_widget_select(void *data)
{
   char **string = data;
   /* Here goes the code that is a respons to the click on the user defined widget. */
   MkDialogue(ADAPTIVE, "", 0);
   AddEditBox(TOPLEFT, 100, "Change text to widget", FPTRSTR, 0, string);
   AddButton(RIGHT, "Change!", change, string);
   AddButton(DOWNLEFT|ALIGNRIGHT, "Close", CloseWin, NULL);
   DisplayWin();
}

static void draw(void *data, BITMAP *bmp)
{
   char **string = data;
   _soft_ellipsefill(bmp, bmp->w/2, bmp->h/2, bmp->w/2-20, bmp->h/2-1, makecol(255,255,255));
   _soft_floodfill(bmp, 0, 0, makecol(210,210,210));
   textout_centre_ex(bmp, GetCguiFont(), *string, bmp->w/2, 4, makecol(0,0,0), -1);
}

static void make_widgets(char **text)
{
   int width = 200, height = 80;
   AddUserDefinedWidget(TOPLEFT, width, height, draw, user_defiend_widget_select, text);
   AddButton(DOWNLEFT|ALIGNRIGHT, "Change", user_defiend_widget_select, text);
   AddButton(DOWNLEFT|ALIGNRIGHT, "\33E~xit", shut_down, NULL);
}

int main(void)
{
   char *init_text = strdup("Text");
   InitCgui(1024, 768, 15);
   MkDialogue(ADAPTIVE, "A user defined widget", 0);
   container_id = StartContainer(DOWNLEFT, ADAPTIVE, "", 0);
   make_widgets(&init_text);
   DisplayWin();
   ProcessEvents();
   free(init_text);
   return 0;
}
END_OF_MAIN()
