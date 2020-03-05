#include <ch.h>
#include <hal.h>
#include <string.h>
#include <ctype.h>
#include "stdutil.h"
#include "printf.h"
#include "picaso4Display.h"

#if defined LCD_240_320 || defined LCD_240_400
#include "hardwareTest.h"
#endif


#define clampColor(r,v,b) ((uint16_t) ((r & 0x1f) <<11 | (v & 0x3f) << 5 | (b & 0x1f)))
#define colorDecTo16b(r,v,b) (clampColor((r*31/100), (v*63/100), (b*31/100)))
#define twoBytesFromWord(x) ((uint8_t)((x & 0xff00) >> 8)), ((uint8_t)(x & 0xff))

typedef enum {KOF_NONE, KOF_ACK, KOF_INT16, 
	      KOF_INT16LENGTH_THEN_DATA} KindOfCommand;

const uint32_t readTimout = TIME_MS2I(500);
static OledStatus oledStatus = OLED_OK;

/* #define OLED(n, ...) (\ */
/* 		      chnReadTimeout (((BaseChannel *) oled), response, sizeof (response), 10), \ */
/* 		      chprintf (oled, __VA_ARGS__),			\ */
/* 		      n == 0 ? 0 : chnReadTimeout (((BaseChannel *) oled), response, MIN(sizeof (response), n), 100)\ */
/* ) */

static void oledTrace (OledConfig *oledConfig, const char* err);
static uint32_t oledSendCommand (OledConfig *oc, KindOfCommand kof, 
				 const char* fct, const uint32_t line, const char *fmt, ...);

static uint32_t oledReceiveAnswer (OledConfig *oc, const uint32_t size,
				   const char* fct, const uint32_t line);
static void sendVt100Seq (BaseSequentialStream *serial, const char *fmt, ...);


#define OLED(...) (oledSendCommand (oledConfig, KOF_ACK, __FUNCTION__, __LINE__, __VA_ARGS__))
#define OLED_KOF(k, ...) (oledSendCommand (oledConfig, k,__FUNCTION__, __LINE__,  __VA_ARGS__))


#define RET_UNLESS_INIT(oledCfg)    {if (oledIsInitialised(oledCfg) == FALSE) return;}
#define RET_UNLESS_4DSYS(oledCfg)  {if (oledCfg->deviceType == TERM_VT100) return;}
#define RET_UNLESS_PICASO(oledCfg)  {if (oledCfg->deviceType != PICASO) return;}
#define RET_UNLESS_GOLDELOX(oledCfg)  {if (oledCfg->deviceType != GOLDELOX) return;}
#define ISPIC(oledCfg)  (oledCfg->deviceType == PICASO) 

static bool oledIsInitialised (const OledConfig *oledConfig) ;
static void oledScreenSaverTimout (OledConfig *oledConfig, uint16_t timout);
static void oledPreInit (OledConfig *oledConfig, uint32_t baud);
static void oledReInit (OledConfig *oledConfig);
static uint32_t oledGetFileError  (OledConfig *oledConfig);
static bool oledFileSeek  (OledConfig *oledConfig, const uint16_t handle, const uint32_t offset);
static uint16_t fgColorIndexTo16b (const OledConfig *oledConfig, const uint8_t colorIndex);
static uint16_t oledTouchGet (OledConfig *oledConfig, uint16_t mode);
static uint16_t getResponseAsUint16 (OledConfig *oledConfig);

static void oledPreInit (OledConfig *oledConfig, uint32_t baud)
{
  oledConfig->bg = colorDecTo16b(0,0,0);
  oledConfig->tbg[0] = mkColor24 (0,0,0);
  oledConfig->fg[0] = mkColor24(50,50,50);
  oledConfig->tbgIdx = oledConfig->fgIdx = 0;
  oledConfig->curXpos =0;
  oledConfig->curYpos =0;
  oledConfig->serial = NULL;

  // initial USART conf only for 4D system screen
  // 9600 bauds because of broken ato baud feature on some screen model
  
  oledConfig->serialConfig.speed = baud;
  oledConfig->serialConfig.cr1 =0;
  oledConfig->serialConfig.cr2 = USART_CR2_STOP1_BITS | USART_CR2_LINEN;
  oledConfig->serialConfig.cr3 =0;
}

void oledInit (OledConfig *oledConfig,  struct SerialDriver *oled, const uint32_t baud,
	      ioportid_t rstGpio, uint32_t rstPin, enum OledConfig_Device dev)
{
  oledConfig->rstGpio = rstGpio;
  oledConfig->rstPin = rstPin;
  oledConfig->deviceType = dev;

  oledHardReset (oledConfig);


  oledPreInit (oledConfig,
	       oledConfig->deviceType == TERM_VT100 ? baud : 9600);
  oledConfig->serial = (BaseSequentialStream *) oled;
  chMtxInit(&(oledConfig->omutex));
  sdStart(oled, &(oledConfig->serialConfig));
  chThdSleepMilliseconds(10);


  // test is no error on kind of device : picaso, goldelox, diablo ...
  if (!oledIsCorrectDevice (oledConfig)) {
    // if not try to change it
    oledConfig->deviceType = (oledConfig->deviceType == PICASO) ? GOLDELOX : PICASO;
    oledSetTextOpacity (oledConfig, true);
    oledSetTextOpacity (oledConfig, true);
    DebugTrace ("oled comm error, try %s\r\n",
		(oledConfig->deviceType == PICASO) ? "PICASO instead GOLDELOX" :
		"GOLDELOX instead PICASO");
  }

  // opaque background
  oledClearScreen (oledConfig);
  oledSetTextOpacity (oledConfig, true);


  // disable screensaver : 0 is special value for disabling screensaver
  // since oled has remanance problem, we activate screensaver after 20 seconds
  oledScreenSaverTimout (oledConfig, 20000);
  
  // use greater speed
  if ((oledConfig->deviceType != TERM_VT100) && (baud != 9600))
    oledSetBaud (oledConfig, baud);
}

