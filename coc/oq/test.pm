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
use bmwqemu qw(diag);
use Telegram::BotAPI;

open(my $pf, '<', $ENV{HOME} . '/.vnc/passwd');
my $password = <$pf>;
close($pf);
my $vnc = consoles::VNC->new(
    {
        hostname => $ARGV[0],
        depth    => 16,
        password => $password,
        port     => $ARGV[1] || 5900
    });

open(my $fh, '<', $ENV{HOME} . '/.botkey');
my $bot_token  = <$fh>;
my $bot_chatid = <$fh>;
chomp $bot_token;
chomp $bot_chatid;
close($fh);

my $botapi = Telegram::BotAPI->new(token => $bot_token);
$vnc->login();

my %pngs;

sub read_png {
    my ($fn) = @_;
    $pngs{$fn} ||= tinycv::read($fn);
    return $pngs{$fn};
}

sub update_screen {
    my $stime = time;
    $vnc->send_update_request;
    my $s = IO::Select->new();
    $s->add($vnc->socket);

    for (my $i = 0; $i < 10; $i++) {
        if ($s->can_read(.2)) {
            if (!$vnc->update_framebuffer) {
                $vnc->send_update_request;
                next;
            }
            #$vnc->_framebuffer->write("last.png");
            last;
        }
    }
    $stime = time - $stime;
    diag "update_screen done $stime";
}

sub on_main_screen {
    #find_needle_coords('bauarbeiter.png');
    my $nimg = read_png('bauarbeiter.png');
    my $roi  = $vnc->_framebuffer->copyrect(465, 22, $nimg->xres, $nimg->yres);
    my $sim  = $roi->similarity($nimg);
    diag "OMS $sim";
    return $sim > 30;
}

sub find_needle_coords {
    my ($nf, $silent) = @_;
    my $nn = read_png($nf);
    my ($sim, $xmatch, $ymatch) = $vnc->_framebuffer->search_needle($nn, 0, 0, $nn->xres, $nn->yres, 1400);
    $sim = $vnc->_framebuffer->copyrect($xmatch, $ymatch, $nn->xres, $nn->yres)->similarity($nn);
    if (!$silent) {
        diag "FNC $nf - $sim X $xmatch Y $ymatch";
    }
    return ($sim, $xmatch, $ymatch);
}

sub park_cursor {
    my ($c) = @_;
    $vnc->send_update_request;
    if ($c) {
        $vnc->mouse_click($vnc->_framebuffer->xres - 20, 20);
    }
    else {
        $vnc->mouse_move_to($vnc->_framebuffer->xres - 20, 20);
    }
    update_screen;
}

#find_needle_coords('chat-close.png');

