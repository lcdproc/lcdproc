/** \file configfile.c
 * Define routines to read INI-file like files.
 */

/* This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright(c) 2001, Joris Robijn
 *          (c) 2003, Rene Wagner
 *          (c) 2006,2007 Peter Marschall
 *
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "shared/report.h"


/** configuration key */
typedef struct _config_key {
	char *name;			/**< name of the config key */
	char *value;			/**< value of the config key */
	struct _config_key *next_key;	/**< pointer to next config key */
} ConfigKey;

/** configuration section */
typedef struct _config_section {
	char *name;			/**< name of the config section */
	ConfigKey *first_key;		/**< config keys in the config section */
	struct _config_section *next_section;	/**< pointer to next config section */
} ConfigSection;


static ConfigSection *first_section = NULL;
/* Yes there is a static. It's C after all :)*/


static ConfigSection *find_section(const char *sectionname);
static ConfigSection *add_section(const char *sectionname);
static ConfigKey *find_key(ConfigSection *s, const char *keyname, int skip);
static ConfigKey *add_key(ConfigSection *s, const char *keyname, const char *value);
#if defined(LCDPROC_CONFIG_READ_STRING)
static char get_next_char_f(FILE *f);
static int process_config(ConfigSection **current_section, char(*get_next_char)(), const char *source_descr, FILE *f);
#else
static int process_config(ConfigSection **current_section, const char *source_descr, FILE *f);
#endif


/**** PUBLIC FUNCTIONS ****/

/** Parse configuration from INI-file style config file into memory.
 * \param filename  Name of the config file.
 * \retval 0        config successfully parsed
 * \retval <0       error occurred
 */
int config_read_file(const char *filename)
{
	FILE *f;
	ConfigSection *curr_section = NULL;
	int result = 0;

	report(RPT_NOTICE, "Using Configuration File: %s", filename);

	f = fopen(filename, "r");
	if (f == NULL) {
		return -1;
	}

#if defined(LCDPROC_CONFIG_READ_STRING)
	result = process_config(&curr_section, get_next_char_f, filename, f);
#else
	result = process_config(&curr_section, filename, f);
#endif

	fclose(f);

	return result;
}


#if defined(LCDPROC_CONFIG_READ_STRING)
int config_read_string(const char *sectionname, const char *str)
/* All the config parameters are placed in the given section in memory.*/
{
	int pos = 0;
	ConfigSection *s;

	/* We use a nested function to transfer the characters from buffer to parser*/
	char get_next_char() {
		return str[pos++];
	}

	if ((s = find_section(sectionname)) == NULL)
		s = add_section(sectionname);

	return process_config(&s, get_next_char, "command line", NULL);
}
#endif


/** Get string from configuration in memory.
 *
 * The strings returned are always NUL-terminated.
 * They should never be modified, and used only short-term.
 * In successive calls this function can re-use the data space !
 *
 * You can do some things with the returned string:
 * \li Scan or parse it:
 *     \code
 *     s = config_get_string(...);
 *     sscanf(s, "%dx%d", &w, &h);  // scan format like: 20x4
 *    \endcode
 *    ...and check the w and h values...
 * \li Copy it to a pre-allocated buffer like \c device[256]:
 *     \code
 *     s = config_get_string(...);
 *     strncpy(device, s, sizeof(device));
 *     device[sizeof(device)-1] = '\0';  // make sure it is terminated
 *    \endcode
 * \li Copy it to a newly allocated space in \c char \c *device:
 *     \code
 *     s = config_get_string(...);
 *     device = malloc(strlen(s)+1);
 *     if (device == NULL) return -5; // or whatever < 0
 *     strcpy( device, s );
 *     \endcode
 *
 * \param sectionname   Name of the section where the key is sought.
 * \param keyname       Name of the key to look for.
 * \param skip          Number of values to skip/ignore before returning the value.
 *                      This is used to iterate through the values of a multi-valued key:
 *                      \c 0 for the first value, \c 1 for the 2nd, ... and \c -1 for the last.
 * \param default_value Default value if section/key is not found
 *                      or \c skip exceeds the number of values of the key.
 * \return              Value found / \c default_value
 */
