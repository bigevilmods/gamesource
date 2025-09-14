# ***********************************************************************************************
#
#
#   \file    DefineBitmaps.pl
#
#   \brief   SWF file decompiler
#
#   (c) 2007 Electronic Arts Inc.
#
#   \version    2.06.00    11/15/07 (RB) Last Version to support only SWF 7
#   \version    2.07.00    04/29/08 (RB) Handle JPEG headers output by Flash 8. 
#                                        Handle empty JPEGEncodingTable output by Flash CS3.
#                                        Whitespace fixed throughout file.
#   \version    2.09.00-2  06/03/09 (RB) Handle DefineBitsJPEG4
#   \version    2.09.01-3  09/02/09 (RB) Die if unlink fails in _bitsJPEG
#   \version    2.09.02-1  10/15/09 (RB) Allow RGB to be divided by alpha, like previous 
#                                        "unattenuate" operation that gx used to do in
#                                        swfc 2.09.00 and earlier. This undoes the 
#                                        premultiply-alpha operation that Flash does. See
#                                        comments near $AptOutput::unpremultiplyAlpha below.
#
# ***********************************************************************************************

package AptDerive;
use strict;
use warnings;
use Compress::Zlib;
use AptOutput;
use AptImage;
use AptMessage ':all';

my $enctable;

# quick and dirty rounding function
sub round
{
    my($number) = shift;
    return int($number + ( .5 * ($number <=> 0)) );
}

# clamps an alpha value such that it can never be less than 1
sub clampAlphaValue
{
    my $alphaVal = shift;
    if ( $alphaVal == 0  )
    {
        $alphaVal = 1.0;
    }
    return $alphaVal;
}

# clamps a value representing R, G or B such that it
# can never be greater than 255
sub clampColorValue
{
    my $colorVal = shift;
    if ( $colorVal > 255.0 )
    {
        $colorVal = 255.0;
    }
    return $colorVal;
}


