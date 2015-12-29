use strict;
use warnings;

BEGIN {
    unshift(@INC, "ppmclibs/blib/arch");
    unshift(@INC, "ppmclibs/blib/lib");
    unshift(@INC, "ocrlibs/blib/arch");
    unshift(@INC, "ocrlibs/blib/lib");
}
use tinycv;
use ocr;
use consoles::VNC;
use IO::Select;
use Time::HiRes qw(sleep gettimeofday time);
use Data::Dumper;
use File::Basename;

open(my $pf, '<', $ENV{HOME} . '/.vnc/passwd');
my $password = <$pf>;
close($pf);
my $vnc = consoles::VNC->new({ hostname => $ARGV[0],
			       password => $password,
			       port => $ARGV[1] || 5900 });

$vnc->login();

my %pngs;

sub read_png {
    my ($fn) = @_;
    $pngs{$fn} ||= tinycv::read($fn);
    return $pngs{$fn};
}

sub update_screen {
    $vnc->send_update_request;
    my $s = IO::Select->new();
    $s->add($vnc->socket);

    #print "can_read\n";
    for (my $i = 0; $i < 5; $i++) {
	if ($s->can_read(1)) {
	    #print "select says yes\n";
	    if (!$vnc->update_framebuffer) {
		$vnc->send_update_request;
		next;
	    }
	    #print "updated\n";
	    $vnc->_framebuffer->write("last.png");
	    return;
	}
    }
}

sub on_main_screen {
    #find_needle_coords('bauarbeiter.png');
    my $nimg = read_png('bauarbeiter.png');
    my $roi = $vnc->_framebuffer->copyrect(465, 22, $nimg->xres, $nimg->yres);
    my $sim = $roi->similarity($nimg);
    print "OMS $sim\n";
    return $sim > 30;
}

sub find_needle_coords {
    my ($nf, $silent) = @_;
    my $nn = read_png($nf);
    my ($sim, $xmatch, $ymatch) = $vnc->_framebuffer->search_needle($nn, 0, 0, $nn->xres, $nn->yres, 1400);
    $sim = $vnc->_framebuffer->copyrect($xmatch, $ymatch, $nn->xres, $nn->yres)->similarity($nn);
    if (!$silent) {
	print "SIM $nf - $sim X $xmatch Y $ymatch\n";
    }
    return ($sim, $xmatch, $ymatch);
}

sub park_cursor {
    my ($c) = @_;
    $vnc->send_update_request;
    if ($c) {
	$vnc->mouse_click( $vnc->_framebuffer->xres - 20, 20 );
    } else {
	$vnc->mouse_move_to( $vnc->_framebuffer->xres - 20, 20 );
    }
    update_screen;
}

#find_needle_coords('chat-close.png');

sub zoom_out {
    my $nimg = read_png('bushes.png');
    my $target_y = 100;
    my $target_x = 899;
    for (my $counter=1; $counter < 18; $counter++) {
	my $tsim = $vnc->_framebuffer->copyrect($target_x, $target_y, $nimg->xres, $nimg->yres)->similarity($nimg);
	return if ($tsim >= 30);
	my ($sim, $xm, $ym) = find_needle_coords('lower-bushes.png');
	if ($sim > 25) { # if we can see the lower end, we won't be able to find the bushes without scrolling down heavily
	    for (my $i = 0; $i < 100; $i++) {
		$vnc->send_pointer_event(1, 150, 20 + $i * 4 );
		update_screen if ($i % 10 == 0);
	    }
	    $vnc->send_pointer_event(0, 150, 20 + 100 * 4 );
	    update_screen;
	    next;
	}
	$vnc->init_x11_keymap;
	
	($sim, $xm, $ym) = find_needle_coords('bushes.png');
	if ($sim > 19) {
	    my $factor = -4;
	    $factor = 4 if ($ym < $target_y);
	    last if (abs($ym - $target_y) < abs($factor) * 2);
	    $vnc->send_pointer_event(0, 150, $ym );
	    update_screen;
	    $vnc->send_pointer_event(1, 150, $ym );
	    update_screen;
	    $vnc->send_pointer_event(1, 150, $target_y );
	    update_screen;
	    $vnc->send_pointer_event(0, 150, $target_y );
	    update_screen;
	} else {
	    $vnc->send_key_event_down($vnc->keymap->{ctrl});
	    for (my $i = 0; $i < 10; $i++) {
		$vnc->send_pointer_event(0x10, int($vnc->_framebuffer->xres * 2 / 10), int($vnc->_framebuffer->yres * 5 / 10));
		$vnc->send_update_request;
	    }
	    update_screen;
	    $vnc->send_key_event_up($vnc->keymap->{ctrl});
	    park_cursor(1);
	}
    }
}

