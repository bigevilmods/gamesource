# -----------------------------------------------------------------------------------------------------------------------------
# --- Perl options and package declarations
#		This section describes the various Perl modules used by this program (FindBin, Cwd, etc).
#		It is also used to specify Perl options (strict, warnings)
#
use strict;					# strict usage of vars, subs, and refs
use warnings;				# display all warnings
use FindBin;				# import package: FindBin, to find directory were the running script resides/originates
use AptCompiler ':all';		# import package: AptCompiler, to process SWF contents
use File::Glob ':glob';		# import package: File, to use the function Glob
use Cwd;					# import package: Cwd, current working directory utility
use Data::Dumper;			# import package: Data, to use function Dumper
use Getopt::Long;			# import package: Getopt, to use the Long type of options
use XML::Simple;			# import package: XML-Simple, for basic XML parser
use Win32::Process;


# -----------------------------------------------------------------------------
# modifications:
#	0.1		2003-??-??	(???)	original version
#	0.2		2004-06-11	(rrv)	added XML configuration function		(FR#535)
#	0.3		2004-06-14	(rrv)	enhanced for PSP processing
#	0.4		2009-06-11	(RB)	Replaced PerlGimex with gx.exe. Replaced rm.exe with unlink() and del.
#                               NOTE: Only tested ps2 and ngc skus. Did not test psp or xbox.
#                               Did not test 'fastbuildtextures' option.
# -----------------------------------------------------------------------------
my $version = "0.4";
#
# --- show help info
sub printSyntax()
{
	print "\n".
	"EAGLREALc - compiler for Apt auxiliary library EAGL REAL, version $version\n".
	"Built for Apt-2.09.01 \n\n".
	"usage: eaglrealc [--nodelete] [--fastbuildtextures] [--info=S] [--gx=S] <sku> <input>\n\n".
	"\t--nodelete          : do not delete the process work files\n".
	"\t--fastbuildtextures : build texture palette\n".
	"\t--info=S            : show process info for S=geometry,textures,sounds,all\n".
	"\t--gx=S              : override full path to gx.exe (default is <EAGLREALc.exe folder>../Apt/gx.exe)\n".
	"\t<sku>               : target platform/sku: ps2, xbox, pc, ngc, psp\n".
	"\t<input>             : SWF file to process\n".
	"Assets are read from <input>_textures, <input>_geometry, <input>_sounds,\n".
	"and <input>_videos. The file names are %d.<type>. At runtime, these assets\n".
	"will be requested by their id (the %d).\n";
}
#
# -----------------------------------------------------------------------------------------------------------------------------
# --- declare local working variables
my $binaryData;
my $sku;
my %shapeData;
#
my $debug				= '';
my $dormdirs			= 1;				# remove working dirs (0=no, else yes)
my $fastbuildtextures	= 0;
my $useDTX5             = 0;                # defualt uses DXT3 for xbox textures, 1=use DTX5
#
# --- counters, accumulators, etc
my $totalGeometryCount	= 0;
my $totalGeometrySize	= 0;
my $totalTextureCount	= 0;
my $totalTextureSize	= 0;
my $totalSoundCount		= 0;
my $totalSoundSize		= 0;
my $displayInfo			= "";
my $apt_gx;

