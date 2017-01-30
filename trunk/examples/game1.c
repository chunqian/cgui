/* CGUI Example program This example shows the usage of the simple "requester" in conjunction with a "game" (illustrated by a bouncing ball)
   that is already running.

   As you can see, there is only one line of code that has anything to do with CGUI.

   The requester-window that will be opened when pressing the esc-key is movable.
   If you don't like the automatically generated short-cuts you can specify your own by prefixing the desired letter in the requester
   string with a tilde character (~). */

#include <time.h>

#include <allegro.h>
#include "cgui.h"
#include "ball.inc"

#if ALLEGRO_SUB_VERSION == 0
#define textout_ex(bmp, f, s, x, y, col, bgcol) textout(bmp, f, s, x, y, col)
#endif

static int current_time, rate, time_freezed;

static void timer_cb(void)
{
   if (!time_freezed)
      current_time++;
}
END_OF_STATIC_FUNCTION(timer_cb);

int main(void)
{
   t_ball *ball;
   int cheight, run = 1;
   BITMAP *background, *console;

   allegro_init();
   install_timer();
   srand(time(NULL));
   LOCK_FUNCTION(timer_cb);
   LOCK_VARIABLE(current_time);
   LOCK_VARIABLE(current_time);
   LOCK_VARIABLE(time_freezed);
   install_int(timer_cb, 1);
   set_color_depth(15);
   if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0) < 0) {
      printf("Unable to set that graphics mode. Try another one!\n");
      return 0;
   }
   rate = 15;
   ball = init_ball(makecol(255, 255, 255));
   install_keyboard();
   cheight = text_height(font) + 2;
   background = create_sub_bitmap(screen, 0, 0, SCREEN_W, SCREEN_H - cheight);
   set_ball_background(ball, background);
   console = create_sub_bitmap(screen, 0, SCREEN_H - cheight, SCREEN_W, cheight);

   clear_to_color(background, makecol(255, 255, 255));
   clear_to_color(console, makecol(0, 0, 0));
   textout_ex(console, font, "Use the ESC key to quit, + and - keys of keypad to control the size", 2, 1, makecol(0, 255, 0), -1);
   while (run) {
      if (current_time > rate) {
         vsync();
         bounce(ball);
         draw_ball(ball, 0, 0, 0, 0);
         current_time = 0;
      }
      if (keypressed()) {
         switch (readkey()>>8) {
         case KEY_ESC:
            time_freezed = 1;
            switch (Req("Exit request", "Please make your decision|Continue|Exit|Pause")) {
            case 0:
               time_freezed = 0;
               ball->pause = 0;
               break;
            case 1:
               run = 0;
               break;
            case 2:
               ball->pause = 1;
               break;
            }
            break;
         case KEY_PLUS_PAD:
            new_size(ball, 1);
            break;
         case KEY_MINUS_PAD:
            new_size(ball, -1);
            break;
         }
      }
   }
   destroy_ball(ball);
   destroy_bitmap(console);
   destroy_bitmap(background);
   return 0;
}
END_OF_MAIN()
