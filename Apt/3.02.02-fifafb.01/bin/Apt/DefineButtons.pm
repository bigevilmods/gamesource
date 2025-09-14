package AptDerive;

sub getid_DefineButton2 { return $_[0]->ButtonID }
sub build_DefineButton2
{
	my $element = shift;

	my $isMenu = (defined $element->Flags && $element->Flags != 0) ? 1 : 0;

	my @buttonRecords;
	foreach my $buttonRecord (@{$element->Characters()})
	{
		my %br;
		$br{states} = $buttonRecord->ButtonStates;
		$br{character} = $buttonRecord->CharacterID;
		$br{layer} = $buttonRecord->PlaceDepth;

		my $mat = $buttonRecord->PlaceMatrix;
		$br{matrix} = [
			$mat->ScaleX, $mat->RotateSkew0,
			$mat->RotateSkew1, $mat->ScaleY,
			$mat->TranslateX, $mat->TranslateY];

		$br{cxform} = $buttonRecord->ColorTransform;
		push @buttonRecords, \%br;
	}

	my @actionRecords;
	foreach my $actionRecord (@{$element->Actions()})
	{
		my %ar;
		$ar{condition} = $actionRecord->Condition;
		$ar{actions} = $actionRecord->Actions();
		push @actionRecords, \%ar;
	}

	return ({
		type => 'button',
		id => $element->ButtonID->value,
		ismenu => $isMenu,
		buttonrecords => \@buttonRecords,
		actionrecords => \@actionRecords,
	}, $element->ButtonID->value);
}

sub getid_DefineButtonSound { return $_[0]->ButtonID }
sub build_DefineButtonSound
{
	my $element = shift;

	my %soundID;
	$soundID{OverUpToIdle} = $element->ButtonSoundChar0->value;
	$soundID{IdleToOverUp} = $element->ButtonSoundChar1->value;
	$soundID{OverUpToOverDown} = $element->ButtonSoundChar2->value;
	$soundID{OverDownToOverUp} = $element->ButtonSoundChar3->value;

	return ({
		sound => {overuptoidle => $element->ButtonSoundChar0->value,
		          idletooverup => $element->ButtonSoundChar1->value,
		          overuptooverdown => $element->ButtonSoundChar2->value,
		          overdowntooverup => $element->ButtonSoundChar3->value}
	}, $element->ButtonID->value);
}

1;
