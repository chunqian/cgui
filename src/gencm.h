#ifndef GENCM_H
#define GENCM_H

struct t_window;

extern struct t_window *CatchMouseWindow(void (*Action) (void *), void *data);

#endif
