use strict;
use warnings;

if (not defined $ARGV[0])
{
	print "usage: ripdoc.pl <headerfile.h>\n";
	exit 1;
}
open IN, "$ARGV[0]" or die "couldn't open $ARGV[0]";

my @curfunc;
my @curfuncdoc;
my $infile = 0;
my $linenum = 0;

while (1)
{
	my $line = <IN>;
	$linenum++;
	last if not defined $line;
	chomp($line);

	if ($line =~ m/\/\/\/\/ripdoc:start:([a-zA-Z0-9.\\\/]+)/)
	{
		die "nested ripdoc:start's (line $linenum)" if $infile;
		open OUT, ">$1" or die "couldn't open $1 for output.";
		$infile = 1;
		next;
	}
	elsif ($line =~ m/\/\/\/\/ripdoc:end/)
	{
		die "ripdoc:end without ripdoc:start (line $linenum)" if not $infile;
		$infile = 0;
		close OUT;
		next;
	}

	if ($infile)
	{
		if ($line =~ m/\/\/\/\/(.*)$/)
		{
			print OUT "$1\n";
		}
		elsif ($line =~ m/\/\/\/ (.*)$/)
		{
			push @curfuncdoc, $1;
		}
		elsif ($#curfuncdoc >= 0 && $line =~ /[a-zA-Z0-9]/)
		{
			push @curfunc, $line;
		}
		elsif ($#curfuncdoc >= 0)
		{
			# remove leading tab if all have some
			my $all = 1;
			foreach my $funcline (@curfunc)
			{
				unless ($funcline =~ m/^\t/)
				{
					$all = 0;
					last;
				}
			}
			if ($all)
			{
				foreach my $funcline (@curfunc)
				{
					$funcline =~ s/^\t//;
				}
			}
			foreach my $funcline (@curfunc)
			{
				$funcline =~ s/\t/    /g;
			}

			print OUT ("\n" . '\begin{verbatim}' . "\n");
			print OUT (join "\n", @curfunc);
			print OUT ('\end{verbatim}' . "\n");
			print OUT ("\n" . '\begin{quote}' . "\n");
			print OUT (join("\n", @curfuncdoc) . "\n\n");
			print OUT ('\end{quote}' . "\n");
			undef @curfuncdoc;
			undef @curfunc;
		}
	}
}
close IN;