sub check_chat_close {
    my $nimg = read_png('chat-close.png');
    my $roi = $vnc->_framebuffer->copyrect(527, 293, $nimg->xres, $nimg->yres);
    my $sim = $roi->similarity($nimg);
    print "SIM chat-close $sim\n";
    if ($sim > 16) {
	my $roi = $vnc->_framebuffer->copyrect(46, 210, 460, $vnc->_framebuffer->yres - 210);
	$roi->chat_ocr;
	$vnc->_framebuffer->write("chats/chat-" . time . ".png");
	$vnc->mouse_click(530, 295);
	park_cursor;
	return 1;
    } else {
	#$roi->write("chat-close-" . time . ".png");
    }
    return;
}

sub fix_main_screen {
    if (on_main_screen) {
	zoom_out;
    } else {
	print "OBSTACLE in the way!\n";
	return if check_chat_close;
	my ($sim, $xmatch, $ymatch) = find_needle_coords('other-device.png');
	if ($sim > 30) {
	    print "waiting for other device\n";
	    sleep(120);
	    ($sim, $xmatch, $ymatch) = find_needle_coords('reload-app.png');
	    $vnc->mouse_click($xmatch + 5, $ymatch + 5);
	    $vnc->send_update_request;
	    while (!on_main_screen) {
		update_screen;
	    }
	    zoom_out;
	    return;
	}
	my $nimg = read_png('spiel-neu-laden.png');
	if ($vnc->_framebuffer->copyrect(556, 486, $nimg->xres, $nimg->yres)->similarity($nimg) > 80) {
	    $vnc->mouse_click(int(556 + $nimg->xres / 2), int(486 + $nimg->yres / 2));
	    $vnc->send_update_request;
	    while (!on_main_screen) {
		update_screen;
	    }
	    zoom_out;
	    return;
	}
	for my $rx (qw/red-X.png red-X2.png end-fight.png home.png okay.png/) {
	    ($sim, $xmatch, $ymatch) = find_needle_coords($rx);
	    if ($sim > 15) {
		$vnc->mouse_click($xmatch + 5, $ymatch + 5);
		park_cursor;
		return fix_main_screen();
	    }
	}
	($sim, $xmatch, $ymatch) = find_needle_coords('pbt.png');
	if ($sim > 25) {
	    print "Personal Break - waiting 2 minutes\n";
	    sleep(30);
	    ($sim, $xmatch, $ymatch) = find_needle_coords('reload-app.png');
	    $vnc->mouse_click($xmatch + 5, $ymatch + 5);
	    park_cursor;
	    return fix_main_screen();
	}
	($sim, $xmatch, $ymatch) = find_needle_coords('retry.png');
	if ($sim > 25) {
	    sleep(30);
	    $vnc->mouse_click($xmatch + 5, $ymatch + 5);
	    park_cursor;
	    return fix_main_screen();
	}
	($sim, $xmatch, $ymatch) = find_needle_coords('coc-icon.png');
	if ($sim > 18) {
	    $vnc->mouse_click($xmatch + 5, $ymatch + 5);
	    $vnc->send_update_request;
	    sleep(3);
	    park_cursor;
	    return fix_main_screen();
	}
	($sim, $xmatch, $ymatch) = find_needle_coords('droid-fullscreen.png');
	if ($sim > 21) {
	    $vnc->mouse_click($xmatch + 20, $ymatch + 20);
	    park_cursor;
	    return fix_main_screen();
	}
	($sim, $xmatch, $ymatch) = find_needle_coords('droidx.png');
	if ($sim > 25) {
	    $vnc->mouse_click($xmatch + 5, $ymatch + 5);
	    park_cursor;
	    while (1) {
		($sim, $xmatch, $ymatch) = find_needle_coords('droidx-yes.png');
		if ($sim > 18) {
		    $vnc->mouse_click($xmatch + 5, $ymatch + 5);
		    last;
		}
		update_screen;
	    }
	    return fix_main_screen();
	}
	print "unknown obstacle\n";
	update_screen;
	sleep(2);
	fix_main_screen();
    }
}

