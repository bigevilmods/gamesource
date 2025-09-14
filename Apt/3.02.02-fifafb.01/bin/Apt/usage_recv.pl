#!/usr/bin/perl -w
use strict;
use IO::Socket;
use Socket;
my $portnum = 16384;
my $proto = getprotobyname('udp');
socket(RECVSOCK, PF_INET, SOCK_DGRAM, $proto) or die "Can't open socket: $!";
my $sin = sockaddr_in($portnum, INADDR_ANY);
bind(RECVSOCK, $sin) or die "Can't bind socket: $!";

my $buf;
for (;;) {
        my ($port, $addr) = unpack_sockaddr_in recv(RECVSOCK, $buf, 2048, 0);
		open LOG, ">>swfc_usage.log" or next;
        print LOG time() . ":" .  "$buf\n\n";
		close LOG;
}


