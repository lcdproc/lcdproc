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


section *find_section(const char *sectionname);
section *add_section(const char *sectionname);
key *find_key(section *s, const char *keyname, int skip);
key *add_key(section *s, const char *keyname, const char *value);
char get_next_char_f(FILE *f);
int process_config(section **current_section, char(*get_next_char)(), char modify_section_allowed, const char *source_descr, FILE *f);


#ifdef WITH_LDAP_SUPPORT
int connect_to_ldap(void);

static LDAP *ld = NULL;
int use_ldap = 0;

static char *ldap_host = NULL, *ldap_base_dn = NULL;
int ldap_port;

/* not supported for now
 * char ldap_user[255] = "",
 *      ldap_pwd[255]  = "";
 */
#endif /* WITH_LDAP_SUPPORT */


/**** EXTERNAL FUNCTIONS ****/

int config_read_file(const char *filename)
{
	FILE *f;
	section *curr_section = NULL;

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

	process_config(&curr_section, get_next_char_f, 1, filename, f);

	fclose(f);

	return 0;
}


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

	process_config(&s, get_next_char, 0, "command line", NULL);

	return 0;
}


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


int config_has_section(const char *sectionname)
{
	return (find_section(sectionname) != NULL) ? 1 : 0;
}


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
int
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

section *find_section(const char *sectionname)
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

section *add_section(const char *sectionname)
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

key *find_key(section *s, const char *keyname, int skip)
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

key *add_key(section *s, const char *keyname, const char *value)
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

char get_next_char_f(FILE *f)
{
	int c = fgetc(f);

	return((c == EOF) ? '\0' : c);
}


/* Parser states*/
#define ST_INITIAL 0
#define ST_IGNORE 1
#define ST_SECTIONNAME 2
#define ST_KEYNAME 3
#define ST_VALUE 10
#define ST_QUOTEDVALUE 11
#define ST_QUOTEDVALUE_ESCCHAR 12
#define ST_INVALID_SECTIONNAME 23
#define ST_INVALID_KEYNAME 24
#define ST_INVALID_VALUE 30
#define ST_INVALID_QUOTEDVALUE 31
#define ST_END 99

/* Limits*/
#define MAXSECTIONNAMELENGTH 40
#define MAXKEYNAMELENGTH 40
#define MAXVALUELENGTH 200



