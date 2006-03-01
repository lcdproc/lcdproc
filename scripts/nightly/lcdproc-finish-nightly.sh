#!/bin/sh
GUNZIP=/bin/gunzip
BZIP2=/usr/bin/bzip2
NIGHTLYDIR=~gfk/www-lcdproc/nightly/
#####
BRANCH=$1

cd ${NIGHTLYDIR}

# Make bzip2 archive
${GUNZIP} --stdout lcdproc-CVS-${BRANCH}.tar.gz > lcdproc-CVS-${BRANCH}.tar
${BZIP2} --force --quiet lcdproc-CVS-${BRANCH}.tar

date > last-nightly.txt

