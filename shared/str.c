#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "str.h"

int
get_args (char **argv, char *str, int max_args)
{
	char *delimiters = " \n\0";
	char *item;
	int i = 0;

	if (!argv)
		return -1;
	if (!str)
		return 0;
	if (max_args < 1)
		return 0;

	//debug("get_args(%i): string=%s", max_args, str);

	// Parse the command line...
	for (item = strtok (str, delimiters); item; item = strtok (NULL, delimiters)) {
		//debug("get_args: item=%s", item);
		if (i < max_args) {
			argv[i] = item;
			i++;
		} else
			return i;
	}

	return i;
}
