/********************************************************************************/
/*										*/
/* Terminal Bomber gmae engine							*/
/* v 0.2 date 13/12/2009							*/
/*										*/
/* Copyright 2008, 2009 Lev Meirovitch						*/
/* <LVMTime@gmail.com>								*/
/*										*/
/* This file is part of "Terminal Bomber".					*/
/*										*/
/* "Terminal Bomber" is free software: you can redistribute it and/or modify	*/
/* it under the terms of the GNU General Public License as published by		*/
/* the Free Software Foundation, either version 3 of the License, or		*/
/* (at your option) any later version.						*/
/*										*/
/* "Terminal Bomber" is distributed in the hope that it will be useful,		*/
/* but WITHOUT ANY WARRANTY; without even the implied warranty of		*/
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the		*/
/* GNU General Public License for more details.					*/
/*										*/
/* You should have received a copy of the GNU General Public License		*/
/* along with Foobar.  If not, see <http://www.gnu.org/licenses/>. 		*/
/*										*/
/********************************************************************************/
 
 
#include <unistd.h>
#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "term_bomber.h"
#include "game_engine.h"
#include "best_times.h"


/* game difficulty array: */
const field_size game_difficulty[GAME_DIFFICULTY_MAX] = {{5, 10, 5},   /* easy */
							{10, 20, 30},  /* medium */
							{10, 38, 50}}; /* hard */

/************************* game display functions *****************************/

