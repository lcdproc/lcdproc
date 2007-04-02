## Process this file with automake to produce Makefile.in

## set automake strictness to 'foreign'
AUTOMAKE_OPTIONS = foreign

SUBDIRS = shared clients server docs scripts

sysconf_DATA = LCDd.conf

EXTRA_DIST = $(sysconf_DATA) CREDITS README.IRman contrib


## convenience targets
install-html-guides: install-html-developerguide install-html-userguide

install-html-developerguide:
	$(MAKE) -C docs $@

install-html-userguide:
	$(MAKE) -C docs $@

dox:
	$(MAKE) -C docs $@

topleveltags:
	$(CTAGS) --format=1 -f - --languages=C --c-kinds=f --recurse=yes  server shared clients \
	| perl -p -e 's/^([^\t]+)\t[^\t]+\/([^\t\/]+)\t(.*)$/$1\t$2\t$3/' > tags

## EOF	
