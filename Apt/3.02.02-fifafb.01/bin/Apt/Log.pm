use strict;
use warnings;
use Sys::Hostname; 
use IO::Socket;
use Socket;

eval
{
	my $destaddr = "10.10.34.127";
	#my $destaddr = "127.0.0.1";
	my $destport = "16384";

	my $proto = getprotobyname('udp');
	socket(SENDSOCK, PF_INET, SOCK_DGRAM, $proto) or die "Can't open socket: $!";
	my $iaddr = gethostbyname($destaddr);
	if (not defined $iaddr) {
		die "Can't resolve $destaddr\n";
	}
	my $sin = sockaddr_in($destport, $iaddr);
	connect(SENDSOCK, $sin) or die "Can't connect socket: $!";

	my $sendstr = hostname() . "\n" . Win32::LoginName() . "\n" . "version" . "\n" . time() . "\n" . "@ARGV";
#	print "sending: '$sendstr'\n";
	while (!send(SENDSOCK, $sendstr, 0)) {
		warn "error: $!\n";
	}
};

1;