sub collect_resources {
    my $done = 0;
    while (1) {
	my $found = 0;
	for my $n (qw/resource_de.png resource_elex.png resource_gold.png/) {
	    my $nn = read_png($n);
	    my ($sim, $xm, $ym) = $vnc->_framebuffer->search_needle($nn, 0, 0, $nn->xres, $nn->yres, 1300);
	    $sim = $vnc->_framebuffer->copyrect($xm, $ym, $nn->xres, $nn->yres)->similarity($nn);
	    if ($sim > 14) {
		$found = 1;
		print "FOUND $n\n";
		$vnc->mouse_click($xm + 10, $ym + 10);
		park_cursor(1);
		sleep .1;
		update_screen;
	    }
	}
	if ($found) {
	    $done = 1;
	} else {
	    last;
	}
    }
    return $done;
}

sub open_army_menu {
    my $nn = read_png('armymenu.png');
    my $sim = $vnc->_framebuffer->copyrect(27, 528, $nn->xres, $nn->yres)->similarity($nn);
    if ($sim > 20) {
	$vnc->mouse_click(35, 540);
	wait_for_screen('armyoverview.png', 541, 21, 5);
	return 1;
    }
    return;
}

sub check_troop {
    my ($nn, $barrack) = @_;
    my $prefix = $barrack ? 'b-' : '';
    for my $t (qw(archer barb balloon cobold dragon giant healer hog minion wallbreaker wizard golem pekka lava witch vali)) {
	my $ref = read_png("troops/$prefix$t.png");
	my $sim = $nn->similarity($ref);
	if ($sim > 20) {
	    return $t;
	}
    }
    return;
}

sub read_army_state {
    my $nn = read_png('armyoverviewTL.png');
    my $img = $vnc->_framebuffer;

    $vnc->mouse_click(260, 694);
    wait_for_screen('armyoverview.png', 541, 21, 5);

    my $fa = read_png('troops-label.png');
    my $tc = $img->copyrect(245, 84, $fa->xres, $fa->yres)->similarity($fa);
    print "TC $tc\n";
    return if $tc < 30;

    my $hash;
    #$vnc->_framebuffer->write('army-22.png');
    for (my $i = 0; $i < 1300; $i++) {
	my $sim = $img->copyrect($i, 123, $nn->xres, $nn->yres)->similarity($nn);
	if ($sim > 23) {
	    my $t = check_troop($img->copyrect($i + 2, 123 + 56, 84, 32));
	    if (!$t) {
		$img->copyrect($i + 2, 123 + 56, 84, 32)->write("troop-$i-A.png");
		die "unrecognized troops";
	    }
	    my $count = $img->copyrect($i + 10, 123 + 5, 70, 22);
	    $hash->{$t} = $count->troop_count('chars_troop_count');
	    $i += 85;
	}
    }
    return $hash;
}

sub check_chat {
    my $nn = read_png('chat.png');
    my $sim = $vnc->_framebuffer->copyrect(4, 290, $nn->xres, $nn->yres)->similarity($nn);
    if ($sim > 40) {
	$vnc->mouse_click(8, 300);
	park_cursor;
	while (1) {
	    update_screen;
	    return 1 if check_chat_close;
	}
    }
    return;
}

sub select_barrack {
    my ($b) = @_;

    my @coords = (375, 467, 559, 651, 780, 873);
    $vnc->mouse_click($coords[$b-1] + 20, 694);
    park_cursor;
    my $sb = read_png('selected-barrack.png');
    my $sb2 = read_png('selected-barrack-boost.png');
    my $stime = time;
    while (time - $stime < 5) {
	update_screen;
	my $sim = $vnc->_framebuffer->copyrect($coords[$b-1], 644, $sb->xres, $sb->yres)->similarity($sb);
	my $sim2 = $vnc->_framebuffer->copyrect($coords[$b-1], 644, $sb->xres, $sb->yres)->similarity($sb2);
	if ($sim > 17 || $sim2 > 17) {
	    sleep 0.1;
	    update_screen;
	    return;
	}
    }
    #$vnc->_framebuffer->copyrect($coords[$b-1], 644, $sb->xres, $sb->yres)->write("failed.png");
    die "failed to select barrack $b";
}

