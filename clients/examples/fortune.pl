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

sleep 1;	# Give server plenty of time to notice us...

print $remote "hello\n";
my $lcdconnect = <$remote>;
#print $lcdconnect;


# Turn off blocking mode...
fcntl($remote, F_SETFL, O_NONBLOCK);


# Set up some screen widgets...
print $remote "client_set name {fortune.pl}\n";
print $remote "screen_add fortune\n";
print $remote "screen_set fortune name {Fortune}\n";
print $remote "widget_add fortune title title\n";
print $remote "widget_set fortune title {Fortune}\n";
print $remote "widget_add fortune text scroller\n";


&update_text();

# Forever, we should do stuff...
while(1)
{
	# Handle input...  (spew it to the console)
        # Also, certain keys scroll the display
	while(defined($input = <$remote>)) {
	    #print $input;

	    # Make sure we handle each line...
	    @lines = split(/\n/, $input);
	    foreach $line (@lines)
	    {
		if ( $line =~ /^success$/ ) next;
		if($line =~ /^ignore (\S)/)  # Update just after disappearing
		{
		    &update_text();
		}
	    }
	}

	# And wait a little while before we check for input again.
	sleep 1;
}

close ($remote)            || die "close: $!";
exit;

sub update_text {
    # Grab some text.
    $text = `fortune`;
    @lines = split(/\n/, $text);
    $text = join(" / ", @lines);
    
    # Now, show a fortune...
    print $remote "widget_set fortune text 1 2 20 4 v 16 {$text}\n";

}