static void oledReInit (OledConfig *oledConfig)
{
  switch (oledConfig->deviceType) {
  case PICASO : 
  case GOLDELOX : 
    oledHardReset (oledConfig);
    const uint32_t baud = oledConfig->serialConfig.speed;
    
    oledSetBaud (oledConfig, 9600);

    // opaque background
    oledSetTextOpacity (oledConfig, true);
    oledClearScreen (oledConfig);
    
    // use greater speed
    if (baud != 9600) 
      oledSetBaud (oledConfig, baud);
    
    break;
    
  case TERM_VT100 : break;
  } 
}


void oledHardReset (OledConfig *oledConfig)
{

  RET_UNLESS_4DSYS(oledConfig);

  palClearPad (oledConfig->rstGpio, oledConfig->rstPin);
  chThdSleepMilliseconds(10);
  palSetPad (oledConfig->rstGpio, oledConfig->rstPin);
  chThdSleepMilliseconds(3500);
}


bool oledIsInitialised (const OledConfig *oledConfig)
{
  return (oledConfig->serial != NULL);
}

void oledAcquireLock (OledConfig *oledConfig)
{ 
  RET_UNLESS_INIT(oledConfig);
  chMtxLock(&(oledConfig->omutex));
}

void oledReleaseLock (OledConfig *oledConfig)
{ 
  RET_UNLESS_INIT(oledConfig);
  (void) oledConfig;
#if (CH_KERNEL_MAJOR > 2)
  chMtxUnlock(&(oledConfig->omutex));
#else
  chMtxUnlock();
#endif
}

void oledGetVersion (OledConfig *oledConfig, char *buffer, const size_t buflen)
{
  RET_UNLESS_4DSYS(oledConfig);
  const uint8_t cmdModel =  ISPIC(oledConfig) ? 0x1a : 0x07;
  const uint8_t cmdPmmc =  ISPIC(oledConfig) ? 0x1c : 0x09;
  const uint8_t cmdSpe =  ISPIC(oledConfig) ? 0x1b : 0x08;

   // get display model
  OLED_KOF (KOF_INT16LENGTH_THEN_DATA, "%c%c", 0x00, cmdModel);  
  strncpy (buffer, (const char *) oledConfig->response, buflen);

  // get Pmmc version
  OLED_KOF (KOF_INT16, "%c%c", 0x00, cmdPmmc);
  const uint16_t pmmc = getResponseAsUint16(oledConfig);
  const uint8_t pmmcMajor = (uint8_t) (pmmc/256);
  const uint8_t pmmcMinor =  (uint8_t) (pmmc%256);
  chsnprintf (&buffer[strlen(buffer)], buflen-strlen(buffer), " Pmmc=%d.%d", pmmcMajor, pmmcMinor);

  // get SPE version
  OLED_KOF (KOF_INT16, "%c%c", 0x00, cmdSpe);
  const uint16_t spe = getResponseAsUint16(oledConfig);
  const uint8_t speMajor = (uint8_t) (spe/256);
  const uint8_t speMinor = (uint8_t) (spe%256);
  chsnprintf (&buffer[strlen(buffer)], buflen-strlen(buffer), " Spe=%d.%d", speMajor, speMinor);

  // string terminaison
  buffer[buflen-1]=0;
}


