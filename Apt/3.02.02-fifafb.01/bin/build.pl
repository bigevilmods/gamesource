use strict;
use FindBin;
use warnings;
use Data::Dumper;

# 02/13/07 (RB)
# Fixed build.pl and the testsuite so that they can run with Perl SDK 5.8.x.
#
# This is achieved by passing --aptroot=AptPath to captest.pl, checktests.pl or build.pl.
# "AptPath" specifies a version of Apt that has a compatible (Perl 5.6) version of swfc.exe 
# and PCGLc.exe.  For example, --aptroot=C:\packages\Apt\1.01.00.  
#
# This is a workaround for the fact that you cannot run swfc.pl or PCGL.pl when Perl 
# SDK 5.8.x is installed.  You can omit --aptroot and the scripts will behave the way they 
# used to; namely, they will try to use swfc.pl and PCGLc.pl from the current Apt version.


=pod

=head2 NAME

build.pl

=head2 SYNOPSIS

Apt pipeline driver

=head2 DESCRIPTION

Runs the Apt pipeline process to convert a Flash SWF file to an Apt BIG file.

=head2 AUTHOR

(not known)

=head2 BUGS

no known bugs

=head2 SEE ALSO

Apt documentation: help-Apt.chm

=head2 COPYRIGHT

copyright 2004, Electronic Arts

=cut

my $debug				= getDebugCompiler();
my $delete				= getNoDelete();
my $sku					= getSKU();
my $stats				= getStats();
my $debugswfc			= getDebugSWFC();
my $debugaux			= getDebugAUX();
my $fastbuild			= getFastBuild();
my $auxInfo 			= getAuxInfo();
my $swfcInfo			= getswfcInfo();
my $doOptimize			= getOptimizeFlag();
my $doDecouple			= getDecoupleFlag();
my $aptRoot             = getAptRoot();
my ($file, $targetname)	= getFilenames();

print "build: sku '$sku', input '$file', output '$targetname'.\n";

my %skuauxcompiler = (
	pcgl	=> "AptAuxPCOpenGL\\pcglc",
	xbox	=> "AptAuxEAGLREAL\\eaglrealc",
	ps2		=> "AptAuxEAGLREAL\\eaglrealc",
	ps2geo	=> "AptAuxEAGLREAL\\eaglrealc",
	pc		=> "AptAuxEAGLREAL\\eaglrealc",
	pcgeo	=> "AptAuxEAGLREAL\\eaglrealc",
	ngc		=> "AptAuxEAGLREAL\\eaglrealc",
	ngcgeo	=> "AptAuxEAGLREAL\\eaglrealc",
	psp		=> "AptAuxEAGLREAL\\eaglrealc",
	pspgeo	=> "AptAuxEAGLREAL\\eaglrealc",
);

my %skuauxcompilerflags = (
	pcgl	=> "",
	xbox	=> "xbox",
	ps2		=> "ps2",
	ps2geo	=> "ps2geo",
	pc		=> "pc",
	pcgeo	=> "pcgeo",
	ngc		=> "ngc",
	ngcgeo	=> "ngcgeo",
	psp		=> "psp",
	pspgeo	=> "pspgeo",
);

my $basename = $targetname;
$basename =~ s/\.apt$//i;

##my $delete = "";
if ($basename eq 'out')
{
	$delete = "--nodelete";
}

#print "basename: $basename, targetname: $targetname, delete: $delete\n";

if (not defined($skuauxcompiler{$sku}))
{
	die "error: unknown sku '$sku'";
}
my $endian;
if( $sku eq 'ngc' || $sku eq 'ngcgeo' )
{
	$endian = 'Network';
}else
{
	$endian = 'Vax';
}

if ( $aptRoot eq '' )
{
	system("perl $debugswfc -I$FindBin::Bin\\Apt $FindBin::Bin\\Apt\\swfc.pl $swfcInfo $doOptimize $doDecouple -endian=$endian --verbosity=5 $stats \"$file\" -o \"$targetname\"") == 0 || die $@;
	system("perl $debugaux -I$FindBin::Bin\\Apt $FindBin::Bin\\$skuauxcompiler{$sku}.pl $debug $auxInfo $delete $fastbuild $skuauxcompilerflags{$sku} \"$basename\"") == 0 || die $@;
}
else
{
    # Let the user specify a custom version of Apt, so that we can use an older
    # version of swfc.  This is needed by the testsuite (PCOpenGL) when the user
    # has an incompatible version of the Perl SDK.
    my $swfcexe = "$aptRoot\\bin\\Apt\\swfc.exe";
    my $auxpipe = "$aptRoot\\bin\\$skuauxcompiler{$sku}.exe";
    print "swfc.exe = $swfcexe\n";
    print "auxpipe = $auxpipe\n";
    if ( ! -e $swfcexe )
    {
        print "error: cannot find $swfcexe"; 
        exit 1;
    }
    if (! -e $auxpipe)
    {
        print "error: cannot find $auxpipe"; 
        exit 1;
    }
    system("$swfcexe $swfcInfo $doOptimize $doDecouple -endian=$endian --verbosity=5 $stats \"$file\" -o \"$targetname\"") == 0 || die "swfc failed\n";
    system("$auxpipe $debug $auxInfo $delete $fastbuild $skuauxcompilerflags{$sku} \"$basename\"") == 0 || die "$auxpipe failed\n";
}


