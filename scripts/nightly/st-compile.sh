#!/bin/sh

BRANCH=$1

# name of the directory where LCDproc sources are.
DIRECTORY=/path/to/smoketest/${BRANCH}
# Platform name on what it is tested
PLATFORM=`uname -s -r -m | sed "s/ /-/g"`
# Compilation report directory
COMPILEREPORT=../st-compile-report/${PLATFORM}:${BRANCH}:

# Find out which of gmake or make to use
GMAKEPATH=`which gmake`
MAKEPATH=`which make`

if test ${GMAKEPATH} && test -f ${GMAKEPATH}; then
        MAKE=${GMAKEPATH}
else
        if test ${MAKEPATH} && test -f ${MAKEPATH}; then
                MAKE=${MAKEPATH}
        else
                echo "Can't find make..."
                exit -1;
        fi
fi

# # # # # # # # # # # # # # # # # #
#echo "Starting smoketest compile simple..."

# Versions managments
mkdir -p st-compile-report
cd ${DIRECTORY}
cvs update -d >/dev/null 2>&1
cd ..
cp -r ${DIRECTORY} ${DIRECTORY}-${PLATFORM}
cd ${DIRECTORY}-${PLATFORM}

# Real stuff
sh autogen.sh >/dev/null 2>&1
./configure --enable-drivers=all --enable-debug >/dev/null 2>&1
#echo "Compiling... Please be patient"
RESULT=`${MAKE} > smoke-make.log 2>&1 && echo yes`
if test ${RESULT}; then
        echo "Success" > ${COMPILEREPORT}result
else
        echo "Failure" > ${COMPILEREPORT}result
fi
echo `date +%s` >> ${COMPILEREPORT}result
cp config.log ${COMPILEREPORT}config.log
cp smoke-make.log ${COMPILEREPORT}smoke-make.log
cd ..
rm -rf ${DIRECTORY}-${PLATFORM}
