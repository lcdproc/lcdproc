#!/bin/sh
# Run this to generate all the initial makefiles, etc.

# set default values for variables
: ${AUTOCONF:=autoconf}
: ${AUTOHEADER:=autoheader}
: ${AUTOMAKE:=automake}
: ${ACLOCAL:=aclocal}

DIE=0

($AUTOCONF --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`autoconf' installed."
  echo "Download the appropriate package for your distribution,"
  echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
  DIE=1
}

($AUTOMAKE --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: You must have \`automake' installed."
  echo "Get ftp://ftp.gnu.org/pub/gnu/automake-1.3.tar.gz"
  echo "(or a newer version if it is available)"
  DIE=1
  NO_AUTOMAKE=yes
}

# if no automake, don't bother testing for aclocal
test -n "$NO_AUTOMAKE" || ($ACLOCAL --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "**Error**: Missing \`aclocal'.  The version of \`automake'"
  echo "installed doesn't appear recent enough."
  echo "Get ftp://ftp.gnu.org/pub/gnu/automake-1.3.tar.gz"
  echo "(or a newer version if it is available)"
  DIE=1
}

if test "$DIE" -eq 1; then
  exit 1
fi

echo "Running aclocal ..."
$ACLOCAL
if grep "^A[CM]_CONFIG_HEADER" configure.ac >/dev/null; then
  echo "Running autoheader..."
  $AUTOHEADER
fi

echo "Running automake  ..."
$AUTOMAKE --add-missing  --copy
echo "Running autoconf ..."
$AUTOCONF
