#!/usr/local/bin/perl -w

use IO::Socket;
use Fcntl;

if($#ARGV < 0)
{
    print "Usage: tail.pl file\n";
    exit;
}
$filename = shift @ARGV;


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
print $lcdconnect;


# Turn off blocking mode...
fcntl($remote, F_SETFL, O_NONBLOCK);


# Set up some screen widgets...
print $remote "client_set name {Tail ($filename)}\n";
print $remote "screen_add tail\n";
print $remote "screen_set tail name {Tail $filename}\n";
print $remote "widget_add tail title title\n";
print $remote "widget_set tail title {Tail: $filename}\n";
print $remote "widget_add tail 1 string\n";
print $remote "widget_add tail 2 string\n";
print $remote "widget_add tail 3 string\n";


# These define the visible part of the file...
$top = 3;    # How far from the end of the file should we start?
$lines = 3;  # How many lines to display?
$left = 1;   # Left/right scrolling position,
$width = 20; # and number of characters per line to show

$slow = 1;   # Should we pause after the current frame?

# Forever, we should do stuff...
while(1)
{
	# Handle input...  (spew it to the console)
        # Also, certain keys scroll the display
	while(defined($input = <$remote>)) {
	    if ( $input =~ /^success$/ ) next;
	    print $input;

	    $slow = -10;
	    # Make sure we handle each line...
	    @lines = split(/\n/, $input);
	    foreach $line (@lines)
	    {
		if($line =~ /^key (\S)/)  # Keypresses are useful.
		{
		    $key = $1;
		    if($key eq "G") { $top++; }
		    if($key eq "H") 
		    { 
			if($top > $lines) {$top--; }
		    }
		    if($key eq "F") { $left++; }
		    if($key eq "E") 
		    { 
			if($left > 1) {$left--; }
		    }
		}
	    }
	}

	# Now, show what the file contains, if anything...
	if( -f $filename )
	{
	    # Grab some text.
	    $right = $left + $width - 1;
	    $text = `tail -$top $filename | head -$lines | cut -c$left-$right`;
	    @lines = split(/\n/, $text);
	    # Now, display that text...
	    for($i=0; $i<$lines; $i++)
	    {
		$j = $i + 1;
		$k = $i + 2;
		if($#lines < $i) { $line = " "; }  # Avoid blank lines
		else { $line = $lines[$i]; }
		if(!($line =~ /\S/)) { $line = " ";}
		print $remote "widget_set tail $j 1 $k {$line }\n";
	    }
	}
	else  # If the file is unreadable, show an error
	{
	    print $remote "widget_set tail one 1 2 {$filename}\n";
	    print $remote "widget_set tail two 1 3 {doesn't exist.}\n";
	    print $remote "widget_set tail three 1 4 { }\n";
	}

	# And wait a little while before we show stuff again.
	if($slow > 0) {sleep 1; $slow++;}
	elsif($slow > 4) {sleep 2; $slow++;}
	elsif($slow > 64) {sleep 4; }
	else {$slow++;}
	# The "slow" thing just lets us have a better response time
	# while the user is pressing keys...  But while the user
	# is inactive, it gradually decreases update frequency.
}

close ($remote)            || die "close: $!";
exit;
