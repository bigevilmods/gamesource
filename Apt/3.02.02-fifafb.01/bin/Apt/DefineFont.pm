# ***********************************************************************************************
#
#
#   \file    DefineFont.pl
#
#   \brief   SWF file decompiler
#
#   (c) 2007 Electronic Arts Inc.
#
#   \version    2.06.00    11/15/07 (RB) Last Version to support only SWF 7
#   \version    2.07.00    04/29/08 (RB) Handle DefineFont3, which just passes through to DefineFont2.
#                                        Added empty callbacks for CSMTextSettings and DefineFontAlignZone tags.
#
# ***********************************************************************************************

package AptDerive;
use strict;
use warnings;

my %defineFontInfo;
my %fontUsedInStaticText;


sub getid_DefineFont3 { getid_DefineFont2(@_) }
sub getid_DefineFont2 { getid_DefineFont(@_) }
sub getid_DefineFont { $_[0]->FontID }

sub build_DefineFont3 { build_DefineFont2(@_) }
sub build_DefineFont2 { build_DefineFont(@_) }
sub build_DefineFont
{
    my $element = shift;

    my $glyphs = undef;
    if ($fontUsedInStaticText{$element->FontID})
    {
        my $glyphshapetable = $element->GlyphShapeTable;
        my $newFills = SWF::Element::Array::FILLSTYLEARRAY1->new;
        my $newLines = SWF::Element::Array::LINESTYLEARRAY1->new;
        $newFills->[0] = SWF::Element::FILLSTYLE1->new(FillStyleType => 0, Color => SWF::Element::RGB->new( Red => 255, Green => 255, Blue => 255 ));

        my @glyphIDs;
        foreach my $glyph (@$glyphshapetable)
        {
            my $shaperecs = $glyph->ShapeRecords;
            my %shapes = buildShapes(shaperecs => $shaperecs, fills => $newFills, lines => $newLines);
            my $char = {
                type => 'shape',
                shapes => \@{$shapes{shapes}}
            };
            push @glyphIDs, addAuxCharacter($char);
        }

        $glyphs = \@glyphIDs;
    }

    my $name = undef;
    if (tagtype($element) eq 'DefineFont2' || tagtype($element) eq 'DefineFont3')
    {
        $name = $element->FontName->value;
    }elsif (tagtype($element) eq 'DefineFont')
    {
        if(defined ($defineFontInfo{$element->FontID}))
        {
            $name = $defineFontInfo{$element->FontID}->FontName;
        }else
        {
            print ("un-resolved DefineFont tag\n");
        }
    }
    return ({
        type => 'font',
        glyphs => $glyphs,
        name => $name,
    }, $element->FontID);
}

sub getid_DefineFontInfo2 { return getid_DefineFontInfo(@_) }
sub getid_DefineFontInfo 
{
	my $element = shift;
	$defineFontInfo{$element->FontID} = $element;
	return $element->FontID;
}

sub build_DefineFontInfo2 
{
	 build_DefineFontInfo(@_) 
}
sub build_DefineFontInfo
{
	return undef;
}

sub getid_DefineText2 { getid_DefineText(@_) }
sub getid_DefineText
{
	# a bit of a hack to note any fonts that actually require glyphs exported.
	# getid_XXX is called before all the build_, which is really what we want here
	my $element = shift;

	my $curFont;
	foreach my $textrec (@{$element->TextRecords})
	{
		my $tagname = tagtype($textrec);

		if ($tagname eq 'TYPE1') # type 1 is style change, type 0 is glyph
		{
			$curFont = $textrec->FontID if defined($textrec->FontID->value);
		}
		$fontUsedInStaticText{$curFont} = 1;
	}

	return $element->CharacterID;
}

sub build_DefineText2 { build_DefineText(@_) }
sub build_DefineText
{
	my $element = shift;

	my @records;
	my $curRecord = undef;

	my $curFont;
	my $curCXForm;
	my $curXOffset = 0;
	my $curYOffset = 0;
	my $curScale = 1.0;
	foreach my $textrec (@{$element->TextRecords})
	{
		my $tagname = tagtype($textrec);

		if ($tagname eq 'TYPE1') # type 1 is style change, type 0 is glyph
		{
			if (defined $curRecord)
			{
				push @records, $curRecord;
			}
			$curFont = $textrec->FontID if defined($textrec->FontID->value);
			if (defined($textrec->TextColor))
			{
				$curCXForm = [
					(tagtype($textrec->TextColor) eq 'RGBA') ? $textrec->TextColor->Alpha : 255,
					$textrec->TextColor->Red,
					$textrec->TextColor->Green,
					$textrec->TextColor->Blue,
					0, 0, 0, 0 ];
			}
			$curXOffset = $textrec->XOffset if defined($textrec->XOffset);
			$curYOffset = $textrec->YOffset if defined($textrec->YOffset);
			$curScale = $textrec->TextHeight / 1000.0 if defined($textrec->TextHeight);
			$curRecord = {
				font => $curFont,
				cxform => $curCXForm,
				xoffset => $curXOffset,
				yoffset => $curYOffset,
				scale => $curScale,
				glyphs => []
			};
		}
		else
		{
			foreach my $glyphentry (@{$textrec->GlyphEntries})
			{
				push @{$curRecord->{glyphs}}, { index => $glyphentry->GlyphIndex, advance => $glyphentry->GlyphAdvance };
			}
		}
	}
	if (defined $curRecord)
	{
		push @records, $curRecord;
	}

	return ({
			type => 'statictext',
				boundingRect => [
					$element->TextBounds->Xmin,
					$element->TextBounds->Ymin,
					$element->TextBounds->Xmax,
					$element->TextBounds->Ymax],
				matrix => [
					$element->TextMatrix->ScaleX, $element->TextMatrix->RotateSkew0,
					$element->TextMatrix->RotateSkew1, $element->TextMatrix->ScaleY,
					$element->TextMatrix->TranslateX, $element->TextMatrix->TranslateY],
				records => \@records
			}, $element->CharacterID);
}



sub getid_DefineEditText { $_[0]->CharacterID }
sub build_DefineEditText
{
	my $element = shift;

	return ({
		type => 'text',
		id => $element->CharacterID,
		boundingRect => [
		$element->Bounds->Xmin,
		$element->Bounds->Ymin,
		$element->Bounds->Xmax,
		$element->Bounds->Ymax],
		fontid => $element->FontID,
		colour => $element->TextColor,
		alignment => $element->Align,
		fontheight => $element->FontHeight,
		readonly => $element->ReadOnly,
		multiline => $element->Multiline,
		wordwrap => $element->WordWrap,
		initialtext => $element->InitialText,
		variable => $element->VariableName,
		}, $element->CharacterID);
}

sub getid_DefineFontAlignZones {
    # $rburkett: TODO: Is this OK, or should it be more complex like getid_DefineText?
    return $_[0]->FontID;
}
sub build_DefineFontAlignZones
{
    return undef;
}


sub getid_CSMTextSettings { 
    return $_[0]->TextID;
}
sub build_CSMTextSettings
{
    return undef;
}

sub getid_DefineFontName { 
    return $_[0]->FontID;
}
sub build_DefineFontName
{
    return undef;
}

1;