void paint_field(WINDOW* field_win, const char *field, int height, int width, int bombs)
{
	/*  paints the game field
	    parameters:
	    feild_win - window to paint in
	    field - array of field squares (game map)
	    height, width - number of squares in filed  */

	int	x, y;
	chtype	cur_char[2]; /* we use this small array to paint two charecters - regular line and intersection in intermidiate order */
	int	field_pos;
	

	/* check parameters for validity: */
	if ((NULL == field_win) || (NULL == field) || (height < 1) || (width < 1) || (height > (LINES / 2)) || (width > (COLS / 2))) return;

	/* clear the window: */
	wattrset(field_win, COLOR_PAIR(COLOR_PAIR_NORMAL) | A_BOLD);
	wclear(field_win);

	/* draw grid corners: */
	
	mvwaddch(field_win, 0, 0, ACS_ULCORNER);
	mvwaddch(field_win, height * 2, 0, ACS_LLCORNER);
	mvwaddch(field_win, 0, width * 2, ACS_URCORNER);
	mvwaddch(field_win, height * 2, width * 2, ACS_LRCORNER);
	

	/* draw top line of filed border: */
	cur_char[0] = ACS_TTEE; cur_char[1] = ACS_HLINE; /* set horizontal and down pointing T as border chars */
	for (x = 1; x < (width * 2); x++)
		mvwaddch(field_win, 0, x, cur_char[x % 2]);

	/* draw bottom line of filed border: */
	cur_char[0] = ACS_BTEE; /* set up pointing T as border chars */
	for (x = 1; x < (width * 2); x++)
		mvwaddch(field_win, height * 2, x, cur_char[x % 2]);

	/* draw left line of filed border: */
	cur_char[0] = ACS_LTEE; cur_char[1] = ACS_VLINE; /* set vertical and left pointing T as border chars */
	for (y = 1; y < (height * 2); y++)
		mvwaddch(field_win, y, 0, cur_char[y % 2]);

	/* draw right line of filed border: */
	cur_char[0] = ACS_RTEE; /* set right pointing T as border chars */
	for (y = 1; y < (height * 2); y++)
		mvwaddch(field_win, y, width * 2, cur_char[y % 2]);

	/* draw side box borders */
	for (y = 1; y < (height * 2); y += 2)
	{
		for (x = 2; x < (width * 2); x += 2)
			mvwaddch(field_win, y, x, ACS_VLINE);
	}

	/* draw horizontal lines */
	cur_char[0] = ACS_PLUS;	cur_char[1] = ACS_HLINE;
	for (y = 2; y < (height * 2); y += 2)
	{
		for (x = 1; x < (width * 2); x++)
			mvwaddch(field_win, y, x, cur_char[x % 2]);
	}

	/* paint field data: */
	for (y = 1; y < (height * 2); y += 2)
	{
		for (x = 1; x < (width * 2); x += 2)
		{
			field_pos = ((((y - 1) / 2) + 1) * (width + 2)) + ((x - 1) / 2) + 1;

			if (field[field_pos] < 0) /* current square is covered */
			{
				wattrset(field_win, COLOR_PAIR(COLOR_PAIR_NORMAL));
				mvwaddch(field_win, y, x, ACS_CKBOARD);
			}
			else /* uncovered squares */
			{
				switch (field[field_pos]) /* paint differently depending on content */
				{
				case 0: /* open square with no bombs around, nothing painted */
					break;

				case 1: /* paint number of bombs in square with appropriate color */
					wattrset(field_win, COLOR_PAIR(COLOR_PAIR_BLUE) | A_BOLD); /* blue digit */
					mvwaddch(field_win, y, x, '1');
					break;

				case 2:
					wattrset(field_win, COLOR_PAIR(COLOR_PAIR_GREEN) | A_BOLD); /* green digit */
					mvwaddch(field_win, y, x, '2');
					break;

				case 3:
					wattrset(field_win, COLOR_PAIR(COLOR_PAIR_RED) | A_BOLD); /* red digit */
					mvwaddch(field_win, y, x, '3');
					break;

				case 4:
					wattrset(field_win, COLOR_PAIR(COLOR_PAIR_YELLOW) | A_BOLD); /* dark-blue digit */
					mvwaddch(field_win, y, x, '4');
					break;

				case 5:
					wattrset(field_win, COLOR_PAIR(COLOR_PAIR_RED)); /* dark-red digit (bordou?) */
					mvwaddch(field_win, y, x, '5');
					break;

				case 6:
					wattrset(field_win, COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD); /* cyan digit (turcoise?) */
					mvwaddch(field_win, y, x, '6');
					break;

				case 7:
					wattrset(field_win, COLOR_PAIR(COLOR_PAIR_PINK) | A_BOLD); /* pink digit */
					mvwaddch(field_win, y, x, '7');
					break;

				case 8:
					wattrset(field_win, COLOR_PAIR(COLOR_PAIR_NORMAL) | A_BOLD); /* white digit */
					mvwaddch(field_win, y, x, '8');
					break;

				case 9:
					wattrset(field_win, COLOR_PAIR(COLOR_PAIR_YELLOW)); /* dark yellow */
					mvwaddch(field_win, y, x, '9');
					break;
	
				case FIELD_SQUARE_EXPLOAD: /* exploaded bomb */
					wattrset(field_win, COLOR_PAIR(COLOR_PAIR_EXPLOAD) | A_BOLD); /* red on yello explosion */
					mvwaddch(field_win, y, x, ACS_DIAMOND);
					break;

				case FIELD_SQUARE_FLAG: /* flagged square */
				case FIELD_SQUARE_FLAGBOMB:
					wattrset(field_win, COLOR_PAIR(COLOR_PAIR_FLAG) | A_BOLD); /* red on blue flag */
					mvwaddch(field_win, y, x, '>');
					break;
				} /* end switch filed[] */
			} /* end else field[] < 0 */
		} /* end for x */
	} /* end for y */
	
	wrefresh(field_win);
	
	/* repaint number of bombs left */
	
	attrset(COLOR_PAIR(COLOR_PAIR_YELLOW) | A_BOLD);
	mvprintw(LINES - 2, 0, GAME_BOMBS_LEFT, bombs);
	
	refresh();
}

/******************************************************************************/

/************************ Game timer functions ********************************/