void oledSetBaud (OledConfig *oledConfig, uint32_t baud)
{
  uint32_t actualbaudRate; 

  struct SerialDriver *sd = (struct SerialDriver *) oledConfig->serial;

  RET_UNLESS_INIT(oledConfig);

  

  
  switch  (oledConfig->deviceType) {
  case PICASO: {
    uint8_t baudCode ;
    switch (baud) {
    case 110    : baudCode = 0x0;  actualbaudRate = 110; break;
    case 300    : baudCode = 0x1;  actualbaudRate = 300; break;
    case 600    : baudCode = 0x2;  actualbaudRate = 600; break;
    case 1200   : baudCode = 0x3;  actualbaudRate = 1200; break;
    case 2400   : baudCode = 0x4;  actualbaudRate = 2402; break;
    case 4800   : baudCode = 0x5;  actualbaudRate = 4808; break;
    case 9600   : baudCode = 0x6;  actualbaudRate = 9632; break;
    case 14400  : baudCode = 0x7;  actualbaudRate = 14423; break;
    case 19200  : baudCode = 0x8;  actualbaudRate = 19264; break;
    case 31250  : baudCode = 0x9;  actualbaudRate = 31250; break;
    case 38400  : baudCode = 0xa;  actualbaudRate = 38527; break;
    case 56000  : baudCode = 0xb;  actualbaudRate = 56250; break;
    case 57600  : baudCode = 0xc;  actualbaudRate = 58594; break;
    case 115200 : baudCode = 0xd;  actualbaudRate = 117188; break;
    case 128000 : baudCode = 0xe;  actualbaudRate = 133929; break;
    case 256000 : baudCode = 0xf;  actualbaudRate = 281250; break;
    case 300000 : baudCode = 0x10; actualbaudRate = 312500; break;
    case 375000 : baudCode = 0x11; actualbaudRate = 401786; break;
    case 500000 : baudCode = 0x12; actualbaudRate = 562500; break;
    case 600000 : baudCode = 0x13; actualbaudRate = 703125; break;
    default : return;
    }
    oledConfig->serialConfig.speed = actualbaudRate; 
    // send command, do not wait response
    OLED_KOF (KOF_NONE, "%c%c%c%c", 0x0, 0x26, 0x0, baudCode);
  }
    break;

  case GOLDELOX: {
    uint16_t baudCode ;
    switch (baud) {
    case 110    : baudCode =  27271;  	actualbaudRate = 110; break;
    case 300    : baudCode =  9999;  	actualbaudRate = 300; break;
    case 600    : baudCode =  4999;  	actualbaudRate = 600; break;
    case 1200   : baudCode =  2499;  	actualbaudRate = 1200; break;
    case 2400   : baudCode =  1249;  	actualbaudRate = 2402; break;
    case 4800   : baudCode =  624;  	actualbaudRate = 4808; break;
    case 9600   : baudCode =  312;  	actualbaudRate = 9585; break;
    case 14400  : baudCode =  207;  	actualbaudRate = 14423; break;
    case 19200  : baudCode =  155;  	actualbaudRate = 19231; break;
    case 31250  : baudCode =  95;  	actualbaudRate = 31250; break;
    case 38400  : baudCode =  77;  	actualbaudRate = 38462; break;
    case 56000  : baudCode =  53;  	actualbaudRate = 55556; break;
    case 57600  : baudCode =  51;  	actualbaudRate = 57692; break;
    case 115200 : baudCode =  25;  	actualbaudRate = 115385; break;
    case 128000 : baudCode =  22;  	actualbaudRate = 130435; break;
    case 256000 : baudCode =  11;  	actualbaudRate = 250000; break;
    case 300000 : baudCode =  10; 	actualbaudRate = 272727; break;
    case 375000 : baudCode =  8; 	actualbaudRate = 333333; break;
    case 500000 : baudCode =  6; 	actualbaudRate = 428571; break;
    case 600000 : baudCode =  4;	actualbaudRate = 600000; break;
    default : return;
    }
    oledConfig->serialConfig.speed = actualbaudRate; 
    // send command, do not wait response
    OLED_KOF (KOF_NONE, "%c%c%c%c", 0x0, 0x0b, twoBytesFromWord(baudCode));
  }
    break;

  case TERM_VT100:
    oledConfig->serialConfig.speed = baud;
    break;
  }

  chThdSleepMilliseconds(10);
  sdStop (sd);
  sdStart (sd, &(oledConfig->serialConfig));

  // wait 150ms, and wait for response at new speed
  chThdSleepMilliseconds(150);
  RET_UNLESS_4DSYS(oledConfig);
  oledReceiveAnswer (oledConfig, 1, __FUNCTION__, __LINE__);
}


void oledPrintFmt (OledConfig *oledConfig, const char *fmt, ...)
{
  char buffer[120];
  char *token, *curBuf;
  bool lastLoop=FALSE;

  va_list ap;
  RET_UNLESS_INIT(oledConfig);

  va_start(ap, fmt);
  chvsnprintf(buffer, sizeof(buffer), fmt, ap);
  va_end(ap);

  if (buffer[0] == 0)
    return;

  if (buffer[1] == 0) {
    oledPrintBuffer (oledConfig, buffer);
    return;
  }
  
  const char* endPtr = &(buffer[strnlen(buffer, sizeof (buffer)) -1]);
  // replace escape color sequence by color command for respective backend
  // ESC c 0 à 9 : couleur index of background and foreground
  // replace escape n by carriage return, line feed
  // replace escape t by horizontal tabulation
  for (curBuf=buffer;(curBuf<endPtr) && (lastLoop == FALSE);) {
    token = index (curBuf, 033);
    if (token == NULL) {
      // on peut imprimer les derniers caractères et terminer
      lastLoop = TRUE;
    } else {
      // token pointe sur le char d'echappement ESC
      // on met un caractère de fin de chaine à la place
      *token++ =0;
    }
    
    if (*curBuf != 0) {
      oledPrintBuffer (oledConfig, curBuf);
      oledConfig->curXpos =  (uint8_t) (oledConfig->curXpos + strnlen (curBuf, sizeof(buffer)));
    }
    
    if (lastLoop == FALSE) {
      // next two char a color coding scheme
      if (tolower((uint32_t) (*token)) == 'c') { 
	const int32_t colorIndex = *++token - '0';
	oledUseColorIndex (oledConfig,  (uint8_t) colorIndex);
	//	DebugTrace ("useColorIndex %d", colorIndex);
	curBuf=token+1;
      } else if (tolower((uint32_t) (*token)) == 'n') { 	
	//	DebugTrace ("carriage return");
	oledGotoXY (oledConfig, 0,  (uint8_t) (oledConfig->curYpos+1));
	curBuf=token+1;
      } else if (tolower((uint32_t) (*token)) == 't') { 	
	//	DebugTrace ("tabulation");
	const uint8_t tabLength =  (uint8_t) (8-(oledConfig->curXpos%8));
	char space[8] = {[0 ... 7] = ' '};
	space[tabLength] = 0;
	oledPrintBuffer (oledConfig, space);
	oledGotoX (oledConfig, (uint8_t) (oledConfig->curXpos + tabLength));
	curBuf=token+1;
      }
    }
  }
}

