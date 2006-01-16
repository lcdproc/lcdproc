## Process this file with automake to produce Makefile.in

## set automake strictness to 'foreign'
AUTOMAKE_OPTIONS = foreign

SUBDIRS = shared clients server docs scripts

EXTRA_DIST = LCDd.conf LCDclients.conf README.IRman debian

dox:
	$(MAKE) -C docs $@

dist-hook:
	rm -rf `find $(distdir)/debian -name CVS`

## EOF	
