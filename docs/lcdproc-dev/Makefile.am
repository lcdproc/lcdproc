## Process this file with automake to produce Makefile.in

EXTRA_DIST =	lcdproc-dev.docbook \
		bookinfo.docbook \
		introduction.docbook \
		shared-files.docbook \
		make-driver.docbook \
		add-your-driver.docbook \
		driver-api.docbook \
		language.docbook \
		license.docbook \
		README.docbook


## convenience targets
xmlto = /usr/bin/xmlto
developerguidedir = $(DESTDIR)$(htmldir)/developer-guide/

install-html-developerguide: $(xmlto) lcdproc-dev.docbook
	$(mkinstalldirs) $(developerguidedir)
	$(xmlto) html -o $(developerguidedir) lcdproc-dev.docbook

## EOF
