#ifndef NCURSE_H_INCLUDED
#define NCURSE_H_INCLUDED

#include "init.h"

#include <ncurses.h>

void *
ncurseApplication (void *arg);

void 
windowInit (void);

void 
windowBasic (data_t *data);

WINDOW *
create_statusWindow (int col, int row, data_t *data);

WINDOW *
update_statusWindow (WINDOW *local_win, int col, int row, data_t *data);

#endif
