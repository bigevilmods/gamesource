package AptDerive;
use strict;
use warnings;

my $tempfilename;
sub getid_DefineVideoStream { $_[0]->CharacterID }
sub build_DefineVideoStream
{
	my $element = shift;
	my $id = $element->CharacterID->value;
	my $width = $element->Width;
	my $height = $element->Height;
	my $numframes = $element->NumFrames;
	$tempfilename = "$main::outdir{video}\\$id.263";

	# we only do codec H.263
	if ($element->CodecID != 2)
	{
		my $codec = $element->CodecID;
		die "error: unknown video codec '$codec'";
	}

	return ({
		type => 'video',
		id => $id,
		tempfilename => $tempfilename
		}, $element->CharacterID);
}

sub build_VideoFrame
{
	my $element = shift;
	my $id = $element->StreamID;
	my $framenum = $element->FrameNum;

	open OUT, ">>$tempfilename" or die "couldn't open $tempfilename";
	binmode(OUT, ":raw");
	syswrite OUT, $element->VideoData;
	close OUT;

	return undef;
}

1;