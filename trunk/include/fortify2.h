#ifndef _FORTIFY2_H_
#define _FORTIFY2_H_

#ifdef FORTIFY

#include "fortify.h"

void fortify_init(void);
void fortify_exit(void);

extern int always_close_fortify_file, fortify_write_traceback;

#endif

#endif
