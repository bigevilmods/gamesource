# quickdump.pl - (c) 2009 Electronic Arts
#
# This script dumps a binary file as C-style hex values.
# usage: quickdump.pl filename [-L | columns]
#
# filename: input file to dump
# -L:       means that you only want to output the file length, not dump it
# columns:  number of columns to dump on one line; default is 16.

use strict;

my $filename = $ARGV[0];    # filename
my $cols = $ARGV[1];        # must be number of columns, or -L
my $buf;                    # current byte being read
my $i = 0;                  # counter
my $only_print_length = 0;  # is 1 if and only if -L has been specified


open( FILE, $filename ) || die "Couldn't open $filename: $!\n";
binmode FILE;

if ( $cols == "-L" )
{
    $only_print_length = 1;
}
elsif ( $cols == 0 )
{
    $cols = 16;
}

while ( read( FILE, $buf, 1 ) ) 
{
    if ( $only_print_length==0 )
    {
        printf( "0x%02x, ", unpack( 'U8', $buf ) );
    }

    $i++;

    if ( $only_print_length==0 )
    {
        printf("\n") if ($i % $cols) == 0;
    }
}
close(FILE);

print($i) if $only_print_length==1;

