package AptMessage;
require Exporter;

our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Win32::Guidgen ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
message
displayInfo
$verbosity
$displayInfo
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
);
	
our $verbosity = 0;

sub message
{
	my $level = shift;
	if ($level <= $verbosity)
	{
		print @_;
	}
}


# Gerneral Purpose "Info" function added by Daniel.
# This looks for the string passed in by the code to see if that string is 
# added on the command line by the user. If the token is present in the user 
# "allowed" list, the string is displayed. Note that the list can be added to
# without modification to this function.
#
# Example command line: "swfc.pl --info=textures,geometry ..\tmp.swf"
#      would diplay messages from the code with a "textures" or "geometry" token.
#      Messages with a "sounds" token for example would not be displayed.
#
# A user specified value of 'all' will display all messages regardless of their 
# token.
#
# issues: This may not react correctly if tokens are substrings of each other. 
# but this should not be an issue (i.e. substring will find 'sound' in 
# 'resounding' even if it is not intended.
#
our $displayInfo = 0;

sub displayInfo
{
	my $passToken = shift;
	
	if ( index($displayInfo, $passToken) >= 0 || $displayInfo eq 'all' )
	{
		print @_;
	}
}

1;
