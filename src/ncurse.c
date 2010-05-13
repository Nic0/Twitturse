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

#define CTRLD   4

extern pthread_mutex_t mutex;

void *
ncurseApplication (void *arg)
{
    window_status_t *window_status;
    window_status = malloc (sizeof(window_status_t));

    window_status->items    = NULL;
    window_status->menu     = NULL;
    window_status->win      = NULL;
    window_status->data     = arg;
    int c;

    pthread_t pidrefresh;
    if (pthread_create (&pidrefresh, NULL, refresh_status_window, window_status) != 0)
        ERROR;


    //menu_driver(window_status->menu, REQ_LAST_ITEM);
	while((c = wgetch(window_status->win)) != 'q')
	{       switch(c)
	        {	case KEY_DOWN:
                pthread_mutex_lock(&mutex);
				    menu_driver(window_status->menu, REQ_DOWN_ITEM);
                pthread_mutex_unlock(&mutex);
				break;
			case KEY_UP:
                pthread_mutex_lock(&mutex);
				    menu_driver(window_status->menu, REQ_UP_ITEM);
                pthread_mutex_unlock(&mutex);
				break;
			case KEY_NPAGE:
                pthread_mutex_lock(&mutex);
				    menu_driver(window_status->menu, REQ_SCR_DPAGE);
                pthread_mutex_unlock(&mutex);
				break;
			case KEY_PPAGE:
                pthread_mutex_lock(&mutex);
				    menu_driver(window_status->menu, REQ_SCR_UPAGE);
                pthread_mutex_unlock(&mutex);
				break;
            case 10: {/* Enter */
                ITEM *cur;
                cur = current_item(window_status->menu);
                status_t *status = item_userptr(cur);
                mvwprintw(window_status->win, 0, 0, status->id);
                break;
            }


		}
                wrefresh(window_status->win);
	}	

	/* Unpost and free all the memory taken up */
        unpost_menu(window_status->menu);
        free_menu(window_status->menu);
        int i;
        for(i = 0; i < window_status->data->statuses->count; ++i)
                free_item(window_status->items[i]);
	endwin();
    pthread_exit(NULL);
}

void windowInit (void)
{
    initscr();
    keypad(stdscr, TRUE);
    start_color();
    use_default_colors();
    cbreak();
    noecho();
    init_pair(1, COLOR_YELLOW, -1);
    init_pair(2, COLOR_CYAN, -1);
    curs_set(0);
}

