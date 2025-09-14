package AptDerive;
use strict;
use warnings;

sub getid_DefineSprite { $_[0]->SpriteID }
sub build_DefineSprite
{
	# sprites are special because they're the only nested tags.. this build_ func gets the whole tag rather than just the element
	my $tag = shift;
	my $element = $tag->{element};
	my $taglist = $tag->{subdata};
	my $movie = AptDerive::getMovie($taglist);

	die "wasn't expecting imports in sprites" if ($#{$movie->{imports}} >= 0);

	return ({
		type => 'sprite',
		movie => $movie->{frames}
	}, $element->SpriteID->value);
}

1;
