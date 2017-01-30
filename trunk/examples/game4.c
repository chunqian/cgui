/* Just for fun we extend game3.c with the possibility to show multiple windows with simultaneous animations.
   We let the first window fill up the entire screen and let the subsequent ones be "normalized".

   Just for variation we now use a "status-field" instead of the edit-box for
   displaying the neumeric value of the spin-buttons. A status-field
   requires a call-back function, and it does not allow for editing. */

#include <time.h>
#include <allegro.h>
#include "cgui.h"
#include "ball.inc"

typedef struct t_game {
   int run;
   t_ball **balls;
   int nballs;
   int rate;
   int time;
   int first;
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
   t_ball *ball = data;
   int x_min, y_min, x_max, y_max;

   if (y < 0) {
      clear_to_color(bmp, makecol(255,255,255)); /* init */
      set_ball_background(ball, bmp);
   } else if (x < 0) {
      draw_ball(ball, &x_min, &y_min, &x_max, &y_max);
      SetBlitLimit(x_min, y_min, x_max, y_max);
   }
}

static int run_game(void *data)
{
   t_game *game = data;
   int i;

   while (game->time < current_time) {
      game->time += game->rate;
      for (i=0; i<game->nballs; i++)
         bounce(game->balls[i]);
   }
   ConditionalRefresh(NULL, 0);
   return game->run;
}

static void close_win(void *ball)
{
   (void)ball;
   CloseWin(NULL);
}

static void updater(int id, void *data, void *calldata, int reason)
{
   (void)data; (void)calldata; (void)reason;
   Refresh(id);
}

static void status_upd(void *data, char *string)
{
   int *size = data;
   sprintf(string, "Size = %d", *size);
}

static void make_ball_win(void *data)
{
   t_game *game = data;
   int id;
   t_ball *ball;

   game->nballs++;
   game->balls = ResizeMem(t_ball*, game->balls, game->nballs);
   ball = game->balls[game->nballs-1] = init_ball(makecol(255,255,255));

   if (game->first)
      MkDialogue(FILLSCREEN, "The Fantastic Bouncing Ball", 0);
   else
      MkDialogue(ADAPTIVE, "The Fantastic Bouncing Ball", W_FLOATING);
   AddButton(TOPLEFT, "\33~Quit", quit, game);
   AddCheck(RIGHT, "~Pause", &ball->pause);
   id = AddStatusField(RIGHT, 80, status_upd, &ball->diameter);
   HookSpinButtons(id, &ball->diameter, 1, 10, 1, 200);
   AddButton(RIGHT, "~New", make_ball_win, game);
   if (!game->first)
      AddButton(RIGHT, "~Close", close_win, ball);
   id = MkCanvas(DOWNLEFT | FILLSPACE | VERTICAL | HORIZONTAL, 200, 100, bounce_wrapper, ball);
   RegisterRefresh(id, updater, NULL);
   game->first = 0;

   DisplayWin();
}

int main(void)
{
   t_game game;
   int i;

   srand(time(NULL));
   InitCgui(640, 480, 15);
   LOCK_FUNCTION(timer_cb);
   LOCK_VARIABLE(current_time);
   install_int(timer_cb, 1);

   game.run = 1;
   game.rate = 15;
   game.time = 0;
   game.first = 1;
   game.nballs = 0;
   game.balls = NULL;

   make_ball_win(&game);

   CguiEventIterateFunction(run_game, &game);

   for (i=0; i<game.nballs; i++)
      destroy_ball(game.balls[i]);
   return 0;
}
END_OF_MAIN()
