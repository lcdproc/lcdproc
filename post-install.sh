#!/bin/sh

INSTALL_PREFIX="/usr/local"

exit_usage() {
    echo "Usage: $(basename "$0") [-h|--help] [app...]" 1>&2
    echo "" 1>&2
    echo "supported apps: lcdproc lcdexec lcdvc LCDd" 1>&2
    echo "if no app is given: use all apps" 1>&2
    exit 1
}

if [ "$#" -eq 0 ]; then
    APPS="lcdproc lcdexec lcdvc LCDd"
elif [ "$#" -eq 1 ] && { [ "$1" = "-h" ] || [ "$1" = "--help" ]; }; then
    exit_usage
else
    APPS=$*
fi

run_mount_script() {
    SPEC_FILE="$INSTALL_PREFIX/share/lcdproc/$1.spec.eqd" sh "$INSTALL_PREFIX/libexec/$1.mount.sh"
}

for APP in $APPS; do
    case "$APP" in
    lcdproc)
        if [ ! "$lcdproc_done" ]; then
            run_mount_script lcdproc
        fi
        lcdproc_done=1
        ;;
    lcdexec)
        if [ ! "$lcdexec_done" ]; then
            run_mount_script lcdexec
        fi
        lcdexec_done=1
        ;;
    lcdvc)
        if [ ! "$lcdvc_done" ]; then
            run_mount_script lcdvc
        fi
        lcdvc_done=1
        ;;
    LCDd)
        if [ ! "$LCDd_done" ]; then
            run_mount_script LCDd && kdb set "/sw/lcdproc/lcdd/#0/current/server/driverpath" "$INSTALL_PREFIX/lib/lcdproc/"
        fi
        LCDd_done=1
        ;;
    *)
        echo "Unknown application $APP" 1>&2
        exit_usage
        ;;
    esac
done
