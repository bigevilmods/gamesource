# ***********************************************************************************************
#
#
#   \file    AptCompiler.pl
#
#   \brief   SWF file decompiler
#
#   (c) 2007 Electronic Arts Inc.
#
#   \version    2.06.00    11/15/07 (RB) Last Version to support only SWF 7
#   \version    2.07.00    04/29/08 (RB) Handle focal radial gradient fill in writestyle().
# 
# ***********************************************************************************************

package AptCompiler;
use AptMessage ':all';
require Exporter;
#use Data::Dumper;

our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Win32::Guidgen ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
getbuffer
writeint
writeintptr
writeshort
writechar
writeptr
align
fixupptr
setrawvalue
writefloat
writematrix
invertmatrix
writeinvertedmatrix
writecxform
writencxform
writecolour
writecolourasarray
writestyle
writedelayedint
writedelayedshort
writedelayedintptr
fixupint
fixupshort
fixupintptr
invertmatrix
writestring
markforfill
fillto
colourColoned
lengthcheckstart
lengthcheckstop
dumplengthstats
getdatasize
$intfmt
$ptrsize
$curPass
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
);

our $intfmt = 'V';
our $ptrsize = "4";
our %FIXUPS;
our $compiledData;
our $curPass = 0;
my @lengthCheckContext;
my %sizeTotals;

sub getbuffer
{
    return $compiledData;
}

sub setrawvalue($)
{
    undef $compiledData;
    $compiledData = shift;
}

sub getdatasize
{
    return length($compiledData);
}

sub align($$)
{
	if ($curPass != $_[0]) { return; }
	
	my $nCount = $_[1];
	
	while ((length($compiledData) % $nCount) != 0)
	{
		$compiledData .= pack "a1";
	}
}

sub writeint($$)
{
	if ($curPass != $_[0]) { return; }

	unless (defined $_[1])
	{
		Carp::cluck "unhappy";
	}

	$compiledData .= pack $intfmt, $_[1];
}

sub writeintptr($$)
{
	if ($curPass != $_[0]) { return; }

	unless (defined $_[1])
	{
		Carp::cluck "unhappy";
	}

	if($ptrsize eq "4")
	{
		if(length($compiledData) % 4 != 0)
		{
			my $lengthqwesdf = length($compiledData);
			die "writeintptr writing $_[1] to unaligned address : $lengthqwesdf\n";
		}

		$compiledData .= pack $intfmt, $_[1];
	}
	else
	{
		align $curPass, 8;
		if(length($compiledData) % 8 != 0)
		{
			my $lengthqwesdf = length($compiledData);
			die "writeintptr writing $_[1] to unaligned address : $lengthqwesdf\n";
		}
		
		if($intfmt eq 'V')
		{
			$compiledData .= pack $intfmt, $_[1];
			$compiledData .= pack $intfmt, 0x00000000;
		}
		else
		{
			$compiledData .= pack $intfmt, 0x00000000;
			$compiledData .= pack $intfmt, $_[1];
		}
	}
}

sub writeshort($$)
{
	if ($curPass != $_[0]) { return; }

	unless (defined $_[1])
	{
		Carp::cluck "unhappy";
	}

	$compiledData .= pack lc($intfmt), $_[1];
}

sub writechar($$)
{
	if ($curPass != $_[0]) { return; }

	unless (defined $_[1])
	{
		Carp::cluck "unhappy";
	}

	$compiledData .= pack 'C', $_[1];
}
sub writeptr($$)
{
	if ($curPass != $_[0]) { return; }

	if (defined $FIXUPS{$_[1]})
	{
		die "internal error: multiple fixups for $_[1] specified";
	}

	if($ptrsize eq '4')
	{
		$FIXUPS{$_[1]} = length $compiledData;

		if(length($compiledData) % 4 != 0)
		{
			my $lengthqwesdf = length($compiledData);
			die "writeintptr writing $_[1] to unaligned address : $lengthqwesdf\n";
		}

		#printf "Marking: %s at %x\n", $_[1], $FIXUPS{$_[1]};
		$compiledData .= pack $intfmt, 0;
	}
	else
	{
		align $curPass, 8;
		
		if(length($compiledData) % 8 != 0)
		{
			my $lengthqwesdf = length($compiledData);
			die "writeintptr writing $_[1] to unaligned address : $lengthqwesdf\n";
		}
		
		$FIXUPS{$_[1]} = length $compiledData;
		#printf "Marking: %s at %x\n", $_[1], $FIXUPS{$_[1]};
		
		if($intfmt eq 'V')
		{
			$compiledData .= pack $intfmt, 0;
			$compiledData .= pack $intfmt, 0;
		}
		else
		{
			$compiledData .= pack $intfmt, 0;
			$compiledData .= pack $intfmt, 0;
		}
	}	
}

