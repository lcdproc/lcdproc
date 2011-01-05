#!/usr/bin/perl -w

# x11amp.pl - an example client for LCDproc

#
# Simple client for LCDproc which controls XMMS/X11AMP MP3 player
# from the keyboard controlled by LCDproc. It can only rewind to
# previous song and skip forward to previous one.
#
#
# Copyright (c) 1999, William Ferrell, Selene Scriven
#               2001, David Glaude
#               2001, Jarda Benkovsky
#               2002, Jonathan Oxer
#               2002, Rene Wagner <reenoo@gmx.de>
#               2006, Peter Marschall
#               2006, Ethan Dicks
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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
#


# NOTE: This is just a test!  It's simple, cheesy, and doesn't do much
#       or even look very nice.
#
#       However, it demonstrates one way to handle input from the server.

use 5.005;
use strict;
use Getopt::Std;
use IO::Socket;
use Fcntl;

############################################################
# Configurable part. Set it according your setup.
############################################################

# Verbose
# 0 : None (only fatal errors)
# 1 : Warnings
# 5 : Explain every step.
my $default_verbosity = 0;
my $verbose;

# Host which runs lcdproc daemon (LCDd)
my $SERVER = "localhost";

# Port on which LCDd listens to requests
my $PORT = "13666";

# Path to command which controls XMMS/X11AMP
my $XMMS = "xmms";

# Commands to set to previous / next song
my $XMMS_FORWARD = "$XMMS --fwd";
my $XMMS_REWIND = "$XMMS --rew";
my $XMMS_PLAY_PAUSE = "$XMMS --play-pause";

############################################################
# End of user configurable parts
############################################################

# catch SIGINT and SIGTERM to invoke our cleanup routine
$SIG{INT} = \&grace;
$SIG{TERM} = \&grace;


my $progname = $0;
   $progname =~ s#.*/(.*?)$#$1#;

# declare functions
sub error($@);
sub usage($);


## main routine ##
my %opt = ();

# get options #
if (getopts('s:p:v:hV', \%opt) == 0) {
  usage(1);
}

# check options
usage(0)  if ($opt{h});
if ($opt{V}) {
  print STDERR $progname ." version 1.2\n";
  exit(0);
}

# check number of arguments
usage(1)  if ($#ARGV >= 0);


# set variables
$SERVER  = defined($opt{s}) ? $opt{s} : $SERVER;
$PORT    = defined($opt{p}) ? $opt{p} : $PORT;
$verbose = defined($opt{v}) ? $opt{v} : $default_verbosity;

# Connect to the server...
my $remote = IO::Socket::INET->new(
		Proto     => 'tcp',
		PeerAddr  => $SERVER,
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
print $lcdresponse if ($verbose >= 5);

# determine LCD size (not needed here, but useful for other clients)
($lcdresponse =~ /lcd.+wid\s+(\d+)\s+hgt\s+(\d+)/);
my $lcdwidth = $1; my $lcdheight= $2;
print "Detected LCD size of $lcdwidth x $lcdheight\n" if ($verbose >= 5);

# Turn off blocking mode...
fcntl($remote, F_SETFL, O_NONBLOCK);

# Set up some screen widgets...
print $remote "client_set name {X11AMP}\n";
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


while(1) {
	# Handle input...
	while (defined(my $line = <$remote>)) {
            chomp $line;
	    print "Received '$line'\n" if ($verbose >= 5);
	    my @items = split(/ /, $line);
	    my $command = shift @items;


	    # Use input to change songs...
	    if ($command eq 'key') {
		my $key = shift @items;

		if ($key eq 'Left') {
		    system($XMMS_REWIND);
		}
		elsif ($key eq 'Right') {
		    system($XMMS_FORWARD);
		}
		elsif ($key eq 'Enter') {
		    system($XMMS_PLAY_PAUSE);
		}
	    }
	    # And ignore everything else
	    elsif ($command eq 'connect') {
	    }
	    elsif ($command eq 'listen') {
	    }
	    elsif ($command eq 'ignore') {
	    }
	    elsif ($command eq 'success') {
	    }
	    else {
		error(0, "Huh?",  $line)
		    if ($line !~ /^\s*$/o);
	    }
	}

	# wait a bit
	sleep 1;
}

# To be called on exit and on SIGINT or SIGTERM.
sub grace() {
        print "Exiting...\n" if ($verbose >= 5);

        # release keys
	print $remote "client_del_key Left\n";
	$lcdresponse = <$remote>;
	print $remote "client_del_key Right\n";
	$lcdresponse = <$remote>;
	print $remote "client_del_key Enter\n";
	$lcdresponse = <$remote>;

        # close socket
        close($remote);
        exit;
}


## print out error message and eventually exit ##
# Synopsis:  error($status, $message)
sub error($@)
{
my $status = shift;
my @msg = @_;

  print STDERR $progname . ": " . join(" ", @msg) . "\n";

  exit($status)  if ($status);
}


## print out usage message and exit ##
# Synopsis:  usage($status)
sub usage($)
{
my $status = shift;

  print STDERR "Usage: $progname [<options>]\n";
  if (!$status) {
    print STDERR "  where <options> are\n" .
                 "    -s <server>                connect to <server> (default: $SERVER)\n" .
                 "    -p <port>                  connect to <port> on <server> (default: $PORT)\n" .
                 "    -v <verbosity>             verbosity level (default: $default_verbosity, max verbosity: 5)\n" .
		 "    -h                         show this help page\n" .
		 "    -V                         display version number\n";
  }
  else {
    print STDERR "For help, type: $progname -h\n";
  }

  exit($status);
}



__END__

=pod

=head1 NAME

x11amp.pl - LCDproc client controlling XMMS/X11AMP

=head1 SYNOPSIS

B<x11amp.pl>
[B<-s> I<server>]
[B<-p> I<port>]
[B<-v> I<verbosity>]
[B<-h>]
[B<-V>]


=head1 DESCRIPTION

B<x11amp.pl> is a simple client for LCDproc which controls an XMMS/X11AMP
MP3 player from the keypad controlled by LCDproc.
It can only rewind to the previous song, skip forward to the next song, and switch between pause and play.

=head1 OPTIONS

=over 4

=item B<-s> I<server>

Connect to the LCDd daemon at host I<server> instead of the default C<localhost>.

=item B<-p> I<port>

Use port I<port> when connecting to the LCDd server instead of the default
LCDd port C<13666>.

=item B<-v> I<verbosity>

Set message verbosity (0 is silent, 1 is warnings only, 5 describes events in detail).

=item B<-h>

Display a short help page and exit.

=item B<-V>

Display x11amp's version number and exit.

=back


=head1 SEE ALSO

L<xmms(6)>,
L<LCDd(8)>


=head1 AUTHORS

x11amp.pl was written by various members of the LCDproc project team;
this manual page was written by Peter Marschall.


=head1 BUGS

Yes, there might be some.  Please report any you find to LCDproc's mailing list.
See the website for more information.


=head1 WEBSITE

Visit B<http://www.lcdproc.org/> for more info and the latest version.

=cut

# EOF
