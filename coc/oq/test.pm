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
$vnc->send_update_request;
my $s = IO::Select->new();
$s->add($vnc->socket);
while (IO::Select->select($s, $s, undef, 20)) {
    next unless $vnc->update_framebuffer;
    $vnc->send_update_request;
    $vnc->_framebuffer->write("last.png");
    last;
}

my $nimg = tinycv::read('spiel-neu-laden.png');
if ($vnc->_framebuffer->copyrect(556, 486, $nimg->xres, $nimg->yres)->similarity($nimg) > 80) {
    $vnc->mouse_click(556 + $nimg->xres / 2, 486 + $nimg->yres / 2);
}