sub zoom_out {
    my $nimg     = read_png('bushes.png');
    my $target_y = 100;
    my $target_x = 899;
    for (my $counter = 1; $counter < 18; $counter++) {
        my $tsim = $vnc->_framebuffer->copyrect($target_x, $target_y, $nimg->xres, $nimg->yres)->similarity($nimg);
        return if ($tsim >= 30);
        my ($sim, $xm, $ym) = find_needle_coords('lower-bushes.png');
        if ($sim > 25) {    # if we can see the lower end, we won't be able to find the bushes without scrolling down heavily
            for (my $i = 0; $i < 100; $i++) {
                $vnc->send_pointer_event(1, 150, 20 + $i * 4);
                update_screen if ($i % 10 == 0);
            }
            $vnc->send_pointer_event(0, 150, 20 + 100 * 4);
            update_screen;
            next;
        }
        $vnc->init_x11_keymap;

        ($sim, $xm, $ym) = find_needle_coords('bushes.png');
        if ($sim > 19) {
            my $factor = -4;
            $factor = 4 if ($ym < $target_y);
            last if (abs($ym - $target_y) < abs($factor) * 2);
            $vnc->send_pointer_event(0, 150, $ym);
            update_screen;
            $vnc->send_pointer_event(1, 150, $ym);
            update_screen;
            $vnc->send_pointer_event(1, 150, $target_y);
            update_screen;
            $vnc->send_pointer_event(0, 150, $target_y);
            update_screen;
        }
        else {
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
    my $roi  = $vnc->_framebuffer->copyrect(527, 293, $nimg->xres, $nimg->yres);
    my $sim  = $roi->similarity($nimg);
    diag "SIM chat-close $sim";
    if ($sim > 16) {
        my $roi = $vnc->_framebuffer->copyrect(46, 210, 460, $vnc->_framebuffer->yres - 210);
        $roi->chat_ocr;
        $vnc->_framebuffer->write("chats/chat-" . time . ".png");
        $vnc->mouse_click(530, 295);
        park_cursor;
        return 1;
    }
    else {
        #$roi->write("chat-close-" . time . ".png");
    }
    return;
}

sub fix_main_screen {
    if (on_main_screen) {
        zoom_out;
    }
    else {
        diag "OBSTACLE in the way!";
        return if check_chat_close;
        my ($sim, $xmatch, $ymatch) = find_needle_coords('other-device.png');
        if ($sim > 30) {
            diag "waiting for other device";
            sleep(12);
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
        for my $rx (qw/red-X.png red-X2.png end-fight.png home.png okay.png gohome.png/) {
            ($sim, $xmatch, $ymatch) = find_needle_coords($rx);
            if ($sim > 15) {
                $vnc->mouse_click($xmatch + 5, $ymatch + 5);
                park_cursor;
                return fix_main_screen();
            }
        }
        ($sim, $xmatch, $ymatch) = find_needle_coords('pbt.png');
        if ($sim > 25) {
            $botapi->sendMessage(
                {
                    chat_id => $bot_chatid,
                    text    => 'Personal Break'
                });
            diag "Personal Break - waiting a minute";
            sleep(60);
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
        diag "unknown obstacle";
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
            print "Resource $n $sim\n";
            if ($sim > 14) {
                $found = 1;
                diag "FOUND $n";
                $vnc->mouse_click($xm + 10, $ym + 10);
                park_cursor(1);
                sleep .1;
                update_screen;
            }
        }
        if ($found) {
            $done = 1;
        }
        else {
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
    my $nn  = read_png('armyoverviewTL.png');
    my $img = $vnc->_framebuffer;

    $vnc->mouse_click(260, 694);
    wait_for_screen('troops-label.png', 245, 84, 8) || return;

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
    $vnc->mouse_click($coords[$b - 1] + 20, 694);
    park_cursor;
    my $sb    = read_png('selected-barrack.png');
    my $sb2   = read_png('selected-barrack-boost.png');
    my $stime = time;
    while (time - $stime < 5) {
        update_screen;
        my $sim  = $vnc->_framebuffer->copyrect($coords[$b - 1], 644, $sb->xres, $sb->yres)->similarity($sb);
        my $sim2 = $vnc->_framebuffer->copyrect($coords[$b - 1], 644, $sb->xres, $sb->yres)->similarity($sb2);
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
    my $img   = $vnc->_framebuffer;
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

    my %rooms = (
        giant       => 5,
        archer      => 1,
        barb        => 1,
        wallbreaker => 2,
        vali        => 8,
        minion      => 2,
        hog         => 5,
        golem       => 30,
        witch       => 12,
        lava        => 30,
        cobold      => 1,
        balloon     => 5,
        wizard      => 4,
        healer      => 14,
        dragon      => 20,
        pekka       => 25
    );

    die "no room for $t" unless $rooms{$t};
    return $rooms{$t};
}

sub time_for_troop {
    my ($t) = @_;

    my %times = (
        giant       => 2 * 60,
        archer      => 20,
        barb        => 20,
        wallbreaker => 2 * 60,
        vali        => 8 * 60,
        minion      => 45,
        hog         => 2 * 60,
        golem       => 45 * 60,
        witch       => 20 * 60,
        lava        => 45 * 60,
        cobold      => 30,
        balloon     => 8 * 60,
        wizard      => 8 * 60,
        healer      => 15 * 60,
        dragon      => 30 * 60,
        pekka       => 45 * 60
    );

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
    }
    elsif ($t eq 'archer') {
        $vnc->mouse_click(526, 405);
    }
    elsif ($t eq 'giant') {
        $vnc->mouse_click(690, 405);
    }
    elsif ($t eq 'cobold') {
        $vnc->mouse_click(845, 405);
    }
    elsif ($t eq 'wallbreaker') {
        $vnc->mouse_click(1012, 405);
    }
    elsif ($t eq 'balloon') {
        $vnc->mouse_click(355, 560);
    }
    elsif ($t eq 'wizard') {
        $vnc->mouse_click(526, 560);
    }
    elsif ($t eq 'healer') {
        $vnc->mouse_click(690, 560);
    }
    elsif ($t eq 'dragon') {
        $vnc->mouse_click(845, 560);
    }
    elsif ($t eq 'pekka') {
        $vnc->mouse_click(1012, 560);
    }
    else {
        die "no coordinates for $t";
    }
    sleep 0.05;
    #update_screen;
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
        select_barrack($i + 1);
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
    diag "ARMY";
    diag Dumper($army);
    diag "BUILDING";
    diag Dumper($building);
    my $soll = {
        giant       => 20,
        wallbreaker => 8
    };
    my $rest = 220;
    if ($total > 215) {
        $rest *= 2;
    }
    for my $t (keys %$soll) {
        my $c = ($building->{$t} || 0) + ($army->{$t} || 0);
        $soll->{$t} *= 2 if ($total > 215);
        if ($soll->{$t} < $c) {
            $soll->{$t} = $c;
        }
        $rest -= $soll->{$t} * room_for_troop($t);
    }
    diag "TOTAL $total REST $rest";
    # align to 4
    $soll->{barb} = int(int($rest * 0 / 100) / 4 + 0.5) * 4;
    $soll->{barb} = 4;
    $rest -= $soll->{barb} * room_for_troop('barb');
    $soll->{archer} = int($rest / room_for_troop('archer'));
    my $diff;
    diag Dumper($soll);
    for my $t (keys %$soll) {
        $diff->{$t} = $soll->{$t} - ($army->{$t} || 0) - ($building->{$t} || 0);
        $diff->{$t} = 0 if ($diff->{$t} < 0);
    }
    diag "DIFF";
    diag Dumper($diff);

    my @newbuilds;

    while (%$diff) {
        my @troops = sort { time_for_troop($b) <=> time_for_troop($a) } keys %$diff;
        my $t = shift @troops;
        if ($diff->{$t} <= 0) {
            delete $diff->{$t};
            next;
        }
        diag "FIND spot for $t - $diff->{$t}";
        $min_train_time = time_for_troops($barracks[0]);
        my $min = 0;
        for (my $i = 1; $i < 4; $i++) {
            my $time = time_for_troops($barracks[$i]);
            if ($time < $min_train_time) {
                $min            = $i;
                $min_train_time = $time;
            }
        }
        diag "FOUND $min";
        $newbuilds[$min]->{$t} ||= 0;
        $newbuilds[$min]->{$t} += 1;
        $barracks[$min]->{$t}  += 1;
        $diff->{$t} -= 1;
    }
    diag "BARCKS";
    diag Dumper($newbuilds[0]);
    diag Dumper($newbuilds[1]);
    diag Dumper($newbuilds[2]);
    diag Dumper($newbuilds[3]);
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
        select_barrack($i + 1);
        select_troops($newbuilds[$i]);
        update_screen;
    }
    $vnc->mouse_click(35, 540);
    park_cursor(1);
    return $total > 215;
}

sub wait_for_screen {
    my ($fn, $x, $y, $timeout) = @_;
    $timeout ||= 5;
    my $endtime = time + $timeout;
    my $nn      = read_png($fn);
    while (time < $endtime) {
        my $sim = $vnc->_framebuffer->copyrect($x, $y, $nn->xres, $nn->yres)->similarity($nn);
        diag "Wait for $fn $sim";
        return 1 if ($sim > 22);
        update_screen;
    }
    $vnc->_framebuffer->write("last.png");
    return;
}

sub check_base_resources {
    my $nn   = read_png('base-gold.png');
    my $gold = 0;
    my $elex = 0;
    my $de   = 0;

    my $sim = $vnc->_framebuffer->copyrect(33, 96, $nn->xres, $nn->yres)->similarity($nn);
    if ($sim > 23) {
        $gold = $vnc->_framebuffer->copyrect(66, 95, 110, 30)->base_count('chars_base_count');
    }
    $nn = read_png('base-elex.png');
    $sim = $vnc->_framebuffer->copyrect(32, 136, $nn->xres, $nn->yres)->similarity($nn);
    if ($sim > 23) {
        $elex = $vnc->_framebuffer->copyrect(66, 135, 110, 30)->base_count('chars_base_count');
    }
    $nn = read_png('base-de.png');
    $sim = $vnc->_framebuffer->copyrect(31, 176, $nn->xres, $nn->yres)->similarity($nn);
    if ($sim > 23) {
        $de = $vnc->_framebuffer->copyrect(66, 176, 110, 30)->base_count('chars_base_count');
    }
    diag "BASE $gold gold $elex elex $de DE";
    # not worth the TH check
    return if ($gold + $elex < 50000 || $de < 100);
    for my $th (glob("ths/*-th-*.png")) {
        my ($sim, $xmatch, $ymatch) = find_needle_coords($th, 1);
        if ($sim > 14) {
            return ($1, $gold, $elex, $de) if $th =~ /.*-th-(\d*).png/;
        }
    }
    diag "UNKNOWN TH";
    return 20;
}

sub worth_it {
    my ($th, $gold, $elex, $de, $count) = @_;
    #return      if ($th < 8);
    if ($th == 8) {
        return ($gold + $elex + $de * 100 > 420000 - $count * 1000);
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
        #$vnc->_framebuffer->copyrect($spot + 11, 659, 4, 4)->write("img-$spot.png");
        my ($r, $g, $b) = $vnc->_framebuffer->copyrect($spot + 9, 658, 4, 4)->avgcolor;
        # ignore everything not really blue
        next if ($b < .5);
        # NOT YET: vali golem witch lava balloon pekka dragon healer
        for my $t (@troops) {
            my $ref = read_png("attack/$t.png") || next;
            my $sim = $vnc->_framebuffer->copyrect($spot + 9, 658, 91, 52)->similarity($ref);
            if ($sim > 15) {
                my $c = $vnc->_framebuffer->copyrect($spot + 4, 629, 88, 27);
                if ($t eq 'CB' || $t eq 'king' || $t eq 'queen') {
                    $c = 1;
                }
                else {
                    $c = $c->troop_count('chars_barrack_count');
                }
                push(@$res, [$t, $spot, $c]);
                $spot += 100;
                while (1) {
                    my $f = shift @troops;
                    if ($f =~ /spell-/) {
                        unshift @troops, $f;
                        last;
                    }
                    last unless $f;
                    last if $f eq $t;
                }
                last;
            }
        }
    }
    return $res;
}

sub spots_on_red_line {
    my ($x1, $y1, $x2, $y2, $count) = @_;
    my $maxcount = $count > 20 ? 20 : $count;
    $count -= $maxcount;
    diag "select $maxcount between $x1+$y1 and $x2+$y2";
    my $delta = ($x2 - $x1) / ($maxcount + 1);
    $delta = 1 if ($delta < 1);
    my $dX = ($x2 - $x1) / ($y2 - $y1);
    diag sprintf("delta $delta - from %f to %f", $x1 + $delta, $x2 - $delta);
    my $x = $x1 + $delta;
    for (; $maxcount > 0; $maxcount--) {
        my $xi = int($x + .5);
        my $y  = int(($x - $x1) / $dX + $y1 + 0.5);
        $vnc->mouse_click($xi, $y);
        sleep 0.03;
        $x += $delta;
    }
    diag "done with clicking";
    update_screen;
    if ($count > 0) {
        return spots_on_red_line($x1, $y1, $x2, $y2, $count);
    }
    return;
}

sub select_attack_troop {
    my ($troops, $kind) = @_;

    for my $ti (@$troops) {
        my ($t, $spot, $count) = @$ti;
        if ($t eq $kind) {
            diag "selecting $kind - at $count now";
            $vnc->mouse_click($spot + 20, 670);
            update_screen;
            if ($t eq 'king' || $t eq 'queen') {
                return $spot;
            }
            else {
                return $count;
            }
        }
    }
    return 0;
}

our $last_check_time;

sub _sleep {
    my ($time) = @_;

    # first time
    my $diff = $last_check_time + $time - time;
    $last_check_time = time + $time;
    diag "_sleep $time -> $diff s";
    return if $diff <= 0;
    sleep($diff);
    return;
}

sub attack {
    my ($x1, $y1, $x2, $y2) = $vnc->_framebuffer->find_red_line;
    diag "RES $x1+$y1 - $x2+$y2";
    #system("aplay /usr/share/xemacs/xemacs-packages/etc/sounds/long-beep.wav");

    # first we select the last spot, so we have reliable base troops
    my $troops = find_attack_troops;
    my $last_spot;
    for my $ti (@$troops) {
        $last_spot = $ti->[1];
    }
    $vnc->mouse_click($last_spot + 20, 670);
    update_screen;
    $troops = find_attack_troops;
    diag Dumper($troops);

    $last_check_time = time;

    # GIANT 1
    my $giants = select_attack_troop($troops, 'giant');
    my $giant_wave = int($giants / 2);
    spots_on_red_line($x1, $y1, $x2, $y2, $giant_wave);

    _sleep(.3);

    # WB 1
    my $wbs = select_attack_troop($troops, 'wallbreaker');
    my $wb_wave = int($wbs / 4);
    spots_on_red_line($x1, $y1, $x2, $y2, $wb_wave);

    _sleep(0.3);

    # ARCHER 1
    my $archers = select_attack_troop($troops, 'archer');
    my $archer_wave = int($archers / 4);
    spots_on_red_line($x1, $y1, $x2, $y2, $archer_wave);

    _sleep(1);

    my $cb = select_attack_troop($troops, 'CB');
    if ($cb) {
        spots_on_red_line($x1, $y1, $x2, $y2, 1);
        _sleep(.5);
    }

    # WB 2
    select_attack_troop($troops, 'wallbreaker');
    spots_on_red_line($x1, $y1, $x2, $y2, $wb_wave);

    _sleep(.3);

    # ARCHER 2
    select_attack_troop($troops, 'archer');
    spots_on_red_line($x1, $y1, $x2, $y2, $archer_wave);

    sleep(1);

    # GIANT 2
    select_attack_troop($troops, 'giant');
    spots_on_red_line($x1, $y1, $x2, $y2, $giants - $giant_wave);

    _sleep(.7);

    # WB 2
    select_attack_troop($troops, 'wallbreaker');
    spots_on_red_line($x1, $y1, $x2, $y2, $wb_wave);

    _sleep(.3);

    # ARCHER 2
    select_attack_troop($troops, 'archer');
    spots_on_red_line($x1, $y1, $x2, $y2, $archer_wave);

    _sleep(1);

    my $barb_wave = 0;
    my $barbs     = 0;
    if (0) {
        # BARB 1
        $barbs = select_attack_troop($troops, 'barb');
        $barb_wave = int($barbs / 3);
        select_attack_troop($troops, 'barb');
        spots_on_red_line($x1, $y1, $x2, $y2, $barb_wave);

        _sleep(.3);
    }

    # WB 3
    select_attack_troop($troops, 'wallbreaker');
    spots_on_red_line($x1, $y1, $x2, $y2, $wb_wave);

    _sleep(2);


    _sleep(1);

    # WB 4
    select_attack_troop($troops, 'wallbreaker');
    spots_on_red_line($x1, $y1, $x2, $y2, $wbs - 3 * $wb_wave);

    _sleep(.3);

    if (0) {
        # BARB 2
        select_attack_troop($troops, 'barb');
        spots_on_red_line($x1, $y1, $x2, $y2, $barb_wave);

        _sleep(5);
    }

    # HEROES
    my $king = select_attack_troop($troops, 'king');
    if ($king) {
        spots_on_red_line($x1, $y1, $x2, $y2, 1);
    }
    my $queen = select_attack_troop($troops, 'queen');
    if ($queen) {
        spots_on_red_line($x1, $y1, $x2, $y2, 1);
    }

    # time for the last wave
    $troops = find_attack_troops;

    if ($king || $queen) {
        _sleep(7);
    }

    if (0) {
        # BARB 3
        $barbs = select_attack_troop($troops, 'barb');
        spots_on_red_line($x1, $y1, $x2, $y2, $barbs);

        _sleep(1);
    }

    # ARCHER 3
    $archers = select_attack_troop($troops, 'archer');
    spots_on_red_line($x1, $y1, $x2, $y2, $archers);

    if ($king || $queen) {
        _sleep(10);
    }

    if ($king) {
        $vnc->mouse_click($king + 20, 670);
    }
    if ($queen) {
        $vnc->mouse_click($king + 20, 670);
    }

    while (1) {
        $troops = find_attack_troops;
        my $drop;
        for my $ti (@$troops) {
            my ($t, $spot, $count) = @$ti;
            if ($t ne 'spell-heal') {
                $drop = 1;
                $vnc->mouse_click($spot + 20, 670);
                sleep .05;
                update_screen;
                if ($t =~ /spell/) {
                    $vnc->mouse_click(700, 370);
                    next;
                }
                spots_on_red_line($x1, $y1, $x2, $y2, $count);
            }
        }
        last unless $drop;
    }
    diag "waiting for home";

    for (my $i = 0; $i < 200; $i++) {
        update_screen;
        my ($sim, $xmatch, $ymatch) = find_needle_coords('home.png', 0);
        if ($sim > 30) {
            # wait for the resources to show up
            sleep 5;
            update_screen;
            $vnc->_framebuffer->write('telegram.png');
            $botapi->sendPhoto(
                {
                    chat_id => $bot_chatid,
                    photo   => {file => "telegram.png"},
                    caption => "took $i seconds"
                });
            $vnc->mouse_click($xmatch + 30, $ymatch + 30);
            return;
        }
    }
}

sub find_worthy_base {
    fix_main_screen;
    return if !on_main_screen;
    $vnc->mouse_click(60, 650);
    # there are 2 different places - with or without shield
    if (wait_for_screen('find-fight.png', 207, 572, 8)) {
        $vnc->mouse_click(220, 590);
    }
    else {
        find_needle_coords('find-fight.png');
        wait_for_screen('find-fight.png', 207, 521, 8) || die "no find-fight";
        $vnc->mouse_click(220, 540);
    }
    my $time_to_next = time;
    my $next         = read_png('next.png');
    my $bases_seen   = 0;
    while (1) {
        update_screen;
        my $sim = $vnc->_framebuffer->copyrect(1118, 503, $next->xres, $next->yres)->similarity($next);
        if ($sim > 30) {
            $bases_seen++;
            my $bfn = "bases/base-" . time . ".png";
            diag "BASE $bfn\n";
            #$vnc->_framebuffer->write($bfn);
            my ($th, $gold, $elex, $de) = check_base_resources;
            if ($th && worth_it($th, $gold, $elex, $de, $bases_seen)) {
                $botapi->sendMessage(
                    {
                        chat_id => $bot_chatid,
                        text    => "Found worthy base $th $gold $elex $de"
                    });
                return 1;
            }
            $time_to_next = time;
            $vnc->mouse_click(1250, 550);
            sleep .3;
            park_cursor;
            next;
        }
        if (time - $time_to_next < 8) {
            diag "TIME " . (time - $time_to_next) . "\n";
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
        if (time - $time_to_next < 25) {
            die "failed to find the next base in 25 seconds";
        }
    }
    return;
}

for my $base (glob("bases/base-*.png")) {
    diag "BASE $base";
    $vnc->_framebuffer(tinycv::read($base));

    print Dumper(find_attack_troops);

    my $found;
    for my $th (glob("ths/*.png")) {
        my ($sim, $xmatch, $ymatch) = find_needle_coords($th, 1);
        diag "$th $sim\n";
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
update_screen;

while (1) {
    fix_main_screen;
    while (on_main_screen) {
        update_screen;
        #next if check_chat;
        $min_train_time = 1;
        if (train_troops) {
            next unless find_worthy_base;
            attack;
            next;
        }
        $min_train_time = 120 if ($min_train_time > 120);
        $min_train_time = 10  if ($min_train_time < 10);
        diag "nothing los - waiting $min_train_time seconds";
        sleep($min_train_time);
        $min_train_time = 0;
        fix_main_screen;
        collect_resources;
    }
}