pthread_t start_timer_thread(WINDOW* game_wnd, timer_thread_params *timer_params)
{
	/* Creates the timer window and starts the thread_create
	   in detached mode so it can be canceled at any time.
	   parameters:
	   start - start time (in milliseconds)
	   thread_params - structure to be passed to the thread
			   containing window pointer and start time
	   return:
	   thread identifier of the timer thread. */
	  
	pthread_attr_t	thread_attr;
	pthread_t	timer_thread;
	

	/* initialize the parameter structure */
	memset(timer_params, 0, sizeof(timer_thread_params));
	timer_params->timer_wnd = newwin(1, 5, LINES - 2, TIMER_WND_X);	/* create timer window */
	
	timer_params->game_wnd = game_wnd;
	timer_params->game_time = 0;
	
	if (NULL == timer_params->timer_wnd) return 0; /* make sure we have a window to print in */
	
	/* create timer control mutex */
	if (0 != pthread_mutex_init(&timer_params->control_mutex, NULL))
	{
		delwin(timer_params->timer_wnd);
		timer_params->timer_wnd = NULL;
		return 0;
	}
	
	/* create paint control mutex */
	if (0 != pthread_mutex_init(&timer_params->paint_mutex, NULL))
	{
		pthread_mutex_destroy(&timer_params->control_mutex);
		delwin(timer_params->timer_wnd);
		timer_params->timer_wnd = NULL;
		return 0;
	}
	
	/* create timer thread */
	if (0 != pthread_attr_init(&thread_attr))
	{
		delwin(timer_params->timer_wnd);
		timer_params->timer_wnd = NULL;
		return 0;
	}
	
	if (0 != pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED))
	{
		(void)pthread_attr_destroy(&thread_attr);
		pthread_mutex_destroy(&timer_params->control_mutex);
		pthread_mutex_destroy(&timer_params->paint_mutex);
		delwin(timer_params->timer_wnd);
		timer_params->timer_wnd = NULL;		
		return 0;	
	}
	
	if (0 != pthread_create(&timer_thread, &thread_attr, timer_thread_func, (void *)timer_params))
	{
		timer_thread = 0;
		pthread_mutex_destroy(&timer_params->control_mutex);
		pthread_mutex_destroy(&timer_params->paint_mutex);	
		delwin(timer_params->timer_wnd);
		timer_params->timer_wnd = NULL;		
	}
	
	(void)pthread_attr_destroy(&thread_attr);
	
	return timer_thread;
}

void stop_timer_thread(pthread_t timer_thread, timer_thread_params *timer_params)
{
	/* Stops the timer thread and destroys the timer window
	   parameters:
	   timer_thread - thread identifyre to stop
	   timer_params - thread parameters structure containing window
			  to be destroied */
	
	
	if (timer_thread) pthread_cancel(timer_thread);
	if (timer_thread) pthread_mutex_destroy(&timer_params->control_mutex);
	if (timer_thread) pthread_mutex_destroy(&timer_params->paint_mutex);
	if (timer_params->timer_wnd) delwin(timer_params->timer_wnd);
}

void *timer_thread_func(void* pParam)
{
	/*********************************************************/
	/* this function runs as a thread in the backgound       */
	/* and constantly updates the timer shoing number of     */
	/* second since game began				 */
	/*							 */
	/* parmaters:						 */
	/* param - pointer to thread_params structure containing */
	/*         initialization data				 */
	/*********************************************************/
	
	timer_thread_params *timer_params = (timer_thread_params *)pParam;
	
	
	/* set thread state so it can be canceled immidiatly */
	(void)pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	(void)pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	/* main printing loop */
	do {
		pthread_mutex_lock(&timer_params->paint_mutex);

		wattrset(timer_params->timer_wnd, COLOR_PAIR(COLOR_PAIR_YELLOW) | A_BOLD);
		mvwprintw(timer_params->timer_wnd, 0, 0, "%5d", timer_params->game_time);
		wrefresh(timer_params->timer_wnd);
		wrefresh(timer_params->game_wnd);

		pthread_mutex_unlock(&timer_params->paint_mutex);

		sleep(1);
		
		pthread_mutex_lock(&timer_params->control_mutex); /* make sure counter is not paused */		
		timer_params->game_time++; /* second has passed, increment the timer */
		pthread_mutex_unlock(&timer_params->control_mutex); /* release control to allow timer pause */
	} while (1);
	
	pthread_exit(0); /* this is here just in case */
}

/******************************************************************************/

/**************************** main game loop **********************************/