sub _bitsLossless
{
	my $element = shift;
	my $bpp = shift;

	my $id = $element->CharacterID->value;
	my $format = $element->BitmapFormat;
	my $width = $element->BitmapWidth;
	my $height = $element->BitmapHeight;
	my $clutsize = $element->BitmapColorTableSize;

	my $z = inflateInit() or die "Can't create zlib stream";
	my $data = $element->ZlibBitmapData->substr(0);
    my ($out, $status) = $z->inflate(\$data);
    defined $out or die "Zlib raised an error $status";
    my $decompressedData = $out;

	my $tempfilename = "$main::outdir{tex}\\$id.tga";
	my $finaltex;

	# 8 bit data
	if ($format == 3)
	{
		my $colourTable = substr($decompressedData, 0, ($clutsize + 1) * $bpp);	# plus one is because (according MM specs) the size is wrong
		my $imageData = substr($decompressedData, ($clutsize + 1) * $bpp);
		undef $decompressedData;

		my @bgrColourTable = unpack('C*', $colourTable);
		my @colourTable32;

        # $i goes through the input at $bpp bytes at a time (3 or 4, depending on input)
        # and $j goes through 4 bytes at a time (because we output in 4 bytes)		
		
		my $j = 0;
		for (my $i=0; $i <= $#bgrColourTable; $i+=$bpp)
		{
		    if ( $bpp != 3 && $AptOutput::unpremultiplyAlpha )
		    {
                # undo the premultiply-alpha operation that Flash
                # performed when the image was added to the fla.
                # see: http://www.quasimondo.com/archives/000665.php 
                #      http://home.comcast.net/~tom_forsyth/blog.wiki.html

		        my $alpha = clampAlphaValue( $bgrColourTable[$i + 3] );
			    $colourTable32[$j+0] = round( clampColorValue( $bgrColourTable[$i + 2] * 255.0 / $alpha ) );
			    $colourTable32[$j+1] = round( clampColorValue( $bgrColourTable[$i + 1] * 255.0 / $alpha ) );
			    $colourTable32[$j+2] = round( clampColorValue( $bgrColourTable[$i + 0] * 255.0 / $alpha ) );
			    $colourTable32[$j+3] = $bgrColourTable[$i + 3];
			    
			}
			else
			{
			    $colourTable32[$j+0] = $bgrColourTable[$i + 2];
			    $colourTable32[$j+1] = $bgrColourTable[$i + 1];
			    $colourTable32[$j+2] = $bgrColourTable[$i + 0];
			    $colourTable32[$j+3] = $bpp == 3 ? 255 : $bgrColourTable[$i + 3];			
			}
			$j += 4;
		}
		my $bgrColourTable = pack ('C*', @colourTable32);

		# unpad data if necessary
		my $modulo = $width % 4;
		if ($modulo != 0)
		{
			my $unpaddedData;
			for (my $k=0; $k < $height; $k++)
			{
				$unpaddedData .= substr($imageData, ($width + (4 - $modulo))*$k, $width);
			}
			$imageData = $unpaddedData;
		}

        AptImage::WriteTga8Palette32($tempfilename, $width, $height, $imageData, @colourTable32);
	}
	elsif ($format == 4)
	{
		die "unsupported type 4 (16 bit) texture format";
	}
	elsif ($format == 5)
	{
		my @imageData = unpack('C*', $decompressedData);
			   
		if ( $AptOutput::unpremultiplyAlpha )
		{
            # undo the premultiply-alpha operation that Flash
            # performed when the image was added to the fla.
            # see: http://www.quasimondo.com/archives/000665.php 
            #      http://home.comcast.net/~tom_forsyth/blog.wiki.html

		    for (my $jj = 0; $jj <= $#imageData; $jj+=4)
		    {
			    my $uorig0 = $imageData[$jj + 0];
			    my $uorig1 = $imageData[$jj + 1];
			    my $uorig2 = $imageData[$jj + 2];
			    my $uorig3 = $imageData[$jj + 3];

			    my $alpha0 = clampAlphaValue( $uorig0 );
			    $imageData[$jj + 0] = round( clampColorValue( $uorig3 * 255.0 / $alpha0 ) );
			    $imageData[$jj + 1] = round( clampColorValue( $uorig2 * 255.0 / $alpha0 ) );
			    $imageData[$jj + 2] = round( clampColorValue( $uorig1 * 255.0 / $alpha0 ) );
			    $imageData[$jj + 3] = $uorig0;
		    }		
		}
		else
		{
		    for (my $ii = 0; $ii <= $#imageData; $ii+=4)
		    {
			    my $orig0 = $imageData[$ii + 0];
			    my $orig1 = $imageData[$ii + 1];
			    my $orig2 = $imageData[$ii + 2];
			    my $orig3 = $imageData[$ii + 3];
			    $imageData[$ii + 0] = $orig3;
			    $imageData[$ii + 1] = $orig2;
			    $imageData[$ii + 2] = $orig1;
			    $imageData[$ii + 3] = $orig0;
		    }
		}
		my $imageData = pack('C*', @imageData);
        AptImage::WriteTga32($tempfilename, $width, $height, $imageData);
	}

	# unattenuate image
    my $gxfilename="\"".$tempfilename."\"";
	{
		my $tempFileSize = (-s $tempfilename);
		displayInfo "textures", "$tempfilename : Size(Bytes):$tempFileSize \n";
		$AptOutput::totalTextures   = $AptOutput::totalTextures + 1;
		$AptOutput::totalTextureSize= $AptOutput::totalTextureSize + $tempFileSize;
	}
		
	return ({
		type => 'texture',
		id => $id,
		tempfilename => $tempfilename
	}, $element->CharacterID->value);
}

sub getid_DefineBitsLossless { return $_[0]->CharacterID }
sub build_DefineBitsLossless
{
	return _bitsLossless(@_, 3);
}

sub getid_DefineBitsLossless2 { return $_[0]->CharacterID }
sub build_DefineBitsLossless2
{
	return _bitsLossless(@_, 4);
}

# stupid flash defines jpegs in two streams.
# lop off the begin/end tag
# stupid gx doesn't handle enctable after image data in separate stream.
# when we dump out the image, we skip the begin, begin app0, skip the 16 bytes of header (actually variable sized, but isn't in
# flash), stick the tables in there, and then dump the rest of the image
sub getid_JPEGTables { return 0 }
sub build_JPEGTables
{
	my $element = shift;
    if (length($element->JPEGData) > 0)
    {
         $enctable = substr($element->JPEGData, 2, -2);
    }
    else
    {
         # It is OK to have an empty JPEGEncodingTable
         $enctable = "";
    }
	return undef;
}

sub getid_DefineBits { return $_[0]->CharacterID };
sub build_DefineBits
{    
	my $element = shift;

	die "internal error, no enctable before definebits" unless defined $enctable;

	my $id = $element->CharacterID->value;

	my $tempfilename = "$main::outdir{tex}\\$id.jpg";
	open OUT, ">$tempfilename" or die "couldn't open $tempfilename";
	binmode(OUT, ":raw");
	syswrite OUT, substr($element->JPEGData, 0, 20);
	syswrite OUT, $enctable;
	syswrite OUT, substr($element->JPEGData, 20);
	close OUT;

	{
		my $tempFileSize = (-s $tempfilename);
		displayInfo "textures", "$tempfilename : Size(Bytes):$tempFileSize\n";
		$AptOutput::totalTextures   = $AptOutput::totalTextures + 1;
		$AptOutput::totalTextureSize= $AptOutput::totalTextureSize + $tempFileSize;
	}
	
	return ({
		type => 'texture',
		id => $id,
		tempfilename => $tempfilename
	}, $element->CharacterID->value);
}

sub _bitsJPEG
{
    my $element = shift;
    my $hasalpha = shift;

    my $id = $element->CharacterID->value;

=pod
    open ORIG, ">orig" or die"";
    binmode(ORIG, ":raw");
    syswrite ORIG, $element->JPEGData;
    close ORIG;
    exit;
=cut

    my $tempfilename = "$main::outdir{tex}\\$id.jpg";
    my $finaltempfilename = "$main::outdir{tex}\\$id.tga";
    open OUT, ">$tempfilename" or die "couldn't open $tempfilename";
    binmode(OUT, ":raw");

    my $width = 0;
    my $height = 0;

    # sometimes flash is confused. it doesn't spit out an enctable (which is fine i think), but then it gets confused
    # and puts END START START ...image... END, rather than START ...enc... END START ...image... END. here we check to
    # see if we have that case, and just dump the whole file instead of our normal mucking.
    if ($element->JPEGData =~ /^\xff\xd9\xff\xd8/)
    {
        # Handle case: END START START ...image... END
        # Start outputing bytes from the 2nd START tag onwards. Typical for Flash MX 2004.
        syswrite OUT, substr($element->JPEGData, 4);
    }
    else
    {
        # swf-file-0.10 did this, so do here for simplicity
        my ($data1, $data2) = split /\xff\xd9/, $element->JPEGData;
        my $dataEnc = $data1 . "\xff\xd9";
        my $dataImage = $data2;
        if ( length($dataImage) == 0 )
        {
            # Handle case: START ...image... END
            # Typical for Flash 8.
            syswrite OUT, $dataEnc
        }
        else
        {
            # Handle case: START ...enc... END START ...image... END.
            # See description above.
            my $tableData = substr($dataEnc, 2, -2);

            syswrite OUT, substr($dataImage, 0, 20);
            syswrite OUT, $tableData;
            syswrite OUT, substr($dataImage, 20);
        }
    }
    close OUT;
    
    my $tempfilename_a = "$main::outdir{tex}\\${id}_a.tga";
    if ($hasalpha)
    {              
        my ($width, $height) = AptImage::ReadJpgSize($tempfilename);
            
        my $z = inflateInit() or die "Can't create zlib stream";
        my $data = $element->BitmapAlphaData->substr(0);
        my ($out, $status) = $z->inflate(\$data);
        defined $out or die "Zlib raised an error $status";
        my $decompressedData = $out;

        AptImage::WriteTga8GrayTo32($tempfilename_a, $width, $height, $decompressedData);
        my $gxfilename="\"".$tempfilename."\"";
        my $gxfilename_a="\"".$tempfilename_a."\"";
        my $finalgxfilename="\"".$finaltempfilename."\"";
        
        my $combine_alpha_option = "-combinealpha";
        if ( $AptOutput::unpremultiplyAlpha )
        {
            $combine_alpha_option = "-combinealpha_unpremultiply";
        }

        my $gxcmdline = "tgautility.exe $combine_alpha_option $gxfilename_a $gxfilename $finalgxfilename";
        if ((system $gxcmdline) != 0)
        {
            die $gxcmdline;
        }

        unlink($tempfilename_a) or die "Could not delete $tempfilename_a";;
    }

    {
        # DC -- Fixed bug. (filename depends on $hasAlpha)
        # Counting code was using the wrong filename is $hasAlpha was false.
        my $fileNameUsed = $hasalpha ? $finaltempfilename : $tempfilename;
        my $tempFileSize = (-s $fileNameUsed);
        if ( defined $tempFileSize )
        {
            displayInfo "textures", "$fileNameUsed : Size(Bytes):$tempFileSize\n";
            $AptOutput::totalTextures   = $AptOutput::totalTextures + 1;
            $AptOutput::totalTextureSize= $AptOutput::totalTextureSize + $tempFileSize;
        }
    }

    unlink($tempfilename) or die "Could not delete $tempfilename";
    
    return ({
        type => 'texture',
        id => $id,
        tempfilename => $hasalpha ? $finaltempfilename : $tempfilename
    }, $element->CharacterID->value);
}

sub getid_DefineBitsJPEG2 { return $_[0]->CharacterID };
sub build_DefineBitsJPEG2
{
	return _bitsJPEG(@_, 0);
}

sub getid_DefineBitsJPEG3 { return $_[0]->CharacterID };
sub build_DefineBitsJPEG3
{
	return _bitsJPEG(@_, 1);
}


sub getid_DefineBitsJPEG4 { return $_[0]->CharacterID };
sub build_DefineBitsJPEG4
{
    message 1, "SWFC: Warning: Treating DefineBitsJPEG4 as DefineBitsJPEG3\n";
	return _bitsJPEG(@_, 1);
}
1;
