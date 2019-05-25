/** \file clients/lcdexec/menu.c
 * Menu parsing and building functions for the \c lcdexec client
 */

/* This file is part of lcdexec, an LCDproc client.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright (c) 2002, Joris Robijn
 * Copyright (c) 2006-7, Peter Marschall
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "shared/report.h"
#include "shared/configfile.h"
#include "shared/sockets.h"

#include "menu.h"
#include "elektragen.h"

#define SET_PARAMETER_DISPLAY_NAME(p)    \
	do                                   \
	{                                    \
		if (strlen(p->displayname) == 0) \
			p->displayname = p->envname; \
	} while (0)

static int menu_sock_send(Menu *menu, Menu *parent, int sock);
static int menu_entries_sock_send(Menu *menu, int sock);
static int command_sock_send(Command *command, Menu *parent, int sock);

static int id = 0;

static void command_process(Command *command)
{
	command->id = id++;

	for (kdb_long_long_t i = 0; i < command->parametersSize; ++i)
	{
		CommandParameter parameter = command->parameters[i];
		switch (command->parameterTypes[i])
		{
		case COMMAND_PARAMETER_TYPE_NUMERIC:
			parameter.numeric->id = id++;
			SET_PARAMETER_DISPLAY_NAME(parameter.numeric);
			break;
		case COMMAND_PARAMETER_TYPE_RING:
			parameter.ring->id = id++;
			SET_PARAMETER_DISPLAY_NAME(parameter.ring);
			break;
		case COMMAND_PARAMETER_TYPE_SLIDER:
			parameter.slider->id = id++;
			SET_PARAMETER_DISPLAY_NAME(parameter.slider);
			break;
		case COMMAND_PARAMETER_TYPE_CHECKBOX:
			parameter.checkbox->id = id++;
			SET_PARAMETER_DISPLAY_NAME(parameter.checkbox);
			break;
		case COMMAND_PARAMETER_TYPE_ALPHA:
		{
			parameter.alpha->id = id++;
			SET_PARAMETER_DISPLAY_NAME(parameter.alpha);
			parameter.alpha->value = calloc(strlen(parameter.alpha->initialValue) + 1, sizeof(char));
			strcpy(parameter.alpha->value, parameter.alpha->initialValue);
		}
		break;
		case COMMAND_PARAMETER_TYPE_IP:
		{
			parameter.ip->id = id++;
			SET_PARAMETER_DISPLAY_NAME(parameter.ip);
			parameter.ip->value = calloc(strlen(parameter.ip->initialValue) + 1, sizeof(char));
			strcpy(parameter.ip->value, parameter.ip->initialValue);
		}
		break;
		}
	}
}

static void menu_process(Menu *menu)
{
	menu->id = id++;
	for (kdb_long_long_t i = 0; i < menu->entriesSize; i++)
	{
		switch (menu->entryTypes[i])
		{
		case MENU_ENTRY_TYPE_MENU:
			menu_process(menu->entries[i].menu);
			break;
		case MENU_ENTRY_TYPE_COMMAND:
			command_process(menu->entries[i].command);
			break;
		}
	}
}

void main_menu_process(Menu *menu)
{
	id = 0;
	menu_process(menu);
}

int main_menu_sock_send(Menu *menu, int sock)
{
	// don't create a separate entry for the main menu
	return menu_entries_sock_send(menu, sock);
}

static int menu_sock_send(Menu *menu, Menu *parent, int sock)
{
	char parent_id[ELEKTRA_MAX_ARRAY_SIZE] = ""; // more than big enough
	if (parent != NULL && parent->id > 0) {
		snprintf(parent_id, ELEKTRA_MAX_ARRAY_SIZE-1, "%d", parent->id);
	}

	// create entry for the menu
	if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" menu \"%s\"\n",
					parent_id, menu->id, menu->displayname) < 0)
		return -1;

	return menu_entries_sock_send(menu, sock);
}

static int menu_entries_sock_send(Menu *menu, int sock)
{
	for (kdb_long_long_t i = 0; i < menu->entriesSize; i++)
	{
		switch (menu->entryTypes[i])
		{
		case MENU_ENTRY_TYPE_MENU:
			if (menu_sock_send(menu->entries[i].menu, menu, sock) < 0)
				return -1;
			break;
		case MENU_ENTRY_TYPE_COMMAND:
			if (command_sock_send(menu->entries[i].command, menu, sock) < 0)
				return -1;
			break;
		}
	}

	return 0;
}

static int command_sock_send(Command *command, Menu *parent, int sock)
{
	char parent_id[ELEKTRA_MAX_ARRAY_SIZE] = ""; // more than big enough
	if (parent != NULL && parent->id > 0) {
		snprintf(parent_id, ELEKTRA_MAX_ARRAY_SIZE-1, "%d", parent->id);
	}

	if (command->parametersSize == 0)
	{
		if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" action \"%s\"\n",
						parent_id, command->id, command->displayname) < 0)
			return -1;

		if (sock_printf(sock, "menu_set_item \"%s\" \"%d\" -menu_result quit\n",
						parent_id, command->id) < 0)
			return -1;
	}
	else
	{
		if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" menu \"%s\"\n",
						parent_id, command->id, command->displayname) < 0)
			return -1;

		kdb_long_t lastId = 0;
		for (kdb_long_long_t i = 0; i < command->parametersSize; i++)
		{
			CommandParameter param = command->parameters[i];
			switch (command->parameterTypes[i])
			{
			case COMMAND_PARAMETER_TYPE_SLIDER:
				lastId = param.slider->id;
				if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" slider -text \"%s\""
									  " -value %d -minvalue %d -maxvalue %d"
									  " -mintext \"%s\" -maxtext \"%s\" -stepsize %d\n",
								parent_id, param.slider->id, param.slider->displayname,
								param.slider->value,
								param.slider->minvalue,
								param.slider->maxvalue,
								param.slider->mintext,
								param.slider->maxtext,
								param.slider->stepsize) < 0)
					return -1;
				break;
			case COMMAND_PARAMETER_TYPE_CHECKBOX:
			{
				lastId = param.checkbox->id;
				const char *strValue = ELEKTRA_TO_CONST_STRING(EnumCheckboxState)(param.checkbox->value);
				int ret = sock_printf(sock, "menu_add_item \"%s\" \"%d\" checkbox -text \"%s\""
											" -value %s -allow_gray %s\n",
									  parent_id, param.checkbox->id, param.checkbox->displayname,
									  strValue, param.checkbox->allowgray ? "true" : "false");
				if (ret < 0)
					return -1;

				break;
			}
			case COMMAND_PARAMETER_TYPE_NUMERIC:
				lastId = param.numeric->id;
				if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" numeric -text \"%s\""
									  " -value %d -minvalue %d -maxvalue %d\n",
								parent_id, param.numeric->id, param.numeric->displayname,
								param.numeric->value,
								param.numeric->minvalue,
								param.numeric->maxvalue) < 0)
					return -1;
				break;
			case COMMAND_PARAMETER_TYPE_RING:
			{
				lastId = param.ring->id;
				char *strings = calloc(1, sizeof(char));
				size_t stringsLen = 0;

				for (kdb_long_long_t j = 0; j < param.ring->stringsSize; j++)
				{
					if (param.ring->strings[i] != NULL)
					{
						size_t paramLen = strlen(param.ring->strings[i]);
						strings = realloc(strings, (stringsLen + 1 + paramLen + 1) * sizeof(char));
						if (stringsLen > 0)
						{
							strings[stringsLen] = '\t';
						}
						strcpy(&strings[stringsLen + 1], param.ring->strings[i]);
						stringsLen += paramLen + 1;
					}
				}
				strings[stringsLen] = '\0';

				if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" ring -text \"%s\""
									  " -value %d -strings \"%s\"\n",
								parent_id, param.ring->id, param.ring->displayname,
								param.ring->value, strings) < 0)
					return -1;

				break;
			}
			case COMMAND_PARAMETER_TYPE_ALPHA:
				lastId = param.alpha->id;
				if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" alpha -text \"%s\""
									  " -value \"%s\" -minlength %d -maxlength %d"
									  " -allow_caps false -allow_noncaps false"
									  " -allow_numbers false -allowed_extra \"%s\"\n",
								parent_id, param.alpha->id, param.alpha->displayname,
								param.alpha->value,
								param.alpha->minlength,
								param.alpha->maxlength,
								param.alpha->allowedchars) < 0)
					return -1;
				break;
			case COMMAND_PARAMETER_TYPE_IP:
				lastId = param.ip->id;
				if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" ip -text \"%s\""
									  " -value \"%s\" -v6 %s\n",
								parent_id, param.ip->id, param.ip->displayname,
								param.ip->value,
								param.ip->v6 ? "true" : "false") < 0)
					return -1;
				break;
			}
		}

		if (sock_printf(sock, "menu_set_item \"%s\" \"%d\" -next _quit_\n",
						parent_id, lastId) < 0)
			return -1;

		if (sock_printf(sock, "menu_add_item \"%s\" \"%d\" action \"%s\"\n",
						parent_id, command->actionId, "Apply!") < 0)
			return -1;

		if (sock_printf(sock, "menu_set_item \"%s\" \"%d\" -menu_result quit\n",
						parent_id, command->actionId) < 0)
			return -1;
	}
	return 0;
}

#if defined(DEBUG)
static void command_dump(Command *command, int level)
{
	if (command == NULL)
	{
		return;
	}

	report(RPT_DEBUG, "%*sCommand("ELEKTRA_LONG_F") {\n", level, "", command->id);
	report(RPT_DEBUG, "%*sdisplayname = \"%s\"\n", level + 1, "", command->displayname);
	report(RPT_DEBUG, "%*sexec = \"%s\"\n", level + 1, "", command->exec);
	report(RPT_DEBUG, "%*sfeedback = %s\n", level + 1, "", command->feedback ? "true" : "false");
	report(RPT_DEBUG, "%*sparameters = [\n", level + 1, "");

	for (kdb_long_long_t i = 0; i < command->parametersSize; ++i)
	{
		CommandParameter parameter = command->parameters[i];
		switch (command->parameterTypes[i])
		{
		case COMMAND_PARAMETER_TYPE_NUMERIC:
			report(RPT_DEBUG, "%*sNumeric("ELEKTRA_LONG_F") {\n", level + 2, "", parameter.numeric->id);
			report(RPT_DEBUG, "%*sdisplayname = \"%s\"\n", level + 3, "", parameter.numeric->displayname);
			report(RPT_DEBUG, "%*senvname = \"%s\"\n", level + 3, "", parameter.numeric->envname);
			report(RPT_DEBUG, "%*sminvalue = " ELEKTRA_LONG_F "\n", level + 3, "", parameter.numeric->minvalue);
			report(RPT_DEBUG, "%*smaxvalue = " ELEKTRA_LONG_F "\n", level + 3, "", parameter.numeric->maxvalue);
			report(RPT_DEBUG, "%*svalue = " ELEKTRA_LONG_F "\n", level + 3, "", parameter.numeric->value);
			report(RPT_DEBUG, "%*s}\n", level + 2, "");
			break;
		case COMMAND_PARAMETER_TYPE_RING:
			report(RPT_DEBUG, "%*sRing("ELEKTRA_LONG_F") {\n", level + 2, "", parameter.ip->id);
			report(RPT_DEBUG, "%*sdisplayname = \"%s\"\n", level + 3, "", parameter.ring->displayname);
			report(RPT_DEBUG, "%*senvname = \"%s\"\n", level + 3, "", parameter.ring->envname);
			report(RPT_DEBUG, "%*sstrings = [\n", level + 3, "");
			for (kdb_long_long_t j = 0; j < parameter.ring->stringsSize; ++j)
			{
				report(RPT_DEBUG, "%*s\"%s\"\n", level + 3, "", parameter.ring->strings[j]);
			}
			report(RPT_DEBUG, "%*s]\n", level + 3, "");
			report(RPT_DEBUG, "%*svalue = " ELEKTRA_LONG_F "\n", level + 3, "", parameter.ring->value);
			report(RPT_DEBUG, "%*s}\n", level + 2, "");
			break;
		case COMMAND_PARAMETER_TYPE_SLIDER:
			report(RPT_DEBUG, "%*sSlider("ELEKTRA_LONG_F") {\n", level + 2, parameter.slider->id);
			report(RPT_DEBUG, "%*sdisplayname = \"%s\"\n", level + 3, "", parameter.slider->displayname);
			report(RPT_DEBUG, "%*senvname = \"%s\"\n", level + 3, "", parameter.slider->envname);
			report(RPT_DEBUG, "%*sminvalue = " ELEKTRA_LONG_F "\n", level + 3, "", parameter.slider->minvalue);
			report(RPT_DEBUG, "%*smaxvalue = " ELEKTRA_LONG_F "\n", level + 3, "", parameter.slider->maxvalue);
			report(RPT_DEBUG, "%*smintext = \"%s\"\n", level + 3, "", parameter.slider->mintext);
			report(RPT_DEBUG, "%*smaxtext = \"%s\"\n", level + 3, "", parameter.slider->maxtext);
			report(RPT_DEBUG, "%*svalue = " ELEKTRA_LONG_F "\n", level + 3, "", parameter.slider->value);
			report(RPT_DEBUG, "%*sstepsize = " ELEKTRA_LONG_F "\n", level + 3, "", parameter.slider->stepsize);
			report(RPT_DEBUG, "%*s}\n", level + 2, "");
			break;
		case COMMAND_PARAMETER_TYPE_CHECKBOX:
			report(RPT_DEBUG, "%*sCheckbox("ELEKTRA_LONG_F") {\n", level + 2, parameter.checkbox->id);
			report(RPT_DEBUG, "%*sdisplayname = \"%s\"\n", level + 3, "", parameter.checkbox->displayname);
			report(RPT_DEBUG, "%*senvname = \"%s\"\n", level + 3, "", parameter.checkbox->envname);
			report(RPT_DEBUG, "%*sallowgray = %s\n", level + 3, "", parameter.checkbox->allowgray ? "true" : "false");
			report(RPT_DEBUG, "%*sontext = \"%s\"\n", level + 3, "", parameter.checkbox->ontext);
			report(RPT_DEBUG, "%*sofftext = \"%s\"\n", level + 3, "", parameter.checkbox->offtext);
			report(RPT_DEBUG, "%*sgraytext = \"%s\"\n", level + 3, "", parameter.checkbox->graytext);
			const char *strValue = ELEKTRA_TO_CONST_STRING(EnumCheckboxState)(parameter.checkbox->value);
			report(RPT_DEBUG, "%*svalue = %s\n", level + 3, "", strValue);
			elektraFree(strValue);
			report(RPT_DEBUG, "%*s}\n", level + 2, "");
			break;
		case COMMAND_PARAMETER_TYPE_ALPHA:
			report(RPT_DEBUG, "%*sAlpha("ELEKTRA_LONG_F") {\n", level + 2, parameter.alpha->id);
			report(RPT_DEBUG, "%*sdisplayname = \"%s\"\n", level + 3, "", parameter.alpha->displayname);
			report(RPT_DEBUG, "%*senvname = \"%s\"\n", level + 3, "", parameter.alpha->envname);
			report(RPT_DEBUG, "%*sallowedchars = \"%s\"\n", level + 3, "", parameter.alpha->allowedchars);
			report(RPT_DEBUG, "%*sminlength = " ELEKTRA_LONG_F "\n", level + 3, "", parameter.alpha->minlength);
			report(RPT_DEBUG, "%*smaxlength = " ELEKTRA_LONG_F "\n", level + 3, "", parameter.alpha->maxlength);
			report(RPT_DEBUG, "%*svalue = \"%s\"\n", level + 3, "", parameter.alpha->value);
			report(RPT_DEBUG, "%*s}\n", level + 2, "");
			break;
		case COMMAND_PARAMETER_TYPE_IP:
			report(RPT_DEBUG, "%*sIp("ELEKTRA_LONG_F") {\n", level + 2, parameter.ip->id);
			report(RPT_DEBUG, "%*sdisplayname = %s\n", level + 3, "", parameter.ip->displayname);
			report(RPT_DEBUG, "%*senvname = %s\n", level + 3, "", parameter.ip->envname);
			report(RPT_DEBUG, "%*sv6 = %s\n", level + 3, "", parameter.ip->v6 ? "true" : "false");
			report(RPT_DEBUG, "%*svalue = %s\n", level + 3, "", parameter.ip->value);
			report(RPT_DEBUG, "%*s}\n", level + 2, "");
			break;
		}
	}

	report(RPT_DEBUG, "%*s]\n", level + 1, "");
	report(RPT_DEBUG, "%*s}\n", level, "");
}

/** dump menu entry hierarchy to screen */
void menu_dump(Menu *menu, int level)
{
	if (menu == NULL)
	{
		return;
	}

	report(RPT_DEBUG, "%*sMenu("ELEKTRA_LONG_F") {\n", level, "", menu->id);
	report(RPT_DEBUG, "%*sdisplayname = \"%s\"\n", level + 1, "", menu->displayname);
	report(RPT_DEBUG, "%*sentries = [\n", level + 1, "");

	for (kdb_long_long_t i = 0; i < menu->entriesSize; ++i)
	{
		switch (menu->entryTypes[i])
		{
		case MENU_ENTRY_TYPE_MENU:
			menu_dump(menu->entries[i].menu, level + 2);
			break;
		case MENU_ENTRY_TYPE_COMMAND:
			command_dump(menu->entries[i].command, level + 2);
			break;
		}
	}

	report(RPT_DEBUG, "%*s]\n", level + 1, "");
	report(RPT_DEBUG, "%*s}\n", level, "");
}
#endif

/* EOF */
