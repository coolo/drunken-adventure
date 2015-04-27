#! /usr/bin/perl

use strict;
use warnings;

my $items = { animals => { 'sumo elephant' => [ 27, 42581, 17782 ],
			   'cowboy giraffe' => [ 26, 40507, 18658 ],
			   'black bear rabbit' => [ 26, 28898, 31466 ],
			   'osaka style queen' => [ 27, 14988, 32282 ],
			   'elephant (wings)' => [ 12, 3980, 2020 ], # lvl1
			   'boss monkey' => [ 21, 36673, 14330 ],
			   'archmage giraffe' => [ 29, 33328, 14248 ], # lvl9
			   'mint chocolate panda' => [ 28, 28029, 43309 ],
			   'new shoot monkey' => [ 25, 30988, 26912 ],
			   'rose quartz elephant' => [ 26, 30547, 29648 ],
			   'bronze sun wukong monkey' => [ 11, 19140, 9812 ],
			   'elephant' => [ 4, 6444, 5522 ], # lvl29
		       },
	      backgrounds => { 'sunny wastelands' => [ 1, 620, 1280 ], # lvl 1
			       'dusk in the sea' => [ 2, 1220, 680 ], # lvl 1
			       'night on the prarie' => [ 1, 1090, 810], # lvl 1
			       'rainbow & house' => [ 7, 1780, 3020], # lvl 1
			       'zoo warring states period' => [6, 3920, 880 ], # lvl 1
			       'hot air ballon carnival' => [ 24, 35877, 22495 ],
			       'zoo cherry blossom illumination' => [ 24, 31769, 20233 ],
			       'wild west town' => [ 25, 22200, 7800 ], # lvl 1
			       'ultimate zoo wrestling' => [ 31, 68308, 22532 ],
			       'mysterious magical library' => [ 31, 65766, 30282 ],
			       'nightlife station' => [ 27, 32510, 8290 ] # lvl 1
			   },
	      decorations => { 'boss gold statue' => [ 14, 6180, 1820 ],
			       'gold shachihoko' => [ 27, 44347, 15022 ],
			       'golden rice bales' => [ 27, 30545, 28072] ,
			       'cherry blossom score attack king cup' => [ 28, 25175, 46205 ],
			       'first day of school boss' => [ 27, 14856, 22664 ], # lvl 9
			       'bell of destiny' => [ 24, 22800, 7200 ], # lvl 1
			       'dragon figurine' => [ 25, 53449, 6226 ],
			       'darts machine' => [24, 42465, 11497 ]
			   }
	      };

my @costs = ( 90, 83, 73, 68, 61 );

# 577265 + 239479 = 985637

my $maxtotal = 0;

#my $total = $cool * 1.5 + $cute / 2;
#	push(@lines, "$turn: '$randomanimal', '$randombackground', '$randomdecoration' $cost: $cool $cute - $total($maxtotal)\n");
# if ($total > $maxtotal) {
#	    $maxtotal = $total;
#	    print join('', @lines), "\n";
#	}
#
#    }
#}

#while (1) { pick_random(); }

my $zoo = { cute => 0, cool => 0, animals => {}, decorations => {}, backgrounds => {} };

for my $tag (qw/animals backgrounds decorations/) {
    for my $key (keys %{$items->{$tag}}) {
	$zoo->{$tag}->{$key} = 0;
    }
}

for my $i (0..5) {
    $zoo->{costs}->{$i+1} = $costs[$i];
}

my $na = scalar keys %{$items->{animals}};
my $nd = scalar keys %{$items->{decorations}};
my $nb = scalar keys %{$items->{backgrounds}};
my $combinations = $na * ($na - 1) * ($na - 2) * $nd * ($nd - 1) * ($nd - 2) * $nb * ($nb - 1) * ($nb - 2);


sub pick {
    my ($tag, $level) = @_;
    if ($level == 6) {
	if ($tag eq 'animals') {
	    return pick('backgrounds', 1);
	}
	if ($tag eq 'backgrounds') {
	    return pick('decorations', 1);
	}
	my $total = int($zoo->{cool} * 1.5 + $zoo->{cute} / 2);
	if ($total > $maxtotal) {
	    $maxtotal = $total;
	}
	print "COOL $zoo->{cool} CUTE $zoo->{cute} TOTAL $total($maxtotal) - $combinations\n";
	return;
    }
    for my $key (keys %{$zoo->{$tag}}) {
	next if $zoo->{$tag}->{$key};

	$combinations--;
	my ($cost, $cool, $cute) = @{$items->{$tag}->{$key}};
	next if ($zoo->{costs}->{$level} < $cost);
	$zoo->{costs}->{$level} -= $cost;
	#print "PICKED $tag $key $level\n";
	#print "COST $zoo->{costs}->{$level}\n";
	$zoo->{cute} += $cute;
	$zoo->{cool} += $cool;
	$zoo->{$tag}->{$key} = $level;
	pick($tag, $level + 1);
	$zoo->{$tag}->{$key} = 0;
	$zoo->{cute} -= $cute;
	$zoo->{cool} -= $cool;
	$zoo->{costs}->{$level} += $cost;
    }
}

pick('animals', 1);
