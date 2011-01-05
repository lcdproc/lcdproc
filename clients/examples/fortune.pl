#!/usr/bin/perl -w

# fortune.pl - an example client for LCDproc

# This is just a small example of a client for LCDd the
# LCDproc server
#
# Copyright (c) 1999, William Ferrell, Selene Scriven
#               2001, David Glaude
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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
#

use 5.005;
use strict;
use Getopt::Std;
use IO::Socket;
use Fcntl;

############################################################
# Configurable part. Set it according your setup.
############################################################

# Path to `fortune' program.
my $FORTUNE = "fortune";

# Host which runs lcdproc daemon (LCDd)
my $SERVER = "localhost";

# Port on which LCDd listens to requests
my $PORT = "13666";

############################################################
# End of user configurable parts
############################################################

my $progname = $0;
   $progname =~ s#.*/(.*?)$#$1#;

# declare functions
sub update_text($);
sub error($@);
sub usage($);


## main routine ##
my %opt = ();

# get options #
if (getopts('F:s:p:hV', \%opt) == 0) {
  usage(1);
}

# check options
usage(0)  if ($opt{h});
if ($opt{V}) {
  print STDERR $progname ." version 1.1\n";
  exit(0);
}

# check number of arguments
usage(1)  if ($#ARGV >= 0);


# set variables
$SERVER = defined($opt{s}) ? $opt{s} : $SERVER;
$PORT = defined($opt{p}) ? $opt{p} : $PORT;
$FORTUNE = defined($opt{F}) ? $opt{F} : $FORTUNE;

# Connect to the server...
my $remote = IO::Socket::INET->new(
		Proto     => 'tcp',
		PeerAddr  => $SERVER,
		PeerPort  => $PORT,
	)
	or  error(1, "cannot connect to LCDd daemon at $SERVER:$PORT");

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
print $remote "client_set name {$progname}\n";
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

update_text($remote);

# Forever, we should do stuff...
while (1)
{
	# Handle input...  (spew it to the console)
        # Also, certain keys scroll the display
	while (defined(my $input = <$remote>)) {
	    #print $input;

	    # Make sure we handle each line...
	    my @lines = split(/\n/, $input);

	    foreach my $line (@lines) {
		next  if ( $line =~ /^success$/ );

		# Update just after disappearing
		if ($line =~ /^ignore (\S)/) {
		    update_text($remote);
	        }
	    }
	}

	# And wait a little while before we check for input again.
	sleep 1;
}

close($remote)  or  error(1, "close() failed: $!");
exit;


# update text on LCD
sub update_text($)
{
my $remote = shift;
my $text;

    # Grab some text
    $text = `$FORTUNE` || error(1, "error running `$FORTUNE'.\nPlease check that the path is correct.");
    # replace new-line by slashes
    $text =~ s,\n, / ,g;

    # Now, show a fortune...
    print $remote "widget_set fortune text 1 2 20 4 v 16 {$text}\n";
    $text = <$remote>;
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
                 "    -F <fortune>               use <fortune> as fortune program (default: $FORTUNE)\n" .
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

fortune.pl -- display fortune messages on the LCD


=head1 SYNOPSIS

B<fortune.pl>
[B<-s> I<server>]
[B<-p> I<port>]
[B<-F> I<fortune>]
[B<-h>]
[B<-V>]


=head1 DESCRIPTION

B<fortune.pl> is a small example client for LCDd, the lcdproc server.

It connects to the LCDd daemon and displays fortune cookies as delivered
by the well known fortune program.


=head1 OPTIONS

=over 4

=item B<-s> I<server>

Connect to the LCDd daemon at host I<server> instead of the default C<localhost>.

=item B<-p> I<port>

Use port I<port> when connecting to the LCDd server instead of the default
LCDd port C<13666>.

=item B<-F> I<fortune>

Use I<fortune> as the program generation fortune messages instead of the
default C<fortune>

=item B<-h>

Display a short help page and exit.

=item B<-V>

Display fortune.pl's version number and exit.

=back


=head1 SEE ALSO

L<fortune(6)>,
L<LCDd(8)>


=head1 AUTHORS

fortune.pl was written by various members of the LCDproc project team;
this manual page was written by Peter Marschall.

=cut

# EOF

