#!/usr/local/bin/perl -w

#
# This is just a test!  It's simple, cheesy, and doesn't do much
# or even look very nice.
#
# However, it demonstrates one way to handle input from the server.
# (although it seems buggy..  oops!  :)
#

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

sleep 1;	# Give server plenty of time to notice us...

print $remote "hello\n";
my $lcdconnect = <$remote>;
#print $lcdconnect;


# Turn off blocking mode...
fcntl($remote, F_SETFL, O_NONBLOCK);


# Set up some screen widgets...
print $remote "client_set name {X11AMP test}\n";
print $remote "screen_add x11amp\n";
print $remote "screen_set x11amp name {X11AMP test}\n";
print $remote "widget_add x11amp title title\n";
print $remote "widget_set x11amp title {X11AMP test}\n";
print $remote "widget_add x11amp one string\n";
print $remote "widget_set x11amp one 1 2 {  <-: E ; F :->}\n";


while(1)
{
	# Handle input...
	while(defined($line = <$remote>)) {
	    @items = split(" ", $line);
	    $command = shift @items;
	    # Use input to change songs...
	    if($command eq "key")
	    {
		$key = shift @items;
		if($key eq "E")
		{
		    system("x11amp --rew");
		}
		if($key eq "F")
		{
		    system("x11amp --fwd");
		}
	    }
	    # And ignore everything else
	    elsif($command eq "connect")
	    {
	    }
	    elsif($command eq "listen")
	    {
	    }
	    elsif($command eq "ignore")
	    {
	    }
	    else {
		if($line =~ /\S/) {print "Huh?  $line\n";}
	    }
	}

	sleep 1;

}

close ($remote)            || die "close: $!";
exit;
