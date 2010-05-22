#ifndef NCURSE_H_INCLUDED
#define NCURSE_H_INCLUDED

#include "init.h"

#include <ncurses.h>
#include <menu.h>

typedef struct window_status_t
{
    data_t  *data;
    ITEM   **items;
    MENU    *menu;
    WINDOW  *win;
    int      refresh;
} window_status_t;


void 
windowInit (void);

void *
ncurseApplication (void *arg);

void *
refresh_status_window (void *arg);

void
retweet_window(window_status_t *window_status, status_t *retweet_status);

void 
detail_status_window (status_t *display_status);

void 
send_tweet_window (window_status_t *window_status);

void 
help_window (void);

void
follow_window(config_t *config);

#endif
