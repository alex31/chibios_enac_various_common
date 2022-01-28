#!/usr/bin/perl

use Modern::Perl '2020';
use feature ':5.30';
no warnings 'experimental::smartmatch';
use List::Util qw(min max);

#         _ __                   _
#        | '_ \                 | |
#        | |_) |  _ __    ___   | |_     ___
#        | .__/  | '__|  / _ \  | __|   / _ \
#        | |     | |    | (_) | \ |_   | (_) |
#        |_|     |_|     \___/   \__|   \___/
sub parseApiFile($); #filename
sub parseApiEntry($); #api description as line
sub openSourceFiles($); #source basename
sub codeGenEnum($); #file handle du fichier C (impl)
sub codeGenCmdArray($); #file handle du fichier C (impl)
sub codeGenAllFunctions(); 
sub codeGenPrototype($$); #file handle du header, ref to function entry
sub codeGenFunction($$); #file handle du fichier C (impl), ref to function entry
sub codeGenPolyPrototype($$); #file handle du header, ref to function entry
sub codeGenPolyFunction($$); #file handle du fichier C (impl), ref to function entry
sub codeGenImgBlitPrototype($$); #file handle du header, ref to function entry
sub codeGenImgBlitFunction($$); #file handle du fichier C (impl), ref to function entry
sub codeGenFileCallPrototype($$);
sub codeGenFileCallFunction($$);
sub codeGenFileRunPrototype($$);
sub codeGenFileRunFunction($$);
sub codeGenFileExecPrototype($$);
sub codeGenFileExecFunction($$);
sub codeGenFileWritePrototype($$);
sub codeGenFileWriteFunction($$);
sub getParamOut($); # take entryref, return an array of arrayref of tripplet (type, name, arrayNum)
sub getParamIn($); # take entryref, return an array of arrayref of tripplet (type, name, arrayNum)
sub getProtoParam($$); # take tripplet (type, name, arrayNum) ref, P_IN or P_OUT, return string, 
sub checkForErr();


my %dispatchFnTable = (
    'img_blitComtoDisplay' => [\&codeGenImgBlitPrototype, \&codeGenImgBlitFunction],
    'gfx_poly.*' => [\&codeGenPolyPrototype, \&codeGenPolyFunction],
    'file_callFunction' => [\&codeGenFileCallPrototype, \&codeGenFileCallFunction],
    'file_run' => [\&codeGenFileRunPrototype, \&codeGenFileRunFunction],
    'file_exec' => [\&codeGenFileExecPrototype, \&codeGenFileExecFunction],
    'file_write' => [\&codeGenFileWritePrototype, \&codeGenFileWriteFunction],
    'zz__default__' => [\&codeGenPrototype, \&codeGenFunction]
    );

#          __ _   _            _               _
#         / _` | | |          | |             | |
#        | (_| | | |    ___   | |__     __ _  | |            __   __   __ _   _ __
#         \__, | | |   / _ \  | '_ \   / _` | | |            \ \ / /  / _` | | '__|
#          __/ | | |  | (_) | | |_) | | (_| | | |   ______    \ V /  | (_| | | |
#         |___/  |_|   \___/  |_.__/   \__,_| |_|  |______|    \_/    \__,_| |_|


# list of function entries. A line is an array ref to :
# scalar name, arrayref cmd[3], arrayRef argIn, arrayRef argOut, scalar comment
use constant  F_NAME => 0;
use constant  F_CMD => 1;
use constant  F_ARGIN => 2;
use constant  F_ARGOUT => 3;
use constant  F_COMMENT => 4;
use constant  P_IN => 100;
use constant  P_OUT => 101;
use constant  P_TYPE => 0;
use constant  P_VAR => 1;
use constant  P_NUM => 2;
use constant  P_OPT => 3;
use constant  ST_OPEN => 0;
use constant  ST_CLOSE => 1;
my @functions;