void play_game(int difficulty)
{
	/*  this is the main game loop
	    parameters:
	    difficulty - game difficulty  */

	WINDOW 		*game_win = NULL;
	int		field_width, field_height, num_of_bombs;
	char		*field = NULL;
	int		cur_pos_x, cur_pos_y, key;
	game_result	game_status = game_not_over;
	int		first_pass, repaint_field;

	
	/* thread variables */
	timer_thread_params*     timer_params;
	pthread_t	        timer_thread;


	if ((difficulty < 0) || (difficulty >= GAME_DIFFICULTY_MAX)) return; /* unknown difficulty setting */

	/* create window for the game: */
	field_width = game_difficulty[difficulty].width; /* window size depends on game difficulty */
	field_height = game_difficulty[difficulty].height;
	num_of_bombs = game_difficulty[difficulty].bombs;

	game_win = newwin((field_height * 2) + 1, (field_width * 2) + 1, (LINES - ((field_height * 2) + 1)) / 2,
										(COLS - ((field_width * 2) + 1)) / 2);
	if (NULL == game_win) return;

	/* create game field: */
	field = initialize_field(difficulty);
	if (NULL == field)
	{
		delwin(game_win);
		return;
	}

	/* print status line at bottom of screen: */
	move(LINES - 1, 0);
	clrtoeol();
	attrset(COLOR_PAIR(COLOR_PAIR_NORMAL) | A_BOLD);
	mvprintw(LINES - 1, 0, GAME_INSTRUCTIONS);
	
	/* print game timer text */
	attrset(COLOR_PAIR(COLOR_PAIR_YELLOW) | A_BOLD);
	mvprintw(LINES - 2, COLS - strlen(GAME_TIMER_TEXT), GAME_TIMER_TEXT);	
	refresh();

	cur_pos_x = 1; cur_pos_y = 1;
	
	/* mark field to be painted: */
	repaint_field = 1;

	timer_params = malloc(sizeof(timer_thread_params));
	if (NULL == timer_params)
	{
		delwin(game_win);
		return;
	}

	timer_thread = start_timer_thread(game_win, timer_params); /* start the game timer */
	
	do /* main game loop */
	{
		if (repaint_field > 0)
		{
			pthread_mutex_lock(&timer_params->paint_mutex);
			paint_field(game_win, field, field_height, field_width, num_of_bombs); /* update the displayed field */
			pthread_mutex_unlock(&timer_params->paint_mutex);
			repaint_field = 0; /* avoid unnecissery refresh */
		}

		pthread_mutex_lock(&timer_params->paint_mutex);
		wmove(game_win, ((cur_pos_y - 1) * 2) + 1 , ((cur_pos_x - 1) * 2) + 1);
		wrefresh(game_win);
		pthread_mutex_unlock(&timer_params->paint_mutex);
		key = getch(); /* get user action */

		switch (key) /* act upon user input */
		{
		case KEY_UP: /* move up */
			first_pass = 1;
			do /* skip open squares */
			{
				cur_pos_y--;
				if (cur_pos_y < 1)
				{
					cur_pos_y = field_height;
					if (first_pass) first_pass = 0;
					else
					{ /* if we scanned this column and there is no place for the cursor skip to previos column */
						cur_pos_x--;
						if (cur_pos_x < 1) cur_pos_x = field_width;
					}
				}
			} while ((field[(cur_pos_y * (field_width + 2)) + cur_pos_x] >= 0) &&
						(field[(cur_pos_y * (field_width + 2)) + cur_pos_x] < FIELD_SQUARE_BOMB));
			break;

		case KEY_DOWN: /* move down */
			first_pass = 1;
			do /* skip open squares */
			{
				cur_pos_y++;
				if (cur_pos_y > field_height)
				{
					cur_pos_y = 1;
					if (first_pass) first_pass = 0;
					else
					{ /* if we scanned this column and there is no place for the cursor skip to previos column */
						cur_pos_x++;
						if (cur_pos_x > field_width) cur_pos_x = 1;
					}
				}
			} while ((field[(cur_pos_y * (field_width + 2)) + cur_pos_x] >= 0) &&
						(field[(cur_pos_y * (field_width + 2)) + cur_pos_x] < FIELD_SQUARE_BOMB));
			break;

		case KEY_LEFT: /* move up */
			first_pass = 1;
			do /* skip open squares */
			{
				cur_pos_x--;
				if (cur_pos_x < 1)
				{
					cur_pos_x = field_width;
					if (first_pass) first_pass = 0;
					else
					{ /* if we scanned this row and there is no place for the cursor skip to previos row */
						cur_pos_y--;
						if (cur_pos_y < 1) cur_pos_y = field_height;
					}
				}
			} while ((field[(cur_pos_y * (field_width + 2)) + cur_pos_x] >= 0) &&
						(field[(cur_pos_y * (field_width + 2)) + cur_pos_x] < FIELD_SQUARE_BOMB));
			break;

		case KEY_RIGHT: /* move down */
			first_pass = 1;
			do /* skip open squares */
			{
				cur_pos_x++;
				if (cur_pos_x > field_width)
				{
					cur_pos_x = 1;
					if (first_pass) first_pass = 0;
					else
					{ /* if we scanned this row and there is no place for the cursor skip to next row */
						cur_pos_y++;
						if (cur_pos_y > field_height) cur_pos_y = 1;
					}
				}
			} while ((field[(cur_pos_y * (field_width + 2)) + cur_pos_x] >= 0) &&
						(field[(cur_pos_y * (field_width + 2)) + cur_pos_x] < FIELD_SQUARE_BOMB));
			break;

		case 'F':
		case 'f':
		case '\t': /* flag the square */
			if (flag_square(field, field_height, field_width, cur_pos_y, cur_pos_x, &num_of_bombs) > 0)
				repaint_field = 1; /* field changed, repaint */
			break;

		case 'U': /* uncover current square */
		case 'u':
		case ' ':
			game_status = uncover_square(field, field_height, field_width, cur_pos_y, cur_pos_x);
			switch (game_status)
			{
			case game_won: /* hurray you won! */
				stop_timer_thread(timer_thread, timer_params);
				move(LINES - 1, 0);
				attrset(COLOR_PAIR(COLOR_PAIR_YELLOW) | A_BOLD);
				clrtoeol();
				mvprintw(LINES - 1, 0, GAME_WON, timer_params->game_time); /* wining message and game duration */
				paint_field(game_win, field, field_height, field_width, 0); /* update the displayed field */
				refresh();
				
				/* pause if no dialog is displayed */
				if (0 == add_game_time(timer_params->game_time, difficulty)) getch();
				
				break;

			case game_lost: /* you found a bomb and blew your self up... */
				stop_timer_thread(timer_thread, timer_params);
				move(LINES - 1, 0);
				attrset(COLOR_PAIR(COLOR_PAIR_RED) | A_BOLD);
				clrtoeol();
				mvprintw(LINES - 1, 0, GAME_LOST);
				paint_field(game_win, field, field_height, field_width, num_of_bombs); /* update the displayed field */
				move(LINES - 1, COLS - 1);
				refresh();
				getch(); /* let the user read the message */
				break;

			case game_quit: /* just get out of the loop */
				stop_timer_thread(timer_thread, timer_params);
			case game_not_over: /* continue playing */
				repaint_field = 1; /* field changed, repaint */
				break;
			}
			break;
				
		case 'P': /* pause the game */
		case 'p':
			pthread_mutex_lock(&timer_params->control_mutex); /* suspend timer count */
			pause_game();
			pthread_mutex_unlock(&timer_params->control_mutex);
			repaint_field = 1; /* redraw field to clear pause window leftovers */
			break;

		case 'Q': /* quit game */
		case 'q':
		case 27: /* ESC */
			if (timer_thread) pthread_mutex_lock(&timer_params->control_mutex); /* pause the time counter */

			/* ask the user if he really wants to abort game */
			if (message_box(MESSAGEBOX_TEXT_ABORTGAME) == MESSAGEBOX_EXIT_YES)
			{
				stop_timer_thread(timer_thread, timer_params);
				game_status = game_quit;
			}
			else
			{
				/* resume the time counter */
				if (timer_thread) pthread_mutex_unlock(&timer_params->control_mutex);
				repaint_field = 1; /* if answer is no, repaint and continue */	
			}
			break;

		default: /* ignore all keys except up and down */
			break;
		} /* end switch key */
	} while (game_not_over == game_status);

	/* clean up: */
	free(field);
	free(timer_params);
	wclear(game_win);
	wrefresh(game_win);
	delwin(game_win);

	/* clear status lines: */
	move(LINES - 2, 0);
	clrtoeol();
	move(LINES - 1, 0);
	clrtoeol();
	refresh();
}