void oledPrintBuffer (OledConfig *oledConfig, const char *buffer)
{
  RET_UNLESS_INIT(oledConfig);
  switch (oledConfig->deviceType) {
  case PICASO : 
    OLED ("%c%c%c%c%c%c", 0xff, 0xe9, 0x00, oledConfig->curYpos, 0x00, oledConfig->curXpos);
    OLED_KOF (KOF_INT16, "%c%c%s%c", 0x0, 0x18, buffer, 0x0);
  break;
  case GOLDELOX : 
    OLED ("%c%c%c%c%c%c", 0xff, 0xe4, 0x00, oledConfig->curYpos, 0x00, oledConfig->curXpos);
    OLED ("%c%c%s%c", 0x0, 0x06, buffer, 0x0);
  break;
  case TERM_VT100 : 
    sendVt100Seq (oledConfig->serial, "%d;%dH",  oledConfig->curYpos+1, oledConfig->curXpos+1);
    directchprintf (oledConfig->serial, buffer);
  } 
}

void oledChangeBgColor (OledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);
  
  const uint16_t oldCol = oledConfig->bg;
  const uint16_t newCol = oledConfig->bg = colorDecTo16b(r,g,b);
  RET_UNLESS_4DSYS(oledConfig);

  const uint8_t cmdBgCol =  ISPIC(oledConfig) ? 0xb4 : 0xbe;
  OLED ("%c%c%c%c%c%c", 0xff, cmdBgCol, 
	twoBytesFromWord(oldCol),
	twoBytesFromWord(newCol));
} 


void oledSetTextBgColor (OledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);
  const uint8_t cmdTbg =  ISPIC(oledConfig) ? 0xe6 : 0x7e;
  const KindOfCommand kof =  ISPIC(oledConfig) ? KOF_INT16 : KOF_ACK;

  switch (oledConfig->deviceType) {
  case PICASO : 
  case GOLDELOX : 
    oledConfig->tbg[0] = mkColor24(r,g,b);
    const uint16_t bg = colorDecTo16b(r,g,b);
    OLED_KOF (kof, "%c%c%c%c", 0xff, cmdTbg,
	      twoBytesFromWord(bg));
    break;
  case TERM_VT100 : 
    sendVt100Seq (oledConfig->serial, "48;2;%d;%d;%dm", r*255/100, g*255/100, b*255/100);
  } 
}

void oledSetTextFgColor (OledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);
  const uint8_t cmdTfg =  ISPIC(oledConfig) ? 0xe7 : 0x7f;
  const KindOfCommand kof =  ISPIC(oledConfig) ? KOF_INT16 : KOF_ACK;

  switch (oledConfig->deviceType) {
  case PICASO : 
  case GOLDELOX : 
    oledConfig->fg[0] = mkColor24(r,g,b);
    const uint16_t fg = colorDecTo16b(r,g,b);
    //    DebugTrace ("Color = 0x%x%x%x%x\r\n", 0xff, cmdTfg, twoBytesFromWord(fg));
    OLED_KOF (kof, "%c%c%c%c", 0xff, cmdTfg, twoBytesFromWord(fg));
    break;
  case TERM_VT100 : 
    sendVt100Seq (oledConfig->serial, "38;2;%d;%d;%dm", r*255/100, g*255/100, b*255/100);
  } 
}



void oledSetTextBgColorTable (OledConfig *oledConfig, uint8_t colorIndex, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);
  if (++colorIndex >= COLOR_TABLE_SIZE) return;

  oledConfig->tbg[colorIndex] = mkColor24(r,g,b);
}

void oledSetTextFgColorTable (OledConfig *oledConfig,  uint8_t colorIndex, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);
  if (++colorIndex >= COLOR_TABLE_SIZE) return;

  oledConfig->fg[colorIndex] = mkColor24(r,g,b);
}

