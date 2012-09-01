## Process this file with automake to produce Makefile.in

man_MANS = lcdproc.1 lcdexec.1 lcdvc.1 LCDd.8 lcdproc-config.5
SUBDIRS = lcdproc-user lcdproc-dev
doxygen_input = doxy-mainpage.md

EXTRA_DIST = lcdproc.1.in \
	lcdexec.1 \
	lcdvc.1.in \
	LCDd.8.in \
	lcdproc-config.5.in \
	$(doxygen_input)


## generate man pages
LCDd.8: LCDd.8.in
	sed -e 's:@SYSCONFDIR@:@sysconfdir@:g' $? > $@

lcdproc.1: lcdproc.1.in
	sed -e 's:@SYSCONFDIR@:@sysconfdir@:g' $? > $@

lcdvc.1: lcdvc.1.in
	sed -e 's:@SYSCONFDIR@:@sysconfdir@:g' $? > $@

lcdproc-config.5: lcdproc-config.5.in
	sed -e 's:@SYSCONFDIR@:@sysconfdir@:g' $? > $@

## clean generated makefiles
CLEANFILES = LCDd.8 lcdproc.1 lcdvc.1 lcdproc-config.5


## convenience targets

.PHONY: install-server-man install-client-man

install-server-man: install-man8 install-man5

install-client-man: install-man1 install-man5


.PHONY: install-html-guides install-html-developerguide install-html-userguide

install-html-guides: install-html-developerguide install-html-userguide

install-html-developerguide:
	$(MAKE) -C lcdproc-dev $@

install-html-userguide:
	$(MAKE) -C lcdproc-user $@


######################################################################
# DOXYGEN stuff
# (stolen from cppunit project (http://cppunit.sourceforge.net/)
######################################################################

# DOX is defined if installer requests dox generation.
# For now, we only install HTML documentation.
if DOX
### htmldir = $(pkgdatadir)/html
### html_DATA = html/index.html
### install-data-hook:
### 	cp -pR html/* $(DESTDIR)$(htmldir)
### # Automake's "distcheck" is sensitive to having files left over
### # after "make uninstall", so we have to clean up the install hook.
### uninstall-local:
### 	rm -rf $(DESTDIR)$(htmldir)
install-data-hook:
uninstall-local:
dox: html/index.html

else
# We repeat the three targets in both the "if" and "else" clauses
# of the conditional, because the generated makefile will contain
# references to the targets (target "install" depends on target
# "install-datahook", for example), and some make programs get upset
# if no target exists.
install-data-hook:
uninstall-local:
dox:

endif

# uncomment this if you want documentation to be build by
# default. Otherwise do it on demand.

#all-local: dox
html/index.html: Doxyfile $(doxygen_input)
	"@DOXYGEN@"
# Make tarfile to distribute the HTML documentation.
doc-dist: dox
	tar -czf $(PACKAGE)-docs-$(VERSION).tar.gz -C html .

pdf: @PACKAGE@.pdf
@PACKAGE@.pdf:
	$(MAKE) -C ./latex pdf
	ln -s ./latex/refman.ps	 @PACKAGE@.ps
	ln -s ./latex/refman.pdf @PACKAGE@.pdf

clean-local:
	rm -f -r latex
	rm -f -r html man @PACKAGE@.ps @PACKAGE@.pdf

## EOF
