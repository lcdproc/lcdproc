#!/bin/sh
NIGHTLYDIR=/home/gfk/lcdproc/nightly
MAKENIGHTLY=${NIGHTLYDIR}/make-nightly.sh
MAKEDOCS=${NIGHTLYDIR}/make-docs.sh
#####

for branch in current stable-0-5-x; do
	# Make nightly tarball
	${MAKENIGHTLY} $branch

	# Make docs
	for doc in dev user; do
	        ${MAKEDOCS} $branch $doc
	done
done
