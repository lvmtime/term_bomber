 /********************************************************************************/
/*										*/
/* Terminal Bomber								*/
/* Text based minesweeper style game						*/
/* v 0.1 date 5/9/2008								*/
/*										*/
/* Copyright 2008 Lev Meirovitch						*/
/* <LVMTime@gmail.com>								*/
/*										*/
/* This file is part of Terminal Bomber.					*/
/*										*/
/* Terminal Bomber is free software: you can redistribute it and/or modify	*/
/* it under the terms of the GNU General Public License as published by		*/
/* the Free Software Foundation, either version 3 of the License, or		*/
/* (at your option) any later version.						*/
/*										*/
/* Terminal Bomber is distributed in the hope that it will be useful,		*/
/* but WITHOUT ANY WARRANTY; without even the implied warranty of		*/
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the		*/
/* GNU General Public License for more details.					*/
/*										*/
/* You should have received a copy of the GNU General Public License		*/
/* along with Terminal Bomber. If not, see <http://www.gnu.org/licenses/>. 	*/
/*										*/
/********************************************************************************/

#include <config.h> /* now with autotools! */

#include <signal.h> /* At least on Ubuntu 8.04 Hardy Heron something in this files is not strictly ANSI complient */
#include <unistd.h>
#include <curses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "term_bomber.h"
#include "game_engine.h"
#include "best_times.h"
 
 
/* menu arrays: */
const char *main_menu[] = {"Play",
			   "Best times",
			   "Configure",
			   "Help",
			   "Exit",
			   NULL};

const char *game_menu[] = {"Easy",
			   "Medium",
			   "Hard",
			   NULL};

int main()
{
	int selection = 0, difficulty;
	struct sigaction	sig_act;
	
	/* ignore Ctrl + C (interrupt signal) */
	sig_act.sa_handler = SIG_IGN;
	sigemptyset(&sig_act.sa_mask);
	sig_act.sa_flags = SA_RESTART;
	
	sigaction(SIGINT, &sig_act, NULL);

	/* initialize the curses library */
	switch(initialize_terminal())
	{
		case 0:
			break;
		case 1:
			fprintf(stderr, "%s: This geme is not nearly as fun without colors so I quit!\n", __FILE__);
			exit(EXIT_FAILURE);
		case 2:
			fprintf(stderr, "%s: I can't paint in pretty colors! Na, na na na!\n", __FILE__);
			exit(EXIT_FAILURE);
		case 3:
			fprintf(stderr, "%s: You can't play this game without a terminal!\n", __FILE__);
			exit(EXIT_FAILURE);		
		default:
			fprintf(stderr, "%s: A strange and unknow error occured while talking to the reminal\nI am scared!\n", __FILE__);
			exit(EXIT_FAILURE);
	}

	srand((unsigned int)time(NULL)); /* set random seeds for bomb spreading */
	load_game_times();

	paint_title();
	refresh();
	
	/* main gaim loop: */
	do
	{
		/* print copyrigth info in the status area */
		move(LINES - 1, 0);
		clrtoeol();
		attrset(COLOR_PAIR(COLOR_PAIR_NORMAL) | A_BOLD);
		mvprintw(LINES - 1, 0, APP_PROGRAMINFO, APP_VERSION, APP_RELEASETYPE);
		mvprintw(LINES - 1, COLS - (strlen(APP_PROGRAMAUTHOR) + 1), APP_PROGRAMAUTHOR);
		refresh();

		selection = do_menu(main_menu); /* display main menu */

		switch(selection) /* act upon options */
		{
		case 0: /* select difficulty and play */
			difficulty = do_menu(game_menu);
			if (difficulty > -1) play_game(difficulty);
			else
				selection = -1; /* for some reason difficulty menu returned error */
			break;

		case 1: /* show time records */
			display_best_times();
			selection = 0;
			break;

		case 2:
			selection = 0;
			break;
		
		case 3: /* show help screen */
			paint_help();
			selection = 0;
			break;

		case 4: /* exit game */
			break;

		default:
			selection = -1;
			break;
		}

	} while ((selection > -1) && (selection != 4));

	if (-1 == selection) /* error occured in gaim loop */
	{
		clear();
		paint_title();
		attron(COLOR_PAIR(COLOR_PAIR_RED));
		mvprintw(3, 10, "An error occured so the game can not be playd.");
		mvprintw(4, 10, "Sorry but I am too lazy to print details so I am just exiting");
		refresh();
		(void)getch();
	}
	
	/* in a real terminal we must clear the main window before exiting */
	clear();
	refresh();

	endwin();
	exit(EXIT_SUCCESS);
}

