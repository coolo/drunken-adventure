#! /usr/bin/perl

use Data::Dumper;
use strict;
use warnings;

my %buildings;

sub add_building {
    my ($nick, $name, $priority ) = @_;
    $buildings{$nick}->{name} = $name;
    $buildings{$nick}->{priority} = $priority;
}

sub set_number {
    my ($nick, $th, $num) = @_;

    for my $lvl ($th..10) {
	$buildings{$nick}->{numbers}->{$lvl} = $num;
    }
}

sub set_level {
    my ($nick, $level, $costs, $effort, $required) = @_;
    $buildings{$nick}->{effort}->{$level} = $effort;
    $buildings{$nick}->{costs}->{$level} = $costs;
    $buildings{$nick}->{required}->{$level} = $required;
    $buildings{$nick}->{maxlvl} = $level;
}

sub set_complete {
    my $nick = shift;
    my $num = 1;
    while (my $level = shift) {
	$buildings{$nick}->{complete}->{$num} = $level;
	$num++;
    }
}

set_number('Wizard_Tower', 5, 1);
set_number('Wizard_Tower', 6, 2);
set_number('Wizard_Tower', 8, 3);
set_number('Wizard_Tower', 9, 4);

set_level('Wizard_Tower', 1, "180,000 gold", '12h', 5);
set_level('Wizard_Tower', 2, "360,000 gold", '1d', 5);
set_level('Wizard_Tower', 3, "720,000 gold", '2d', 6);
set_level('Wizard_Tower', 4, "1,280,000 gold", '3d', 7);
set_level('Wizard_Tower', 5, "1,960,000 gold", '4d', 8);
set_level('Wizard_Tower', 6, "2,680,000 gold", '5d', 8);
set_level('Wizard_Tower', 7, "5,360,000 gold", '7d', 9);
set_level('Wizard_Tower', 8, "6,480,000 gold", '10d', 10);

set_number('Cannon', 1, 2);
set_number('Cannon', 5, 3);
set_number('Cannon', 7, 5);
set_number('Cannon', 10, 6);

set_level('Cannon', 2, "1,250 gold", "15min", 1);
set_level('Cannon', 3, "4,000 gold", "45min", 2);
set_level('Cannon', 4, "16,000 gold", "2h", 3);
set_level('Cannon', 5, "50,000 gold", "6h", 4);
set_level('Cannon', 6, "100,000 gold", "12h", 5);
set_level('Cannon', 7, "200,000 gold", "1d", 6);
set_level('Cannon', 8, "400,000 gold", "2d", 7);
set_level('Cannon', 9, "800,000 gold", "3d", 8);
set_level('Cannon', 10,	"1,600,000 gold", "4d", 8);
set_level('Cannon', 11,	"3,200,000 gold", "5d", 9);
set_level('Cannon', 12,	"6,400,000 gold", "6d", 10);
set_level('Cannon', 13,	"7,500,000 gold", "7d", 10);

set_number('Archer_Tower', 2, 1);
set_number('Archer_Tower', 4, 2);
set_number('Archer_Tower', 5, 3);
set_number('Archer_Tower', 7, 4);
set_number('Archer_Tower', 8, 5);
set_number('Archer_Tower', 9, 6);
set_number('Archer_Tower', 10, 7);
set_level('Archer_Tower', 2, '3,000 Gold', '30min', 2);
set_level('Archer_Tower', 3, '5,000 Gold', '45min', 3);
set_level('Archer_Tower', 4, '20,000 Gold', '4h', 4);
set_level('Archer_Tower', 5, '80,000 Gold', '12h', 5);
set_level('Archer_Tower', 6, '180,000 Gold', '1d', 5);
set_level('Archer_Tower', 7, '360,000 Gold', '2d', 6);
set_level('Archer_Tower', 8, '720,000 Gold', '3d', 7);
set_level('Archer_Tower', 9, '1,500,000 Gold', '4d', 8);
set_level('Archer_Tower', 10, '2,500,000 Gold', '5d', 8);
set_level('Archer_Tower', 11, '4,500,000 Gold', '6d', 9);
set_level('Archer_Tower', 12, '6,500,000 Gold', '7d', 10);
set_level('Archer_Tower', 13, '7,500,000 Gold', '8d', 10);

