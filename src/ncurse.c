#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ncurses.h>
#include <unistd.h>

#include "ncurse.h"
#include "init.h"

#define ERROR fprintf (stderr, \
        "%s:%d Error (%d) : %s\n", \
        __FILE__, __LINE__, \
        errno, strerror(errno))

extern pthread_mutex_t mutex;

void *
ncurseApplication (void *arg)
{
    data_t *data = arg;
    windowInit();
    while(1) {
        pthread_mutex_lock(&mutex);
        windowBasic(data);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
}

void windowInit (void)
{
    initscr();
    raw();
    keypad(stdscr, TRUE);
    
    //noecho();
    //curs_set(0);
    echo();
    curs_set(1);

    refresh();
}

void windowBasic (data_t *data)
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