/******************************************************************************/

/************************** field control functions ***************************/

char* initialize_field(int difficulty)
{
	/*  allocates memory for the game field and initializes it with values
	    parameters:
	    difficulty - game difficulty  */

	char *field = NULL;
	int  size_of_field, i, bomb_pos;
	int  real_field_width = game_difficulty[difficulty].width + 2;
	int  real_field_height = game_difficulty[difficulty].height + 2;

	if ((difficulty < 0) || (difficulty >= GAME_DIFFICULTY_MAX)) return NULL; /* unknown difficulty setting */

	/* allocate memory fo the field */
	size_of_field = real_field_width * real_field_height;
	field = malloc(size_of_field);
	if (NULL == field) return NULL;

	/* clear the field */
	memset(&field[real_field_width], FIELD_SQUARE_EMPTY, size_of_field - (real_field_width * 2));

	/* initialize border as open + no mines (this is to avoid different cases of check) */
	memset(field, 0, real_field_width);
	memset(&field[size_of_field - real_field_width], 0, real_field_width);
	for (i = real_field_width; i < (real_field_width * (real_field_height - 1)); i += real_field_width)
	{
		field[i] = 0;
		field[i + real_field_width - 1] = 0;
	}

	/* spread bombs */
	for (i = 0; i < game_difficulty[difficulty].bombs; i++)
	{
		bomb_pos = rand() % (size_of_field - (real_field_width * 2)) + real_field_width;

		/* make sure the position is empty */
		while ((FIELD_SQUARE_BOMB == field[bomb_pos]) || (0 == field[bomb_pos]))
		{
			bomb_pos++; /* if not move one square */
			if (bomb_pos >= size_of_field) bomb_pos = 0;
		}

		field[bomb_pos]	= FIELD_SQUARE_BOMB;
	}

	return field;
}