set_number('Mortar', 3, 1);
set_number('Mortar', 6, 2);
set_number('Mortar', 7, 3);
set_number('Mortar', 8, 4);
set_level('Mortar', 1, '8,000 Gold', '8h', 3);
set_level('Mortar', 2, '32,000 Gold', '12h', 4);
set_level('Mortar', 3, '120,000 Gold', '1d', 5);
set_level('Mortar', 4, '400,000 Gold', '2d', 6);
set_level('Mortar', 5, '800,000 Gold', '4d', 7);
set_level('Mortar', 6, '1,600,000 Gold', '5d', 8);
set_level('Mortar', 7, '3,200,000 Gold', '7d', 9);
set_level('Mortar', 8, '6,400,000 Gold', '10d', 10);

add_building('Air_Defense', 'Air Defense', 890);
set_number('Air_Defense', 4, 1);
set_number('Air_Defense', 7, 2);
set_number('Air_Defense', 8, 3);
set_number('Air_Defense', 9, 4);
set_level('Air_Defense', 1, '22,500 Gold', '5h', 4);
set_level('Air_Defense', 2, '90,000 Gold', '1d', 4);
set_level('Air_Defense', 3, '270,000 Gold', '3d', 5);
set_level('Air_Defense', 4, '540,000 Gold', '5d', 6);
set_level('Air_Defense', 5, '1,080,000 Gold', '6d', 7);
set_level('Air_Defense', 6, '2,160,000 Gold', '8d', 8);
set_level('Air_Defense', 7, '4,320,000 Gold', '10d', 9);
set_level('Air_Defense', 8, '7,560,000 Gold', '12d', 10);

add_building('Air_Sweeper', 'Air Sweeper', 890);
set_number('Air_Sweeper', 6, 1);

set_level('Air_Sweeper', 1, '500,000 Gold', '1d', 6);
set_level('Air_Sweeper', 2, '750,000 Gold', '3d', 6);
set_level('Air_Sweeper', 3, '1,250,000 Gold', '5d', 7);
set_level('Air_Sweeper', 4, '2,400,000 Gold', '7d', 8);
set_level('Air_Sweeper', 5, '4,800,000 Gold', '8d', 9);
set_level('Air_Sweeper', 6, '7,200,000 Gold', '9d', 10);

add_building('Hidden_Tesla', 'Hidden Tesla', 850);
set_number('Hidden_Tesla', 7, 2);
set_number('Hidden_Tesla', 8, 3);
set_number('Hidden_Tesla', 9, 4);
set_level('Hidden_Tesla', 1, '1,000,000 Gold', '1d', 7);
set_level('Hidden_Tesla', 2, '1,250,000 Gold', '3d', 7);
set_level('Hidden_Tesla', 3, '1,500,000 Gold', '5d', 7);
set_level('Hidden_Tesla', 4, '2,000,000 Gold', '6d', 8);
set_level('Hidden_Tesla', 5, '2,500,000 Gold', '8d', 8);
set_level('Hidden_Tesla', 6, '3,000,000 Gold', '10d', 8);
set_level('Hidden_Tesla', 7, '3,500,000 Gold', '12d', 9);
set_level('Hidden_Tesla', 8, '5,000,000 Gold', '14d', 10);

set_number('Spell_Factory', 5, 1);
set_level('Spell_Factory', 1, '200,000 Elixir', '1d', 5);
set_level('Spell_Factory', 2, '400,000 Elixir', '2d', 6);
set_level('Spell_Factory', 3, '800,000 Elixir', '4d', 7);
set_level('Spell_Factory', 4, '1,600,000 Elixir', '5d', 9);
set_level('Spell_Factory', 5, '3,200,000 Elixir', '6d', 10);

add_building('X_Bow', 'X-Bow', 850);
set_number('X_Bow', 9, 2);
set_number('X_Bow', 10, 3);
set_level('X_Bow', 1, '3,000,000 Gold', '7d', 9);
set_level('X_Bow', 2, '5,000,000 Gold', '10d', 9);
set_level('X_Bow', 3, '7,000,000 Gold', '14d', 9);
set_level('X_Bow', 4, '8,000,000 Gold', '14d', 10);

add_building('Inferno_Tower', 'Inferno Tower', 900);
set_number('Inferno_Tower', 10, 2);
set_level('Inferno_Tower', 1, '5,000,000 Gold', '7d', 10);
set_level('Inferno_Tower', 2, '6,500,000 Gold', '10d', 10);
set_level('Inferno_Tower', 3, '8,000,000 Gold', '14d', 10);