#                             _
#                            (_)
#         _ __ ___     __ _   _    _ __
#        | '_ ` _ \   / _` | | |  | '_ \
#        | | | | | | | (_| | | |  | | | |
#        |_| |_| |_|  \__,_| |_|  |_| |_|
my ($fileName, $sourceBaseName) = @ARGV;
die "api description filename and/or source base name  is/are missing\n" unless scalar(@ARGV) >= 2;
die "cannot open $fileName\n" unless -r $fileName;
my ($headerFh, $implFh) = openSourceFiles($sourceBaseName);
parseApiFile($fileName);
#checkForErr();
#codeGenEnum($implFh);
#codeGenCmdArray($implFh);
codeGenAllFunctions();

close($headerFh);
close($implFh);

system("clang-format --style=Google -i $sourceBaseName.c $sourceBaseName.h");





#                        _
#                       | |
#         ___    _   _  | |__    ___
#        / __|  | | | | | '_ \  / __|
#        \__ \  | |_| | | |_) | \__ \
#        |___/   \__,_| |_.__/  |___/
sub parseApiFile($)
{
    my $fn = shift;
    open(my $fh, "<", $fn) or die "cannot open < $fn: $!";
    
    while (my $l = <$fh>) {
	next if $l =~ /^#/;
	parseApiEntry($l);
    }
    close($fh);
}



sub parseApiEntry($)
{
    my $desc = shift;
    chomp($desc);
#    $desc =~ s/\s//g;
    my ($name, $cmd_gold, $cmd_pic, $cmd_dia, $cmd_pixx, $argIn, $argOut, @comment) = split(/;/, $desc);
    $cmd_gold = "CMD_NOT_IMPL" unless $cmd_gold =~ /0x/;
    $cmd_pic = "CMD_NOT_IMPL" unless $cmd_pic =~ /0x/;
    $cmd_dia = "CMD_NOT_IMPL" unless $cmd_dia =~ /0x/;
    $cmd_pixx = "CMD_NOT_IMPL" unless $cmd_pixx =~ /0x/;
    my @funEntry = ($name,
		    [$cmd_gold, $cmd_pic, $cmd_dia, $cmd_pixx],
		    [split(/,/, $argIn)],
		    [split(/,/, $argOut)],
		    \@comment
	);
    push(@functions, \@funEntry);
}


sub checkForErr()
{
    my (%gold, %pic, %diab, %pixx);
    my @uniqFn=({}, {}, {}, {});
    # look for function that do not return arg but ack
    #    foreach my $fnRef (grep ((scalar(@{$_->[F_ARGOUT]}) == 1), @functions)) {
    # look for all functions
    foreach my $fnRef (@functions) {
	if ($fnRef->[F_CMD]->[0] =~ /0x/) {
	    die "goldelox $fnRef->[F_CMD]->[0] used for $gold{$fnRef->[F_CMD]->[0]} and $fnRef->[F_NAME]\n"
		if exists $gold{$fnRef->[F_CMD]->[0]};
	    $gold{$fnRef->[F_CMD]->[0]} = $fnRef->[F_NAME];
	}
	
	if ($fnRef->[F_CMD]->[1] =~ /0x/) {	
	    die "picaso $fnRef->[F_CMD]->[1] used for $pic{$fnRef->[F_CMD]->[1]} and $fnRef->[F_NAME]\n"
		if exists $pic{$fnRef->[F_CMD]->[1]};
	    $pic{$fnRef->[F_CMD]->[1]} = $fnRef->[F_NAME];
	}
	
	if ($fnRef->[F_CMD]->[2] =~ /0x/) {	
	    die "diablo16 $fnRef->[F_CMD]->[2] used for $diab{$fnRef->[F_CMD]->[2]} and $fnRef->[F_NAME]\n"
		if exists $diab{$fnRef->[F_CMD]->[2]};
	    $diab{$fnRef->[F_CMD]->[2]} = $fnRef->[F_NAME];
	}

	if ($fnRef->[F_CMD]->[3] =~ /0x/) {	
	    die "pixxi $fnRef->[F_CMD]->[3] used for $diab{$fnRef->[F_CMD]->[3]} and $fnRef->[F_NAME]\n"
		if exists $diab{$fnRef->[F_CMD]->[3]};
	    $diab{$fnRef->[F_CMD]->[3]} = $fnRef->[F_NAME];
	}
    }

    # pour chaque architecture trouver les codes de cmd qui ne sont pas présents dans les 2 autres
    foreach my $cmd (keys %gold) {
	say "gold cmd ${gold{$cmd}}[$cmd] not found on picaso/diablo16"
	    if (!exists $pic{$cmd}) and (!exists $diab{$cmd});
	$uniqFn[0]->{${gold{$cmd}}} = 1 unless exists $pic{$cmd} or exists $diab{$cmd};
	#say "DBG> uniqFn[0]->{${gold{$cmd}}} = 1"  unless exists $pic{$cmd} or exists $diab{$cmd};
    }
    say '----------';
     foreach my $cmd (keys %pic) {
	 say "pic cmd ${pic{$cmd}}[$cmd] not found on goldelox/diablo16"
	     if (!exists $gold{$cmd}) and (!exists $diab{$cmd});
	$uniqFn[1]->{${pic{$cmd}}} = 1 unless exists $pic{$cmd} or exists $diab{$cmd};
    }
    say '----------';
     foreach my $cmd (keys %diab) {
	 say "diab cmd ${diab{$cmd}}[$cmd] not found on goldelox/picaso"
	     if (!exists $gold{$cmd}) and (!exists $pic{$cmd});
	$uniqFn[2]->{${diab{$cmd}}} = 1 unless exists $pic{$cmd} or exists $diab{$cmd};
    }
   say '----------';
     foreach my $cmd (keys %pixx) {
	 say "pixx cmd ${pixx{$cmd}}[$cmd] not found on goldelox/picaso"
	     if (!exists $gold{$cmd}) and (!exists $pic{$cmd});
	$uniqFn[2]->{${pixx{$cmd}}} = 1 unless exists $pic{$cmd} or exists $pixx{$cmd};
    }

}

