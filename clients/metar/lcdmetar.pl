#!/usr/bin/perl -w

# Following is the POD documentation, type perldoc lcdmetar.pl to read it.
=pod

=head1 NAME

lcdMetar - Fetches METAR Weather information and send it to LCDproc.

=head1 SYNOPSIS

B<lcdmetar.pl> [I<METAR-Code>]

=head1 DESCRIPTION

B<lcdMetar> is a program to fetch METAR Weather information (e.g.
temperature) from a nearby (or not) airport using Geo::METAR and the
LWP modules.

Given an airport site code on the command line, lcdMetar
fetches the current weather observations,
and displays them via LCDproc (http://lcdproc.omnipotent.net/)

=head1 OPTIONS

=over 4

=item B<METAR-Code>

The METAR code related of the city you want weather observations.

For fun, here are some example METAR codes:

=over

=item * LA     : KLAX

=item * Dallas : KDFW

=item * Detroit: KDTW

=item * Chicago: KMDW

=item * Graz/Austria (Thalerhof) : LOWG

=item * Quebec City: CYQB

=item * Amundsen-Scott South Pole Station: NZSP

=back

More informations about METAR codes is available at:
https://www.weather.gov/media/wrh/mesowest/metar_decode_key.pdf

=back

=head1 DIAGNOSTICS

=over 4

=item Cannot connect to LCDproc port

By default, lcdMetar tries to connect to localhost port 13666. If you get this
error, that means that this is not possible to connect to this port. You can change
the port lcdMetar connects to by modifying the script's variables $host and $port.

=item METAR is too short! Something went wrong.

The METAR data we received is not what we expected, check out NOAA's web site
(https://www.aviationweather.gov/metar?gis=off) to see if something has changed.

=item Can't connect to METAR source

lcdMetar tried to fetch weather observations from NOAA's web site and failed. It
will retry in 15 minutes.

=back

=head1 REQUIRES

Perl 5.004, Geo::METAR, LWP::Simple, LWP::Protocol::https;

These are all available on CPAN: http://www.cpan.org/

=head1 DISCLAMER

Data distribution via the Internet is not considered an operational
delivery mechanism by the NWS due to our inability to insure access
to this service, therefore, the information available here shall not
be used for flight planning or other operational purposes.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

=head1 BUGS

Yes, there might be some. Please report any one you find to LCDproc's mailing list.
See the website for more informations.

=head1 WEBSITE

Visit B<http://www.lcdproc.org/> for more infos and the lastest version.

=cut



use strict;
use Geo::METAR;
use LWP::Simple;
use LWP::Protocol::https;
use IO::Socket;
use Fcntl;

############################################################
# Configurable part. Set it according your setup.
############################################################

# Verbose
# 0 : None (only fatal errors)
# 1 : Warnings
# 5 : Explain every step.
my $verbose = 1;

# METAR Code for your city/region.
my $site_code;

# Host which runs lcdproc daemon (LCDd)
my $host = "localhost";

# Port on which LCDd listens to requests
my $port = "13666";

# Metric or English data system?
# Can be either "metric", "nautical" or "english"
my $datasystem = "metric";

############################################################
# End of user configurable parts
############################################################
$SIG{INT} = \&grace;
$SIG{TERM} = \&grace;

# Get the site code.
$site_code = shift @ARGV || $site_code;
die "Usage: $0 <site_code>\n" unless $site_code;

# Connect to the server...
print "Connecting to LCDproc at $host\n" if ($verbose >= 5);
my $remote = IO::Socket::INET->new(
                Proto     => "tcp",
                PeerAddr  => $host,
                PeerPort  => $port,
        ) or die "Cannot connect to LCDproc port\n";

# Make sure our messages get there right away
$remote->autoflush(1);

sleep 1;        # Give server plenty of time to notice us...

print $remote "hello\n";
my $lcdconnect = <$remote>;
print $lcdconnect if ($verbose >= 5);
# connect LCDproc 0.4.2 protocol 0.3 lcd wid 20 hgt 4 cellwid 5 cellhgt 8
($lcdconnect =~ /lcd.+wid\s+(\d+)\s+hgt\s+(\d+)/);
my $lcdwidth = $1; my $lcdheight= $2;
print "Detected LCD size of $lcdwidth x $lcdheight\n" if ($verbose >= 5);

# Turn off blocking mode...
fcntl($remote, F_SETFL, O_NONBLOCK);

# Set up some screen widgets...
print $remote "client_set name {Metar}\n";
print $remote "screen_add metar\n";
print $remote "screen_set metar name {Metar}\n";
print $remote "widget_add metar title title\n";
print $remote "widget_set metar title {Weather $site_code}\n";
print $remote "widget_add metar temp string\n";
print $remote "widget_add metar wind string\n";
print $remote "widget_add metar visib scroller\n";
print $remote "widget_add metar cloud string\n" if ($lcdheight > 4);

while (1) {
	# fetch weather information
	# NOAA decomissioned weather.noaa.gov on 15-Jun-2016.
	#my $data = get("http://weather.noaa.gov/cgi-bin/mgetmetar.pl?cccc=$site_code");
	my $wx_url = "https://tgftp.nws.noaa.gov/data/observations/metar/stations/${site_code}.TXT";
	print "Fetching weather information from\n  '${wx_url}'\n" if ($verbose >= 5);
	my $data = get($wx_url);

	if (not $data) {
	    warn "Can't connect to METAR source." if ($verbose >= 1);
	} else {

	    # Yep, get the data and find the METAR.

	    my $m = new Geo::METAR;
	    $data =~ s/\n//go;                          # remove newlines
	    $data =~ m/($site_code\s\d+Z.*)/go;       # find the METAR string
	    my $metar = $1;                             # keep it
            print "Metar is '${metar}'\n" if ($verbose >= 5);

	    # Sanity check
		die "METAR is too short! Something went wrong." if (length($metar)<1);

	    # pass the data to the METAR module.
	    # print("$metar\n");
	    $m->metar($metar);

	    # ask for the temperature(s)
	    my $temp; my $temp_u; my $dew; my $dew_u; my $wind; my $wind_u; my $wind_dir;

	    # Set temp, dewpoint, and wind values based on requested output measurement type
	    if ($datasystem eq "nautical") {
	    	$temp = $m->C_TEMP;
	    	$temp_u = "C";
	    	$dew = $m->C_DEW;
	    	$dew_u = "C";
	    	$wind = $m->WIND_KTS;
	    	$wind_u = " Knots";
	    	$wind_dir = $m->WIND_DIR_DEG ."deg";
	    } elsif ($datasystem eq "english") {
	    	$temp = $m->F_TEMP;
	    	$temp_u = "F";
	    	$dew = $m->F_DEW;
	    	$dew_u = "F";
	    	$wind = $m->WIND_MPH;
	    	$wind_u = "mph";
	    	$wind_dir = $m->WIND_DIR_ENG;
	    } else {
	    	# Default: metric system (aka: international system)
	    	$temp = $m->C_TEMP;
	    	$temp_u = "C";
	    	$dew = $m->C_DEW;
	    	$dew_u = "C";
	    	$wind = $m->WIND_MPH * 1.609344;
	    	$wind_u = "km/h";
	    	$wind_dir = $m->WIND_DIR_ENG;
	    }

	    # Get forecast time
	    my $metartime = $m->TIME;

	    # Get sky conditions and visibility
	    my $sky = $m->SKY;
	    my $visibility = $m->VISIBILITY;

	    # print out temp, dewpoint and wind values if verbosity high enough
	    print "Converted Temp is '${temp}${temp_u}', Dewpoint is '${dew}${dew_u}', Winds are '${wind}${wind_u} ${wind_dir}'\n" if ($verbose >= 5);

	    # Define formatted strings for all widgets
	    my $w_title, my $w_temp, my $w_wind, my $w_vis, my $w_cloud;

	    # Build formatted title string for title widget
	    $w_title = sprintf("%s%s %s", ($lcdwidth < 28 ? "" : "Weather "), $site_code, $metartime);

	    # Build formatted temp/dewpoint string for temp widget
	    $w_temp  = sprintf("Temp %i%s", $temp, $temp_u);
	    $w_temp .= sprintf(" (%i%s Dew)", $dew, $dew_u) if ($dew and $dew_u);

	    # Build formatted wind string for wind widget
	    $w_wind  = sprintf("Wind %i%s, %s", $wind, $wind_u, $wind_dir) if ($wind_dir and $wind);

	    # Build formatted visibility string for visib widget
	    # (this one is more elaborate due to the numerous ways Geo::METAR formats visibility)
	    $w_vis   = sprintf("Visibility %s", $visibility);
	    $w_vis   =~ s/Visibility/Vis/        if (length($w_vis) > $lcdwidth); # try to make it fit
	    $w_vis   =~ s/\s*meters/m/             if (length($w_vis) > $lcdwidth); # try to make it fit
	    $w_vis   =~ s/\s*Statute\s+Miles/sm/ if (length($w_vis) > $lcdwidth); # try to make it fit

	    # Display widget values if verbosity high enough
	    if ($verbose >= 5) {
	        print "Sending to LCDd:\n";
	        print " '$w_title'\n";
	        print " '$w_temp'\n";
	        print " '$w_wind'\n";
	        print " '$w_cloud\n" if ($w_cloud and $lcdheight > 4);
		print " '$w_vis'\n" if ($w_vis);
	    }

	    print $remote sprintf("widget_set metar title {%s}\n", $w_title);
	    print $remote sprintf("widget_set metar temp 1 2 {%s}\n", $w_temp) if ($w_temp);
	    print $remote sprintf("widget_set metar wind 1 3 {%s}\n", $w_wind) if ($w_wind);
	    print $remote sprintf("widget_set metar visib 1 %i %i %i h 3 {%s}\n", $lcdheight, $lcdwidth, $lcdheight, $w_vis ) if ($w_vis);
	    print $remote sprintf("widget_set metar cloud 1 4 {Sky %s}\n", join(',', @{$m->{sky}}) ) if ($m->{sky} and ($lcdheight>4));
	} # end else

	# eat all input from LCDd
	while(defined(my $input = <$remote>)) { }

	print "Sleeping for 15 minutes.\n" if ($verbose >= 5);
	sleep 900;
}
# Should never go there since above is an infinite loop,
# send a SIGINT or SIGTERM to exit nicely.

# To be called on exit and on SIGINT or SIGTERM.
sub grace() {
	print "Exiting...\n" if ($verbose >= 5);
	close($remote);
	exit;
}

__END__
