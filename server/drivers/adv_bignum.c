/** \file server/drivers/adv_bignum.c
 * Library to generate big numbers on displays with different numbers
 * of custom characters.
 */

/*-
 * Copyright (C) 2006 Stefan Herdler,
 *               2006 Peter Marschall
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 */

/** \file
	Requirements to the calling driver / display:
	\li \c cellwidth == 5 (also with 6, but with gaps)
	\li \c cellheight == 7 or 8
	\li Needed functions:
		\li \c drvthis->get_free_chars()
		\li \c drvthis->set_char() [only if get_free_chars() returns a value > 0]
		\li \c drvthis->chr()
		\li \c drvthis->height()
	\li Custom characters (if available) at char offset+0, offset+1, ..., offset+get_free_chars()-1

	USAGE:

	Just call \c lib_adv_bignum(drvthis, x, num, offset, do_init)
	from \c drvthis->num().
	The library does everything needed to show bignumbers EXCEPT checking or
	setting the CCMODE variable. The driver has to do this (see example
	below).

	Example: Call of the lib_adv_bignum from a driver's num function:
\code
	#include "adv_bignum.h"

	MODULE_EXPORT void
	serialVFD_num(Driver *drvthis, int x, int num)
	{
		PrivateData *p = drvthis->private_data;
		int do_init = 0;

		if (p->ccmode != CCMODE_BIGNUM) { // Are the customcharacters set up correctly? If not:
			do_init = 1;	// Lib_adv_bignum has to set the customcharacters.
			p->ccmode = CCMODE_BIGNUM; // Switch customcharactermode to bignum.
		}
		// Lib_adv_bignum does everything needed to show the bignumbers.
		lib_adv_bignum(drvthis, x, num, 0, do_init);
	}
\endcode
*/

#include "lcd.h"
#include "adv_bignum.h"

/* internal functions to set up bignumbers on 2- or 3-line displays */
static void adv_bignum_num_2_0(Driver * drvthis, int x, int num, int height, int offset, int do_init);
static void adv_bignum_num_2_1(Driver * drvthis, int x, int num, int height, int offset, int do_init);
static void adv_bignum_num_2_2(Driver * drvthis, int x, int num, int height, int offset, int do_init);
static void adv_bignum_num_2_5(Driver * drvthis, int x, int num, int height, int offset, int do_init);
static void adv_bignum_num_2_6(Driver * drvthis, int x, int num, int height, int offset, int do_init);
static void adv_bignum_num_2_28(Driver * drvthis, int x, int num, int height, int offset, int do_init);

/* internal functions to set up bignumbers on displays with at least 4 lines */
static void adv_bignum_num_4_0(Driver * drvthis, int x, int num, int height, int offset, int do_init);
static void adv_bignum_num_4_3(Driver * drvthis, int x, int num, int height, int offset, int do_init);
static void adv_bignum_num_4_8(Driver * drvthis, int x, int num, int height, int offset, int do_init);

/* internal function to write a bignumber to the display */
static void adv_bignum_write_num(Driver * drvthis, char num_map[][4][3], int x, int num, int height, int offset);


/**
 * Draw a big number to the display.
 *
 * This function determines the best possible bignumbertype for the display
 * depending on the display's height and the number of (available) custom
 * characters. It then calls an internal bignumber function to set up and draw
 * the desired number.
 *
 * \param drvthis  Pointer to driver structure.
 * \param x        Position at which the bignumber starts (leftmost column).
 * \param num      The number to draw (0..9 or 10 for colon ':').
 * \param offset   Offset at which custom characters can be placed in CGRAM.
 * \param do_init  Set to 1 for the library to set up custom chars, set to 0
 *                 if the driver already has set up the custom chars.
 */
