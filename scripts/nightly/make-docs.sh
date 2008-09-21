#!/bin/sh

DIR=/home/gfk/lcdproc/nightly

# stable-0-5-x or current
BRANCH=$1

# dev or user
DOCS=$2

XMLTO=/usr/bin/xmlto
SSH=/usr/bin/ssh
RSYNC=/usr/bin/rsync

#############################
cd ${DIR}
DOCSPATH=${DIR}/${BRANCH}/docs/lcdproc-${DOCS}
DOCSFILE=${DOCSPATH}/lcdproc-${DOCS}.docbook

# Generate HTML
${XMLTO}  xhtml-nochunks ${DOCSFILE} >/dev/null  2>&1
mv lcdproc-${DOCS}.html ${BRANCH}-${DOCS}.html

# Generate PDF
#${XMLTO} pdf ${DOCSFILE} >/dev/null  2>&1
#mv lcdproc-${DOCS}.pdf ${BRANCH}-${DOCS}.pdf

# Generate TXT
${XMLTO} txt ${DOCSFILE} >/dev/null  2>&1
mv lcdproc-${DOCS}.txt ${BRANCH}-${DOCS}.txt

# Upload the thing
#${SCP} -r -qB \
${RSYNC} -qr \
${BRANCH}-${DOCS}.html ${BRANCH}-${DOCS}.txt \
gfk,lcdproc@web.sourceforge.net:htdocs/docs/

# Finish the process on the server
#${SSH} -q -l gfk shell.sourceforge.net "sh lcdproc-compress-html-docs.sh ${BRANCH}-${DOCS}-html"
