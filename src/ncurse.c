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
#include <form.h>

#include "init.h"
#include "ncurse.h"

#define ERROR fprintf (stderr, \
        "%s:%d Error (%d) : %s\n", \
        __FILE__, __LINE__, \
        errno, strerror(errno))

//#define CTRLD   4

extern pthread_mutex_t mutex;

void *
ncurseApplication (void *arg)
{
    /*  TODO do a function of it */
    window_status_t *window_status;
    window_status = malloc (sizeof(window_status_t));

    window_status->items    = NULL;
    window_status->menu     = NULL;
    window_status->win      = NULL;
    window_status->data     = arg;
    window_status->refresh  = 0;
    int c;

    pthread_t pidrefresh;
    if (pthread_create (&pidrefresh, NULL, refresh_status_window, window_status) != 0)
        ERROR;


    //menu_driver(window_status->menu, REQ_LAST_ITEM);
	while((c = wgetch(window_status->win)) != 'q')
	{       switch(c)
	        {
                case KEY_DOWN:
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
                case 10: {  /* Enter */
                    ITEM *cur;
                    cur = current_item(window_status->menu);
                    status_t *status = item_userptr(cur);
                    mvwprintw(window_status->win, 0, 0, status->id);
                    break;
                }
                case 't': 
                    send_tweet_window(window_status);
                    clear();
                    break;
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

void
send_tweet_window (window_status_t *window_status)
{
    int x = 0;
    int y = 0;
    getmaxyx(stdscr, y, x);

    FIELD *tweet[1];
    FORM  *my_form      = NULL;
    int ch;

    tweet[0] = new_field(3, 70, 1, 5, 0, 0);
    tweet[1] = NULL;
    set_field_back(tweet[0], A_UNDERLINE);
    my_form = new_form(tweet);

    WINDOW *window_tweet = NULL;
    window_tweet = newwin(6, 80, 2, (x/2)-40);
    keypad(window_tweet, TRUE);
    //wrefresh(window_tweet);


	/* Set main window and sub window */
    set_form_win(my_form, window_tweet);
    set_form_sub(my_form, derwin(window_tweet, 6, 80, 2, 2));
    
    post_form(my_form);
    box(window_tweet, 0, 0);
    mvwprintw(window_tweet, 0, 2, "Tweet this ! (ESC to abord)");
    wrefresh(window_tweet);
    curs_set(1);
    form_driver(my_form, REQ_BEG_FIELD);
    int quit = 0;
	while((ch = wgetch(window_tweet)) != 27) {
    switch(ch)
		{
            case KEY_LEFT:
                form_driver(my_form, REQ_PREV_CHAR);
                break;
            case KEY_RIGHT:
                form_driver(my_form, REQ_NEXT_CHAR);
                break;
			case 127: /* Backspace */
				form_driver(my_form, REQ_PREV_CHAR);
				form_driver(my_form, REQ_DEL_CHAR);
				break;
            case 330: /* Suppr. */
                form_driver(my_form, REQ_DEL_CHAR);
                break;
            case 10: /* Enter */
                mvwprintw (window_tweet, 4, 10, "Tweet this ? (y)es (n)o");
                int chr;
                chr = wgetch(window_tweet);
                switch(chr) {
                    case 'y':
                        mvwprintw (window_tweet, 4, 10, "The tweet will be sent soon ");
                        char *tweet_send = NULL;
                        char *formbuff = NULL;
                        form_driver(my_form, REQ_VALIDATION);
                        formbuff = field_buffer(tweet[0], 0);
                        tweet_send = strndup(formbuff, 140);
                        post_status(tweet_send);
                        quit = 1;
                        window_status->refresh = 1;
                        break;
                    case 'n':
                        mvwprintw (window_tweet, 4, 10, "                           ");
                        form_driver(my_form, REQ_END_FIELD);
                        break;
                }
                break;
			default:
				/* If this is a normal character, it gets */
				/* Printed				  */	
				form_driver(my_form, ch);
				break;
		}
  /*      char *formbuff = NULL;
        form_driver(my_form, REQ_VALIDATION);
        formbuff = field_buffer(tweet[0], 0);
        mvwprintw (window_tweet, 4, 10, "'%s'", formbuff);*/
        wrefresh(window_tweet);
        if (quit == 1)
            break;
	}
	/* Un post form and free the memory */
    unpost_form(my_form);
    //free_form(my_form);
    free_field(tweet[0]);

    wborder(window_tweet, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wrefresh(window_tweet);
    delwin(window_tweet);
    curs_set(0);
}

void 
windowInit (void)
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
    window_status_t *window_status = NULL;
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
    //pthread_mutex_lock(&mutex);
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

    //pthread_mutex_unlock(&mutex);
	     //Unpost and free all the memory taken up 
        /*unpost_menu(window_status->menu);
        free_menu(window_status->menu);
        for(i = 0; i < n_choices; ++i)
                free_item(window_status->items[i]);*/
    while (1) { 
        //pthread_mutex_lock(&mutex);
        if ((strcmp(window_status->data->statuses->first->id, first_status->id)) != 0 || 
             window_status->refresh == 1) {

        window_status->refresh = 0;
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
    sleep(1);
    }
}