void
lib_adv_bignum(Driver * drvthis, int x, int num, int offset, int do_init)
{
	int height = drvthis->height(drvthis);
	int customchars = drvthis->get_free_chars(drvthis);

	if (height >= 4) {
		height = 4;	/* not ideal: we always start at the 1st line */

		if (customchars == 0) {	/* 4 lines and no customchars */
			adv_bignum_num_4_0(drvthis, x, num, height, offset, do_init);
		}
		else if (customchars < 8) {	/* 4 lines and customchars < 8 */
			adv_bignum_num_4_3(drvthis, x, num, height, offset, do_init);
		}
		else {		/* 4 lines and customchars >= 8 */
			adv_bignum_num_4_8(drvthis, x, num, height, offset, do_init);
		}
	}
	else if (height >= 2) {
		height = 2;	/* Also for 3 line displays! */

		if (customchars == 0) {	/* 2 lines and no customchars */
			adv_bignum_num_2_0(drvthis, x, num, height, offset, do_init);
		}
		else if (customchars == 1) {	/* 2 lines and customchars = 1 */
			adv_bignum_num_2_1(drvthis, x, num, height, offset, do_init);
		}
		else if (customchars < 5) {	/* 2 lines and customchars = 2...4 */
			adv_bignum_num_2_2(drvthis, x, num, height, offset, do_init);
		}
		else if (customchars < 6) {	/* 2 lines and customchars = 5 */
			adv_bignum_num_2_5(drvthis, x, num, height, offset, do_init);
		}
		else if (customchars < 28) {	/* 2 lines and customchars = 6...27 */
			adv_bignum_num_2_6(drvthis, x, num, height, offset, do_init);
		}
		else {		/* 2 lines and customchars >= 28 */
			adv_bignum_num_2_28(drvthis, x, num, height, offset, do_init);
		}
	}

	return;
}

/**
 * Writes the selected type of bignumber by calling the driver's chr function.
 * It is called by display-depending bignumber functions to write the numbers.
 *
 * \param drvthis  Pointer to driver structure.
 * \param num_map  Array of 4x3 matrices describing of which (custom)
 *                 characters the bignumbers are composed of.
 * \param x        Position at which the bignumber starts (leftmost column).
 * \param num      The number to draw (0..9 or 10 for colon ':').
 * \param height   Height of the bignumber (currently 2 or 4)
 * \param offset   Offset at which custom characters can be placed in CGRAM.
 */
static void
adv_bignum_write_num(Driver * drvthis, char num_map[][4][3], int x, int num, int height, int offset)
{
	int y, dx;

	for (y = 0; y < height; y++) {
		if (num == 10) {/* ":" is only 1 character wide. */
			unsigned char c = num_map[num][y][0];

			/* increase c by offset if it is a user-defined character */
			if (c < ' ')
				c += offset;

			drvthis->chr(drvthis, x, y + 1, c);
		}
		else {
			for (dx = 0; dx < 3; dx++) {
				unsigned char c = num_map[num][y][dx];

				/* increase c by offset if it is a user-defined character */
				if (c < ' ')
					c += offset;

				drvthis->chr(drvthis, x + dx, y + 1, c);
			}
		}
	}
}


/*
 * #################  Display dependent functions. #################
 */

/**
 * This function is called for a 2 line display without custom characters.
 * (pretty ugly looking)
 */
static void
adv_bignum_num_2_0(Driver * drvthis, int x, int num, int height, int offset, int do_init)
{
	static char num_map[][4][3] = {	/* Defines the character placing inside the bignumber. */
		{		/* 0 */
			" ||",
			" ||",
			"   ",
			"   "
		},
		{		/* 1 */
			"  |",
			"  |",
			"   ",
			"   "
		},
		{		/* 2 */
			"  ]",
			" [ ",
			"   ",
			"   "
		},
		{		/* 3 */
			"  ]",
			"  ]",
			"   ",
			"   "
		},
		{		/* 4 */
			" L|",
			"  |",
			"   ",
			"   "
		},
		{		/* 5 */
			" [ ",
			"  ]",
			"   ",
			"   "
		},
		{		/* 6 */
			" [ ",
			" []",
			"   ",
			"   "
		},
		{		/* 7 */
			"  7",
			"  |",
			"   ",
			"   "
		},
		{		/* 8 */
			" []",
			" []",
			"   ",
			"   "
		},
		{		/* 9 */
			" []",
			"  ]",
			"   ",
			"   "
		},
		{		/* colon */
			".",
			".",
			" ",
			" "
		}
	};

	/* write the number */
	adv_bignum_write_num(drvthis, num_map, x, num, height, offset);
}


