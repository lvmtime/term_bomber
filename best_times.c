/********************************************************************************/
/*										*/
/* Terminal Bomber gmae engine							*/
/* v 0.2 date 3/11/2008								*/
/*										*/
/* Copyright 2008 Lev Meirovitch						*/
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

 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>
#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "term_bomber.h"
#include "game_engine.h"
#include "best_times.h"


/* global var to hold game time records */
static time_record	best_times[GAME_DIFFICULTY_MAX][NUM_TIME_RECORDS];
 
 
/************************** main times functions ******************************/

void display_best_times()
{
	/**
	 Displays the 'best times' screen
	 **/
	
	WINDOW *panel_win[GAME_DIFFICULTY_MAX];
	WINDOW *main_win;
	int	win_pos_x[GAME_DIFFICULTY_MAX], win_pos_y[GAME_DIFFICULTY_MAX];
	int	i, key;
	
	/* create main window */
	main_win = newwin(LINES - 3, COLS, 2, 0);
	if (NULL == main_win) return;
	wclear(main_win);
	
	/* initialize window positions */
	win_pos_x[0] = (COLS - (((MAX_PLAYER_NAME + 6) * 3) + 8)) / 2;
	win_pos_x[1] = win_pos_x[0] + MAX_PLAYER_NAME + 10;
	win_pos_x[2] = win_pos_x[1] + MAX_PLAYER_NAME + 10;
	win_pos_y[0] = win_pos_y[1] = win_pos_y[2] = (LINES - (NUM_TIME_RECORDS + 5)) / 2;
	
	/* create panel windows */
	for (i = 0; i < GAME_DIFFICULTY_MAX; i++)
		panel_win[i] = subwin(main_win, NUM_TIME_RECORDS + 2, MAX_PLAYER_NAME + 6, win_pos_y[i], win_pos_x[i]);
	
	/* paint screen title and instructions */
	wattrset(main_win, COLOR_PAIR(COLOR_PAIR_RED) | A_BOLD);
	mvwprintw(main_win, 0, (COLS - strlen(BEST_TIMES_TITLE)) / 2, "%s", BEST_TIMES_TITLE);
	wattrset(main_win, COLOR_PAIR(COLOR_PAIR_BLUE) | A_BOLD);
	mvwprintw(main_win, LINES - 4, 0, "%s", BEST_TIMES_EXIT);
	
	do /* "mini menu" loop with two options: reset or exit */
	{
	       /* display times */		
		for (i = 0; i < GAME_DIFFICULTY_MAX; i++) paint_time_section(panel_win[i], i);
		wmove(main_win, LINES - 4, COLS - 1);
		wrefresh(main_win);

		key = getch(); 	/* wait for user input */
		if (('c' == key) || ('C' == key) || (0x14a == key))
		{
			key = 0;
			if (MESSAGEBOX_EXIT_YES == message_box(RESET_TIMES_QUESTION))
			{
				set_default_times();
				(void)save_game_times();
			}
		}
	} while (0 == key);
	
	/* clean up */
	for (i = 0; i < GAME_DIFFICULTY_MAX; i++)
		delwin(panel_win[i]);

	wclear(main_win);
	wrefresh(main_win);
	delwin(main_win);
}