sub openSourceFiles($)
{
    my $bn = shift;
    my $headerName = "${bn}.h";
    my $implName = "${bn}.c";
    open(my $headerFh, ">", $headerName) or die "cannot open > $headerName: $!";
    open(my $implFh, ">", $implName) or die "cannot open > $implName: $!";


    print $headerFh <<EOL;
#pragma once
#include <ch.h>
#include <hal.h>

typedef struct FdsConfig FdsConfig;
EOL

    print $implFh "#include \"$headerName\"\n\n";
    print $implFh <<EOL;
/*
    This code has been generated from API description
    All hand modifications will be lost at next generation 
*/
static bool gfx_polyxxx(const FdsConfig *fdsConfig, uint16_t cmd, uint16_t n, const uint16_t vx[], const uint16_t vy[], uint16_t color)
{
 struct {
  uint16_t cmd;
   uint16_t n;
} command1 = {
   .cmd = cmd,
  .n = __builtin_bswap16(n)
};	

  struct {
  uint8_t ack;
  } response;
  
  fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command1, sizeof(command1),
		     NULL, 0);

  fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) vx, sizeof(vx[0]) * n,
		     NULL, 0);

  fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) vy, sizeof(vy[0]) * n,
		     NULL, 0);

  color = __builtin_bswap16(color);
  return fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &color, sizeof(color),
		     (uint8_t *) &response, sizeof(response)) != 0;
}

   
EOL
    return ($headerFh, $implFh);
}


sub codeGenEnum($)
{
    my $fh = shift;
    my @fnList = map($_->[F_NAME], @functions);
    print $fh "enum apiEntries_t {\n";
    print $fh join(",\n", map("api_$_", @fnList));
    print $fh "\n};\n";

    print $fh "enum qdsType_t {\n";
    print $fh "cmd_goldelox, cmd_picaso, cmd_diablo, cmd_pixxi\n};\n\n";
    print $fh "#define CMD_NOT_IMPL 0xbaba\n";
}

