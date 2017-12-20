1. Update years in copyright notices in
  * server/main.c (two times)
  * clients/lcdproc/main.c

2a. Update date in
  * docs/lcdproc-user/bookinfo.docbook
  * docs/lcdproc-dev/bookinfo.docbook

2b. Update version in
  * docs/lcdproc-user/how-to-obtain.docbook

3. Add release to ChangeLog.md

4. Make a branch for the release, unless it is a stable update:
  git checkout -b lcdproc-0.5.x

5. Update version in
  * configure.ac
  * docs/lcdproc-user/bookinfo.docbook
  * docs/lcdproc-dev/bookinfo.docbook

  Major version only:
  * server/menuscreens.c
  * docs/lcdproc-dev/driver-api.docbook
  * docs/lcdproc-dev/introduction.docbook

6. Create the release tarball by running:
  sh autogen.sh
  ./configure
  make distcheck

7. Test the newly created release tarball at least once. Try to build
  and install it with all drivers enabled. Better: Try to build and
  install with several library options turned on and off.

8. Create the release notes. The release notes should be an easy to
  read summary of changes in this release. The ChangeLog file is not
  very good as release notes as it is just a chronological list of
  things that happen.
  The release notes should talk about:
   + Fingerprint (SHA-1) of the release tarball
   + Known bugs
   + New drivers
   + Other important changes within drivers, the server core and
     clients
   + Everything else the user has to know for upgrading his installed version
     grouped by topics of interest.

9a. Add the release branch on github:
  git push -u origin lcdproc-0.5.9:lcdproc-0.5.9

9b. Create the release on github. Use the release notes as description and
  upload the release tarball. You need to disable noscript entirely to
  successfully upload a file.

10. Announce the release on the website and the mailinglists:
  lcdproc@lists.omnipotent.net, lcdproc-announce@lists.sourceforge.net