sub check_barrack {
    my $img = $vnc->_framebuffer;
    my $minus = read_png('minus.png');
    my $hash;
    for (my $i = 200; $i < 940; $i++) {
	my $sim = $img->copyrect($i, 132, $minus->xres, $minus->yres)->similarity($minus);
	if ($sim > 25) {
	    my $t = check_troop($img->copyrect($i - 71, 162, 84, 32), 1);
	    if (!$t) {
		$img->copyrect($i - 71, 162, 84, 32)->write("troop-$i-B.png");
	    }
	    my $tc = $img->copyrect($i - 71, 135, 60, 24)->troop_count('chars_barrack_count');
	    $hash->{$t} ||= 0;
	    $hash->{$t} += $tc;
	    $i += 90;
	}
    }
    return $hash;
}

sub room_for_troop {
    my ($t) = @_;
    
    my %rooms = ( giant => 5,
		  archer => 1,
		  barb => 1,
		  wallbreaker => 2,
		  vali => 8,
		  minion => 2,
		  hog => 5,
		  golem => 30,
		  witch => 12,
		  lava => 30,
		  cobold => 1,
		  balloon => 5,
		  wizard => 4,
		  healer => 14,
		  dragon => 20,
		  pekka => 25 );
    
    die "no room for $t" unless $rooms{$t};
    return $rooms{$t};
}

sub time_for_troop {
    my ($t) = @_;
    
    my %times = ( giant => 2*60,
		  archer => 20,
		  barb => 20,
		  wallbreaker => 2*60,
		  vali => 8*60,
		  minion => 45,
		  hog => 2*60,
		  golem => 45*60,
		  witch => 20*60,
		  lava => 45*60,
		  cobold => 30,
		  balloon => 8*60,
		  wizard => 8*60,
		  healer => 15*60,
		  dragon => 30*60,
		  pekka => 45*60 );
    
    die "no time for $t" unless $times{$t};
    return $times{$t};
}

sub time_for_troops {
    my ($building) = @_;
    my $time = 0;

    for my $t (keys %$building) {
	$time += $building->{$t} * time_for_troop($t);
    }
    return $time;
}

sub select_troop {
    my ($t) = @_;
    
    if ($t eq 'barb') {
	$vnc->mouse_click(355, 405);
    } elsif ($t eq 'archer') {
	$vnc->mouse_click(526, 405);
    } elsif ($t eq 'giant') {
	$vnc->mouse_click(690, 405);
    } elsif ($t eq 'cobold') {
	$vnc->mouse_click(845, 405);
    } elsif ($t eq 'wallbreaker') {
	$vnc->mouse_click(1012, 405);
    } elsif ($t eq 'balloon') {
	$vnc->mouse_click(355, 560);
    } elsif ($t eq 'wizard') {
	$vnc->mouse_click(526, 560);
    } elsif ($t eq 'healer') {
	$vnc->mouse_click(690, 560);
    } elsif ($t eq 'dragon') {
	$vnc->mouse_click(845, 560);
    } elsif ($t eq 'pekka') {
	$vnc->mouse_click(1012, 560);
    } else {
	die "no coordinates for $t";
    }
    sleep 0.05;
    update_screen;
}

sub select_troops {
    my ($troops) = @_;
    for my $t (keys %$troops) {
	my $number = $troops->{$t};
	for (my $number = $troops->{$t}; $number; $number--) {
	    select_troop($t);
	}
    }
    return;
}

my @barracks;
our $min_train_time = 1;

