#!/usr/bin/env perl

use strict;
use warnings;

use 5.010;

use Convert::Binary::C;
use Digest::SHA1;
use POSIX qw/mkfifo/;

$SIG{INT}  = 'teardown';
$SIG{QUIT} = 'teardown';
$SIG{TERM} = 'teardown';

use constant INCDIR => 'inc/';
use constant CMDFIFO => 'var/fifos/cmd';

sub setup {
    state $c //= Convert::Binary::C->new(
        ByteOrder => 'LittleEndian',
        Include   => [INCDIR(), '/usr/include']
    );

    $c->parse_file(INCDIR()."hack.h");

    mkfifo(CMDFIFO(), 644);
}

sub teardown {
    unlink(CMDFIFO()) or die "$!";
    exit(0);
}

sub recv_structs {
}

sub send_structs {
}

setup();
while (1) { };
