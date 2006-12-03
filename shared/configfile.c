/*
 * configfile.c
 * This file is part of LCDd, the lcdproc server.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 *
 * Copyright(c) 2001, Joris Robijn
 *          (c) 2003, Rene Wagner
 *
 *
 * Defines routines to read ini-file-like files.
 * Optionally retrieves settings from an LDAP directory(OpenLDAP 2.1.x)
 */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef WITH_LDAP_SUPPORT
# include <ldap.h>
#endif /* WITH_LDAP_SUPPORT */

#include "shared/report.h"


typedef struct key {
	char *name;
	char *value;
	struct key *next_key;
} key;

typedef struct section {
	char *name;
	key *first_key;
	struct section *next_section;
} section;


static section *first_section = NULL;
/* Yes there is a static. It's C after all :)*/


static section *find_section(const char *sectionname);
static section *add_section(const char *sectionname);
static key *find_key(section *s, const char *keyname, int skip);
static key *add_key(section *s, const char *keyname, const char *value);
static char get_next_char_f(FILE *f);
#if defined(LCDPROC_CONFIG_READ_STRING)
static int process_config(section **current_section, char(*get_next_char)(), const char *source_descr, FILE *f);
#else
static int process_config(section **current_section, const char *source_descr, FILE *f);
#endif


#ifdef WITH_LDAP_SUPPORT
static int connect_to_ldap(void);

static LDAP *ld = NULL;
int use_ldap = 0;

static char *ldap_host = NULL, *ldap_base_dn = NULL;
int ldap_port;

/* not supported for now
 * char ldap_user[255] = "",
 *      ldap_pwd[255]  = "";
 */
#endif /* WITH_LDAP_SUPPORT */


/**** PUBLIC FUNCTIONS ****/

/** Parse configuration from INI-file style config file into memory.
 * \param filename Name of the config file.
 * \returns 0 : config successfully parsed
 * \returns <0 : error occurred
 */
int config_read_file(const char *filename)
{
	FILE *f;
	section *curr_section = NULL;
	int result = 0;

#ifdef WITH_LDAP_SUPPORT
	LDAPURLDesc *url = NULL;
	int retval;
#endif /* WITH_LDAP_SUPPORT */   

	report(RPT_NOTICE, "Using Configuration File: %s", filename);

#ifdef WITH_LDAP_SUPPORT
	if (ldap_is_ldap_url(filename)) {
		use_ldap = 1;

		if (0 != (retval = ldap_url_parse(filename, &url))) {
			report(RPT_ERR, "Errors parsing LDAP URL %s: %s", filename, ldap_err2string(retval));
			ldap_free_urldesc(url);
			return(-1);
		}

		ldap_host = strdup(url->lud_host);
		ldap_port = url->lud_port;
		report(RPT_INFO, "Using LDAP server: %s:%d", ldap_host, ldap_port);

		ldap_base_dn = strdup(url->lud_dn);
		report(RPT_INFO, "Using LDAP base DN: %s", ldap_base_dn);

		ldap_free_urldesc(url);

		if (connect_to_ldap() < 0) {
			debug(RPT_DEBUG, "connect_to_ldap returned errors.");
			return(-1);
		}

		return 0;
	}
#endif /* WITH_LDAP_SUPPORT */ 

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
	section *s;

	/* We use a nested fuction to transfer the characters from buffer to parser*/
	char get_next_char() {
		return str[pos++];
	}

	if ((s = find_section(sectionname)) == NULL)
		s = add_section(sectionname);

	return process_config(&s, get_next_char, "command line", NULL);
}
#endif


/** Get string from configuration in memory.
 * \param sectionname   Name of the section where the key is sought.
 * \param keyname       Name of the key to look for.
 * \param skip          Number of values to skip/ignore before returning the value.
 * \param default_value Default value if section/key is not found.
 * \return Value found / default value
 */
