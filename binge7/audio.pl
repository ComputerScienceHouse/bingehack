#!/usr/bin/perl
use strict;
use warnings;

my $tts = "festival --pipe";
my $board = "./rx";
my %players = ();

my %race_adjectivizer = (elf => "elvish", barbarian => "barbarian");
open(my $BOARD, "$board |") or die "Couldn't open board executable.\n";

while (<$BOARD>){
#chomp $line;
my $line = $_;
print "Got: $line";
print "\n";

my @player_import = split(",", $line);
my ($name, $race, $gender, $alignment, $class, $hp, $hpmax, $ulevel, $ac, $num_prayers, $num_wishes, $num_deaths, $num_moves, $dlevel, $msg) = @player_import;

#Fixing a bug :-)
if($name eq "port34"){
	$name = "port__";
}

if(exists($players{ $name } )){
	# check to see if something important changed
	say_if_important(player_hasher(@player_import));
	$players{ $name } = player_hasher(@player_import);
}
else{
	my %player_stats = %{player_hasher(@player_import)};
	$players{ $name } = \%player_stats;
	my $long_class = class_expander($class);
	play_text("$name the $race $long_class has entered $msg!\n");
}


}

sub say_if_important{
	my $ptr = shift;
	my %new_player_stats = %{$ptr};
	my $name = $new_player_stats{ 'name' };
	my %old_player_stats = %{$players{ $name }};
	my $old_race = $old_player_stats{'race'};
	my $new_race = $new_player_stats{'race'};
	my $old_gender = $old_player_stats{'gender'};
	my $new_gender = $new_player_stats{'gender'};

	unless( $old_race eq $new_race ){
		my $a_or_an = starts_with_vowel($new_race)? "an": "a";
		play_text("$name the $old_race has changed into $a_or_an $new_race.\n");
	}
	unless($old_gender eq $new_gender ){
		play_text("$name decided to bat for the other team. ${gender_pronoun($old_gender)} is now a ${gender_pronoun($new_gender)}.\n");
	}
	unless($old_player_stats{ 'num_prayers' } eq $new_player_stats{ 'num_prayers' } ){
		play_text("$name got down on ${gender_possessive($new_gender)} knees and prayed to ${gender_possessive($new_gender)} god.\n");
	}
	unless($old_player_stats{ 'num_wish' } eq $new_player_stats{ 'num_wish' }){
		play_text("$name pretends that airplanes in the night sky are like shooting stars. ${gender_pronoun($new_gender)} made a wish!\n");
	}
}

sub gender_pronoun{
	if(shift =~ m/^male/){
		return "he";
	}
	else{
		return "her";
	}
}

sub gender_possessive{
	return gender_pronoun(shift)? "his" : "her";
}

sub class_expander{
	my $class = shift;
	$class =~ tr/[A-Z]/[a-z]/;
	my $gender = shift;
	if($class eq "arc"){
		return "archeologist";
	}
	if($class eq "bar"){
		return "barbarian";
	}
	if($class eq "cav"){
		if($gender eq "m"){
			return "caveman";
		}
		if($gender eq "f"){
			return "cavewoman";
		}
	}
	if($class eq "hea"){
		return "healer";
	}
	if($class eq "kni"){
		return "knight";
	}
	if($class eq "mon"){
		return "monk";
	}
	if($class eq "pri"){
		if($gender eq "m"){
			return "priest";
		}
		if($gender eq "f"){
			return "priestess";
		}
	}
	if($class eq "ran"){
		return "ranger";
	}
	if($class eq "rog"){
		return "rogue";
	}
	if($class eq "sam"){
		return "samurai";
	}
	if($class eq "tou"){
		return "tourist";
	}
	if($class eq "val"){
		return "valkyrie";
	}
	if($class eq "wiz"){
		return "wizard";
	}
	return "UNKNOWN CLASS";
}
sub starts_with_vowel{
	my $str = shift;
	if($str =~ m/^{aeiou}/){
		return 1;
	}
	return 0;
}

sub player_hasher{
	my @player_stats = @_;
        my %player_hash;
	my ($name, $race, $gender, $alignment, $class, $hp, $hpmax, $ulevel, $ac, $num_prayers, $num_wishes, $num_deaths, $num_moves, $dlevel, $msg) = @player_stats;
        $player_hash{ 'name' } = $name;
        $player_hash{ 'race' } = $race;
        $player_hash{ 'gender' } = $gender;
        $player_hash{ 'alignment' } = $alignment;
        $player_hash{ 'class' } = $class;
        $player_hash{ 'hp' } = $hp;
        $player_hash{ 'hpmax' } = $hpmax;
        $player_hash{ 'ulevel' } = $ulevel;
        $player_hash{ 'ac' } = $ac;
        $player_hash{ 'num_prayers' } = $num_prayers;
        $player_hash{ 'num_wishes' } = $num_wishes;
        $player_hash{ 'num_deaths' } = $num_deaths;
        $player_hash{ 'num_moves' } = $num_moves;
        $player_hash{ 'dlevel' } = $dlevel;
        $player_hash{ 'msg' } = $msg;
	return \%player_hash;
}

sub play_text{
	my $text = shift;
	print "Saying: $text";
	`echo "$text" | festival --tts`;
}