/**
 * This function is usable for a 2 line display with 1 custom character.
 * (not a beauty, but useable)
 */
static void
adv_bignum_num_2_1(Driver * drvthis, int x, int num, int height, int offset, int do_init)
{
	static char num_map[][4][3] = {	/* Defines the character placing inside the bignumber. */
		{		/* 0 */
			{'|', 0, '|'},
			{"|_|"},
			{"   "},
			{"   "}
		},
		{		/* 1 */
			{"  |"},
			{"  |"},
			{"   "},
			{"   "}
		},
		{		/* 2 */
			{' ', 0, ']'},
			{" [_"},
			{"   "},
			{"   "}
		},
		{		/* 3 */
			{' ', 0, ']'},
			{" _]"},
			{"   "},
			{"   "}
		},
		{		/* 4 */
			{" L|"},
			{"  |"},
			{"   "},
			{"   "}
		},
		{		/* 5 */
			{' ', '[', 0},
			{" _]"},
			{"   "},
			{"   "}
		},
		{		/* 6 */
			{' ', '[', 0},
			{" []"},
			{"   "},
			{"   "}
		},
		{		/* 7 */
			{' ', 0, '|'},
			{"  |"},
			{"   "},
			{"   "}
		},
		{		/* 8 */
			{" []"},
			{" []"},
			{"   "},
			{"   "}
		},
		{		/* 9 */
			{" []"},
			{" _]"},
			{"   "},
			{"   "}
		},
		{		/* colon */
			{"."},
			{"."},
			{" "},
			{" "}
		}
	};

	/* Set customcharacters if needed. */
	if (do_init) {
		static unsigned char bignum[1][8] = {	/* stored customcharacter */
			[0] = {
				b__XXXXX,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
			}
		};
		drvthis->set_char(drvthis, offset + 0, bignum[0]);
	}

	/* write the number */
	adv_bignum_write_num(drvthis, num_map, x, num, height, offset);
}


/**
 * This function is usable for a 2 line display with at least 2 custom characters.
 * (o.k.)
 */
static void
adv_bignum_num_2_2(Driver * drvthis, int x, int num, int height, int offset, int do_init)
{
	static char num_map[][4][3] = {	/* Defines the character placing inside the bignumber. */
		{		/* 0 */
			{'|', 0, '|'},
			"|_|",
			"   ",
			"   "
		},
		{		/* 1 */
			"  |",
			"  |",
			"   ",
			"   "
		},
		{		/* 2 */
			{' ', 1, '|'},
			"|_ ",
			"   ",
			"   "
		},
		{		/* 3 */
			{' ', 1, '|'},
			" _|",
			"   ",
			"   "
		},
		{		/* 4 */
			"|_|",
			"  |",
			"   ",
			"   "
		},
		{		/* 5 */
			{'|', 1, ' '},
			" _|",
			"   ",
			"   "
		},
		{		/* 6 */
			{'|', 0, ' '},
			{'|', 1, '|'},
			"   ",
			"   "
		},
		{		/* 7 */
			{' ', 0, '|'},
			"  |",
			"   ",
			"   "
		},
		{		/* 8 */
			{'|', 1, '|'},
			"|_|",
			"   ",
			"   "
		},
		{		/* 9 */
			{'|', 1, '|'},
			" _|",
			"   ",
			"   "
		},
		{		/* colon */
			".",
			".",
			" ",
			" "
		}
	};

	/* Set customcharacters if needed. */
	if (do_init) {
		int i;
		static unsigned char bignum[2][8] = {	/* stored customcharacter */
			[0] = {
				b__XXXXX,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
			},
			[1] = {
				b__XXXXX,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
				b__XXXXX,
				b__XXXXX,
			}
		};

		/* put the customcharacters into the display */
		for (i = 0; i < 2; i++) {
			drvthis->set_char(drvthis, offset + i, bignum[i]);
		}
	}

	/* write the number */
	adv_bignum_write_num(drvthis, num_map, x, num, height, offset);
}


