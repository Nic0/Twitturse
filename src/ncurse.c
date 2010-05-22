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
#include "utils.h"

#define ERROR fprintf (stderr, \
        "%s:%d Error (%d) : %s\n", \
        __FILE__, __LINE__, \
        errno, strerror(errno))

extern pthread_mutex_t mutex;
/*  First application ncurses call, but a thread is done from here
 *  to display the main window with statuses (on refresh_status_window
 *  fuction) so here is basicly only the main navigation menu between tweets
 */
void *
ncurseApplication (void *arg)
{
    /*  TODO do a function of it */
    window_status_t *window_status = NULL;
    window_status = malloc (sizeof(window_status_t));
    window_status->items    = NULL;
    window_status->menu     = NULL;
    window_status->win      = NULL;
    window_status->data     = arg;
    window_status->refresh  = 0;

    pthread_t pidrefresh;
    if (pthread_create (&pidrefresh, NULL, refresh_status_window, window_status) != 0)
        ERROR;

    /*  Here the main menu to navigate between tweets, or choose an action
     */
    int c;
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
                case 32:        /* Espace */
                case 10: {      /* Enter */
                    window_status->refresh = -1;
                    ITEM *cur;
                    cur = current_item(window_status->menu);
                    status_t *status = item_userptr(cur);
                    detail_status_window (status);
                    window_status->refresh = 1;
                    break;
                }
                case 't':       /* t = write a tweet    */

                    window_status->refresh = -1;
                    send_tweet_window(window_status);
                    clear();
                    window_status->refresh = 1;
                    break;
                
                case 'r': {    /* r = retweet          */
                    ITEM *cur;
                    cur = current_item(window_status->menu);
                    status_t *status = NULL;
                    status = initStatus(status);
                    status = item_userptr(cur);
                    retweet_window(window_status, status);
                    clear();
                   /* menu_driver(window_status->menu, REQ_UP_ITEM);
                    menu_driver(window_status->menu, REQ_DOWN_ITEM);*/
                    break;
                }
                case 'c':   /*  c = clear   */
                    clear_statuses (window_status->data);
                    window_status->refresh = 1;
                    clear();
                    break;
                case 'h':   /*  h = help menu   */
                    window_status->refresh = -1;
                    help_window();
                    window_status->refresh = 1;
                    break;
                case 'f':
                    window_status->refresh = -1;
                    follow_window(window_status->data->config);
                    window_status->refresh = 1;
                    break;
                case 'u': {
                    window_status->refresh = -1;
                    ITEM *cur;
                    cur = current_item(window_status->menu);
                    status_t *status = NULL;
                    status = item_userptr(cur);
                    unfollow_window(window_status->data->config, status);
                    window_status->refresh = 1;
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

    /*  navigation menu of    ***  send_tweet_window  ***
     *  fill up the field, and confirmation to send the tweet
     */
	while((ch = wgetch(window_tweet)) != 27) {
        switch(ch) {

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
                    case 'y': {
                        char *tweet_send = NULL;
                        char *formbuff = NULL;
                        form_driver(my_form, REQ_VALIDATION);
                        formbuff = field_buffer(tweet[0], 0);
                        tweet_send = strndup(formbuff, 140);
                        post_status(tweet_send, window_status->data->config);
                        quit = 1;
                        window_status->refresh = 1;
                        break;
                    }
                    case 'n':
                        mvwprintw (window_tweet, 4, 10, "                           ");
                        form_driver(my_form, REQ_END_FIELD);
                        break;
                }
                break;
			default:
				/* If this is a normal character, it gets printed   */
				form_driver(my_form, ch);
				break;
		}
        wrefresh(window_tweet);
        if (quit == 1)
            break;
	}
    unpost_form(my_form);
    //free_form(my_form);
    free_field(tweet[0]);

    wborder(window_tweet, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wrefresh(window_tweet);
    delwin(window_tweet);
    curs_set(0);
}
/*  Init ncurses routine, and set colors as well,
 *  TODO: colors should be choose from the config file.
 */
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
/*  This display a small window with every detail of a tweet,
 *  like pseudo, (TODO time), full tweet's text...
 *  TODO: if press 'r' key, this should retweet it
 */
void
detail_status_window (status_t *display_status)
{
    
    WINDOW *local_win;

    int x = 0;
    int y = 0;
    getmaxyx(stdscr, y, x);

    local_win = newwin(8, 80, 2, (x/2)-40);

    FIELD *tweet[1];
    FORM  *my_form      = NULL;
    int ch;

    tweet[0] = new_field(3, 70, 3, 5, 0, 0);
    tweet[1] = NULL;
    set_field_buffer(tweet[0], 0, display_status->text);
    my_form = new_form(tweet);

    keypad(local_win, TRUE);
    set_form_win(my_form, local_win);
    set_form_sub(my_form, derwin(local_win, 6, 80, 0, 0));
    
    post_form(my_form);
    box(local_win, 0, 0);
    mvwprintw(local_win, 1, 5, "Name: %s", display_status->pseudo);
        
    wrefresh(local_win);

    getchar();

    wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wclear(local_win);
    wrefresh(local_win);
    delwin(local_win);
}

/*  Main routine for display the background menu of statuses
 *  Containing :
 *  name:            screen_pseudo
 *  description:     text (text of the tweet)
 *  userptr:         who is a struct of the curent status, with
 *  every information needed to display in a singular window if
 *  needed.
 *
 *  Certainly should be done better as I go througt to display
 *  the first time, and again with a while to refresh thir
 *  should be done only with the second part... TODO
 */

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
            //TODO set a HL if the pseudo of the tweet is yourself (config->login)
         /*       if(strcmp(cur_status->pseudo, data->config->login) == 0)
          *          set_item_opts(my_items[i], COLOR_PAIR(2));
          */          
                if (cur_status->prev != NULL)
                    cur_status = cur_status->prev;
        }
    pthread_mutex_unlock(&mutex);
	
    
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
        set_menu_mark(window_status->menu, " > ");

        refresh();
        
	    post_menu(window_status->menu);
        menu_driver(window_status->menu, REQ_LAST_ITEM);
    
	    wrefresh(window_status->win);
	    refresh();

    while (1) { 
        //pthread_mutex_lock(&mutex);
        if ((strcmp(window_status->data->statuses->first->id, first_status->id)) != 0 || 
             window_status->refresh == 1 &&
             window_status->refresh != -1) {

        window_status->refresh = 0;
		ITEM *cur_id;
		cur_id = current_item(window_status->menu);
		status_t *status_id = item_userptr(cur_id);

        menu_driver(window_status->menu, REQ_LAST_ITEM);
            int was_last=0;
            //We don't want move down if it's the last status anymore
        /*if (cur_id == current_item(window_status->menu))
            was_last = 1;*/


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

		        /*if(strcmp(cur_status->pseudo, data->config->login) == 0)
		         *  set_item_opts(my_items[i], COLOR_PAIR(2));
		         */  
		        if (loc_status->prev != NULL)
		            loc_status = loc_status->prev;
		}
	    
		window_status->menu = new_menu((ITEM **)window_status->items);
		window_status->win = newwin(row-2, col-2, 1, 1);
		keypad(window_status->win, TRUE);
		set_menu_win(window_status->menu, window_status->win);
		set_menu_sub(window_status->menu, derwin(window_status->win, 
		                                         row-2, col-4, 3, 1));
		set_menu_format(window_status->menu, row-2, 1);
		set_menu_fore(window_status->menu, COLOR_PAIR(2));
		set_menu_mark(window_status->menu, " > ");
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

void
retweet_window (window_status_t *window_status, status_t *retweet_status)
{
    char *buffer = NULL;
    buffer = strdup("RT @");
    buffer = cat_chaine (buffer, retweet_status->pseudo);
    buffer = cat_chaine (buffer, " ");
    buffer = cat_chaine (buffer, retweet_status->text);

    int chr = NULL;

    int y = 0;
    int x = 0;
    getmaxyx(window_status->win , y, x);
    mvwprintw(window_status->win, y-1, 10, "Re-Tweet this ? (y) (n)");
	chr = wgetch(window_status->win);
    switch(chr) {
        case 'y':
            post_retweet(buffer, window_status->data->config,
                         retweet_status);
            break;
        default:
            break;
    }
    window_status->refresh = 1;
}

void
help_window (void)
{
    WINDOW *local_win = NULL;
    int x = 0;
    int y = 0;
    getmaxyx(stdscr, y, x);
    local_win = newwin(y-2, x-2, 0, 0);
    box(local_win, 0, 0);


    mvwprintw(local_win, 1, 20, "Help Menu:");
    mvwprintw(local_win, 3, 10, "t : tweet");
    mvwprintw(local_win, 4, 10, "r : retweet");
    mvwprintw(local_win, 5, 10, "c : clear all tweets");
    mvwprintw(local_win, 6, 10, "f : follow someone");
    mvwprintw(local_win, 7, 10, "u : unfollow someone");
    mvwprintw(local_win, 8, 10, "h : show this help menu");
    mvwprintw(local_win, 9, 10, "space/enter : view the current tweet");

    wgetch(local_win);

    wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wclear(local_win);
    wrefresh(local_win);
    delwin(local_win);
}

void
follow_window(config_t *config)
{
    int x = 0;
    int y = 0;
    getmaxyx(stdscr, y, x);
    
    
    FIELD *follow[1];
    FORM  *my_form      = NULL;
    int ch;

    follow[0] = new_field(1, 20, 1, 5, 0, 0);
    follow[1] = NULL;
    set_field_back(follow[0], A_UNDERLINE);
    my_form = new_form(follow);

    WINDOW *local_win = NULL;
    local_win = newwin(4, 40, 2, (x/2)-20);
    keypad(local_win, TRUE);
    //wrefresh(window_tweet);


	/* Set main window and sub window */
    set_form_win(my_form, local_win);
    set_form_sub(my_form, derwin(local_win, 4, 40, 2, 2));
    
    post_form(my_form);
    box(local_win, 0, 0);
    mvwprintw(local_win, 0, 2, "Follow this fella ! (ESC to abord)");
    wrefresh(local_win);
    curs_set(1);
    form_driver(my_form, REQ_BEG_FIELD);
    int quit = 0;

    /*  navigation menu of    ***  send_tweet_window  ***
     *  fill up the field, and confirmation to send the tweet
     */
	while((ch = wgetch(local_win)) != 27) {
        switch(ch) {

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
                mvwprintw (local_win, 3, 10, "Follow him ? (y)es (n)o");
                int chr;
                chr = wgetch(local_win);
                switch(chr) {
                    case 'y': {
                        char *follow_send = NULL;
                        char *formbuff = NULL;
                        form_driver(my_form, REQ_VALIDATION);
                        formbuff = field_buffer(follow[0], 0);
                        follow_send = strndup(formbuff, 140);
                        post_follow(follow_send, config);
                        quit = 1;
                        break;
                    }
                    case 'n':
                        mvwprintw (local_win, 4, 10, "                           ");
                        form_driver(my_form, REQ_END_FIELD);
                        break;
                }
                break;
			default:
				/* If this is a normal character, it gets printed   */
				form_driver(my_form, ch);
				break;
		}
        wrefresh(local_win);
        if (quit == 1)
            break;
	}
    unpost_form(my_form);
    //free_form(my_form);
    free_field(follow[0]);

    curs_set(0);

    wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wclear(local_win);
    wrefresh(local_win);
    delwin(local_win);
}

void
unfollow_window (config_t *config, status_t *status)
{
    int x = 0;
    int y = 0;
    getmaxyx(stdscr, y, x);
    
    WINDOW *local_win = NULL;
    local_win = newwin(6, 40, 2, (x/2)-20);
    keypad(local_win, TRUE);
    box(local_win, 0, 0);
    mvwprintw(local_win, 0, 2, "UnFollow this guy ! (ESC to abord)");

    mvwprintw(local_win, 2, 5, "UnFollow %s ?", status->pseudo);
    mvwprintw(local_win, 3, 5, "(y)es, (n)o, (s)omeone else");
    wrefresh(local_win);

    int ch;
    if(ch = wgetch(local_win) == 'y') {
        post_unfollow(status->pseudo, config);
    } else if (ch = wgetch(local_win) == 's') {
        wclear(local_win);
        box(local_win, 0, 0);
        FIELD *follow[1];
        FORM  *my_form      = NULL;
        follow[0] = new_field(1, 20, 1, 5, 0, 0);
        follow[1] = NULL;
        set_field_back(follow[0], A_UNDERLINE);
        my_form = new_form(follow);
        //wrefresh(window_tweet);


	    /* Set main window and sub window */
        set_form_win(my_form, local_win);
        set_form_sub(my_form, derwin(local_win, 4, 40, 2, 2));
    
        post_form(my_form);
        wrefresh(local_win);
        curs_set(1);
        form_driver(my_form, REQ_BEG_FIELD);
        int quit = 0;

        /*  navigation menu of    ***  send_tweet_window  ***
        *  fill up the field, and confirmation to send the tweet
        */
	    while((ch = wgetch(local_win)) != 27) {
            switch(ch) {

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
                    mvwprintw (local_win, 3, 10, "Follow him ? (y)es (n)o");
                    int chr;
                    chr = wgetch(local_win);
                    switch(chr) {
                        case 'y': {
                            char *follow_send = NULL;
                            char *formbuff = NULL;
                            form_driver(my_form, REQ_VALIDATION);
                            formbuff = field_buffer(follow[0], 0);
                            follow_send = strndup(formbuff, 140);
                            post_unfollow(follow_send, config);
                            quit = 1;
                            break;
                        }   
                        case 'n':
                            mvwprintw (local_win, 4, 10, "                           ");
                            form_driver(my_form, REQ_END_FIELD);
                            break;
                    }
                    break;
			    default:
				    /* If this is a normal character, it gets printed   */
				    form_driver(my_form, ch);
				    break;
		    }
        wrefresh(local_win);
        if (quit == 1)
            break;
	    }
        unpost_form(my_form);
         //free_form(my_form);
        free_field(follow[0]);
    } 

    curs_set(0);

    wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wclear(local_win);
    wrefresh(local_win);
    delwin(local_win);
}
