#!/bin/sh
GUNZIP=/bin/gunzip
BZIP2=/usr/bin/bzip2
NIGHTLYDIR=~gfk/www-lcdproc/nightly/
#####
BRANCH=$1

cd ${NIGHTLYDIR}

# Make bzip2 archive
${GUNZIP} --stdout lcdproc-GIT-${BRANCH}.tar.gz > lcdproc-GIT-${BRANCH}.tar
${BZIP2} --force --quiet lcdproc-GIT-${BRANCH}.tar

date > last-nightly.txt