void oledUseColorIndex (OledConfig *oledConfig, uint8_t colorIndex)
{
  RET_UNLESS_INIT(oledConfig);
  if (++colorIndex >= COLOR_TABLE_SIZE) return;

  if (oledConfig->deviceType != TERM_VT100) {
    if (oledConfig->fg[0].rgb != oledConfig->fg[colorIndex].rgb)  {
      oledSetTextFgColor (oledConfig, oledConfig->fg[colorIndex].r, 
			  oledConfig->fg[colorIndex].g, oledConfig->fg[colorIndex].b);
    }
    
    if (oledConfig->tbg[0].rgb != oledConfig->tbg[colorIndex].rgb)  {
      oledSetTextBgColor (oledConfig, oledConfig->tbg[colorIndex].r, 
			  oledConfig->tbg[colorIndex].g, oledConfig->tbg[colorIndex].b);
    }
  } else {
    oledSetTextFgColor (oledConfig, oledConfig->fg[colorIndex].r, 
			oledConfig->fg[colorIndex].g, oledConfig->fg[colorIndex].b);
    oledSetTextBgColor (oledConfig, oledConfig->tbg[colorIndex].r, 
			oledConfig->tbg[colorIndex].g, oledConfig->tbg[colorIndex].b);
  }
}


void oledSetTextOpacity (OledConfig *oledConfig, bool opaque)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

  const uint8_t cmdOpa =  ISPIC(oledConfig) ? 0xdf : 0x77;
  const KindOfCommand kof =  ISPIC(oledConfig) ? KOF_INT16 : KOF_ACK;
  OLED_KOF (kof, "%c%c%c%c", 0xff, cmdOpa, 0x00, opaque); 
}


void oledSetTextAttributeMask (OledConfig *oledConfig, enum OledTextAttribute attrib)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

  const uint8_t cmdAttrib =  ISPIC(oledConfig) ? 0xda : 0x72;
  const KindOfCommand kof =  ISPIC(oledConfig) ? KOF_INT16 : KOF_ACK;
  OLED_KOF (kof, "%c%c%c%c", 0xff, cmdAttrib, 0x00, attrib); 
}

void oledSetTextGap (OledConfig *oledConfig, uint8_t xgap, uint8_t ygap)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

  const uint8_t cmdXgap =  ISPIC(oledConfig) ? 0xe2 : 0x7a;
  const uint8_t cmdYgap =  ISPIC(oledConfig) ? 0xe1 : 0x79;
  const KindOfCommand kof =  ISPIC(oledConfig) ? KOF_INT16 : KOF_ACK;
  OLED_KOF (kof, "%c%c%c%c", 0xff, cmdXgap, 0x00, xgap); 
  OLED_KOF (kof, "%c%c%c%c", 0xff, cmdYgap, 0x00, ygap); 
}

void oledSetTextSizeMultiplier (OledConfig *oledConfig, uint8_t xmul, uint8_t ymul)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

  const uint8_t cmdXmul =  ISPIC(oledConfig) ? 0xe4 : 0x7c;
  const uint8_t cmdYmul =  ISPIC(oledConfig) ? 0xe3 : 0x7b;
  const KindOfCommand kof =  ISPIC(oledConfig) ? KOF_INT16 : KOF_ACK;
  OLED_KOF (kof, "%c%c%c%c", 0xff, cmdXmul, 0x00, xmul); 
  OLED_KOF (kof, "%c%c%c%c", 0xff, cmdYmul, 0x00, ymul); 

}

void oledSetScreenOrientation (OledConfig *oledConfig, enum OledScreenOrientation orientation)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

  const uint8_t cmdMode =  ISPIC(oledConfig) ? 0x9e : 0x68;
  const KindOfCommand kof =  ISPIC(oledConfig) ? KOF_INT16 : KOF_ACK;
  OLED_KOF (kof, "%c%c%c%c", 0xff, cmdMode, 0x00, orientation); 
}


void oledGotoXY (OledConfig *oledConfig, uint8_t x, uint8_t y)
{  
  RET_UNLESS_INIT(oledConfig);

   oledConfig->curXpos=x;
   oledConfig->curYpos=y;

   if (oledConfig->deviceType == TERM_VT100) {
     sendVt100Seq (oledConfig->serial, "%d;%dH", y+1,x+1);
   } 
}

void oledGotoX (OledConfig *oledConfig, uint8_t x)
{  
  RET_UNLESS_INIT(oledConfig);

  oledConfig->curXpos=x;
  if (oledConfig->deviceType == TERM_VT100) {
    sendVt100Seq (oledConfig->serial, "%d;%dH", oledConfig->curYpos+1,x+1);
  }
}

uint8_t oledGetX  (const OledConfig *oledConfig)
{  
  if (oledIsInitialised(oledConfig) == FALSE)
    return 0;
  
  return oledConfig->curXpos;
}

uint8_t oledGetY  (const OledConfig *oledConfig)
{  
  if (oledIsInitialised(oledConfig) == FALSE)
    return 0;
  
  return oledConfig->curYpos;
}

void oledGotoNextLine (OledConfig *oledConfig)
{  
  RET_UNLESS_INIT(oledConfig);

  oledConfig->curXpos=0;
  oledConfig->curYpos++;
  if (oledConfig->deviceType == TERM_VT100) {
    sendVt100Seq (oledConfig->serial, "B");
    //    chprintf (oledConfig->serial, "\r\n");
  }
}