sub train_troops {
    return if (!open_army_menu);

    my $building = {};
    for (my $i = 0; $i < 6; $i++) {
	select_barrack($i+1);
	my $b = $barracks[$i] = check_barrack;
	for my $t (keys %$b) {
	    $building->{$t} += $b->{$t};
	}
    }
    my $army = read_army_state;
    return unless $army;
    my $total = 0;
    for my $t (keys %$army) {
	$total += $army->{$t} * room_for_troop($t);
    }
    print "ARMY\n";
    print Dumper($army);
    print "BUILDING\n";
    print Dumper($building);
    my $soll = { giant => 12,
		 wallbreaker => 12 };
    my $rest = 220;
    if ($total == 220) {
	$rest *= 2;
    }
    for my $t (keys %$soll) {
	my $c = ($building->{$t} || 0) + ($army->{$t} || 0);
	$soll->{$t} *= 2 if ($total == 220);
	if ($soll->{$t} < $c) {
	    $soll->{$t} = $c;
	}
	$rest -= $soll->{$t} * room_for_troop($t);
    }
    print "TOTAL $total REST $rest\n";
    # align to 4
    $soll->{barb} = int(int($rest * 30 / 100) / 4 + 0.5) * 4;
    $rest -= $soll->{barb} * room_for_troop('barb');
    $soll->{archer} = int($rest / room_for_troop('archer'));
    my $diff;
    print Dumper($soll);
    for my $t (keys %$soll) {
	$diff->{$t} = $soll->{$t} - ($army->{$t} || 0) - ($building->{$t} || 0);
	$diff->{$t} = 0 if ($diff->{$t} < 0);
    }
    print "DIFF\n";
    print Dumper($diff);

    my @newbuilds;

    while (%$diff) {
	my @troops = sort { time_for_troop($b) <=> time_for_troop($a) } keys %$diff;
	my $t = shift @troops;
	if ($diff->{$t} <= 0) {
	    delete $diff->{$t};
	    next;
	}
	print "FIND spot for $t - $diff->{$t}\n";
	$min_train_time = time_for_troops($barracks[0]);
	my $min = 0;
	for (my $i = 1; $i < 4; $i++) {
	    my $time = time_for_troops($barracks[$i]);
	    if ($time < $min_train_time) {
		$min = $i;
		$min_train_time = $time;
	    }
	}
	print "FOUND $min\n";
	$newbuilds[$min]->{$t} ||= 0;
	$newbuilds[$min]->{$t} += 1;
	$barracks[$min]->{$t} += 1;
	$diff->{$t} -= 1;
    }
    print "BARCKS\n";
    print Dumper($newbuilds[0]);
    print Dumper($newbuilds[1]);
    print Dumper($newbuilds[2]);
    print Dumper($newbuilds[3]);
    $min_train_time = 10000;
    for (my $i = 0; $i < 4; $i++) {
	my $sum = 0;
	for my $v (values %{$newbuilds[$i]}) {
	    $sum += $v;
	}
	my $time = time_for_troops($barracks[$i]);
	if ($time < $min_train_time) {
	    $min_train_time = $time;
	}
	next unless $sum;
	select_barrack($i+1);
	select_troops($newbuilds[$i]);
    }
    park_cursor(1);
    return $total == 220;
}

sub wait_for_screen {
    my ($fn, $x, $y, $timeout) = @_;
    $timeout ||= 5;
    my $nn = read_png($fn);
    for (my $i = 0; $i < $timeout; $i++) {
	my $sim = $vnc->_framebuffer->copyrect($x, $y, $nn->xres, $nn->yres)->similarity($nn);
	print "SIM $sim\n";
	return 1 if ($sim > 30);
	update_screen;
    }
    return;
}

sub check_base_resources {
    my $nn = read_png('base-gold.png');
    my $gold = 0;
    my $elex = 0;
    my $de = 0;

    my $sim = $vnc->_framebuffer->copyrect(33, 96, $nn->xres, $nn->yres)->similarity($nn);
    if ($sim > 23) {
	$gold = $vnc->_framebuffer->copyrect(66, 95, 110, 30)->base_count('chars_base_count');
    }
    $nn = read_png('base-elex.png');
    $sim = $vnc->_framebuffer->copyrect(32, 136, $nn->xres, $nn->yres)->similarity($nn);
    if ($sim > 23) {
	$elex =  $vnc->_framebuffer->copyrect(66, 135, 110, 30)->base_count('chars_base_count');
    }
    $nn = read_png('base-de.png');
    $sim = $vnc->_framebuffer->copyrect(31, 176, $nn->xres, $nn->yres)->similarity($nn);
    if ($sim > 23) {
	$de =  $vnc->_framebuffer->copyrect(66, 176, 110, 30)->base_count('chars_base_count');
    }
    print "BASE $gold gold $elex elex $de DE\n";
    # not worth the TH check
    return if ($gold + $elex < 150000 || $de < 100);
    for my $th (glob("ths/*-th-*.png")) {
	my ($sim, $xmatch, $ymatch) = find_needle_coords($th, 1);
	if ($sim > 14) {
	    return ($1, $gold, $elex, $de) if $th =~ /.*-th-(\d*).png/;
	}
    }
    print "UNKNOWN TH";
    return 20;
}

