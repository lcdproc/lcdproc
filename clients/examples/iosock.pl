#!/usr/bin/perl -w

use IO::Socket;
use Fcntl;

# iosock.pl - an example client for LCDproc

#
# This LCDproc client takes a list of machines to ping and and reports
# number of those which ping and number of those which do not and their list
#
#
# Copyright (c) 1999, William Ferrell, Scott Scriven
#               2001, David Glaude
#               2001, Jarda Benkovsky
#               2002, Jonathan Oxer
#               2002, Rene Wagner <reenoo@gmx.de>
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# any later version.
#
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this file; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
#


############################################################
# Configurable part. Set it according your setup.
############################################################

# Host which runs lcdproc daemon (LCDd)
$HOST = "localhost";

# Port on which LCDd listens to requests
$PORT = "13666";

# Path to `ping' command
$PING = "ping";

# list of hosts to be ping'ed
@MACHINES = ("lcdproc.omnipotent.net", 
	     "www.linux.org",
	     "www.daemonnews.org", 
	     "127.0.0.1",
	     "seurat",
	     "rodin",
	     "matisse",
	     );

############################################################
# End of user configurable parts
############################################################


# Connect to the server...
$remote = IO::Socket::INET->new(
		Proto     => "tcp",
		PeerAddr  => $HOST,
		PeerPort  => $PORT,
	)
	|| die "Cannot connect to LCDproc port\n";

# Make sure our messages get there right away
$remote->autoflush(1);

sleep (1);	# Give server plenty of time to notice us...

print $remote "hello\n";
# Note: it's good practice to listen for a response after a print to the
# server even if there isn't meant to be one. If you don't, you may find
# your program crashes after running for a while when the buffers fill up:
my $lcdresponse = <$remote>;
print $lcdresponse;


# Turn off blocking mode...
fcntl($remote, F_SETFL, O_NONBLOCK);


# Set up some screen widgets...
print $remote "client_set name {Test Client (Perl)}\n";
$lcdresponse = <$remote>;
print $remote "screen_add pings\n";
$lcdresponse = <$remote>;
print $remote "screen_set pings name {Ping Status}\n";
$lcdresponse = <$remote>;
print $remote "widget_add pings title title\n";
$lcdresponse = <$remote>;
print $remote "widget_set pings title {Ping Status}\n";
$lcdresponse = <$remote>;
print $remote "widget_add pings one string\n";
$lcdresponse = <$remote>;
print $remote "widget_add pings two string\n";
$lcdresponse = <$remote>;
print $remote "widget_set pings one 1 2 {Checking machines...}\n";
$lcdresponse = <$remote>;


my ($machine, %down, %up, $i, $list);

while(1)
{
	#print "Main loop...\n";
	# Handle input...  (just spew it to the console)
	while(defined($line = <$remote>)) {
	    if ( $line =~ /^success$/ ) { next; }
	    print $line;
	}

	undef %down;
	undef %up;
	foreach $machine (@MACHINES)
	{
		`$PING -c 1 $machine`;
		if($?) { $down{$machine} = 1; }
		else { $up{$machine} = 1; }
	}
	$i = 0;  $list = "";
	foreach $machine (keys %up)
	{
		$i++;
		$list .= "$machine, ";
	}
	print $remote "widget_set pings one 1 2 {Machines Up: $i}\n";
	my $lcdresponse = <$remote>;
	$i = 0;  $list = "";
	foreach $machine (keys %down)
	{
		$i++;
		$list .= "$machine, ";
	}
	print $remote "widget_set pings two 1 3 {Down ($i): $list}\n";
	$lcdresponse = <$remote>;
}

close ($remote)            || die "close: $!";
exit;
