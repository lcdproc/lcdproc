#!/bin/sh
#
# Check the configuration file parser (shared/configfile.c).
#
# Builds tests/configfile-test.c against the parser, reads
# tests/config-parse.conf with it, answers the queries in
# tests/config-parse.queries and compares the answers, including any
# warnings, with tests/config-parse.expected. Needs a configured source tree
# (for config.h) and a C compiler:
#
#   ./configure && sh tests/config-parse.sh
#
# Set CC to use a compiler other than cc.

set -u

top=$(cd "$(dirname "$0")/.." && pwd)
tests=$top/tests

if [ ! -f "$top/config.h" ]; then
	echo "missing $top/config.h: run ./configure first" >&2
	exit 2
fi

work=$(mktemp -d "${TMPDIR:-/tmp}/lcdproc-test.XXXXXX") || exit 2
trap 'rm -rf "$work"' EXIT

if ! ${CC:-cc} -I"$top" -include "$top/config.h" -o "$work/configfile-test" \
		"$tests/configfile-test.c" "$top/shared/configfile.c"; then
	echo "FAIL: building the parser test program"
	exit 1
fi

# The parser names the file it reads; leave that line out, as the path varies.
(cd "$tests" && "$work/configfile-test" config-parse.conf < config-parse.queries) \
	| grep -v '^report: Using Configuration File' > "$work/actual"

if diff -u "$tests/config-parse.expected" "$work/actual"; then
	echo "PASS: configuration file parser ($(grep -c ' = ' "$work/actual") answers)"
else
	echo "FAIL: configuration file parser (differences above: - expected, + actual)"
	exit 1
fi
