## Process this file with automake to produce Makefile.in

EXTRA_DIST =	add-your-driver.docbook \
		bookinfo.docbook \
		driver-api.docbook \
		introduction.docbook \
		language.docbook \
		lcdproc-dev.css \
		lcdproc-dev.docbook \
		license.docbook \
		make-driver.docbook \
		programming.docbook \
		README.docbook \
		releasing.docbook \
		shared-files.docbook


## convenience targets
developerguidedir ?= $(DESTDIR)$(htmldir)/developer-guide/
docreleasedir = lcdproc-$(PACKAGE_VERSION)-dev-html

.PHONY: install-html-developerguide doc-release doc-online

install-html-developerguide: lcdproc-dev.docbook lcdproc-dev.css
	$(mkinstalldirs) $(developerguidedir)
	$(XMLTO) html -o $(developerguidedir) \
	  --stringparam html.stylesheet=lcdproc-dev.css lcdproc-dev.docbook
	cp lcdproc-dev.css $(developerguidedir)

doc-release: lcdproc-dev.docbook lcdproc-dev.css
	$(XMLTO) html -o $(docreleasedir) \
	--stringparam html.stylesheet=lcdproc-dev.css lcdproc-dev.docbook
	cp lcdproc-dev.css $(docreleasedir)
	$(AMTAR) -czf $(docreleasedir).tar.gz $(docreleasedir)

doc-online: lcdproc-dev.docbook
	$(XMLTO) html-nochunks \
	  --stringparam html.stylesheet=lcdproc-dev.css lcdproc-dev.docbook && \
	mv lcdproc-dev.html current-dev.html

## EOF
