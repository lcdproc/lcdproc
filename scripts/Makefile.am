## Process this file with automake to produce Makefile.in

EXTRA_DIST = README \
             init-LCDd.LSB.in \
	     init-lcdproc.LSB.in \
	     init-lcdexec.LSB.in \
	     init-lcdvc.LSB.in \
             init-LCDd.debian.in \
	     init-lcdproc.debian.in \
	     init-lcdexec.debian.in \
	     init-lcdvc.debian.in \
             init-LCDd.rpm.in \
	     init-lcdproc.rpm.in debian

dist-hook:
	rm -rf `find $(distdir)/debian -name CVS`

## EOF