bool oledIsCorrectDevice (OledConfig *oledConfig)
{
  if (oledIsInitialised(oledConfig) == FALSE) 
    return false;

  if (oledConfig->deviceType == TERM_VT100) 
    return true;

  const uint8_t cmdCls =  ISPIC(oledConfig) ? 0xcd : 0xd7;
  OLED ("%c%c", 0xff, cmdCls);
  return (oledConfig->response[0] == 0x06) ? true : false;
}

void oledClearScreen (OledConfig *oledConfig)
{
  RET_UNLESS_INIT(oledConfig);

  if (oledConfig->deviceType == TERM_VT100) {
    sendVt100Seq (oledConfig->serial, "2J");
  } else {
    const uint8_t cmdCls =  ISPIC(oledConfig) ? 0xcd : 0xd7;
    OLED ("%c%c", 0xff, cmdCls);
  }
}


void oledDrawPoint (OledConfig *oledConfig, const uint16_t x, const uint16_t y, 
		    const uint8_t colorIndex)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

  const uint8_t cmdDP =  ISPIC(oledConfig) ? 0xc1 : 0xcb;
  const uint16_t fg = fgColorIndexTo16b (oledConfig, (uint8_t) (colorIndex+1));

  OLED ("%c%c%c%c%c%c%c%c",
	0xff, cmdDP,
	twoBytesFromWord(x),
	twoBytesFromWord(y),
	twoBytesFromWord(fg));
}


void oledDrawLine (OledConfig *oledConfig, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2, 
		   const uint8_t colorIndex)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

  const uint8_t cmdDL =  ISPIC(oledConfig) ? 0xc8 : 0xd2;
  const uint16_t fg = fgColorIndexTo16b (oledConfig, (uint8_t) (colorIndex+1));

  OLED ("%c%c%c%c%c%c%c%c%c%c%c%c",
	0xff, cmdDL,
	twoBytesFromWord(x1),
	twoBytesFromWord(y1),
	twoBytesFromWord(x2),
	twoBytesFromWord(y2),
	twoBytesFromWord(fg));
}

void oledDrawRect (OledConfig *oledConfig, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2,
		   const bool filled,
		   const uint8_t colorIndex)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

  uint8_t cmdDR =  ISPIC(oledConfig) ? 0xc4 : 0xcf;
  if (filled) 
    cmdDR--;

  const uint16_t fg = fgColorIndexTo16b (oledConfig, (uint8_t) (colorIndex+1));

  OLED ("%c%c%c%c%c%c%c%c%c%c%c%c",
	0xff, cmdDR,
	twoBytesFromWord(x1),
	twoBytesFromWord(y1),
	twoBytesFromWord(x2),
	twoBytesFromWord(y2),
	twoBytesFromWord(fg));
}

/*
  API available for PICASO and DIABLO, not for GOLDELOX 

 */
void oledScreenCopyPaste (OledConfig *oledConfig, 
			  const uint16_t xs, const uint16_t ys, 
			  const uint16_t xd, const uint16_t yd,
			  const uint16_t width, const uint16_t height)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);
  if (oledConfig->deviceType != PICASO) {
    return;
  }

  const uint8_t cmdDR =  0xad;

  OLED ("%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
	0xff, cmdDR,
	twoBytesFromWord(xs),
	twoBytesFromWord(ys),
	twoBytesFromWord(xd),
	twoBytesFromWord(yd),
	twoBytesFromWord(width),
	twoBytesFromWord(height));
}


void oledEnableTouch (OledConfig *oledConfig, bool enable)
{
   RET_UNLESS_INIT(oledConfig);

  if (oledConfig->deviceType != PICASO) {
    return;
  }
  
  OLED ("%c%c%c%c", 0xff, 0x38, 0x00, enable ? 0x00 : 0x01);
}

static uint16_t oledTouchGet (OledConfig *oledConfig, uint16_t mode)
{
  if (oledIsInitialised(oledConfig) == FALSE) 
    return 0xff;
  
  if (oledConfig->deviceType != PICASO) {
    return 0xff;
  }
  
  OLED_KOF (KOF_INT16, "%c%c%c%c", 0xff, 0x37, twoBytesFromWord(mode));
  uint16_t ret = (uint16_t) ((oledConfig->response[1] << 8) | oledConfig->response[2]);
  return ret;
}


uint16_t oledTouchGetStatus (OledConfig *oledConfig)
{
  return oledTouchGet (oledConfig, 0);
}

uint16_t oledTouchGetXcoord (OledConfig *oledConfig)
{
  return oledTouchGet (oledConfig, 1);
}

uint16_t oledTouchGetYcoord (OledConfig *oledConfig)
{
  return oledTouchGet (oledConfig, 2);
}




bool oledInitSdCard (OledConfig *oledConfig)
{ 
  if (oledIsInitialised(oledConfig) == FALSE) return FALSE;
  if (oledConfig->deviceType != PICASO) return FALSE;

  OLED_KOF (KOF_INT16, "%c%c", 0xff, 0x89);
  if (oledConfig->response[2] != 0) {
    // card is present, mount fat16 fs
    DebugTrace ("oledInitSdCard sd presence=%d", oledConfig->response[2]);
    OLED_KOF (KOF_INT16, "%c%c", 0xff, 0x03);
    DebugTrace ("oledInitSdCard fat16 mount=%d", oledConfig->response[2]);
  }
  
  return (oledConfig->response[2] != 0);
}

