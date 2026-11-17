#!/bin/sh
#
# Check that the bundled clients (lcdproc, lcdexec, lcdvc) survive LCDd
# restarts.
#
# Runs against a built source tree, with LCDd's text driver standing in for
# a display, so no display hardware is needed:
#
#   ./configure --enable-drivers=text && make
#   sh tests/client-reconnect.sh
#
# Each case starts LCDd, starts a client, restarts LCDd under it and checks
# what the new LCDd displays or logs.
#
# Set LCDPROC_TEST_PORT to use a port other than 13667.

set -u

top=$(cd "$(dirname "$0")/.." && pwd)
lcdd=$top/server/LCDd
client=$top/clients/lcdproc/lcdproc
lcdexec=$top/clients/lcdexec/lcdexec
lcdvc=$top/clients/lcdvc/lcdvc
port=${LCDPROC_TEST_PORT:-13667}
# The screen the client shows, and text that identifies it on the display
screen=C
marker="CPU LOAD"

for f in "$lcdd" "$client" "$lcdexec" "$lcdvc" "$top/server/drivers/text.so"; do
	if [ ! -x "$f" ] && [ ! -f "$f" ]; then
		echo "missing $f: build first (./configure --enable-drivers=text && make)" >&2
		exit 2
	fi
done

work=$(mktemp -d "${TMPDIR:-/tmp}/lcdproc-test.XXXXXX") || exit 2
lcdd_pid=
client_pid=
failed=0

cleanup() {
	[ -n "$client_pid" ] && kill "$client_pid" 2>/dev/null
	[ -n "$lcdd_pid" ] && kill "$lcdd_pid" 2>/dev/null
	wait 2>/dev/null
	rm -rf "$work"
}
trap cleanup EXIT
trap 'exit 1' INT TERM

cat > "$work/LCDd.conf" <<EOF
[server]
DriverPath=$top/server/drivers/
Driver=text
Bind=127.0.0.1
Port=$port
ServerScreen=no
[text]
Size=20x4
EOF

# start_lcdd OUTPUT: the display goes to OUTPUT, the log to OUTPUT.log
start_lcdd() {
	"$lcdd" -c "$work/LCDd.conf" -f -s 0 -r 4 > "$work/$1" 2> "$work/$1.log" &
	lcdd_pid=$!
	sleep 1
}

stop_lcdd() {
	kill "$lcdd_pid" 2>/dev/null
	wait "$lcdd_pid" 2>/dev/null
	lcdd_pid=
}

# start_client RECONNECTDELAY
start_client() {
	printf '[lcdproc]\nServer=127.0.0.1\nPort=%s\nReconnectDelay=%s\n' "$port" "$1" \
		> "$work/lcdproc.conf"
	# Turn the default screens off, then the one under test on.
	"$client" -c "$work/lcdproc.conf" -f '!C' '!M' '!L' '!T' '!A' "$screen" \
		2> "$work/client.log" &
	client_pid=$!
	sleep 3
}

stop_client() {
	kill "$client_pid" 2>/dev/null
	wait "$client_pid" 2>/dev/null
	client_pid=
}

# A client that has exited but not been waited for is a zombie, which kill -0
# still finds, so look at its state instead.
client_alive() {
	state=$(ps -o stat= -p "$client_pid" 2>/dev/null)
	[ -n "$state" ] && [ "${state#Z}" = "$state" ]
}

# The text driver's output is buffered, so only read it once that LCDd has
# exited.
shows_client() {
	grep -q "$marker" "$work/$1"
}

fd_count() {
	ls "/proc/$client_pid/fd" 2>/dev/null | wc -l
}

result() {
	if [ "$1" = ok ]; then
		echo "PASS: $2"
	else
		echo "FAIL: $2"
		sed 's/^/    client: /' "$work/client.log"
		failed=1
	fi
}

# 1. A quick LCDd restart: the client reconnects and sets its screen up again.
start_lcdd first.out
start_client 1
stop_lcdd
start_lcdd second.out
sleep 3
alive=no
client_alive && alive=yes
stop_lcdd
if shows_client first.out; then
	result ok "client shows its screen"
else
	result fail "client shows its screen"
fi
if [ $alive = yes ] && shows_client second.out; then
	result ok "client reconnects after an LCDd restart"
else
	result fail "client reconnects after an LCDd restart"
fi
stop_client

# 2. A longer outage, with many failed connection attempts: the client keeps
#    trying, and failed attempts don't leak descriptors.
start_lcdd first.out
start_client 1
before=$(fd_count)
stop_lcdd
sleep 6
during=$(fd_count)
start_lcdd second.out
sleep 3
after=$(fd_count)
alive=no
client_alive && alive=yes
stop_lcdd
if [ $alive = yes ] && shows_client second.out; then
	result ok "client reconnects after a longer LCDd outage"
else
	result fail "client reconnects after a longer LCDd outage"
