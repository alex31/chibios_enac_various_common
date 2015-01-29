#include <ch.h>
#include <hal.h>
#include "globalVar.h"
#include "stdutil.h"
#include "printf.h"
#include "oledDisplay.h"
#include "string.h"
//#include "lcdDisplay.h"

#define clampColor(r,v,b) ((r & 0x1f) <<11 | (v & 0x3f) << 5 | (b & 0x1f))
#define colorDecTo16b(r,v,b) (clampColor((r*31/100), (v*63/100), (b*31/100)))


/* #define OLED(n, ...) (\ */
/* 		      chnReadTimeout (((BaseChannel *) oled), response, sizeof (response), 10), \ */
/* 		      chprintf (oled, __VA_ARGS__),			\ */
/* 		      n == 0 ? 0 : chnReadTimeout (((BaseChannel *) oled), response, MIN(sizeof (response), n), 100)\ */
/* ) */

static void oledTrace (oledConfig *oledConfig, const char* err);

#define OLED(n, ...) (\
		      chnReadTimeout (((BaseChannel *) oledConfig->serial), oledConfig->response, \
				      sizeof (oledConfig->response), TIME_IMMEDIATE), \
		      chprintf (oledConfig->serial, __VA_ARGS__),	\
		      n == 0 ? 0 : chnReadTimeout (((BaseChannel *) oledConfig->serial), \
						   oledConfig->response, \
						   MIN(sizeof (oledConfig->response), n), 100), \
		      oledConfig->response[0] != 0x6 ?  \
		      (oledTrace (oledConfig, oledConfig->response[0] == 0x15 ? "NACK" : "OLED Protocol error")) :0 \
		      )




#define RET_UNLESS_INIT(oledCfg) {if (oledIsInitialised(oledCfg) == FALSE) return;}

static bool_t oledIsInitialised (oledConfig *oledConfig) ;
static void oledSetBaud (oledConfig *oledConfig, uint32_t baud);
static void oledPreInit (oledConfig *oledConfig, uint32_t baud);
static void oledReInit (oledConfig *oledConfig);

static void oledPreInit (oledConfig *oledConfig, uint32_t baud)
{
  oledConfig->bg = colorDecTo16b(0,0,0);
  oledConfig->tbg[0] = colorDecTo16b(0,0,0);
  oledConfig->fg[0] = colorDecTo16b(50,50,50);
  oledConfig->tbgIdx = oledConfig->fgIdx = 0;
  oledConfig->curXpos =0;
  oledConfig->curYpos =0;
  oledConfig->serial = NULL;

  // initial USART conf
  // 9600 bauds because of broken ato baud feature on some screen model
  oledConfig->serialConfig.speed = baud;
  oledConfig->serialConfig.cr1 =0;
  oledConfig->serialConfig.cr2 = USART_CR2_STOP1_BITS | USART_CR2_LINEN;
  oledConfig->serialConfig.cr3 =0;
  
}

void oledInit (oledConfig *oledConfig,  struct SerialDriver *oled, const uint32_t baud,
	       GPIO_TypeDef *rstGpio, uint32_t rstPin)
{
  oledConfig->rstGpio = rstGpio;
  oledConfig->rstPin = rstPin;
  oledHardReset (oledConfig);

  oledPreInit (oledConfig, 9600);
  oledConfig->serial = (BaseSequentialStream *) oled;
  chMtxInit(&(oledConfig->omutex));
  sdStart(oled, &(oledConfig->serialConfig));
  chThdSleepMilliseconds(10);
  OLED (10, "UV%c", 0x1); // init, print version flush response buffer
  OLED (1, "O%c", 0x1); // opaque background
  
  // use greater speed
  if (baud != 9600) {
    oledSetBaud (oledConfig, baud);
    chThdSleepMilliseconds(100);
  }
}

static void oledReInit (oledConfig *oledConfig)
{
  oledHardReset (oledConfig);
  const uint32_t baud = oledConfig->serialConfig.speed;

  oledSetBaud (oledConfig, 9600);
  chThdSleepMilliseconds(10);
  OLED (10, "UV%c", 0x1); // init, print version flush response buffer
  OLED (1, "O%c", 0x1); // opaque background
  oledClearScreen (oledConfig);

  
  // use greater speed
  if (baud != 9600) {
    oledSetBaud (oledConfig, baud);
    chThdSleepMilliseconds(100);
  }
}


void oledHardReset (oledConfig *oledConfig)
{
  palClearPad (oledConfig->rstGpio, oledConfig->rstPin);
  chThdSleepMilliseconds(10);
  palSetPad (oledConfig->rstGpio, oledConfig->rstPin);
  chThdSleepMilliseconds(700);
}


bool_t oledIsInitialised (oledConfig *oledConfig)
{
  return (oledConfig->serial != NULL);
}

void oledAcquireLock (oledConfig *oledConfig)
{ 
  RET_UNLESS_INIT(oledConfig);
  chMtxLock(&(oledConfig->omutex));
}