/**
 * This function is usable for a 2 line display with at least 5 characters.
 * (nice bignumbers)
 */
static void
adv_bignum_num_2_5(Driver * drvthis, int x, int num, int height, int offset, int do_init)
{
	static char num_map[][4][3] = {	/* Defines the character placing inside the bignumber. */
		{{3, 0, 2},	/* 0 */
		{3, 1, 2},
		{"   "},
		{"   "}},
		{{' ', ' ', 2},	/* 1 */
		{' ', ' ', 2},
		{"   "},
		{"   "}},
		{{' ', 4, 2},	/* 2 */
		{3, 1, ' '},
		{"   "},
		{"   "}},
		{{' ', 4, 2},	/* 3 */
		{' ', 1, 2},
		{"   "},
		{"   "}},
		{{3, 1, 2},	/* 4 */
		{' ', ' ', 2},
		{"   "},
		{"   "}},
		{{3, 4, ' '},	/* 5 */
		{' ', 1, 2},
		{"   "},
		{"   "}},
		{{3, 0, ' '},	/* 6 */
		{3, 4, 2},
		{"   "},
		{"   "}},
		{{' ', 0, 2},	/* 7 */
		{' ', ' ', 2},
		{"   "},
		{"   "}},
		{{3, 4, 2},	/* 8 */
		{3, 1, 2},
		{"   "},
		{"   "}},
		{{3, 4, 2},	/* 9 */
		{' ', 1, 2},
		{"   "},
		{"   "}},
		{{'.'},		/* : */
		{'.'},
		{"   "},
		{"   "}
		}
	};

	/* Set customcharacters if needed. */
	if (do_init) {
		int i;
		static unsigned char bignum[5][8] = {	/* stored customcharacter */
			[0] = {
				b__XXXXX,
				b__XXXXX,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
			},
			[1] = {
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
				b__XXXXX,
				b__XXXXX,
				b__XXXXX,
			},
			[2] = {
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
			},
			[3] = {
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
			},
			[4] = {
				b__XXXXX,
				b__XXXXX,
				b_______,
				b_______,
				b_______,
				b__XXXXX,
				b__XXXXX,
				b__XXXXX,
			}
		};

		/* put the customcharacters into the display */
		for (i = 0; i < 5; i++) {
			drvthis->set_char(drvthis, offset + i, bignum[i]);
		}
	}

	/* write the number */
	adv_bignum_write_num(drvthis, num_map, x, num, height, offset);
}

/**
 * This function is usable for a 2 line display with at least 6 custom characters.
 * (nice bignumbers)
 */
