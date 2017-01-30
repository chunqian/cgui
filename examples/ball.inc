/* This is actually not a demonstration of a useful game - it is just here to show some properties of
   CGUI that may be interesting if you plan on using it when developing a game. */

#include <allegro.h>
#include <math.h>
#include <stdlib.h>

#include "cgui/mem.h"

typedef struct t_ball {
   int diameter;
   int prev_diameter;
   int pending_resize;
   int prev_x, prev_y;
   double x, y;
   double dx, dy;
   double velocity;
   int r, g, b;
   int bcol;
   int pause;
   BITMAP *bmp;
   BITMAP *circle;
} t_ball;

void bounce(t_ball *ball)
{
   double ang;

   if (ball->pause)
      return;

   ball->x += ball->dx;
   ball->y += ball->dy;
   if (ball->x < 0) {	  /* hit left border */
      ball->x = 0;
      if (ball->dy > 0) {
         ang = 5.497787 + ((rand() % 11) - 5) * 0.01;	/* 7pi/4 */
         ball->dx = ball->velocity * cos(ang);
         ball->dy = ball->velocity * -sin(ang);
      } else {
         ang = 0.785398 + ((rand() % 11) - 5) * 0.01;	/* pi/4 */
         ball->dx = ball->velocity * cos(ang);
         ball->dy = ball->velocity * -sin(ang);
      }
   }
   if (ball->x > ball->bmp->w - ball->diameter) {	/* hit right border */
      ball->x = ball->bmp->w - ball->diameter;
      if (ball->dy > 0) {
         ang = 3.926991 + ((rand() % 11) - 5) * 0.01;	/* 5pi/4 */
         ball->dx = ball->velocity * cos(ang);
         ball->dy = ball->velocity * -sin(ang);
      } else {
         ang = 2.356194 + ((rand() % 11) - 5) * 0.01;	/* 3pi/4 */
         ball->dx = ball->velocity * cos(ang);
         ball->dy = ball->velocity * -sin(ang);
      }
   }
   if (ball->y < 0) {	  /* hit top border */
      ball->y = 0;
      if (ball->dx > 0) {
         ang = 5.497787 + ((rand() % 11) - 5) * 0.01;	/* 7pi/4 */
         ball->dx = ball->velocity * cos(ang);
         ball->dy = ball->velocity * -sin(ang);
      } else {
         ang = 3.926991 + ((rand() % 11) - 5) * 0.01;	/* 5pi/4 */
         ball->dx = ball->velocity * cos(ang);
         ball->dy = ball->velocity * -sin(ang);
      }
   }
   if (ball->y > ball->bmp->h - ball->diameter) {	/* hit bottom border */
      ball->y = ball->bmp->h - ball->diameter;
      if (ball->dx > 0) {
         ang = 0.785398 + ((rand() % 11) - 5) * 0.01;	/* pi/4 */
         ball->dx = ball->velocity * cos(ang);
         ball->dy = ball->velocity * -sin(ang);
      } else {
         ang = 2.356194 + ((rand() % 11) - 5) * 0.01;	/* 3pi/4 */
         ball->dx = ball->velocity * cos(ang);
         ball->dy = ball->velocity * -sin(ang);
      }
   }
}

void new_size(t_ball *ball, int dr)
{
   if (ball->diameter += dr*2 > 0) {
      ball->diameter += dr*2;
      ball->pending_resize = 1;
      destroy_bitmap(ball->circle);
      ball->circle = create_bitmap(ball->diameter, ball->diameter);
      clear_to_color(ball->circle, bitmap_mask_color(ball->circle));
      circlefill(ball->circle, ball->diameter/2, ball->diameter/2, ball->diameter/2, makecol(ball->r, ball->g, ball->b));
   }
}

t_ball *init_ball(int bcol)
{
   t_ball *ball;

   ball = GetMem0(t_ball, 1);
   ball->prev_diameter = ball->diameter = 17;
   ball->x = rand() % 300;
   ball->y = rand() % 300;
   ball->bcol = bcol;
   if (rand() & 0x80)
      ball->dx = 3;
   else
      ball->dx = -3;
   if (rand() & 0x40)
      ball->dy = 4;
   else
      ball->dy = -4;
   ball->velocity = 5.0;
   ball->r = rand() % 256;
   ball->g = rand() % 256;
   ball->b = rand() % 256;
   if (ball->r + ball->g + ball->b > 720) {	/* not too bright */
      switch (rand() % 3) {
      case 0:
         ball->r = 0;
         break;
      case 1:
         ball->g = 0;
         break;
      case 2:
         ball->b = 0;
         break;
      }
   }
   new_size(ball, 0);
   return ball;
}

void draw_ball(t_ball *ball, int *x_min, int *y_min, int *x_max, int *y_max)
{
   int x1, x2, y1, y2;

   x1 = ball->prev_x;
   y1 = ball->prev_y;
   x2 = ball->prev_x + ball->prev_diameter;
   y2 = ball->prev_y + ball->prev_diameter;

   ball->prev_x = ball->x;
   ball->prev_y = ball->y;

   rectfill(ball->bmp, x1, y1, x2, y2, ball->bcol);
   if (ball->prev_diameter != ball->diameter) {
      new_size(ball, (ball->diameter - ball->prev_diameter) / 2);
   }
   ball->prev_diameter = ball->diameter;
   draw_sprite(ball->bmp, ball->circle, ball->x, ball->y);
   if (x_min) {
      *x_min = MIN(x1, ball->x);
   }
   if (y_min) {
      *y_min = MIN(y1, ball->y);
   }
   if (x_max) {
      *x_max = MAX(x2, ball->x + ball->diameter);
   }
   if (y_max) {
      *y_max = MAX(y2, ball->y + ball->diameter);
   }
}

void destroy_ball(t_ball *ball)
{
   destroy_bitmap(ball->circle);
   free(ball);
}

void set_ball_background(t_ball *ball, BITMAP *bmp)
{
   ball->bmp = bmp;
}