void oledReleaseLock (oledConfig *oledConfig)
{ 
  RET_UNLESS_INIT(oledConfig);
  (void) oledConfig;
  chMtxUnlock();
}

void oledPrintVersion (oledConfig *oledConfig)
{
  uint32_t getRes (uint32_t code) {
    switch (code) {
    case 0x22 : return 220;
    case 0x28 : return 128;
    case 0x32 : return 320;
    case 0x60 : return 160;
    case 0x64 : return 64;
    case 0x76 : return 176;
    case 0x96 : return 96;
    case 0xff : return 0;
    default: return 0;
    }
  }

  RET_UNLESS_INIT(oledConfig);
  OLED (5, "V%c", 0x1);
  for (uint8_t i = 0; i< 5; i++) {
    DebugTrace ("response[%d] = 0x%x", i, oledConfig->response[i]);
  }

}

void oledSetBaud (oledConfig *oledConfig, uint32_t baud)
{
  
  uint8_t baudCode ;
  oledConfig->serialConfig.speed = baud;
  struct SerialDriver *sd = (struct SerialDriver *) oledConfig->serial;

  RET_UNLESS_INIT(oledConfig);

  switch (baud) {
  case 110    : baudCode = 0x0; break;
  case 300    : baudCode = 0x1; break;
  case 600    : baudCode = 0x2; break;
  case 1200   : baudCode = 0x3; break;
  case 2400   : baudCode = 0x4; break;
  case 4800   : baudCode = 0x5; break;
  case 9600   : baudCode = 0x6; break;
  case 14400  : baudCode = 0x7; break;
  case 19200  : baudCode = 0x8; break;
  case 31250  : baudCode = 0x9; break;
  case 38400  : baudCode = 0xa; break;
  case 56000  : baudCode = 0xb; break;
  case 57600  : baudCode = 0xc; break;
  case 115200 : baudCode = 0xd; break;
  case 128000 : baudCode = 0xe; break;
  case 256000 : baudCode = 0xf; break;

  default : return;
  }

  OLED (1, "Q%c", baudCode);

  sdStop (sd);
  sdStart (sd, &(oledConfig->serialConfig));
}

void oledPrintFmt (oledConfig *oledConfig, const char *fmt, ...)
{
  char buffer[80];
  va_list ap;
  RET_UNLESS_INIT(oledConfig);

  va_start(ap, fmt);
  chvsnprintf(buffer, sizeof(buffer), fmt, ap); 
  va_end(ap);

  OLED (1, "s%c%c%c%c%c%s%c", oledConfig->curXpos, oledConfig->curYpos, 0x02,
	(oledConfig->fg[0] & 0xff00) >> 8, oledConfig->fg[0] & 0xff, 
	buffer, 0x0); 

}

void oledPrintBuffer (oledConfig *oledConfig, const char *buffer)
{
  RET_UNLESS_INIT(oledConfig);

  OLED (1, "s%c%c%c%c%c%s%c", oledConfig->curXpos, oledConfig->curYpos, 0x02,
	(oledConfig->fg[0] & 0xff00) >> 8, oledConfig->fg[0] & 0xff, 
	buffer, 0x0); 

}

void oledChangeBgColor (oledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);
  oledConfig->bg = colorDecTo16b(r,g,b);
  OLED (1, "B%c%c", ((oledConfig->bg & 0xff00) >> 8), (oledConfig->bg & 0xff));
  
}

void oledSetTextBgColor (oledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);
  oledConfig->tbg[0] = colorDecTo16b(r,g,b);
  OLED (1, "K%c%c", ((oledConfig->tbg[0] & 0xff00) >> 8), (oledConfig->tbg[0] & 0xff));
}

void oledSetTextFgColor (oledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);
 oledConfig->fg[0] = colorDecTo16b(r,g,b);
}

void oledSetTextBgColorTable (oledConfig *oledConfig, uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);
  if (++index >= COLOR_TABLE_SIZE) return;

  oledConfig->tbg[index] = colorDecTo16b(r,g,b);
}

void oledSetTextFgColorTable (oledConfig *oledConfig,  uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);
  if (++index >= COLOR_TABLE_SIZE) return;

  oledConfig->fg[index] = colorDecTo16b(r,g,b);
}

void oledUseColorIndex (oledConfig *oledConfig, uint8_t index)
{
  RET_UNLESS_INIT(oledConfig);
  if (++index >= COLOR_TABLE_SIZE) return;

  oledConfig->fg[0] = oledConfig->fg[index];

  if (oledConfig->tbg[0] != oledConfig->tbg[index])  {
    oledConfig->tbg[0] = oledConfig->tbg[index];
    OLED (1, "K%c%c", ((oledConfig->tbg[0] & 0xff00) >> 8), (oledConfig->tbg[0] & 0xff));
  }
}




