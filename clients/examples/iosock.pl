#!/usr/bin/perl -w

use 5.005;
use strict;
use Getopt::Std;
use IO::Socket;
use Fcntl;

# iosock.pl - an example client for LCDproc

#
# This LCDproc client takes a list of machines to ping and and reports
# number of those which ping and number of those which do not and their list
#
#
# Copyright (c) 1999, William Ferrell, Selene Scriven
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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
#


############################################################
# Configurable part. Set it according your setup.
############################################################

# Host which runs lcdproc daemon (LCDd)
my $SERVER = "localhost";

# Port on which LCDd listens to requests
my $PORT = "13666";

# Path to `ping' command
my $PING = "ping";

# list of hosts to be ping'ed
my @MACHINES = ("lcdproc.omnipotent.net",
	     "www.linux.org",
	     "www.daemonnews.org",
	     "127.0.0.1",
	     "seurat",
	     "rodin",
	     "matisse",
	     );

############################################################
# End of user configurable parts
############################################################

my $progname = $0;
   $progname =~ s#.*/(.*?)$#$1#;

# declare functions
sub error($@);
sub usage($);


## main routine ##
my %opt = ();

# get options #
if (getopts('F:s:p:P:hV', \%opt) == 0) {
  usage(1);
}

# check options
usage(0)  if ($opt{h});
if ($opt{V}) {
  print STDERR $progname ." version 1.1\n";
  exit(0);
}

# check number of arguments
#usage(1)  if ($#ARGV >= 0);

# set variables
$SERVER = defined($opt{s}) ? $opt{s} : $SERVER;
$PORT = defined($opt{p}) ? $opt{p} : $PORT;
$PING = defined($opt{P}) ? $opt{P} : $PING;
@MACHINES = ($#ARGV >= 0) ? @ARGV : @MACHINES;

# Connect to the server...
my $remote = IO::Socket::INET->new(
		Proto     => 'tcp',
		PeerAddr  => $SERVER,
		PeerPort  => $PORT,
	)
	or  error(1, "cannot connect to LCDd daemon at $SERVER:$PORT");

# Make sure our messages get there right away
$remote->autoflush(1);

sleep (1);	# Give server plenty of time to notice us...

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
print $remote "screen_add pings\n";
$lcdresponse = <$remote>;
print $remote "screen_set pings name {Ping Status}\n";
$lcdresponse = <$remote>;
print $remote "widget_add pings title title\n";
$lcdresponse = <$remote>;
print $remote "widget_set pings title {Ping Status}\n";
$lcdresponse = <$remote>;
print $remote "widget_add pings one string\n";
$lcdresponse = <$remote>;
print $remote "widget_add pings two string\n";
$lcdresponse = <$remote>;
print $remote "widget_set pings one 1 2 {Checking machines...}\n";
$lcdresponse = <$remote>;

while (1) {
	my %pingstatus = ();	# pingstatus{host} = 1 <=> host is up
	my @list;

	#print "Main loop...\n";

	# Handle input...  (just spew it to the console)
	while (defined(my $line = <$remote>)) {
	    next  if ($line =~ /^success$/o);
	    print $line;
	}

	foreach my $machine (@MACHINES) {
		`$PING -c 1 $machine`;
		$pingstatus{$machine} = ($?) ? 0 : 1;
	}

	# count reachablei (up) machines
	@list = grep { $pingstatus{$_} == 1 } @MACHINES;
	print $remote "widget_set pings one 1 2 {Machines Up: ",scalar(@list),"}\n";
	my $lcdresponse = <$remote>;

	# count unreachable (down) machines
	@list = grep { $pingstatus{$_} == 0 } @MACHINES;
	print $remote "widget_set pings two 1 3 {Down (",scalar(@list),"): ",join(", ", @list),"}\n";
	$lcdresponse = <$remote>;

	# wait a bit
	sleep(1);
}

close($remote)  or  error(1, "close() failed: $!");
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

  print STDERR "Usage: $progname [<options>] [<host> ...]\n";
  if (!$status) {
    print STDERR "  where <options> are\n" .
                 "    -s <server>                connect to <server> (default: $SERVER)\n" .
                 "    -p <port>                  connect to <port> on <server> (default: $PORT)\n" .
                 "    -P <ping>                  use <ping> as the ping cmmand (default: $PING)\n" .
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

iosock.pl -- show reachability of hosts and display it on the LCD


=head1 SYNOPSIS

B<iosock.pl>
[B<-s> I<server>]
[B<-p> I<port>]
[B<-P> I<ping>]
[B<-h>]
[B<-V>]
[I<host> ...]


=head1 DESCRIPTION

B<iosock.pl> is a small example client for LCDd, the lcdproc server.

It tries to ping the servers given as parameters and displays on the LCD
whether they are reachable or not.
If no hosts are given as parameters a built-in list of default machines will be checked.

=head1 OPTIONS

=over 4

=item B<-s> I<server>

Connect to the LCDd daemon at host I<server> instead of the default C<localhost>.

=item B<-p> I<port>

Use port I<port> when connecting to the LCDd server instead of the default
LCDd port C<13666>.

=item B<-P> I<ping>

Use I<ping> as the command to check whether the remote hosts are reachable
instead of the default C<ping>.

Any I<ping> given using this options must understand the C<-c 1>
command line option of the default C<ping>.

=item B<-h>

Display a short help page and exit.

=item B<-V>

Display iosock.pl's version number and exit.

=back


=head1 SEE ALSO

L<LCDd(8)>


=head1 AUTHORS

iosock.pl was written by various members of the LCDproc project team;
this manual page was written by Peter Marschall.

=cut

# EOF

