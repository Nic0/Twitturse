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
#include <menu.h>

#include "init.h"
#include "ncurse.h"

#define ERROR fprintf (stderr, \
        "%s:%d Error (%d) : %s\n", \
        __FILE__, __LINE__, \
        errno, strerror(errno))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD   4

char *choices[] = {
        "Choice 1",
        "Choice 2",
        "Choice 3",
        "Choice 4",
        "Choice 5",
        "Choice 6",
        "Choice 7",
        "Choice 8",
        "Choice 9",
        "Choice 10",
        "Exit",
        (char *)NULL,
};


extern pthread_mutex_t mutex;

void *
ncurseApplication (void *arg)
{
    sleep(7);
    data_t *data = arg;

    ITEM **my_items = NULL;
    int c = 0;
    MENU * my_menu = NULL;
        WINDOW *my_menu_win = NULL;
        int n_choices = 0;
        int i = 0;
    
    windowInit();

        int row = 0;
        int col = 0;
        getmaxyx(stdscr,row,col);

	/* Create items */
        n_choices = ARRAY_SIZE(choices);
        my_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
        
        status_t * cur_status = NULL;
        cur_status = initStatus (cur_status);
        *cur_status = *data->statuses->last;

        for(i = 0; i < n_choices; ++i) {
                my_items[i] = new_item(cur_status->pseudo, cur_status->text);
                cur_status = cur_status->prev;
        }

	/* Crate menu */
	my_menu = new_menu((ITEM **)my_items);

	/* Create the window to be associated with the menu */
        my_menu_win = newwin(row-2, col-2, 1, 1);
        keypad(my_menu_win, TRUE);
     
	/* Set main window and sub window */
        set_menu_win(my_menu, my_menu_win);
        set_menu_sub(my_menu, derwin(my_menu_win, row-4, col-4, 3, 1));
	set_menu_format(my_menu, row-4, 1);
			
	/* Set menu mark to the string " * " */
        set_menu_mark(my_menu, " > ");

	/* Print a border around the main window and print a title */
        //box(my_menu_win, 0, 0);
	print_in_middle(my_menu_win, 1, 0, 40, "Tweets", COLOR_PAIR(1));
/*	mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
	mvwhline(my_menu_win, 2, 1, ACS_HLINE, 50);
	mvwaddch(my_menu_win, 2, 51, ACS_RTEE);*/
        
	/* Post the menu */
	post_menu(my_menu);
	wrefresh(my_menu_win);
/*	
	attron(COLOR_PAIR(2));
	mvprintw(LINES - 2, 0, "Use PageUp and PageDown to scoll down or up a page of items");
	mvprintw(LINES - 1, 0, "Arrow Keys to navigate (F1 to Exit)");
	attroff(COLOR_PAIR(2));*/
	refresh();

	while((c = wgetch(my_menu_win)) != KEY_F(1))
	{       switch(c)
	        {	case KEY_DOWN:
				menu_driver(my_menu, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
				menu_driver(my_menu, REQ_UP_ITEM);
				break;
			case KEY_NPAGE:
				menu_driver(my_menu, REQ_SCR_DPAGE);
				break;
			case KEY_PPAGE:
				menu_driver(my_menu, REQ_SCR_UPAGE);
				break;
		}
                wrefresh(my_menu_win);
	}	

	/* Unpost and free all the memory taken up */
        unpost_menu(my_menu);
        free_menu(my_menu);
        for(i = 0; i < n_choices; ++i)
                free_item(my_items[i]);
	endwin();
}

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color)
{	

    int length, x, y;

	float temp;

	if(win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if(startx != 0)
		x = startx;
	if(starty != 0)
		y = starty;
	if(width == 0)
		width = 80;

	length = strlen(string);
	temp = (width - length)/ 2;
	x = startx + (int)temp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);
	refresh();
    
}














void windowInit (void)
{
    initscr();
    keypad(stdscr, TRUE);
    start_color();
    cbreak();
    noecho();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    //curs_set(1);
}

WINDOW *
update_statusWindow (WINDOW *local_win, int col, int row, data_t *data)
{
    int rowstd = 0;
    int colstd = 0;
    getmaxyx(stdscr, rowstd, colstd);
    //if (rowstd != row || colstd != col) {
    //    local_win = create_statusWindow(colstd, rowstd, data);
   // } else {

    status_t *cur_status = NULL;
    if ((cur_status = initStatus(cur_status)) == NULL)
        ERROR;
    if (data->statuses->first != NULL) {
        *cur_status = *data->statuses->first;

        int i;
        for (i = row-2; i>0; i--) {

        if (strlen (cur_status->text) > col-20)
            i--;

            wchar_t *buff1 = NULL;
            wchar_t *buff2 = NULL;
            buff1 = strdup(cur_status->pseudo);
            buff2 = strdup(cur_status->text);

            mvwprintw (local_win, i, 2, buff1);
            mvwprintw (local_win, i, 18, buff2);

            if(cur_status->next != NULL)
                *cur_status = *cur_status->next;
            else
                break;
        }
    }
    wrefresh (local_win);
    return local_win;
    //}
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
            mvwprintw (local_win, i, 18, cur_status->text);

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
