#!/usr/bin/perl -w

# Copyright (c) 2008, Ethan Dicks <ethan.dicks@gmail.com>
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

# Following is the POD documentation, type perldoc lcdident.pl to read it.

=head1 NAME

lcdident - identify LCDproc displays

=head1 SYNOPSIS

B<lcdident.pl>
[B<-s> I<server>]
[B<-p> I<port>]
[B<-t> I<seconds>]
[B<-v> I<verbosity>]
[B<-h>]
[B<-V>]

=head1 DESCRIPTION

B<lcdident> is a simple LCDproc client program that writes a port
number and display geometry to a particular instance of B<LCDd>.  For
systems with multiple display modules and multiple B<LCDd> processes,
it can be quite handy to know which TCP port is associated with
which physical module.


B<lcdident> is also an example of how to code for different display
geometries.  In particular, it differentiates between 2-line and
more-than-2-line (typically 4-line) displays.

=head1 OPTIONS

 -s <server>
        connect to <server>.

 -p <port>
        connect to port <port>

 -t <seconds>
	how long to keep this client running (default is 5 seconds).

 -v <verbosity>
        set logging verbosity level to <verbosity>.

 -V
        display the present version number of lcdident.

 -h
        print out a list of the available program options and exit.


=head1 DIAGNOSTICS

=over 4

=item Cannot connect to LCDproc port

By default, lcdident tries to connect to localhost port 13666, but by using
the B<-s> and B<-p> options, you can point B<lcdident> at other servers and
other ports.  If you get this error, that means that there was a problem
opening up the requested port on the requested machine.  Typically this
means that B<LCDd> is not listening on that particular port.  The two most
common causes of this are because B<LCDd> was configured to listen on a
different port, or because B<LCDd> is not running at all.

=back

=head1 REQUIRES

Perl 5.004;

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
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301

=head1 SEE ALSO

L<LCDd>

=head1 BUGS

Yes, there might be some. Please report any you find to LCDproc's mailing list.
See the website for more information.

=head1 AUTHORS

B<lcdident> was written by Ethan Dicks <ethan.dicks@gmail.com>

=head1 WEBSITE

Visit B<http://www.lcdproc.org/> for more info and the lastest version.

=cut

use 5.004;
use strict;
use IO::Socket;
use Getopt::Std;
use Fcntl;

############################################################
# Configurable part. Set it according your setup
# or use input args.
############################################################

# Verbose
# 0 : None (only fatal errors)
# 1 : Warnings
# 5 : Explain every step.
my $verbose = 0;

# Host which runs lcdproc daemon (LCDd)
my $server = "localhost";

# Port on which LCDd listens to requests
my $port = "13666";

# wait-time (in seconds) when we are all done
my $waittime = 5;

############################################################
# End of user configurable parts
############################################################
$SIG{INT} = \&grace;
$SIG{TERM} = \&grace;

my $progname = $0;
   $progname =~ s#.*/(.*?)$#$1#;

# declare functions
sub grace();
sub usage($);

# Process input arguments
my %opt = ();

# get options #
if (getopts('s:p:v:t:hV', \%opt) == 0) {
    usage(1);
}

# check options
usage(0)  if ($opt{h});

if ($opt{V}) {
    print STDERR $progname ." version 1.05\n";
    exit(0);
}

# check number of arguments
usage(1)  if ($#ARGV >= 0);

# set variables
$server  = defined($opt{s}) ? $opt{s} : $server;
$port    = defined($opt{p}) ? $opt{p} : $port;
$verbose = defined($opt{v}) ? $opt{v} : $verbose;
$waittime= defined($opt{t}) ? $opt{t} : $waittime;


# Connect to the server...
print "Connecting to LCDproc at $server\n" if ($verbose >= 5);
my $remote = IO::Socket::INET->new(
                Proto     => "tcp",
                PeerAddr  => $server,
                PeerPort  => $port,
        ) or die "Cannot connect to LCDproc port ($server:$port)\n";

# Make sure our messages get there right away
$remote->autoflush(1);

sleep 1;        # Give server plenty of time to notice us...

print $remote "hello\n";
my $lcdconnect = <$remote>;
print $lcdconnect if ($verbose >= 5);
# connect LCDproc 0.5.2 protocol 0.3 lcd wid 20 hgt 4 cellwid 5 cellhgt 8
($lcdconnect =~ /lcd.+wid\s+(\d+)\s+hgt\s+(\d+)/);
my $lcdwidth = $1; my $lcdheight= $2;
print "Detected LCD size of $lcdwidth x $lcdheight\n" if ($verbose >= 5);

# Turn off blocking mode...
fcntl($remote, F_SETFL, O_NONBLOCK);

# Set up some screen widgets...
print $remote "client_set name {lcdident}\n";
print $remote "screen_add lcdident\n";
print $remote "screen_set lcdident name {lcdident}\n";

print $remote "widget_add lcdident title title\n";
print $remote "widget_set lcdident title {lcdident}\n";

print $remote "widget_add lcdident ident1 string\n";
print $remote "widget_add lcdident ident2 string\n";


# Compose the ident message(s) and calculate where to render it/them
my $ident1;
my $ident2;
my $ident2_y = $lcdheight;     # put the second bit on the bottom line
my $ident1_y = $ident2_y - 1;  # put the first bit one line up from the second

# check for 2-line displays to see if we have to put all the info on one line
if ($lcdheight > 2) {
	$ident1 = "Port ${port}";
	$ident2 = "Geom ${lcdwidth}x${lcdheight}";
} else {
	$ident1 = "Port $port ${lcdwidth}x${lcdheight}";
	$ident2 = "";

	$ident1_y = $lcdheight;  # use the bottom line
}

# Display the ident message(s)
if ($ident1) {
	my $cmd = sprintf("widget_set lcdident ident1 1 %d {%s}", $ident1_y, $ident1);
	print "Cmd is '$cmd'\n" if ($verbose >= 5);
	print $remote $cmd, "\n";
}
if ($ident2) {
	my $cmd = sprintf("widget_set lcdident ident2 1 %d {%s}", $ident2_y, $ident2);
	print "Cmd is '$cmd'\n" if ($verbose >= 5);
	print $remote $cmd, "\n";
}

# hold the client screen up for as long as we were asked (default 5 seconds);
print "Keeping screen up for $waittime seconds\n" if ($verbose >= 5);
sleep($waittime);

# eat all input from LCDd
while(defined(my $input = <$remote>)) { }

# all done
grace();

# send a SIGINT or SIGTERM to exit nicely.

# To be called on exit and on SIGINT or SIGTERM.
sub grace() {
	print "Exiting...\n" if ($verbose >= 5);
	close($remote);
	exit;
}

## print out usage message and exit ##
# Synopsis:  usage($status)
sub usage($)
{
my $status = shift;

  print STDERR "Usage: $progname [<options>]\n";
  if (!$status) {
    print STDERR "  where <options> are\n" .
                 "    -s <server>                connect to <server> (default: $server)\n" .
                 "    -p <port>                  connect to <port> on <server> (default: $port)\n" .
                 "    -t <seconds>               how long to hold the client screen up (default: $waittime seconds).\n" .
                 "    -v <verbosity>             verbosity level (default: $verbose, max verbosity: 5)\n" .
                 "    -h                         show this help page\n" .
                 "    -V                         display version number\n";
  }
  else {
    print STDERR "For help, type: $progname -h\n";
  }

  exit($status);
}

__END__