static void
adv_bignum_num_2_6(Driver * drvthis, int x, int num, int height, int offset, int do_init)
{
	static char num_map[][4][3] = {	/* Defines the character placing inside the bignumber. */
		{{3, 0, 2},	/* 0 */
		{3, 1, 2},
		{"   "},
		{"   "}},
		{{' ', ' ', 2},	/* 1 */
		{' ', ' ', 2},
		{"   "},
		{"   "}},
		{{' ', 5, 2},	/* 2 */
		{3, 4, ' '},
		{"   "},
		{"   "}},
		{{' ', 5, 2},	/* 3 */
		{' ', 4, 2},
		{"   "},
		{"   "}},
		{{3, 1, 2},	/* 4 */
		{' ', ' ', 2},
		{"   "},
		{"   "}},
		{{3, 5, ' '},	/* 5 */
		{' ', 4, 2},
		{"   "},
		{"   "}},
		{{3, 5, ' '},	/* 6 */
		{3, 4, 2},
		{"   "},
		{"   "}},
		{{' ', 0, 2},	/* 7 */
		{' ', ' ', 2},
		{"   "},
		{"   "}},
		{{3, 5, 2},	/* 8 */
		{3, 4, 2},
		{"   "},
		{"   "}},
		{{3, 5, 2},	/* 9 */
		{' ', 1, 2},
		{"   "},
		{"   "}},
		{{'.'},		/* : */
		{'.'},
		{"   "},
		{"   "}}
	};

	/* Set customcharacters if needed. */
	if (do_init) {
		int i;
		static unsigned char bignum[6][8] = {	/* stored customcharacter */
			[0] = {
				b__XXXXX,
				b__XXXXX,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
			},
			[1] = {
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
				b__XXXXX,
				b__XXXXX,
				b__XXXXX,
			},
			[2] = {
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
			},
			[3] = {
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
			},
			[4] = {
				b__XXXXX,
				b_______,
				b_______,
				b_______,
				b_______,
				b__XXXXX,
				b__XXXXX,
				b__XXXXX,
			},
			[5] = {
				b__XXXXX,
				b__XXXXX,
				b_______,
				b_______,
				b_______,
				b_______,
				b__XXXXX,
				b__XXXXX,
			},
		};

		/* put the customcharacters into the display */
		for (i = 0; i < 6; i++) {
			drvthis->set_char(drvthis, offset + i, bignum[i]);
		}
	}

	/* write the number */
	adv_bignum_write_num(drvthis, num_map, x, num, height, offset);
}


/**
 * This function is usable for a 2 line display with 28 or more custom characters.
 * (Wow, allmost graphical)
 */
