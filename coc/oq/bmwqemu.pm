package bmwqemu;

use base qw/Exporter/;
our @EXPORT = qw/diag/;

use Time::HiRes qw(sleep gettimeofday time);

our $first_time = time;

sub diag {
    my ($args) = @_;
    chomp $args;
    printf "%.2f: %s\n", time - $first_time, $args;
}

1;