sub codeGenCmdArray($)
{
    my $fh = shift;
    my $nbEntries = scalar(@functions);
    print $fh "static  const uint16_t cmdCodeByType[$nbEntries][4] = {\n";
    foreach my $fnEntryRef (@functions) {
	print $fh "{__builtin_bswap16($fnEntryRef->[F_CMD]->[0]), ".
	          "__builtin_bswap16($fnEntryRef->[F_CMD]->[1]),  " .
	          "__builtin_bswap16($fnEntryRef->[F_CMD]->[2]),  " .
	          "__builtin_bswap16($fnEntryRef->[F_CMD]->[3])},\n"; 
    }
    print $fh "};\n\n";
}


sub codeGenAllFunctions()
{
    my $fh = shift;
    GFX_FUNCTION: foreach my $fnEntryRef (@functions) {
	say $headerFh '';
	#	say $headerFh "// $fnEntryRef->[F_COMMENT]" if $fnEntryRef->[F_COMMENT];
	foreach my $cmmt (@{$fnEntryRef->[F_COMMENT]}) {
	    say $headerFh "// $cmmt" if $cmmt;
	}
	my $goldNotreturn = join(', ', map($_->[P_VAR], grep($_->[P_OPT] =~ /:/,  getParamOut($fnEntryRef))));
	 say $headerFh "// argument(s) $goldNotreturn not returned if screen is of goldelox type" if $goldNotreturn;
	foreach my $filterName (sort keys %dispatchFnTable) {
	    if (($fnEntryRef->[F_NAME] =~ /${filterName}$/) or ($filterName eq 'zz__default__')) {
		my ($protoFn, $implFn) = @{$dispatchFnTable{$filterName}};
		&$protoFn($headerFh, $fnEntryRef);
		say $headerFh ';';
		&$implFn($implFh, $fnEntryRef);
		next GFX_FUNCTION;
	    }
	}
    }
}


