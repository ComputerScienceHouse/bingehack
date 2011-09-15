#!/usr/bin/env perl

use strict;
use warnings;

use 5.010;

use Convert::Binary::C;
use JSON::XS;

$SIG{INT}  = 'teardown';
$SIG{QUIT} = 'teardown';
$SIG{TERM} = 'teardown';

use constant INCDIR  => 'inc/';

sub setup {
    state $c //= Convert::Binary::C->new(
        ByteOrder => 'LittleEndian',
        Include   => [INCDIR(), '/usr/include']
    );

    $c->parse_file(INCDIR()."hack.h");

    return $c;
}

sub teardown {
    exit(0);
}

sub recv_structs {
}

sub send_structs {
}

my $c = setup();
say $c->sizeof('monst')." bytes";
given ($ARGV[0]) {
    when ("save") { recv_structs($c) }
    when ("restore") { send_structs($c) }
}
