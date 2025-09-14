#
# get_gperf.pl		(c) 2009 Electronic Arts
#
# This script downloads the Win32 executable gperf.exe from SourceForge.
# You can change the gperf version, URL and filename below, if you need to.
#

use LWP;
use Encode;
use Archive::Zip;

#
# Download gperf executable
#

print "Downloading gperf binaries...\n";

# NOTE: Apt only has an Open Source Deal Sheet for gperf version 2.7.2.
# IF YOU CHANGE THIS VERSION YOU MUST SUBMIT YOUR OWN OPEN SOURCE DEAL SHEET.
my $gperf_version = "2.7.2";

my $filename = 'gperf-' . $gperf_version . '-bin.zip';
my $url = 'http://downloads.sourceforge.net/sourceforge/gnuwin32/' . $filename;

my $browser = LWP::UserAgent->new;
my $response = $browser->get( $url );

die "Failed to get URL '$url'. Error: " . $response->status_line unless $response->is_success;
die "Unexpected MIME type '$response->content_type'" unless ($response->content_type == 'application/zip');

open FILE, ">$filename" || die "Could not open output file '$filename' for writing.";
binmode FILE;
my $bytes = $response->content;
my $len = @bytes;
die "Failed to write '$filename'" unless ( defined( syswrite FILE, $bytes ) );

close FILE;

#
# Unzip gperf
#

print "Unzipping $filename...\n";

my $zip = Archive::Zip->new();
die "Cannot find '$filename'" unless ( -e $filename );
die "Failed while reading from zip file '$filename'" unless ( $zip->read( $filename ) == AZ_OK );
die "Failed to unzip '$filename'" unless ( $zip->extractTree( '', 'gperf-' . $gperf_version . '-bin/' ) == AZ_OK );