const char *config_get_string(const char *sectionname, const char *keyname,
		int skip, const char *default_value)
{
	ConfigKey *k = find_key(find_section(sectionname), keyname, skip);

	if (k == NULL)
		return default_value;

	return k->value;

/* This is the safer way:*/

	/* Reallocate memory space for the return value*/
	/*
	string_storage = realloc(string_storage,(strlen(k->value) / 256 + 1) * 256);
	strcpy(string_storage, k->value);

   But then you also need a global static string_storage = NULL;
*/
}


/** Get boolean value from configuration in memory.
 *
 * Legal boolean values are:
 * \li \c 0 , \c false , \c off , \c no or \c n for FALSE.
 * \li \c 1 , \c true , \c on , \c yes or \c y for TRUE
 *
 * \param sectionname   Name of the section where the key is sought.
 * \param keyname       Name of the key to look for.
 * \param skip          Number of values to skip/ignore before returning the value.
 *                      This is used to iterate through the values of a multi-valued key:
 *                      \c 0 for the first value, \c 1 for the 2nd, ... and \c -1 for the last.
 * \param default_value Default value if section/key is not found, value is no legal boolean,
 *                      or \c skip exceeds the number of values of the key.
 * \return              Value found / \c default_value
 */
short config_get_bool(const char *sectionname, const char *keyname,
		int skip, short default_value)
{
	ConfigKey *k = find_key(find_section(sectionname), keyname, skip);

	if (k == NULL)
		return default_value;

	if ((strcasecmp(k->value, "0") == 0) || (strcasecmp(k->value, "false") == 0) ||
	    (strcasecmp(k->value, "n") == 0) || (strcasecmp(k->value, "no") == 0) ||
	    (strcasecmp(k->value, "off") == 0)) {
		return 0;
	}
	if ((strcasecmp(k->value, "1") == 0) || (strcasecmp(k->value, "true") == 0) ||
	    (strcasecmp(k->value, "y") == 0) || (strcasecmp(k->value, "yes") == 0) ||
	    (strcasecmp(k->value, "on") == 0)) {
		return 1;
	}
	return default_value;
}


/** Get tristate value from configuration in memory.
 *
 * Legal tristate values are:
 * \li \c 0 , \c false , \c off , \c no or \c n for 0.
 * \li \c 1 , \c true , \c on , \c yes or \c y for 1
 * \li \c 2 or the given name of the third state for 2
 *
 * \param sectionname   Name of the section where the key is sought.
 * \param keyname       Name of the key to look for.
 * \param skip          Number of values to skip/ignore before returning the value.
 *                      This is used to iterate through the values of a multi-valued key:
 *                      \c 0 for the first value, \c 1 for the 2nd, ... and \c -1 for the last.
 * \param name3rd       Name of the 3rd state
 * \param default_value Default value if section/key is not found, value is no legal boolean,
 *                      or \c skip exceeds the number of values of the key.
 * \return              Value found / \c default_value
 */
short config_get_tristate(const char *sectionname, const char *keyname,
		int skip, const char *name3rd, short default_value)
{
	ConfigKey *k = find_key(find_section(sectionname), keyname, skip);

	if (k == NULL)
		return default_value;

	if ((strcasecmp(k->value, "0") == 0) || (strcasecmp(k->value, "false") == 0) ||
	    (strcasecmp(k->value, "n") == 0) || (strcasecmp(k->value, "no") == 0) ||
	    (strcasecmp(k->value, "off") == 0)) {
		return 0;
	}
	if ((strcasecmp(k->value, "1") == 0) || (strcasecmp(k->value, "true") == 0) ||
	    (strcasecmp(k->value, "y") == 0) || (strcasecmp(k->value, "yes") == 0) ||
	    (strcasecmp(k->value, "on") == 0)) {
		return 1;
	}
	if ((strcasecmp(k->value, "2") == 0) ||
	    ((name3rd != NULL) && (strcasecmp(k->value, name3rd) == 0))) {
		return 2;
	}
	return default_value;
}


