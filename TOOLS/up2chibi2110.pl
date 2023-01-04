#!/usr/bin/perl

#emacsclient cfg/mcuconf.h && /home/alex/DEV/STM32/CHIBIOS/COMMON/various/TOOLS/up2chibi2110.pl && make && cd .. && ls

use Modern::Perl '2020';
use feature ':5.30';
use Cwd 'abs_path';
use Getopt::Long;
no warnings 'experimental::smartmatch';

use constant  CHIBIOS_ROOT =>
    '/home/alex/DEV/STM32/CHIBIOS/ChibiOS_21.11_stable/';
use constant  CHIBIOS_UPDATER => CHIBIOS_ROOT . "tools/updater/";
use constant UP_CHRT => 'update_chconf_rt.sh';
use constant UP_HAL => 'update_halconf.sh';
use constant MCUCONF => 'cfg/mcuconf.h';

sub updateMakefile();
sub getMcuFamilies();
sub getMcuconfScripts();
sub getMcuconfMatchingScripts();
sub getLocalDefMcuconf($); #mcuconf path, return array of line
sub appendLocalDefMcuconf($\@); #mcuconf path, ref to array local defs

sub updtateXconf($$$); #conf file, updater dir, scriptName

my %options;
GetOptions (\%options, 'hal', 'make', 'ch', 'mcu', 'all');

die ("should specifie -all, or combination of -hal, -ch, -mcu\n")
    unless %options;

$options{make} = $options{hal} = $options{ch} = $options{mcu} = 1
    if exists $options{all};

my $dos2unix = -e '/usr/bin/dos2unix' ? '/usr/bin/dos2unix' : '/bin/dos2unix';
my @mcuconfScripts = getMcuconfMatchingScripts();

if (scalar(@mcuconfScripts) > 1) {
    die(sprintf("more than one mcuconf updater exist : %s\n",
	join(' ,', @mcuconfScripts)));	
} elsif (scalar(@mcuconfScripts) == 0) {
    die "no mcuconf updater found\n";	
}

my $projPath = abs_path('.');
say "project path is $projPath";
say "will use $mcuconfScripts[0]";

updateMakefile()
    if exists $options{make};
my @localDefs = getLocalDefMcuconf(MCUCONF);
updtateXconf("$projPath/cfg/halconf.h", CHIBIOS_UPDATER, UP_HAL)
    if exists $options{hal};
updtateXconf("$projPath/cfg/chconf.h", CHIBIOS_UPDATER, UP_CHRT)
    if exists $options{ch};
updtateXconf("$projPath/cfg/mcuconf.h", CHIBIOS_UPDATER, $mcuconfScripts[0])
    if exists $options{ch};

chdir($projPath);
appendLocalDefMcuconf(MCUCONF, @localDefs)
        if exists $options{ch};


sub getMcuFamilies()
{
    my @matchingMcus;
    
    open(my $fh, "<",  MCUCONF) or
	die sprintf("cannot open < %s: $!\n", MCUCONF);

    while (my $l= <$fh>) {
        chomp $l;
	my ($match) = $l =~ m/STM32(\w{4})_MCUCONF/ ;
	push(@matchingMcus, $match) if defined $match;
    }

    close($fh);
    
#    say('DBG1> ' . join(' ,', @matchingMcus));
    return @matchingMcus;
}

sub getMcuconfScripts()
{
    opendir(my $dh, CHIBIOS_UPDATER) ||
	die(sprintf("%s directory not found\n", CHIBIOS_UPDATER));
    my @updateMcuconf = grep(/update_mcuconf_stm32/, readdir($dh));
    closedir($dh);
#    say('DBG2> ' . join(' ,', @updateMcuconf));
    return @updateMcuconf;
}

sub  getMcuconfMatchingScripts()
{
    my @families = getMcuFamilies();
#    say('DBG7> ' . join(' ,', @families));
    my @scripts = getMcuconfScripts();
    my %matchingScripts;

    foreach my $s (@scripts) {
	foreach my $f (@families) {
	    $f =~ s/F767/F76X/;
	    $matchingScripts{$s} = 1 if $s =~ /$f/i;
	}
    }
    return keys %matchingScripts;
}

sub updtateXconf($$$) #conf file, updater dir, scriptName
{
    my ($confFile, $updaterDir, $scriptName) = @_;
    die "file $confFile not found" unless -e $confFile;
    die "directory $updaterDir not found" unless -d $updaterDir;

    chdir $updaterDir;
    die "file $scriptName not found" unless -e $scriptName;
    system('/bin/cp', $confFile, '/tmp');
    my @com = ('/bin/bash', $scriptName, $confFile);
#    say(sprintf("DBG4 > %s", join(', ', @com)));
    system(@com);
}

sub getLocalDefMcuconf($) #mcuconf path, return array of line
{
    my $mcuconf = shift;
    my @defs;

    die "file $mcuconf not found" unless -e $mcuconf;
    open(my $fh, "<",  $mcuconf) or
	die sprintf("cannot open < %s: $!\n", $mcuconf);

    my $markFound = 0;
    while (my $l= <$fh>) {
	$markFound = 1 if $l =~ /\/\/\s*local defs/;
	push(@defs, $l) if $markFound;
    }
    close($fh);
#    say(sprintf("DBG5 > %s\n", join(', ', @defs)));
    return @defs;
}


sub appendLocalDefMcuconf($\@) #mcuconf path, return array of line
{
    my ($mcuconf, $defsRef) = @_;
    my @finalContent;    

    die "file $mcuconf not found" unless -e $mcuconf;
    system($dos2unix, $mcuconf);
    open(my $fh, "<",  $mcuconf) or
	die sprintf("cannot open < %s: $!\n", $mcuconf);


    while (my $l= <$fh>) {
	push(@finalContent, $l) unless $l =~ /#endif.*MCUCONF_H/;
    }
    close($fh);
    push(@finalContent, @$defsRef);
    
    open($fh, ">",  $mcuconf) or
	die sprintf("cannot open > %s: $!\n", $mcuconf);
    $fh->print(@finalContent);
    close($fh);
    system($dos2unix, $mcuconf);
}

sub updateMakefile()
{
    my $command = 'perl -i.bak -ape \'s|MY_DIRNAME=\s*([/.]*).*|MY_DIRNAME=${1}ChibiOS_21.11_stable|\' Makefile';
    system $command;
    $command = 'perl -i.bak -ape \'s|^CHIBIOS\s+:=.*|CHIBIOS  := \$(RELATIVE)/\$(notdir \$(MY_DIRNAME))|\' Makefile';
    system $command;
}