sub codeGenFunction($$)
{
    my ($fh, $fnEntryRef) = @_;
    my @paramIn = getParamIn($fnEntryRef);
    my @inP = map(getProtoParam($_, P_IN), @paramIn);
    my @gpo = getParamOut($fnEntryRef);
    my @outP = map(getProtoParam($_, P_IN), @gpo);
    codeGenPrototype($implFh, $fnEntryRef);
    my $check = '';
    if (grep($_ eq 'CMD_NOT_IMPL', @{$fnEntryRef->[F_CMD]})) {
	$check = "osalDbgAssert(command1.cmd != CMD_NOT_IMPL, \"function $fnEntryRef->[F_NAME] unimplemented for this screen\");";
    }
    my $fixSize = join(' - ', map("sizeof(*$_->[P_VAR])", grep($_->[P_OPT] =~ /:/,  @gpo)));
    $fixSize = '0' unless $fixSize;
    my $giveBackString = (scalar(@{$fnEntryRef->[F_ARGOUT]} == 3) and ($fnEntryRef->[F_ARGOUT]->[2] =~ /str\[n\]/));
    #say "DBG> select $fnEntryRef->[F_NAME]" if $giveBackString;
    print $implFh <<EOL;

{
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16($fnEntryRef->[F_CMD]->[0]),
					  __builtin_bswap16($fnEntryRef->[F_CMD]->[1]),
					  __builtin_bswap16($fnEntryRef->[F_CMD]->[2]),
				          __builtin_bswap16($fnEntryRef->[F_CMD]->[3])};

  bool stus = false;
EOL
    my ($inDecl, $inInit, $initSep, $currSD, $currTransmit) = ('', '', '', '', '');

  my $structName = "command1";
  my $structState = ST_OPEN;
  $currSD = " struct {\n  uint16_t cmd;\n";
  $currTransmit =
      "   stus =  fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,\n" .
      "                     (uint8_t *) &command1, sizeof(command1),\n";
  #	  "	                NULL, 0) ;\n"
  
  my @cstring = ();
  my @regular = ();
  foreach my $inArg (@paramIn) {
      if (($inArg->[P_TYPE] =~ 'char.*\*') and ($inArg->[P_VAR] =~ 'cstr|name')) {
	  # close current struct, add cstring name to the list of the buffer that will need
	  # to be transmited apart
	  push(@cstring, $inArg);
	  if ($structState == ST_OPEN) {
	      $structState = ST_CLOSE;
	      $currSD .= "}  __attribute__ ((__packed__)) $structName = {";
	      $currSD .= " .cmd = cmds[fdsConfig->deviceType],\n" if $structName eq "command1";
	      $currSD .= join(",\n  ", map(".$_->[P_VAR] = __builtin_bswap16($_->[P_VAR]) ", @regular));
	      $currSD .= "};\n";
	      $structName++;
	  }
	  $currTransmit .= "	                NULL, 0)  != 0;\n";
	  $currTransmit .= "   stus =  fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,\n" .
	      "                     (uint8_t *) $inArg->[P_VAR], strlen($inArg->[P_VAR]) + 1,\n";
      } else { # regular
	  if ($structState == ST_CLOSE) {
	      $currSD .=  "struct {\n";
	      $currTransmit .= "	                NULL, 0)  != 0;\n";
	      $currTransmit .= "   stus =  fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,\n" .
		  "                     (uint8_t *) &$structName, sizeof($structName),\n";
	      
	      $structState = ST_OPEN;
	  }
	  push(@regular, $inArg);
	  $currSD .=  "  $inArg->[P_TYPE]  $inArg->[P_VAR];\n";
      }
  }
  #      say ("========");
  if ($structState == ST_OPEN) {
      $currSD .= "}  __attribute__ ((__packed__)) $structName = {";
      $currSD .= " .cmd = cmds[fdsConfig->deviceType],\n" if $structName eq "command1";
      $currSD .= join(",\n  ", map(".$_->[P_VAR] = __builtin_bswap16($_->[P_VAR]) ", @regular));
      $currSD .= "};\n";
  }
  if (not $giveBackString) {
      if ($fixSize ne '0') {
	  $currTransmit .= "(uint8_t *) &response, sizeof(response) - (fdsConfig->deviceType == GOLDELOX ?  $fixSize  : 0)) != 0;\n";
      } else {
	  $currTransmit .= "(uint8_t *) &response, sizeof(response)) != 0;\n";
      }
  } else {
      my $varLenName = "(*$gpo[1]->[P_VAR])";
      $currTransmit .= <<EOL;
				      (uint8_t *) str, 3) == 3;
 if ((!stus) || (str[0] != QDS_ACK)) {
   DebugTrace("%s error ", __FUNCTION__);
   str[0] = 0;
   return false;
 }
 const size_t dynSize =  MIN(getResponseAsUint16((uint8_t *) str),  $varLenName-1);
 $varLenName = dynSize;
 stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
			       NULL, 0,
			       (uint8_t *) str, dynSize) == dynSize;
  if (!stus) {
   DebugTrace("%s error ", __FUNCTION__);
   str[0] = 0;
 } else {
   str[dynSize] = 0;
 }
EOL
  }
  #	  say("DBG> '$currSD'");
  #	  say("DBG> '$currTransmit'");
  
  
  if (scalar(@inP)) {
	$inDecl = join(";\n  ", @inP) . ';';
	$inInit = join(",\n  ", map(".$_->[P_VAR] = __builtin_bswap16($_->[P_VAR]) ",
				    grep ($_->[P_TYPE] =~ /int16_t/, @paramIn)));
	my $inInitSpe = join(",\n  ", map(".$_->[P_VAR] = $_->[P_VAR] ",
					  grep ($_->[P_TYPE] !~ /int16_t/, @paramIn)));
	if ($inInit) {
	    $inInit .= ', ' . $inInitSpe if $inInitSpe;
	} else {
	    $inInit = $inInitSpe if $inInitSpe;
	}
	$initSep = ',';
    }
#   print $implFh <<EOL;

#  struct {
#   uint16_t cmd;
#   $inDecl
# } __attribute__ ((__packed__)) command1 = {
#   .cmd = cmds[fdsConfig->deviceType]$initSep
#   $inInit
# };	
# EOL
  say $implFh $currSD;
  
  if (scalar(@outP)) {
      if (not $giveBackString) {
	my $outDecl = join(";\n  ", @outP) . ';';
	print $implFh <<EOL;

  struct {
  $outDecl
  } __attribute__ ((__packed__)) response;
  $check
EOL
      }
      say $implFh $currTransmit;
 } else {
    print $implFh <<EOL;
    stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command1, sizeof(command1),
		     NULL, 0) == 0; 
