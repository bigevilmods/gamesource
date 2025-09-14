# ***********************************************************************************************
#
#
#   \file    AptDerive.pl
#
#   \brief   SWF file decompiler
#
#   (c) 2007 Electronic Arts Inc.
#
#   \version    2.06.00    11/15/07 (RB) Last Version to support only SWF 7
#   \version    2.07.00    04/29/08 (RB) Appended to %controltags: PlaceObject3, ImportAssets2, FileAttributes and DefineScalingGrid.
#                                        Fixed whitespace in getCharacters().
#                                        Handle PlaceObject3 flags in getMovie().
#                                        Handle ImportAssets2 in getMovie().
#   \version    2.09.00-3  06/19/09 (RB) Warn if tag is being ignored,
#
# ***********************************************************************************************

package AptDerive;
use strict;
use warnings;
use Data::Dumper;
use SWF::Parser;
use SWF::Element;
use DefineShape;
use DefineBitmaps;
use DefineFont;
use DefineSprite;
use DefineButtons;
use DefineSound;
use DefineVideo;
use DefineMorph;

require Exporter;
our @ISA = qw(Exporter);
our @EXPORT = qw(%ObjectReallyGetReferenced);

# this hash contain the charactor ids who are referenced 
# The hash is constructed several place
# 1. in AptDerive, processing placeobject1/2 
# 2. in AptDerive, processing the exports list
# 3. in AptOutpur, go through all the buttons's 1/2/4 button records
our %ObjectReallyGetReferenced;

sub tagtype
{
	my ($tagname) = ref($_[0])=~/:([^:]+)$/;
	return $tagname;
}

my @auxCharacters;
my $lastRealID = 0;

sub addAuxCharacter
{
	push @auxCharacters, $_[0];
	return $#auxCharacters + $lastRealID;
}

my %controlTags = (
    'DoAction' => 1,
    'DoInitAction' => 1,
    'FrameLabel' => 1,
    'PlaceObject' => 1,
    'PlaceObject2' => 1,
    'PlaceObject3' => 1,	
    'Protect' => 1,
    'EnableDebugger' => 1,
    'EnableDebugger2' => 1,
    'RemoveObject' => 1,
    'RemoveObject2' => 1,
    'SetBackgroundColor' => 1,
    'ShowFrame' => 1,
    'End' => 1,
    'ImportAssets' => 1,
    'ImportAssets2' => 1,
    'FileAttributes' => 1,	
    'ExportAssets' => 1,
    'StartSound' => 1,
    'DefineScalingGrid' => 1,    
);

sub findLastID
{
	my $taglist = shift;

	foreach my $tag (@$taglist)
	{
		my $element = $tag->{element};
		my ($tagname) = ref($element)=~/:([^:]+)$/;

		next if ($controlTags{$tagname});

		my $charID;
		eval "(\$charID) = getid_$tagname(\$element);";
		print $@ and exit 1 if $@;

		if ($charID > $lastRealID)
		{
			$lastRealID = $charID;
		}
	}
	$lastRealID++;
}

# not sure what tag 25 is, but we're going to ignore it for now...
sub getid_UnknownTag25 { 0; }
sub build_UnknownTag25 { undef; }

# $bug #1020  SWF file conversion error: Tag No. 25 is unknown
sub getid_Unknown { 0; } 
sub build_Unknown { undef; }


# not sure what tag 63 is but is related to debugging: ignore
sub getid_UnknownTag63 { 0; }
sub build_UnknownTag63 { undef; }


