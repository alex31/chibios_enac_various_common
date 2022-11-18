#!/usr/bin/perl


use Modern::Perl '2020';
use feature ':5.30';
no warnings 'experimental::smartmatch';

use Udev::FFI;
use Udev::FFI::Devnum qw(:all); # <- import major, minor and makedev
use Getopt::Long;

use constant NONE => 0;
use constant STLINK => 1;
use constant BMP => 2;

sub usage();

my %options;
my $flashTool = NONE;

GetOptions (\%options, 'stlink', 'bmp', 'ftdi', 'help');
usage() if exists  $options{help};
usage()  unless scalar (@ARGV == 1) or exists $options{ftdi};

my $programFile = $ARGV[0] // '';
die "$programFile not readable\n" unless -r $programFile or exists $options{ftdi};

unless (exists $options{ftdi}) {
    for ($programFile) {
	when(/.bin$/) {
	    $flashTool = STLINK;
	}
	when(/.elf$/) {
	    $flashTool = BMP;
	}
    }
}

unless (exists $options{ftdi}) {
    die "$programFile is of unknow file type (neither .bin or .elf)\n" unless $flashTool != NONE;
}

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
unless ($monitor->filter_by_subsystem_devtype('tty')) {
    warn "Ouch!";
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
	    next if ($devIndex % 2) == 0;
	    
	    say sprintf ("%s %s", $action, $device->get_devnode());
	    
	    next unless ($action eq 'add');
	    my @com;
	    
	    for ($flashTool) {
		when (STLINK) {
		    @com = ('/bin/st-flash', 'write', $programFile, '0x08000000');
		}
		when (BMP) {
		    if (-x "$ENV{HOME}/BIN/bmpflash") {
			@com = ("$ENV{HOME}/BIN/bmpflash", $programFile);
		    } elsif (-x "/usr/local/bin/bmpflash") {
			@com = ("/usr/local/bin/bmpflash", $programFile);
		    } else {
			die "bmpflash not found\n";
		    }
		}
	    }
	    
	    say "Device detected, will flash : " . join(', ', @com);
	    sleep 1;
	    system(@com);
	    system('/bin/paplay',
		   '/usr/share/sounds/Oxygen-Sys-App-Positive.ogg')
		if -x '/bin/paplay';
	    say "flash done, you can unplug, hit ctrl-C when finish";
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
  usage : $0 [-stlink, -bmp, -ftdi] file.bin or file.elf
        :    if file.bin, use st-flash, if file.elf, use black magic probe (bmpflash)
EOL
  exit (-1);
}