set_number('Bomb', 3, 2);
set_number('Bomb', 5, 4);
set_number('Bomb', 7, 6);

set_level('Bomb', 2, '1,400 Gold', '15min', 3);
set_level('Bomb', 3, '10,000 Gold', '2h', 5);
set_level('Bomb', 4, '100,000 Gold', '8h', 7);
set_level('Bomb', 5, '1,000,000 Gold', '1d', 8);
set_level('Bomb', 6, '1,500,000 Gold', '2d', 9);

set_number('Spring_Trap', 4, 2);
set_number('Spring_Trap', 6, 4);
set_number('Spring_Trap', 8, 6);

set_number('Giant_Bomb', 6, 1);
set_number('Giant_Bomb', 7, 2);
set_number('Giant_Bomb', 8, 3);
set_number('Giant_Bomb', 9, 4);
set_number('Giant_Bomb', 10, 5);

set_level('Giant_Bomb', 2, '27,500 Gold', '6h', 6);
set_level('Giant_Bomb', 3, '17,500 Gold', '1d', 8);
set_level('Giant_Bomb', 4, '20,000 Gold', '3d', 10);

set_number('Gold_Mine', 1, 1);
set_number('Gold_Mine', 2, 2);
set_number('Gold_Mine', 3, 3);
set_number('Gold_Mine', 4, 4);
set_number('Gold_Mine', 5, 5);
set_number('Gold_Mine', 6, 6);
set_number('Gold_Mine', 10, 7);

set_level('Gold_Mine', 2, '450 Elixir', '15min', 1);
set_level('Gold_Mine', 3, '700 Elixir', '15min', 2);
set_level('Gold_Mine', 4, '1,400 Elixir', '1h', 2);
set_level('Gold_Mine', 5, '3,000 Elixir', '2h', 3);
set_level('Gold_Mine', 6, '7,000 Elixir', '6h', 3);
set_level('Gold_Mine', 7, '14,000 Elixir', '12h', 4);
set_level('Gold_Mine', 8, '28,000 Elixir', '1d', 4);
set_level('Gold_Mine', 9, '56,000 Elixir', '2d', 5);
set_level('Gold_Mine', 10, '84,000 Elixir', '3d', 5);
set_level('Gold_Mine', 11, '168,000 Elixir', '4d', 7);

set_number('Gold_Storage', 1, 1);
set_number('Gold_Storage', 3, 2);
set_number('Gold_Storage', 8, 3);
set_number('Gold_Storage', 9, 4);
set_level('Gold_Storage', 2, '1050 Elixir', '30min', 2);
set_level('Gold_Storage', 3, '1,500 Elixir', '1h', 2);
set_level('Gold_Storage', 4, '3,000 Elixir', '2h', 3);
set_level('Gold_Storage', 5, '6,000 Elixir', '3h', 3);
set_level('Gold_Storage', 6, '12,000 Elixir', '4h', 3);
set_level('Gold_Storage', 7, '25,000 Elixir', '6h', 4);
set_level('Gold_Storage', 8, '50,000 Elixir', '8h', 4);
set_level('Gold_Storage', 9, '100,000 Elixir', '12h', 5);
set_level('Gold_Storage', 10, '250,000 Elixir', '1d', 6);

set_number('Elixir_Storage', 1, 1);
set_number('Elixir_Storage', 3, 2);
set_number('Elixir_Storage', 8, 3);
set_number('Elixir_Storage', 9, 4);
set_level('Elixir_Storage', 2, '1050 Gold', '30min', 2);
set_level('Elixir_Storage', 3, '1,500 Gold', '1h', 2);
set_level('Elixir_Storage', 4, '3,000 Gold', '2h', 3);
set_level('Elixir_Storage', 5, '6,000 Gold', '3h', 3);
set_level('Elixir_Storage', 6, '12,000 Gold', '4h', 3);
set_level('Elixir_Storage', 7, '25,000 Gold', '6h', 4);
set_level('Elixir_Storage', 8, '50,000 Gold', '8h', 4);
set_level('Elixir_Storage', 9, '100,000 Gold', '12h', 5);
set_level('Elixir_Storage', 10, '250,000 Gold', '1d', 6);