sub fixupptr($$)
{
    if ($curPass != $_[0]) { return; }

    if (!(defined $FIXUPS{$_[1]}))
    {
        die "internal error: fixup for $_[1] not found";
    }
    
    if($ptrsize eq '4')
    {
        if(length($compiledData) % 4 != 0)
        {
            my $lengthqwesdf = length($compiledData);
            die "fixupptr writing $_[1] to unaligned address : $lengthqwesdf\n";
        }
    }
    else
    {
        align $curPass, 8;
        
        if($FIXUPS{$_[1]} % 8 != 0)
        {
            die "fixupptr writing to $FIXUPS{$_[1]} \n";
        }   
    }
    
    my $length = length($compiledData);
    
    $ptrAddress = length($compiledData);

    if($ptrsize eq '4')
    {
        substr($compiledData, $FIXUPS{$_[1]}, 4) = pack $intfmt, $ptrAddress;
    }
    else
    {
        align $curPass, 8;
        if($intfmt eq 'V')
        {
            substr($compiledData, $FIXUPS{$_[1]}, 4) = pack 'V', $ptrAddress;
        }
        else
        {
            substr($compiledData, $FIXUPS{$_[1]} + 4, 4) = pack 'N', $ptrAddress;
        }
    }
    
    $FIXUPS{$_[1]} = undef;
    
    die "bad" if $length != length($compiledData);
}

sub writefloat($$)
{
	if ($curPass != $_[0]) { return; }

	unless (defined $_[1])
	{
		Carp::cluck "writefloat isn't happy";
	}
	if ($intfmt eq 'V')
	{
		$compiledData .= pack 'f', $_[1];
	}
	else
	{
		my $fNum = pack 'f', $_[1];
		my @bytes = unpack 'aaaa', $fNum;
		# swizzle necessary
		@bytes = reverse @bytes if ($intfmt eq 'N');

		$compiledData .= pack 'aaaa', @bytes;
	}
}

sub writematrix($$)
{
	if ($curPass != $_[0]) { return; }

	if (defined $_[1])
	{
		if ($curPass == $_[0])
		{
			#print "a:". $_[1][0] ."\n";
			#print "b:".$_[1][1] . "\n";
			#print "c:".$_[1][2] . "\n";
			#print "d:".$_[1][3] . "\n";
			#print "x:".$_[1][4] / 20.0 . "\n";
			#print "y:".$_[1][5] / 20.0 . "\n";
			#print "\n";
		}

		writefloat $curPass, $_[1][0];
		writefloat $curPass, $_[1][1];
		writefloat $curPass, $_[1][2];
		writefloat $curPass, $_[1][3];
		writefloat $curPass, $_[1][4] / 20.0;
		writefloat $curPass, $_[1][5] / 20.0;
	}
	else
	{
		#print "writing undef matrix\n";
		writefloat $curPass, 1;
		writefloat $curPass, 0;
		writefloat $curPass, 0;
		writefloat $curPass, 1;
		writefloat $curPass, 0;
		writefloat $curPass, 0;
	}
}

sub invertmatrix($)
{
	my $in = $_[0];
	my @out;

	my $det = $in->[0] * $in->[3] - $in->[1] * $in->[2];

	$out[0] = $in->[3] / $det;
	$out[1] = -$in->[1] / $det;
	$out[2] = -$in->[2] / $det;
	$out[3] = $in->[0] / $det;
	$out[4] = -($out[0] * $in->[4] + $out[2] * $in->[5]);
	$out[5] = -($out[1] * $in->[4] + $out[3] * $in->[5]);
	return \@out;
}

