use strict;
use warnings;

BEGIN {
    unshift(@INC, "ppmclibs/blib/arch");
    unshift(@INC, "ppmclibs/blib/lib");
}
use consoles::VNC;
use IO::Select;

my $vnc = consoles::VNC->new(
    {
        hostname => $ARGV[0],
        depth    => 8,
        port     => $ARGV[1] || 5900
    });

sub update_screen {
    $vnc->send_update_request;
    my $s = IO::Select->new();
    $s->add($vnc->socket);

    for (my $i = 0; $i < 10; $i++) {
        if ($s->can_read(.2)) {
            if (!$vnc->update_framebuffer) {
                $vnc->send_update_request;
                next;
            }
            $vnc->_framebuffer->write("last.png");
            last;
        }
    }
}

$vnc->login;

while (1) {
	 update_screen;
 }
	
