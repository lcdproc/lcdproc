#!/usr/local/bin/perl -w

use IO::Socket;
use Fcntl;

# Connect to the server...
$remote = IO::Socket::INET->new(
		Proto     => "tcp",
		PeerAddr  => "localhost",
		PeerPort  => "13666",
	)
	|| die "Cannot connect to LCDproc port\n";

# Make sure our messages get there right away
$remote->autoflush(1);

`sleep 1`;	# Give server plenty of time to notice us...

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
	    print $line;
	}

	undef %down;
	undef %up;
	foreach $machine       ("ZakaZak", 
				"webfoot",
				"degas", 
				"cassat", 
				"miro", 
				"monet",
				"dali",
				"escher",
				"seurat",
				"rodin",
				"matisse",
				)
	{
		`ping -c 1 $machine`;
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
