#!/bin/sh
NIGHTLYDIR=/home/gfk/lcdproc/nightly
SSH=/usr/bin/ssh
MAKENIGHTLY=${NIGHTLYDIR}/make-nightly.sh
#####

for branch in current stable-0-5-x; do
	${MAKENIGHTLY} $branch
	${SSH} -q lcdproc.sourceforge.net "sh lcdproc-finish-nightly.sh $branch"
done
