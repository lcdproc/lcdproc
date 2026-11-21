## Process this file with automake to produce Makefile.in

## set automake strictness to 'foreign'
AUTOMAKE_OPTIONS = foreign
SUBDIRS = shared clients server docs scripts
sysconf_DATA = LCDd.conf
EXTRA_DIST = $(sysconf_DATA) CREDITS.md contrib
AM_CFLAGS =
CLEANFILES = compile_commands.json

dist-hook:
	rm -rf `find $(distdir)/contrib -name CVS`

# Only add -MJ when we are actually generating the compilation database
# (avoids polluting normal builds)
if HAVE_CLANG_MJ
COMPDB_CFLAGS = -MJ $(@:.o=.o.json)
else
COMPDB_CFLAGS =
endif

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

.PHONY: install-html-guides install-html-developerguide install-html-userguide compile_commands

install-html-guides: install-html-developerguide install-html-userguide

install-html-developerguide:
	$(MAKE) -C docs $@

install-html-userguide:
	$(MAKE) -C docs $@

compile_commands:
	@if command -v bear >/dev/null 2>&1; then \
		echo "Generating compile_commands.json via Bear..."; \
		$(MAKE) clean; \
		bear -- $(MAKE) -j$$(nproc); \
		echo "Done (Bear)."; \
	elif $(CC) -MJ /dev/null -c -x c /dev/null -o /dev/null >/dev/null 2>&1; then \
		echo "Generating compile_commands.json via Clang -MJ..."; \
		$(MAKE) AM_CFLAGS="$(AM_CFLAGS) -MJ \$$(@:.o=.o.json)" all; \
		echo '[' > compile_commands.json; \
		find . -name '*.o.json' -print0 | xargs -0 cat >> compile_commands.json; \
		sed -i -e '$$s/,$$//' compile_commands.json; \
		echo ']' >> compile_commands.json; \
		find . -name '*.o.json' -delete; \
		echo "Done (Clang -MJ)."; \
	else \
		echo "Warning: neither Bear nor a Clang that supports -MJ is available."; \
		echo "  - Install Bear (e.g. 'sudo apt install bear'), or"; \
		echo "  - Reconfigure with CC=clang"; \
		exit 1; \
	fi

.PHONY: dox topleveltags

dox:
	$(MAKE) -C docs $@

topleveltags:
	$(CTAGS) --format=1 -f - --languages=C --c-kinds=f --recurse=yes  server shared clients \
	| perl -p -e 's/^([^\t]+)\t[^\t]+\/([^\t\/]+)\t(.*)$/$1\t$2\t$3/' > tags



## EOF
