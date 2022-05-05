#!/usr/bin/perl

use Modern::Perl '2014';
use feature ':5.18';
no warnings 'experimental::smartmatch';
use Getopt::Long;
use String::LCSS;
use List::Util qw(min max);

# TODO
#
#

my $pinGraph = {};
my $pinByName = {};


my %ospeedName = (
    '2M' => 'VERYLOW',
    '25M' => 'LOW',
    '50M' => 'MEDIUM',
    '100M' => 'HIGH'
    );

my $nameMaxLen = 0;
my $fhw;
my $mcuCore;
my $mcuFamily;

sub usage();
sub parseHeader($);
sub writeHeader($);
sub generateCfg ();

my %options;
GetOptions (\%options, 'kup', 'help');
usage() if exists  $options{help};

#         __  __    ___    _____   _   _
#        |  \/  |  / _ \  |_   _| | \ | |
#        | \  / | | |_| |   | |   |  \| |
#        | |\/| | |  _  |   | |   | . ` |
#        | |  | | | | | |  _| |_  | |\  |
#        |_|  |_| |_| |_| |_____| |_| \_|

usage()  unless scalar (@ARGV == 2) ;
my ($headerFileName, $configFileName) = @ARGV;
unless (-r $headerFileName) {
    warn "$headerFileName not existant or not readable\n";
    usage();
}

open($fhw, ">", $configFileName) or die "cannot open > $configFileName: $!";

my @c_header = parseHeader($headerFileName);
writeHeader (1);
syswrite ($fhw, join ('', @c_header));
writeHeader (2);
generateCfg ();

close ($fhw);

#         _ __
#        | '_ \
#        | |_) |   __ _   _ __   ___     ___
#        | .__/   / _` | | '__| / __|   / _ \
#        | |     | (_| | | |    \__ \  |  __/
#        |_|      \__,_| |_|    |___/   \___|
#         _    _                      _
#        | |  | |                    | |
#        | |__| |   ___    __ _    __| |    ___   _ __
#        |  __  |  / _ \  / _` |  / _` |   / _ \ | '__|
#        | |  | | |  __/ | (_| | | (_| |  |  __/ | |
#        |_|  |_|  \___|  \__,_|  \__,_|   \___| |_|
sub parseHeader($)
{
    my $headerFileName = shift;
    open(my $fh, "<", $headerFileName) or die "cannot open < $headerFileName: $!";
    my $hasMatched = 0;
    my @headerLines;
    

    while (my $l= <$fh>) {
	chomp $l;
	$l =~ s/\r//;
	given ($l) {
	    when (/\#define\s+GPIO([A-Q])_(\w+)\s+(\d+)/) {
		my ($port, $pinName, $pinIndex) = ($1, $2, $3);
#		say "$port, $pinName, $pinIndex";
		my $completeName = "GPIO" . $port . '_' . $pinName;
		$pinGraph->{$port}->[$pinIndex] //= {};
		$pinGraph->{$port}->[$pinIndex]->{'NAME'} = $pinName;
		$pinByName->{$completeName} = $pinGraph->{$port}->[$pinIndex];
		$nameMaxLen = max ($nameMaxLen, length ($pinName));
		$hasMatched = 1;
	    }


	    when (/\#define\s+P([A-Q])(\d+)_(\w+)/) {
		my ($port, $pinIndex, $pinName) = ($1, $2, $3);
#		say "$port, $pinName, $pinIndex";
		my $completeName = "GPIO" . $port . '_' . $pinName;
		$pinGraph->{$port}->[$pinIndex] //= {};
		$pinGraph->{$port}->[$pinIndex]->{'NAME'} = $pinName;
		$pinByName->{$completeName} = $pinGraph->{$port}->[$pinIndex];
		$nameMaxLen = max ($nameMaxLen, length ($pinName));
		$hasMatched = 1;
	    }


	    when (/\#define\s+STM32([FHL]\d)(\w+)/) {
		$mcuCore = $1;
		$mcuFamily = $1.$2;
		$mcuFamily =~ s/xx//;
	    }
	    
	    when (/PIN_MODE_([A-Z]+)\((GPIO\w*)\)/) {
		warn "not registered PIN $2\n" unless exists $pinByName->{$2};
		$pinByName->{$2}->{MODE} = $1;
#		say "MODE $2 is $1";
	    }

	    when (/PIN_OTYPE_([A-Z]+)\((GPIO\w*)\)/) {
		warn "not registered PIN $2\n" unless exists $pinByName->{$2};
		$pinByName->{$2}->{OTYPE} = $1;
#		say "$2 is $1";
	    }

	    when (/PIN_OSPEED_([A-Z]+)\((GPIO\w*)\)/) {
		warn "not registered PIN $2\n" unless exists $pinByName->{$2};
		$pinByName->{$2}->{SPEED} = $1;
#		say "$2 is $1";
	    }

	    when (/PIN_OSPEED_(\d+M)\((GPIO\w*)\)/) {
		warn "not registered PIN $2\n" unless exists $pinByName->{$2};
		$pinByName->{$2}->{SPEED} = $ospeedName{$1};
#		say "$2 is $1";
	    }

	    when (/PIN_PUPDR_([A-Z]+)\((GPIO\w*)\)/) {
		warn "not registered PIN $2\n" unless exists $pinByName->{$2};
		$pinByName->{$2}->{PUPDR} = $1;
#		say "$2 is $1";
	    }

	    when (/PIN_ODR_([A-Z]+)\((GPIO\w*)\)/) {
		warn "not registered PIN $2\n" unless exists $pinByName->{$2};
		$pinByName->{$2}->{ODR} = $1;
#		say "$2 is $1";
	    }

	    when (/PIN_AFIO_AF\((GPIO\w*),\s*(\d+).*\)/) {
		warn "not registered PIN $1\n" unless exists $pinByName->{$1};
		$pinByName->{$1}->{AF} = $2;
#		say "$1 is AF($2)";
	    }

	    when (/PIN_LOCKR_([A-Z]+)\((GPIO\w*)\)/) {
		warn "not registered PIN $2\n" unless exists $pinByName->{$2};
		$pinByName->{$2}->{LOCKR} = $1;
#		say "$2 is $1";
	    }

	    when (/PIN_ASCR_([A-Z]+)\((GPIO\w*)\)/) {
		warn "not registered PIN $2\n" unless exists $pinByName->{$2};
		$pinByName->{$2}->{ASCR} = $1;
#		say "$2 is $1";
	    }
	}
	push (@headerLines, "$l\n") unless $hasMatched;
#	syswrite ($fhw, "$l\n") unless $hasMatched;
    }
    close ($fh);
    return @headerLines;
}