/** Get integer from configuration in memory.
 * \param sectionname   Name of the section where the key is sought.
 * \param keyname       Name of the key to look for.
 * \param skip          Number of values to skip/ignore before returning the value.
 *                      This is used to iterate through the values of a multi-valued key:
 *                      \c 0 for the first value, \c 1 for the 2nd, ... and \c -1 for the last.
 * \param default_value Default value if section/key is not found, value is no integer,
 *                      or \c skip exceeds the number of values of the key.
 * \return              Value found / \c default_value
 */
long int config_get_int(const char *sectionname, const char *keyname,
		int skip, long int default_value)
{
	ConfigKey *k = find_key(find_section(sectionname), keyname, skip);

	if (k != NULL) {
		char *end;
		long int v = strtol(k->value, &end, 0);

		if ((end != NULL) && (end != k->value) && (*end == '\0'))
			/* Conversion successful */
			return v;
	}
	return default_value;
}


/** Get floating point number from configuration in memory.
 * \param sectionname   Name of the section where the key is sought.
 * \param keyname       Name of the key to look for.
 * \param skip          Number of values to skip/ignore before returning the value.
 *                      This is used to iterate through the values of a multi-valued key:
 *                      \c 0 for the first value, \c 1 for the 2nd, ... and \c -1 for the last.
 * \param default_value Default value if section/key is not found, value is no floating point number
 *                      or \c skip exceeds the number of values of the key.
 * \return              Value found / \c default_value
 */
double config_get_float(const char *sectionname, const char *keyname,
		int skip, double default_value)
{
	ConfigKey *k = find_key(find_section(sectionname), keyname, skip);

	if (k != NULL) {
		char *end;
		double v = strtod(k->value, &end);

		if ((end != NULL) && (end != k->value) && (*end == '\0'))
			/* Conversion successful*/
			return v;
	}
	return default_value;
}


/** Test whether the configuration contains a specific section.
 * \param sectionname  Name of the section to look for.
 * \retval 0           section not in config
 * \retval 1           section in config
 */
int config_has_section(const char *sectionname)
{
	return (find_section(sectionname) != NULL) ? 1 : 0;
}


/** Test whether the configuration contains a specific key in a specific section.
 * \param sectionname  Name of the section where the key is sought.
 * \param keyname      Name of the key to look for.
 * \retval 0           key or section not found
 * \retval n           key found with \c n values (\c n > 0)
 */
int config_has_key(const char *sectionname, const char *keyname)
{
	ConfigSection *s = find_section(sectionname);
	int count = 0;

	if (s != NULL) {
		ConfigKey *k;

		for (k = s->first_key; k != NULL; k = k->next_key) {
			/* Did we find the right key ?*/
			if (strcasecmp(k->name, keyname) == 0)
				count++;
		}
	}
	return count;
}


/** Clear configuration. */
void config_clear(void)
{
	ConfigSection *s;
	ConfigSection *next_s;

	for (s = first_section; s != NULL; s = next_s) {
		ConfigKey *k;
		ConfigKey *next_k;

		for (k = s->first_key; k != NULL; k = next_k) {
			/* Advance before we destroy the current key */
			next_k = k->next_key;

			free(k->name);
			free(k->value);
			free(k);
		}
		/* Advance before we destroy the current section */
		next_s = s->next_section;

		/* And destroy it */
		free(s->name);
		free(s);
	}
	/* Finally make everything inaccessible */
	first_section = NULL;
}


/**** INTERNAL FUNCTIONS ****/

static ConfigSection *find_section(const char *sectionname)
{
	ConfigSection *s;

	for (s = first_section; s != NULL; s = s->next_section) {
		if (strcasecmp(s->name, sectionname) == 0) {
			return s;
		}
	}
	return NULL; /* not found */
}


static ConfigSection *add_section(const char *sectionname)
{
	ConfigSection *s;
	ConfigSection **place = &first_section;

	for (s = first_section; s != NULL; s = s->next_section)
		place = &(s->next_section);

	*place = (ConfigSection *) malloc(sizeof(ConfigSection));
	if (*place != NULL) {
		(*place)->name = strdup(sectionname);
		(*place)->first_key = NULL;
		(*place)->next_section = NULL;
	}

	return(*place);
}


