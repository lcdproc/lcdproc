/*
 * Defines routines to read ini-file-like files.
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package
 *
 * (c) 2001 Joris Robijn
 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "shared/report.h"


typedef struct key {
	char * name;
	char * value;
	struct key * next_key;
} key;

typedef struct section {
	char * name;
	key * first_key;
	struct section * next_section;
} section;


static section * first_section = NULL;
// Yes there is a static. It's C after all :)


section * find_section( char * sectionname );
section * add_section( char * sectionname );
key * find_key( section * s, char * keyname, int skip );
key * add_key( section * s, char * keyname, char * value );
int process_config( section ** current_section, char (*get_next_char)(), char modify_section_allowed, char * source_descr );


//// EXTERNAL FUNCTIONS ////

#define FILECHUNKSIZE 10

int config_read_file( char *filename )
{
	FILE * f;
	char buf[FILECHUNKSIZE];
	int bytesread=0;
	int pos=0;
	section * curr_section = NULL;

	// We use a nested fuction to transfer the characters from buffer to parser
	char get_next_char() {
		if( pos>=bytesread ) {
			if( !( bytesread = fread( buf, 1, FILECHUNKSIZE, f ))) {
				// We're at the end
				return 0;
			}
			pos = 0;
		}
		return buf[pos++];
	}

	f = fopen( filename, "r" );
	if( f==NULL ) {
		return -1;
	}

	process_config( &curr_section, get_next_char, 1, filename );

	fclose( f );

	return 0;
}


int config_read_string( char *sectionname, char *str )
// All the config parameters are placed in the given section in memory.
{
	int pos=0;
	section * s;

	// We use a nested fuction to transfer the characters from buffer to parser
	char get_next_char() {
		return str[pos++];
	}

	if( !( s=find_section( sectionname ))) {
		s=add_section( sectionname );
	}

	process_config( &s, get_next_char, 0, "command line" );

	return 0;
}


char *config_get_string( char * sectionname, char * keyname,
		int skip, char * default_value )
{
	section * s;
	key * k;

	s = find_section( sectionname );
	if( !s ) return default_value;
	k = find_key( s, keyname, skip );
	if( !k ) return default_value;

	return k->value;

/* This is the safer way:

	// Reallocate memory space for the return value
	string_storage = realloc( string_storage, ( strlen( k->value ) / 256 + 1) * 256 );
	strcpy( string_storage, k->value );

   But then you also need a global static string_storage = NULL;
*/
}


unsigned char config_get_bool( char *sectionname, char *keyname,
		unsigned int skip, unsigned char default_value )
{
	section * s;
	key * k;

	s = find_section( sectionname );
	if( !s ) return default_value;
	k = find_key( s, keyname, skip );
	if( !k ) return default_value;

	if( strcasecmp( k->value, "0" )==0 || strcasecmp( k->value, "false" )==0
	|| strcasecmp( k->value, "n" )==0 || strcasecmp( k->value, "no" )==0 ) {
		return 0;
	}
	if( strcasecmp( k->value, "1" )==0 || strcasecmp( k->value, "true" )==0
	|| strcasecmp( k->value, "y" )==0 || strcasecmp( k->value, "yes" )==0 ) {
		return 1;
	}
	return default_value;
}


long int config_get_int( char *sectionname, char *keyname,
		unsigned int skip, long int default_value )
{
	section * s;
	key * k;
	long int v;
	char * v_end;

	s = find_section( sectionname );
	if( !s ) return default_value;
	k = find_key( s, keyname, skip );
	if( !k ) return default_value;

	v = strtol( k->value, &v_end, 0 );
	if( v_end-(k->value) != strlen(k->value) ) {
		// Conversion not succesful
		return default_value;
	}
	return v;
}


double config_get_float( char *sectionname, char *keyname,
		unsigned int skip, double default_value )
{
	section * s;
	key * k;
	double v;
	char * v_end;

	s = find_section( sectionname );
	if( !s ) return default_value;
	k = find_key( s, keyname, skip );
	if( !k ) return default_value;

	v = strtod( k->value, &v_end );
	if( v_end-(k->value) != strlen(k->value) ) {
		// Conversion not succesful
		return default_value;
	}
	return v;
}


int config_has_section( char *sectionname )
{
	section * s;

	s = find_section( sectionname );
	if( s ) return 1; else return 0;
}


