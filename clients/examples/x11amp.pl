#!/usr/bin/perl -w

#
# Simple client for LCDproc which controls XMMS/X11AMP MP3 player
# from the keyboard controlled by LCDproc. It can only rewind to
# previous song and skip forward to previous one.
#
# This is just a test!  It's simple, cheesy, and doesn't do much
# or even look very nice.
#
# However, it demonstrates one way to handle input from the server.
# (although it seems buggy..  oops!  :)
#

use IO::Socket;
use Fcntl;

############################################################
# Configurable part. Set it according your setup.
############################################################

# Host which runs lcdproc daemon (LCDd)
$HOST = "localhost";

# Port on which LCDd listens to requests
$PORT = "13666";

# Path to command which controls XMMS/X11AMP
$XMMS = "xmms";

# Commands to set to previous / next song
$XMMS_FORWARD = "$XMMS --fwd";
$XMMS_REWIND = "$XMMS --rew"

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
		    system($XMMS_REWIND);
		}
		if($key eq "F")
		{
		    system($XMMS_FORWARD);
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
	    elsif($command eq "success")
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
