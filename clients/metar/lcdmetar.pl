#!/usr/bin/perl -w

# $Id$
#
# LICENSE:
# GPL - GNU Public License 
#
# Brief Description
# =================
#
# lcdmetar.pl is a program to fetch METAR Weather information (e.g.
# temperature) from a nearby (or not) airport using Geo::METAR and the
# LWP modules.
#
# Given an airport site code on the command line, lcdmetar.pl
# fetches the current temperature, wind and clound information,
#  and displays it via lcdproc (http://lcdproc.omnipotent.net)
# For fun, here are some example airports:
#
# LA     : KLAX
# Dallas : KDFW
# Detroit: KDTW
# Chicago: KMDW
# Graz/Austria (Thalerhof) : LOWG
#
# no warranty - use at your own risc. 
# DO NOT PLAN FLIGHTS ETC ON THIS INFORMATION!!!!
#


use IO::Socket;
use Fcntl;

# Get the site code.

my $site_code = shift @ARGV;

die "Usage: $0 <site_code>\n" unless $site_code;

# Get the modules we need.

use Geo::METAR;
use LWP::UserAgent;
# use strict;

# Connect to the server...
$remote = IO::Socket::INET->new(
                Proto     => "tcp",
                PeerAddr  => "localhost",
                PeerPort  => "13666",
        )
        || die "Cannot connect to LCDproc port\n";

# Make sure our messages get there right away
$remote->autoflush(1);

sleep 1;        # Give server plenty of time to notice us...

print $remote "hello\n";
my $lcdconnect = <$remote>;
#print $lcdconnect;


# Turn off blocking mode...
fcntl($remote, F_SETFL, O_NONBLOCK);

# Set up some screen widgets...
print $remote "client_set name {Metar}\n";
print $remote "screen_add metar\n";
print $remote "screen_set metar name {Metar}\n";
print $remote "widget_add metar title title\n";
print $remote "widget_set metar title {Weather $site_code}\n";
print $remote "widget_add metar temp string\n";
print $remote "widget_add metar cloud string\n";
print $remote "widget_add metar wind string\n";



my $ua = new LWP::UserAgent;

my $req = new HTTP::Request GET =>
  "http://weather.noaa.gov/cgi-bin/mgetmetar.pl?cccc=$site_code";

while (1==1) {

	my $lcdconnect = <$remote>;
	my $response = $ua->request($req);

	if (!$response->is_success) {

	    print $response->error_as_HTML;
	    my $err_msg = $response->error_as_HTML;
	    warn "$err_msg\n\n";

	} else {

	    # Yep, get the data and find the METAR.

	    my $m = new Geo::METAR;
	    my $data;
	    $data = $response->as_string;               # grap response
	    $data =~ s/\n//go;                          # remove newlines
	    $data =~ m/($site_code\s\d+Z.*?)</go;       # find the METAR string
	    my $metar = $1;                             # keep it

	    # Sanity check

	    if (length($metar)<10) {
	        die "METAR is too short! Something went wrong.";
	    }
	
	    # pass the data to the METAR module.
	    # print("$metar\n");
	    $m->metar($metar);

	    # ask for the temperature(s)
	    my $c_temp = $m->C_TEMP;
	    my $metartime = $m->TIME;
	    my $c_dew = $m->C_DEW;
	    my $f_temp = $m->F_TEMP;
	    my $wind_dir_eng = $m->WIND_DIR_DEG;
	    my $wind_mph = $m->WIND_MPH;
	    my $sky = $m->SKY;
	    my $time = localtime(time);
	    print $remote "widget_set metar title {Weather LOWG " . $metartime . "}\n";
	    print $remote "widget_set metar temp 1 2 {Temp " .$c_temp. "C (" .$c_dew. "C Dew)}\n";
	    print $remote "widget_set metar wind 1 3 {Wind " .$wind_dir_eng. "G, " .$wind_mph. "mph}\n";
	    print $remote "widget_set metar cloud 1 4 {Sky " . join(',', @{$m->{sky}}) . "}\n";
            # print "The temperature at $site_code is $c_temp C as of $time.\n";
	    # print "The wind blows to $wind_dir_eng, speed $wind_mph mph\n";
	$lcdconnect = <$remote>;

	} # end else
	for ( $i=1 ; $i < 600 ; $i++ ) {
		$lcdconnect = <$remote>;
		# printf "sleeping 1 second";
#		sleep 1;
	}
#	sleep 600;
}
close($remote);
exit;

__END__


