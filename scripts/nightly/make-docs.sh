#!/bin/sh

SF_ACCOUNT=${SF_ACCOUNT:-$USER}
LCDPROC_DIR=${LCDPROC_DIR:-$HOME/lcdproc}
NIGHTLY_DIR=${NIGHTLY_DIR:-$LCDPROC_DIR/nightly}

# stable-0-5-x or current (default)
BRANCH=${1:-current}

# dev or user (default)
DOCS=${2:-user}

XMLTO=/usr/bin/xmlto
RSYNC=/usr/bin/rsync

DOCSPATH=${NIGHTLY_DIR}/${BRANCH}/docs/lcdproc-${DOCS}
DOCSFILE=${DOCSPATH}/lcdproc-${DOCS}.docbook
CSSFILE=${DOCSPATH}/lcdproc-${DOCS}.css

#############################
test -d ${NIGHTLY_DIR} && cd ${NIGHTLY_DIR}

# Generate HTML
${XMLTO} xhtml-nochunks --stringparam html.stylesheet=${CSSFILE} ${DOCSFILE} >/dev/null 2>&1
mv lcdproc-${DOCS}.html ${BRANCH}-${DOCS}.html

# Generate PDF
#${XMLTO} pdf ${DOCSFILE} >/dev/null 2>&1
#mv lcdproc-${DOCS}.pdf ${BRANCH}-${DOCS}.pdf

# Generate TXT
${XMLTO} txt ${DOCSFILE} >/dev/null 2>&1
mv lcdproc-${DOCS}.txt ${BRANCH}-${DOCS}.txt

# Upload to sf.net
${RSYNC} -qr \
	${BRANCH}-${DOCS}.html ${BRANCH}-${DOCS}.txt \
	${SF_ACCOUNT},lcdproc@web.sourceforge.net:htdocs/docs/

# EOF
