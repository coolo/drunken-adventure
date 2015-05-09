#! /usr/bin/perl

use strict;
use warnings;

my $items = { animals =>
	      {
	       'cheer team panda red' => [ 31, 57879, 42027 ],
	       'cheer team panda purple' => [ 31, 57879, 42027 ],
	       'archmage giraffe' => [ 29, 51409, 21669 ],
	       'mint chocolate panda' => [ 28, 28029, 43309 ],
	       'spring leaf rabbit' => [28, 10000, 30800 ], # lvl 1
	       'osaka style queen' => [ 27, 14988, 32282 ], # lvl 29
	       'sumo elephant' => [ 27, 42581, 17782 ],
	       'gold boss monkey' => [ 27, 60689, 30537 ],
	       'black bear rabbit' => [ 26, 28898, 31466 ],
	       'cowboy giraffe' => [ 26, 40507, 18658 ],
	       'rose quartz elephant' => [ 26, 30547, 29648 ],
	       'new shoot monkey' => [ 25, 30988, 26912 ],
	       'neon turle crocodile' => [ 25, 25428, 9904 ], # lvl 9
	       'silver sun wukong monkey' => [ 15, 32302, 17974 ],
	       'elephant (wings)' => [ 12, 3980, 2020 ], # lvl1
	       'elephant' => [ 4, 6444, 5522 ], # lvl29
	      },
	      backgrounds =>
	      {
	       'ultimate zoo wrestling' => [ 31, 68308, 22532 ],
	       'mysterious magical library' => [ 31, 65766, 30282 ],
	       'zoo stardom stage' => [31, 7000, 45500], # lvl 1
	       'dandelion field' => [ 31, 9700, 21800 ], # lvl 1
	       'nightlife station' => [ 27, 36572, 9300 ], # lvl 16
	       'wild west town' => [ 25, 22200, 7800 ], # lvl 1
	       'hot air ballon carnival' => [ 24, 35877, 22495 ],
	       'zoo cherry blossom illumination' => [ 24, 31769, 20233 ],
	       'thunderstorm painting' => [ 24, 41517, 9658 ], # lvl 47
	       'rainbow & house' => [ 7, 1780, 3020], # lvl 1
	       'night on the prarie' => [ 2, 1220, 680 ], # lvl 1
	       'sunny wastelands' => [ 1, 1090, 810 ], # lvl 1
	       'dusk in the sea' => [ 1, 620, 1280 ], # lvl 1
	      },
	      decorations =>
	      {
	       'gemston globe' => [ 30, 30800, 15700 ], # lvl 1
	       'ultra gold combat mech' => [ 28, 58148, 17269 ], # lvl 49
	       'lucky golden owl' => [ 28, 23800, 17280 ], # lvl 1
	       'cherry blossom score attack king cup' => [ 28, 25175, 46205 ],
	       'golden week king cup' => [ 28, 9200, 31800 ], # lvl 1
	       'gem pinapple' => [28, 50651, 17276 ],
	       'producer boss' => [28, 32000, 9000], # lvl 1
	       'gold shachihoko' => [ 27, 44347, 15022 ],
	       'golden rice bales' => [ 27, 30545, 28072] ,
	       'sleeping beauty bed' => [ 27, 11300, 21200 ], # lvl 1
	       'first day of school boss' => [ 27, 14856, 22664 ], # lvl 9
	       'zoo florar ballon' => [27, 11650, 19850], # lvl1
	       'dragon figurine' => [ 25, 53449, 6226 ],
	       'golden gourd' => [ 24, 20800, 8200], # lvl 1
	       'bell of destiny' => [ 24, 22800, 7200 ], # lvl 1
	       'darts machine' => [24, 42465, 11497 ],
	       'classic zoo lunch' => [ 24, 6800, 23200], #lvl 1,
	       'supersonic jet' => [ 24, 20100, 10400], # lvl 1
	       'dragon stone' => [ 23, 24200, 5300 ], # lvl 1
	       'boss gold statue' => [ 14, 6180, 1820 ], # lvl 1
	      }
	  };

my @costs = ( 98, 92, 82, 77, 69, 57 );

# 819127 + 392206 = 1424793.5

use AI::Genetic;

