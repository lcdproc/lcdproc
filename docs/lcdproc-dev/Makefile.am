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
xmlto ?= /usr/bin/xmlto
developerguidedir ?= $(DESTDIR)$(htmldir)/developer-guide/

.PHONY: install-html-developerguide

install-html-developerguide: $(xmlto) lcdproc-dev.docbook lcdproc-dev.css
	$(mkinstalldirs) $(developerguidedir)
	$(xmlto) html -o $(developerguidedir) \
	  --stringparam html.stylesheet=lcdproc-dev.css lcdproc-dev.docbook
	cp lcdproc-dev.css $(developerguidedir)

## EOF
