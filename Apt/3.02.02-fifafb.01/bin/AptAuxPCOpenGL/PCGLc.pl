# ***********************************************************************************************
#
#
#   \file    PCGLc.pl
#
#   \brief   Apt Data Exporter Legacy PC OpenGL Viewer
#
#   (c) 2009 Electronic Arts Inc.
#
#   \version    2.09.00    06/05/09 (RB) Added comment block.
#   \version    2.09.01    06/05/09 (RB) Replaced Gimex with gx.exe. Replaced rm.exe with del.
#   \version    2.09.02    09/23/09 (RB) Replaced gx.exe with tgautility.exe.
# 
# ***********************************************************************************************

use strict;
use warnings;
use AptCompiler ':all';
use File::Glob ':glob';
use FindBin;
use Data::Dumper;
use Getopt::Long;
use Win32::Process;

sub printSyntax()
{
	print "\n".
	      "PCGLc - compiler for Apt auxiliary library PC OpenGL . version 0.2 .\n".
          "Built for Apt-2.09.02 \n".
		  "usage: pcglc <input>\n\n".
		  "Assets are read from <input>_textures, <input>_geometry, <input>_sounds,\n".
		  "and <input>_videos. The file names are %d.<type>. At runtime, these assets\n".
		  "will be requested by their id (the %d).\n";
}

my $displayInfo = "";

sub displayInfo
{
	my $passToken = shift;
	
	if ( index($displayInfo, $passToken) >= 0 || $displayInfo eq 'all' )
	{
		print @_;
	}
}

my $binaryData;
my $sku;
my %shapeData;

my $totalGeometryCount = 0;
my $totalGeometrySize  = 0;
my $totalTextureCount  = 0;
my $totalTextureSize   = 0;
my $totalSoundCount    = 0;
my $totalSoundSize     = 0;

sub _multMatrix($$)
{
	my @m0 = @{$_[0]};
	my @m1 = @{$_[1]};
	my @out;

	for (my $j=0; $j<4; $j++)
	{
		for (my $i=0; $i<4; $i++)
		{
			$out[$j*4 + $i] = $m0[$j*4 + 0]*$m1[0*4 + $i] + 
						      $m0[$j*4 + 1]*$m1[1*4 + $i] +
							  $m0[$j*4 + 2]*$m1[2*4 + $i] +
							  $m0[$j*4 + 3]*$m1[3*4 + $i];
		}
	}
	return @out;
}

sub _scaleMatrix($@)
{
	my $scale = shift;
	my @matrix = @_;	
	my @scaleMatrix = ($scale, 0, 0, 0,
	                   0, $scale, 0, 0,
					   0, 0, $scale, 0,
					   0, 0, 0, 1);


	return _multMatrix(\@matrix, \@scaleMatrix);
}

# Converts a bunch of .ru geometry description files into a runtime binary format.
sub convertGeometry($$)
{
	my $inputDirectory = shift;
	my $outputFile = shift;

	#print "inputDirectory: $inputDirectory\n";
	my @geomFiles = bsd_glob("$inputDirectory\\*.ru");
	#print "geomfiles:" . Dumper(\@geomFiles);
	my $fileNum = 0;
	my $chunkNumber = 0;
	foreach my $geomFile (@geomFiles)
	{
		$fileNum++;
		my $shapeID;
		if ($geomFile =~ m/([0-9]+)\.ru$/)
		{
			$shapeID = $1;
		}
		# print "parsing: $geomFile\n";
		 #print "\r[$fileNum/${$#geomFiles + 1}]";

		if (not open(GF, "$geomFile"))
		{
			print "$!\n";
			print "$@\n";
			die "couldn't open \"$geomFile\" !";
		}
		my %tridata;
		my $curStyle;
		while (my $line = <GF>)
		{
			# print "$line\n";
			chomp($line);
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
				push @{$shapeData{$shapeID}->[$chunkNumber]{$curStyle}}, $line;
			}
		}
		close(GF);
		$totalGeometryCount= $totalGeometryCount + 1;
	}
	#print Dumper(\%shapeData);

	$intfmt = 'V'; # litte-endian for PC
	
	open OUT, ">$outputFile" or die "couldn't open '$outputFile' for output";
	binmode(OUT, ":raw");

	my $fileHeader = pack "a24", "Apt PCGL geometry file\032\0";
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

	my @ids = keys(%shapeData);
	for ($curPass = 0; $curPass <= 5; $curPass++)
	{
		align 0, 4;
		writeint 0, scalar (@ids); # nShapeIDs
		writeptr 0, "aShapes"; # aShapes
		
		align 1, 4;
		fixupptr 1, "aShapes";
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
						writeint 4, 0;						# nTextureID

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
	{
		my $tempFileSize = (-s $outputFile);
		displayInfo "geometry", "$outputFile: Size(Bytes):$tempFileSize \n";
		$totalGeometrySize = $totalGeometrySize + $tempFileSize;
	}
}

