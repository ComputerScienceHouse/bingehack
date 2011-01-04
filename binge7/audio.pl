#!/usr/bin/perl
use strict;
use warnings;
my $tts = "festival -tts";
my $board = "./rx";

open(my $BOARD, "$board |") or die "Couldn't open board executable.\n";

foreach my $line (<$BOARD>){
#chomp $line;
print $line;

}


sub play_text{
	my $text = shift;
	open(my $TTS, '>', "| $tts") or die $!;
	print TTS $text;
	close(TTS);
}