int config_has_key( char *sectionname, char *keyname )
{
	section * s;
	key * k;

	int count = 0;

	s = find_section( sectionname );
	if( !s ) return 0;

	for( k=s->first_key; k; k=k->next_key ) {

		// Did we find the right key ?
		if( strcasecmp( k->name, keyname ) == 0 ) {
			count ++;
		}
	}
	return count;
}


void config_clear()
{
	report( RPT_WARNING, "config_clear is unimplemented" );
}


//// INTERNAL FUNCTIONS ////

section * find_section( char * sectionname )
{
	section * s;

	for( s=first_section; s; s=s->next_section ) {
		if( strcasecmp( s->name, sectionname ) == 0 ) {
			return s;
		}
	}
	return NULL; // not found
}

section * add_section( char * sectionname )
{
	section *s;
	section ** place = &first_section;

	for( s=first_section; s; s=s->next_section )
		place = &(s->next_section);

	*place = (section*) malloc( sizeof( section ));
	(*place)->name = (char*) malloc( strlen(sectionname)+1 );
	strcpy( (*place)->name, sectionname );
	(*place)->first_key = NULL;
	(*place)->next_section = NULL;

	return (*place);
}

key * find_key( section * s, char * keyname, int skip )
{
	key * k;
	int count = 0;
	key * last_key = NULL;

	// Check for NULL section
	if(!s) return NULL;

	for( k=s->first_key; k; k=k->next_key ) {

		// Did we find the right key ?
		if( strcasecmp( k->name, keyname ) == 0 ) {
			if( count == skip ) {
				return k;
			} else {
				count ++;
				last_key = k;
			}
		}
	}
	if( skip == -1 ) {
		return last_key;
	}
	return NULL; // not found
}

key * add_key( section * s, char * keyname, char * value )
{
	key * k;

	key ** place = &( s->first_key );

	for( k=s->first_key; k; k=k->next_key )
		place = &(k->next_key);

	*place = (key*) malloc( sizeof( key ));
	(*place)->name = (char*) malloc( strlen(keyname)+1 );
	strcpy( (*place)->name, keyname );
	(*place)->value = (char*) malloc( strlen(value)+1 );
	strcpy( (*place)->value, value );
	(*place)->next_key = NULL;

	return (*place);
}


// Parser states
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

// Limits
#define MAXSECTIONNAMELENGTH 40
#define MAXKEYNAMELENGTH 40
#define MAXVALUELENGTH 200