fi
if [ -d /proc/self/fd ]; then
	# While LCDd is down the client holds one descriptor fewer (no
	# connection); once reconnected, the count is back where it started.
	if [ "$during" -lt "$before" ] && [ "$after" -eq "$before" ]; then
		result ok "no descriptor leak across failed attempts ($before, $during, $after)"
	else
		result fail "no descriptor leak across failed attempts ($before, $during, $after)"
	fi
fi
stop_client

# 3. A screen that updates rarely (Disk, every 256 ticks) is set up again
#    straight away too, not only when its timer next runs out.
screen=D
marker="DISKS"
start_lcdd first.out
start_client 1
stop_lcdd
start_lcdd second.out
sleep 3
stop_lcdd
if shows_client first.out && shows_client second.out; then
	result ok "slow-updating screen comes back straight after a reconnect"
else
	result fail "slow-updating screen comes back straight after a reconnect"
fi
stop_client
screen=C
marker="CPU LOAD"

# 4. ReconnectDelay=0: the client exits as soon as LCDd drops the connection.
start_lcdd first.out
start_client 0
stop_lcdd
sleep 2
if client_alive; then
	result fail "ReconnectDelay=0: client exits when LCDd goes away"
else
	result ok "ReconnectDelay=0: client exits when LCDd goes away"
fi
stop_client

# lcdexec has no screens of its own, only menu entries in LCDd's menu, which
# LCDd logs at level 4 as "Client [N] is using the menu".

# start_lcdexec RECONNECTDELAY
start_lcdexec() {
	cat > "$work/lcdexec.conf" <<EOF
[lcdexec]
Address=127.0.0.1
Port=$port
Foreground=true
Shell=/bin/sh
ReconnectDelay=$1
[MainMenu]
Entry=Hello
[Hello]
DisplayName=Hello
Exec="true"
EOF
	"$lcdexec" -c "$work/lcdexec.conf" 2> "$work/client.log" &
	client_pid=$!
	sleep 2
}

has_menu() {
	grep -q "is using the menu" "$work/$1.log"
}

# 5. lcdexec reconnects and sets its menu up again.
start_lcdd first.out
start_lcdexec 1
stop_lcdd
start_lcdd second.out
sleep 3
alive=no
client_alive && alive=yes
stop_lcdd
if [ $alive = yes ] && has_menu first.out && has_menu second.out; then
	result ok "lcdexec reconnects and sets its menu up again"
else
	result fail "lcdexec reconnects and sets its menu up again"
fi
stop_client

# 6. lcdexec with ReconnectDelay=0 exits with an error when LCDd goes away.
start_lcdd first.out
start_lcdexec 0
stop_lcdd
sleep 2
if client_alive; then
	result fail "lcdexec ReconnectDelay=0: exits with an error when LCDd goes away"
else
	wait "$client_pid"
	status=$?
	if [ $status -ne 0 ]; then
		result ok "lcdexec ReconnectDelay=0: exits with an error when LCDd goes away"
	else
		result fail "lcdexec ReconnectDelay=0: exits with an error when LCDd goes away (status 0)"
	fi
fi
client_pid=

# lcdvc copies a virtual console to the display. Regular files stand in for
# /dev/vcs and /dev/vcsa: the vcsa header gives the console size (4 lines
# of 20) and cursor position, and the vcs file its text.

# start_lcdvc RECONNECTDELAY
start_lcdvc() {
	printf '\004\024\000\000' > "$work/vcsa"
	printf '%-20s%-60s' 'lcdvc console text' '' > "$work/vcs"
	cat > "$work/lcdvc.conf" <<EOF
[lcdvc]
Address=127.0.0.1
Port=$port
Foreground=true
ReconnectDelay=$1
vcsDevice=$work/vcs
vcsaDevice=$work/vcsa
EOF
	"$lcdvc" -c "$work/lcdvc.conf" 2> "$work/client.log" &
	client_pid=$!
	sleep 3
}

# 7. lcdvc reconnects and shows the console again.
marker="lcdvc console text"
start_lcdd first.out
start_lcdvc 1
stop_lcdd
start_lcdd second.out
sleep 3
alive=no
client_alive && alive=yes
stop_lcdd
if [ $alive = yes ] && shows_client first.out && shows_client second.out; then
	result ok "lcdvc reconnects and shows the console again"
else
	result fail "lcdvc reconnects and shows the console again"
fi
stop_client

# 8. lcdvc with ReconnectDelay=0 exits with an error when LCDd goes away.
start_lcdd first.out
start_lcdvc 0
stop_lcdd
sleep 2
if client_alive; then
	result fail "lcdvc ReconnectDelay=0: exits with an error when LCDd goes away"
else
	wait "$client_pid"
	status=$?
	if [ $status -ne 0 ]; then
		result ok "lcdvc ReconnectDelay=0: exits with an error when LCDd goes away"
	else
		result fail "lcdvc ReconnectDelay=0: exits with an error when LCDd goes away (status 0)"
	fi
fi
client_pid=

exit $failed
