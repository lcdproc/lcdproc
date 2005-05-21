AUTOMAKE_OPTIONS = foreign
SUBDIRS = shared clients server docs scripts
EXTRA_DIST = LCDd.conf LCDclients.conf README.IRman debian

dist-hook:
	rm -rf `find $(distdir)/debian -name CVS`
