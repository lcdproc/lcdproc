#!/usr/bin/perl -w

# fortune.pl - an example client for LCDproc

# This is just a small example of a client for LCDd the
# LCDproc server
#
# Copyright (c) 1999, William Ferrell, Scott Scriven
#               2001, David Glaude
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

use IO::Socket;
use Fcntl;

############################################################
# Configurable part. Set it according your setup.
############################################################

# Path to `fortune' program.
$FORTUNE = "fortune";

# Host which runs lcdproc daemon (LCDd)
$HOST = "localhost";

# Port on which LCDd listens to requests
$PORT = "13666";

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
# Note: it's good practice to listen for a response after a print to the
# server even if there isn't meant to be one. If you don't, you may find
# your program crashes after running for a while when the buffers fill up:
my $lcdresponse = <$remote>;
#print $lcdresponse;


# Turn off blocking mode...
fcntl($remote, F_SETFL, O_NONBLOCK);


# Set up some screen widgets...
print $remote "client_set name {fortune.pl}\n";
$lcdresponse = <$remote>;
print $remote "screen_add fortune\n";
$lcdresponse = <$remote>;
print $remote "screen_set fortune name {Fortune}\n";
$lcdresponse = <$remote>;
print $remote "widget_add fortune title title\n";
$lcdresponse = <$remote>;
print $remote "widget_set fortune title {Fortune}\n";
$lcdresponse = <$remote>;
print $remote "widget_add fortune text scroller\n";
$lcdresponse = <$remote>;


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
		if ( $line =~ /^success$/ ) { next; }
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
    $text = `$FORTUNE` || die "\n$0: Error running `$FORTUNE'.\nPlease check that the path is correct.\n\n";
    @lines = split(/\n/, $text);
    $text = join(" / ", @lines);

    # Now, show a fortune...
    print $remote "widget_set fortune text 1 2 20 4 v 16 {$text}\n";
	my $lcdresponse = <$remote>;

}
