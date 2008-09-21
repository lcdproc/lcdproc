#!/bin/sh

LCDPROC_DIR=${LCDPROC_DIR:-$HOME/lcdproc}
NIGHTLY_DIR=${NIGHTLY_DIR:-$LCDPROC_DIR/nightly}

MAKENIGHTLY=${NIGHTLY_DIR}/make-nightly.sh
MAKEDOCS=${NIGHTLY_DIR}/make-docs.sh
#####

for branch in current stable-0-5-x; do
	# Make nightly tarball
	${MAKENIGHTLY} $branch

	# Make docs
	for doc in dev user; do
	        ${MAKEDOCS} $branch $doc
	done
done

# EOF
