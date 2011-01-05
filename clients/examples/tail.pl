#!/usr/bin/perl -w

# tail.pl - an example client for LCDproc

#
# This client for LCDproc displays the tail of a specified file.
# It's possible to change the visible part of the file with LCDproc
# controlled keys
#
#
# Copyright (c) 1999, William Ferrell, Selene Scriven
#               2001, David Glaude
#               2001, Jarda Benkovsky
#               2002, Jonathan Oxer
#               2002, Rene Wagner <reenoo@gmx.de>
#               2008, Peter Marschall
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

# Host which runs LCDproc daemon (LCDd)
my $SERVER = "localhost";

# Port on which LCDd listens to requests
my $PORT = "13666";

############################################################
# End of user configurable parts
############################################################

# These define the visible part of the file...
my $top = 3;    # How far from the end of the file should we start by default?
my $lines = 3;  # How many lines to display by default ?
my $left = 1;   # Left/right scrolling position,
my $width = 20; # and number of characters per line to show


my $progname = $0;
   $progname =~ s#.*/(.*?)$#$1#;

# declare functions
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
usage(1)  if ($#ARGV != 0);


# set variables
$SERVER = defined($opt{s}) ? $opt{s} : $SERVER;
$PORT = defined($opt{p}) ? $opt{p} : $PORT;
my $filename = shift @ARGV;
my $slow = 1;   # Should we pause after the current frame?


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
# your program crashes after running for a while when the buffers fill up.
my $lcdresponse = <$remote>;
#print $lcdresponse;

# get width & height from server's greet message
if ($lcdresponse =~ /\bwid\s+(\d+)\b/) {
	$width = 0 + $1;
}
if ($lcdresponse =~ /\bhgt\s+(\d+)\b/) {
	$lines = (0 + $1) - 1;
	$top = $lines;
}

# Turn off blocking mode...
fcntl($remote, F_SETFL, O_NONBLOCK);

# Set up some screen widgets...
print $remote "client_set name {$progname}\n";
$lcdresponse = <$remote>;
print $remote "screen_add tail\n";
$lcdresponse = <$remote>;
print $remote "screen_set tail name {Tail $filename}\n";
$lcdresponse = <$remote>;
print $remote "widget_add tail title title\n";
$lcdresponse = <$remote>;
print $remote "widget_set tail title {Tail: $filename}\n";
$lcdresponse = <$remote>;
# create one widget called lineX (x in {1,2,...}) per line
for (my $i = 1; $i <= $lines; $i++) {
	print $remote "widget_add tail line$i string\n";
	$lcdresponse = <$remote>;
}

# NOTE: You have to ask LCDd to send you keys you want to handle
print $remote "client_add_key Left\n";
$lcdresponse = <$remote>;
print $remote "client_add_key Right\n";
$lcdresponse = <$remote>;
print $remote "client_add_key Up\n";
$lcdresponse = <$remote>;
print $remote "client_add_key Down\n";
$lcdresponse = <$remote>;


# Forever, we should do stuff...
while (1) {
	# Handle input...  (spew it to the console)
        # Also, certain keys scroll the display
	while(defined(my $input = <$remote>)) {
	    next  if ($input =~ /^success$/o);

	    #print $input;

	    $slow = -10;
	    # Make sure we handle each line...
	    my @lines = split(/\n/, $input);
	    foreach my $line (@lines) {
		if ($line =~ /^key (\S+)/) { # Keypresses are useful.
		    my $key = $1;

		    if ($key eq 'Down') {
			$top--  if ($top > $lines);
		    }
		    elsif ($key eq 'Up') {
		    	$top++;
		    }
		    elsif ($key eq 'Right') {
		    	$left++;
		    }
		    elsif ($key eq 'Left') {
			$left--  if ($left > 1);
		    }
		}
	    }
	}

	# Now, show what the file contains, if anything...
	if ( -f $filename ) {
	    # Grab some text.
	    my $right = $left + $width - 1;
	    # get the data by calling external programs tail, head & cut
	    my $text = `tail -$top $filename | head -$lines | cut -c$left-$right`;
	    my @lines = split(/\n/, $text);

	    # Now, display that text...
	    for (my $i = 0; $i < $lines; $i++) {
		my $j = $i + 1;
		my $k = $i + 2;

		# Avoid blank lines
		my $line = ($#lines < $i) ? " " : $lines[$i];
		$line = " "  if ($line =~ /^\s*$/o);

		# expand tabs to spaces
		while ((my $idx = index($line, "\t")) >= 0) {
		    use integer;	# calculate integers like in C
		    my $w = 8 + 8 * ($idx / 8) - $idx;
		    my $subst = " " x $w;

		    $line =~ s/\t/" " x $w/e;
		}

		# write to the display
		print $remote "widget_set tail line$j 1 $k {$line}\n";
		my $lcdresponse = <$remote>;
	    }
	}
	else { # If the file is unreadable, show an error
	    print $remote "widget_set tail line1 1 2 {$filename}\n";
		$lcdresponse = <$remote>;
	    print $remote "widget_set tail line2 1 3 {doesn't exist.}\n";
		$lcdresponse = <$remote>;
	    print $remote "widget_set tail line3 1 4 { }\n";
		$lcdresponse = <$remote>;
	}

	# And wait a little while before we show stuff again.
	if ($slow > 0) { sleep 1; $slow++; }
	elsif ($slow > 4) { sleep 2; $slow++; }
	elsif ($slow > 64) { sleep 4; }
	else  { $slow++; }
	# The "slow" thing just lets us have a better response time
	# while the user is pressing keys...  But while the user
	# is inactive, it gradually decreases update frequency.
}

close ($remote)  or  error(1, "close() failed");
exit;


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

  print STDERR "Usage: $progname [<options>] <file>\n";
  if (!$status) {
    print STDERR "  where <options> are\n" .
                 "    -s <server>                connect to <server> (default: $SERVER)\n" .
                 "    -p <port>                  connect to <port> on <server> (default: $PORT)\n" .
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

tail.pl -- show tail of a file on LCD


=head1 SYNOPSIS

B<tail.pl>
[B<-s> I<server>]
[B<-p> I<port>]
[B<-h>]
[B<-V>]
I<file>


=head1 DESCRIPTION

B<tail.pl> is a small example client for LCDd, the LCDproc server.

It shows the tail of th file given as parameter on the LCD.

If the LCD supports keys you can use the keys mapped to C<Up>, C<Down>,
C<Left> and C<Right> to scroll the contents shown on the screen.


=head1 OPTIONS

=over 4

=item B<-s> I<server>

Connect to the LCDd daemon at host I<server> instead of the default C<localhost>.

=item B<-p> I<port>

Use port I<port> when connecting to the LCDd server instead of the default
LCDd port C<13666>.

=item B<-h>

Display a short help page and exit.

=item B<-V>

Display tail.pl's version number and exit.

=back


=head1 SEE ALSO

L<tail(1)>,
L<LCDd(8)>


=head1 AUTHORS

tail.pl was written by various members of the LCDproc project team;
this manual page was written by Peter Marschall.

=cut

# EOF

