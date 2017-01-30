#ifndef MENBARCM_H
#define MENBARCM_H

struct t_window;
extern struct t_window *MenuBarCatchMouse(void (*Action) (void *), void *data);

#endif
