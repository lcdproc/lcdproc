/*********************************************
** cfg.c example
** gareth@omnipotent.net 1/June/1998
**
** This code will create cfg_testfile.cfg if
** it doesn't already exist and set some 
** values within it.  Simple really.
*/ 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cfg.h"


#define TESTFILE "cfg_testfile.cfg"

int main() {
	cfg *cfgfile;

	/* First create our object */
	if ((cfgfile=cfg_new())==NULL) {
		printf("Error creating config object\n");
		exit(255);
	}

	/* Then find a config file to open */
	if (cfgfile->openfile(cfgfile,TESTFILE,CFG_READWRITE)!=0) {
		printf("Error reading file: %s\n",cfgfile->lasterror);
		cfgfile->free(cfgfile);
	}

	/* Save to disk on every update */
	cfgfile->autoflush(cfgfile,1); 

	/* print out the current settings - Will return NULL if */
	/* they don't currently exist (eg. new file) */
	printf("'A test string'	is set to '%s'\n", 
		cfgfile->getstring(cfgfile,"A test string"));
	printf("'A test integer' is set to %d\n\n",
		cfgfile->getint(cfgfile,"A test integer"));

	/* set to some new values */
	cfgfile->setstring(cfgfile,"A test string","Marvin the paranoid android");
	cfgfile->setint(cfgfile,"A test integer",42);

	/* print out the new settings */
	printf("'A test string'	is now set to '%s'\n", 
		cfgfile->getstring(cfgfile,"A test string"));
	printf("'A test integer' is now set to %d\n\n",
		cfgfile->getint(cfgfile,"A test integer"));


	/* set the values to something else with autoflush off */
	cfgfile->autoflush(cfgfile,0);
	cfgfile->setstring(cfgfile,"A test string","Life, don't talk to me about life");
	cfgfile->setint(cfgfile,"A test integer",417362);

	/* print out the new settings */
	printf("'A test string'	is finally set to '%s'\n", 
		cfgfile->getstring(cfgfile,"A test string"));
	printf("'A test integer' is finally set to %d\n\n",
		cfgfile->getint(cfgfile,"A test integer"));

	/* flush the last set of data to disk as autoflush is off */
	cfgfile->flush(cfgfile);


	/* cleanup */

	cfgfile->free(cfgfile);

	return(0);
}

