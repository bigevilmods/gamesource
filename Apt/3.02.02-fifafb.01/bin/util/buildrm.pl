use strict;
use warnings;
use FindBin;
use Data::Dumper;
use File::Glob ':glob';
use File::Basename;
use File::Path;
use File::Copy;

sub canonpath($)
{
	my $path = shift;
	
	while ($path =~ s'[/\\]\w+[/\\]\.\.'')
	{}

	$path =~ s/\//\\/g;	
	return $path;
}

my $sku = 'ps2';

if (not defined($ENV{EAGLHOME}))
{
	die "error: 'EAGLHOME' env var must be set\n";
}

if (not defined($ENV{PS2HOME}) )
{
	# If using the buildrm.build file we will have a separate env var.
	if( not defined($ENV{PS2GCCBIN}) )
	{
		die "error: 'PS2HOME' env var must be set\n";
	}
}

my $bindir = "$ENV{EAGLHOME}\\bin.pc";
my $outdir = "$FindBin::Bin\\..\\..\\build\\rms\\$sku";
my $utildir = "$FindBin::Bin\\..\\util";
$bindir = canonpath($bindir);
$outdir = canonpath($outdir);
$utildir = canonpath($utildir);

$ENV{"RUNTIME_TARGET"} = $sku;
$ENV{"EAGL_${sku}_RMS_PATH"} = "$ENV{EAGLHOME}\\ps2\\rendermethods";
$ENV{"EAGL_${sku}_COMPUTATION_PATH"} = "$ENV{EAGLHOME}\\ps2\\computations";

system("del /s /f /q \"$outdir\" > NUL" );
rmdir("$outdir");
mkpath("$outdir");

my @ldFiles;
chdir("$ENV{EAGLHOME}\\$sku\\rendermethods");
my @ps2RmsFiles = ("GouraudApt.rms", 
                   "LineApt.rms", 
                   "TextureApt.rms");

foreach my $rmsFile (@ps2RmsFiles)
{
	my ($basename, $path, $suffix) = fileparse($rmsFile, "\.rms");
	
	print "Building '$basename.rms'...\n";
	
	my $buildrm = "$bindir\\$sku\\buildrm.exe -i $rmsFile -o $outdir";
	system($buildrm) and die "error: buildrm failed on '$rmsFile'";	
	
	my @m4Files = bsd_glob("$outdir\\$basename?.m4");
	foreach my $m4File (@m4Files)
	{
		my ($basename2, $pathtmp, $suffixtmp) = fileparse($m4File, "\.m4");
		
		if ($sku eq 'ps2')
		{
			my $m4 = "$bindir\\m4.exe $m4File >> $outdir\\$basename2.vsm";
			system($m4) and die "error: m4 failed on '$m4File'";		

			my $dvpasm;
			if( defined($ENV{PS2GCCBIN}) )
			{
				# if PS2GCCBin is defined, use this, it is setup by the nant build script.
				$dvpasm = "$ENV{PS2GCCBIN}\\ee-dvp-as.exe -g -als -o $outdir\\tempvo.o $outdir\\$basename2.vsm > $outdir\\$basename2.lst";
			}
			else
			{
				# This uses PS2Home for backwards compatability. (for other teams, and backwards compat.).
				my $as_exe = "$ENV{PS2HOME}\\usr\\local\\sce\\ee\\gcc\\bin\\ee-dvp-as.exe";
				if ( ! -e $as_exe )
				{
				    $as_exe = "$ENV{PS2HOME}\\sce\\ee\\gcc\\bin\\ee-dvp-as.exe";
				}
				$dvpasm = "$as_exe -g -als -o $outdir\\tempvo.o $outdir\\$basename2.vsm > $outdir\\$basename2.lst";
			}
			system($dvpasm) and die "error: dvpasm failed on '$outdir\\$basename2.vsm'\n$dvpasm";		
			
			push @ldFiles, "$outdir\\$basename2.vo";
			my $vumung = "$bindir\\$sku\\vumung.exe $outdir\\tempvo.o $outdir\\$basename2.vo";
			system($vumung) and die "error: vumung failed on '$outdir\\$basename2.vo'";
		}
		else
		{
			die "error: unknown sku '$sku'\n";
		}
	}
	
	push @ldFiles, "$outdir\\$basename.elf";
}

my $outfile = "aptrm";

my $ld = "$bindir\\ld.exe -r @ldFiles -o $outdir\\$outfile.o";
system($ld) and die "error: ld failed on '$outdir\\$outfile.o'";
copy("$outdir\\$outfile.o", "$outdir\\..\\rendermethod.o");

chdir("$outdir\\..");

my $dumper =  "..\\..\\bin\\util\\quickdump.pl";
die "Cannot find $dumper" unless ( -e $dumper  );

print "Running $dumper...\n";

open H_FILE, ">", "$FindBin::Bin\\..\\..\\source\\AptViewer\\dump_aptrmps2.h" or die "Couldn't open output headerfile";

print H_FILE "unsigned char rendermethod_o\[\] __attribute__((aligned(128))) = {\n";
print H_FILE `$dumper rendermethod.o 12` . "\n";
print H_FILE "};\n";
print H_FILE "unsigned int rendermethod_o_len = " . `$dumper rendermethod.o -L"` . ";\n" ;
close H_FILE;

my $ar;

if( defined($ENV{PS2GCCBIN}) )
{
	$ar = "$ENV{PS2GCCBIN}\\ee-ar.exe r $outdir\\$outfile.a @ldFiles";
}
else
{
	my $ar_exe = "$ENV{PS2HOME}\\usr\\local\\sce\\ee\\gcc\\bin\\ee-ar.exe";
	if ( ! -e $ar_exe )
	{
	    $ar_exe = "$ENV{PS2HOME}\\sce\\ee\\gcc\\bin\\ee-ar.exe";
	}
	$ar = "$ar_exe r $outdir\\$outfile.a @ldFiles";	
}
system($ar) and die "error: ee-ar failed on '$outdir\\$outfile.a'";