sub getCharacters($)
{
    my $taglist = shift;
    my @characters;

    undef @auxCharacters;
    findLastID($taglist);

    #needed to add the following block as, there were problems if import id is the last one in characters list.
    foreach my $tag (@$taglist)
    {    
        my $element = $tag->{element};
        my ($tagname) = ref($element)=~/:([^:]+)$/;        
        if ($tagname eq 'ImportAssets' || $tagname eq 'ImportAssets2')
        {
            foreach my $import (@{$element->Assets})
            {
                my $pp = $import->ID->value;
                $characters[$pp] = undef;		
                if($pp >= $lastRealID)
                {
                    $lastRealID++;
                }
            }
        }
    }

    foreach my $tag (@$taglist)
    {
        my $element = $tag->{element};
        my ($tagname) = ref($element)=~/:([^:]+)$/;

        next if ($controlTags{$tagname});

        my $ret;
        my $position = undef;
        if ($tagname eq 'DefineSprite')
        {
            eval "(\$ret, \$position) = build_$tagname(\$tag);";
        }
        else
        {
            if ($tagname eq 'DefineBitsLossless')
            {
                ($ret, $position) = build_DefineBitsLossless($element);
            }
            else
            {
                eval "(\$ret, \$position) = build_$tagname(\$element);";
            }
        }
        if (defined $ret)
        {
            die "internal error: tag id (position) not returned from build_$tagname" unless defined($position);
            if (defined($characters[$position]))
            {
                $characters[$position] = {%{$characters[$position]}, %{$ret}};
            }
            else
            {
                $characters[$position] = $ret;
            }
            
        }
        else
        {
            message 1, "SWFC: ignoring swf tag '$tagname'\n";
        }
        print $@ and exit 1 if $@;
    }
    
    return [ @characters, @auxCharacters ];
}