int process_config( section ** current_section, char (*get_next_char)(), char modify_section_allowed, char * source_descr )
{
	char state = ST_INITIAL;
	char ch;
	char sectionname[MAXSECTIONNAMELENGTH+1];
	int sectionname_pos = 0;
	char keyname[MAXKEYNAMELENGTH+1];
	int keyname_pos = 0;
	char value[MAXVALUELENGTH+1];
	int value_pos = 0;
	int quote = 0;
	key * k;
	int line_nr = 1;

	while( state != ST_END ) {

		ch = get_next_char();

		// Secretly keep count of the line numbers
		if( ch == '\n' ) {
			line_nr ++;
		}

		switch( state ) {
		  case ST_INITIAL:
			switch( ch ) {
			  case '\n':
			  case '\r':
			  case '\t':
			  case ' ':
				break;
			  case '#':
			  case ';':
			  case '=':
			  case ']':
				// It's a comment or an error
				state = ST_IGNORE;
				break;
			  case '[':
				// It's a section name
				state = ST_SECTIONNAME;
				sectionname[0] = 0;
				sectionname_pos = 0;
				break;
			  case 0:
				break;
			  default:
				// It's a keyname
				state = ST_KEYNAME;
				keyname[0] = ch;
				keyname[1] = 0;
				keyname_pos = 1;
			}
			break;
		  case ST_IGNORE:
			switch( ch ) {
			  case '\n':
				state = ST_INITIAL;
				break;
			}
			break;
		  case ST_SECTIONNAME:
			switch( ch ) {
			  case '\n':
				report( RPT_WARNING, "Section name incorrectly closed on line %d of %s: %s", line_nr, source_descr, sectionname );
				state = ST_INITIAL;
				break;
			  case '\r':
			  case '\t':
			  case ' ':
			  case '"':
			  case '[':
				report( RPT_WARNING, "Section name contains invalid chars on line %d of %s: %s", line_nr, source_descr, sectionname );
				state = ST_INVALID_SECTIONNAME;
				break;
			  case ']':
				if( !( *current_section=find_section( sectionname ))) {
               				*current_section=add_section( sectionname );
				}
				state = ST_INITIAL;
				break;
			  case 0:
				report( RPT_WARNING, "Section name incorrectly closed on line %d of %s: %s", line_nr, source_descr, sectionname );
				break;
			  default:
				if( sectionname_pos<MAXSECTIONNAMELENGTH ) {
					sectionname[sectionname_pos++] = ch;
					sectionname[sectionname_pos] = 0;
				} else {
					report( RPT_WARNING, "Section name too long on line %d of %s: %s", line_nr, source_descr, sectionname );
					state = ST_INVALID_SECTIONNAME;
				}
			}
			break;
		  case ST_INVALID_SECTIONNAME:
			switch( ch ) {
			  case '\n':
			  case ']':
				state = ST_INITIAL;
				break;
			}
			break;
		  case ST_KEYNAME:
			switch( ch ) {
			  case 0:
			  case '\n':
			  case '\r':
			  case '\t':
			  case ' ':
				report( RPT_WARNING, "Loose word found on line %d of %s: %s", line_nr, source_descr, keyname );
				state = ST_INITIAL;
				break;
			  case '"':
			  case '[':
			  case ']':
				report( RPT_WARNING, "Key name contains invalid characters on line %d of %s: %s", line_nr, source_descr, keyname );
				state = ST_INVALID_KEYNAME;
				break;
			  case '=':
				state = ST_VALUE;
				value[0] = 0;
				value_pos = 0;
				break;
			  default:
				if( keyname_pos>=MAXKEYNAMELENGTH ) {
					report( RPT_WARNING, "Key name too long on line %d of %s: %s", line_nr, source_descr, keyname );
					state = ST_INVALID_KEYNAME;
				} else {
					keyname[keyname_pos++] = ch;
					keyname[keyname_pos] = 0;
				}
			}
			break;
		  case ST_INVALID_KEYNAME:
			switch( ch ) {
			  case '\n':
				state = ST_INITIAL;
			  //case ' ':
			}
			break;
		  case ST_VALUE:
			switch( ch ) {
			  case '[':
			  case ']':
			  case '#':
			  case ';':
			  case '=':
				report( RPT_WARNING, "Value contains invalid characters on line %d of %s, at key: %s", line_nr, source_descr, keyname );
				state = ST_INVALID_VALUE;
				break;
			  case '"':
				state = ST_QUOTEDVALUE;
				break;
			  case 0:
			  case '\n':
			  case '\r':
			  case '\t':
			  case ' ':
				// Value complete !
				if( ! *current_section ) {
					report( RPT_WARNING, "Data before any section on line %d of %s with key: %s", line_nr, source_descr, keyname );
				}
				else {
					// Store the value
					k = add_key( *current_section, keyname, value );
				}
				// And be ready for next thing...
				state = ST_INITIAL;
				break;
			  default:
				if( value_pos<MAXVALUELENGTH ) {
					value[value_pos++] = ch;
					value[value_pos] = 0;
				} else {
					report( RPT_WARNING, "Value key is too long on line %d of %s, at key: %s", line_nr, source_descr, keyname );
					state = ST_INVALID_KEYNAME;
				}
			}
			break;
		  case ST_INVALID_VALUE:
			switch( ch ) {
			  case '\n':
				state = ST_INITIAL;
				break;
			  case '"':
				state = ST_INVALID_QUOTEDVALUE;
			}
			break;
		  case ST_QUOTEDVALUE:
			switch( ch ) {
			  case 0:
			  case '\n':
				report( RPT_WARNING, "String is incorrectly terminated on line %d of %s: %s", line_nr, source_descr, keyname );
				state = ST_INITIAL;
				break;
			  case '\\':
				if( !quote ) {
					quote = 1;
					break;
				}
			  case '"':
				if( !quote ) {
					state = ST_VALUE;
					break;
				}
			  default:
				if( quote ) {
					switch( ch ) {
					  case 'n': ch = '\n'; break;
					  case 'r': ch = '\r'; break;
					  case 't': ch = '\t'; break;
					  // default: litteral
					}
					quote = 0;
				}
				value[value_pos++] = ch;
				value[value_pos] = 0;
			}
			break;
		  case ST_INVALID_QUOTEDVALUE:
			switch( ch ) {
			  case '\n':
				state = ST_INITIAL;
				break;
			  case '"':
				state = ST_INVALID_VALUE;
			}
			break;
		}
		if( ch == 0 ) {
			state = ST_END;
		}
	}
	return 0;
}