int process_config(section **current_section, char(*get_next_char)(), char modify_section_allowed, const char *source_descr, FILE *f)
{
	char state = ST_INITIAL;
	char ch;
	char sectionname[MAXSECTIONNAMELENGTH+1];
	int sectionname_pos = 0;
	char keyname[MAXKEYNAMELENGTH+1];
	int keyname_pos = 0;
	char value[MAXVALUELENGTH+1];
	int value_pos = 0;
	int escape = 0;
	key *k;
	int line_nr = 1;

	while (state != ST_END) {

		ch = (f != NULL)
			? get_next_char(f)
			: get_next_char();

		/* Secretly keep count of the line numbers*/
		if (ch == '\n')
			line_nr++;

		switch (state) {
		  case ST_INITIAL:
			switch (ch) {
			  case '\n':
			  case '\r':
			  case '\t':
			  case ' ':
				break;
			  case '#':
			  case ';':
			  case '=':
			  case ']':
				/* It's a comment or an error*/
				state = ST_IGNORE;
				break;
			  case '[':
				/* It's a section name*/
				state = ST_SECTIONNAME;
				sectionname[0] = '\0';
				sectionname_pos = 0;
				break;
			  case '\0':
				break;
			  default:
				/* It's a keyname*/
				state = ST_KEYNAME;
				keyname[0] = ch;
				keyname[1] = '\0';
				keyname_pos = 1;
			}
			break;
		  case ST_IGNORE:
			switch (ch) {
			  case '\n':
				state = ST_INITIAL;
				break;
			}
			break;
		  case ST_SECTIONNAME:
			switch (ch) {
			  case '\0':
			  case '\n':
				report(RPT_WARNING, "Section name incorrectly closed on line %d of %s: %s", line_nr, source_descr, sectionname);
				state = ST_INITIAL;
				break;
			  case '\r':
			  case '\t':
			  case ' ':
			  case '"':
			  case '[':
				report(RPT_WARNING, "Section name contains invalid chars on line %d of %s: %s", line_nr, source_descr, sectionname);
				state = ST_INVALID_SECTIONNAME;
				break;
			  case ']':
				if (!(*current_section=find_section(sectionname))) {
               				*current_section=add_section(sectionname);
				}
				state = ST_INITIAL;
				break;
			  default:
				if (sectionname_pos < MAXSECTIONNAMELENGTH) {
					sectionname[sectionname_pos++] = ch;
					sectionname[sectionname_pos] = '\0';
				} else {
					report(RPT_WARNING, "Section name too long on line %d of %s: %s", line_nr, source_descr, sectionname);
					state = ST_INVALID_SECTIONNAME;
				}
			}
			break;
		  case ST_INVALID_SECTIONNAME:
			switch (ch) {
			  case '\n':
			  case ']':
				state = ST_INITIAL;
				break;
			}
			break;
		  case ST_KEYNAME:
			switch (ch) {
			  case '\0':
			  case '\n':
			  case '\r':
			  case '\t':
			  case ' ':
				report(RPT_WARNING, "Loose word found on line %d of %s: %s", line_nr, source_descr, keyname);
				state = ST_INITIAL;
				break;
			  case '"':
			  case '[':
			  case ']':
				report(RPT_WARNING, "Key name contains invalid characters on line %d of %s: %s", line_nr, source_descr, keyname);
				state = ST_INVALID_KEYNAME;
				break;
			  case '=':
				state = ST_VALUE;
				value[0] = '\0';
				value_pos = 0;
				break;
			  default:
				if (keyname_pos>=MAXKEYNAMELENGTH) {
					report(RPT_WARNING, "Key name too long on line %d of %s: %s", line_nr, source_descr, keyname);
					state = ST_INVALID_KEYNAME;
				} else {
					keyname[keyname_pos++] = ch;
					keyname[keyname_pos] = '\0';
				}
			}
			break;
		  case ST_INVALID_KEYNAME:
			switch (ch) {
			  case '\n':
				state = ST_INITIAL;
			  /*case ' ':*/
			}
			break;
		  case ST_VALUE:
			switch (ch) {
			  case '[':
			  case ']':
			  case '#':
			  case ';':
			  case '=':
				report(RPT_WARNING, "Value contains invalid characters on line %d of %s, at key: %s", line_nr, source_descr, keyname);
				state = ST_INVALID_VALUE;
				break;
			  case '"':
				state = ST_QUOTEDVALUE;
				break;
			  case '\0':
			  case '\n':
			  case '\r':
			  case '\t':
			  case ' ':
				/* Value complete !*/
				if (! *current_section) {
					report(RPT_WARNING, "Data before any section on line %d of %s with key: %s", line_nr, source_descr, keyname);
				}
				else {
					/* Store the value*/
					k = add_key(*current_section, keyname, value);
				}
				/* And be ready for next thing...*/
				state = ST_INITIAL;
				break;
			  default:
				if (value_pos<MAXVALUELENGTH) {
					value[value_pos++] = ch;
					value[value_pos] = '\0';
				} else {
					report(RPT_WARNING, "Value key is too long on line %d of %s, at key: %s", line_nr, source_descr, keyname);
					state = ST_INVALID_KEYNAME;
				}
			}
			break;
		  case ST_INVALID_VALUE:
			switch (ch) {
			  case '\n':
				state = ST_INITIAL;
				break;
			  case '"':
				state = ST_INVALID_QUOTEDVALUE;
			}
			break;
		  case ST_QUOTEDVALUE:
			switch (ch) {
			  case '\0':
			  case '\n':
				report(RPT_WARNING, "String is incorrectly terminated on line %d of %s: %s", line_nr, source_descr, keyname);
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
					  case 'n': ch = '\n'; break;
					  case 'r': ch = '\r'; break;
					  case 't': ch = '\t'; break;
					  /* default: literal char  (i.e. ignore '\') */
					}
					escape = 0;
				}
				value[value_pos++] = ch;
				value[value_pos] = '\0';
			}
			break;
		  case ST_INVALID_QUOTEDVALUE:
			switch (ch) {
			  case '\n':
				state = ST_INITIAL;
				break;
			  case '"':
				state = ST_INVALID_VALUE;
			}
			break;
		}
		if (ch == '\0') {
			state = ST_END;
		}
	}
	return 0;
}

#if 0
void config_dump(void)
{
section *s;

	for (s = first_section; s != NULL; s = s->next_section) {
		key *k;

		fprintf(stderr, "[%s]\n", s->name);

		for (k = s->first_key; k != NULL; k = k->next_key)
			fprintf(stderr, "%s = '%s'\n", k->name, k->value);

		fprintf(stderr, "\n");
	}	
}
#endif