#
my $pspldexe			= "C:\\packages\\PSP\\0.2.3\\SDK\\sce\\psp\\gcc-0.10.4\\bin\\psp-ld.exe ";
#
my $gimexpspdir			= "C:\\packages\\gxtoolkit\\5.80-psp-emu-alpha-01\\bin\\pc-vc71";
my $gimexpspdll			= "$gimexpspdir\\gimex.dll";
my $gimexpspexe_gx		= "$gimexpspdir\\gx.exe -v2 -b0 ";
my $gimexpspexe_ginfo	= "$gimexpspdir\\ginfo.exe";
#
# ---------------------------------------------------------
# --- load configuration file for runtime options
# ---	returns hash-table with contents from XML ini file
# ---------------------------------------------------------
my $bin = $FindBin::Bin;
my $cfgxml = "$bin/EAGLREALc.xml";
#
sub loadXmlOptions($)
{
	my $xinif = shift;
	my $config = XMLin( $xinif  );
	###print Dumper($config);
	return $config;
}
# ---------------------------------------------------------
# --- get value of configuration parameter
# ---	call parameters: (config-hash-table,key-name)
# ---	returns value asociated with parameter key-name
# ---------------------------------------------------------
sub getXmlOption($$)
{
	my $cfg = shift;
	my $key = shift;
	my $retval = "";
	if(!defined $cfg)
	{
		print ">>> no configuration value for: $key\n";
		return $retval;
	}
	my $val = $cfg->{options}->{$key}->{value};
	if(defined $val)
	{
		$retval = lc $val;
	}
	return $retval;
}
# ---------------------------------------------------------
# --- display optional process information
# ---------------------------------------------------------
sub displayInfo
{
	# --- use function's 1st call parameter to identify what kind of info to display, if any
	# --- it shifts the first parameter out, leaving the second in the input buffer ('@_' is also known as @ARGV)
	my $passToken = shift;
	
	# --- if the 'passToken' is defined in the var $displayInfo, or if showing 'all' values...
	if ( index($displayInfo, $passToken) >= 0 || $displayInfo eq 'all' )
	{
		# --- print the second function call parameter
		print @_;
	}
}
# ---------------------------------------------------------
# --- extract TextureMatrix from .EA3 file
# ---	returns result
# ---------------------------------------------------------
sub _checkForTextureMatrix($)
{
	my $ea3File = shift;
	my @tmvalues;
	my $cnt;
	if($debug) { print "checkForTextureMatrix:  $ea3File\n"; }
	open FEA, "<$ea3File" or die "couldn't open $ea3File for input";
	while( <FEA> )
	{
		if(/TextureMatrix/)
		{
			##@tmvalues = split " ", $_;
			##$cnt = $#tmvalues;
			print ": " . $_;
		}
	}
	close FEA;
	return;
}
# ---------------------------------------------------------
# --- multiply parameter matrices
# ---	returns the result
# ---------------------------------------------------------
sub _multMatrix($$)
{
	my @m0 = @{$_[0]};		# first matrix
	my @m1 = @{$_[1]};		# second matrix
	my @out;				# outbound results

	# --- for each row...
	for (my $j=0; $j<4; $j++)
	{
		# --- multiply with each column...
		for (my $i=0; $i<4; $i++)
		{
			$out[$j*4 + $i] = $m0[$j*4 + 0]*$m1[0*4 + $i] + 
						      $m0[$j*4 + 1]*$m1[1*4 + $i] +
							  $m0[$j*4 + 2]*$m1[2*4 + $i] +
							  $m0[$j*4 + 3]*$m1[3*4 + $i];
		}
	}
	# --- return results
	return @out;
}
# ---------------------------------------------------------
# --- scale matrix using parameter scaling factor
# ---	returns the result
# ---------------------------------------------------------
sub _scaleMatrix($@)
{
	my $scale = shift;		# first paramater
	my @matrix = @_;		# second parameter

	# --- create scale matrix
	my @scaleMatrix = ($scale, 0, 0, 0,
	                   0, $scale, 0, 0,
					   0, 0, $scale, 0,
					   0, 0, 0, 1);

	# --- return results
	return _multMatrix(\@matrix, \@scaleMatrix);
}
# ---------------------------------------------------------
# --- convert geometry for use by EAGL
# ---------------------------------------------------------
sub convertGeometryEAGL($$$)
{
	# --- function call parameters
	my $inputDirectory	= shift;
	my $outputFile		= shift;
	my $sku				= shift;

	# --- late point to be checking, no?
	if (not defined($ENV{EAGLHOME}))
	{
		die "error: 'EAGLHOME' env var must be set\n";
	}

	#print "inputDirectory: $inputDirectory\n";
	
	# --- get the running script's home directory
	my $bin = $FindBin::Bin;

	# --- replace unix file delimiters with WinDoze's
	$bin =~ s/\//\\/g;

	# --- add directory to environment search PATH
	$ENV{PATH} = "$bin;$bin\\..;$ENV{PATH}";

	# --- get all the RU files and stuff them into an array
	my @ruFiles = bsd_glob("$inputDirectory\\*.ru");

	# --- if no .RU files found, return with error status
	return '' if ($#ruFiles == -1);

	# 2004/05/06 --DC-- Modified conversion process to take only 128 parameters at a time. This helps prevent
	# command line issues where we would overflow the commandline vars to retoea3.exe. 
	#
	# This basically converts the files in batches of 128 files. 
	while( @ruFiles != 0 )
	{
		my @ruLimitedFiles = splice( @ruFiles, 0, 128);
		if($debug) { print "rulst:  @ruLimitedFiles\n"; }
	
		my $ruString = '';

		foreach my $ru (@ruLimitedFiles)
		{
			if($debug) { print "RU:  $ru\n"; }
			$ruString .= "\"$ru\" ";
		}

		system("rutoea3.exe $ruString \"=$inputDirectory\"") and die "error: couldn't convert files from .ru --> .ea3";
	}
	
	# --- get environment variable specifying the runtime target platform
	$ENV{"RUNTIME_TARGET"} = $sku;

	# --- tweak if gamecube
	my $eaglsku;
	if($sku eq 'ngc')
	{
		$eaglsku = 'gc';
	}
	else
	{
		$eaglsku = $sku;
	}

	# --- set the environment vars needed for rendermethods and computations
	$ENV{"EAGL_${eaglsku}_RMS_PATH"} = "$ENV{EAGLHOME}\\$eaglsku\\rendermethods";
	$ENV{"EAGL_${eaglsku}_COMPUTATION_PATH"} = "$ENV{EAGLHOME}\\$eaglsku\\computations";

	# --- get all the .EA3 files and stuff them into an array
	my @ea3Files = bsd_glob("$inputDirectory\\*.ea3");
	my @oFiles;
	# --- get current directory
	my $curdir = cwd;
	# --- change Unix file delimiters to WinDoze's
	$curdir =~ s/\\/\//g;

	# --- for each .EA3 file, process it with the utility EA3C
	foreach my $ea3File (@ea3Files)
	{
		if($sku eq 'psp')
		{
			# --- check the EA3 file for a TextureMatrix
			_checkForTextureMatrix($ea3File);
		}
		
		my $oFile = $ea3File;
		# --- create output file name from EA3 file name
		$oFile =~ s/\.ea3$/\.o/;

		# --- build command string to call EA3C
		##my $ea3c = "\"$ENV{EAGLHOME}\\bin.pc\\$eaglsku\\ea3c.exe\" \"$ea3File\"";
		my $ea3c = "\"$ENV{EAGLHOME}\\bin.pc\\$eaglsku\\ea3c.exe\" \"$ea3File\"";
		#my $ea3c = "\"$ENV{EAGLHOME}\\bin.pc\\$eaglsku\\ea3cd.exe\" \"$ea3File\"";
        $ea3c .= " -m \"$bin\\rms.lst\"";
#		$ea3c .= " --mesh-only";
		$ea3c .= " -N";
		$ea3c .= " -o \"$oFile\"";
		$ea3c .= " -v0";
		if($sku eq 'pc' || $sku eq 'xbox')
		{
            $ea3c .= " --converter-dir $ENV{EAGLHOME}\\lib.pc\\$eaglsku ";
		}
		
		# --- change WinDoze's file delimiters back to Unix delmiters
		$oFile =~ s/\\/\//g;
		# --- remove current directory name from the file name
		$oFile =~ s-$curdir/--g;
		# --- change Unix file delimiters back to WinDoze's
		$oFile =~ s/\//\\/g;
		# --- save stripped filename in array
		push @oFiles, $oFile;
		# --- show the command about to be executed
		if($debug)
		{
			print ">>> $ea3c\n";
		};
		# --- run the EA3C program: input .EA3 file; output .O file; show error message if it fails
		system($ea3c);

		# --- get file size of .O file
	    my $tempFileSize = (-s $oFile);
		# --- display info, if any
	    displayInfo "geometry", "$oFile : Size(Bytes):$tempFileSize \n";
		# --- tally the geometry count
	    $totalGeometryCount= $totalGeometryCount + 1;
		# --- accumulate the file size in the totals bucket
	    $totalGeometrySize = $totalGeometrySize + $tempFileSize;
	}
#
	my $ld;
	if($sku eq 'pcxxx')	# not ever used!
	{
		my $lumpy = "lumpy -b0 -v0 -align128 -paths0 $outputFile.big @oFiles";
		system($lumpy) and die "error: ld failed on '$outputFile'";
		return $outputFile.".big";	#???
	}
	else	# always falls thru!
	{
		# --- open linker response file and write the names of the .O files
		my $ldresponse = "ldresponse.txt";
		open (LDRESPONSE, ">$ldresponse") || die "error: can't open linker reponse file\n";
		my $ldstr = "\"" . (join "\" \"", @oFiles) . "\"";
		print LDRESPONSE "INPUT($ldstr)";
		close LDRESPONSE;

		# --- load and link the .O files, and write out the image
		#if($sku eq 'psp')
		#{
		#	$ld = "$pspldexe -r -T $ldresponse -o $outputFile";
		#}
		#else
		{
			$ld = "$ENV{EAGLHOME}\\bin.pc\\ld.exe -r -T $ldresponse -o $outputFile";
		}

		if($debug)
		{
			print ">>> INPUT($ldstr)\n";
			print ">>> $ld\n";
		}
		print ">>> $ld\n";
		system($ld) and die "error: ld failed on $outputFile";
		# --- delete the response file
		if ($dormdirs) { unlink($ldresponse); };
	}

	# --- return the output file name
	return $outputFile;
}
# ---------------------------------------------------------------------------------
# --- Convert a bunch of .ru geometry description files into runtime binary format.
# ---------------------------------------------------------------------------------
sub convertGeometry($$)
{
	# --- function call parameters
	my $inputDirectory	= shift;
	my $outputFile		= shift;

	#print "inputDirectory: $inputDirectory\n";

	# --- fetch all the .RU files
	my @geomFiles = bsd_glob("$inputDirectory\\*.ru");

	#print "geomfiles:" . Dumper(\@geomFiles);

	my $fileNum = 0;
	my $chunkNumber = 0;

	# --- for each geometry file...
	foreach my $geomFile (@geomFiles)
	{
		# --- count
		$fileNum++;
		# --- shape identifier
		my $shapeID;
		if ($geomFile =~ m/([0-9]+)\.ru$/)
		{
			$shapeID = $1;
		}
		# print "parsing: $geomFile\n";
		# print "\r[$fileNum/${$#geomFiles + 1}]";

		# --- try to open the file, exit if it fails
		if (not open(GF, "$geomFile"))
		{
			print "$!\n";		# System error message
			print "$@\n";		# Perl error message
			die "couldn't open \"$geomFile\" !";
		}
		#
		my %tridata;
		my $curStyle;
		# --- read geometry file, and for each record...
		while (my $line = <GF>)
		{
			# print "$line\n";
			# --- clear trailing cr/lf, if any
			chomp($line);
			# --- check for chunk delimiters (\A matches the beginning of the string)
			if ($line =~ /\As .*/)
			{
				# New style
				$curStyle = $line;
				#print "new style: $curStyle\n";
				$chunkNumber++;
			}
			elsif ($line =~ /\Ac/)
			{
				$chunkNumber++;
			}
			else
			{
				#  --- stuff geo entry into shape array, using the current style identified above
				push @{$shapeData{$shapeID}->[$chunkNumber]{$curStyle}}, $line;
			}
		}
		close(GF);
	}
	#print Dumper(\%shapeData);

	open OUT, ">$outputFile" or die "couldn't open '$outputFile' for output";
	binmode(OUT, ":raw");

	# --- append a packed 24-char string 
	my $fileHeader = pack "a24", "Apt EAGL geometry file\032\0";
    setrawvalue($fileHeader);
    	
=pod
	foreach $shapeID (keys (%shapeData)) while loop instead
	{
	my ($shapeID, $
		# print "$shapeID\n";
		while (my ($unit, $data) = each(%$shapeUnits))
		{
			# print "\t$unit:\n";
			foreach my $subunit (@$data)
			{
				# print "\t\t$subunit\n";
			}
		}
	}
=cut

	# --- get all the shape Id's into an array
	my @ids = keys(%shapeData);

	# --- loop thru multiple passes
	# ---	curPass is an element in AptCompiler
	for ($curPass = 0; $curPass <= 5; $curPass++)
	{
		# --- write raw data to outputFile when 
		# --- AptCompiler::align: pass#, byte alignment
		# ---	on pass #0
		align 0, 4;
		writeint 0, scalar (@ids); # nShapeIDs
		writeptr 0, "aShapes"; # aShapes

		# --- on pass #1
		align 1, 4;
		fixupptr 1, "aShapes";
		# --- for each shape id
		foreach my $id (@ids)
		{
			my $shape = $shapeData{$id};

			my @units;
			foreach my $chunk (@$shape)
			{
				push @units, keys %$chunk;
			}

			align 2, 4;
			writeint 2, $id;
			writeint 2, scalar (@units);	# nUnits
			writeptr 2, "shape${id}_apUnits";	# apUnits
			
			align 3, 4;
			fixupptr 3, "shape${id}_apUnits";	# apUnits fixup

			foreach my $chunk (@$shape)
			{
				my @units = keys %$chunk;

				foreach my $unit (@units)
				{
					writeptr 3, "shape$chunk${id}_apUnits_${unit}";

					align 4, 4;
					fixupptr 4, "shape$chunk${id}_apUnits_${unit}";
					(my $eType, my @args) = split(/:/, substr($unit, 2));
					if ($eType eq "s")
					{
						# writeint 4, 666;
						writeint 4, 1;	# eType = TriSolid

						my $tris = $chunk->{$unit};
						writeint 4, scalar(@$tris);		# nPrims
						# print scalar(@$tris) . "\n";
						writeptr 4, "shape$chunk${id}_${unit}_data";	# pData

						writecolourasarray 4, @args;	# nColour

						align 5, 4;
						fixupptr 5, "shape$chunk${id}_${unit}_data";	# pData fixup
						#print "style: solid @args\n" if $curPass == 0;
						foreach my $tri (@$tris)
						{
							my @vertices = split(/:/, substr($tri, 2));
							writefloat 5, $vertices[0];
							writefloat 5, $vertices[1];
							writefloat 5, $vertices[2];
							writefloat 5, $vertices[3];
							writefloat 5, $vertices[4];
							writefloat 5, $vertices[5];
							#print "@vertices\n" if $curPass == 0;
						}
						# print "@$tris[0]\n\n" if $curPass == 0;
					}
					elsif ($eType eq "tw" || $eType eq "tc")
					{
						# writeint 4, 666;
						writeint 4, $eType eq "tw" ? 4 : 5;	# eType = TriTiled

						my $tris = $chunk->{$unit};
						writeint 4, scalar(@$tris);			# nPrims
						writeptr 4, "shape$chunk${id}_${unit}_data";	# pData
						
						writecolourasarray 4, @args[0..3];	# nColour
						
						if ($curPass == 4)
						{
							my @afMatrix = (
								$args[5], $args[6], 0, 0,
								$args[7], $args[8], 0, 0,
								0, 0, 1, 0,
								$args[9], $args[10], 0, 1 );
							
							foreach (@afMatrix)
							{
								writefloat 4, $_;
							}
						}
						writeint 4, $args[4];				# nBitmapCharID
						writeint 4, 0;						# pShape

						align 5, 4;
						fixupptr 5, "shape$chunk${id}_${unit}_data";	# pData fixup
						foreach my $tri (@$tris)
						{
							my @vertices = split(/:/, substr($tri, 2));
							writefloat 5, $vertices[0];
							writefloat 5, $vertices[1];
							writefloat 5, $vertices[2];
							writefloat 5, $vertices[3];
							writefloat 5, $vertices[4];
							writefloat 5, $vertices[5];
						}
						# print "@$tris[0]\n\n" if $curPass == 0;
					}
					elsif ($eType eq "l")
					{
						writeint 4, 6;	# eType = LineSolid

						my $lines = $chunk->{$unit};
						writeint 4, scalar(@$lines);		# nPrims
						writeptr 4, "shape$chunk${id}_${unit}_data";	# pData

						writecolourasarray 4, @args[1..4];	# nColour

						align 5, 4;
						fixupptr 5, "shape$chunk${id}_${unit}_data";	# pData fixup
						foreach my $line (@$lines)
						{
							my @vertices = split(/:/, substr($line, 2));
							writefloat 5, $vertices[0];
							writefloat 5, $vertices[1];
							writefloat 5, $vertices[2];
							writefloat 5, $vertices[3];
						}
						# print "@$tris[0]\n\n" if $curPass == 0;
					}
					else
					{
						die "unsupported primitive";
					}
				}
			}
		}

	}

	my $bufferData = getbuffer();
    syswrite OUT, $bufferData;
	close(OUT);
	
	#{
	my $tempFileSize = (-s $outputFile);
	displayInfo "geometry", "$outputFile : Size(Bytes):$tempFileSize \n";
	$totalGeometryCount= $totalGeometryCount + 1;
	$totalGeometrySize = $totalGeometrySize + $tempFileSize;
	#}
	return $outputFile;
}
# ---------------------------------------------------------------------------------
# --- Convert sound files
# ---------------------------------------------------------------------------------
sub convertSounds
{
	my $inputDirectory = shift;
	my $outputFile = shift;

    unlink("\"$outputFile\"");
    unlink("\"${outputFile}_lut\"");

	my @files = bsd_glob("$inputDirectory/*.wav");
	if ($#files < 0)
	{
		return;
	}
	
	# write out bank
	if($sku eq 'ps2' || $sku eq 'ps2geo')
	{
		system("sx -v0 -noprogress -ps2bank -eaxa_blk -playlocmaincpu @files -=$outputFile");
	}
	elsif($sku eq 'ngc' || $sku eq 'ngcgeo')
	{
		system("sx -v0 -noprogress -gcbank -eaxa_blk -playlocmaincpu @files -=$outputFile");
	}
	elsif($sku eq 'pc' || $sku eq 'pcgeo')
	{
		system("sx -v0 -noprogress -pcbank -eaxa_blk -playlocmaincpu @files -=$outputFile");
	}
	elsif($sku eq 'xbox' || $sku eq 'xboxgeo')
	{
		system("sx -v0 -noprogress -xboxbank -eaxa_blk -playlocmaincpu @files -=$outputFile");
	}
	elsif($sku eq 'psp')
	{
		print "Sound disabled for PSP emulator...\n";
		return;
	}else
	{
		die "add the corresponding for other sku here!";
	}

	# now write bank soundinfo table to convert from filename to patch number
	open OUT, ">${outputFile}_lut" or die "couldn't open '${outputFile}_lut' for output";
	binmode(OUT, ":raw");

	$curPass = 0;
    my $fileHeader = pack "a4", "ASFT";
	setrawvalue( $fileHeader );

	align 0, 4;
	writeint 0, $#files + 1;

	# write AptAuxSoundInfo array
	my $index = 0;
	foreach my $file (@files)
	{
		writeint 0, 0;						# nHandle
		$file =~ /(stream_)?(\d*)\.wav/;
		writeint 0, defined($1) ? -$2 : $2;	# nCharID
		writeint 0, $index++;				# nPatchID
	}

    my $bufferData = getbuffer();
    syswrite OUT, $bufferData;
	
	close(OUT);
	{
	    my $tempFileSize = (-s $outputFile);
	    displayInfo "sounds", "$outputFile : Size(Bytes):$tempFileSize \n";
	    $totalSoundCount= $totalSoundCount + 1;
		$totalSoundSize = $totalSoundSize + $tempFileSize;
	}
}


# gx will give us the width and height
# of an image, but the perl system()
# function can only return 8-bit values,
# so we use Win32::Process instead. 
sub get_dimension
{  
    my $texfile = shift; 
    my $width_or_height = shift;   
	my $ProcessObj;
	Win32::Process::Create($ProcessObj,
        "$apt_gx",
        "gx -return$width_or_height -w0 -v0 $texfile",
        0,
        NORMAL_PRIORITY_CLASS,
        ".") || die "Could not run gx on $texfile";

    $ProcessObj->Wait(30000);
    my $retval;
    $ProcessObj->GetExitCode($retval);
    return $retval;
}


# ---------------------------------------------------------------------------------
# --- Convert textures
# ---	returns output file name
# ---------------------------------------------------------------------------------
sub convertTextures
{
	my $inputDirectory	= shift;
	my $outputFile		= shift;
	my $needreturn		= 0;

	my @inputfiles = bsd_glob("$inputDirectory/*.*");

	return '' if ($#inputfiles < 0);

	if($sku eq 'psp' || $sku eq 'pspgeo')
	{
		$ENV{"GIMEX_DRIVERNAME"} = $gimexpspdll;
	}
#	
#
	if($debug)
	{
		print "- convertTextures: files: @inputfiles\n";
	};
#
	my $cmd;
	my $framename;
	my $index = 0;
	my $metalbin = "";
	my $swizzle = "";
	my $halfalpha = "";
    my $currTextType = "";
    my @framenames;
    my @hotspots;
    my @processedfiles;

	foreach my $inputfilename (@inputfiles)
	{
        my $orig_w = get_dimension( $inputfilename, "width" );
        my $orig_h = get_dimension( $inputfilename, "height" );
        my $w = $orig_w;
		my $h = $orig_h;

        push @hotspots, "$w,$h";

		die "Could not get width from $inputfilename" if ($w <= 0);
		die "Could not get height from $inputfilename" if ($h <= 0);

		
		if ($w > 1)
		{
			$w = 2 << (log($w-1) / log(2));
		}		
		if ($h > 1)
		{
			$h = 2 << (log($h-1) / log(2));
		}

		if ($sku eq 'ps2')
		{
			# rudimentary ps2 width/height validate
			my %invalidSize = (
				'64x512' => '128x512',
				'64x256' => '128x256',
				'64x128' => '128x128',
				'64x16' => '64x32',
				'64x8' => '64x32',
				'64x4' => '64x32',
				'64x2' => '64x32',
				'64x1' => '64x32',
				'32x512' => '128x512',
				'32x256' => '128x256',
				'32x128' => '128x128',
				'32x64' => '64x64',
				'32x8' => '32x16',
				'32x4' => '32x16',
				'32x2' => '32x16',
				'32x1' => '32x16',
				'16x512' => '128x512',
				'16x32' => '32x32',
			);

			my $size = "${w}x${h}";
			if ($sku eq 'ps2' && defined($invalidSize{$size}))
			{
				$size = $invalidSize{$size};
				
				print "\nwarning: resizing invalid sized texture: '${w}x${h}' --> '$size'\n";

				($w, $h) = split(/x/, $size);
			}
			else
			{
				$w = $orig_w;
                if($sku eq 'ps2') {
    				while (($w % 8) != 0) { $w++ }
                } else {
                    while (($w % 4) != 0) { $w++ }
                }
				$h = $orig_h;
				while (($h % 4) != 0) { $h++ }
			}
		}
		elsif ($sku eq 'ngc' || $sku eq 'ngcgeo' )
		{
			# this for 8 bit texture, gc require min size of 8 bit texture to be 4* 8
			if($h < 8)
			{
				$h = 8;
			}
			if($w < 4)
			{
				$w = 4;
			}
		}

# --- since these cases do nothing, comment them out
#		elsif ($sku eq 'xbox')
#		{
###			$w = $props->{width};
###			$h = $props->{height};
#		}
		elsif ($sku eq 'psp')
        	{
			$w = $orig_w;
			$h = $orig_h;
		}
		
		$needreturn = 1;

		my %textype = (
			'xbox'		=> 'GMX_DXT3',
			'xboxgeo'	=> 'GMX_DXT3',
			'pc'		=> 'GMX_ARGB_8888',
			'pcgeo'		=> 'GMX_ARGB_8888',
			'ps2'		=> 'GMX_PALETTED_8BIT',
			'ps2geo'	=> 'GMX_PALETTED_8BIT',
			'ngc'		=> 'GMX_PALETTED_8BIT',
			'ngcgeo'	=> 'GMX_PALETTED_8BIT',
			'psp'		=> 'GMX_DXT5',
			'pspgeo'	=> 'GMX_DXT5',
		);
		
		my %friendlytextype = (
		    'GMX_DXT3'  => '-dxt3',
		    'GMX_ARGB_8888'  => '-8888',
		    'GMX_PALETTED_8BIT'  => '-n256 -ma7',
		    'GMX_DXT5'  => '-dxt5',
		);

        my $framename = $inputfilename;
		#$framename =~ m/\/(\d*)$/i;
		$framename =~ m/(\d*)\..*$/i;
		push @framenames, $1;
		
		my $gx_cmd = "$apt_gx -overwrite2 -sethot0,$orig_w,$orig_h";
		
		if($sku eq 'psp' || $sku eq 'pspgeo')
		{
			# On PSP, we have no need of the metalbin element, so we need to set our properties differently.			
		}
		else
		{
            #$tex = Gimex::Crop($tex, 0, 0, $w, $h);            (LEO) we now crop after the conversion to reduce undesired artifacts in image			
			$metalbin = "-metalbin ";
		}

		if($sku eq 'ngc' || $sku eq 'ngcgeo')
		{
			$swizzle = "-swizzle ";
		}

		if ($fastbuildtextures && ($textype{$sku} eq 'GMX_PALETTED_8BIT' || $textype{$sku} eq 'GMX_PALETTED_4BIT'))
		{
			if($sku eq 'psp' || $sku eq 'pspgeo')
			{
				if($textype{$sku} eq 'GMX_PALETTED_4BIT')
				{
					$cmd = sprintf( "$gimexpspexe_gx %s -4 -exact ", $inputfilename );
				}
				else
				{
					$cmd = sprintf( "$gimexpspexe_gx %s -8 -exact ", $inputfilename );
				}
				if($debug)
				{
					print ">>> $cmd\n";
				}
				else
				{
					print ">>> $cmd\n";
					system( "$cmd" );
				}
			}
			else
			{
				$cmd = sprintf( "$apt_gx %s -%d -exact ", $inputfilename, ($textype{$sku} eq 'GMX_PALETTED_8BIT') ? 8 : 4 );				
				system( "$cmd" );
			}
		}
		else
		{
			if($sku eq 'psp' || $sku eq 'pspgeo')
			{
				##$cmd = sprintf( "$gimexpspexe_gx %s -8888 -exact ", $inputfilename );
				$cmd = sprintf( "$gimexpspexe_gx -8888 -exact %s ", $inputfilename );
				if($debug)
				{
					print ">>> $cmd\n";
				}
				else
				{
					print ">>> $cmd\n";
					system( "$cmd" );
				}				
			}
			else
			{
                if($sku eq 'xbox' && $useDTX5 eq 1)
                {
                    $currTextType = 'GMX_DXT5';
                }
                else
                {
                    $currTextType = $textype{$sku};
                }
                $currTextType = $friendlytextype{$currTextType};
			}
		}

        if ($sku eq 'ps2' || $sku eq 'ps2geo')
        {
		    $halfalpha = "-halfalpha ";		    
        }
        if ($sku ne 'psp' && $sku ne 'pspgeo')                  #(LEO) we now crop after the conversion to reduce undesired artifacts in image
        {            
            $gx_cmd = $gx_cmd . " -tga -32 -8888 -pack0 -crop0,0,$w,$h,0,1 ";
            my $processedfile = $inputfilename;
            $processedfile =~ s/\....$/\.tga/g;
            push @processedfiles, $processedfile;
        }
        else
        {
            push @processedfiles, $inputfilename;
        }
                
        print ">>>Running GX: $gx_cmd \n" if ($debug);
        system($gx_cmd . " $inputfilename");
		
		$totalTextureCount= $totalTextureCount + 1;

	}	# end foreach my $tex (@texs)

	my %texext = (
		'xbox'		=> 'xsh',
		'xboxgeo'	=> 'xsh',

		'pc'		=> 'fsh',
		'pcgeo'		=> 'fsh',

		'ps2'		=> 'ssh',
		'ps2geo'	=> 'ssh',

		'ngc'		=> 'gsh',
		'ngcgeo'	=> 'gsh',

		'psp'		=> 'msh',
		'pspgeo'	=> 'msh',
	);

	$outputFile = "$outputFile.$texext{$sku}";
	
	unlink($outputFile);

	my $filesnames = join ' ', @processedfiles;

	if($sku eq 'psp' || $sku eq 'pspgeo')
	{
		#
		#$cmd = sprintf( "$gimexpspexe_gx %s -msh -metalbin =%s ", $filesnames, $outputFile );
		$cmd = sprintf( "$gimexpspexe_gx %s -msh =%s ", $filesnames, $outputFile );
		#
		if($debug)
		{
			print ">>> $cmd\n";
		}
		else
		{
			system( "$cmd" );
		}
		#
	}
	else
	{	
		$cmd = sprintf( "$apt_gx %s $halfalpha $currTextType -$texext{$sku} $metalbin $swizzle  =%s ", $filesnames, $outputFile );
		print ">>>Saving final output: $cmd \n" if ($debug);
		system($cmd);
		
		my $index = 0;
		foreach my $localframename (@framenames)
		{
		    print ">>Updating framename and hotspot for frame " . $index . " as $localframename and $hotspots[$index]\n"  if ($debug);;
		    $cmd = sprintf( "$apt_gx -overwrite2 -name$localframename -sethot$hotspots[$index] -if(frame==$index) $outputFile" );
		    system($cmd);
		    $index = $index + 1;
		}
	}
	#print "\n" if $needreturn;

	if( -f $outputFile )
    {
		my $tempFileSize = (-s $outputFile);
		displayInfo "textures", "$outputFile : Size(Bytes):$tempFileSize \n";
		$totalTextureSize = $totalTextureSize + $tempFileSize;
	}

	if($sku eq 'psp' || $sku eq 'pspgeo')
	{
		$ENV{"GIMEX_DRIVERNAME"} = "";
	}

	return $outputFile;
}
# -----------------------------------------------------------------------------------------------------------------------------
# --- MAIN
# -----------------------------------------------------------------------------------------------------------------------------
unless (GetOptions(
			'delete!'				=> \$dormdirs,
			'info=s'				=> \$displayInfo,
			'gx=s'  				=> \$apt_gx,
			'fastbuildtextures!'	=> \$fastbuildtextures,
			'debug'					=> \$debug,
            'useDTX5'               => \$useDTX5,            
			)
		)
{
	printSyntax;
}

$sku = shift;
unless (defined $sku)
{
	printSyntax;
	exit 1;
}

if($sku eq 'ngc' || $sku eq 'ngcgeo')
{
	$intfmt = 'N';
}else
{
	$intfmt = 'V';
}

my $inputFilename = shift;
unless (defined $inputFilename)
{
	printSyntax;
	exit 1;
}

unless (defined $apt_gx)
{
    $apt_gx = "$FindBin::Bin/../Apt/gx.exe";
}
unless ( -e $apt_gx )
{
    print "Cannot find gx executable '$apt_gx'\n";
    exit 1;
}

# ----------------------------------------------
# --- process options from XML file
my $xconfig;
# --- default value for option "huff"
my $huffopt	= "";
if( -f $cfgxml )
{
	# --- load configuration options from XML file: EAGLREALc.xml
	$xconfig	= loadXmlOptions( $cfgxml );
	# --- get value for option "huff"
	my $opt	= getXmlOption( $xconfig, "huff" );
	if($opt ne "")
	{
		if( ($opt eq "no")	|| ($opt ne "yes") ) 
		{
			$huffopt = "";
		}
        else
        {
            $huffopt = "yes";
        }
	}
	$opt	= getXmlOption( $xconfig, "nodelete" );
	if($opt ne "")
	{
		if($opt eq "yes") 
		{
			$dormdirs = 1;
		}
        else
        {
            $dormdirs = 0;
        }
	}
	$opt	= getXmlOption( $xconfig, "fastbuildtextures" );
	if($opt ne "")
	{
		if($opt eq "yes") 
		{
			$fastbuildtextures = 1;
		}
        else
        {
            $fastbuildtextures = 0;
        }
	}
	$opt	= getXmlOption( $xconfig, "useDTX5" );
	if($opt ne "")
	{
		if($opt eq "yes") 
		{
			$useDTX5 = 1;
		}
        else
        {
            $useDTX5 = 0;
        }
	}
	# --- other options?
}
else
{
	print  ">>> configuration file not found: $cfgxml\n";
}
# ----------------------------------------------
#
$inputFilename =~ s/\//\\/g;
my $inputDirectory = $inputFilename;

my @geoNames;
if ($sku eq 'ps2' || $sku eq 'ngc' ||  $sku eq 'pc' || $sku eq 'xbox' || $sku eq 'psp')
{
	@geoNames = convertGeometryEAGL("${inputDirectory}_geometry", "\"$inputFilename.o\"",$sku);
	###if( -f "$inputFilename.o" ) { print "\n#2# file exists: $inputFilename.o\n"; }
	###print "### : @geoNames\n";
}
elsif ($sku eq 'ps2geo' || $sku eq 'ngcgeo' ||  $sku eq 'pcgeo' || $sku eq 'xboxgeo' || $sku eq 'pspgeo')
{
	@geoNames = convertGeometry("${inputDirectory}_geometry", "$inputFilename.geo");
}
else
{
	die "unknown sku: $sku";
}
#
###if( -f "$inputFilename.o" ) { print "\n#2# file exists: $inputFilename.o\n"; }
#
my $texFile = convertTextures("${inputDirectory}_textures", "$inputFilename");
convertSounds("${inputDirectory}_sounds", "$inputFilename.asf");
#
my $sndFiles = "";
if (-e "$inputFilename.asf")
{
	$sndFiles .= "\"$inputFilename.asf\"";
	$sndFiles .= " \"$inputFilename.asf_lut\"";
}
#
my $cmd;
if ($sku eq 'psp' )
{
	$cmd =  "lumpy -b0 -v0 -align16 -paths0 \"${inputFilename}.big\" \"$inputFilename.apt\" \"$inputFilename.const\" @geoNames \"$texFile\" $sndFiles";
	if($debug)
	{
		print ">>> $cmd\n";
	}
	system("$cmd");
	##system("lumpy -b0 -v0 -align128 -paths0 \"${inputFilename}.big\" \"$inputFilename.apt\" \"$inputFilename.const\" @geoNames \"$texFile\" $sndFiles");
}
else
{
	$cmd = "lumpy -b0 -v0 -align128 -paths0 \"${inputFilename}.big\" \"$inputFilename.apt\" \"$inputFilename.const\" @geoNames \"$texFile\" $sndFiles";
	if($debug)
	{
		print ">>> $cmd\n";
	}
	system("$cmd");
}
#
if ($sku eq 'pc' || $sku eq 'ngc')
{
	if($huffopt ne "")
	{
		$cmd = "huff -tj -v0 -b0 \"${inputFilename}.big\"";
		if($debug)
		{
			print ">>> $cmd\n";
		}
		system("$cmd");
	}
	else
	{
		print "... bypassing HUFF encoding ...\n";
	}
}

displayInfo "summary", "Final Geometry Files :$totalGeometryCount Size(Bytes):$totalGeometrySize \n" ;
displayInfo "summary", "Final Texture Files  :$totalTextureCount Size(Bytes):$totalTextureSize \n" ;
displayInfo "summary", "Final Sound Files    :$totalSoundCount Size(Bytes):$totalSoundSize \n" ;
if (( -e "${inputFilename}.apt" ) && ( -e "${inputFilename}.const" ))
{
    my $tempFileSize = (-s "${inputFilename}.apt");
    displayInfo "summary", ".apt   File Size(Bytes):$tempFileSize \n" ;
    $tempFileSize = (-s "${inputFilename}.const");
    displayInfo "summary", ".const File Size(Bytes):$tempFileSize \n" ;
}

if ($dormdirs)
{
	system("del /s /q /f \"${inputFilename}_textures\" > NUL");
	system("rmdir \"${inputFilename}_textures\"");
	
	system("del /s /q /f \"${inputFilename}_geometry\" > NUL");
	system("rmdir \"${inputFilename}_geometry\"");
	
	system("del /s /q /f \"${inputFilename}_videos\" > NUL");
	system("rmdir \"${inputFilename}_videos\"");
	
	system("del /s /q /f \"${inputFilename}_sounds\" > NUL");
	system("rmdir \"${inputFilename}_sounds\"");
	
	unlink("\"${inputFilename}.apt\"");
	unlink("\"${inputFilename}.const\"");
	unlink("\"${inputFilename}.o\"");
	unlink("\"${inputFilename}.asf\"");
	unlink("\"${inputFilename}.asf_lut\"");
}
#eof#
