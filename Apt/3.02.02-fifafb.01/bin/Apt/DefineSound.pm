package AptDerive;
use strict;
use warnings;
use AptMessage ':all';

my %streaminfo;
my $streamIndex = 1;

sub _decode($$$)
{
	my ($flag, $offset, $bits) = @_;
	my @bitMask = (0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff);

	return ($flag >> $offset) & $bitMask[$bits];
}

sub _writeWaveFile(%)
{
	my %info = @_;

    message 1, "SWFC: Sound not supported. Skipping sound asset '$info{filename}'\n";

    displayInfo "sounds", "$info{filename} : Size(Bytes):0 Rate(kHz):0\n";

}

sub getid_DefineSound { $_[0]->SoundID; }
sub build_DefineSound
{
	my $element = shift;
	my $id = $element->SoundID->value;
	my %soundinfo;
	$soundinfo{filename} = "$main::outdir{sound}\\$id";
	$soundinfo{format} = _decode($element->Flags, 4, 4);
	$soundinfo{rate} = _decode($element->Flags, 2, 2);
	$soundinfo{size} = _decode($element->Flags, 1, 1) ? 16 : 8;
	$soundinfo{channels} =_decode($element->Flags, 0, 1) ? 2 : 1;
	$soundinfo{samples} = $element->SoundSampleCount;
	$soundinfo{data} = $element->SoundData;

	_writeWaveFile(%soundinfo);

	return ({
		type => 'sound'
	}, $element->SoundID);
}

sub getid_SoundStreamHead2 { 0; }
sub getid_SoundStreamHead { 0; }
sub build_SoundStreamHead2 { build_SoundStreamHead(@_) }
sub build_SoundStreamHead
{	
	my $element = shift;
	flush_SoundStreams();

	$streaminfo{id}       = $streamIndex++;
	$streaminfo{filename} = sprintf("$main::outdir{sound}\\stream_%04d", $streaminfo{id});
	$streaminfo{format}   = 2;  # ??? always mp3?
	$streaminfo{rate}     = $element->StreamSoundRate;
	$streaminfo{size}     = $element->StreamSoundSize ? 16 : 8;
	$streaminfo{channels} = $element->StreamSoundType ? 2 : 1;
	$streaminfo{samples}  = $element->StreamSoundSampleCount;
	$streaminfo{data}     = "";
	$streaminfo{cursample}= 0;

	return undef;
}

sub getid_SoundStreamBlock { 0; }
sub build_SoundStreamBlock
{
	my $element = shift;

	$streaminfo{cursample} ++;
	$streaminfo{data} .= substr($element->StreamSoundData, 4);

	return undef;
}

sub flush_SoundStreams
{
	if ($streamIndex > 1 && $streaminfo{cursample} > 0)
	{
		_writeWaveFile(%streaminfo);
	}
}

1;
