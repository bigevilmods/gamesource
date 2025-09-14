# ***********************************************************************************************
#
#
#   \file    swfc.pl
#
#   \brief   SWF file decompiler
#
#   (c) 2007 Electronic Arts Inc.
#
#   \version    2.06.00    11/15/07 (RB) Last Version to support only SWF 7
#   \version    2.07.00    04/29/08 (RB) Introduced limited support for SWF 8 and 9 format.
#   \version    2.09.00-2  06/03/09 (RB) Introduced limited support for SWF 10 format.
#   \version    2.09.00-3  06/19/09 (RB) Removed sound support. No longer depends on sx.exe or madplay.
#   \version    2.09.01-1  08/27/09 (RB) Removed usage of gx. No unattenuate operation. Uses tgautility.
# 
# For more information on running and building SWFC, please see
#   http://confluence.ea.com/display/APT/Technical+Reference-Apt-Build+Configuration
#
# ***********************************************************************************************

# Tell Perl that it can find additional includes in some nearby subdirectories.
# This only works when running the .pl file directly; if compiling with perlapp,
# you need to use --lib to tell it where to look.
use lib "SWF-File-0.36/lib";
use lib "Data-TemporaryBag-0.08/lib";

use strict;
use warnings;
use File::Basename;
use FindBin;
use SWF::Parser;
use SWF::Element;
use SWF::BinStream;
use Data::Dumper;
use AptDerive;
use AptOutput;
use AptMessage ':all';
use Getopt::Long;
use Log;

my @tags;
my $fps;
my $swfversion;
my @rect;

sub header {
    my ($self, $signature, $version, $length, $xmin, $ymin, $xmax, $ymax, $rate, $count) = @_;
	$fps = $rate;
	@rect = ($xmin, $ymin, $xmax, $ymax);
	$swfversion = $version;
	#print Dumper(\@_);
}