EOL
}

  if ((scalar(@outP) > 1) and  (not $giveBackString)) {
      my @paramOut = getParamOut($fnEntryRef);
      say $fh "if (fdsConfig->deviceType != GOLDELOX) {\n"  if ($fixSize ne '0');
      say $fh join(";\n  ", map("  if ($_->[P_VAR] != NULL) \n       *$_->[P_VAR] = __builtin_bswap16(response.$_->[P_VAR]);",
				grep($_->[P_VAR] ne 'ack', @paramOut)));
      say $fh '';
      say $fh "}\n" if ($fixSize ne '0');
  }
    if ((scalar(@outP) > 0) and  (not $giveBackString)) {
      say $fh "  return stus && (response.ack == QDS_ACK);\n}\n";
  } else {
      say $fh "  return stus;\n}\n";
  }
}


sub codeGenPolyFunction($$)
{
    my ($fh, $fnEntryRef) = @_;
    my @paramIn = getParamIn($fnEntryRef);
    my @inP = map(getProtoParam($_, P_IN), @paramIn);
    my @outP = map(getProtoParam($_, P_IN), getParamOut($fnEntryRef));
    my $check = '';
    if (grep($_ eq 'CMD_NOT_IMPL', @{$fnEntryRef->[F_CMD]})) {
	$check = "osalDbgAssert(cmds[fdsConfig->deviceType] != CMD_NOT_IMPL, " .
	         "\"function $fnEntryRef->[F_NAME] unimplemented for this screen\");";
    }
    print $implFh <<EOL;
bool $fnEntryRef->[F_NAME](const FdsConfig *fdsConfig, uint16_t n, const uint16_t vx[], const uint16_t vy[], uint16_t color)
{
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16($fnEntryRef->[F_CMD]->[0]),
					  __builtin_bswap16($fnEntryRef->[F_CMD]->[1]),
					  __builtin_bswap16($fnEntryRef->[F_CMD]->[2]),
				          __builtin_bswap16($fnEntryRef->[F_CMD]->[3])};
  $check
  return gfx_polyxxx(fdsConfig,
	      cmds[fdsConfig->deviceType],
	      n, vx, vy, color);

}
EOL
}


sub codeGenImgBlitFunction($$)
{
    my ($fh, $fnEntryRef) = @_;
    print $implFh <<EOL;
bool img_blitComtoDisplay(const FdsConfig *fdsConfig, uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *  data)
{
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16($fnEntryRef->[F_CMD]->[0]),
					  __builtin_bswap16($fnEntryRef->[F_CMD]->[1]),
					  __builtin_bswap16($fnEntryRef->[F_CMD]->[2]),
				          __builtin_bswap16($fnEntryRef->[F_CMD]->[3])};
  bool stus = false;
 struct {
  uint16_t cmd;
  uint16_t  x;
  uint16_t  y;
  uint16_t  width;
  uint16_t  height;
}  __attribute__ ((__packed__)) command1 = { .cmd = cmds[fdsConfig->deviceType],
  .x = __builtin_bswap16(x) ,
  .y = __builtin_bswap16(y) ,
  .width = __builtin_bswap16(width) ,
  .height = __builtin_bswap16(height)};


  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL, "function img_blitComtoDisplay unimplemented for this screen");
   fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                     (uint8_t *) &command1, sizeof(command1),
		      NULL, 0);

   stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                     (uint8_t *) data, sizeof(*data) * width * height,
	             (uint8_t *) &response, sizeof(response)) != 0;


  return stus;
}

EOL
}