if ($sku eq 'xbox')
{
	print "Copying to Xbox...\n";
	my $xbcp = "xbcp /d /y /t /f /q $basename.big $FindBin::Bin\\..\\test\\*.ttf xe:\\AptViewerEAGLREAL";
	system($xbcp);
}
if($sku eq 'ngc' || $sku eq 'ngcgeo')
{
	if( not defined($ENV{DVDROOT}))
	{
		print "build: No DVDROOT (env var) defined, skip copy to it!\n";
	}else
	{
		print "Copying to ngc...\n";
		my $ngcpy = "copy /y $basename.big $ENV{DVDROOT}";
		system($ngcpy);
	}
}
sub getSKU
{
   	for (my $i = 0; $i <= $#ARGV; $i++)
   	{
   		my $arg = $ARGV[$i];
   
   		if ($arg =~ m/sku=(.*)/i)
   		{
   			splice @ARGV, $i, 1;
   			if (open LASTSKU, ">lastsku.txt")
   			{
   				print LASTSKU "$1";
   				close LASTSKU;
   			}
   			return $1;
   		}
   	}
   
   	open LASTSKU, "lastsku.txt" or return 'pcgl';
   	my $sku = <LASTSKU>;
   	chomp($sku);
   	return $sku;
}

sub getAuxInfo
{
	for (my $i = 0; $i <= $#ARGV; $i++)
	{
		my $arg = $ARGV[$i];

		if ($arg =~ m/--auxInfo=(.*)/i)
		{
			splice @ARGV, $i, 1;
			return "--info=$1";
		}
	}
	return '';
}

sub getOptimizeFlag
{
	for (my $i = 0; $i <= $#ARGV; $i++)
	{
		my $arg = $ARGV[$i];

		if ($arg =~ m/(--optimize)/i)
		{
			splice @ARGV, $i, 1;
			return "--optimize";
		}
	}
	return "";
}

sub getDecoupleFlag
{
	for (my $i = 0; $i <= $#ARGV; $i++)
	{
		my $arg = $ARGV[$i];

		if ($arg =~ m/(--decouple)/i)
		{
			splice @ARGV, $i, 1;
			return "--decouple";
		}
	}
	return "";
}

sub getswfcInfo
{
	for (my $i = 0; $i <= $#ARGV; $i++)
	{
		my $arg = $ARGV[$i];

		if ($arg =~ m/--swfcInfo=(.*)/i)
		{
			splice @ARGV, $i, 1;
			return "--info=$1";
		}
	}
	return '';
}

sub getStats
{
	for (my $i = 0; $i <= $#ARGV; $i++)
	{
		my $arg = $ARGV[$i];

		if ($arg =~ m/(--stats)/i)
		{
			splice @ARGV, $i, 1;
			return $1;
		}
	}
	return '';
}

sub getDebugSWFC
{
	for (my $i = 0; $i <= $#ARGV; $i++)
	{
		my $arg = $ARGV[$i];

		if ($arg =~ m/(--debugswfc)/i)
		{
			splice @ARGV, $i, 1;
			return '-d';
		}
	}
	return '';
}

sub getDebugAUX
{
	for (my $i = 0; $i <= $#ARGV; $i++)
	{
		my $arg = $ARGV[$i];

		if ($arg =~ m/(--debugaux)/i)
		{
			splice @ARGV, $i, 1;
			return '-d';
		}
	}
	return '';
}

sub getFastBuild
{
	for (my $i = 0; $i <= $#ARGV; $i++)
	{
		my $arg = $ARGV[$i];

		if ($arg =~ m/(--fastbuild)/i)
		{
			splice @ARGV, $i, 1;
			return '--fastbuildtextures';
		}
	}
	return '';
}

sub getDebugCompiler
{
	for (my $i = 0; $i <= $#ARGV; $i++)
	{
		my $arg = $ARGV[$i];

		if ($arg =~ m/--debug/i)
		{
			splice @ARGV, $i, 1;
			return "--debug";
		}
	}
	return '';
}

sub getNoDelete
{
	for (my $i = 0; $i <= $#ARGV; $i++)
	{
		my $arg = $ARGV[$i];

		if ($arg =~ m/--nodelete/i)
		{
			splice @ARGV, $i, 1;
			return "--nodelete";
		}
	}
	return '';
}

sub getFilenames
{
	my $file = shift @ARGV;

	unless (defined $file)
	{
		print "build [sku=<sku>] [--swfcInfo=S] [--auxInfo=S] input[.swf] [output.apt]\n";
		print " --xxxInfo params cause swfc or aux interpreter to display file data.\n";
		print "  [--swfcInfo=S]    S: geometry,textures,sounds,summary,all\n";
		print "  [--auxInfo=S]     S: geometry,textures,sounds,summary,all\n";
		print "  [--optimize]      Enables aTack Optimizer\n";
        print "  [--aptroot=<Apt folder>]   Specifies a custom version of Apt.\n";
		exit 1;
	}

	if ($file =~ m/\.fla$/)
	{
		$file =~ s/\.fla$//;
	}
	if ($file =~ m/\.big$/)
	{
		$file =~ s/\.big$//;
	}

	if (! -e $file)
	{
		my $origFile = $file;
		$file = "$file.swf";
		if (! -e $file)
		{
			print "neither '$origFile' nor '$file' exist.\n";
			exit 1;
		}
	}

	my $target = shift @ARGV;
	unless (defined $target)
	{
		$target = 'out.apt';
	}

	if ($target !~ m/\.apt$/i)
	{
		$target .= '.apt';
	}

	return ($file, $target);
}


sub getAptRoot
{
	for (my $i = 0; $i <= $#ARGV; $i++)
	{
		my $arg = $ARGV[$i];

		if ($arg =~ m/--aptroot=(.*)/i)
		{
			splice @ARGV, $i, 1;
			return $1;
		}
	}
	return '';
}

