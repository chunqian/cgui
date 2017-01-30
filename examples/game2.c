#include <time.h>

#include <allegro.h>
#include "cgui.h"
#include "ball.inc"

#define CONSOLE_HEIGHT 30

typedef struct t_game {
   int run;
   t_ball *ball;
   int rate;
   int time;
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

static void shrink(void *ball)
{
   new_size(ball, -1);
}

static void grow(void *ball)
{
   new_size(ball, 1);
}

static int run_game(void *data)
{
   t_game *game = data;
   int x_min, y_min, x_max, y_max;

   while (game->time < current_time) {
      game->time += game->rate;
      bounce(game->ball);
   }
   draw_ball(game->ball, &x_min, &y_min, &x_max, &y_max);
   /* SetBlitLimit is just to avoid update of an unnecessary big area of the screen. */
   SetBlitLimit(x_min, y_min, x_max, y_max);
   Refresh(ID_DESKTOP);
   return game->run;
}

int main(void)
{
   t_game game;
   BITMAP *bmp, *background;

   srand(time(NULL));
   InitCgui(640, 480, 15);
   LOCK_FUNCTION(timer_cb);
   LOCK_VARIABLE(current_time);
   install_int(timer_cb, 1);

   game.run = 1;
   game.rate = 15;
   game.time = 0;
   bmp = create_bitmap(SCREEN_W, SCREEN_H);
   background = create_sub_bitmap(bmp, 0, 0, SCREEN_W, SCREEN_H - CONSOLE_HEIGHT);
   clear_to_color(bmp, makecol(255,255,255));
   game.ball = init_ball(makecol(255,255,255));
   set_ball_background(game.ball, background);

   MkDialogue(SCREEN_W, CONSOLE_HEIGHT, NULL, 0);
   AddButton(TOPLEFT, "\33~Quit", quit, &game);
   AddCheck(RIGHT, "~Pause", &game.ball->pause);
   AddButton(RIGHT, " - ", shrink, game.ball);
   AddButton(RIGHT, " + ", grow, game.ball);
   SetWindowPosition(0, SCREEN_H - CONSOLE_HEIGHT);
   DisplayWin();

   DesktopImage(bmp);
   Refresh(ID_DESKTOP);

   /* Run the "game loop" */
   CguiEventIterateFunction(run_game, &game);

   destroy_ball(game.ball);
   destroy_bitmap(background);
   destroy_bitmap(bmp);
   return 0;
}
END_OF_MAIN()
