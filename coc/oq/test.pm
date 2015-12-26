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

open(my $pf, '<', $ENV{HOME} . '/.vnc/passwd');
my $password = <$pf>;
close($pf);
my $vnc = consoles::VNC->new({ hostname => $ARGV[0],
			       password => $password,
			       port => $ARGV[1] || 5900 });

$vnc->login();
sub update_screen {
    #$vnc->_framebuffer(tinycv::read('last.png'));
    #return;
    $vnc->send_update_request;
    my $s = IO::Select->new();
    $s->add($vnc->socket);

    #print "can_read\n";
    while ($s->can_read(20)) {
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

sub on_main_screen {
    update_screen if (!$vnc->_framebuffer);
    my $nimg = tinycv::read('bauarbeiter.png');
    my $roi = $vnc->_framebuffer->copyrect(453, 6, $nimg->xres, $nimg->yres);
    $nimg->threshold(220);
    $roi->threshold(220);
    my $sim = $roi->similarity($nimg);
    print "OMS $sim\n";
    return $sim > 20;
}

sub find_needle_coords {
    my ($nf) = @_;
    my $nn = tinycv::read($nf);
    my ($sim, $xmatch, $ymatch) = $vnc->_framebuffer->search_needle($nn, 0, 0, $nn->xres, $nn->yres, 1400);
    $sim = $vnc->_framebuffer->copyrect($xmatch, $ymatch, $nn->xres, $nn->yres)->similarity($nn);
    print "SIM $nf - $sim X $xmatch Y $ymatch\n";
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
    my $nimg = tinycv::read('bushes.png');
    my $target_y = 100;
    my $target_x = 899;
    my $sim = $vnc->_framebuffer->copyrect($target_x, $target_y, $nimg->xres, $nimg->yres)->similarity($nimg);
    if ($sim < 30) {
	my ($sim, $xmatch, $ymatch) = find_needle_coords('lower-bushes.png');
	if ($sim > 25) { # if we can see the lower end, we won't be able to find the bushes without scrolling down heavily
	    for (my $i = 0; $i < 100; $i++) {
		$vnc->send_pointer_event(1, 150, 20 + $i * 4 );
		update_screen if ($i % 10 == 0);
	    }
	    $vnc->send_pointer_event(0, 150, 20 + 100 * 4 );
	    update_screen;
	    zoom_out();
	}
	$vnc->init_x11_keymap;
	for (my $counter=1; $counter < 18; $counter++) {
	    my ($sim, $xm, $ym) = find_needle_coords('bushes.png');
	    if ($sim > 19) {
		my $factor = -4;
		$factor = 4 if ($ym < $target_y);
		last if (abs($ym - $target_y) < 8);
		$vnc->send_pointer_event(0, 150, $ym );
		update_screen;
		for (my $i = 0; $i < abs($ym - $target_y) / abs($factor); $i++) {
		    $vnc->send_pointer_event(1, 150, $ym + $i*$factor );
		    update_screen;
		}
		$vnc->send_pointer_event(0, 150, $target_y );
		update_screen;
	    } else {
		$vnc->send_key_event_down($vnc->keymap->{ctrl});
		$vnc->send_pointer_event(0x10, int($vnc->_framebuffer->xres * 2 / 10), int($vnc->_framebuffer->yres * 5 / 10));
		update_screen;
		$vnc->send_key_event_up($vnc->keymap->{ctrl});
		park_cursor(1);
	    }
	}
    }
}

sub check_chat_close {
    my $nimg = tinycv::read('chat-close.png');
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
	my $nimg = tinycv::read('spiel-neu-laden.png');
	if ($vnc->_framebuffer->copyrect(556, 486, $nimg->xres, $nimg->yres)->similarity($nimg) > 80) {
	    $vnc->mouse_click(int(556 + $nimg->xres / 2), int(486 + $nimg->yres / 2));
	    $vnc->send_update_request;
	    while (!on_main_screen) {
		update_screen;
	    }
	    zoom_out;
	    return;
	}
	($sim, $xmatch, $ymatch) = find_needle_coords('red-X.png');
	if ($sim > 15) {
	    $vnc->mouse_click($xmatch + 5, $ymatch + 5);
	    $vnc->send_update_request;
	    while (!on_main_screen) {
		update_screen;
	    }
	    zoom_out;
	    return;
	}
	($sim, $xmatch, $ymatch) = find_needle_coords('pbt.png');
	if ($sim > 25) {
	    print "Personal Break - waiting 2 minutes\n";
	    sleep(30);
	    ($sim, $xmatch, $ymatch) = find_needle_coords('reload-app.png');
	    $vnc->mouse_click($xmatch + 5, $ymatch + 5);
	    park_cursor;
	    fix_main_screen();
	    return;
	}
	($sim, $xmatch, $ymatch) = find_needle_coords('retry.png');
	if ($sim > 25) {
	    sleep(30);
	    $vnc->mouse_click($xmatch + 5, $ymatch + 5);
	    park_cursor;
	    fix_main_screen();
	    return;
	}
	($sim, $xmatch, $ymatch) = find_needle_coords('coc-icon.png');
	if ($sim > 18) {
	    $vnc->mouse_click($xmatch + 5, $ymatch + 5);
	    $vnc->send_update_request;
	    sleep(3);
	    park_cursor;
	    fix_main_screen();
	    return;
	}
	($sim, $xmatch, $ymatch) = find_needle_coords('droid-fullscreen.png');
	if ($sim > 21) {
	    $vnc->mouse_click($xmatch + 20, $ymatch + 20);
	    park_cursor;
	    fix_main_screen();
	    return;
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
	    fix_main_screen();
	    return;
	}
	print "unknown obstacle\n";
	sleep(10);
	update_screen;
	fix_main_screen();
    }
}

sub collect_resources {
    my $done = 0;
    while (1) {
	my $found = 0;
	for my $n (qw/resource_de.png resource_elex.png resource_gold.png/) {
	    my $nn = tinycv::read($n);
	    my ($sim, $xm, $ym) = $vnc->_framebuffer->search_needle($nn, 0, 0, $nn->xres, $nn->yres, 1000);
	    $sim = $vnc->_framebuffer->copyrect($xm, $ym, $nn->xres, $nn->yres)->similarity($nn);
	    if ($sim > 14) {
		$found = 1;
		print "FOUND $n\n";
		$vnc->mouse_click($xm + 10, $ym + 10);
		park_cursor;
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
    my $nn = tinycv::read('armymenu.png');
    my $sim = $vnc->_framebuffer->copyrect(27, 528, $nn->xres, $nn->yres)->similarity($nn);
    if ($sim > 17) {
	$vnc->mouse_click(35, 540);
	while (1) {
	    update_screen;
	    my ($sim, $xmatch, $ymatch) = find_needle_coords('armyoverview.png');
	    return 1 if ($sim > 20);
	    #$vnc->mouse_click(35, 540);
	}
    }
    return;
}

sub check_troop {
    my ($nn, $barrack) = @_;
    my $prefix = $barrack ? 'b-' : '';
    for my $t (qw(archer barb cobold dragon giant healer hog minion wallbreaker wizard golem pekka lava witch vali)) {
	my $ref = tinycv::read("troops/$prefix$t.png");
	my $sim = $nn->similarity($ref);
	if ($sim > 20) {
	    my $ref = tinycv::read("troops/$t.png");
	    my $sim1 = $nn->similarity($ref);
	    $ref = tinycv::read("troops/b-$t.png");
	    my $sim2 = $nn->similarity($ref);
	    print "TROP $t $sim1 $sim2\n";
	    return $t;
	}
    }
    return;
}

sub read_army_state {
    find_needle_coords('armyoverviewTL.png');
    my $nn = tinycv::read('armyoverviewTL.png');
    my $img = $vnc->_framebuffer;

    my $hash;
    for (my $i = 0; $i < 1300; $i++) {
	my $sim = $img->copyrect($i, 123, $nn->xres, $nn->yres)->similarity($nn);
	if ($sim > 23) {
	    my $t = check_troop($img->copyrect($i + 2, 123 + 56, 84, 32));
	    if (!$t) {
		$img->copyrect($i + 2, 123 + 56, 84, 32)->write("troop-$i-A.png");
	    }
	    my $count = $img->copyrect($i + 10, 123 + 5, 70, 22);
	    $hash->{$t} = $count->troop_count('chars_troop_count');
	    $i += 85;
	}
    }
    print Dumper($hash);
    return $hash;
}

sub check_chat {
    my $nn = tinycv::read('chat.png');
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
    my $sb = tinycv::read('selected-barrack.png');
    while (1) {
	update_screen;
	my $sim = $vnc->_framebuffer->copyrect($coords[$b-1], 644, $sb->xres, $sb->yres)->similarity($sb);
	if ($sim > 30) {
	    sleep 0.3;
	    update_screen;
	    return;
	}
    }
    return;
}

sub check_barrack {
    my $img = $vnc->_framebuffer;
    my $minus = tinycv::read('minus.png');
    my $hash;
    for (my $i = 200; $i < 940; $i++) {
	my $sim = $img->copyrect($i, 132, $minus->xres, $minus->yres)->similarity($minus);
	if ($sim > 25) {
	    my $t = check_troop($img->copyrect($i - 71, 162, 84, 32), 1);
	    if (!$t) {
		$img->copyrect($i - 71, 162, 84, 32)->write("troop-$i-B.png");
	    }
	    my $tc = $img->copyrect($i - 71, 135, 60, 24)->troop_count('chars_barrack_count');
	    $hash->{$t} = $tc;
	    $i += 90;
	}
    }
    print Dumper($hash);
    return $hash;
}

while (1) {
    fix_main_screen;
    while (on_main_screen) {
	update_screen;
	next if check_chat;
	next if collect_resources;
	if (open_army_menu) {
	    read_army_state;

	    select_barrack(5);
	    print "SELECTED\n";
	    check_barrack;
	    exit(1);
	}
	print "nothing los\n";
	sleep(10);
    }
}

print "done with main screen\n";
