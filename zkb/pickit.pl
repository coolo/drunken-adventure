#! /usr/bin/perl

use strict;
use warnings;

my $items = { animals =>
	      {
		  'spring leaf rabbit' => [28, 10000, 30800 ], # lvl 1
		  'sumo elephant' => [ 27, 42581, 17782 ],
		  'cowboy giraffe' => [ 26, 40507, 18658 ],
		  'black bear rabbit' => [ 26, 28898, 31466 ],
		  'osaka style queen' => [ 27, 14988, 32282 ],
		  'elephant (wings)' => [ 12, 3980, 2020 ], # lvl1
		  'boss monkey' => [ 21, 36673, 14330 ],
		  'archmage giraffe' => [ 29, 33328, 14248 ], # lvl9
		  'mint chocolate panda' => [ 28, 28029, 43309 ],
		  'new shoot monkey' => [ 25, 30988, 26912 ],
		  'rose quartz elephant' => [ 26, 30547, 29648 ],
		  'silver sun wukong monkey' => [ 15, 22960, 11898 ], # lvl 18
		  'elephant' => [ 4, 6444, 5522 ], # lvl29
	      },
	      backgrounds =>
	      {
		  'sunny wastelands' => [ 1, 620, 1280 ], # lvl 1
		  'dusk in the sea' => [ 2, 1220, 680 ], # lvl 1
		  'night on the prarie' => [ 1, 1090, 810], # lvl 1
		  'rainbow & house' => [ 7, 1780, 3020], # lvl 1
		  'hot air ballon carnival' => [ 24, 35877, 22495 ],
		  'zoo cherry blossom illumination' => [ 24, 31769, 20233 ],
		  'wild west town' => [ 25, 22200, 7800 ], # lvl 1
		  'ultimate zoo wrestling' => [ 31, 68308, 22532 ],
		  'mysterious magical library' => [ 31, 65766, 30282 ],
		  'nightlife station' => [ 27, 32510, 8290 ] # lvl 1
	      },
	      decorations =>
	      {
		  'boss gold statue' => [ 14, 6180, 1820 ],
		  'gold shachihoko' => [ 27, 44347, 15022 ],
		  'golden rice bales' => [ 27, 30545, 28072] ,
		  'cherry blossom score attack king cup' => [ 28, 25175, 46205 ],
		  'first day of school boss' => [ 27, 14856, 22664 ], # lvl 9
		  'bell of destiny' => [ 24, 22800, 7200 ], # lvl 1
		  'dragon figurine' => [ 25, 53449, 6226 ],
		  'darts machine' => [24, 42465, 11497 ],
		  'gem pinapple' => [28, 31474, 12607], # lvl 8
		  'zoo florar ballon' => [27, 11650, 19850],
	      }
	  };

my @costs = ( 90, 84, 74, 69, 61 );

# 577265 + 239479 = 985637

use AI::Genetic;

my $maxtotal = 0;

my $zoo = { cute => 0, cool => 0, animals => {}, decorations => {}, backgrounds => {} };

for my $tag (qw/animals backgrounds decorations/) {
    for my $key (keys %{$items->{$tag}}) {
	$zoo->{$tag}->{$key} = 0;
    }
}

for my $i (0..5) {
    $zoo->{costs}->{$i+1} = $costs[$i];
}

my $ga = new AI::Genetic(
    -fitness    => \&fitnessFunc,
    -type       => 'listvector',
    -population => 2000,
    -crossover  => 0.9,
    -mutation   => 0.05,
    -terminate  => \&terminateFunc,
  );

my @initvars;
for (1..5) {
    push(@initvars, [ '', keys %{$items->{animals}} ]);
    push(@initvars, [ '', keys %{$items->{backgrounds}} ]);
    push(@initvars, [ '', keys %{$items->{decorations}} ]);
}

$ga->init(\@initvars);

$ga->evolve('rouletteTwoPoint', 100);
print "Best score = ", $ga->getFittest->score, ".\n";
print Dumper($ga->getFittest->genes());
exit(0);

use Data::Dumper;
sub fitnessFunc {
    my @genes = @{shift @_};

    my $fitness = 0;

    #print "FI " . Dumper(\@genes), "\n";
    my %used;

    for my $r (1..5) {
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

    #print "FI res $fitness\n";
    return $fitness;
}

sub terminateFunc {
    my $ga = shift;

    print $ga->getFittest->score, " '", join("', '", $ga->getFittest->genes), "\n";
    # terminate if reached some threshold.
    #return 1 if $ga->getFittest->score > 1985636;
    return 0;
}

pick('animals', 1);
