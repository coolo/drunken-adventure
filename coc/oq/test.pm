use strict;
use warnings;

BEGIN {
    unshift(@INC, "ppmclibs/blib/arch");
    unshift(@INC, "ppmclibs/blib/lib");
}
use tinycv;
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
    $vnc->send_update_request;
    my $s = IO::Select->new();
    $s->add($vnc->socket);

    while (IO::Select->select($s, $s, undef, 20)) {
	next unless $vnc->update_framebuffer;
	$vnc->_framebuffer->write("last.png");
	return;
    }
}

update_screen;

sub on_main_screen {
    my $nimg = tinycv::read('bauarbeiter.png');
    my $roi = $vnc->_framebuffer->copyrect(453, 6, $nimg->xres, $nimg->yres);
    $nimg->threshold(220);
    $roi->threshold(220);
    my $sim = $roi->similarity($nimg);
    return $sim > 19;
}

sub find_needle_coords {
    my ($nf) = @_;
    my $nn = tinycv::read($nf);
    my ($sim, $xmatch, $ymatch) = $vnc->_framebuffer->search_needle($nn, 0, 0, $nn->xres, $nn->yres, 1000);
    $sim = $vnc->_framebuffer->copyrect($xmatch, $ymatch, $nn->xres, $nn->yres)->similarity($nn);
    print "SIM $nf - $sim X $xmatch Y $ymatch\n";
    return ($sim, $xmatch, $ymatch);
}

sub park_cursor {
    $vnc->mouse_click( 10, 10 );
    update_screen;
}

#find_needle_coords('chat.png');

sub zoom_out {
    my $nimg = tinycv::read('bushes.png');
    my $target_y = 100;
    my $target_x = 899;
    my $sim = $vnc->_framebuffer->copyrect($target_x, $target_y, $nimg->xres, $nimg->yres)->similarity($nimg);
    if ($sim < 30) {
	$vnc->init_x11_keymap;
	for (my $counter=1; $counter < 18; $counter++) {
	    my ($sim, $xm, $ym) = find_needle_coords('bushes.png');
	    if ($sim > 19) {
		my $factor = -1;
		$factor = 1 if ($ym < $target_y);
		last if (abs($ym - $target_y) < 8);
		$vnc->send_pointer_event(0, 150, $ym );
		update_screen;
		for (my $i = 0; $i < abs($ym - $target_y); $i++) {
		    $vnc->send_pointer_event(1, 150, $ym + $i*$factor );
		    update_screen if ($i % 10 == 0);
		}
		$vnc->send_pointer_event(0, 150, $target_y );
		update_screen;
	    } else {
		$vnc->send_key_event_down($vnc->keymap->{ctrl});
		$vnc->send_pointer_event(0x10, int($vnc->_framebuffer->xres * 2 / 10), int($vnc->_framebuffer->yres * 5 / 10));
		update_screen;
		$vnc->send_pointer_event(0, 10, 10 );
		$vnc->send_key_event_up($vnc->keymap->{ctrl});
		park_cursor;
	    }
	}
    }
}

sub fix_main_screen {
    if (on_main_screen) {
	zoom_out;
    } else {
	print "OBSTACLE in the way!\n";
	my ($sim, $xmatch, $ymatch) = find_needle_coords('chat-close.png');
	if ($sim > 30) {
	    $vnc->mouse_click($xmatch + 5, $ymatch + 5);
	    park_cursor;
	    return;
	}
	($sim, $xmatch, $ymatch) = find_needle_coords('other-device.png');
	if ($sim > 30) {
	    print "waiting for other device\n";
	    sleep(120);
	    ($sim, $xmatch, $ymatch) = find_needle_coords('reload-app.png');
	    $vnc->mouse_click($xmatch + 5, $ymatch + 5);
	    while (!on_main_screen) {
		update_screen;
	    }
	    zoom_out;
	    return;
	}
	my $nimg = tinycv::read('spiel-neu-laden.png');
	if ($vnc->_framebuffer->copyrect(556, 486, $nimg->xres, $nimg->yres)->similarity($nimg) > 80) {
	    $vnc->mouse_click(int(556 + $nimg->xres / 2), int(486 + $nimg->yres / 2));
	    while (!on_main_screen) {
		update_screen;
	    }
	    zoom_out;
	    return;
	}

	print "unknown obstacle\n";
	sleep(10);
	fix_main_screen;
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

sub check_chat {
    my $nn = tinycv::read('chat.png');
    my $sim = $vnc->_framebuffer->copyrect(4, 290, $nn->xres, $nn->yres)->similarity($nn);
    if ($sim > 40) {
	$vnc->mouse_click(8, 300);
	while (1) {
	    update_screen;
	    my ($sim, $xmatch, $ymatch) = find_needle_coords('chat-close.png');
	    if ($sim > 30) {
		$vnc->_framebuffer->write("chat-" . time . ".png");
		$vnc->mouse_click($xmatch + 5, $ymatch + 5);
		return 1;
	    }
	}
    }
    return;
}

while (1) {
    fix_main_screen;
    while (on_main_screen) {
	sleep(1);
	update_screen;
	next if check_chat;
	next if collect_resources;
	print "nothing los\n";
    }
}

print "done with main screen\n";
