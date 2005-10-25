man_MANS = lcdproc.1 lcdexec.1 LCDd.8
SUBDIRS = lcdproc-user lcdproc-dev
doxygen_input = header.html footer.html

EXTRA_DIST = README.dg \
	README.dg2 \
	README.glcdlib \
	README.imon \
	menustuff.txt \
	netstuff.txt \
	lcdproc.1 \
	lcdexec.1 \
	LCDd.8 \
	$(doxygen_input)

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
	$(RM) -r latex
	$(RM) -r html man @PACKAGE@.ps @PACKAGE@.pdf