void oledListSdCardDirectory (OledConfig *oledConfig)
{
  bool remainFile=TRUE;
  uint32_t fileNo=0;

  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_PICASO(oledConfig);

  while (remainFile) {
    if (fileNo++ == 0) 
      // find first file and report
      remainFile = OLED_KOF (KOF_INT16LENGTH_THEN_DATA, "%c%c*.*%c", 0x00, 0x24, 0x0);
    else
      // find next file and report
      remainFile = OLED_KOF (KOF_INT16LENGTH_THEN_DATA, "%c%c", 0x00, 0x25);
    
    if (remainFile) {
      DebugTrace ("File [%lu] = %s", fileNo, oledConfig->response);
    }
  }
}


void oledSetSoundVolume (OledConfig *oledConfig, uint8_t percent)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_PICASO(oledConfig);
  percent = MIN (100, percent);
  percent = (uint8_t) (percent + 27);
  OLED ("%c%c%c%c", 0xff, 0x00, 0x00, percent);
}

void oledPlayWav (OledConfig *oledConfig, const char* fileName)
{  
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_PICASO(oledConfig);
  OLED_KOF (KOF_INT16, "%c%c%s%c", 0x0, 0x0b, fileName, 0x0);
}

// No buzzer on our OLED-96-G2
/* void oledPlayBeep (OledConfig *oledConfig, uint8_t note, uint16_t duration) */
/* {   */
/*   RET_UNLESS_INIT(oledConfig); */
/*   RET_UNLESS_GOLDELOX(oledConfig); */
/*   OLED ("%c%c%c%c%c%c", 0xff, 0xda,  */
/* 	0, note, twoBytesFromWord(duration)); */
/* } */

uint32_t oledOpenFile (OledConfig *oledConfig, const char* fileName, uint16_t *handle)
{
  if (oledIsInitialised(oledConfig) == FALSE) return 0;
  if (oledConfig->deviceType != PICASO) return 0;

  OLED_KOF (KOF_INT16, "%c%c%s%c%c", 0x0, 0x0a, fileName, 0x0, 'r' /* READ */);
  *handle = getResponseAsUint16 (oledConfig);
  return oledGetFileError (oledConfig);
}


void oledCloseFile (OledConfig *oledConfig, const uint16_t handle)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_PICASO(oledConfig);
   const union {
     uint16_t val;
     uint8_t  buf [sizeof(uint16_t)] ;
   } hndl = {.val = handle};

   OLED_KOF (KOF_INT16, "%c%c%c%c", 0xff, 0x18,  hndl.buf[1], hndl.buf[0]);
}



void oledDisplayGci (OledConfig *oledConfig, const uint16_t handle, uint32_t offset)
{
   RET_UNLESS_INIT(oledConfig);
   RET_UNLESS_PICASO(oledConfig);

   const union {
     uint16_t val;
     uint8_t  buf [sizeof(uint16_t)] ;
   }  hndl = {.val = handle};

   if (oledFileSeek (oledConfig, handle, offset)) {
     OLED_KOF (KOF_INT16, "%c%c%c%c%c%c%c%c", 0xff, 0x11, 0x0, 0x0, 0x0, 0x0, hndl.buf[1], hndl.buf[0]);

     if (oledConfig->response[2]) {
       DebugTrace ("oledDisplayGci error %d", oledConfig->response[2]);
     }
   } else {
     DebugTrace ("oledDisplayGci oledFileSeek error");
   }
}





/*
#                 _ __           _                    _                   
#                | '_ \         (_)                  | |                  
#                | |_) |  _ __   _   __   __   __ _  | |_     ___         
#                | .__/  | '__| | |  \ \ / /  / _` | | __|   / _ \        
#                | |     | |    | |   \ V /  | (_| | \ |_   |  __/        
#                |_|     |_|    |_|    \_/    \__,_|  \__|   \___|        
*/
/* static bool oled (uint32_t respLen, const char *fmt, ...) */
/* { */
/*   va_list ap; */
/*     va_start(ap, fmt); */
/*   chvsnprintf(buffer, size, fmt, ap);  */
/*   va_end(ap); */
/* } */

static uint16_t getResponseAsUint16 (OledConfig *oledConfig)
{
  return (uint16_t) ((oledConfig->response[1] << 8) | oledConfig->response[2]);
}

static  uint16_t fgColorIndexTo16b (const OledConfig *oledConfig, const uint8_t colorIndex) {
  const Color24 fg = oledConfig->fg[colorIndex];
  
  return (colorDecTo16b(fg.r, fg.g, fg.b));
}

static void oledTrace (OledConfig *oledConfig, const char* err)
{
  static uint32_t errCount=0;

  //  DebugTrace (err);
  if (strcmp (err, "NACK") != 0) {
    if (errCount++ == 5) {
      errCount = 0;
      oledReInit (oledConfig);
    }
  }
}