static void
adv_bignum_num_2_28(Driver * drvthis, int x, int num, int height, int offset, int do_init)
{
	static char num_map[][4][3] = {	/* Defines the character placing inside the bignumber. */
		{{15, 6, 2},	/* 0 */
		{14, 4, 5},
		{"   "},
		{"   "}},
		{{' ', 26, ' '},/* 1 */
		{' ', 10, ' '},
		{"   "},
		{"   "}},
		{{1, 6, 2},	/* 2 */
		{7, 8, 9},
		{"   "},
		{"   "}},
		{{0, 11, 2},	/* 3 */
		{3, 13, 5},
		{"   "},
		{"   "}},
		{{25, 21, 23},	/* 4 */
		{17, 22, 24},
		{"   "},
		{"   "}},
		{{10, 11, 12},	/* 5 */
		{3, 13, 5},
		{"   "},
		{"   "}},
		{{15, 11, 16},	/* 6 */
		{14, 13, 5},
		{"   "},
		{"   "}},
		{{17, 18, 19},	/* 7 */
		{' ', 20, ' '},
		{"   "},
		{"   "}},
		{{15, 11, 2},	/* 8 */
		{14, 13, 5},
		{"   "},
		{"   "}},
		{{15, 11, 2},	/* 9 */
		{3, 13, 5},
		{"   "},
		{"   "}},
		{{27},		/* : */
		{27},
		{"   "},
	{"   "}}};

	/* Set customcharacters if needed. */
	if (do_init) {
		int i;
		static unsigned char bignum[28][8] = {	/* stored customcharacter */
			[0] = {
				b_____XX,
				b____XXX,
				b____XXX,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
			},
			[1] = {
				b_____XX,
				b____XXX,
				b____XXX,
				b____XXX,
				b_______,
				b_______,
				b_______,
				b_______,
			},
			[2] = {
				b__XX___,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
			},
			[3] = {
				b_______,
				b_______,
				b_______,
				b_______,
				b____XXX,
				b____XXX,
				b_____XX,
				b_____XX,
			},
			[4] = {
				b_______,
				b_______,
				b_______,
				b_______,
				b__XXXXX,
				b__XXXXX,
				b__XXXXX,
				b__XXXXX,
			},
			[5] = {
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XX___,
				b__X____,
			},
			[6] = {
				b__XXXXX,
				b__XXXXX,
				b__XXXXX,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
			},
			[7] = {
				b_______,
				b_______,
				b_______,
				b______X,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
			},
			[8] = {
				b____XXX,
				b___XXXX,
				b__XXXX_,
				b__XXX__,
				b__XXXXX,
				b__XXXXX,
				b__XXXXX,
				b__XXXXX,
			},
			[9] = {
				b__X____,
				b_______,
				b_______,
				b_______,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
			},
			[10] = {
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
			},
			[11] = {
				b__XXXXX,
				b__XXXXX,
				b__XXXXX,
				b_______,
				b_______,
				b_______,
				b__XXXXX,
				b__XXXXX,
			},
			[12] = {
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
			},
			[13] = {
				b__XXXXX,
				b_______,
				b_______,
				b_______,
				b__XXXXX,
				b__XXXXX,
				b__XXXXX,
				b__XXXXX,
			},
			[14] = {
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b_____XX,
				b_____XX,
			},
			[15] = {
				b_____XX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
			},
			[16] = {
				b__XX___,
				b__XXX__,
				b__XXX__,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
			},
			[17] = {
				b____XXX,
				b____XXX,
				b____XXX,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
			},
			[18] = {
				b__XXXXX,
				b__XXXXX,
				b__XXXXX,
				b_______,
				b_____XX,
				b_____XX,
				b____XXX,
				b____XXX,
			},
			[19] = {
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XX___,
				b__X____,
				b_______,
				b_______,
			},
			[20] = {
				b___XXX_,
				b___XXX_,
				b__XXXX_,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__XXX__,
			},
			[21] = {
				b______X,
				b_____XX,
				b____XXX,
				b___XXXX,
				b__XXXXX,
				b__XXX_X,
				b__XX__X,
				b__XX__X,
			},
			[22] = {
				b__XXXXX,
				b__XXXXX,
				b__XXXXX,
				b______X,
				b______X,
				b______X,
				b______X,
				b______X,
			},
			[23] = {
				b__X____,
				b__X____,
				b__X____,
				b__X____,
				b__X____,
				b__X____,
				b__X____,
				b__X____,
			},
			[24] = {
				b__XXX__,
				b__XXX__,
				b__XXX__,
				b__X____,
				b__X____,
				b__X____,
				b__X____,
				b__X____,
			},
			[25] = {
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
				b______X,
				b______X,
			},
			[26] = {
				b____XXX,
				b____XXX,
				b___XXXX,
				b__XXXXX,
				b____XXX,
				b____XXX,
				b____XXX,
				b____XXX,
			},
			[27] = {
				b_______,
				b_______,
				b_______,
				b____XX_,
				b____XX_,
				b_______,
				b_______,
				b_______,
			}
		};

		/* put the customcharacters into the display */
		for (i = 0; i < 28; i++) {
			drvthis->set_char(drvthis, offset + i, bignum[i]);
		}
	}

	/* write the number */
	adv_bignum_write_num(drvthis, num_map, x, num, height, offset);
}


/**
 * This function is usable for a 4 line display without custom characters.
 * (o.k.)
 */