sub worth_it {
    my ($th, $gold, $elex, $de) = @_;
    if ($th == 8) {
	return ($gold + $elex + $de * 100 > 220000)
    }
    if ($th == 19) {
	return ($gold + $elex > 600000 && $de > 2000);
    }
    return;
}

sub find_attack_troops {
    my $res;
    my @troops = qw(barb archer cobold giant wallbreaker wizard minion 
		    hog CB king queen spell-heal spell-poison spell-haste );
    for (my $spot = 7; $spot < 1250; $spot++) {
	# NOT YET: vali golem witch lava balloon pekka dragon healer
	for my $t (@troops) {
	    my $ref = read_png("attack/$t.png") || next;
	    my $img = $vnc->_framebuffer->copyrect($spot, 658, 100, 52);
	    my ($sim, $xmatch, $ymatch) = $img->search_needle($ref, 0, 0, $ref->xres, $ref->yres, 70);
	    my $f = $img->copyrect($xmatch, $ymatch, $ref->xres, $ref->yres);
	    $sim = $f->similarity($ref);
	    #$img->write("img-$spot.png");
	    #print "I $t $spot $sim\n";
	    if ($sim > 15) {
		my $c = $vnc->_framebuffer->copyrect($spot + 4, 629, 88, 27);
		if ($t eq 'CB' || $t eq 'king' || $t eq 'queen') {
		    $c = 1;
		} else {
		    $c = $c->troop_count('chars_barrack_count');
		}
		push(@$res, [$t, $spot, $c ]);
		$spot += 100;
		do {
		    my $f = shift @troops;
		    if ($f =~ /spell-/) {
			unshift @troops, $f;
			last;
		    }
		    last if $f eq $t;
		} while ($f);
		last;
	    }
	}
    }
    return $res;
}

sub spots_on_red_line {
    my ($x1, $y1, $x2, $y2, $count) = @_;
    print "select $count between $x1+$y1 and $x2+$y2\n";
    my $clicks = 0;
    my $delta = int(($x2 - $x1) / ($count + 1));
    my $dX = ($x2 - $x1) / ($y2 - $y1);
    for (my $x = $x1 + $delta; $x <= $x2 - $delta; $x += $delta) {
	my $y = int(($x - $x1) / $dX + $y1 + 0.5);
	$vnc->mouse_click($x, $y);
	next;
	$clicks++;
	if ($clicks > 6) {
	    update_screen;
	    $clicks = 0;
	} else {
	    $vnc->send_update_request;
	}
    }
    update_screen;
}

sub select_attack_troop {
    my ($kind) = @_;

    my $troops = find_attack_troops;
    print Dumper($troops);
    for my $ti (@$troops) {
	my ($t, $spot, $count) = @$ti;
	if ($t eq $kind) {
	    $vnc->mouse_click($spot + 20, 670);
	    $vnc->send_update_request;
	    return $count;
	}
    }
    return 0;
}