sub codeGenFileCallFunction($$)
{
    my ($fh, $fnEntryRef) = @_;
    print $implFh <<EOL;
bool file_callFunction(const FdsConfig *fdsConfig, uint16_t handle, uint16_t n,
                       const uint16_t *args, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16($fnEntryRef->[F_CMD]->[0]),
					  __builtin_bswap16($fnEntryRef->[F_CMD]->[1]),
					  __builtin_bswap16($fnEntryRef->[F_CMD]->[2]),
				          __builtin_bswap16($fnEntryRef->[F_CMD]->[3])};

  
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t n;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .n = __builtin_bswap16(n)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_callFunction unimplemented for this screen");
  fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
		     (uint8_t *)&command1, sizeof(command1),
		     NULL, 0);
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)args, n * sizeof(*args),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}
    
EOL
}

sub codeGenFileRunFunction($$)
{
    my ($fh, $fnEntryRef) = @_;
    print $implFh <<EOL;
 bool file_run(const FdsConfig *fdsConfig, const char *filename, uint16_t n,
              const uint16_t *args, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16($fnEntryRef->[F_CMD]->[0]),
					  __builtin_bswap16($fnEntryRef->[F_CMD]->[1]),
					  __builtin_bswap16($fnEntryRef->[F_CMD]->[2]),
				          __builtin_bswap16($fnEntryRef->[F_CMD]->[3])};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };
  struct {
    uint16_t n;
  } __attribute__((__packed__))
  command2 = {.n = __builtin_bswap16(n)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_run unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                         (uint8_t *)&command1, sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)filename, strlen(filename) + 1,
			    NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command2, sizeof(command2),
			    NULL, 0);
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)args,
                            n * sizeof(*args), (uint8_t *)&response,
                            sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
 }
    
EOL
}
    
sub codeGenFileWriteFunction($$)
{
    my ($fh, $fnEntryRef) = @_;
    print $implFh <<EOL;
 bool file_write(const FdsConfig *fdsConfig, uint16_t size, const uint8_t *source,
                uint16_t handle, uint16_t *count) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16($fnEntryRef->[F_CMD]->[0]),
					  __builtin_bswap16($fnEntryRef->[F_CMD]->[1]),
					  __builtin_bswap16($fnEntryRef->[F_CMD]->[2]),
				          __builtin_bswap16($fnEntryRef->[F_CMD]->[3])};


  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t size;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .size = __builtin_bswap16(size)};

  struct {
    uint16_t handle;
  } __attribute__((__packed__)) command2 = {.handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t count;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_write unimplemented for this screen");
  fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
		     (uint8_t *)&command1, sizeof(command1),
                     NULL, 0);
  fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
		     source, size, NULL,
		     0);
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
			    (uint8_t *)&command2, sizeof(command2),
			    (uint8_t *)&response, sizeof(response));
  
  if (count != NULL) *count = __builtin_bswap16(response.count);
  
  return stus && (response.ack == QDS_ACK);
}
   
EOL
}
    
sub codeGenFileExecFunction($$)
{
    my ($fh, $fnEntryRef) = @_;
    print $implFh <<EOL;
 bool file_exec(const FdsConfig *fdsConfig, const char *filename, uint16_t n,
                const uint16_t *args, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16($fnEntryRef->[F_CMD]->[0]),
					  __builtin_bswap16($fnEntryRef->[F_CMD]->[1]),
					  __builtin_bswap16($fnEntryRef->[F_CMD]->[2]),
				          __builtin_bswap16($fnEntryRef->[F_CMD]->[3])};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };
  struct {
    uint16_t n;
  } __attribute__((__packed__))
  command2 = {.n = __builtin_bswap16(n)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_run unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                         (uint8_t *)&command1, sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)filename, strlen(filename) + 1,
			    NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command2, sizeof(command2),
			    NULL, 0);
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)args,
                            n * sizeof(*args), (uint8_t *)&response,
                            sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
 }
    
EOL
}
  
sub codeGenPrototype($$)
{
    my ($fh, $fnEntryRef) = @_;
    my @inP = map(getProtoParam($_, P_IN), getParamIn($fnEntryRef));
    my $protoargsIn = 'const FdsConfig *fdsConfig' ;
    $protoargsIn .= (', ' .  join(', ', @inP)) if scalar(@inP);
    my @outP = map(getProtoParam($_, P_OUT), getParamOut($fnEntryRef));
    my $protoargsOut = scalar(@outP) ? join(', ', @outP) : '';
    print $fh "bool $fnEntryRef->[F_NAME](${protoargsIn}${protoargsOut})";
}
    