static void
adv_bignum_num_4_0(Driver * drvthis, int x, int num, int height, int offset, int do_init)
{
	/* Ugly code extracted by David GLAUDE from lcdm001.c ;) */
	/* Moved to driver.c by Joris Robijn */
	/* Moved to adv_bignum.c by Stefan Herdler */
	static char num_map[][4][3] = {	/* Defines the character placing inside the bignumber. */
		{		/* 0 */
			" _ ",
			"| |",
			"|_|",
			"   "
		},
		{		/* 1 */
			"   ",
			"  |",
			"  |",
			"   "
		},
		{		/* 2 */
			" _ ",
			" _|",
			"|_ ",
			"   "
		},
		{		/* 3 */
			" _ ",
			" _|",
			" _|",
			"   "
		},
		{		/* 4 */
			"   ",
			"|_|",
			"  |",
			"   "
		},
		{		/* 5 */
			" _ ",
			"|_ ",
			" _|",
			"   "
		},
		{		/* 6 */
			" _ ",
			"|_ ",
			"|_|",
			"   "
		},
		{		/* 7 */
			" _ ",
			"  |",
			"  |",
			"   "
		},
		{		/* 8 */
			" _ ",
			"|_|",
			"|_|",
			"   "
		},
		{		/* 9 */
			" _ ",
			"|_|",
			" _|",
			"   "
		},
		{		/* colon */
			" ",
			".",
			".",
			" "
		}
	};

	/* write the number */
	adv_bignum_write_num(drvthis, num_map, x, num, height, offset);
}


/**
 * This function is usable for a 4 line display with at least 3 custom characters.
 * (nice bignumbers)
 */
static void
adv_bignum_num_4_3(Driver * drvthis, int x, int num, int height, int offset, int do_init)
{
	static char num_map[][4][3] = {	/* Defines the character placing inside the bignumber. */
		{{3, 1, 3},	/* 0 */
		{3, ' ', 3},
		{3, ' ', 3},
		{3, 2, 3}},
		{{' ', ' ', 3},	/* 1 */
		{' ', ' ', 3},
		{' ', ' ', 3},
		{' ', ' ', 3}},
		{{' ', 1, 3},	/* 2 */
		{' ', 2, 3},
		{3, ' ', ' '},
		{3, 2, ' '}},
		{{' ', 1, 3},	/* 3 */
		{' ', 2, 3},
		{' ', ' ', 3},
		{' ', 2, 3}},
		{{3, ' ', 3},	/* 4 */
		{3, 2, 3},
		{' ', ' ', 3},
		{' ', ' ', 3}},
		{{3, 1, ' '},	/* 5 */
		{3, 2, ' '},
		{' ', ' ', 3},
		{' ', 2, 3}},
		{{3, 1, ' '},	/* 6 */
		{3, 2, ' '},
		{3, ' ', 3},
		{3, 2, 3}},
		{{' ', 1, 3},	/* 7 */
		{' ', ' ', 3},
		{' ', ' ', 3},
		{' ', ' ', 3}},
		{{3, 1, 3},	/* 8 */
		{3, 2, 3},
		{3, ' ', 3},
		{3, 2, 3}},
		{{3, 1, 3},	/* 9 */
		{3, 2, 3},
		{' ', ' ', 3},
		{' ', 2, 3}},
		{{" "},		/* : */
		{'.'},
		{'.'},
		{" "}}
	};

	/* Set customcharacters if needed. */
	if (do_init) {
		int i;
		static unsigned char bignum[3][8] = {	/* stored customcharacter */
			[0] = {
				b__XXXXX,
				b__XXXXX,
				b__XXXXX,
				b_______,
				b_______,
				b_______,
				b_______,
				b_______,
			},
			[1] = {
				b_______,
				b_______,
				b_______,
				b_______,
				b__XXXXX,
				b__XXXXX,
				b__XXXXX,
				b__XXXXX,
			},
			[2] = {
				b___XXX_,
				b___XXX_,
				b___XXX_,
				b___XXX_,
				b___XXX_,
				b___XXX_,
				b___XXX_,
				b___XXX_,
			}
		};

		/* put the customcharacters into the display */
		for (i = 0; i < 3; i++) {
			drvthis->set_char(drvthis, offset + i + 1, bignum[i]);
		}
	}

	/* write the number */
	adv_bignum_write_num(drvthis, num_map, x, num, height, offset);
}


