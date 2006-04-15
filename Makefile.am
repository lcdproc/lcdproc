## Process this file with automake to produce Makefile.in

## set automake strictness to 'foreign'
AUTOMAKE_OPTIONS = foreign

SUBDIRS = shared clients server docs scripts

sysconf_DATA = LCDd.conf

EXTRA_DIST = $(sysconf_DATA) CREDITS README.IRman contrib


dox:
	$(MAKE) -C docs $@

## EOF	
