#!/usr/bin/perl


use Modern::Perl '2020';
use feature ':5.30';


use Udev::FFI;
use Udev::FFI::Devnum qw(:all); # <- import major, minor and makedev
use Getopt::Long;
use Syntax::Keyword::Match;

use constant NONE => 0;
use constant STLINK => 1;
use constant BMP => 2;
use constant DFU => 3;

sub usage();

my %options;
my $flashTool = NONE;

GetOptions (\%options, 'dfu', 'stlink', 'bmp', 'ftdi', 'help');
usage() if exists  $options{help};
usage()  unless scalar (@ARGV == 1) or exists $options{ftdi};

my $programFile = $ARGV[0] // '';
die "$programFile not readable\n" unless -r $programFile or exists $options{ftdi};

unless (exists $options{ftdi}) {
    match ($programFile : =~) {
	case(/.bin$/) {
	    $flashTool = STLINK;
	}
	case(/.elf$/) {
	    $flashTool = BMP;
	}
    }
}

unless (exists $options{ftdi}) {
    die "$programFile is of unknow file type (neither .bin or .elf)\n" unless $flashTool != NONE;
}

$flashTool = DFU if exists $options{dfu};

# get udev library version
my $udev_version = Udev::FFI::udev_version() or
    die "Can't get udev library version: $@";
 
 
# create Udev::FFI object
my $udev = Udev::FFI->new() or
    die "Can't create Udev::FFI object: $@";
 
 
# create udev monitor
my $monitor = $udev->new_monitor() or
    die "Can't create udev monitor: $@";
 
# add filter to monitor
if (exists $options{dfu}) {
    unless ($monitor->filter_by_subsystem_devtype('usb', 'usb_device')) {
	warn "Ouch!";
    }
} else {
    unless ($monitor->filter_by_subsystem_devtype('tty')) {
	warn "Ouch!";
    }
}
 
# start monitor
unless (exists $options{ftdi}) {
    if ($monitor->start()) {
	for (;;) {
	    # poll devices, now insert or remove your block device
	    my $device = $monitor->poll(); # blocking read
	    my $action = $device->get_action();
	    my $devName = $device->get_sysname();
	    
	    my ($devIndex) = $devName =~ m/(\d+)$/;

	    next if ($devIndex % 2) == 0 && exists $options{bmp};
	    say sprintf ("%s %s", $action, $device->get_devnode());
	    
	    next unless ($action eq 'add');
	    my @com;
	    
	    match ($flashTool : ==) {
		case (STLINK) {
		    @com = ('/bin/st-flash', 'write', $programFile, '0x08000000');
		}
		case (BMP) {
		    if (-x "$ENV{HOME}/BIN/bmpflash") {
			@com = ("$ENV{HOME}/BIN/bmpflash", $programFile);
		    } elsif (-x "/usr/local/bin/bmpflash") {
			@com = ("/usr/local/bin/bmpflash", $programFile);
		    } else {
			die "bmpflash not found\n";
		    }
		}
		case (DFU) {
		    my $dfuUtil;
		    $dfuUtil = '/usr/local/bin/dfu-util' if -x '/usr/local/bin/dfu-util';
		    $dfuUtil = '/bin/dfu-util' if -x '/bin/dfu-util';
		    @com = ($dfuUtil, '-d', '1d50:6018', '-c',
			    '1', '-i', '0', '-a', '0', '-s', '0x08002000:leave', '-D',
			    $programFile);
		}
	    }
	    
	    say "Device detected, will flash : " . join(', ', @com);
	    sleep 1;
	    system(@com);
	    say "flash done, you can unplug, hit ctrl-C when finish";
	    system('/bin/paplay',
		   '/usr/share/sounds/Oxygen-Sys-App-Positive.ogg')
		if -x '/bin/paplay';
	}
    }
} else {
    if ($monitor->start()) {
	for (;;) {
	    # poll devices, now insert or remove your block device
	    my $device = $monitor->poll(); # blocking read
	    my $action = $device->get_action();
	    my $devName = $device->get_sysname();
	    
	    say sprintf ("%s %s", $action, $device->get_devnode());
	    
	    next unless ($action eq 'add');
	    my @com = ('/home/alex/DEV/STM32/CHIBIOS/C21DEV/NUCLEOL432/rfbox/tools' .
		       '/ftdi_rfbox_eeprom_configure.pl');
	    
	    say "Device detected, will flash : " . join(', ', @com);
	    system(@com);
	    system('/bin/paplay', '/usr/share/sounds/Oxygen-Sys-App-Positive.ogg');
	    say "flash done, you can unplug, hit ctrl-C when finish";
	}
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
  usage : $0 [-stlink, -dfu, -bmp, -ftdi] file.bin or file.elf
        :    if file.bin, use st-flash or dfu-util, if file.elf, use black magic probe (bmpflash)
EOL
  exit (-1);
}