sub attack {
    my ($x1, $y1, $x2, $y2) = $vnc->_framebuffer->find_red_line;
    print "RES $x1+$y1 - $x2+$y2\n";
    system("aplay /usr/share/xemacs/xemacs-packages/etc/sounds/long-beep.wav");
    my $giants = select_attack_troop('giant');
    my $cc = $giants / 2;
    spots_on_red_line($x1, $y1, $x2, $y2, $cc);
    update_screen;
    $giants -= $cc;
    sleep 1;
    my $archers = select_attack_troop('archer');
    $cc = $archers / 3;
    spots_on_red_line($x1, $y1, $x2, $y2, $cc);
    update_screen;
    $archers -= $cc;
    my $cb = select_attack_troop('CB');
    if ($cb) {
	spots_on_red_line($x1, $y1, $x2, $y2, 1);
    }
    my $wbs = select_attack_troop('wallbreaker');
    $cc = $wbs / 4;
    spots_on_red_line($x1, $y1, $x2, $y2, $cc);
    update_screen;
    $wbs -= $cc;
    $cc = $giants / 2;
    spots_on_red_line($x1, $y1, $x2, $y2, $cc);
    update_screen;
    $giants -= $cc;
    my $barbs = select_attack_troop('barb');
    $cc = $barbs / 3;
    spots_on_red_line($x1, $y1, $x2, $y2, $cc);
    update_screen;
    $archers -= $cc;
}

sub find_worthy_base {
    fix_main_screen;
    return if !on_main_screen;
    $vnc->mouse_click(60, 650);
    # there are 2 different places - with or without shield
    if (wait_for_screen('find-fight.png', 207, 572, 5)) {
	$vnc->mouse_click(220, 590);
    } else {
	wait_for_screen('find-fight.png', 207, 521, 5) || die "no find-fight";
	$vnc->mouse_click(220, 540);
    }
    my $time_to_next = time;
    my $next = read_png('next.png');
    while (1) {
	update_screen;
	my $sim = $vnc->_framebuffer->copyrect(1118, 503, $next->xres, $next->yres)->similarity($next);
	print "NEXT $sim\n";
	if ($sim > 30) {
	    my $bfn = "bases/base-" . time . ".png";
	    print "BASE $bfn\n";
	    $vnc->_framebuffer->write($bfn);
	    my ($th, $gold, $elex, $de) = check_base_resources;
	    if ($th && worth_it($th, $gold, $elex, $de)) {
		return 1;
	    }
	    $time_to_next = time;
	    $vnc->mouse_click(1250, 550);
	    sleep .3;
	    park_cursor;
	    next;
	}
	if (time - $time_to_next < 8) {
	    print "TIME " . (time - $time_to_next) . "\n";
	    update_screen;
	    next;
	}
	my $nn = read_png('reload-game.png');
	$sim = $vnc->_framebuffer->copyrect(561, 488, $nn->xres, $nn->yres)->similarity($nn);
	if ($sim > 30) {
	    $vnc->mouse_click(590, 570);
	    return;
	}
	$nn = read_png('retry.png');
	$sim = $vnc->_framebuffer->copyrect(549, 510, $nn->xres, $nn->yres)->similarity($nn);
	if ($sim > 30) {
	    $vnc->mouse_click(590, 530);
	    return;
	}
	$nn = read_png('end-fight.png');
	$sim = $vnc->_framebuffer->copyrect(36, 551, $nn->xres, $nn->yres)->similarity($nn);
	if ($sim > 30) {
	    return;
	}
    }
    return;
}

for my $base (glob("bases/base-*.png")) {
    print "BASE $base\n";
    $vnc->_framebuffer(tinycv::read($base));

    my $found;
    for my $th (glob("ths/*.png")) {
	my ($sim, $xmatch, $ymatch) = find_needle_coords($th, 1);
	print "$th $sim\n";
	if ($sim > 14) {
	    if ($th =~ /-th-(\d+).png/) {
		rename($base, "bases/th$1-" . basename($base));
		$found = 1;
		last;
	    }
	}
    }
    if (!$found) {
	rename($base, "bases/thX-" . basename($base));
    }
    $vnc->_framebuffer(undef);
}

my $stime = time;
while (time - $stime < 3 && !$vnc->_framebuffer) {
    update_screen;
}
die "still no screen" unless $vnc->_framebuffer;

while (1) {
    fix_main_screen;
    while (on_main_screen) {
	update_screen;
	next if check_chat;
	if (train_troops) {
	    next unless find_worthy_base;
	    attack;
	}
	$min_train_time = 120 if ($min_train_time > 120);
	$min_train_time = 10 if ($min_train_time < 10);
	print "nothing los - waiting $min_train_time seconds\n";
	sleep($min_train_time);
	$min_train_time = 0;
	collect_resources;
    }
}

print "done with main screen\n";
