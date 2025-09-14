use strict;
use warnings;
use FindBin;
use Data::Dumper;
use File::Glob ':glob';
use File::Basename;
use File::Path;
use File::Copy;
#
my $debug = "";
#
sub canonpath($)
{
	my $path = shift;
	
	while ($path =~ s'[/\\]\w+[/\\]\.\.'')
	{}

	$path =~ s/\//\\/g;	
	return $path;
}
#
if (not defined($ENV{"APTHOME"}))
{
	die "error: 'APTHOME' env var must be set\n";
}
if (not defined($ENV{"EAGLHOME"}))
{
	die "error: 'EAGLHOME' env var must be set\n";
}
if (not defined($ENV{"PSPHOME"}))
{
	die "error: 'PSPHOME' env var must be set\n";
}
if (not defined($ENV{"RUNTIME_TARGET"}))
{
	die "error: 'RUNTIME_TARGET' env var must be set\n";
}
if (not defined($ENV{"GCCBIN"}))
{
	die "error: 'GCCBIN' env var must be set\n";
}
#
my $exedir			= "$FindBin::Bin";
$exedir =~ s/\//\\/g;
#
my $apthome			= $ENV{"APTHOME"};
my $eaglhome		= $ENV{"EAGLHOME"};
my $gccbin			= $ENV{"GCCBIN"};
my $sku				= $ENV{"RUNTIME_TARGET"};
#
my $outdir			= "$apthome\\build\\rms\\$sku";
#
my $bindir			= "$eaglhome\\bin.pc";
my $rendermethods	= "$eaglhome\\$sku\\rendermethods";
my $computations	= "$eaglhome\\$sku\\computations";
#
$bindir = canonpath($bindir);
$outdir = canonpath($outdir);
#
$ENV{"EAGL_${sku}_RMS_PATH"}			= "$rendermethods";
$ENV{"RMS_SEARCHPATH"}					= "$rendermethods";
#
$ENV{"EAGL_${sku}_COMPUTATION_PATH"}	= "$computations";
#
if( "$debug" ne "" )
{
	print "- dirs: bin( $bindir ) : out( $outdir )\n";
	print "- dirs: rm( $rendermethods ) : comp( $computations )\n";
}
system("del /s /f /q \"$outdir\" > NUL" );
rmdir("$outdir");
mkpath("$outdir");
#
if( "$debug" ne "" ){	print "- chdir: $eaglhome\\$sku\\rendermethods\n";	};
#
chdir("$eaglhome\\$sku\\rendermethods");
#
my @pspRmsFiles = (
					"GouraudApt.rms",
					"TextureApt.rms"
					);
					###"Texture.rms"
					###"TextureApt.rms"

foreach my $rmsFile (@pspRmsFiles)
{
	my ($basename, $path, $suffix) = fileparse($rmsFile, "\.rms");

	print "Build: [$basename] : [$path] : [$suffix] : '$basename.rms' \n";
	
	my $buildrm = "$bindir\\$sku\\buildrmd.exe -v0 -i $rmsFile -o $outdir";

	print "\n... $buildrm ...\n\n";
	if( "$debug" eq "" )
	{
		system($buildrm) and die "error: buildrm failed on '$rmsFile'";	
	}

}
#
my $outfile = "aptrm";
#
chdir($outdir);
my $cmd = "copy $exedir\\psp_rms_makefile.mak makefile";
if( "$debug" ne "" ){	print "- cmd: $cmd\n";	};
system( "$cmd" );
$cmd = "make";
system( "$cmd" );
#
print "DONE!\n";
exit 0;
# ------------------------------------------------------------------------------
#
