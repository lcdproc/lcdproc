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
userguidedir ?= $(DESTDIR)$(htmldir)/user-guide/
docreleasedir = lcdproc-$(PACKAGE_VERSION)-user-html

.PHONY: install-html-userguide doc-release doc-online

install-html-userguide: lcdproc-user.docbook lcdproc-user.css
	$(mkinstalldirs) $(userguidedir)
	$(XMLTO) html -o $(userguidedir) \
	  --stringparam html.stylesheet=lcdproc-user.css lcdproc-user.docbook
	cp lcdproc-user.css $(userguidedir)

doc-release: lcdproc-user.docbook lcdproc-user.css
	$(XMLTO) html -o $(docreleasedir) \
	--stringparam html.stylesheet=lcdproc-user.css lcdproc-user.docbook
	cp lcdproc-user.css $(docreleasedir)
	$(AMTAR) -czf $(docreleasedir).tar.gz $(docreleasedir)

doc-online: lcdproc-user.docbook
	$(XMLTO) html-nochunks \
	  --stringparam html.stylesheet=lcdproc-user.css lcdproc-user.docbook && \
	mv lcdproc-user.html current-user.html

## EOF
