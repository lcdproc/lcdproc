#!/usr/bin/perl -w

use IO::Socket;
use Fcntl;

#
# This LCDproc client takes a list of machines to ping and and reports
# number of those which ping and number of those which do not and their list
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
my $lcdconnect = <$remote>;
print $lcdconnect;


# Turn off blocking mode...
fcntl($remote, F_SETFL, O_NONBLOCK);


# Set up some screen widgets...
print $remote "client_set name {Test Client (Perl)}\n";
print $remote "screen_add pings\n";
print $remote "screen_set pings name {Ping Status}\n";
print $remote "widget_add pings title title\n";
print $remote "widget_set pings title {Ping Status}\n";
print $remote "widget_add pings one string\n";
print $remote "widget_add pings two string\n";
print $remote "widget_set pings one 1 2 {Checking machines...}\n";


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
	$i = 0;  $list = "";
	foreach $machine (keys %down)
	{
		$i++;
		$list .= "$machine, ";
	}
	print $remote "widget_set pings two 1 3 {Down ($i): $list}\n";
}

close ($remote)            || die "close: $!";
exit;
