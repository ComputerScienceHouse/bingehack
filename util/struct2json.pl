#!/usr/bin/env perl

use strict;
use warnings;

use 5.010;

use Convert::Binary::C;
use Data::Dumper;
use JSON::XS;

use constant INCDIR  => 'inc/';

sub setup {
    state $c //= Convert::Binary::C->new(
        ByteOrder => 'LittleEndian',
        Include   => [INCDIR(), '/usr/include']
    );

    $c->parse_file(INCDIR()."hack.h");

    return $c;
}

sub recv_structs {
    while (<STDIN>) {
        my $struct_name = $_;
        my $struct_size = $c->sizeof($struct_name);

        read(<STDIN>, my $struct_data, $struct_size) or die $!;
        my $struct_obj = $c->unpack($struct_name, $struct_data);
		print Dumper($struct_obj);
    }
}

sub send_structs {
}

my $c = setup();
given ($ARGV[0]) {
    when ("save") { recv_structs($c) }
    when ("restore") { send_structs($c) }
}