set_number('Dark_Elixir_Storage', 7, 1);
set_level('Dark_Elixir_Storage', 1, '600,000 Elixir', '1d', 7);
set_level('Dark_Elixir_Storage', 2, '1,200,000 Elixir', '2d', 7);
set_level('Dark_Elixir_Storage', 3, '1,800,000 Elixir', '3d', 8);
set_level('Dark_Elixir_Storage', 4, '2,400,000 Elixir', '4d', 8);
set_level('Dark_Elixir_Storage', 5, '3,000,000 Elixir', '5d', 9);

set_number('Army_Camp', 1, 1);
set_number('Army_Camp', 3, 2);
set_number('Army_Camp', 5, 3);
set_number('Army_Camp', 7, 4);
set_level('Army_Camp', 1, '250 Elixir', '15min', 1);
set_level('Army_Camp', 2, '2,500 Elixir', '1h', 2);
set_level('Army_Camp', 3, '10,000 Elixir', '3h', 3);
set_level('Army_Camp', 4, '100,000 Elixir', '8h', 4);
set_level('Army_Camp', 5, '250,000 Elixir', '1d', 5);
set_level('Army_Camp', 6, '750,000 Elixir', '3d', 6);
set_level('Army_Camp', 7, '2,250,000 Elixir', '5d', 9);

set_number('Barracks', 1, 1);
set_number('Barracks', 2, 2);
set_number('Barracks', 4, 3);
set_number('Barracks', 7, 4);
set_level('Barracks', 2, '1,200 Elixir', '15min', 1);
set_level('Barracks', 3, '2,500 Elixir', '2h', 1);
set_level('Barracks', 4, '5,000 Elixir', '4h', 2);
set_level('Barracks', 5, '10,000 Elixir', '10h', 3);
set_level('Barracks', 6, '80,000 Elixir', '16h', 4);
set_level('Barracks', 7, '240,000 Elixir', '1d', 5);
set_level('Barracks', 8, '700,000 Elixir', '2d', 6);
set_level('Barracks', 9, '1,500,000 Elixir', '4d', 7);

set_number('Dark_Barracks', 7, 1);
set_number('Dark_Barracks', 8, 2);
set_level('Dark_Barracks', 1, '750,000 Elixir', '3d', 7);
set_level('Dark_Barracks', 2, '1,250,000 Elixir', '5d', 7);
set_level('Dark_Barracks', 3, '1,750,000 Elixir', '6d', 8);
set_level('Dark_Barracks', 4, '2,250,000 Elixir', '7d', 8);
set_level('Dark_Barracks', 5, '2,750,000 Elixir', '8d', 9);

set_number('Laboratory', 3, 1);
set_level('Laboratory', 1, '25,000 Elixir', '30min', 3);
set_level('Laboratory', 2, '50,000 Elixir', '5h', 4);
set_level('Laboratory', 3, '90,000 Elixir', '12h', 5);
set_level('Laboratory', 4, '270,000 Elixir', '1d', 6);
set_level('Laboratory', 5, '500,000 Elixir', '2d', 7);
set_level('Laboratory', 6, '1,000,000 Elixir', '4d', 8);
set_level('Laboratory', 7, '2,500,000 Elixir', '5d', 9);
set_level('Laboratory', 8, '4,000,000 Elixir', '6d', 10);

set_number('Dark_Elixir_Drill', 7, 1);
set_number('Dark_Elixir_Drill', 8, 2);
set_number('Dark_Elixir_Drill', 10, 3);
set_level('Dark_Elixir_Drill', 1, '1,000,000 Elixir', '1d', 7);
set_level('Dark_Elixir_Drill', 2, '1,500,000 Elixir', '2d', 7);
set_level('Dark_Elixir_Drill', 3, '2,000,000 Elixir', '3d', 7);
set_level('Dark_Elixir_Drill', 4, '3,000,000 Elixir', '4d', 9);
set_level('Dark_Elixir_Drill', 5, '4,000,000 Elixir', '6d', 9);

