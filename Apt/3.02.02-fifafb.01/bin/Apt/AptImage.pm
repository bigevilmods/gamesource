package AptImage;

# $senzee collection of image related utilities to replace gimex for perl version 5.8

sub WriteTgaHeaderRaw                        # ($width, $height, $bpp, $colormaptype, $imagetype, $colors)
{
    my $width        = shift;
    my $height       = shift;
    my $bpp          = shift;
    my $colormaptype = shift;
    my $imagetype    = shift;
    my $colors       = shift;    
    
    syswrite OUT, (pack 'C', 0);             # num chars in id field
    syswrite OUT, (pack 'C', $colormaptype); # colour map type 0 no palette or colour map type 1 paletted
    syswrite OUT, (pack 'C', $imagetype);    # image type 1: uncompressed colour mapped image or 2: rgb image or 3: grey scale
    syswrite OUT, (pack 'v', 0);             # base of colour map indices
    syswrite OUT, (pack 'v', $colors);       # length of colour map indices
    syswrite OUT, (pack 'C', 32);            # bpp for colour map entries
    syswrite OUT, (pack 'v', 0);             # x origin
    syswrite OUT, (pack 'v', 0);             # y origin
    syswrite OUT, (pack 'v', $width);        # width
    syswrite OUT, (pack 'v', $height);       # height
    syswrite OUT, (pack 'C', $bpp);          # number of bits stored in a pixel index
    syswrite OUT, (pack 'C', 32);            # image descriptor byte (32 == origin in top left)
}

sub WriteTga32                              # ($filename, $width, $height, $imagedata)
{
    my $filename  = shift;
    my $width     = shift;
    my $height    = shift;
    my $imagedata = shift;

    open OUT, ">$filename" or die "couldn't open $filename";
    binmode(OUT, ":raw");
    WriteTgaHeaderRaw($width, $height, 32, 0, 2, 0);
    syswrite OUT, (pack 'a*', $imagedata);  # image
    close OUT;
}

sub WriteTga8Palette32                      # ($filename, $width, $height, $imagedata, @palette)
{
    my $filename   = shift;
    my $width      = shift;
    my $height     = shift;
    my $imagedata  = shift;
    my @palette    = @_;   
    my $colors     = $#palette / 4;   
    
    open OUT, ">$filename" or die "couldn't open $filename";
    binmode(OUT, ":raw");
    WriteTgaHeaderRaw($width, $height, 8, 1, 1, $colors + 1);
    for (my $i = 0; $i < $colors; $i++)
    {
        my $j = $i * 4;
        syswrite OUT, pack('C4', ($palette[$j+0], $palette[$j+1], $palette[$j+2], $palette[$j+3]));
    }
    syswrite OUT, (pack 'a*', $imagedata);  # image
    close OUT;    
}

sub WriteTga8Gray                           # ($filename, $width, $height, $imagedata)
{
    my $filename   = shift;
    my $width      = shift;
    my $height     = shift;
    my $imagedata  = shift; 
    
    open OUT, ">$filename" or die "couldn't open $filename";
    binmode(OUT, ":raw");
    WriteTgaHeaderRaw($width, $height, 8, 0, 3, 0);
    syswrite OUT, (pack 'a*', $imagedata);
    close OUT;
}

sub WriteTga8GrayTo32                       # ($filename, $width, $height, $imagedata)
{
    my $filename   = shift;
    my $width      = shift;
    my $height     = shift;
    my $imagedata  = shift; 
    
    my $data = "", $color;
    for (my $i = 0; $i < length($imagedata); $i++)
    {
        $color = unpack 'C', substr($imagedata, $i, 1);
        $data .= pack('C4', ($color, $color, $color, $color));
    }    
    WriteTga32($filename, $width, $height, $data);
}

sub ReadJpgSize                             # ($filename)
{
    my $filename   = shift;    
    open IN, "<$filename" or die "couldn't open $filename";
    binmode(IN, ":raw");
    my $done = 0;
    my $marker = 0xff;
    my $length = 0;
    my $dummy = 0;
    my $width = 0;
    my $height = 0;
    while ($done == 0)
    {
        $marker = 0xff;
        while ($marker == 0xff)
        {
            sysread IN, $marker, 1;
            $marker = unpack 'C', $marker;
        }
        if ($marker != 0x01 && ($marker < 0xd0 || $marker > 0xd9))
        {        
            sysread IN, $length, 2;
            $length = (unpack 'n', $length) - 2;
            if ($marker >= 0xc0 && $marker <= 0xc3) # our start of frame markers
            {
                sysread IN, $marker, 1;
                sysread IN, $height, 2;
                sysread IN, $width,  2;
                $height = unpack 'n', $height;    # ye olde height
                $width  = unpack 'n', $width;     # & width       
                $done = 1;
            }
            else
            {
                sysread IN, $dummy, $length
            }
        }
    }
    close IN;
    
    return ($width, $height);
}

1;