static void oledScreenSaverTimout (OledConfig *oledConfig, uint16_t timout)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_GOLDELOX(oledConfig);

  OLED ("%c%c%c%c", 0x00, 0x0c, twoBytesFromWord(timout));
}

static uint32_t oledGetFileError  (OledConfig *oledConfig)
{
  if (oledIsInitialised(oledConfig) == FALSE) return 0;
  if (oledConfig->deviceType != PICASO) return 0;

  OLED_KOF (KOF_INT16, "%c%c", 0xff, 0x1f);

  if (oledConfig->response[2]) {
    DebugTrace ("oledGetFileError error %d", oledConfig->response[2]);
  }

  return oledConfig->response[2];
}

static bool oledFileSeek  (OledConfig *oledConfig, const uint16_t handle, const uint32_t offset)
{
  if (oledIsInitialised(oledConfig) == FALSE) return FALSE;
  if (oledConfig->deviceType != PICASO) return FALSE;

  const union {
    uint32_t val;
    uint8_t  buf [sizeof(uint32_t)] ;
  } ofst = {.val = offset};

  const union {
    uint16_t val;
    uint8_t  buf [sizeof(uint16_t)] ;
  } hndl  = {.val = handle};

  OLED_KOF (KOF_INT16, "%c%c%c%c%c%c%c%c", 0xff, 0x16, hndl.buf[1], hndl.buf[0],
	    ofst.buf[3], ofst.buf[2], ofst.buf[1], ofst.buf[0]);
  
  return oledConfig->response[2] == 1;
}

static uint32_t oledReceiveAnswer (OledConfig *oc, const uint32_t size,
				   const char* fct, const uint32_t line)
{
  (void) fct;
  (void) line;

  BaseChannel *serial =  (BaseChannel *)  oc->serial;
  uint8_t *response = oc->response;

  if (size > sizeof (oc->response)) {
    oledTrace (oc, "oledConfig->response buffer too small");
    return 0;
  }
  

  const uint32_t ret = chnReadTimeout (serial, response, size, readTimout);
  if (ret != size) {
#if defined LCD_240_320 || defined LCD_240_400
    hardwareSetState (HW_uart1, FALSE);
#endif
    DebugTrace ("oledReceiveAnswer ret[%lu] != expectedSize[%lu] @%s : line %lu", ret, size, fct, line);
    oledTrace (oc, "LCD Protocol error");
    oledStatus = OLED_ERROR;
  } else if (response[0] != 0x6) {
#if defined LCD_240_320 || defined LCD_240_400
    hardwareSetState (HW_uart1, FALSE);
#endif
    DebugTrace ("oledReceiveAnswer get NACK [%d] @%s : line %lu XY=[%d,%d]", response[0], fct, line,
		oc->curXpos, oc->curYpos);
    oledTrace (oc, "NACK");
    oledStatus = OLED_ERROR;
  } else {
#if defined LCD_240_320 || defined LCD_240_400
    hardwareSetState (HW_uart1, TRUE);
#endif
    oledStatus = OLED_OK;
  }
  return ret;
}


static uint32_t oledSendCommand (OledConfig *oc, KindOfCommand kof, 
				 const char* fct, const uint32_t line, const char *fmt, ...)
{
  va_list ap;
  uint32_t ret=0;
  
  BaseChannel * serial =  (BaseChannel *)  oc->serial;
  uint8_t *response = oc->response;
  const  uint32_t respBufferSize = sizeof (oc->response);
  
  // purge read buffer
  chnReadTimeout (serial, response, respBufferSize, TIME_IMMEDIATE);
  
  // send command
  va_start(ap, fmt);
  directchvprintf(oc->serial, fmt, ap); 
  va_end(ap);
  
  // get response
  switch (kof) {
  case KOF_NONE :
    return 0;
    break;
    
  case  KOF_ACK:
    ret = oledReceiveAnswer (oc, 1, fct, line);
    break;
    
  case  KOF_INT16:
    ret = oledReceiveAnswer (oc, 3, fct, line);
    break;
    
  case KOF_INT16LENGTH_THEN_DATA:
    ret = oledReceiveAnswer (oc, 3, fct, line);
    if (ret == 3) {
      // little endianness
      const uint32_t len = getResponseAsUint16 (oc);
      const uint32_t size = MIN(len, respBufferSize-1);
      
      //DebugTrace ("receiveLen=%d constrainedSize=%d", len, size);
      if (size) {
	ret = chnReadTimeout (serial, response, size, readTimout);
	if (ret != size) 
	  oledTrace (oc, "KOF_INT16LENGTH_THEN_DATA: OLED Protocol error");
	else 
	  response[ret] = 0; // NULL string termination 
      } else {
	ret = 0;
      }
    }
    break;
  }
  
  
  return ret;
}

OledStatus oledGetStatus(void)
{
  return oledStatus;
}


static void sendVt100Seq (BaseSequentialStream *serial, const char *fmt, ...)
{
  char buffer[80] = {0x1b, '['};
  va_list ap;
  
  va_start(ap, fmt);
  chvsnprintf(&(buffer[2]), sizeof (buffer)-2, fmt, ap); 
  va_end(ap);

  directchprintf (serial, buffer);
}