sub getMovie($)
{
	my $taglist = shift;
	my $curSoundStream = 1;

	my @imports;
	my @exports;

	my @frames;
	my @curframe;

	foreach my $tag (@$taglist)
	{
		my $element = $tag->{element};
		my ($tagname) = ref($element)=~/:([^:]+)$/;

		#print "tagname: '$tagname'\n";
		
		my $data = { type => $tagname };
		if ($tagname eq 'DoAction' || $tagname eq 'DoInitAction')
		{
			my @actions;
			foreach my $individualAction (@{$element->Actions})
			{
				push @actions, $individualAction;
			}

			# stupid thing spits out empty DoAction's
			if ($#actions >= 0)
			{
				if ($tagname eq 'DoInitAction')
				{
					$data->{spriteid} = $element->SpriteID;
				}
				$data->{actions} = \@actions;
				push @curframe, $data;
			}
		}
		elsif ($tagname eq 'FrameLabel')
		{
			#print "Label: ${$element->Label}\n";
			push @curframe, { type => 'FrameLabel', label => $element->Name };
		}
		elsif ($tagname eq 'PlaceObject' || $tagname eq 'PlaceObject2' || $tagname eq 'PlaceObject3')
		{
			if ($tagname eq 'PlaceObject')
			{
				die "getMovie: Unexpected PlaceObject tag";
			}

			my $data = { type => $tagname };

			$data->{flags} = $element->Flags;
			$data->{depth} = $element->Depth;
            
            if ( $tagname eq 'PlaceObject3' )
            {
                if ($element->PlaceFlagHasFilterList)
                {
                    foreach my $filt (@{$element->SurfaceFilterList})
                    {
                        push @{$data->{filters}}, $filt; 
                    }
                }

                if ($element->PlaceFlagHasBlendMode)
                {
                    $data->{blendmode} = $element->BlendMode;
                }       
                
                if ($element->PlaceFlagHasCacheAsBitmap)
                {
                    $data->{cacheasbitmap} = $element->PlaceFlagHasCacheAsBitmap;
                }                         
            }
            
			if ($element->PlaceFlagHasCharacter)
			{
				$data->{characterid} = $element->CharacterID;
				$ObjectReallyGetReferenced{$element->CharacterID} = 1;
			}
			
			if ($element->PlaceFlagHasMatrix)
			{
				my $mat = $element->Matrix;
				$data->{matrix} = [
					$mat->ScaleX, $mat->RotateSkew0,
					$mat->RotateSkew1, $mat->ScaleY,
					$mat->TranslateX, $mat->TranslateY];
			}
			
			if ($element->PlaceFlagHasColorTransform)
			{
				my $cxform = $element->ColorTransform;
				$data->{cxform} = [
					$cxform->AlphaMultTerm,
					$cxform->RedMultTerm,
					$cxform->GreenMultTerm,
					$cxform->BlueMultTerm,

					$cxform->AlphaAddTerm,
					$cxform->RedAddTerm,
					$cxform->GreenAddTerm,
					$cxform->BlueAddTerm
				];
			}
			
			if ($element->PlaceFlagHasRatio)
			{
				$data->{ratio} = $element->Ratio;
			}

			if ($element->PlaceFlagHasName)
			{
				$data->{name} = $element->Name;
			}
			
			if ($element->PlaceFlagHasClipDepth)
			{
				$data->{clipdepth} = $element->ClipDepth;
			}
			
			if ($element->PlaceFlagHasClipActions)
			{
				foreach my $action (@{$element->ClipActions})
				{
					my @actions;
					foreach my $individualAction (@{$action->Actions})
					{
						push @actions, $individualAction;
					}
					# Following code is commented as there are no EventFlags6 in new swfc0.34
					#push @{$data->{actions}}, {
					#	triggerflags => $action->EventFlags | ((defined($action->EventFlags6) ? $action->EventFlags6 : 0) << 16),
					#	keycode => defined($action->KeyCode) ? $action->KeyCode : 0,
					#	actions => \@actions
					#};

					push @{$data->{actions}}, {
						triggerflags => $action->EventFlags,
						keycode => defined($action->KeyCode) ? $action->KeyCode : 0,
						actions => \@actions
					};
					#print $action->EventFlags ;
				}
			}
			
			push @curframe, $data;
		}
		elsif ($tagname eq 'RemoveObject' || $tagname eq 'RemoveObject2')
		{
			if ($tagname eq 'RemoveObject')
			{
				die "hmm.. didn't thing RemoveObject was ever used";
			}
			$data->{depth} = $element->Depth;
			push @curframe, $data;
		}
		elsif ($tagname eq 'SetBackgroundColor')
		{
			my $colour = $element->BackgroundColor;
			$data->{colour} = $colour;
			push @curframe, $data;
		}
		elsif ($tagname eq 'ShowFrame')
		{
			push @frames, [ @curframe ];
			undef @curframe;
		}
		elsif ($tagname eq 'StartSound')
		{
			my $id = $element->SoundID;
			$data->{id} = $id;
			push @curframe, $data;
		}
		elsif ($tagname eq 'SoundStreamHead')
		{
			$data->{id} = $curSoundStream++;
			push @curframe, $data;
		}
		elsif ($tagname eq 'ExportAssets')
		{
			foreach my $export (@{$element->Assets})
			{
				push @exports, {
					name => $export->Name,
					id => $export->ID->value
				};
				$AptDerive::ObjectReallyGetReferenced{$export->ID->value} = 1;
				
			}
		}
		elsif ($tagname eq 'ImportAssets' || $tagname eq 'ImportAssets2')
		{
			foreach my $import (@{$element->Assets})
			{
				push @imports, {
					url => $element->URL,
					name => $import->Name,
					id => $import->ID->value
				};
			}
		}
		else
		{
			# ignore all characters and other control tags that we don't care about/understand
			#print "(debug info, not necessarily bad) Ignored $tagname\n";
		}
	}

	return {
		frames => \@frames,
		imports => \@imports,
		exports => \@exports
	};

}

sub getTimeStamp($)
{
    my $taglist = shift;

	foreach my $tag (@$taglist)
	{
		my $element = $tag->{element};
		my ($tagname) = ref($element)=~/:([^:]+)$/;
		
		if ($tagname eq 'UnknownTag63')
		{
            my @stampArray;
            for my $i (@{$element->Mark})
            {
                push @stampArray, @{$i}[0];
            }
            return { tag => 63, stamp => \@stampArray };
		}
	}
    return undef;
}

sub isDebugSwf($)
{
    my $taglist = shift;
    
    foreach my $tag (@$taglist)
    {
        my $element = $tag->{element};
        my ($tagname) = ref($element)=~/:([^:]+)$/;
        
        if ($tagname eq 'EnableDebugger2')
        {
            return 1;
        }
    }
    return 0;
}
