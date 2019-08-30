#!/bin/sh

if [ -z "$SPEC_FILE" ]; then
	# TODO: set SPEC_FILE to the installed path of your spec.eqd file
	SPEC_FILE='/usr/local/share/lcdproc/lcdproc.spec.eqd'
fi

if ! [ -f "$SPEC_FILE" ]; then
	echo "ERROR: SPEC_FILE points to non-existent file" 1>&2
	exit 1
fi

error_other_mp() {
	echo "ERROR: another mountpoint already exists on spec/sw/lcdproc/lcdproc/#0/current. Please umount first." 1>&2
	exit 1
}

if kdb mount -13 | grep -Fxq 'spec/sw/lcdproc/lcdproc/#0/current'; then
	if ! kdb mount | grep -Fxq 'lcdproc.overlay.spec.eqd on spec/sw/lcdproc/lcdproc/#0/current with name spec/sw/lcdproc/lcdproc/#0/current'; then
		error_other_mp
	fi

	MP=$(echo "spec/sw/lcdproc/lcdproc/#0/current" | sed 's:\\:\\\\:g' | sed 's:/:\\/:g')
	if [ "$(kdb get "system/elektra/mountpoints/$MP/getplugins/#5#specload#specload#/config/file")" != "$SPEC_FILE" ]; then
		error_other_mp
	fi
else
	sudo kdb mount -R noresolver "lcdproc.overlay.spec.eqd" "spec/sw/lcdproc/lcdproc/#0/current" specload "file=$SPEC_FILE"
fi

if kdb mount -13 | grep -Fxq '/sw/lcdproc/lcdproc/#0/current'; then
	if ! kdb mount | grep -Fxq 'ldcproc.conf on /sw/lcdproc/lcdproc/#0/current with name /sw/lcdproc/lcdproc/#0/current'; then
		echo "ERROR: another mountpoint already exists on /sw/lcdproc/lcdproc/#0/current. Please umount first." 1>&2
		exit 1
	fi
else
	sudo kdb spec-mount '/sw/lcdproc/lcdproc/#0/current'
fi