void oledSound (oledConfig *oledConfig, uint16_t freq, uint16_t duration)
{
  RET_UNLESS_INIT(oledConfig);
  duration = MAX (duration, 1000); // duration < 1000 hangs 4ds screen
  OLED (1, "N%c%c%c%c",  (uint8_t) ((freq & 0xff00) >> 8), (uint8_t) (freq & 0xff),
	(uint8_t) ((duration & 0xff00) >> 8), (uint8_t) (duration & 0xff));
}

void oledGotoXY (oledConfig *oledConfig, uint8_t x, uint8_t y)
{  
  RET_UNLESS_INIT(oledConfig);
  oledConfig->curXpos=x;
  oledConfig->curYpos=y;
}

void oledGotoX (oledConfig *oledConfig, uint8_t x)
{  
  RET_UNLESS_INIT(oledConfig);
  oledConfig->curXpos=x;
}

void oledGotoNextLine (oledConfig *oledConfig)
{  
  RET_UNLESS_INIT(oledConfig);
  oledConfig->curXpos=0;
  oledConfig->curYpos++;
}

void oledClearScreen (oledConfig *oledConfig)
{
  RET_UNLESS_INIT(oledConfig);
  OLED (1, "E");
}

void oledInitSdCard (oledConfig *oledConfig)
{
  RET_UNLESS_INIT(oledConfig);
  OLED (1, "@i");
}

void oledListSdCardDirectory (oledConfig *oledConfig)
{
  RET_UNLESS_INIT(oledConfig);
  uint8_t gc;
  uint32_t retV;

  DebugTrace ("sd card list:");
  const uint8_t command[] = {0x40, 0x64, 0x2a, 0x00};
  chnReadTimeout (((BaseChannel *) oledConfig->serial), oledConfig->response, 
		  sizeof (oledConfig->response), TIME_IMMEDIATE);
  
  chSequentialStreamWrite (((BaseChannel *) oledConfig->serial), command, sizeof(command));
  do {
    retV = chnReadTimeout (((BaseChannel *) oledConfig->serial), &gc, 
		  sizeof (gc), 10);
    chSequentialStreamPut (chp, gc);
    if (gc == 0x0A) {
      chSequentialStreamPut (chp, '\r');
    }
  } while (retV && (gc != 0x6) && (gc != 0x15));

  if (retV ==0) {
    DebugTrace (" ..... Timout");
  } else {
    if (gc == 0x6) {
      DebugTrace (" ..... Done");
    } else {
      DebugTrace (" ..... NACK");
    }
  }
}

void oledSetSoundVolume (oledConfig *oledConfig, uint8_t percent)
{
  percent = MIN (100, percent);
  percent +=27;
  OLED (1, "v%c", percent);
}

void oledPlayWav (oledConfig *oledConfig, const char* fileName)
{  
  RET_UNLESS_INIT(oledConfig);
  OLED (1, "@l%c%s%c", 1, fileName, 0);
}

void oledDisplayCgi (oledConfig *oledConfig, const char* fileName, uint32_t offset)
{
  union {
    uint32_t val;
    uint8_t  buf [sizeof(uint32_t)] ;
  } ofst;

  ofst.val = offset;
     
  RET_UNLESS_INIT(oledConfig);
    OLED (1, "@m%s%c%c%c%c%c%c%c%c%c", fileName, 0, 0, 0, 0, 0, ofst.buf[3], ofst.buf[2], ofst.buf[1], 
  	ofst.buf[0]);

  /* char buffer [32] = {[0 ... sizeof(buffer)-1] = 0xff}; */
  /* chsnprintf (buffer, sizeof(buffer),  */
  /* 	      "@m%s%c%c%c%c%c%c%c%c%c", fileName, 0, 0, 0, 0, 0, ofst.buf[3], ofst.buf[2], ofst.buf[1],  */
  /* 	      ofst.buf[0]); */

  /* for (uint32_t i=0; i< 21; i++) { */
  /*   DebugTrace ("buffer[%d] = 0x%x [%c]", i, buffer[i], buffer[i]); */
  /*   chSequentialStreamPut (oledConfig->serial, buffer[i]); */
  /* } */

}

/*
#                 _ __           _                    _                   
#                | '_ \         (_)                  | |                  
#                | |_) |  _ __   _   __   __   __ _  | |_     ___         
#                | .__/  | '__| | |  \ \ / /  / _` | | __|   / _ \        
#                | |     | |    | |   \ V /  | (_| | \ |_   |  __/        
#                |_|     |_|    |_|    \_/    \__,_|  \__|   \___|        
*/
/* static bool_t oled (uint32_t respLen, const char *fmt, ...) */
/* { */
/*   va_list ap; */
/*     va_start(ap, fmt); */
/*   chvsnprintf(buffer, size, fmt, ap);  */
/*   va_end(ap); */
/* } */

static void oledTrace (oledConfig *oledConfig, const char* err)
{
  static uint32_t errCount=0;

  DebugTrace (err);
  if (errCount++ == 5) {
    errCount = 0;
    oledReInit (oledConfig);
  }
  
}
