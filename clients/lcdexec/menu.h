/** \file clients/lcdexec/menu.h
 * Declare constants, data types and functions for menu functions in clients/lcdexec/menu.c.
 */

/* This file is part of lcdexec, an LCDproc client.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2002, Joris Robijn
 * Copyright (c) 2006-7, Peter Marschall
 */

#ifndef LCDEXEC_MENU_H
#define LCDEXEC_MENU_H

/* boolean values */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#include <kdbtypes.h>

typedef enum
{
	MENU_ENTRY_TYPE_COMMAND = 0,
	MENU_ENTRY_TYPE_MENU = 1,
} MenuEntryType;

typedef enum
{
	COMMAND_PARAMETER_TYPE_SLIDER = 0,
	COMMAND_PARAMETER_TYPE_CHECKBOX = 1,
	COMMAND_PARAMETER_TYPE_NUMERIC = 2,
	COMMAND_PARAMETER_TYPE_RING = 3,
	COMMAND_PARAMETER_TYPE_ALPHA = 4,
	COMMAND_PARAMETER_TYPE_IP = 5,
} CommandParameterType;

typedef enum
{
	CHECKBOX_STATE_OFF = 0,
	CHECKBOX_STATE_ON = 1,
	CHECKBOX_STATE_GRAY = 2,
} CheckboxState;

typedef union
{
	struct SliderParameter * slider;
	struct CheckboxParameter * checkbox;
	struct NumericParameter * numeric;
	struct RingParameter * ring;
	struct AlphaParameter * alpha;
	struct IpParameter * ip;
} CommandParameter;

typedef union
{
	struct Command * command;
	struct Menu * menu;
} MenuEntry;

typedef struct Command
{
	kdb_long_t id;
	kdb_long_t actionId;
	const char *displayname;
	const char *exec;
	kdb_boolean_t feedback;
	kdb_long_long_t parametersSize;
	CommandParameterType *parameterTypes;
	CommandParameter *parameters;
} Command;

typedef struct Menu
{
	kdb_long_t id;
	const char *displayname;
	kdb_long_long_t entriesSize;
	MenuEntryType *entryTypes;
	MenuEntry *entries;
} Menu;

typedef struct AlphaParameter
{
	kdb_long_t id;
	const char *displayname;
	const char *envname;
	const char *allowedchars;
	kdb_long_t minlength;
	kdb_long_t maxlength;
	const char *initialValue;
	char *value;
} AlphaParameter;

typedef struct CheckboxParameter
{
	kdb_long_t id;
	const char *displayname;
	const char *envname;
	kdb_boolean_t allowgray;
	const char *offtext;
	const char *ontext;
	const char *graytext;
	CheckboxState value;
} CheckboxParameter;

typedef struct IpParameter
{
	kdb_long_t id;
	const char *displayname;
	const char *envname;
	kdb_boolean_t v6;
	const char *initialValue;
	char *value;
} IpParameter;

typedef struct NumericParameter
{
	kdb_long_t id;
	const char *displayname;
	const char *envname;
	kdb_long_t minvalue;
	kdb_long_t maxvalue;
	kdb_long_t value;
} NumericParameter;

typedef struct RingParameter
{
	kdb_long_t id;
	const char *displayname;
	const char *envname;
	kdb_long_long_t stringsCount;
	const char **strings;
	size_t stringsLength;
	kdb_long_t value;
} RingParameter;

typedef struct SliderParameter
{
	kdb_long_t id;
	const char *displayname;
	const char *envname;
	const char *mintext;
	const char *maxtext;
	kdb_long_t minvalue;
	kdb_long_t maxvalue;
	kdb_long_t stepsize;
	kdb_long_t value;
} SliderParameter;

int main_menu_sock_send(Menu *menu, int sock);
void main_menu_process(Menu *menu);
void menu_dump(Menu *menu, int level);

#endif