/**
 * This function is usable for a 4 line display with 8 or more custom characters.
 * (nice bignumbers)
 */
static void
adv_bignum_num_4_8(Driver * drvthis, int x, int num, int height, int offset, int do_init)
{
	static char num_map[][4][3] = {	/* Defines the character placing inside the bignumber. */
		{{1, 2, 3},	/* 0 */
		{6, 32, 6},
		{6, 32, 6},
		{7, 2, 32}},
		{{7, 6, 32},	/* 1 */
		{32, 6, 32},
		{32, 6, 32},
		{7, 2, 32}},
		{{1, 2, 3},	/* 2 */
		{32, 5, 0},
		{1, 32, 32},
		{2, 2, 0}},
		{{1, 2, 3},	/* 3 */
		{32, 5, 0},
		{3, 32, 6},
		{7, 2, 32}},
		{{32, 3, 6},	/* 4 */
		{1, 32, 6},
		{2, 2, 6},
		{32, 32, 0}},
		{{1, 2, 0},	/* 5 */
		{2, 2, 3},
		{3, 32, 6},
		{7, 2, 32}},
		{{1, 2, 32},	/* 6 */
		{6, 5, 32},
		{6, 32, 6},
		{7, 2, 32}},
		{{2, 2, 6},	/* 7 */
		{32, 1, 32},
		{32, 6, 32},
		{32, 0, 32}},
		{{1, 2, 3},	/* 8 */
		{4, 5, 0},
		{6, 32, 6},
		{7, 2, 32}},
		{{1, 2, 3},	/* 9 */
		{4, 3, 6},
		{32, 1, 32},
		{7, 32, 32}},
		{{32, 32, 32},	/* colon (only 1st column used) */
		{0, 32, 32},
		{0, 32, 32},
		{32, 32, 32}}
	};

	/* Set customcharacters if needed. */
	if (do_init) {
		int i;
		static unsigned char bignum[8][8] = {	/* stored customcharacter */
			[0] = {
				b__XX___,
				b__XX___,
				b__XX___,
				b__XX___,
				b_______,
				b_______,
				b_______,
				b_______,
			},
			[1] = {
				b_____XX,
				b_____XX,
				b_____XX,
				b_____XX,
				b__XX___,
				b__XX___,
				b__XX___,
				b__XX___,
			},
			[2] = {
				b__XX_XX,
				b__XX_XX,
				b__XX_XX,
				b__XX_XX,
				b_______,
				b_______,
				b_______,
				b_______,
			},
			[3] = {
				b_______,
				b_______,
				b_______,
				b_______,
				b__XX___,
				b__XX___,
				b__XX___,
				b__XX___,
			},
			[4] = {
				b__XX___,
				b__XX___,
				b__XX___,
				b__XX___,
				b_____XX,
				b_____XX,
				b_____XX,
				b_____XX,
			},
			[5] = {
				b_______,
				b_______,
				b_______,
				b_______,
				b__XX_XX,
				b__XX_XX,
				b__XX_XX,
				b__XX_XX,
			},
			[6] = {
				b__XX___,
				b__XX___,
				b__XX___,
				b__XX___,
				b__XX___,
				b__XX___,
				b__XX___,
				b__XX___,
			},
			[7] = {
				b_____XX,
				b_____XX,
				b_____XX,
				b_____XX,
				b_______,
				b_______,
				b_______,
				b_______,
			}
		};

		/* put the customcharacters into the display */
		for (i = 0; i < 8; i++) {
			drvthis->set_char(drvthis, offset + i, bignum[i]);
		}
	}

	/* write the number */
	adv_bignum_write_num(drvthis, num_map, x, num, height, offset);
}