int flag_square(char *field, int height, int width, int cur_y, int cur_x, int *bombs)
{
	/*  flags the square
	    parameters:
	    field - game field
	    height, width - dimentions of game field
	    cur_y, cur_x - current position on field
	    return:
	    0 - no change (square probably already open)
	    1 - flagged (repaint)
	    2 - unflagged (repaint)  */


	/* parameter validity check: */
	if ((NULL == field) || (height < 1) || (width < 1) || (cur_y > height) || (cur_x > width)|| (cur_y < 1) || (cur_x < 1))
		return 0; /* nothing happened */

	/* square already open */
	if ((field[(cur_y * (width + 2)) + cur_x] >= 0) && (field[(cur_y * (width + 2)) + cur_x] <= 10)) return 0;

	if (field[(cur_y * (width + 2)) + cur_x] > 10) /* has flag */
	{
		field[(cur_y * (width + 2)) + cur_x] -= FLAG_VALUE; /* remove flag */
		(*bombs)++; /* update number of bombs left to find */
		return 2;
	}
	
	field[(cur_y * (width + 2)) + cur_x] += FLAG_VALUE; /* set flag */
	(*bombs)--; /* update number of bombs left to find */
	return 1; /* square flagged */
}

game_result uncover_square(char *field, int height, int width, int cur_y, int cur_x)
{
	/*  this is the main part of thegame engine. This function uncovers squares,
	    sheck for bomb hits and report game compleation.
	    parameters:
	    field - game field
	    height, width - dimentions of game field
	    cur_y, cur_x - current position on field */

	int real_field_width = width + 2;
	int x, y, work_x, work_y;
	field_point field_queue[MAX_QUEUE_SIZE]; /* queue array for squares to be processed */
	int field_queue_pos;

	/* parameter validity check: */
	if ((NULL == field) || (height < 1) || (width < 1) || (cur_y > height) || (cur_x > width) || (cur_y < 1) || (cur_x < 1))
		return game_not_over; /* no error code, just continue */

	/* check if square is flagged or already open: */
	if (field[(cur_y * real_field_width) + cur_x] >= 0) return game_not_over;

	if (FIELD_SQUARE_BOMB == field[(cur_y * real_field_width) + cur_x]) /* kaboom!!! */
	{
		field[(cur_y * real_field_width) + cur_x] = FIELD_SQUARE_EXPLOAD;
		return game_lost;
	}

	/* reset queue of squares to process: */
	memset(field_queue, -1, sizeof(field_queue));
	field_queue[0].x = (char)cur_x; field_queue[0].y = (char)cur_y;
	field_queue_pos = 1;

	do /* uncover all squares in a 'cascade' */
	{
		/* start work on last square in queue: */
		field_queue_pos--;
		work_x = field_queue[field_queue_pos].x;
		work_y = field_queue[field_queue_pos].y;

		field[(work_y * real_field_width) + work_x] = 0;
		for (y = work_y - 1; y <= work_y + 1; y++) /* count sorrounding bombs */
		{
			for (x = work_x - 1; x <= work_x + 1; x++)
			{
				if ((FIELD_SQUARE_BOMB == field[(y * real_field_width) + x]) ||
							(FIELD_SQUARE_FLAGBOMB == field[(y * real_field_width) + x]))
				{
					field[(work_y * real_field_width) + work_x]++;
				}
			} /* end for x */
		} /* end for y */

		if (0 == field[(work_y * real_field_width) + work_x]) /* no bombs around squere, add uncovered neighbors to queue */
		{
			for (y = work_y - 1; y <= work_y + 1; y++)
			{
				for (x = work_x - 1; x <= work_x + 1; x++)
				{
					if (field[(y * real_field_width) + x] < 0)
					{
						field_queue[field_queue_pos].x = x;
						field_queue[field_queue_pos].y = y;
						field_queue_pos++;
					}
				} /* end for x */
			} /* end for y */
		}/* end if 0 bombs */
	} while (field_queue_pos != 0);

	/* check if all squares have been uncovered: */
	for (x = real_field_width; x < ((height + 1) * real_field_width); x++)
	{
		if ((field[x] == FIELD_SQUARE_EMPTY) || (field[x] == FIELD_SQUARE_FLAG))
			return game_not_over; /* found an uncovered empty square or false flag */
	}

	return game_won;
}

