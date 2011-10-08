#!/usr/bin/perl
use strict;
use warnings;
use Env qw( FESTIVAL );

my $festival  = "$FESTIVAL" eq "" ? "festival" : "$FESTIVAL";
my $tts = "$festival --pipe";
my $board = "./rx";
my %players = ();

my $team_ant=0;
my $i = 0;
while (<STDIN>){
	my $line = $_;

	my @player_import = split(",", $line);
	chomp($player_import[-1]);
	my ($name, $race, $gender, $alignment, $class, $hp, $hpmax, $ulevel, $ac, $num_prayers, $num_wishes, $num_deaths, $num_moves, $dlevel, $msg) = @player_import;
	if($name =~ m/port34/){
		$name='port__';
		$player_import[0]=$name;
	}

	if(exists($players{ $name } )){
		# check to see if something important changed
		say_if_important(player_hasher(@player_import));
		$players{ $name } = player_hasher(@player_import);
	}
	else{
		my %player_stats = %{player_hasher(@player_import)};
		$players{ $name } = \%player_stats;
		my $long_class = class_expander($class,$gender);
		play_text("$name the $long_class has entered $msg!");
	}
}

sub say_if_important{
	my $ptr = shift;
	my %new_player_stats = %{$ptr};
	my $name = $new_player_stats{ 'name' };
	my %old_player_stats = %{$players{ $name }};
	my $old_gender = $old_player_stats{'gender'};
	my $new_gender = $new_player_stats{'gender'};
	my $long_old_gender = gender_possessive($old_gender);
	my $long_new_gender = gender_possessive($new_gender);
	my $old_gender_pronoun = gender_pronoun($old_gender);
	my $new_gender_pronoun = gender_pronoun($new_gender);
	my $old_msg = $old_player_stats{'msg'};
	my $new_msg = $new_player_stats{'msg'};
	my $new_long_class = class_expander($new_player_stats{'class'},$new_player_stats{'gender'});

	unless($old_gender eq $new_gender ){
		play_text("$name decided to bat for the other team. $old_gender_pronoun is now a $new_gender_pronoun.");
	}
	if( defined($new_player_stats{'num_prayers'}) && ( ! defined($old_player_stats{'num_prayers'}) || $old_player_stats{'num_prayers'} ne $new_player_stats{'num_prayers'} ) ){
		play_text("$name got down on $long_new_gender knees and prayed to $long_new_gender god.");
	}
	if( defined($new_player_stats{'num_wishes'}) && ( ! defined($old_player_stats{'num_wishes'} || $old_player_stats{'num_wishes'} ge $new_player_stats{'num_wishes'}) ) ){
		play_text("$name pretends that airplanes in the night sky are like shooting stars. $new_gender_pronoun made a wish!");
	}
	if( defined($new_player_stats{'ulevel'}) && ( !defined($old_player_stats{'ulevel'}) || ($old_player_stats{'ulevel'} ne $new_player_stats{'ulevel'}) ) and $new_player_stats{'ulevel'} > 5 ){
		if( !defined($old_player_stats{'ulevel'}) || $old_player_stats{ 'ulevel' } gt $new_player_stats{ 'ulevel' }){
			play_text("Poor $name. $new_gender_pronoun just lost a level!");
		} elsif($new_player_stats{'ulevel'} > 5){ #First 5 are basically just audio spam and not worthy of mention.
			play_text("Congratulations to $name $new_gender_pronoun just hit level $new_player_stats{'ulevel'}.");
		}
	}
	if( defined($new_msg) && ( !defined($old_msg) || $old_msg ne $new_msg ) ){
		if($new_msg =~ m/quit/){
			play_text("$name the $new_long_class rage-quit.");
		}
		elsif($new_msg =~ m/escaped/){
			play_text("$name the $new_long_class escaped the dungeon!");
		}
		elsif($new_msg =~ m/Sokoban/){
			play_text("$name the $new_long_class has entered $new_msg");
		}
		elsif($new_msg =~ m/^killed/){
			if($new_msg =~ m/ ant /i ){
				$team_ant++;
				play_text("Go team ant! This has been kill number $team_ant for team ant.");
			}
			if($new_msg =~ m/ water moccasin/i){
				play_text("An endless stream of snakes pours forth!");
			}
			play_text("$name the $new_long_class was $new_msg");
		}
		else{
			play_text("$name the $new_long_class has entered the $new_msg");
		}
	} else {
		if( defined($new_player_stats{'num_deaths'}) && ( !defined($old_player_stats{'num_deaths'}) || $old_player_stats{'num_deaths'} lt $new_player_stats{'num_deaths'} ) ) {
			play_text("$name has died.");
		}
	}
}

sub gender_pronoun{
	if(shift =~ m/^m/){
		return "he";
	}
	else{
		return "she";
	}
}

sub gender_possessive{
	return gender_pronoun(shift) eq "he" ? "his" : "her";
}

sub class_expander{
	my $class = shift;
	$class =~ tr/[A-Z]/[a-z]/;
	my $gender = shift;
	if($class =~ m/^arc/){
		return "archeologist";
	}
	if($class =~ m/^bar/){
		return "barbarian";
	}
	if($class =~ m/^cav/){
		if($gender eq "m"){
			return "caveman";
		}
		if($gender eq "f"){
			return "cavewoman";
		}
	}
	if($class =~ m/^hea/){
		return "healer";
	}
	if($class =~ m/^kni/){
		return "knight";
	}
	if($class =~ m/^mon/){
		return "monk";
	}
	if($class =~ m/^pri/){
		if($gender eq "m"){
			return "priest";
		}
		if($gender eq "f"){
			return "priestess";
		}
	}
	if($class =~ m/^ran/){
		return "ranger";
	}
	if($class =~ m/^rog/){
		return "rogue";
	}
	if($class =~ m/^sam/){
		return "samurai";
	}
	if($class =~ m/^tou/){
		return "tourist";
	}
	if($class =~ m/^val/){
		return "valkyrie";
	}
	if($class =~ m/^wiz/){
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
	print "Saying: $text\n";
	`echo "$text" | "$festival" --tts`;
}