const char *config_get_string(const char *sectionname, const char *keyname,
		int skip, const char *default_value)
{
	key *k = find_key(find_section(sectionname), keyname, skip);

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
 * \param sectionname   Name of the section where the key is sought.
 * \param keyname       Name of the key to look for.
 * \param skip          Number of values to skip/ignore before returning the value.
 * \param default_value Default value if section/key is not found or value is no legal boolean.
 * \return Value found / default value
 */
short config_get_bool(const char *sectionname, const char *keyname,
		int skip, short default_value)
{
	key *k = find_key(find_section(sectionname), keyname, skip);

	if (k == NULL)
		return default_value;

	if (strcasecmp(k->value, "0") == 0 || strcasecmp(k->value, "false") == 0
	|| strcasecmp(k->value, "n") == 0 || strcasecmp(k->value, "no") == 0 
	|| strcasecmp(k->value, "off") == 0) {
		return 0;
	}
	if (strcasecmp(k->value, "1") == 0 || strcasecmp(k->value, "true") == 0
	|| strcasecmp(k->value, "y") == 0 || strcasecmp(k->value, "yes") == 0
	|| strcasecmp(k->value, "on") == 0) {
		return 1;
	}
	return default_value;
}


/** Get integer from configuration in memory.
 * \param sectionname   Name of the section where the key is sought.
 * \param keyname       Name of the key to look for.
 * \param skip          Number of values to skip/ignore before returning the value.
 * \param default_value Default value if section/key is not found or value is no integer.
 * \return Value found / default value
 */
long int config_get_int(const char *sectionname, const char *keyname,
		int skip, long int default_value)
{
	key *k = find_key(find_section(sectionname), keyname, skip);

	if (k != NULL) {
		char *end;
		long int v = strtol(k->value, &end, 0);

		if ((end != NULL) && (end != k->value) && (*end == '\0'))
			/* Conversion succesful */
			return v;
	}
	return default_value;
}


/** Get floating point number from configuration in memory.
 * \param sectionname   Name of the section where the key is sought.
 * \param keyname       Name of the key to look for.
 * \param skip          Number of values to skip/ignore before returning the value.
 * \param default_value Default value if section/key is not found or value is no floating point number.
 * \return Value found / default value
 */
double config_get_float(const char *sectionname, const char *keyname,
		int skip, double default_value)
{
	key *k = find_key(find_section(sectionname), keyname, skip);

	if (k != NULL) {
		char *end;
		double v = strtod(k->value, &end);

		if ((end != NULL) && (end != k->value) && (*end == '\0'))
			/* Conversion succesful*/
			return v;
	}
	return default_value;
}


/** Test whether the configuration containis a specific section.
 * \param sectionname Name of the section to look for.
 * \return 0 = section not in config; 1 = section in config
 */
int config_has_section(const char *sectionname)
{
	return (find_section(sectionname) != NULL) ? 1 : 0;
}


/** Test whether the configuration contains a specific key in a specfic section.
 * \param sectionname Name of the section where the key is sought.
 * \param keyname     Name of the key to look for.
 * \return 0 =  key or section not found; n = key found with n values
 */
int config_has_key(const char *sectionname, const char *keyname)
{
	section *s = find_section(sectionname);
	int count = 0;

	if (s != NULL) {
		key *k;

		for (k = s->first_key; k != NULL; k = k->next_key) {
			/* Did we find the right key ?*/
			if (strcasecmp(k->name, keyname) == 0)
				count++;
		}	
	}
	return count;
}


/** Clear configuration. */
void config_clear()
{
	section *s;
	section *next_s;

	for (s = first_section; s != NULL; s = next_s) {
		key *k;
		key *next_k;

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
	/* Finally make everything inaccessable */
	first_section = NULL;
}


/**** INTERNAL FUNCTIONS ****/

#ifdef WITH_LDAP_SUPPORT
static int
connect_to_ldap(void)
{
	int retval;
	LDAPMessage *res;

	debug(RPT_INFO, "Connecting to LDAP server: %s:%d", ldap_host, ldap_port);

	if (!(ld = ldap_init(ldap_host, ldap_port))) {
		report(RPT_ERR, "LDAP session could not be initialized.");
		return(-1);
	}

/*****************************************************
 * disabled unless you really have a DN/pwd to bind to
 * WARNING: LCDd should not have LDAP write access!!
 *
 *	if (LDAP_SUCCESS != (retval = ldap_simple_bind_s(ld, ldap_user, ldap_pwd))) {
 *		report(RPT_ERR, "LDAP login on %s:%d failed: %s", ldap_host, ldap_port, ldap_err2string(retval));
 *		ldap_unbind(ld);
 *		ld = NULL;
 *		
 *		return(-1);
 *	}
 *	fprintf(stderr, "LDAP login successful on %s:%d\n", ldap_host, ldap_port);
 ********************************************************/

	/* check for the existence of the config object... */
	if (LDAP_SUCCESS != (retval = ldap_search_s(ld, ldap_base_dn, LDAP_SCOPE_BASE, "objectClass=lcdprocConfig", NULL, 0, &res))) {
		report(RPT_ERR, "Could not access LDAP server on %s:%d", ldap_host, ldap_port);
		return(-1);
	}
	if (0 == ldap_count_entries(ld, res)) {
		report(RPT_ERR, "No configuration object found in LDAP at: %s", ldap_base_dn);
		return(-1);
	}
	debug(RPT_DEBUG, "Configuration LDAP object found.");
	return 0;
}

#define BUFSIZE 255
#endif /* WITH_LDAP_SUPPORT */


static section *find_section(const char *sectionname)
{
	section *s;

#ifdef WITH_LDAP_SUPPORT
	LDAPMessage *res;
	int retval;
	char *filter = NULL;

	if (use_ldap) {
		debug(RPT_DEBUG, "Searching LDAP for section [%s]", sectionname);
		if (NULL == (filter = malloc(BUFSIZE))){
			report(RPT_ERR, "Could not allocate memory in find_section()");
			return NULL;
		}
		strcpy(filter, "cn=");
		strncat(filter, sectionname, BUFSIZE);
		if (LDAP_SUCCESS != (retval = ldap_search_s(ld, ldap_base_dn, LDAP_SCOPE_ONELEVEL, filter, NULL, 0, &res))) {
			if (NULL != filter) {
				free(filter);
				filter = NULL;
			}
			ldap_msgfree(res);
			report(RPT_ERR, "Could not access LDAP server on %s:%d", ldap_host, ldap_port);
			return NULL;
		}
		if (NULL != filter) {
			free(filter);
			filter = NULL;
		}
		if (0 == ldap_count_entries(ld, res)) {
			debug(RPT_DEBUG, "Section [%s] not found in LDAP.", sectionname);
			return NULL;
		}
		ldap_msgfree(res);
		debug(RPT_DEBUG, "Found section [%s] in LDAP", sectionname);
		s = (section*) malloc(sizeof(section));
		if (s != NULL) {
			s->name = strdup(sectionname);
			s->first_key = NULL;
			s->next_section = NULL;
		}
		return s;
	}
#endif /* WITH_LDAP_SUPPORT */

	for (s = first_section; s != NULL; s = s->next_section) {
		if (strcasecmp(s->name, sectionname) == 0) {
			return s;
		}
	}
	return NULL; /* not found */
}


static section *add_section(const char *sectionname)
{
	section *s;
	section **place = &first_section;

	for (s = first_section; s != NULL; s = s->next_section)
		place = &(s->next_section);

	*place = (section*) malloc(sizeof(section));
	if (*place != NULL) {
		(*place)->name = strdup(sectionname);
		(*place)->first_key = NULL;
		(*place)->next_section = NULL;
	}	

	return(*place);
}


static key *find_key(section *s, const char *keyname, int skip)
{
	key *k;
	int count = 0;
	key *last_key = NULL;

#ifdef WITH_LDAP_SUPPORT	
	LDAPMessage *res;
	LDAPMessage *entry;
	int retval;
	char *buf = NULL;
	char **vals;
#endif /* WITH_LDAP_SUPPORT */

	/* Check for NULL section*/
	if (s == NULL)
		return NULL;

#ifdef WITH_LDAP_SUPPORT
	if (use_ldap) {
		debug(RPT_DEBUG, "Searching LDAP for key '%s' in section [%s] skipping %d entries.", keyname, s->name, skip);

		if (NULL == (buf = malloc(BUFSIZE))){
			report(RPT_ERR, "Could not allocate memory in find_key().");
		}
		strcpy(buf, "cn=");
		strncat(buf, s->name, BUFSIZE);
		if (LDAP_SUCCESS != (retval = ldap_search_s(ld, ldap_base_dn, LDAP_SCOPE_ONELEVEL, buf, NULL, 0, &res))) {
			if (NULL != buf) {
				free(buf);
				buf=NULL;
			}
			ldap_msgfree(res);
			report(RPT_ERR, "Could not access LDAP server on %s:%d", ldap_host, ldap_port);
			return NULL;
		}
		if (NULL == (entry = ldap_first_entry(ld, res))) {
			debug(RPT_DEBUG, "Section [%s] not found in LDAP.", s->name);
			if (NULL != buf) {
				free(buf);
				buf = NULL;
			}
			/* DON'T enable the following
			 * ldap_msgfree(entry);
			 * ldap_msgfree below does that already
			 */
			ldap_msgfree(res);
			return NULL;
		}


		strcpy(buf, "lcdproc");
		strncat(buf, keyname, BUFSIZE);
		/* debug(RPT_DEBUG, "Key name translated to attribute name: %s", buf); */
		vals = ldap_get_values(ld, entry, buf);

		if (skip+1 > ldap_count_values(vals)) {
			debug(RPT_DEBUG, "No such entry found.");
			if (NULL != buf) {
				free(buf);
				buf = NULL;
			}
			ldap_value_free(vals);
			/* DON'T enable the following
			 * ldap_msgfree(entry);
			 * ldap_msgfree below does that already
			 */
			ldap_msgfree(res);
			return NULL;
		}
		/* DON'T enable the following
		 * ldap_msgfree(entry);
		 * ldap_msgfree below does that already
		 */
		ldap_msgfree(res);
		if (vals && vals[skip]) {
			if (NULL != buf) {
				free(buf);
				buf = NULL;
			}
			buf=strdup(vals[skip]);
			debug(RPT_DEBUG, "Entry found. Value is: %s", buf);
			ldap_value_free(vals);

			k = (key *) malloc(sizeof(key));
			if ( k != NULL) {
				k->name = strdup(keyname);
				k->value = strdup(buf);
				k->next_key = NULL;
			}	

			if (NULL != buf) {
				free(buf);
				buf = NULL;
			}
			return k;
		} 
		report(RPT_ERR, "LDAP server encountered errors.");
		ldap_value_free(vals);
		if (NULL != buf) {
			free(buf);
			buf = NULL;
		}
		return NULL;
	}
#endif /* WITH_LDAP_SUPPORT */

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


static key *add_key(section *s, const char *keyname, const char *value)
{
	if (s != NULL) {
		key *k;
		key **place = &(s->first_key);

		for (k = s->first_key; k != NULL; k = k->next_key)
			place = &(k->next_key);

		*place = (key *) malloc(sizeof(key));
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
static int process_config(section **current_section, char(*get_next_char)(), const char *source_descr, FILE *f)
#else
static int process_config(section **current_section, const char *source_descr, FILE *f)
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
	key *k;
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
				state = ST_INITIAL;	/* alrady at the end, no resync required */	
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
			//  	/* no spaces allowed in section labels WHY? */
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
			//  	/* character invalid in key names WHY ? */
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
			/* assignement: "=" */
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
			  	report(RPT_WARNING, "Assigment expected on line %d of %s: %s",
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
				/* fall though */
			  case '"':
				if (!escape) {
					state = ST_VALUE;
					break;
				}
				/* fall though */
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
section *s;

	for (s = first_section; s != NULL; s = s->next_section) {
		key *k;

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
