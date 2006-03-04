#!/usr/bin/perl -w

# x11amp.pl - an example client for LCDproc

#
# Simple client for LCDproc which controls XMMS/X11AMP MP3 player
# from the keyboard controlled by LCDproc. It can only rewind to
# previous song and skip forward to previous one.
#
#
# Copyright (c) 1999, William Ferrell, Scott Scriven
#               2001, David Glaude
#               2001, Jarda Benkovsky
#               2002, Jonathan Oxer
#               2002, Rene Wagner <reenoo@gmx.de>
#               2006, Peter Marschall
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


# NOTE: This is just a test!  It's simple, cheesy, and doesn't do much
#       or even look very nice.
#
#       However, it demonstrates one way to handle input from the server.
#       (although it seems buggy..  oops!  :)


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
$XMMS_REWIND = "$XMMS --rew";
$XMMS_PLAY_PAUSE = "$XMMS --play-pause";

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
print $remote "client_set name {X11AMP test}\n";
$lcdresponse = <$remote>;
print $remote "screen_add x11amp\n";
$lcdresponse = <$remote>;
print $remote "screen_set x11amp name {X11AMP test}\n";
$lcdresponse = <$remote>;
print $remote "widget_add x11amp title title\n";
$lcdresponse = <$remote>;
print $remote "widget_set x11amp title {$XMMS test}\n";
$lcdresponse = <$remote>;
print $remote "widget_add x11amp one string\n";
$lcdresponse = <$remote>;
print $remote "widget_set x11amp one 1 2 {[<<]  [>|]  [>>]}\n";
$lcdresponse = <$remote>;

# NOTE: You have to ask LCDd to send you keys you want to handle
print $remote "client_add_key Left\n";
$lcdresponse = <$remote>;
print $remote "client_add_key Right\n";
$lcdresponse = <$remote>;
print $remote "client_add_key Enter\n";
$lcdresponse = <$remote>;


while(1)
{
	# Handle input...
	while(defined($line = <$remote>)) {
	    @items = split(" ", $line);
	    $command = shift @items;
	    # Use input to change songs...
	    if($command eq "key") {
		my $key = shift @items;

		if($key eq "Left") {
		    system($XMMS_REWIND);
		}
		elsif($key eq "Right") {
		    system($XMMS_FORWARD);
		}
		elsif($key eq "Enter") {
		    system($XMMS_PLAY_PAUSE);
		}
	    }
	    # And ignore everything else
	    elsif($command eq "connect") {
	    }
	    elsif($command eq "listen") {
	    }
	    elsif($command eq "ignore") {
	    }
	    elsif($command eq "success") {
	    }
	    else {
		if($line =~ /\S/) {print "Huh?  $line\n";}
	    }
	}

	sleep 1;

}

close ($remote)  ||  die "close: $!";
exit;


__END__

=pod

=head1 NAME

x11amp - LCDproc client controlling XMMS/X11AMP

=head1 SYNOPSIS

B<x11amp.pl>

=head1 DESCRIPTION

x11amp is a simple client for LCDproc which controls an XMMS/X11AMP
MP3 player from the keyboard controlled by LCDproc.
It can only rewind to the previous song previous song, skip forward
to the next song and switch between pause and play.

=head1 REQUIRES

Perl >= 5.005, IO::Socket, Fcntl

These are all available on CPAN: http://www.cpan.org/.

=head1 DISCLAMER

This program is free software; you can redistribute it and/or modify 
it under the terms of the GNU General Public License version 2 as 
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
for more details.

You should have received a copy of the GNU General Public License along 
with this program; if not, write to the Free Software Foundation, Inc., 
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

=head1 BUGS

Yes, there might be some. Please report any one you find to LCDproc's mailing list.
See the website for more information.

=head1 WEBSITE

Visit B<http://www.lcdproc.org/> for more infos and the lastest version.

=cut

# EOF
