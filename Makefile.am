## Process this file with automake to produce Makefile.in

## set automake strictness to 'foreign'
AUTOMAKE_OPTIONS = foreign

SUBDIRS = shared clients server docs scripts

sysconf_DATA = LCDd.conf

EXTRA_DIST = $(sysconf_DATA) CREDITS contrib


dist-hook:
	rm -rf `find $(distdir)/contrib -name CVS`


## convenience targets

.PHONY: $(SUBDIRS) install-server install-clients

$(SUBDIRS):
	$(MAKE) -C $@

clients server: shared

install-server: server
	$(MAKE) -C server install
	$(MAKE) -C docs install-server-man

install-clients: clients
	$(MAKE) -C clients install
	$(MAKE) -C docs install-client-man


.PHONY: install-html-guides install-html-developerguide install-html-userguide

install-html-guides: install-html-developerguide install-html-userguide

install-html-developerguide:
	$(MAKE) -C docs $@

install-html-userguide:
	$(MAKE) -C docs $@


.PHONY: dox topleveltags

dox:
	$(MAKE) -C docs $@

topleveltags:
	$(CTAGS) --format=1 -f - --languages=C --c-kinds=f --recurse=yes  server shared clients \
	| perl -p -e 's/^([^\t]+)\t[^\t]+\/([^\t\/]+)\t(.*)$/$1\t$2\t$3/' > tags

## EOF
