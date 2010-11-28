## Process this file with automake to produce Makefile.in

SUBDIRS = drivers

EXTRA_DIST =	bookinfo.docbook \
		configuration.docbook \
		contact.docbook \
		drivers.docbook \
		how-to-obtain.docbook \
		installation.docbook \
		introduction.docbook \
		lcdproc-user.css \
		lcdproc-user.docbook \
		license.docbook \
		parameters.ent \
		running.docbook \
		README.docbook


## convenience targets
xmlto ?= /usr/bin/xmlto
userguidedir = $(DESTDIR)$(htmldir)/user-guide/

.PHONY: install-html-userguide

install-html-userguide: $(xmlto) lcdproc-user.docbook lcdproc-user.css
	$(mkinstalldirs) $(userguidedir)
	$(xmlto) html -o $(userguidedir) \
	  --stringparam html.stylesheet=lcdproc-user.css lcdproc-user.docbook
	cp lcdproc-user.css $(userguidedir)

## EOF