int add_game_time(int game_time, int difficulty)
{
	/**
	 Checks if current game time broke a record, prompts for player name and adds arecord if so.
	 
	 parameters:
	 game_time - time to check
	 difficulty - category to check against
	 
	 return:
	 -1 - name input aborted
	 0 - time not added
	 1 - time added
	 
	 **/
	
	WINDOW *input_win;
	int 	place, res;
	int	x, y, width, height;
	char	name_buffer[MAX_PLAYER_NAME];
	
	
	/* check if time should be added */
	place = is_record_time(game_time, difficulty);
	if (place < 0) return 0;
	
	/* set up input window parameters */
	height = NAME_INPUT_HEIGHT;
	width = NAME_INPUT_WIDTH;
	x = (COLS - width) / 2;
	y = (LINES - height) / 2;
	
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	
	/* create input window an paint content */
	input_win = newwin(height, width, y, x);
	if (NULL == input_win) return 0;
	
	wclear(input_win);
	wattrset(input_win, COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
	box(input_win, ACS_VLINE, ACS_HLINE);
	
	wmove(input_win, 0, ((width - strlen(NAME_INPUT_TITLE)) / 2) - 1);
	waddch(input_win, ACS_RTEE);
	wprintw(input_win, "%s", NAME_INPUT_TITLE);
	waddch(input_win, ACS_LTEE);

	wattrset(input_win, COLOR_PAIR(COLOR_PAIR_NORMAL) | A_BOLD);
	mvwprintw(input_win, 1, 2, NAME_INPUT_MESSAGE, game_time);
	mvwprintw(input_win, 2, 2, NAME_INPUT_PROMPT);
	
	/* read player name */
	res = input_name(input_win, 3, 2, name_buffer);
	
	if (res != 1) return -1; /* fail */
	
	/* move down all records from the insert point */
	for (res  = (NUM_TIME_RECORDS - 1); res > place; res--)
	{
		best_times[difficulty][res].game_time = best_times[difficulty][res - 1].game_time;
		memcpy(best_times[difficulty][res ].name, best_times[difficulty][res - 1].name, MAX_PLAYER_NAME);
	}
	
	/* insert new record */
	best_times[difficulty][place].game_time = game_time;
	memcpy(best_times[difficulty][place].name, name_buffer, MAX_PLAYER_NAME);
	
	(void)save_game_times();
	
	wclear(input_win);
	wrefresh(input_win);
	delwin(input_win);
	
	return 1;
}

/*******************************************************************************/

/****************************** file managment *********************************/

int load_game_times()
{
	/* Initialize the best times structure and attempt to load best times from file

	   returns:
	   0 - Successful
	   1 - File not found (array filled with defaults)
	   2 - Error reading from file
	*/
	
	
	int 		i, j;
	struct passwd	*pwd;
	char		file_path[FILENAME_MAX];
	FILE		*times_file;
	char		buffer[MAX_PLAYER_NAME + sizeof(int)];
	
	/* initialize the array */
	set_default_times();
	
	/* get home directory from envirment varible and construct file path*/
	pwd = getpwuid(getuid());
	sprintf(file_path, "%s/%s/%s", pwd->pw_dir, TIMES_FILEPATH, TIMES_FILENAME);
	
	/* open 'best tmes' list file */
	times_file = fopen(file_path, "rb");
	if (NULL == times_file) return 1;
	
	/* read high scores in a loop */
	for (i = 0; i < GAME_DIFFICULTY_MAX; i++)
	{
		for (j = 0; j < NUM_TIME_RECORDS; j++)
		{
			/* read single score line */
			if (fread(buffer, 1, sizeof(buffer), times_file) < sizeof(buffer))
			{
				fclose(times_file);
				return 2;
			}
			
			best_times[i][j].game_time = *((int*)&buffer[MAX_PLAYER_NAME]);
			
			/* simple validation of score time */
			if ((best_times[i][j].game_time > 9999) || (best_times[i][j].game_time < 1))
				best_times[i][j].game_time = 9999;
			else
				memcpy(best_times[i][j].name, buffer, MAX_PLAYER_NAME); /* copy player name from buffer */
		}
	} /* end for i */
	
	fclose(times_file);
	return 0;
}

int save_game_times()
{
	/**
	 Save best time records to file
	 Creat the file and the directory in current user's home folder if needed
	 
	 return:
	 0 - Successful
	 1 - Error accessing directory
	 2 - Error creating file
	 3 - Error writing to fle
	 **/
	
	int		times_file;
	int 		i, j;
	struct passwd	*pwd;
	char		file_path[FILENAME_MAX];
	
	
	/* switch to program directory */
	pwd = getpwuid(getuid());
	sprintf(file_path, "%s/%s", pwd->pw_dir, TIMES_FILEPATH);
	i = access(file_path, F_OK);
	
	if (i != 0) /* if it doesn't exist, try to creat it */
	{	
		if (0 != mkdir(file_path, S_IRWXU | S_IRGRP))
			return 1;
	}
	
	/* open the file - creat if needed or empty if exists */
	
	strcat(file_path, "/"TIMES_FILENAME);
	times_file = open(file_path, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP);
	if (times_file < 0) return 2;
	
	/* write times array to file */
	for (i = 0; i < GAME_DIFFICULTY_MAX; i++)
	{
		for (j = 0; j < NUM_TIME_RECORDS; j++)
		{
			/* write player name */
			if (write(times_file, best_times[i][j].name, MAX_PLAYER_NAME) < MAX_PLAYER_NAME)  
			{
				close(times_file);
				return 3;
			}
			
			/* write time */
			if (write(times_file, &best_times[i][j].game_time, sizeof(int)) < sizeof(int))  
			{
				close(times_file);
				return 3;
			}
		}
	} /* end for i */
	
	close(times_file);
	
	return 0;
}

/*******************************************************************************/

/****************************** helper functions *******************************/

void set_default_times()
{
	/** 
	 Sets default values to time record array
	 **/
	
	int i, j;
	
	
	for (i = 0; i < GAME_DIFFICULTY_MAX; i++)
	{
		for (j = 0; j < NUM_TIME_RECORDS; j++)
		{
			strncpy(best_times[i][j].name, "Nobody", MAX_PLAYER_NAME + 1);
			best_times[i][j].game_time = 9999; /*max out the time*/
		}
	}
}

int is_record_time(int game_time, int difficulty)
{
	/**
	 Checks if current time beats any record.
	 
	 parameters:
	 game_time - time to chek
	 difficulty - category to check in
	 
	 return:
	 if this is a 'record' time, return its position in the array
	 -1 means time is not a rcord breaker
	 -2 bad parameter
	 **/
	
	int	i;
	
	
	if (difficulty >= GAME_DIFFICULTY_MAX) return -2;
	
	for (i = 0; i < NUM_TIME_RECORDS; i++)
		if (game_time < best_times[difficulty][i].game_time) return i;
	
	return -1;
}

int input_name(WINDOW *cur_win, int start_y, int start_x, char *player_name)
{
	/**
	 Reads limited length string from input
	 
	 parameters:
	 cur_win - window to print in
	 start_y, start_x - coordinates for input field
	 player_name - input buffer for player name
	 
	 return:
	 0 - input cancled (ESC) - player name will not be changed
	 1 - input confirmed (Enter)
	 2 - bad parameter
	**/
	
	int	key, str_len = 0;
	char	in_buffer[MAX_PLAYER_NAME + 1];
	

	/* verify parameters */
	if ((NULL == cur_win) || (NULL == player_name) || (start_x < 0) || (start_y < 0) ||
	    (start_x > (COLS - MAX_PLAYER_NAME)) || (start_y >= LINES)) return 2;
	
	memset(in_buffer, 0, sizeof(in_buffer));
	wattrset(cur_win, COLOR_PAIR(COLOR_PAIR_NORMAL) | A_BOLD);
	
	keypad(cur_win, TRUE); /* make sure this specific window is in keypad mode */
	
	/* input loop */
	do
	{
		mvwprintw(cur_win, start_y, start_x, "%-*s", MAX_PLAYER_NAME, in_buffer);
		wrefresh(cur_win);
		
		key = mvwgetch(cur_win, start_y, start_x + strlen(in_buffer));
		
		if ((KEY_LEFT == key) || (KEY_BACKSPACE == key)) /* erase character */
		{
			if (str_len > 0) in_buffer[--str_len] = '\0';
		}
		else if ((key > 9) && (key < 127)) /* add character */
		{
			if (str_len < MAX_PLAYER_NAME) in_buffer[str_len++] = (char)key;
		}		
	} while ((key != KEY_ENTER) && (key != '\n') && (key != 27));
	
	keypad(cur_win, FALSE);
	
	if (27 == key) return 0;
	memcpy(player_name, in_buffer, MAX_PLAYER_NAME);
	
	return 1;
}

void paint_time_section(WINDOW *cur_win, int difficulty)
{
	/**
	 Prints list of best times for a given game difficulty
	 
	 parameters:
	 cur_win - window to paint in
	 difficulty - array setion to display
	 **/
	
	int 	i;
	char	title_str[MAX_TITLE_LEN + 1];
	
	
	if (NULL == cur_win) return; /* quick check */
	
	wclear(cur_win);
	
	/* make section title uppercase */
	memset(title_str, 0, sizeof(title_str));
	for (i = 0; (i < strlen(game_menu[difficulty])) && (i < MAX_TITLE_LEN); i++)
		title_str[i] = (char)toupper(game_menu[difficulty][i]);
	
	/* print section title */
	wattrset(cur_win, COLOR_PAIR(COLOR_PAIR_GREEN) | A_BOLD);
	mvwprintw(cur_win, 0, ((MAX_PLAYER_NAME + 6) - strlen(title_str)) / 2, "%s", title_str);
	
	/* print records */
	for (i = 0; i < NUM_TIME_RECORDS; i++)
		paint_time_line(cur_win, difficulty, i);
	
	wrefresh(cur_win);
}

void paint_time_line(WINDOW *cur_win, int difficulty, int record_num)
{
	/**
	 Print a single line with player name and record time
	 
	 parameters:
	 cur_win - window to paint in.
	 difficulty - current array section
	 record_num - current line
	 **/
	
	
	wattrset(cur_win, COLOR_PAIR(COLOR_PAIR_NORMAL) | A_BOLD);
	mvwprintw(cur_win, record_num + 2, 0, "%s", best_times[difficulty][record_num].name);
	wattrset(cur_win, COLOR_PAIR(COLOR_PAIR_YELLOW) | A_BOLD);
	mvwprintw(cur_win, record_num + 2, MAX_PLAYER_NAME + 2, "%4d", best_times[difficulty][record_num].game_time);
}

/*******************************************************************************/