set_number('Elixir_Collector', 1, 1);
set_number('Elixir_Collector', 2, 2);
set_number('Elixir_Collector', 3, 3);
set_number('Elixir_Collector', 4, 4);
set_number('Elixir_Collector', 5, 5);
set_number('Elixir_Collector', 6, 6);
set_number('Elixir_Collector', 10, 7);
set_level('Elixir_Collector', 2, '450 Gold', '15min', 1);
set_level('Elixir_Collector', 3, '700 Gold', '15min', 2);
set_level('Elixir_Collector', 4, '1,400 Gold', '1h', 2);
set_level('Elixir_Collector', 5, '3,500 Gold', '2h', 3);
set_level('Elixir_Collector', 6, '7,000 Gold', '6h', 3);
set_level('Elixir_Collector', 7, '14,000 Gold', '12h', 4);
set_level('Elixir_Collector', 8, '28,000 Gold', '1d', 4);
set_level('Elixir_Collector', 9, '56,000 Gold', '2d', 5);
set_level('Elixir_Collector', 10, '84,000 Gold', '3d', 5);
set_level('Elixir_Collector', 11, '168,000 Gold', '4d', 7);

set_number('Skeleton_Trap', 8, 2);
set_number('Skeleton_Trap', 10, 3);
set_level('Skeleton_Trap', 1, '6,000 Gold', '15min', 8);
set_level('Skeleton_Trap', 2, '600,000 Gold', '6h', 8);
set_level('Skeleton_Trap', 3, '1,300,000 Gold', '1d', 9);

set_number('Seeking_Air_Mine', 7, 1);
set_number('Seeking_Air_Mine', 8, 2);
set_number('Seeking_Air_Mine', 9, 4);
set_number('Seeking_Air_Mine', 10, 5);
set_level('Seeking_Air_Mine', 1, '15,000 Gold', '15min', 7);
set_level('Seeking_Air_Mine', 2, '2,000,000 Gold', '1d', 9);
set_level('Seeking_Air_Mine', 3, '4,000,000 Gold', '3d', 10);

set_number('Air_Bomb', 5, 2);
set_number('Air_Bomb', 8, 4);
set_number('Air_Bomb', 10, 5);
set_level('Air_Bomb', 2, '24,000 Gold', '4h', 5);
set_level('Air_Bomb', 3, '200,000 Gold', '12h', 7);
set_level('Air_Bomb', 4, '1,500,000 Gold', '1d', 9);

set_number('Clan_Castle', 3, 1);
set_level('Clan_Castle', 2, '325,000 Gold and Elixir', '6h', 4);
set_level('Clan_Castle', 3, '700,000 Gold and Elixir', '1d', 6);
set_level('Clan_Castle', 4, '1,000,000 Gold and Elixir', '2d', 8);
set_level('Clan_Castle', 5, '1,500,000 Gold and Elixir', '7d', 9);
set_level('Clan_Castle', 6, '2,000,000 Gold and Elixir', '14d', 10);

set_number('th', 1, 1);
set_level('th', 2, '1,000 Gold', '15min', 1);
set_level('th', 3, '4,000 Gold', '3h', 2);
set_level('th', 4, '25,000 Gold', '1d', 3);
set_level('th', 5, '150,000 Gold', '2d', 4);
set_level('th', 6, '750,000 Gold', '4d', 5);
set_level('th', 7, '1,200,000 Gold', '6d', 6);
set_level('th', 8, '2,000,000 Gold', '8d', 7);
set_level('th', 9, '3,000,000 Gold', '10d', 8);
set_level('th', 10, '4,000,000 Gold', '14d', 9);

# Base
add_building('th', 'Town Hall', 300);
set_complete('th', 6);

add_building('Clan_Castle', 'Clan Castle', 800);
set_complete('Clan_Castle', 3);

# Defenses
add_building('Wizard_Tower', 'Wizard Tower', 630);
set_complete('Wizard_Tower', 2, 3);
add_building('Cannon', 'Cannon', 550);
set_complete('Cannon', 5, 7, 7);
add_building('Archer_Tower', 'Archer Tower', 660);
set_complete('Archer_Tower', 7, 7, 7);
add_building('Mortar', 'Mortar', 600);
set_complete('Mortar', 3, 4);
add_building('Air_Defense', 'Air Defense', 690);
set_complete('Air_Defense', 4);
add_building('Air_Sweeper', 'Air Sweeper', 690);
set_complete('Air_Sweeper', 2);
add_building('Hidden_Tesla', 'Hidden Tesla', 650);
set_complete('Hidden_Tesla');
add_building('X_Bow', 'X-Bow', 650);
set_complete('X_Bow');
add_building('Inferno_Tower', 'Inferno Tower', 700);
set_complete('Inferno_Tower');
add_building('Air_Bomb', 'Air_Bomb', 650);
set_complete('Air_Bomb', 2, 2);
add_building('Giant_Bomb', 'Giant Bomb', 620);
set_complete('Giant_Bomb', 2);
add_building('Seeking_Air_Mine', 'Seeking Air Mine', 600);
set_complete('Seeking_Air_Mine');
add_building('Skeleton_Trap', 'Skeleton Trap', 600);
set_complete('Skeleton_Trap');
add_building('Spring_Trap', 'Spring Trap', 600);
set_complete('Spring_Trap', 1, 1, 1, 1);
add_building('Bomb', 'Bomb', 910);
set_complete('Bomb', 3, 3, 3, 3);