void pause_game()
{
	/* Displays a message that the game is paused and waits for the user to hit a key
	   to resume the game */
	
	WINDOW* pause_msg_wnd;
	int     width = strlen(PAUSE_TXT_HITKEY) + 2;
	
	
	/* create window for pause message center screen */
	pause_msg_wnd = newwin(4, width, (LINES - 4) / 2, (COLS - width) / 2);
	if (!pause_msg_wnd) return; /* make sure we have a window before preceeding */
	
	/* print the text */
	wclear(pause_msg_wnd);
	wattrset(pause_msg_wnd, A_BOLD | COLOR_PAIR(COLOR_PAIR_CYAN));
	box(pause_msg_wnd, ACS_VLINE, ACS_HLINE);
	wattron(pause_msg_wnd, COLOR_PAIR(COLOR_PAIR_NORMAL));
	mvwprintw(pause_msg_wnd, 1, (width - strlen(PAUSE_TXT_PAUSE)) / 2, PAUSE_TXT_PAUSE);
	wattron(pause_msg_wnd, COLOR_PAIR(COLOR_PAIR_YELLOW));
	mvwprintw(pause_msg_wnd, 2, 1, PAUSE_TXT_HITKEY);
	wrefresh(pause_msg_wnd);

	/* get the cursor out of the way and update the screen */
	move(LINES - 1, COLS - 1);
	refresh();
	
	(void)getch(); /* wait for any key */
	
	/* cleanup */
	wclear(pause_msg_wnd);
	wrefresh(pause_msg_wnd);	
	delwin(pause_msg_wnd);
}

/******************************************************************************/
