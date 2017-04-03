/********************************************************************************/
/*										*/
/* Terminal Bomber gmae engine							*/
/* v 0.2 date 13/2/2009								*/
/*										*/
/* Copyright 2008 - 2009 Lev Meirovitch						*/
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

#ifndef _GAME_ENGINE_H
#define _GAME_ENGINE_H

/* game field definitions: */
/** vlues 0 - 9 indicate open square with 0 - 9 bombs next to it **/
/** negative values mean covered squares **/
#define FIELD_SQUARE_EMPTY	(-1)
#define FIELD_SQUARE_BOMB	(-2)

#define FIELD_SQUARE_EXPLOAD	10 /* exploaded bomb */
#define FIELD_SQUARE_FLAG	19 /* square flagged as possible bomb (but really empty) */
#define FIELD_SQUARE_FLAGBOMB	18 /* square flagged and has a bomb */

#define FLAG_VALUE		20 /* this value is added or subtracted when square is flagged */

/* maximum game difficulty: */
#define GAME_DIFFICULTY_MAX	3
#define MAX_QUEUE_SIZE		379 /* maximu squares for proccessing (size of queue) */


typedef struct {
		char y, x;
} field_point;

/* game status: */
typedef enum {game_not_over, game_quit, game_won, game_lost} game_result;

/* game difficulty field sizes: */
typedef struct {
	int height;
	int width;
	int bombs; /* number of bombs for this difficulty setting */
} field_size;

/* timer display thread parameters structure */
typedef struct {
	WINDOW*		 timer_wnd;
	WINDOW*		 game_wnd; /* game windwo to return the cursor to */
	pthread_mutex_t	 control_mutex; /* used to pause the counter */
	pthread_mutex_t	 paint_mutex; /* marshels screen painting */
	int  game_time;
} timer_thread_params;

extern const field_size game_difficulty[GAME_DIFFICULTY_MAX];

/* gmae display functions */
void paint_field(WINDOW *field_win, const char *field, int height, int width, int bombs);

/* game functions: */
void play_game(int difficulty);
char* initialize_field(int difficulty);
int flag_square(char *field, int height, int width, int cur_y, int cur_x, int *bombs);
game_result uncover_square(char *field, int height, int width, int cur_y, int cur_x);
void pause_game();

/* timer functions */
pthread_t start_timer_thread(WINDOW* game_wnd, timer_thread_params *timer_params);
void stop_timer_thread(pthread_t timer_thread, timer_thread_params *timer_params);
void *timer_thread_func(void* pParam);

#endif /*_GAME_ENGINE_H*/
