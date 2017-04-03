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

 
#ifndef _BEST_TIMES_H
#define _BEST_TIMES_H
 
 
#define MAX_PLAYER_NAME		12
#define MAX_TITLE_LEN		20

#define NUM_TIME_RECORDS	5
#define TIMES_FILENAME		"best_times.lst"
#define TIMES_FILEPATH		".term_bomber"
 
#define NAME_INPUT_HEIGHT	5
#define NAME_INPUT_WIDTH	42
 
#define NAME_INPUT_TITLE	"Congratulations!"
#define NAME_INPUT_MESSAGE	"You finished in record time of %d sec."
#define NAME_INPUT_PROMPT	"Pleas enter your name:"
 
#define BEST_TIMES_TITLE	"BEST TIMES"
#define BEST_TIMES_EXIT		"Hit C or Del to reset records, any other key to return to main menu."

#define RESET_TIMES_QUESTION   "Are you sure you want to reset best times?"
 
/* structure for record times table */
typedef struct {
	char name[MAX_PLAYER_NAME + 1];
	int  game_time;
} time_record;

/* main times functions */
void display_best_times();
int add_game_time(int game_time, int difficulty);

/* file managment */
int load_game_times();
int save_game_times();

/* helper functions */
void set_default_times();
int is_record_time(int game_time, int difficulty);
int input_name(WINDOW *cur_win, int start_y, int start_x, char *player_name);
void paint_time_section(WINDOW *cur_win, int difficulty);
void paint_time_line(WINDOW *cur_win, int difficulty, int record_num);

#endif /* _BEST_TIMES_H */