int initialize_terminal()
{
	/*  initialzes curses library and sets up color peirs and input status
	    return:
	    0 - sucess
	    1 - terminal does not support colors
	    2 - error initializing colors
	    3 - output redirected to something other then a console */

	
	if (!isatty(1)) return 3;
	
	initscr();

	if (!has_colors()) /* check for color suppor (maybe later we can work without it) */
	{
		endwin();
		return 1;
	}

	if (OK != start_color()) /* enter color mode */
	{
		endwin();
		return 2;
	}

	/* initialize color pairs: */
	init_pair(COLOR_PAIR_NORMAL, COLOR_WHITE, COLOR_BLACK);
	init_pair(COLOR_PAIR_CYAN, COLOR_CYAN, COLOR_BLACK);
	init_pair(COLOR_PAIR_PINK, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(COLOR_PAIR_GREEN, COLOR_GREEN, COLOR_BLACK);
	init_pair(COLOR_PAIR_RED, COLOR_RED, COLOR_BLACK);
	init_pair(COLOR_PAIR_BLUE, COLOR_BLUE, COLOR_BLACK);
	init_pair(COLOR_PAIR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
	init_pair(COLOR_PAIR_EXPLOAD, COLOR_RED, COLOR_YELLOW);
	init_pair(COLOR_PAIR_FLAG, COLOR_RED, COLOR_BLUE);
	init_pair(COLOR_PAIR_MENU_SELECT, COLOR_BLACK, COLOR_BLUE);

	/* set up keyboard for menu and gameplay zhandling: */
	noecho();
	cbreak();
	keypad(stdscr, TRUE);

	attrset(COLOR_PAIR(COLOR_PAIR_NORMAL));
	erase();
	return 0;
}

/************************** main screen functions ******************************/

void paint_title()
{
	/*  paints the colorfull and centered title line  */

	int line_start; /* start position */

	line_start = (COLS - (strlen(APP_TITLE) + 4)) / 2;
	if (line_start < 0) line_start = 0; /* just in case this is a small terminal */

	/* print firs decoration: */
	attrset(COLOR_PAIR(COLOR_PAIR_RED) | A_BOLD);
	mvaddch(0, line_start, '-');
	mvaddch(0, line_start + strlen(APP_TITLE) + 3, '-');

	/* print second decoration: */
	attron(COLOR_PAIR(COLOR_PAIR_GREEN));
	mvaddch(0, line_start + 1, '=');
	mvaddch(0, line_start + strlen(APP_TITLE) + 2, '=');

	/* print title: */
	attron(COLOR_PAIR(COLOR_PAIR_CYAN));
	mvaddnstr(0, line_start + 2, APP_TITLE, strlen(APP_TITLE));

	move(LINES - 1, COLS - 1); /* get the curser out of the way */
}

/*******************************************************************************/

/************************** menu screen functions ******************************/

void paint_menu(WINDOW *menu_win, const char **menu, int width, int selected)
{
	/*  paints the menu options, selection and window border
	    parameters:
	    menu_win - window to paint to
	    menu - array of menu strings
	    width, height - size of window (in lines and chars)  */

	int cur_line = 1;

	/* clear the menu window: */
	wattrset(menu_win, COLOR_PAIR(COLOR_PAIR_NORMAL) | A_BOLD);
	wclear(menu_win);

	box(menu_win, ACS_VLINE, ACS_HLINE);
	while (*menu != NULL)
	{
		if (0 == selected) wattrset(menu_win, COLOR_PAIR(COLOR_PAIR_MENU_SELECT)); /* switch color if this is the selected line */
		mvwprintw(menu_win, cur_line, 1, " %-*s", width - 3, *menu);
		if (0 == selected) wattrset(menu_win, COLOR_PAIR(COLOR_PAIR_NORMAL) | A_BOLD); /* turn off selection color */

		menu++; cur_line++;
		selected--; /* little tric counting down to selection */
	}
	move(LINES - 1, COLS - 1); /* get the cursor out of the way */
	wrefresh(menu_win);
}

int do_menu(const char **menu)
{
	/*  displays a menu and allows selection
	    this function blocks until selection is made
	    parameters:
	    menu - array of menu strings
	    return:
	    index of selected string in menu or -1 on error  */

	WINDOW *menu_win;
	int x, y, height, width;
	int cur_selection = 0, num_options;
	int key;


	if ((num_options = calc_menu_pos(menu, &y, &x, &height, &width)) == 0) return -1; /* calculate menu position on screen based on text size */

	/* create window for the menu: */
	menu_win = newwin(height, width, y, x);
	if (NULL == menu_win) return -1;

	/* commence menu loop: */
	do
	{
		paint_menu(menu_win, menu, width, cur_selection);
		key = getch();

		switch (key) /* act upon user input */
		{
		case KEY_UP:
			cur_selection--;
			if (cur_selection < 0) cur_selection = num_options - 1;
			break;

		case KEY_DOWN:
			cur_selection++;
			if (cur_selection == num_options) cur_selection = 0;
			break;

		default: /* ignore all keys except up and down */
			break;
		}
	} while ((key != KEY_ENTER) && (key != '\n')); /* exit loop when 'Enter' hit */

	/* clean up: */
	wclear(menu_win);
	wrefresh(menu_win);
	delwin(menu_win);

	return cur_selection; /* return the menu option */
}

int calc_menu_pos(const char **menu, int *y, int *x, int *height, int *width)
{
	/*  calculates size and position of menu window, assuming we want it centered verticaly and horizontally
	    parameters:
	    menu - array of menu strings
	    x, y - pointers to vars that recive position of menu window (top left corner)
	    height, width - ponters to vars that recive menu window size (including border)
	    return:
	    number of options in menu  */


	int menu_width = 0, menu_height = 0;


	while (menu[menu_height] != NULL) /* loop to calculate number of line in menu */
	{
		if (strlen(menu[menu_height]) > menu_width) menu_width = strlen(menu[menu_height]); /* save the longeses (widest) menu string length */
		menu_height++;
	}

	menu_width += 2;

	if ((menu_height > (LINES - 2)) || (menu_width > COLS)) return 0; /* check if menu fits on screen */

	*height = menu_height + 2;
	*width = menu_width + 2;

	*x = (COLS - menu_width) / 2; /* center horizontally */
	*y = ((LINES - menu_height) / 2) - 1; /* center vertically and take title + 1 line for specing in to account */

	return menu_height;
}
/*******************************************************************************/

/************************ Help display function ********************************/

void paint_help()
{
	/* Displays the help screen including license information */
	
	WINDOW	*help_win;
	int	win_width = COLS;
	int	win_height = LINES - 2;

	
	/* create help window */
	
	help_win = newwin(win_height, win_width, 2, 0);
	
	/* print program information */
	wattrset(help_win, COLOR_PAIR(COLOR_PAIR_GREEN) | A_BOLD);
	mvwprintw(help_win, 0, (win_width - strlen("ABOUT")) / 2, "ABOUT");
	
	wattron(help_win, COLOR_PAIR(COLOR_PAIR_NORMAL));
	mvwprintw(help_win, 2, 0, "%s - %s   <%s>", APP_TITLE, APP_PROGRAMAUTHOR, APP_AUTHORMAIL);
	mvwprintw(help_win, 3, 0, "Version: %s %s Build: " __DATE__ " " __TIME__ " %s", APP_VERSION, APP_RELEASETYPE,
													APP_SVNREV);
	
	/* print the GNU/GPL license */
	wattroff(help_win, A_BOLD);
	mvwprintw(help_win, 5, 0, "%s is free software: you can redistribute it and/or modify", APP_TITLE);
	mvwprintw(help_win, 6, 0, "it under the terms of the GNU General Public License as published by");
	mvwprintw(help_win, 7, 0, "the Free Software Foundation, either version 3 of the License, or");
	mvwprintw(help_win, 8, 0, "(at your option) any later version.");
	mvwprintw(help_win, 10, 0, "%s is distributed in the hope that it will be useful,", APP_TITLE);
	mvwprintw(help_win, 11, 0, "but WITHOUT ANY WARRANTY; without even the implied warranty of");
	mvwprintw(help_win, 12, 0, "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the");
	mvwprintw(help_win, 13, 0, "GNU General Public License for more details.");
	mvwprintw(help_win, 15, 0, "You should have received a copy of the GNU General Public License");
	mvwprintw(help_win, 16, 0, "along with %s. If not, see <http://www.gnu.org/licenses/>.", APP_TITLE);
	
	wattrset(help_win, COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
	mvwprintw(help_win, win_height - 2, 0, "Page 1 of 2");
	wattron(help_win, COLOR_PAIR(COLOR_PAIR_BLUE));
	mvwprintw(help_win, win_height - 1, 0, "Press any key to continue");	

	/* update the window and wait for user to hit a key */
	move(LINES - 1, COLS - 1);
	wrefresh(help_win);
	getch();

	/* print game instructions */
	wclear(help_win);
	wattrset(help_win, COLOR_PAIR(COLOR_PAIR_YELLOW) | A_BOLD);
	mvwprintw(help_win, 0, (win_width - strlen("HOW TO PLAY")) / 2, "HOW TO PLAY");
	
	wattrset(help_win, COLOR_PAIR(COLOR_PAIR_NORMAL));
	mvwprintw(help_win, 2, 0, "The goal of the game is to clear the field by uncovering all");
	mvwprintw(help_win, 3, 0, "the squares that do not have bombs while avoiding those that do.");
	mvwprintw(help_win, 5, 0, "To uncover a square position the cursor on it using the ");
	wattron(help_win, A_BOLD);
	wprintw(help_win, "arrow keys");
	wattroff(help_win, A_BOLD);
	mvwprintw(help_win, 6, 0, "and press ");
	wattron(help_win, A_BOLD);
	wprintw(help_win, "SPACE");
	wattroff(help_win, A_BOLD);
	wprintw(help_win, " or ");
	wattron(help_win, A_BOLD);
	wprintw(help_win, "U");
	wattroff(help_win, A_BOLD);
	wprintw(help_win, " key.");
	mvwprintw(help_win, 7, 0, "If the square is empty it will be uncovered along with all adjacent");
	mvwprintw(help_win, 8, 0, "empty squares.");
	mvwprintw(help_win, 9, 0, "Numbers printed in the square indicate how many adjacent squares");
	mvwprintw(help_win, 10, 0, "have bombs.");
	mvwprintw(help_win, 12, 0, "You can use flags to mark squares that you suspect have bombs.");
	mvwprintw(help_win, 13, 0, "Flagging a square will help you avoid uncovering it accidentally");
	mvwprintw(help_win, 14, 0, "and loosing the game as well as keep track of the number of bombs");
	mvwprintw(help_win, 15, 0, "left to find.");
	mvwprintw(help_win, 16, 0, "You can set or remove a flag on the current square using ");
	wattron(help_win, A_BOLD);
	wprintw(help_win, "TAB");
	wattroff(help_win, A_BOLD);
	wprintw(help_win, " or ");
	wattron(help_win, A_BOLD);
	wprintw(help_win, "F");
	wattroff(help_win, A_BOLD);
	wprintw(help_win, " key.");
	mvwprintw(help_win, 18, 0, "Use ");
	wattron(help_win, A_BOLD);
	wprintw(help_win, "ESC");
	wattroff(help_win, A_BOLD);
	wprintw(help_win, " or ");
	wattron(help_win, A_BOLD);
	wprintw(help_win, "Q");
	wattroff(help_win, A_BOLD);
	wprintw(help_win, " to abort the game at any time and return to main menu.");
	mvwprintw(help_win, 19, 0, "Use ");
	wattron(help_win, A_BOLD);
	wprintw(help_win, "P");
	wattroff(help_win, A_BOLD);
	wprintw(help_win, " to pause the game.");

	wattrset(help_win, COLOR_PAIR(COLOR_PAIR_CYAN) | A_BOLD);
	mvwprintw(help_win, win_height - 2, 0, "Page 2 of 2");
	wattron(help_win, COLOR_PAIR(COLOR_PAIR_BLUE));
	mvwprintw(help_win, win_height - 1, 0, "Press any key to return to main menu");

	/* update the window and wait for user to hit a key */
	move(LINES - 1, COLS - 1);
	wrefresh(help_win);
	getch();

	/* cleare the screen before exiting */
	wclear(help_win);
	wrefresh(help_win);
	delwin(help_win);	
}

/*******************************************************************************/

/************************** Message box functions ******************************/

int message_box(char* message_text)
{
	/**
	   displays a confirmation message box in the
	   center of the screen and returns a yes or no user selection.

	   parameters:
	   message_text - text of the question to display

	   return:
	   MESSAGEBOX_EXIT_YES - user answered yes
	   MESSAGEBOX_EXIT_NO - user answered no
	   MESSAGEBOX_ERROR - some error occured
	 **/
	 
	int	answer = MESSAGEBOX_EXIT_NO;
	WINDOW  *msg_wnd;
	int     y, x, width, key, yes_x, no_x;
	
	
	/* calculate window size and position */
	width = strlen(message_text) + 4;
	if (width < (strlen(MESSAGEBOX_TEXT_YES) + strlen(MESSAGEBOX_TEXT_NO) + 5))
		width = strlen(MESSAGEBOX_TEXT_YES) + strlen(MESSAGEBOX_TEXT_NO) + 5;

	y = (LINES - 4) / 2;
	x = (COLS - width) / 2;
	
	/* calculate position of answer bottuns (yes / no) */
	yes_x = ((width / 2) - strlen(MESSAGEBOX_TEXT_YES)) / 2;
	no_x = (((width / 2) - strlen(MESSAGEBOX_TEXT_NO)) / 2) + (width / 2);
	
	/* create the window */
	msg_wnd = newwin(4, width, y, x);
	if (NULL == msg_wnd) return MESSAGEBOX_ERROR;
	
	/* paint border and question */
	wclear(msg_wnd);
	wattrset(msg_wnd, COLOR_PAIR(COLOR_PAIR_YELLOW) | A_BOLD);
	box(msg_wnd, ACS_VLINE, ACS_HLINE);
	
	wmove(msg_wnd, 0, ((width - strlen(MESSAGEBOX_TITLE)) / 2) - 1);
	waddch(msg_wnd, ACS_RTEE);
	waddstr(msg_wnd, MESSAGEBOX_TITLE);
	waddch(msg_wnd, ACS_LTEE);
	
	wattron(msg_wnd, COLOR_PAIR(COLOR_PAIR_NORMAL));
	mvwaddstr(msg_wnd, 1, 1, message_text);
	
	/* loop to let the user choose an answer */
	do {
		if (MESSAGEBOX_EXIT_YES == answer)
			wattron(msg_wnd, COLOR_PAIR(COLOR_PAIR_MENU_SELECT));
		else
			wattron(msg_wnd, COLOR_PAIR(COLOR_PAIR_CYAN));
		mvwaddstr(msg_wnd, 2, yes_x, MESSAGEBOX_TEXT_YES);

		if (MESSAGEBOX_EXIT_NO == answer)
			wattron(msg_wnd, COLOR_PAIR(COLOR_PAIR_MENU_SELECT));
		else
			wattron(msg_wnd, COLOR_PAIR(COLOR_PAIR_CYAN));
		
		mvwaddstr(msg_wnd, 2, no_x, MESSAGEBOX_TEXT_NO);		
		wrefresh(msg_wnd);
		move(LINES - 1, COLS - 1);
		refresh();
		
		key = getch();
		switch(key) /* move the selection between two choices */
		{
		case KEY_LEFT:
		case KEY_RIGHT:
			answer = (MESSAGEBOX_EXIT_YES == answer) ? MESSAGEBOX_EXIT_NO : MESSAGEBOX_EXIT_YES;
			break;
		case 27: /* ESC = cancel question */
			answer = MESSAGEBOX_EXIT_NO;
			key = KEY_ENTER;
		}
	} while ((key != KEY_ENTER) && (key != '\n'));
	
	wclear(msg_wnd);
	wrefresh(msg_wnd);
	delwin(msg_wnd);
	
	return answer;
}

/*******************************************************************************/