void *
refresh_status_window (void *arg)
{
    window_status_t *window_status;
    window_status = arg;
    status_t *cur_status    = NULL;
    status_t *first_status   = NULL;
    if ((cur_status = initStatus (cur_status)) == NULL)
        ERROR;
    if ((first_status = initStatus (first_status)) == NULL)
        ERROR;

    int n_choices   = 0;
    int row         = 0;
    int col         = 0;
    
    windowInit();
    getmaxyx(stdscr,row,col);

	/* Create items */

        while (window_status->data->statuses->count == 0) 
            sleep(1);

    pthread_mutex_lock(&mutex);
        n_choices = window_status->data->statuses->count;
        window_status->items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
        
        *cur_status = *window_status->data->statuses->last;
        *first_status = *window_status->data->statuses->first;

        
        int i;
        for(i = 0; i < n_choices; ++i) {
                window_status->items[i] = new_item(cur_status->pseudo,
                                                   cur_status->text);
                set_item_userptr(window_status->items[i], cur_status);

         /*       if(strcmp(cur_status->pseudo, data->config->login) == 0)
          *          set_item_opts(my_items[i], COLOR_PAIR(2));
          */          
                if (cur_status->prev != NULL)
                    cur_status = cur_status->prev;
        }
    pthread_mutex_unlock(&mutex);
	
    
    /* Crate menu */
    pthread_mutex_lock(&mutex);
	window_status->menu = new_menu((ITEM **)window_status->items);

	/* Create the window to be associated with the menu */
        window_status->win = newwin(row-2, col-2, 1, 1);
        keypad(window_status->win, TRUE);
     
	/* Set main window and sub window */
        set_menu_win(window_status->menu, window_status->win);
        set_menu_sub(window_status->menu, derwin(window_status->win, 
                                                 row-2, col-4, 3, 1));
	    set_menu_format(window_status->menu, row-2, 1);
		set_menu_fore(window_status->menu, COLOR_PAIR(2));

	/* Set menu mark to the string " * " */
        set_menu_mark(window_status->menu, " > ");

    refresh();
        
	/* Post the menu */
	post_menu(window_status->menu);
    menu_driver(window_status->menu, REQ_LAST_ITEM);
    
	wrefresh(window_status->win);
	refresh();

    pthread_mutex_unlock(&mutex);
	     //Unpost and free all the memory taken up 
        /*unpost_menu(window_status->menu);
        free_menu(window_status->menu);
        for(i = 0; i < n_choices; ++i)
                free_item(window_status->items[i]);*/
    while (1) { 
        //pthread_mutex_lock(&mutex);
        if ((strcmp(window_status->data->statuses->first->id, first_status->id)) != 0) {
            /*status_t *loc_status;
            loc_status = initStatus(loc_status);
            *loc_status = *window_status->data->statuses->last;
            
            while (1) {
                loc_status = loc_status->prev;
                if ((strcmp(loc_status->id, first_status->id)) == 0 || 
                    loc_status->prev == NULL)
                    break;
            }
            mvwprintw(window_status->win, 0, 0, loc_status->id);
            mvwprintw(window_status->win, 1, 0, first_status->id);
            
            wrefresh(window_status->win);
            

            while (1){
                if (loc_status->prev != NULL)
                    loc_status = loc_status->prev;
                i++;
                window_status->items[i] = new_item(loc_status->pseudo, 
                                                   loc_status->text);
            mvwprintw(window_status->win, 2, 0, loc_status->text);
            wrefresh(window_status->win);
                if (loc_status->prev == NULL)
                    break;
            }
        //pthread_mutex_unlock(&mutex);
            mvwprintw(window_status->win, 3, 0, "pwet3");
	        window_status->menu = new_menu((ITEM **)window_status->items);

            wrefresh(window_status->win);
            unpost_menu(window_status->menu);
            post_menu(window_status->menu);
            wrefresh(window_status->win);
            doupdate();*/

	/* Unpost and free all the memory taken up */

                ITEM *cur_id;
                cur_id = current_item(window_status->menu);
                status_t *status_id = item_userptr(cur_id);


        unpost_menu(window_status->menu);
        free_menu(window_status->menu);
        
        for(i = 0; i < window_status->data->statuses->count; ++i)
                free_item(window_status->items[i]);


        //pthread_mutex_lock(&mutex);
            *first_status = *window_status->data->statuses->first;

        
        n_choices = window_status->data->statuses->count;
        window_status->items = (ITEM **)calloc(n_choices+50, sizeof(ITEM *));
        status_t *loc_status = NULL;
        loc_status = initStatus(loc_status);

        *loc_status = *window_status->data->statuses->last;
        *first_status = *window_status->data->statuses->first;

        
        int i;
        for(i = 0; i < n_choices; ++i) {
                window_status->items[i] = new_item(loc_status->pseudo,
                                                   loc_status->text);
                set_item_userptr(window_status->items[i], loc_status);

         /*       if(strcmp(cur_status->pseudo, data->config->login) == 0)
          *          set_item_opts(my_items[i], COLOR_PAIR(2));
          */          
                if (loc_status->prev != NULL)
                    loc_status = loc_status->prev;
        }
    //pthread_mutex_unlock(&mutex);
	
    
    /* Crate menu */
   // pthread_mutex_lock(&mutex);
	window_status->menu = new_menu((ITEM **)window_status->items);

	/* Create the window to be associated with the menu */
        window_status->win = newwin(row-2, col-2, 1, 1);
        keypad(window_status->win, TRUE);
     
	/* Set main window and sub window */
        set_menu_win(window_status->menu, window_status->win);
        set_menu_sub(window_status->menu, derwin(window_status->win, 
                                                 row-2, col-4, 3, 1));
	    set_menu_format(window_status->menu, row-2, 1);
		set_menu_fore(window_status->menu, COLOR_PAIR(2));

	/* Set menu mark to the string " * " */
        set_menu_mark(window_status->menu, " > ");
    //pthread_mutex_unlock(&mutex);
    refresh();
        
	/* Post the menu */
	post_menu(window_status->menu);
    menu_driver(window_status->menu, REQ_LAST_ITEM);

    while (1) {
        ITEM *cur;
        cur = current_item(window_status->menu);
        status_t *status = item_userptr(cur);
        if ((strcmp (status->id, status_id->id)) != 0)
            menu_driver(window_status->menu, REQ_UP_ITEM);
        else
            break;
    }

    
	wrefresh(window_status->win);
	refresh();

        } 

        sleep(5);
    }



}
