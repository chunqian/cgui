/* CGUI Example program showing how to build some type of "game" on top of CGUI.

   This time we put all the objects into a window instead. This is how one normally builds user interfaces.

   We also uses another type of objects for re-sizing the ball: a combination of an edit-box and a pair of spinbuttons.
   Read the help section for `SetHotKey' to see how you use + and - as hot keys for re-sizing the ball. */

#include <time.h>
#include <allegro.h>
#include "cgui.h"
#include "ball.inc"


typedef struct t_game {
   int run;
   t_ball *ball;
   int rate;
   int time;
   int canvas_id;
} t_game;

volatile static int current_time;

static void timer_cb(void)
{
   current_time++;
}
END_OF_STATIC_FUNCTION(timer_cb);

static void quit(void *data)
{
   t_game *game = data;
   game->run = 0;
}

static void bounce_wrapper(BITMAP *bmp, int x, int y, void *data)
{
   t_game *game = data;
   int x_min, y_min, x_max, y_max;

   if (y < 0) {
      clear_to_color(bmp, makecol(255,255,255)); /* init */
      set_ball_background(game->ball, bmp);
   } else if (x < 0) {
      draw_ball(game->ball, &x_min, &y_min, &x_max, &y_max);
      SetBlitLimit(x_min, y_min, x_max, y_max);
   }
}

static int run_game(void *data)
{
   t_game *game = data;

   while (game->time < current_time) {
      game->time += game->rate;
      bounce(game->ball);
   }
   Refresh(game->canvas_id);
   return game->run;
}

int main(void)
{
   t_game game;
   int id;

   srand(time(NULL));
   InitCgui(640, 480, 15);
   LOCK_FUNCTION(timer_cb);
   LOCK_VARIABLE(current_time);
   install_int(timer_cb, 1);

   game.run = 1;
   game.rate = 15;
   game.time = 0;
   game.ball = init_ball(makecol(255,255,255));

   /* This line creates the window (added to "game2") */
   MkDialogue(FILLSCREEN, "The Fantastic Bouncing Ball", 0);
   AddButton(TOPLEFT, "\33~Quit", quit, &game);
   AddCheck(RIGHT, "~Pause", &game.ball->pause);
   id = AddEditBox(RIGHT, 50, "Size", FINT, 10, &game.ball->diameter);
   HookSpinButtons(id, &game.ball->diameter, 1, 10, 1, 200); /* Spin-interval 1-200, steps 1-10 */
   game.canvas_id = MkCanvas(DOWNLEFT | FILLSPACE | VERTICAL | HORIZONTAL, 200, 100, bounce_wrapper, &game);
   DisplayWin();

   CguiEventIterateFunction(run_game, &game);

   destroy_ball(game.ball);
   return 0;
}
END_OF_MAIN()