sub writeinvertedmatrix($$)
{
	if ($curPass != $_[0]) { return; }

	writematrix $_[0], invertmatrix($_[1]);
}

sub _tocolour($)
{
	return ($_[0] / 256.0) * 255.0;
}

sub writencxform($$)
{
	if ($curPass != $_[0]) { return; }

	if (defined $_[1])
	{
		if($intfmt eq 'V')
		{
			writechar($curPass, (defined $_[1][3]) ? _tocolour($_[1][3]) : 255);
			writechar($curPass, (defined $_[1][2]) ? _tocolour($_[1][2]) : 255);
			writechar($curPass, (defined $_[1][1]) ? _tocolour($_[1][1]) : 255);
			writechar($curPass, (defined $_[1][0]) ? _tocolour($_[1][0]) : 255);
			writechar($curPass, (defined $_[1][7]) ? _tocolour($_[1][7]) : 0);
			writechar($curPass, (defined $_[1][6]) ? _tocolour($_[1][6]) : 0);
			writechar($curPass, (defined $_[1][5]) ? _tocolour($_[1][5]) : 0);
			writechar($curPass, (defined $_[1][4]) ? _tocolour($_[1][4]) : 0);
		}else
		{
			writechar($curPass, (defined $_[1][0]) ? _tocolour($_[1][0]) : 255);
			writechar($curPass, (defined $_[1][1]) ? _tocolour($_[1][1]) : 255);
			writechar($curPass, (defined $_[1][2]) ? _tocolour($_[1][2]) : 255);
			writechar($curPass, (defined $_[1][3]) ? _tocolour($_[1][3]) : 255);
			writechar($curPass, (defined $_[1][4]) ? _tocolour($_[1][4]) : 0);
			writechar($curPass, (defined $_[1][5]) ? _tocolour($_[1][5]) : 0);
			writechar($curPass, (defined $_[1][6]) ? _tocolour($_[1][6]) : 0);
			writechar($curPass, (defined $_[1][7]) ? _tocolour($_[1][7]) : 0);
		}
	}
	else
	{
		writechar $curPass, 255;
		writechar $curPass, 255;
		writechar $curPass, 255;
		writechar $curPass, 255;
		writechar $curPass, 0;
		writechar $curPass, 0;
		writechar $curPass, 0;
		writechar $curPass, 0;
	}
}

sub writecxform($$)
{
	if ($curPass != $_[0]) { return; }

	if (defined $_[1])
	{
		writefloat($curPass, (defined $_[1][0]) ? ($_[1][0] / 256.0) : 1);
		writefloat($curPass, (defined $_[1][1]) ? ($_[1][1] / 256.0) : 1);
		writefloat($curPass, (defined $_[1][2]) ? ($_[1][2] / 256.0) : 1);
		writefloat($curPass, (defined $_[1][3]) ? ($_[1][3] / 256.0) : 1);
		writefloat($curPass, (defined $_[1][4]) ? ($_[1][4] / 256.0) : 0);
		writefloat($curPass, (defined $_[1][5]) ? ($_[1][5] / 256.0) : 0);
		writefloat($curPass, (defined $_[1][6]) ? ($_[1][6] / 256.0) : 0);
		writefloat($curPass, (defined $_[1][7]) ? ($_[1][7] / 256.0) : 0);
	}
	else
	{
		writefloat $curPass, 1;
		writefloat $curPass, 1;
		writefloat $curPass, 1;
		writefloat $curPass, 1;
		writefloat $curPass, 0;
		writefloat $curPass, 0;
		writefloat $curPass, 0;
		writefloat $curPass, 0;
	}
}

sub writecolour
{
	if ($curPass != $_[0]) { return; }

	unless (defined $_[1])
	{
		Carp::cluck "undefined colour passed in";
	}

	my $r = defined($_[1]->Red) ? $_[1]->Red : 255;
	my $g = defined($_[1]->Green) ? $_[1]->Green : 255;
	my $b = defined($_[1]->Blue) ? $_[1]->Blue : 255;
	my $a = 255;
	if ($_[1]->isa('SWF::Element::RGBA'))
	{
		if (defined($_[1]->Alpha))
		{
			$a = $_[1]->Alpha;
		}
	}

	my $num;
	$num = ($a * 2**24) + ($r * 2**16) + ($g * 2**8) + ($b);
	$compiledData .= pack $intfmt, $num;
}

