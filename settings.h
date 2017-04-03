/********************************************************************************/
/*										*/
/* Terminal Bomber								*/
/* Text based minesweeper style game						*/
/* v 0.3 date 19/6/2010								*/
/*										*/
/* Copyright 2008, 2009, 2010 Lev Meirovitch					*/
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

#ifndef _SETTINGS_H
#define _SETTINGS_H

/* setting status */
#define SETTING_ON	1
#define SETTING_OFF	0

/* setting checks */
#define OPEN_FLAGGED	(SETTING_ON == settings.open_flagged)
#define CHEAT_MODE	(SETTING_ON == settings.cheat_mode)
#define USE_COLORS	(SETTING_ON == settings.colors_on)

/* setting defaults */
#define OPEN_FLAGGED_DEFAULT	SETTING_OFF
#define CHEAT_MODE_DEFAULT	SETTING_OFF
#define USE_COLORS_DEFAULT	SETTING_ON

/* settings structure */
typedef struct {
	unsigned char open_flagged; /* allow opening of flagged square */
	unsigned char cheat_mode; /* allow cheating by subtracting time for uncovered bombs */
	unsigned char colors_on; /* use colored display mode */
} settings_data;

extern settings_data settings; /* global variable to hold application settings */

/* functions for managing settings file */
int load_settings();
int save_settings();

/* settings screen functions */

void sttings_screen();
void paint_sttings(WINDOW *cur_win);

#endif /* _SETTINGS_H */