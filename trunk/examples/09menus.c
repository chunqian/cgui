/* CGUI Example program. This example shows how to use menus. */
#include <string.h>
#include <allegro.h>
#include "cgui.h"

#define MAXLEN   20
#define MAXNRWIN 50

typedef struct t_win {
   int opt, fix_flag, id, used;
   char label[MAXLEN];
} t_win;

typedef struct t_windata {
   int nr, opt, fix_flag;
   t_win wins[MAXNRWIN];
} t_windata;

static void close_show_code(void *data)
{
   int *id = data;

   *id = 0;
   CloseWin(NULL);
}

static void show_code(void *data)
{
   int id;
   static int winid;
   (void)data;

   if (winid) {
      SetFocusOn(winid);
   } else {
      winid = MkDialogue(ADAPTIVE, "Code view", W_FLOATING);
      id = AddTextBox(DOWNLEFT, "../examples/09menus.c", 670, 54, TB_TEXTFILE|TB_FRAMESINK|TB_PREFORMAT|3);
      MakeStretchable(id, NULL, NULL, 0);
      AddButton(DOWNLEFT, "~Close", close_show_code, &winid);
      DisplayWin();
   }
}

static int win_options[] = {0, W_FLOATING, W_FLOATING | W_SIBLING };
static const char *opt_texts[3] =
                  {"Modal child", "Floating child", "Floating sister"};

void quit(void *data)
{
   (void)data;
   StopProcessEvents();
}

void destroy_window(void *data)
{
   t_win *w = data;

   w->used = 0;
   CloseWin(NULL);
}

void status_printer(void *data, char *string)
{
   t_win *w = data;

   strcpy(string, opt_texts[w->opt]);
}

void create_window(void *data)
{
   t_win *w = data;
   int options, id, i;

   CloseWin(NULL);
   w->used = 1;
   options = win_options[w->opt];
   if (w->fix_flag)
      options |= W_NOMOVE;

   w->id = MkDialogue(ADAPTIVE, w->label, options);
   StartContainer(TOPLEFT, ADAPTIVE, "Different ways to display a selection", CT_BORDER);

   StartContainer(TOPLEFT|FILLSPACE, ADAPTIVE, "Drop down:", CT_BORDER);
   id = AddDropDownS(DOWNLEFT|ALIGNRIGHT, 0, "Selected option:", &w->opt, opt_texts, 3);
   DeActivate(id);
   EndContainer();

   StartContainer(DOWNLEFT|FILLSPACE, ADAPTIVE, "Radio buttons:", CT_BORDER);
   id = MkRadioContainer(DOWNLEFT|ALIGNRIGHT, &w->opt, R_HORIZONTAL);
   for (i=0; i<3; i++)
      AddRadioButton(opt_texts[i]);
   EndRadioContainer();
   DeActivate(id);
   AddTag(LEFT, "Selected option:");
   EndContainer();

   StartContainer(DOWNLEFT|FILLSPACE, ADAPTIVE, "Tag:", CT_BORDER);
   AddTag(DOWNLEFT|ALIGNRIGHT, opt_texts[w->opt]);
   AddTag(LEFT, "Selected option:");
   EndContainer();

   StartContainer(DOWNLEFT|FILLSPACE, ADAPTIVE, "Text object:", CT_BORDER);
   AddTextBox(DOWNLEFT|ALIGNRIGHT, opt_texts[w->opt], 100, 0, TB_FRAMESINK);
   AddTag(LEFT, "Selected option:");
   EndContainer();

   StartContainer(DOWNLEFT|FILLSPACE, ADAPTIVE, "Status field:", CT_BORDER);
   AddStatusField(DOWNLEFT|ALIGNRIGHT, 100, status_printer, w);
   AddTag(LEFT, "Selected option:");
   EndContainer();

   EndContainer();

   AddButton(DOWNLEFT, "\33Close", destroy_window, w);
   DisplayWin();
}

void new_window_selection(void *data)
{
   t_windata *wd = data;
   t_win *w;
      int i,j;

   for (i=0; i<wd->nr; i++) {
      if (!wd->wins[i].used) {
         for (j=i+1; j<wd->nr; j++)
            wd->wins[j-1] = wd->wins[j];
         wd->nr--;
      }
   }
   w = wd->wins + wd->nr++;
   memset(w, 0, sizeof(t_win));
   w->opt = wd->opt;
   w->fix_flag = wd->fix_flag;

   MkDialogue(ADAPTIVE, "Create A Window", 0);
   AddEditBox(TOPLEFT, 100, "Window name:", FSTRING, MAXLEN, w->label);
   AddButton(DOWNLEFT, "OK", create_window, w);
   AddButton(RIGHT, "\33Cancel", CloseWin, NULL);
   DisplayWin();
}

void file_menu(void *data)
{
   t_windata *wd = data;
   int id;

   id = MkMenuItem(0, "New Window", "Ctrl-N", new_window_selection, wd);
   if (wd->nr >= MAXNRWIN)
      DeActivate(id);
   MkMenuItem(0, "Quit", "Ctrl-Q", quit, NULL);
}

void options_menu(void *data)
{
   t_windata *wd = data;

   int id = MkMenuCheck(&wd->fix_flag, "Not movable");
   DeActivate(id);
   MkGroove();
   MkMenuRadio(&wd->opt, 3, opt_texts[0], opt_texts[1], opt_texts[2]);
}

void select_a_window(void *data)
{
   t_win *w = data;

   SetFocusOn(w->id);
}

void windows_menu(void *data)
{
   t_windata *wd = data;
   int i;

   for (i = 0; i < wd->nr; i++)
      if (wd->wins[i].used)
         MkMenuItem(0, wd->wins[i].label, "", select_a_window, wd->wins + i);
   MkMenuItem(0, "Show code", "", show_code, NULL);
   MkGroove();
   MkMenuItem(1, "Recursive menu test", "", windows_menu, wd);
   MkMenuItem(1, "Duplicate test", "", windows_menu, wd);
}

int main(void)
{
   t_windata wd;
   memset(&wd, 0, sizeof(t_windata));
   wd.opt = 1;
   InitCgui(1024, 768, 15);
   MkDialogue(FILLSCREEN, "Main Window: menu example", 0);
   MakeMenuBar();
   MkMenuBarItem("File", file_menu, &wd);
   MkMenuBarItem("Options", options_menu, &wd);
   MkMenuBarItem("Windows", windows_menu, &wd);
   EndMenuBar();
   AddTextBox(DOWNLEFT, "This example show how to use menus._"
   "Some of the menu-selections (the recursive) have no special meaning._"
   "They are just there to show how it looks when using sub-menus._"
   "If you create some child-windows their names will show up as selections "
   "in the \"Windows\" menu, and you can select active child-window using the menu._"
   "The created windows shows the options that were used when the window was "
   "created (such windows options can only be specified at creation time)._ _"
   "The purpose is to demonstrate some various ways to display a selection._"
   "It is a matter of taste which one to use. A strict personal opinion is that "
   "the ones that must be deactivated (and therefore are \"greyed\"), should not "
   "be used. If data may be subject for change (which is not the case here) the "
   "status-field is the prefered one." , 500, 0, TB_FRAMESINK|TB_LINEFEED_);
   SetHotKey(0, quit, NULL, KEY_Q, KEY_Q);
   SetHotKey(0, new_window_selection, &wd, KEY_N, KEY_N);

   DisplayWin();
   ProcessEvents();
   DeInitCgui();
   return 0;
}
END_OF_MAIN()
