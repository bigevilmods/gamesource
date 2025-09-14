my %ConstValues;  # The hash table of values to write out
my @names;
my @files;

# rstradling
# Process the files and store the values in the hash table
# Takes in the filename to read
sub ProcessFile($)
{
	my $fileName = shift;
	open IN, $fileName or die "Could not open the file $filename\n";
	my $data = <IN>;
	my @nodeHeaderData;
	
	if (defined $data)
	{
		chomp($data);
		@nodeHeaderData = split /\:/, $data;
		$ConstValues{$nodeHeaderData[0]} = "${nodeHeaderData[1]}";
		do
		{
			$data = <IN>;
			chomp($data);
			if (defined $data)
			{
				@nodeHeaderData = split /\:/, $data;
				$ConstValues{$nodeHeaderData[0]} = "${nodeHeaderData[1]}";			
			}
		}
		while defined ($data) 
	}
	close IN;
}

# rstradling
# write out all the strings in sorted order that were captured from.
# the const.txt files
# Takes in the name of the file to write to and open.
sub WriteFile($)
{
	my $outFile = shift;
	open OUT, ">$outFile" or die "Could not open the file $outFile\n";
	binmode(OUT, ":crlf");
	# write out the sorted hash table
	foreach my $key (sort keys %ConstValues)
	{
		print OUT "$key : $ConstValues{$key}\n";
	}		
	close OUT;
}

# rstradling
# prints out the syntax of the command-line if the parameters are incorrect
sub printSyntax()
{
	print "\n".
	      "ConstTextCollate . Collates the outputted *.const.txt files into one file\n\n".
		  "usage: ConstTextCollate <inputDir> <outputFileName>\n\n".
		  "\n";

}

# check the arguments
my $inputDir = $ARGV[0];
unless (defined $inputDir)
{
	printSyntax;
	exit 1;
}
my $outputFilename = $ARGV[1];
unless (defined $outputFilename)
{
	printSyntax;
	exit 1;
}


# open up the input directory
opendir(DIR, $inputDir) or die "Can't open the directory '$inputDir'\n";

# look at all the files
@names = readdir(DIR) or die "Unable to read the directory '$inputDir'\n";
closedir(DIR);

my $temp;
# find the files with the .const.txt extension
foreach $name (@names)
{
	next if ($name eq ".");
	next if ($name eq "..");
	if (-d $name)
	{
		# directory continue on.
		next;
	}
	if ($name =~ m/.\.const\.txt/) 
	{
		# found a .const.txt file
		$temp = "${inputDir}\\${name}";
		ProcessFile($temp);
		push @files, "$inputDir\\$name";

	}
	else
	{
		# do nothing.
	}
}

WriteFile($outputFilename);