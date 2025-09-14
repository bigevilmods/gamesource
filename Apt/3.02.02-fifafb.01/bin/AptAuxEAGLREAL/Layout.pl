use strict;
use warnings;
use Gimex;
use IPC::Open3;

my @tgas = glob("\\fifa2004\\apt\\test\\out_textures\\*.tga");
my @jpgs = glob("\\fifa2004\\apt\\test\\out_textures\\*.jpg");
my @filenames = (@tgas, @jpgs);
my @texs = Gimex::Load(@filenames);
my @texprops = Gimex::GetProperties(@texs);
die "huh?" if $#filenames != $#texprops;

open (OUT, ">layouttmp") || die "couldn't open layouttmp";
for (my $i = 0; $i <= $#texprops; $i++)
{
	my $w = $texprops[$i]->{width};
	my $h = $texprops[$i]->{height};
	die "texture '$filenames[$i]' is too big ($w x $h)" if ($w > 1024 || $h > 1024);
	print OUT "$i|$w|$h\n";
}
close OUT;

my @pages;

open (IN, "arrange.exe|") || die "couldn't spawn";
while (<IN>)
{
	chomp;
	#print "$_\n";
	my @fields = split /\|/;
	Gimex::SetProperties($texs[$fields[0]], defaultx => $fields[2], defaulty => $fields[3]);
	push @{$pages[$fields[1]]}, $texs[$fields[0]];
}
close IN;

my @final;
for (my $pageIndex = 0; $pageIndex <= $#pages; $pageIndex++)
{
	my $page = $pages[$pageIndex];
	print "processing $pageIndex...\n";
	my $flattened = Gimex::Flatten($page);
	Gimex::SetProperties($flattened, framename => $pageIndex);
	push @final, Gimex::SetFormat($flattened, GMX_PALETTED_8BIT);
}
Gimex::Save(\@final, "tpage.fsh");