static ConfigKey *find_key(ConfigSection *s, const char *keyname, int skip)
{
	ConfigKey *k;
	int count = 0;
	ConfigKey *last_key = NULL;

	/* Check for NULL section*/
	if (s == NULL)
		return NULL;

	for (k = s->first_key; k != NULL; k = k->next_key) {

		/* Did we find the right key ?*/
		if (strcasecmp(k->name, keyname) == 0) {
			if (count == skip)
				return k;

			count++;
			last_key = k;
		}
	}
	if (skip == -1)
		return last_key;

	return NULL; /* not found*/
}


static ConfigKey *add_key(ConfigSection *s, const char *keyname, const char *value)
{
	if (s != NULL) {
		ConfigKey *k;
		ConfigKey **place = &(s->first_key);

		for (k = s->first_key; k != NULL; k = k->next_key)
			place = &(k->next_key);

		*place = (ConfigKey *) malloc(sizeof(ConfigKey));
		if (*place != NULL) {
			(*place)->name = strdup(keyname);
			(*place)->value = strdup(value);
			(*place)->next_key = NULL;
		}

		return(*place);
	}
	return NULL;
}


#if defined(LCDPROC_CONFIG_READ_STRING)
static char get_next_char_f(FILE *f)
{
	int c = fgetc(f);

	return((c == EOF) ? '\0' : c);
}
#endif


/* Parser states */
#define ST_INITIAL		0
#define ST_COMMENT		257
#define ST_SECTIONLABEL		258
#define ST_KEYNAME		259
#define ST_ASSIGNMENT		260
#define ST_VALUE		261
#define ST_QUOTEDVALUE		262
#define ST_SECTIONLABEL_DONE	263
#define ST_VALUE_DONE		264
#define ST_INVALID_SECTIONLABEL	265
#define ST_INVALID_KEYNAME	266
#define ST_INVALID_ASSIGNMENT	267
#define ST_INVALID_VALUE	268
#define ST_END			999

/* Limits */
#define MAXSECTIONLABELLENGTH	40
#define MAXKEYNAMELENGTH	40
#define MAXVALUELENGTH		200


