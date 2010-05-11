#ifndef NCURSE_H_INCLUDED
#define NCURSE_H_INCLUDED

#include "init.h"

#include <ncurses.h>

void *
ncurseApplication (void *arg);

void 
windowInit (void);

void
print_in_middle(WINDOW *win, int starty, int startx, int width, char *string,
                chtype color);

#endif
