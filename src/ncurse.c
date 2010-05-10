#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#ifdef HAVE_NCURSESW_CURSES_H
#include <ncursesw/ncurses.h>
#elif HAVE_NCURSES_H
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include "init.h"
#include "ncurse.h"

#define ERROR fprintf (stderr, \
        "%s:%d Error (%d) : %s\n", \
        __FILE__, __LINE__, \
        errno, strerror(errno))

extern pthread_mutex_t mutex;

void *
ncurseApplication (void *arg)
{
    clear();
    data_t *data = arg;
    windowInit();

        int row = 0;
        int col = 0;
        getmaxyx(stdscr,row,col);

        WINDOW *statusWindow;
        statusWindow = create_statusWindow(col, row, data);

        delwin (statusWindow);
    }
}

void windowInit (void)
{
    initscr();
    raw();
    keypad(stdscr, TRUE);
    
    //noecho();
    //curs_set(0);
    //echo();
    curs_set(1);

    refresh();
}

WINDOW *
create_statusWindow (int col, int row, data_t *data)
{
    WINDOW *local_win = NULL;
    local_win = newwin (row, col, 0, 0);

    status_t *cur_status = NULL;
    if ((cur_status = initStatus(cur_status)) == NULL)
        ERROR;
    if (data->statuses->first != NULL) {
        *cur_status = *data->statuses->first;

        int i;
        for (i = row-2; i>0; i--) {

        if (strlen (cur_status->text) > col-17)
            i--;

            mvwprintw (local_win, i, 2, cur_status->pseudo);
            mvwprintw (local_win, i, 15, cur_status->text);

            if(cur_status->next != NULL)
                *cur_status = *cur_status->next;
            else
                break;
        }
    }
    wrefresh (local_win);
    return local_win;
}

void windowStatus (data_t *data)
{   
    erase();
    int row = 0;
    int col = 0;
    getmaxyx(stdscr, row, col);

    status_t *cur_status = NULL;
    if ((cur_status = initStatus(cur_status)) == NULL)
        ERROR;
    if (data->statuses->first != NULL) {    
        *cur_status = *data->statuses->first;
    
        int i;
        for (i = row-2; i>0; i--) {
            
            if (strlen (cur_status->text) > col-17)
                i--;

            move(i, 2);
            printw("%s", cur_status->pseudo);
            move(i, 15);
            printw("%s", cur_status->text);

            if(cur_status->next != NULL)
                *cur_status = *cur_status->next;
            else
                break;
        }
    }

    refresh();
}
