package AptDerive;
use strict;
use warnings;

sub getid_DefineMorphShape2 { getid_DefineMorphShape(@_) }
sub build_DefineMorphShape2 { build_DefineMorphShape(@_) }

sub getid_DefineMorphShape { $_[0]->CharacterID }
sub build_DefineMorphShape
{
	my $element = shift;
	
	my $endShape = 0;
	my $fills = $element->MorphFillStyles;
	my $lines = $element->MorphLineStyles;
	my @builtShapes;

	for ($endShape=0; $endShape<2; $endShape++)
	{
		my $newFills = SWF::Element::Array::FILLSTYLEARRAY3->new;
		my $newLines = SWF::Element::Array::LINESTYLEARRAY3->new;

		# for some random reason, fillstyle0/1 is reverse for morph shapes
		my $shaperecs = $endShape ? $element->EndEdges->ShapeRecords() : $element->StartEdges->ShapeRecords();
		foreach my $shape (@$shaperecs)
		{
			my $tagname = tagtype($shape);

			if ($tagname eq 'STYLECHANGERECORD')
			{
				my $fillStyle0 = $shape->FillStyle0;
				my $fillStyle1 = $shape->FillStyle1;
				$shape->configure(FillStyle0 => $fillStyle1, FillStyle1 => $fillStyle0); 
			}
		}

		for (my $index = 0; $index <= $#$fills; $index++)
		{
			my $fill = $fills->[$index];

			my $newFill = SWF::Element::FILLSTYLE3->new;
			$newFill->configure(FillStyleType => $fill->FillStyleType); 
			$newFill->configure(Color => $endShape ? $fill->EndColor : $fill->StartColor);
			$newFill->configure(GradientMatrix => $endShape ? $fill->EndGradientMatrix : $fill->StartGradientMatrix);
			my @gradRecords;
			for (my $gIndex = 0; $gIndex <= $#{$fill->Gradient}; $gIndex++)
			{
				my $grad = $fill->Gradient->[$gIndex];
				push @gradRecords, SWF::Element::GRADRECORD3->new(Ratio => $endShape ? $grad->StartRatio : $grad->EndRatio,
				                                                Color => $endShape ? $grad->StartColor : $grad->EndColor);
			}
			$newFill->configure(Gradient => SWF::Element::Array::GRADIENT3->new(@gradRecords));
			$newFill->configure(BitmapID => $fill->BitmapID);
			$newFill->configure(BitmapMatrix => $endShape ? $fill->EndBitmapMatrix : $fill->StartBitmapMatrix);

			push @$newFills, $newFill;
		}

		for (my $index = 0; $index <= $#$lines; $index++)
		{
			my $line = $lines->[$index];

			my $newLine = SWF::Element::LINESTYLE3->new;
			$newLine->configure(Width => $endShape ? $line->EndWidth : $line->StartWidth);
			$newLine->configure(Color => $endShape ? $line->EndColor : $line->StartColor);
			
			push @$newLines, $newLine;
		}

		my %shapes;
		if ($endShape)
		{
			%shapes = buildShapes(shaperecs => $shaperecs, fills => $newFills, lines => $newLines, 
			                      curFillStyle0Index => $builtShapes[0]->{curFillStyle0Index}, 
								  curFillStyle1Index => $builtShapes[0]->{curFillStyle1Index}, 
								  curLineStyleIndex => $builtShapes[0]->{curLineStyleIndex});
		}
		else
		{
			%shapes = buildShapes(shaperecs => $shaperecs, fills => $newFills, lines => $newLines);
		}
		$builtShapes[$endShape] = \%shapes;
	}

	my $startChar = {
		type => 'shape',
	};
	$startChar->{id} = addAuxCharacter($startChar);
	$startChar->{shapes} = \@{$builtShapes[0]->{shapes}};
	my $endChar = {
		type => 'shape',
	};
	$endChar->{id} = addAuxCharacter($endChar);
	$endChar->{shapes} = \@{$builtShapes[1]->{shapes}};

	return ({
		type => 'morph',
		boundingRect => [
			$element->StartBounds()->Xmin,
			$element->StartBounds()->Ymin,
			$element->StartBounds()->Xmax,
			$element->StartBounds()->Ymax],
		startCharID => $startChar->{id},
		endCharID => $endChar->{id},
	}, $element->CharacterID);
}

1;