sub writecolourasarray
{
	if ($curPass != $_[0]) { return; }

	unless (defined $_[1])
	{
		Carp::cluck "undefined colour passed in";
	}
	my $r = $_[1];
	my $g = $_[2];
	my $b = $_[3];
	my $a = defined ($_[4]) ? $_[4] : 255;

	my $num;
	$num = ($a * 2**24) + ($r * 2**16) + ($g * 2**8) + ($b);
	$compiledData .= pack $intfmt, $num;
}

sub writestyle
{
	if ($curPass != $_[0]) { return; }

	my $style = $_[1];
	if ($style->FillStyleType == 0)
	{
		writeint $_[0], $style->FillStyleType;
		writecolour $_[0], $style->Color;
		# pad.. lame
		writeint $_[0],0;
		writeint $_[0],0;
		writeint $_[0],0;
		writeint $_[0],0;
		writeint $_[0],0;
		writeint $_[0],0;
		writeint $_[0],0;
	}
	elsif ($style->FillStyleType == 0x10 || $style->FillStyleType == 0x12 || $style->FillStyleType == 0x13)
	{
        # 0x10 = linear gradient fill
        # 0x12 = radial gradient fill
        # 0x13 = focal radial gradient fill
		die "internal error: gradients should be textures by this point";
	}
	elsif ($style->FillStyleType == 0x40 || $style->FillStyleType == 0x41)
	{
		writeint $_[0], $style->FillStyleType;
		writecolour $_[0], $style->Color;
		writeint $_[0], $style->BitmapID;
		writeinvertedmatrix $_[0], [ $style->BitmapMatrix->ScaleX, $style->BitmapMatrix->RotateSkew0,
					$style->BitmapMatrix->RotateSkew1, $style->BitmapMatrix->ScaleY,
					$style->BitmapMatrix->TranslateX, $style->BitmapMatrix->TranslateY];
	}
}

sub writedelayedint($$)
{
	if ($curPass != $_[0]) { return; }
	
	if (defined $FIXUPS{$_[1]})
	{
		die "internal error: multiple fixups for $_[1] specified";
	}
	
	$FIXUPS{$_[1]} = length $compiledData;
	
	#printf "Marking: %s at %x\n", $_[1], $FIXUPS{$_[1]};
	
	writeint($_[0], 0);
}

sub writedelayedshort($$)
{
	if ($curPass != $_[0]) { return; }
	
	if (defined $FIXUPS{$_[1]})
	{
		die "internal error: multiple fixups for $_[1] specified";
	}
	
	$FIXUPS{$_[1]} = length $compiledData;
	
	printf "Marking: %s at %x\n", $_[1], $FIXUPS{$_[1]};
	
	writeshort($_[0], 0);
}


sub fixupint($$$)
{
    if ($curPass != $_[0]) { return; }

    if (!(defined $FIXUPS{$_[1]}))
    {
        die "internal error: fixup for $_[1] not found";
    }

    #printf "Fixingup: %s at %x\n", $_[1], $FIXUPS{$_[1]};

    substr($compiledData, $FIXUPS{$_[1]}, 4) = pack $intfmt, $_[2];
    
    $FIXUPS{$_[1]} = undef;
}


sub fixupshort($$$)
{
    if ($curPass != $_[0]) { return; }

    if (!(defined $FIXUPS{$_[1]}))
    {
        die "internal error: fixup for $_[1] not found";
    }

    printf "Fixingup: %s at %x\n", $_[1], $FIXUPS{$_[1]};

    if($intfmt eq 'V')
    {
        substr($compiledData, $FIXUPS{$_[1]}, 2) = pack 'V', $_[2];
    }
    else
    {
        substr($compiledData, $FIXUPS{$_[1]}, 2) = pack 'N', $_[2];
    }

    
    $FIXUPS{$_[1]} = undef;
}

