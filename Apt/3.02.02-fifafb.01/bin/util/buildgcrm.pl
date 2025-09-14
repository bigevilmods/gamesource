use strict;
use warnings;
use FindBin;
use Data::Dumper;
use File::Glob ':glob';
use File::Basename;
use File::Path;
use File::Copy;

sub canonpath($)
{
	my $path = shift;
	
	while ($path =~ s'[/\\]\w+[/\\]\.\.'')
	{}

	$path =~ s/\//\\/g;	
	return $path;
}

my $sku = 'ngc';
my $eaglsku = 'gc';

if (not defined($ENV{EAGLHOME})) {
	die "error: 'EAGLHOME' env var must be set\n";
}
if (not defined($ENV{CWFOLDER})) {
	die "error: 'CWFOLDER' env var must be set\n";
}
if (not defined($ENV{DolphinRoot})) {
	die "error: 'DolphinRoot' env var must be set\n";
}
if (not defined($ENV{REALCORE})) {
	die "error: 'REALCORE' env var must be set\n";
}

my $bindir = "$ENV{EAGLHOME}\\bin.pc";
my $outdir = "$FindBin::Bin\\..\\..\\build\\rms\\$sku";
my $cwdir = $ENV{CWFOLDER};
my $cwbindir = $cwdir."\\PowerPC_EABI_Tools\\Command_Line_Tools\\";
my $dolphindir = $ENV{DolphinRoot};
my $realdir = $ENV{REALCORE};
my $eagldir = $ENV{EAGLHOME};

$bindir = canonpath($bindir);
$outdir = canonpath($outdir);

$ENV{"RUNTIME_TARGET"} = $eaglsku;
$ENV{"EAGL_${eaglsku}_RMS_PATH"} = "$ENV{EAGLHOME}\\$eaglsku\\rendermethods";
#print "set "."EAGL_${eaglsku}_RMS_PATH"."= "."$rmsdir;$ENV{EAGLHOME}\\$eaglsku\\rendermethods\n";
$ENV{"EAGL_${eaglsku}_COMPUTATION_PATH"} = "$ENV{EAGLHOME}\\$eaglsku\\computations";
#print "set "."EAGL_${eaglsku}_COMPUTATION_PATH"."= "."$FindBin::Bin\\..\\..\\source\\AptAuxEAGLREAL\\$eaglsku\\computations;$ENV{EAGLHOME}\\$eaglsku\\computations\n";
system("del /s /f /q \"$outdir\" > NUL" );
rmdir("$outdir");
mkpath("$outdir");

my @ldFiles;

my @ngcRmsFiles = ("GouraudApt.rms", 
                   "TextureApt.rms");

foreach my $rmsFile (@ngcRmsFiles)
{
	my ($basename, $path, $suffix) = fileparse($rmsFile, "\.rms");
	
	print "Building '$basename.rms'...\n";
	
	my $buildrm = "$bindir\\$eaglsku\\buildrm.exe -i $rmsFile -o $outdir";
    chdir("$ENV{EAGLHOME}\\$eaglsku\\rendermethods");
	system($buildrm) and die "error: buildrm failed on '$rmsFile'";	

#	my $compilecmd = "$cwbindir\\mwcceppc.exe -d NGC -d GC -d GEKKO -d HW2 -d BUG_CLR_LOAD_DLY -d TRK_INTEGRATION -d MARLIN=DI -d EPPC -d ORCA -d GX_REV=2 -d GXFIFO_SERIAL -d __HWSIM=0 -d __MIPSEL__ -d _DEBUG";
	my $compilecmd = "$cwbindir\\mwcceppc.exe -d NGC -d GC -d GEKKO -d HW2 -d BUG_CLR_LOAD_DLY -d TRK_INTEGRATION -d MARLIN=DI -d EPPC -d ORCA -d GX_REV=2 -d GXFIFO_SERIAL -d __HWSIM=0 -d __MIPSEL__";
	$compilecmd .= " -i \"$cwdir\\PowerPC_EABI_Support\\MSL\\MSL_C\\MSL_Common\\Include\"";
	$compilecmd .= " -i \"$cwdir\\PowerPC_EABI_Support\\MSL\\MSL_C\\PPC_EABI\\Include\"";
	$compilecmd .= " -i \"$cwdir\\PowerPC_EABI_Support\\Include\\Prefix\"";
	$compilecmd .= " -i \"$dolphindir\\include\"";
	$compilecmd .= " -i \"$dolphindir\\include\"";
	$compilecmd .= " -i \"$eagldir\\include\\cmn\"";
	$compilecmd .= " -i \"$eagldir\\include\\gc\"";
	$compilecmd .= " -i \"$realdir\\include\"";
	
	$compilecmd .= " -nodefaults  -nosyspath  -w on  -w noextracomma  -w nocomma  -w noimplicit  -w nopedantic  -w nounusedarg  -w nounusedvar  -w noempty  -align powerpc  -RTTI off  -char signed  -enum int  -proc gekko  -c  -msgstyle gcc  -fp hardware  -Cpp_exceptions off  -g  -O0  -sym on ";
	$compilecmd .= "  -o $basename.o";
	$compilecmd .= "  -c $basename.cpp";
	print("$compilecmd\n");
    chdir($outdir);     
	system($compilecmd);
	push @ldFiles, "$outdir\\$basename.elf";
}

my $linkercmd = "$cwbindir\\mwldeppc.exe -library -fp hardware ";
$linkercmd .= "-o $FindBin::Bin\\..\\..\\lib\\$sku\\aptrm.a ";
print "linking...\n";
foreach my $rmsFile (@ngcRmsFiles)
{
	my ($basename, $path, $suffix) = fileparse($rmsFile, "\.rms");
	$linkercmd .= " $basename.o";
}	
print("$linkercmd\n");
system($linkercmd);
		
