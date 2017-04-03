/********************************************************************************/
/*										*/
/* Terminal Bomber								*/
/* Text based minesweeper style game						*/
/* v 0.2 date 13/2/2009								*/
/*										*/
/* Copyright 2008, 2009 Lev Meirovitch						*/
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


#ifndef _TERM_BOMBER_H
#define _TERM_BOMBER_H


/* string used in the app: */
#define APP_TITLE		PACKAGE_NAME
#define APP_VERSION		VERSION
#define APP_RELEASETYPE		"SVN"
#define APP_SVNREV		"$Revision: 16 $"

#define APP_PROGRAMINFO		"ver. %s %s"
#define APP_PROGRAMAUTHOR	"Copyright 2008, 2009 Lev Meirovitch"
#define APP_AUTHORMAIL		PACKAGE_BUGREPORT

#define GAME_INSTRUCTIONS 	"Arrows - select, Space / U - unciover, Tab / F - flag, P - pause, Esc / Q - Quit"
#define GAME_BOMBS_LEFT		"Bombs remaining: %3d"
#define GAME_LOST		"Game over, you lost!"
#define GAME_WON		"Congratulations! You compleated the game in %d seconds."
#define GAME_TIMER_TEXT		"Time:       sec."

/* pause window text */
#define PAUSE_TXT_PAUSE		"Game paused"
#define PAUSE_TXT_HITKEY	"Hit any key to continue."

#define TIMER_WND_X		((COLS - strlen(GAME_TIMER_TEXT)) + 6)

/* message box defenitions */

#define MESSAGEBOX_TITLE		"Confirmation"
#define MESSAGEBOX_TEXT_YES		"<Yes>"
#define MESSAGEBOX_TEXT_NO		"<No>"
#define MESSAGEBOX_TEXT_ABORTGAME       "Are you sure you wan to quit this game?"

#define MESSAGEBOX_ERROR	-1

#define MESSAGEBOX_EXIT_YES     0
#define MESSAGEBOX_EXIT_NO      1

/* color pairs: */
#define COLOR_PAIR_NORMAL	1
#define COLOR_PAIR_CYAN		2
#define COLOR_PAIR_BLUE		3
#define COLOR_PAIR_GREEN	4
#define COLOR_PAIR_RED		5
#define COLOR_PAIR_YELLOW	6
#define COLOR_PAIR_PINK		7
#define COLOR_PAIR_EXPLOAD	8
#define COLOR_PAIR_FLAG		9
#define COLOR_PAIR_MENU_SELECT	10

extern const char *main_menu[];
extern const char *game_menu[];

/* initilization functions: */
int initialize_terminal();

/* display functions: */
void paint_title();
void paint_filed();
void paint_menu(WINDOW *menu_win, const char **menu, int width, int selected);
void paint_help();

/* menu functions: */
int do_menu(const char **menu);
int calc_menu_pos(const char **menu, int *y, int *x, int *height, int *width);

/* message box functions */
int message_box(char* message_text);

#endif /*_TERM_BOMBER_H*/
