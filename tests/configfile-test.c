/*
 * Test driver for shared/configfile.c, used by tests/config-parse.sh.
 *
 *   configfile-test FILE < QUERIES
 *
 * Reads FILE with config_read_file(), then answers one query per line of
 * standard input, printing each result so the script can compare it with
 * the expected output. Queries:
 *
 *   string SECTION KEY SKIP   value of KEY (SKIP-th occurrence), or (none)
 *   count SECTION KEY         number of occurrences of KEY
 *   section SECTION           whether SECTION exists
 *
 * Blank lines and lines starting with '#' are copied to the output, so the
 * expected output can be read alongside the queries. Warnings from the
 * parser are printed too, as "report: ...".
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "shared/configfile.h"

/* The parser reports problems through report(); print them for checking. */
void
report(const int level, const char *format, ...)
{
	va_list ap;

	(void) level;
	printf("report: ");
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
	printf("\n");
}

int
main(int argc, char **argv)
{
	char line[512], cmd[32], section[128], key[128];
	int skip;

	if (argc != 2) {
		fprintf(stderr, "usage: %s FILE < QUERIES\n", argv[0]);
		return 2;
	}
	if (config_read_file(argv[1]) < 0)
		printf("config_read_file: failed\n");

	while (fgets(line, sizeof(line), stdin) != NULL) {
		line[strcspn(line, "\n")] = '\0';
		if ((line[0] == '\0') || (line[0] == '#')) {
			printf("%s\n", line);
			continue;
		}
		if ((sscanf(line, "%31s %127s %127s %d", cmd, section, key, &skip) == 4)
		    && (strcmp(cmd, "string") == 0)) {
			const char *v = config_get_string(section, key, skip, NULL);

			if (v != NULL)
				printf("string %s %s %d = \"%s\"\n", section, key, skip, v);
			else
				printf("string %s %s %d = (none)\n", section, key, skip);
		}
		else if ((sscanf(line, "%31s %127s %127s", cmd, section, key) == 3)
			 && (strcmp(cmd, "count") == 0)) {
			printf("count %s %s = %d\n", section, key, config_has_key(section, key));
		}
		else if ((sscanf(line, "%31s %127s", cmd, section) == 2)
			 && (strcmp(cmd, "section") == 0)) {
			printf("section %s = %d\n", section, config_has_section(section));
		}
		else {
			printf("bad query: %s\n", line);
		}
	}

	config_clear();
	return 0;
}
