#! /usr/bin/perl

use Data::Dumper;

my %buildings;

sub add_building {
    my ($nick, $name, $priority ) = @_;
    $buildings{$nick} = { name => $name, numbers => {}, priority => $priority };
    for my $th (1..10) {
	$buildings{$nick}->{numbers}->{$th} = 0;
    }
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
    my ($nick, $num, $level) = @_;
    $buildings{$nick}->{complete}->{$num} = $level;
}

add_building('wt', 'Wizard Tower', 830);
set_number('wt', 5, 1);
set_number('wt', 6, 2);
set_number('wt', 8, 3);
set_number('wt', 9, 4);

set_level('wt', 1, "180,000 gold", '12h', 5);
set_level('wt', 2, "360,000 gold", '1d', 5);
set_level('wt', 3, "720,000 gold", '2d', 6);
set_level('wt', 4, "1,280,000 gold", '3d', 7);
set_level('wt', 5, "1,960,000 gold", '4d', 8);
set_level('wt', 6, "2,680,000 gold", '5d', 8);
set_level('wt', 7, "5,360,000 gold", '7d', 9);
set_level('wt', 8, "6,480,000 gold", '10d', 10);

add_building('cannon', 'Cannon', 500);

set_number('cannon', 1, 2);
set_number('cannon', 5, 3);
set_number('cannon', 7, 5);
set_number('cannon', 10, 6);

set_level('cannon', 2, "1,250 gold", "15min", 1);
set_level('cannon', 3, "4,000 gold", "45min", 2);
set_level('cannon', 4, "16,000 gold", "2h", 3);
set_level('cannon', 5, "50,000 gold", "6h", 4);
set_level('cannon', 6, "100,000 gold", "12h", 5);
set_level('cannon', 7, "200,000 gold", "1d", 6);
set_level('cannon', 8, "400,000 gold", "2d", 7);
set_level('cannon', 9, "800,000 gold", "3d", 8);
set_level('cannon', 10,	"1,600,000 gold", "4d", 8);
set_level('cannon', 11,	"3,200,000 gold", "5d", 9);
set_level('cannon', 12,	"6,400,000 gold", "6d", 10);
set_level('cannon', 13,	"7,500,000 gold", "7d", 10);

set_complete('wt', 1, 3);
set_complete('wt', 2, 1);

set_complete('cannon', 1, 5);
set_complete('cannon', 2, 6);
set_complete('cannon', 3, 7);

#print Dumper(\%buildings);

for my $nick (sort keys %buildings) {
    for my $num (1..$buildings{$nick}->{numbers}->{10}) {
	my $firstlvl = 1;
	$firstlvl++ if ( !$buildings{$nick}->{effort}->{$firstlvl} );

	for my $lvl ($firstlvl..$buildings{$nick}->{maxlvl}) {
	    print "task $nick\_$num\_$lvl \"";
	    my @deps;
	    if ($lvl == $firstlvl) {
		printf "Build %s $num as Level $lvl", $buildings{$nick}->{name};
	    } else {
		printf "Upgrade %s $num to Level $lvl", $buildings{$nick}->{name};
		push(@deps, "!$nick\_$num\_" . ($lvl - 1));
	    }
	    print "\" { \n";
	    if (($buildings{$nick}->{complete}->{$num} // 0) < $lvl) {
		printf "  \${builder_effort \"%s\"}\n", $buildings{$nick}->{effort}->{$lvl};
	    }
	    my $thlvl = $buildings{$nick}->{required}->{$lvl};
	    while ($num > $buildings{$nick}->{numbers}->{$thlvl}) {
		$thlvl++;
	    }
	    push(@deps, "!th_$thlvl") if ($thlvl > 1);
	    print "  priority " . $buildings{$nick}->{priority} . "\n";
	    print "  depends " . join(', ', @deps) . "\n" if (@deps);
	    print "}\n\n";
	}
    }
}