sub writedelayedintptr($$)
{
	if ($curPass != $_[0]) { return; }
	
	if (defined $FIXUPS{$_[1]})
	{
		die "internal error: multiple fixups for $_[1] specified";
	}
	
	if($ptrsize eq '8')
	{
	    align $curPass, 8;
	}
	
	$FIXUPS{$_[1]} = length $compiledData;
	
	if($ptrsize eq '4')
    {
        $compiledData .= pack $intfmt, 0;
    }
    else
    {
        $compiledData .= pack $intfmt, 0;
        $compiledData .= pack $intfmt, 0;
    }
	#printf "Marking: %s at %x\n", $_[1], $FIXUPS{$_[1]};
}

# value in $_[2]
sub fixupintptr($$$)
{
    if ($curPass != $_[0]) { return; }

    if (!(defined $FIXUPS{$_[1]}))
    {
        die "internal error: fixup for $_[1] not found";
    }
    
    if($ptrsize eq '8')
    {
        align $curPass, 8;
    }
    
    my $length = length($compiledData);
    
    if($ptrsize eq '4')
    {
        substr($compiledData, $FIXUPS{$_[1]}, 4) = pack $intfmt, $_[2];
    }
    else
    {
        align $curPass, 8;
        if($intfmt eq 'V')
        {
            substr($compiledData, $FIXUPS{$_[1]}, 4) = pack 'V', $_[2];
        }
        else
        {
            substr($compiledData, $FIXUPS{$_[1]} + 4, 4) = pack 'N', $_[2];
        }
    }
    
    $FIXUPS{$_[1]} = undef;
    
    die "bad" if $length != length($compiledData);
}

sub writestring($$)
{
	if ($curPass != $_[0]) { return; }

	unless (defined $_[1])
	{
		Carp::cluck "undefined string passed in";
	}
	my $len = length($_[1]) + 1;
	$compiledData .= pack "a$len", $_[1];
}

=pod
sub writefixedstring($$$)
{
	if ($curPass != $_[0]) { return; }

	length($_[1]) < $_[2] or Carp::cluck("string is longer than fixed string length"); # account for terminator
	$compiledData .= pack "a$_[2]", $_[1];
}
=cut

sub markforfill
{
	if ($curPass != $_[0]) { return; }
	return length($compiledData);
}

sub fillto
{
	if ($curPass != $_[0]) { return; }
	my $pass = shift;
	my $startPos = shift;
	my $fillSize = shift;
	my $endPos = length($compiledData);
	my $fillAmount = $startPos + $fillSize - $endPos;
	die "fillto is trying to fill negative amount" if $fillAmount < 0;
	$compiledData .= pack "a$fillAmount";
}

sub colourColoned
{
	my $r = defined($_[0]->Red) ? $_[0]->Red : 255;
	my $g = defined($_[0]->Green) ? $_[0]->Green : 255;
	my $b = defined($_[0]->Blue) ? $_[0]->Blue : 255;
	my $a = 255;
	if ($_[0]->isa('SWF::Element::RGBA'))
	{
		if (defined($_[0]->Alpha))
		{
			$a = $_[0]->Alpha;
		}
	}
	return $r . ":" . $g . ":" .  $b . ":" . $a;
}

sub lengthcheckstart
{
	my $item = shift;
	push @lengthChecks, { name => $item, pos => length($compiledData) };
}

sub _getNameFor
{
	my $str;
	foreach my $item (@_)
	{
		$str .= $item->{name} . '/';
	}
	return substr($str, 0, -1);
}

sub lengthcheckstop
{
	my $item = shift;
	die "internal error: stopping '$item', expecting " . $lengthChecks[$#lengthChecks]->{name} unless ($lengthChecks[$#lengthChecks]->{name} eq $item);
	my $name = _getNameFor(@lengthChecks);
	$sizeTotals{$name} += length($compiledData) - $lengthChecks[$#lengthChecks]->{pos};
	pop @lengthChecks;
}

sub dumplengthstats
{
	my $total = 0;
	print '-'x79 . "\n";
	my @keys = keys %sizeTotals;

	@keys = sort @keys;

	foreach my $key (@keys)
	{
		my $k = $key;
		$k =~ s/(.*?)\//\t/g;
		print "$k: " . $sizeTotals{$key} . "\n";
		$total += $sizeTotals{$key};
	}
	die "unclosed keys: ".Dumper(\@lengthChecks) if (scalar @lengthChecks != 0);
	print '-'x79 . "\n";
}


1;