my $ga = new AI::Genetic(
    -fitness    => \&fitnessFunc,
    -type       => 'listvector',
    -population => 3000,
    -crossover  => 0.8,
    -mutation   => 0.2,
    -terminate  => \&terminateFunc,
  );

my $slots = 6;

my @initvars;
for (1..$slots) {
    push(@initvars, [ '', keys %{$items->{animals}} ]);
    push(@initvars, [ '', keys %{$items->{backgrounds}} ]);
    push(@initvars, [ '', keys %{$items->{decorations}} ]);
}

our $maximum = 0;
our $gener = 0;

$ga->init(\@initvars);

my $round;

$ga->evolve('tournamentTwoPoint', 2000);
print "Best score = ", $ga->getFittest->score, ".\n";
#print Dumper($ga->getFittest->genes());

my @genes = @{$ga->getFittest->genes};
my %good;

my $cool = 0;
my $cute = 0;
for my $r (1..$slots) {
   my $animal = shift @genes;
   push(@{$good{a}}, $animal);

   print "A: $animal\n";
   $animal = $items->{animals}->{$animal} || [0, 0, 0];
   my $background = shift @genes;
   push(@{$good{b}}, $background);
   print "B: $background\n";
   $background = $items->{backgrounds}->{$background} || [0, 0, 0];
   my $decoration = shift @genes;
   push(@{$good{d}}, $decoration);
   print "D: $decoration\n";
   $decoration = $items->{decorations}->{$decoration} || [0, 0, 0];
   $cool += ($animal->[1] + $background->[1] + $decoration->[1]);
   $cute += ($animal->[2] + $background->[2] + $decoration->[2]);
   print "\n";
}
print "Cool: $cool, Cute: $cute\n";

$ga = new AI::Genetic(
		      -population => 1000,
		      -fitness    => \&fitnessFunc,
		      -type       => 'listvector',
		      -terminate  => \&terminateFunc,
		     );

@initvars = ();
for (1..$slots) {
    push(@initvars, $good{a} );
    push(@initvars, $good{b} );
    push(@initvars, $good{d} );
}

#$ga->init(\@initvars);

#$round = 2;

#$ga->evolve('tournamentTwoPoint', 1000);

exit(0);

use Data::Dumper;
sub fitnessFunc {
    my @genes = @{shift @_};

    my $fitness = 0;

    #print "FI " . Dumper(\@genes), "\n" if $round;
    my %used;

    for my $r (1..$slots) {
	my $animal = shift @genes;

	if ($animal) {
	    $animal = $items->{animals}->{$animal};
	    return 0 if defined $used{$animal};
	    $used{$animal} = 1;
	} else {
	    $animal = [0, 0, 0];
	}
	my $background = shift @genes;
	if ($background) {
	    $background = $items->{backgrounds}->{$background};
	    return 0 if defined $used{$background};
	    $used{$background} = 1;
	} else {
	    $background = [0, 0, 0];
	}
	my $decoration = shift @genes;
	if ($decoration) {
	    $decoration = $items->{decorations}->{$decoration};
	    return 0 if defined $used{$decoration};
	    $used{$decoration} = 1;
	} else {
	    $decoration = [0, 0, 0];
	}
	#print Dumper($animal), " b ", Dumper($background), " d ", Dumper($decoration);
	if ($animal->[0] + $background->[0] + $decoration->[0] > $costs[$r-1]) {
	    return 0;
	}
	$fitness += ($animal->[1] + $background->[1] + $decoration->[1]) * 1.5;
	$fitness += ($animal->[2] + $background->[2] + $decoration->[2]) / 2;
    }

    print "FI res $fitness $maximum\n" if $round;
    return 0 if ($round && $fitness < $maximum);

    return $fitness;
}

sub terminateFunc {
    my $ga = shift;

    if ($ga->getFittest->score > $maximum) {
       print $ga->getFittest->score, " '", join("', '", $ga->getFittest->genes), "\n";
       $maximum = $ga->getFittest->score ;
       $gener = $ga->generation;
       return 0;
    }
    # terminate if reached some threshold.
    return 1 if $ga->generation - $gener > 150;
    return 0;
}

pick('animals', 1);