sub codeGenPolyPrototype($$)
{
    my ($fh, $fnEntryRef) = @_;
    print $fh "bool $fnEntryRef->[F_NAME](const FdsConfig *fdsConfig, uint16_t n, const uint16_t vx[], const uint16_t vy[], uint16_t color)";
}
    
sub codeGenImgBlitPrototype($$)
{
    my ($fh, $fnEntryRef) = @_;
    print $fh "bool img_blitComtoDisplay(const FdsConfig *fdsConfig, uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *  data)";
}
    
sub codeGenFileCallPrototype($$)
{
    my ($fh, $fnEntryRef) = @_;
    print $fh "bool file_callFunction(const FdsConfig *fdsConfig, uint16_t handle, uint16_t n,\n" . 
                       "const uint16_t *args, uint16_t *value)";
}
    
sub codeGenFileRunPrototype($$)
{
    my ($fh, $fnEntryRef) = @_;
    print $fh "bool file_run(const FdsConfig *fdsConfig, const char *filename, uint16_t n,\n" .
              "const uint16_t *args, uint16_t *value)"
}
    
sub codeGenFileExecPrototype($$)
{
    my ($fh, $fnEntryRef) = @_;
    print $fh "bool file_exec(const FdsConfig *fdsConfig, const char *filename, uint16_t n,\n" .
              "const uint16_t *args, uint16_t *value)"
}
    
sub codeGenFileWritePrototype($$)
{
    my ($fh, $fnEntryRef) = @_;
    print $fh "bool file_write(const FdsConfig *fdsConfig, uint16_t size, const uint8_t *source,\n" .
              "uint16_t handle, uint16_t *count)"
}
    
    
sub getParamIn($)
{
    my $fnEntryRef = shift;
    my @params = @{$fnEntryRef->[F_ARGIN]};
    my @retArr;
#    say "DBG> " . join('; ', @params);
    foreach my $paramDesc (@params) {
	my ($type) = $paramDesc =~ /\((.+)\)/;
	$paramDesc =~ s/\(.+\)//;
	my ($num) = $paramDesc =~ /\[(.+)\]/;
	$paramDesc =~ s/\[(.+)\]//;
	$type //= 'uint16_t';
	$num //= 0;
	push(@retArr, [$type, $paramDesc, $num, '']);
#	say "DBG> type=$type name = $paramDesc num=$num" 
    }
    return @retArr;
}

sub getParamOut($)
{
    my $fnEntryRef = shift;
    my @params = @{$fnEntryRef->[F_ARGOUT]};
    my @retArr;

#    say "DBG> " . join('; ', @params);
    foreach my $paramDesc (@params) {
	my ($type) = $paramDesc =~ /\((.+)\)/;
	$paramDesc =~ s/\(.+\)//;
	my ($num) = $paramDesc =~ /\[(.+)\]/;
	$paramDesc =~ s/\[(.+)\]//;
	if ($paramDesc =~ /ack/) {
	    $type = 'uint8_t' ;
	} else {
	    $type //= 'uint16_t';
	}
	$num //= 0;
	my ($opt) = $paramDesc =~ /(:)/;
	$opt //= '';
	$paramDesc =~ s/(:)//;
	push(@retArr, [$type, $paramDesc, $num, $opt]);
    }
    return @retArr;
}

sub getProtoParam($$) # take tripplet (type, name, arrayNum) ref, P_IN or P_OUT, return string, 
{
    my ($paramRef, $inOut) = @_;
    my ($type, $name, $num) = @$paramRef;

    die "incorrect P_IN/P_OUT\n" unless ($inOut == P_IN) or ($inOut == P_OUT);
    $name = "*$name" if $inOut == P_OUT;
    
    my $proto = "$type $name";
    $proto = "" if $name eq '*ack';
#    say "DBG> proto = '$proto'" if $proto;
    return $proto;
}