#if defined(LCDPROC_CONFIG_READ_STRING)
static int process_config(ConfigSection **current_section, char(*get_next_char)(), const char *source_descr, FILE *f)
#else
static int process_config(ConfigSection **current_section, const char *source_descr, FILE *f)
#endif
{
	int state = ST_INITIAL;
	int ch;
	char sectionname[MAXSECTIONLABELLENGTH+1];
	int sectionname_pos = 0;
	char keyname[MAXKEYNAMELENGTH+1];
	int keyname_pos = 0;
	char value[MAXVALUELENGTH+1];
	int value_pos = 0;
	int escape = 0;
	ConfigKey *k;
	int line_nr = 1;
	int error = 0;

#if !defined(LCDPROC_CONFIG_READ_STRING)
	if (f == NULL)
		return(0);
#endif

	while (state != ST_END) {

#if defined(LCDPROC_CONFIG_READ_STRING)
		ch = (f != NULL)
			? get_next_char(f)
			: get_next_char();
#else
		ch = fgetc(f);
		if (ch == EOF)
			ch = '\0';
#endif

		/* Secretly keep count of the line numbers */
		if (ch == '\n')
			line_nr++;

		switch (state) {
		  case ST_INITIAL:
			switch (ch) {
			  case '#':
			  case ';':
				/* comment start */
				state = ST_COMMENT;
				/* fall through */
			  case '\0':
			  case '\n':
			  case '\r':
			  case '\t':
			  case ' ':
				/* ignore spaces */
				break;
			  case '[':
				/* section name */
				state = ST_SECTIONLABEL;
				sectionname_pos = 0;
				sectionname[sectionname_pos] = '\0';
				break;
			  default:
				/* key word */
				state = ST_KEYNAME;
				keyname_pos = 0;
				keyname[keyname_pos++] = ch;
				keyname[keyname_pos] = '\0';
			}
			break;
		  case ST_SECTIONLABEL:
			/* section label: "["{non-space chars}+"]" */
			switch (ch) {
			  case '\0':
			  case '\n':
				/* premature end of label */
				report(RPT_WARNING, "Unterminated section label on line %d of %s: %s",
						line_nr, source_descr, sectionname);
				error = 1;
				state = ST_INITIAL;	/* already at the end, no resync required */
				break;
			  case ']':
				/* label terminated: find/create section */
				if (!(*current_section = find_section(sectionname))) {
					*current_section = add_section(sectionname);
				}
				state = ST_SECTIONLABEL_DONE;
				break;
			//  case '\r':
			//  case '\t':
			//  case ' ':
			//	/* no spaces allowed in section labels WHY? */
			//	report(RPT_WARNING, "Invalid character in section label on line %d of %s: %s",
			//			line_nr, source_descr, sectionname);
			//	error = 1;
			//	state = ST_INVALID_SECTIONLABEL;	/* resync required */
			//	break;
			  default:
				/* append char to section label */
				if (sectionname_pos < MAXSECTIONLABELLENGTH) {
					sectionname[sectionname_pos++] = ch;
					sectionname[sectionname_pos] = '\0';
					break;
				}
				report(RPT_WARNING, "Section name too long on line %d of %s: %s",
						line_nr, source_descr, sectionname);
				error = 1;
				state = ST_INVALID_SECTIONLABEL;	/* resync required */
			}
			break;
		  case ST_KEYNAME:
			/* key name: {non-space chars}+ */
			switch (ch) {
			  case '\r':
			  case '\t':
			  case ' ':
				/* ignore trailing spaces */
				if (keyname_pos != 0)
					state = ST_ASSIGNMENT;
				break;
			  case '\0':
			  case '\n':
				/* premature end of line */
				report(RPT_WARNING, "Loose word found on line %d of %s: %s",
						line_nr, source_descr, keyname);
				error = 1;
				state = ST_INITIAL;	/* already at the end; no resync required */
				break;
			  case '=':
				/* end of key reached, "=" found, now we need a value */
				state = ST_VALUE;
				value[0] = '\0';
				value_pos = 0;
				break;
			//  case '"':
			//  case '[':
			//  case ']':
			//	/* character invalid in key names WHY ? */
			//	report(RPT_WARNING, "Invalid character in key name on line %d of %s: %s",
			//			line_nr, source_descr, keyname);
			//	error = 1;
			//	state = ST_INVALID_KEYNAME;	/* resync required */
			//	break;
			  default:
				/* append char to key name */
				if (keyname_pos < MAXKEYNAMELENGTH) {
					keyname[keyname_pos++] = ch;
					keyname[keyname_pos] = '\0';
					break;
				}
				report(RPT_WARNING, "Key name too long on line %d of %s: %s",
						line_nr, source_descr, keyname);
				error = 1;
				state = ST_INVALID_KEYNAME;	/* resync required */
			}
			break;
		  case ST_ASSIGNMENT:
			/* assignment: "=" */
			switch (ch) {
			  case '\t':
			  case ' ':
				/* ignore leading spaces */
				break;
			  case '=':
				/* "=" found, now we need a value */
				state = ST_VALUE;
				value[0] = '\0';
				value_pos = 0;
				break;
			  default:
				report(RPT_WARNING, "Assignment expected on line %d of %s: %s",
						line_nr, source_descr, keyname);
				error = 1;
				state = ST_INVALID_ASSIGNMENT;
			}
			break;
		  case ST_VALUE:
			/* value: {non-space char}+ | "\""{any potentially-quoted char}+"\"" */
			switch (ch) {
			  case '#':
			  case ';':
				/* allow comment if we already had a value */
				/* WHY ONLY THEN ? 'xx=' can be seen as equivalent to 'xx=""' */
				if (value_pos > 0) {
					state = ST_COMMENT;
					break;
				}
				/* fall through */
			  case '[':
			  case ']':
			  case '=':
				/* illegal characters WHY? */
				report(RPT_WARNING, "Invalid character '%c' in value on line %d of %s, at key: %s",
						ch, line_nr, source_descr, keyname);
				error = 1;
				state = ST_INVALID_VALUE;
				break;
			  case '\t':
			  case ' ':
				/* ignore leading spaces */
				if (value_pos == 0)
					break;
				/* fall through */
			  case '\0':
			  case '\n':
			  case '\r':
				/* value complete */
				if (!*current_section) {
					report(RPT_WARNING, "Data outside sections on line %d of %s with key: %s",
							line_nr, source_descr, keyname);
					error = 1;
				}
				else {
					/* Store the value*/
					k = add_key(*current_section, keyname, value);
				}
				/* And be ready for next thing...*/
				state = ((ch == ' ') || (ch == '\t')) ? ST_VALUE_DONE : ST_INITIAL;
				break;
			  case '"':
				/* quoted string */
				state = ST_QUOTEDVALUE;
				break;
			  default:
				/* append char to value */
				if (value_pos < MAXVALUELENGTH) {
					value[value_pos++] = ch;
					value[value_pos] = '\0';
					break;
				}
				report(RPT_WARNING, "Value too long on line %d of %s, at key: %s",
						line_nr, source_descr, keyname);
				error = 1;
				state = ST_INVALID_VALUE;
			}
			break;
		  case ST_QUOTEDVALUE:
			/* a quoted part of a string */
			switch (ch) {
			  case '\0':
			  case '\n':
				report(RPT_WARNING, "Premature end of quoted string on line %d of %s: %s",
						line_nr, source_descr, keyname);
				error = 1;
				state = ST_INITIAL;
				break;
			  case '\\':
				if (!escape) {
					escape = 1;
					break;
				}
				/* fall through */
			  case '"':
				if (!escape) {
					state = ST_VALUE;
					break;
				}
				/* fall through */
			  default:
				if (escape) {
					switch (ch) {
					  case 'a': ch = '\a'; break;
					  case 'b': ch = '\b'; break;
					  case 'f': ch = '\f'; break;
					  case 'n': ch = '\n'; break;
					  case 'r': ch = '\r'; break;
					  case 't': ch = '\t'; break;
					  case 'v': ch = '\v'; break;
					  /* default: literal char  (i.e. ignore '\') */
					}
					escape = 0;
				}
				value[value_pos++] = ch;
				value[value_pos] = '\0';
			}
			break;
		  case ST_SECTIONLABEL_DONE:
		  case ST_VALUE_DONE:
			switch (ch) {
			  case ';':
			  case '#':
				state = ST_COMMENT;
				break;
			  case '\0':
			  case '\n':
				state = ST_INITIAL;
				break;
			  case '\t':
			  case ' ':
				break;
			  default:
				/* illegal characters */
				report(RPT_WARNING, "Invalid character '%c' on line %d of %s",
						ch, line_nr, source_descr);
				error = 1;
				state = ST_INVALID_VALUE;
			 }
		  case ST_INVALID_SECTIONLABEL:
			/* invalid section label: resync up to end of label/next line */
			if (ch == ']')
				state = ST_INITIAL;
			/* fall through */
		  case ST_INVALID_ASSIGNMENT:
		  case ST_INVALID_KEYNAME:
		  case ST_INVALID_VALUE:
		  case ST_COMMENT:
			/* comment or error: ignore anything up to the next line */
			if (ch == '\n')
				state = ST_INITIAL;
		}
		if (ch == '\0') {
			if ((!error) && (state != ST_INITIAL) && (state != ST_COMMENT) &&
			    (state != ST_SECTIONLABEL_DONE) && (state != ST_VALUE_DONE)) {
				report(RPT_WARNING, "Premature end of configuration on line %d of %s: %d",
						line_nr, source_descr, state);

				error = 1;
			}
			state = ST_END;
		}

	}
	return (error) ? -1 : 0;
}


#if CONFIGFILE_DEBUGTEST
void config_dump(void)
{
ConfigSection *s;

	for (s = first_section; s != NULL; s = s->next_section) {
		ConfigKey *k;

		fprintf(stderr, "[%s]\n", s->name);

		for (k = s->first_key; k != NULL; k = k->next_key)
			fprintf(stderr, "%s = \"%s\"\n", k->name, k->value);

		fprintf(stderr, "\n");
	}
}


int main(int argc, char *argv[])
{
	if (argc > 0)
		config_read_file(argv[1]);
	config_dump();
}
#endif