# Offense
add_building('Army_Camp', 'Army Camp', 830);
set_complete('Army_Camp', 6, 6, 6);
add_building('Barracks', 'Barracks', 800);
set_complete('Barracks', 8, 8, 8);
add_building('Dark_Barracks', 'Dark Barracks', 800);
set_complete('Dark_Barracks');
add_building('Laboratory', 'Laboratory', 950);
set_complete('Laboratory', 4);
add_building('Spell_Factory', 'Spell Factory', 750);
set_complete('Spell_Factory', 2);

# Resources
add_building('Elixir_Collector', 'Elixir Collector', 250);
set_complete('Elixir_Collector', 6, 5, 6, 6, 9, 8);
add_building('Elixir_Storage', 'Elixir Storage', 500);
set_complete('Elixir_Storage', 9, 10);
add_building('Gold_Mine', 'Gold Mine', 240);
set_complete('Gold_Mine', 4, 6, 6, 7, 8, 10);
add_building('Gold_Storage', 'Gold Storage', 500);
set_complete('Gold_Storage', 10, 10);
add_building('Dark_Elixir_Drill', 'Dark Elixir Drill', 500);
set_complete('Dark_Elixir_Drill');
add_building('Dark_Elixir_Storage', 'Dark Elixir Storage', 500);
set_complete('Dark_Elixir_Storage');

#print Dumper(\%buildings);

open(TASKS, ">tasks.tji");
my %tasks;

for my $nick (sort keys %buildings) {
    for my $num (1..$buildings{$nick}->{numbers}->{10}) {
	my $firstlvl = 1;
	$firstlvl++ if ( !$buildings{$nick}->{effort}->{$firstlvl} );

	$buildings{$nick}->{name} ||= $nick;
	$buildings{$nick}->{priority} //= 500;
	$buildings{$nick}->{maxlvl} //= 1;

	for my $lvl ($firstlvl..$buildings{$nick}->{maxlvl}) {
	    my $taskname = "$nick\_$num\_$lvl";
	    my $descr;
	    my @deps;
	    my $costs = $buildings{$nick}->{costs}->{$lvl};
	    if ($lvl == $firstlvl) {
		$descr = sprintf "Build %s $num as Level $lvl ($costs)", $buildings{$nick}->{name};
	    } else {
		$descr = sprintf "Upgrade %s $num to Level $lvl ($costs)", $buildings{$nick}->{name};
		if ($nick ne 'th') {
		    push(@deps, "$nick\_$num\_" . ($lvl - 1));
		}
	    }
	    print TASKS "task $taskname \"$descr\" {\n";
	    $tasks{$taskname}->{descr} = $descr;
	    if (($buildings{$nick}->{complete}->{$num} // 0) < $lvl) {
		printf TASKS "  \${builder_effort \"%s\"}\n", $buildings{$nick}->{effort}->{$lvl};
		$tasks{$taskname}->{effort} = $buildings{$nick}->{effort}->{$lvl};
	    }
	    my $thlvl = $buildings{$nick}->{required}->{$lvl};
	    while ($num > $buildings{$nick}->{numbers}->{$thlvl}) {
		$thlvl++;
	    }
	    push(@deps, "th_1_$thlvl") if ($thlvl > 1);
	    $tasks{$taskname}->{priority} = $buildings{$nick}->{priority} // 500;
	    print TASKS "  priority " . ($buildings{$nick}->{priority} // 500) . "\n";
	    print TASKS "  depends " . join(', ', map { "!$_" } @deps) . "\n" if (@deps);
	    $tasks{$taskname}->{deps} = \@deps;
	    print TASKS "}\n\n";
	}
    }
}

#print Dumper(\%tasks);