sub tag
{
    my ($self, $tag, $length, $stream) = @_;
    my $element = SWF::Element::Tag->new(Tag => $tag, Length => $length);

    if ($tag == 73)
    {
        # DefineFontAlignZones tag requires special handling:
        # Get the number of glyphs from the last tag (DefineFont3)
        my $define_font3_data = $self->{_taglist}[-1];
        my $define_font3_element = $define_font3_data->{element};
        die "Expected DefineFont3 before DefineFontAlignZones" unless $define_font3_element->is_tagtype("DefineFont3");
        my $num_glyphs = @{$define_font3_element->GlyphShapeTable};
        $element->unpack($stream, $num_glyphs);
    } 
    else
    {
        $element->unpack($stream);
    }
    my $tagdata = { element => $element, subdata => [] };
    push @{$self->{_taglist}}, $tagdata;
    my ($tagname) = ref($element)=~/:([^:]+)$/;
    if ($tagname eq 'DefineSprite')
    {	
        foreach my $ct (@{$element->ControlTags()})
        {
            push @{$tagdata->{subdata}}, { element => $ct, subdata => [] };
        }
    }
    if ($#{$tagdata->{subdata}} == -1)
    {
        delete $tagdata->{subdata};
    }
}

#
#
# entry point
#
#

sub printSyntax()
{
	print "\n".
	      "swfc: Apt SWF decompiler\n".
          "Built for Apt-3.02.02\n\n".
		  "usage: swfc [options] <input.swf>\n\n".
		  "where options include:\n".
		  "\t--optimize\t\tEnables aTack optimizer\n".
          "\t--ptrsize=[4 | 8]\tBuilds data for different pointer sizes, size 4 is used by default\n".
		  "\t--no-unpremultiply\tDo not unpremultiply alpha for textures. Normally, the unpremultiply\n".
          "\t\t\t\talpha operation undoes the premultiply-alpha operation that Flash\n".
          "\t\t\t\tperforms on every image. By default, swfc WILL perform the unpremultiply\n".
          "\t\t\t\toperation, however you can use this option to prevent it from doing so.\n".
		  "\t--verbosity=N\n".
		  "\t\t\t0: Display errors only (default)\n".
		  "\t\t\t1: More verbose; also display warnings\n".
		  "\t\t\t2 or higher: Extra verbose; display debug info\n".
		  "\t--info=S\t\tS: geometry,textures,sounds,all \n".
		  "\t\t\t\t   (any combination, comma separated)\n".
		  "\t-o <output.apt>\t\tOutput file name\n".
		  "\t--endian=E\t\tE: 'Vax' or 'little' for little endian\n".
		  "\t\t\t\t   'Network' or 'big' for big endian\n".
		  "\t-outputConstValues\tOutput the constant values to a file.  The name is gotten from the\n".
		  "\t\t\t\toutputfilename and adding const.txt\n".
	      "\n".
		  "\t--debug\t\t\tEnables debugging information output.\n".
	      "\n".
		  "Assets are output to <output>_textures, <output>_geometry, <output>_sounds,\n".
		  "and <output>_videos. The file names are %d.<type>. At runtime, these assets\n".
		  "will be requested by their id (the %d).\n".
		  "\n".
          "\t--additionalPaths=S\t\tS: Additional environment paths, often used for outsourcing\n".
          "\t\t\t\tnon-distributable executable files such as triangle.exe\n".
		  "\n";
}

my $outputFilename;
my $statistics;
my $endian;
my $ptrSize;
my $outputConstFile;
my $doOptimize;   
my $doNotOptimize; # This forces no Optimizations (default Behavior)
my $doDecouple;
my $doNotDivideByAlpha;
my $writeDebugInfo;
my $additionalPaths;
unless (GetOptions(
	'endian=s' => \$endian,
	'ptrsize=s' => \$ptrSize,
	'verbosity=i' => \$verbosity,
	'info=s' => \$displayInfo,
	'stats!' => \$statistics,
	'o=s' => \$outputFilename,
	'no-optimize' => \$doNotOptimize,
	'optimize' => \$doOptimize,
	'decouple' => \$doDecouple,
	'no-unpremultiply' => \$doNotDivideByAlpha,
	'outputConstValues' => \$outputConstFile,
	'debug' => \$writeDebugInfo,
    'additionalPaths=s' => \$additionalPaths))
{
	printSyntax;
}

if(defined $doDecouple)
{
    # print "\nNOTE: The --decouple argument is deprecated.\n";
}

if (defined $doNotDivideByAlpha)
{
    $AptOutput::unpremultiplyAlpha = 0;
}


# Ever since Apt 2.05.00, there's no such thing as decoupled or non-decoupled.
# The output should be the same as what used to be called 'decoupled' though.

$doDecouple = 1;

my $inputFilename = shift;
unless (defined $inputFilename)
{
	printSyntax;
	exit 1;
}

unless (defined $outputFilename)
{
	$outputFilename = $inputFilename;
	$outputFilename =~ s/\.swf$/.apt/i;
	#print "no output file name specified with -o: defaulting to '$outputFilename'.\n";
}

unless (!defined $outputConstFile)
{
	$outputConstFile = $outputFilename;
	$outputConstFile =~ s/\.apt$/.const.txt/i;
	#print "no outputConstFile name specified with -outputConstValues: defaulting to '$outputConstFile'.\n";
}

if (not defined $additionalPaths)
{
    $additionalPaths="";
}

my $outputBasename = $outputFilename;
$outputBasename =~ s/\.apt$//i;

# make/clean up output directories for assets
my %dir = (
		geom => "_geometry",
		tex => "_textures",
		sound => "_sounds",
		video => "_videos"
		);

our %outdir;

my $bin = $FindBin::Bin;
$bin =~ s/\//\\/g;
# convert slashes, and add bin/Apt and bin to the path so that derive modules can use them
$ENV{PATH} = "$additionalPaths;$bin;$bin\\..;$ENV{PATH}";

while(my ($key, $outputDirectory) = each (%dir))
{
	my $name = "${outputBasename}${outputDirectory}";
	$outdir{$key} = $name;
	if (-e $name)
	{
		system("del /s /q $name > NUL");
		rmdir("$name");
	}
	mkdir($name);
}

if (defined $writeDebugInfo)
{
	$writeDebugInfo = 1;
}
else
{
	$writeDebugInfo = 0;
}

#run debugInfo builder, and generate unoptimized apd file
if ( $writeDebugInfo )
{
    my $file; 
	my $dir;
	my $ext;
	($file,$dir,$ext) = fileparse($inputFilename, qr/\.[^.]*/);

    # Only call DebugInfoBuilder when input file is .swf.
    if($ext eq ".swf")
    {
        my $commandLine = "$bin\\..\\DebugInfoBuilder.exe \"$inputFilename\"";
        my $result = system($commandLine);
        if ( $result != 0 )
        {
            exit 1;
        }
    }
}

# optimization Area ---
if( defined $doOptimize) 
{
	my $file; 
	my $dir;
	my $ext;
	($file,$dir,$ext) = fileparse($inputFilename, qr/\.[^.]*/);	
	my $optFileName = "$dir\\$file.eaf";
	
	# input filename extention defaults to .swf if not provided.
	if($ext eq "")
	{
		$inputFilename = "$inputFilename.swf"
	}
	
	if($ext eq ".eaf")
	{
		print "warning, --optimize used with already optimized input file!\n"
	}
	else
	{

		# run aTack Compiler
        my $firstPassCommandLine = "$bin\\..\\aTackCompiler.exe \"$inputFilename\" \"$optFileName\"";
        my $commandLine = "$bin\\..\\aTackCompiler.exe \"$optFileName\" \"$optFileName\"";

        if ( $writeDebugInfo )
        {
            $firstPassCommandLine .= " --debug";
            $commandLine .= " --debug";
        }

        my $lastcsum = system($firstPassCommandLine);
        my $currcsum = system($commandLine);
		while($lastcsum != $currcsum )
		{
			$lastcsum = $currcsum;
            $currcsum = system($commandLine);
		}
	}
	$inputFilename = $optFileName;
}


message 1, "SWFC: loading '$inputFilename...'\n";
my $parser = SWF::Parser->new( 'header-callback' => \&header, 'tag-callback' => \&tag);
$parser->{_taglist} = [];
$parser->parse_file( $inputFilename );

my $taglist = $parser->{_taglist};

message 1, "SWFC: processing '$inputFilename...'\n";
message 1, "Extracting characters...\n";
my $characters = AptDerive::getCharacters($taglist);
message 1, "Extracting movie...\n";
my $movie = AptDerive::getMovie($taglist);

my $timeStamp = AptDerive::getTimeStamp($taglist);
my $isDebugSwf = AptDerive::isDebugSwf($taglist);
unless ($isDebugSwf)
{
    $writeDebugInfo = 0;
}

message 1, "Writing to '$outputFilename'...\n";
#print "swfc: $rect[0] $rect[1] $rect[2] $rect[3]\n";

if (not defined $endian)
{
	$endian = 'little';
}
if (not defined $ptrSize)
{
	$ptrSize = '4';
}

AptOutput::toFile($outputBasename, $endian, $ptrSize, $taglist, $characters, $movie, $fps, \@rect, $statistics, $outputConstFile, $swfversion, $writeDebugInfo, $timeStamp, $isDebugSwf);

AptOutput::dispOutFileInfo();

message 1, "SWFC: wrote '$outputFilename'\n";