#          __ _                                        _
#         / _` |                                      | |
#        | (_| |   ___   _ __     ___   _ __    __ _  | |_     ___
#         \__, |  / _ \ | '_ \   / _ \ | '__|  / _` | | __|   / _ \
#          __/ | |  __/ | | | | |  __/ | |    | (_| | \ |_   |  __/
#         |___/   \___| |_| |_|  \___| |_|     \__,_|  \__|   \___|
#                 __    __ _
#                / _|  / _` |
#          ___  | |_  | (_| |
#         / __| |  _|  \__, |
#        | (__  | |     __/ |
#         \___| |_|    |___/
sub generateCfg ()
{
    foreach my $port (sort keys %{$pinGraph}) {
#	say "\n"; 
	syswrite ($fhw, "\n\n");
	foreach my $pinIndex (0 .. 15) {
	    next unless exists $pinGraph->{$port}->[$pinIndex];
	    my $o = $pinGraph->{$port}->[$pinIndex];
	    my $jpin = sprintf ("%02d", $pinIndex);
	    my $name = $o->{NAME};
	    unless (exists $options{kup}) {
		next if $name =~ /^PIN\d+$/;
	    }
	    $name .= join ('', (' ') x ($nameMaxLen - length ($name)));
	    my $mode = $o->{MODE};
	    die "mode not defined for $name\n" unless defined $mode;
	    $mode .= join ('', (' ') x (9 - length ($mode)));
	    my $otype = $o->{OTYPE};
	    my $speed = 'SPEED_' . $o->{SPEED};
	    my $pupdr = $o->{PUPDR};
	    $pupdr .= join ('', (' ') x (8 - length ($pupdr)));
	    my $odr = 'LEVEL_' . $o->{ODR};
	    my $af = 'AF' . $o->{AF};
	    
	    my $ascr = exists $o->{ASCR} ? "\tASCR_" . $o->{ASCR} : '';
	    my $lockr = exists $o->{LOCKR} ? "\tLOCKR_" . $o->{LOCKR} : '';
	    my $line = "P${port}${jpin}\t${name}\t${mode}\t${otype}\t${speed}\t${pupdr}\t${odr}\t${af}${ascr}${lockr}\n";
#	    print $line;
	    syswrite ($fhw, $line);
	}
    }
}   

#                           _    _
#                          (_)  | |
#        __      __  _ __   _   | |_     ___
#        \ \ /\ / / | '__| | |  | __|   / _ \
#         \ V  V /  | |    | |  \ |_   |  __/
#          \_/\_/   |_|    |_|   \__|   \___|
#         _    _                      _
#        | |  | |                    | |
#        | |__| |   ___    __ _    __| |    ___   _ __
#        |  __  |  / _ \  / _` |  / _` |   / _ \ | '__|
#        | |  | | |  __/ | (_| | | (_| |  |  __/ | |
#        |_|  |_|  \___|  \__,_|  \__,_|   \___| |_|
sub writeHeader($)
{
    my $index = shift;
    if ($index == 1) {
	syswrite ($fhw, <<EOL);
MCU_MODEL = $mcuFamily
CHIBIOS_VERSION = 3.0

HEADER
EOL
    } elsif ($index == 2) {
	syswrite ($fhw, <<EOL);
CONFIG
# PIN NAME PERIPH_TYPE AF_NUMBER or
# PIN NAME FUNCTION PP_or_OPENDRAIN PIN_SPEED PULL_RESISTOR INITIAL_LEVEL AF_NUMBER
# SPEED : SPEED_VERYLOW, SPEED_LOW, SPEED_MEDIUM, SPEED_HIGH
#
# DEFAULT AND SYS
EOL
 }
}


#                                 __ _
#                                / _` |
#         _   _   ___     __ _  | (_| |   ___
#        | | | | / __|   / _` |  \__, |  / _ \
#        | |_| | \__ \  | (_| |   __/ | |  __/
#         \__,_| |___/   \__,_|  |___/   \___|
sub usage ()
{
  $0 =~ s|.*/||;
  my ($x) = ' ' x length($0) ;

  print <<EOL ;
  usage : $0 boardFile.h boardFile.cfg (if boardFile is '-', use stdout)
      $x -kup (keep unused pin) : output a pin line even is pin name is PINn
EOL
exit (-1);
}