# Returns the width and height from a JPG file.
# Taken from AptImage.pm
sub ReadJpgSize                             # ($filename)
{
    my $filename   = shift;    
    open IN, "<$filename" or die "couldn't open $filename";
    binmode(IN, ":raw");
    my $done = 0;
    my $marker = 0xff;
    my $length = 0;
    my $dummy = 0;
    my $width = 0;
    my $height = 0;
    while ($done == 0)
    {
        $marker = 0xff;
        while ($marker == 0xff)
        {
            sysread IN, $marker, 1;
            $marker = unpack 'C', $marker;
        }
        if ($marker != 0x01 && ($marker < 0xd0 || $marker > 0xd9))
        {        
            sysread IN, $length, 2;
            $length = (unpack 'n', $length) - 2;
            if ($marker >= 0xc0 && $marker <= 0xc3) # our start of frame markers
            {
                sysread IN, $marker, 1;
                sysread IN, $height, 2;
                sysread IN, $width,  2;
                $height = unpack 'n', $height;    # ye olde height
                $width  = unpack 'n', $width;     # & width       
                $done = 1;
            }
            else
            {
                sysread IN, $dummy, $length
            }
        }
    }
    close IN;
    
    return ($width, $height);
}

# Returns the width and height from a TGA file.
sub ReadTgaSize                             # ($filename)
{
    my $filename   = shift;    
    my $dummy = 0;
    my $width = 0;
    my $height = 0;    
    open IN, "<$filename" or die "couldn't open $filename";
    binmode(IN, ":raw");
    sysread IN, $dummy, 12;
    sysread IN, $width, 2;
    sysread IN, $height, 2;
    $width  = unpack 'v', $width;      
    $height = unpack 'v', $height;
    close IN;
    return ($width, $height);
}


# gx will give us the width and height
# of an image, but the perl system()
# function can only return 8-bit values,
# so we use Win32::Process instead. 
sub get_dimension
{  
    my $texfile = shift; 
    my $width = 0;
    my $height = 0;    
    if ($texfile =~ m/([0-9]+)\.[Jj][Pp][Gg]$/)
    {
        return ReadJpgSize($texfile);
    }
    else
    {
        if ($texfile =~ m/([0-9]+)\.[Tt][Gg][Aa]$/)
        {
            return ReadTgaSize($texfile);
        }
    }
    
    return ($width, $height);
}

sub convertTextures
{
	my $inputDirectory = shift;
	my $outputBase = shift;

	my @names;
	my @files = bsd_glob("$inputDirectory\\*.*");
	
	foreach my $texfile (@files)
	{
	    my ($w, $h) = get_dimension( $texfile );

		die "Could not get width from $texfile" if ($w <= 0);
		die "Could not get height from $texfile" if ($h <= 0);

		
		if ($w > 1)
		{
			$w = 2 << (log($w-1) / log(2));
		}		
		if ($h > 1)
		{
			$h = 2 << (log($h-1) / log(2));
		}
		
		my $framename = $texfile;
		$framename =~ m/(\d*)\..*$/i;
        my $name = "$inputDirectory\\$1.tga";
        		
		die "Failed to convert $texfile to TGA and crop" if (system( "$FindBin::Bin/../Apt/tgautility.exe -upsize $w $h $texfile $name" ) != 0);		
	    
		push @names, "\"$name\"";
		
		{
		    my $tempFileSize = (-s $name);
		    displayInfo "textures", "$name : Size(Bytes):$tempFileSize \n";
		    $totalTextureCount= $totalTextureCount + 1;
		    $totalTextureSize = $totalTextureSize + $tempFileSize;
	    }
	}

	return @names;
}


my $dormdirs    = 1;

unless (GetOptions(
	'delete!' => \$dormdirs,
	'info=s' => \$displayInfo
	))
{
	printSyntax;
	exit 1;
}

my $inputFilename = shift;

unless (defined $inputFilename)
{
	printSyntax;
	exit 1;
}

unless (defined $inputFilename)
{
	printSyntax;
	exit 1;
}

$inputFilename =~ s/\//\\/g;
my $inputDirectory = $inputFilename;

convertGeometry("${inputFilename}_geometry", "$inputFilename.geo");
my @texNames = convertTextures("${inputFilename}_textures", "$inputFilename");

$ENV{PATH} = "$FindBin::Bin;$FindBin::Bin\\..;$ENV{PATH}";
system("lumpy -b0 -v0 -paths0 \"${inputFilename}.big\" \"$inputFilename.apt\" \"$inputFilename.const\" \"$inputFilename.geo\" @texNames \"${inputFilename}_sounds\\*.wav\"");

displayInfo "summary", "Final Geometry Files :$totalGeometryCount Size(Bytes):$totalGeometrySize \n" ;
displayInfo "summary", "Final Texture Files  :$totalTextureCount Size(Bytes):$totalTextureSize \n" ;
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
	unlink("\"${inputFilename}.geo\"");
	unlink("\"${inputFilename}.fsh\"");
}
unlink(@texNames);

