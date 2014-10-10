#include <ch.h>
#include <hal.h>
#include <string.h>
#include <ctype.h>
#include "globalVar.h"
#include "stdutil.h"
#include "printf.h"
#include "picaso4Display.h"

#if defined LCD_240_320 || defined LCD_240_400
#include "hardwareTest.h"
#endif


#define clampColor(r,v,b) ((r & 0x1f) <<11 | (v & 0x3f) << 5 | (b & 0x1f))
#define colorDecTo16b(r,v,b) (clampColor((r*31/100), (v*63/100), (b*31/100)))
#define twoBytesFromWord(x) ((x & 0xff00) >> 8), (x & 0xff)

typedef enum {KOF_NONE, KOF_ACK, KOF_INT16, 
	      KOF_INT16LENGTH_THEN_DATA} KindOfCommand;

const uint32_t readTimout = 1000;

/* #define OLED(n, ...) (\ */
/* 		      chnReadTimeout (((BaseChannel *) oled), response, sizeof (response), 10), \ */
/* 		      chprintf (oled, __VA_ARGS__),			\ */
/* 		      n == 0 ? 0 : chnReadTimeout (((BaseChannel *) oled), response, MIN(sizeof (response), n), 100)\ */
/* ) */

static void oledTrace (oledConfig *oledConfig, const char* err);
static uint32_t oledSendCommand (struct oledConfig *oc, KindOfCommand kof, 
				 const char* fct, const uint32_t line, const char *fmt, ...);

static uint32_t oledReceiveAnswer (struct oledConfig *oc, const uint32_t size,
				   const char* fct, const uint32_t line);
static void sendVt100Seq (BaseSequentialStream *serial, const char *fmt, ...);


#define OLED(...) (oledSendCommand (oledConfig, KOF_ACK, __FUNCTION__, __LINE__, __VA_ARGS__))
#define OLED_KOF(k, ...) (oledSendCommand (oledConfig, k,__FUNCTION__, __LINE__,  __VA_ARGS__))


#define RET_UNLESS_INIT(oledCfg)    {if (oledIsInitialised(oledCfg) == FALSE) return;}
#define RET_UNLESS_PICASO(oledCfg)  {if (oledConfig->deviceType != PICASO) return;}

static bool_t oledIsInitialised (oledConfig *oledConfig) ;
static void oledSetBaud (oledConfig *oledConfig, uint32_t baud);
static void oledPreInit (oledConfig *oledConfig, uint32_t baud);
static void oledReInit (oledConfig *oledConfig);
static uint32_t oledGetFileError  (oledConfig *oledConfig);
static bool_t oledFileSeek  (oledConfig *oledConfig, const uint16_t handle, const uint32_t offset);
static uint16_t fgColorIndexTo16b (const oledConfig *oledConfig, const uint8_t index);
static uint16_t oledTouchGet (oledConfig *oledConfig, uint16_t mode);

static void oledPreInit (oledConfig *oledConfig, uint32_t baud)
{
  oledConfig->bg = colorDecTo16b(0,0,0);
  oledConfig->tbg[0] = mkColor24 (0,0,0);
  oledConfig->fg[0] = mkColor24(50,50,50);
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
	       GPIO_TypeDef *rstGpio, uint32_t rstPin, enum OledConfig_Device dev)
{
  oledConfig->rstGpio = rstGpio;
  oledConfig->rstPin = rstPin;
  oledConfig->deviceType = dev;

  oledHardReset (oledConfig);


  oledPreInit (oledConfig, 9600);
  oledConfig->serial = (BaseSequentialStream *) oled;
  chMtxInit(&(oledConfig->omutex));
  sdStart(oled, &(oledConfig->serialConfig));
  chThdSleepMilliseconds(10);

  // opaque background
  if (oledConfig->deviceType != TERM_VT100) {
    OLED_KOF (KOF_INT16, "%c%c%c%c", 0xff, 0xdf, 0x00, 0x01); 
  }

  oledClearScreen (oledConfig);
  
  // use greater speed
  if (baud != 9600) 
    oledSetBaud (oledConfig, baud);
}

static void oledReInit (oledConfig *oledConfig)
{
  RET_UNLESS_PICASO(oledConfig);
  oledHardReset (oledConfig);
  const uint32_t baud = oledConfig->serialConfig.speed;

  oledSetBaud (oledConfig, 9600);

  // opaque background
  OLED_KOF (KOF_INT16, "%c%c%c%c", 0xff, 0xdf, 0x00, 0x01); 
  oledClearScreen (oledConfig);
  
  // use greater speed
  if (baud != 9600) 
    oledSetBaud (oledConfig, baud);
}


void oledHardReset (oledConfig *oledConfig)
{

  RET_UNLESS_PICASO(oledConfig);

  palClearPad (oledConfig->rstGpio, oledConfig->rstPin);
  chThdSleepMilliseconds(10);
  palSetPad (oledConfig->rstGpio, oledConfig->rstPin);
  chThdSleepMilliseconds(3500);
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

void oledGetVersion (oledConfig *oledConfig, char *buffer, const size_t buflen)
{
  RET_UNLESS_PICASO(oledConfig);

   // get display model6
  OLED_KOF (KOF_INT16LENGTH_THEN_DATA, "%c%c", 0x00, 0x1a);  
  strncpy (buffer, (const char *) oledConfig->response, buflen);

  // get Pmmc version
  OLED_KOF (KOF_INT16, "%c%c", 0x00, 0x1c);
  const uint16_t pmmc = getResponseAsUint16(oledConfig);
  const uint8_t pmmcMajor = pmmc/256;
  const uint8_t pmmcMinor = pmmc%256;
  chsnprintf (&buffer[strlen(buffer)], buflen-strlen(buffer), " Pmmc=%d.%d", pmmcMajor, pmmcMinor);

  // get SPE version
  OLED_KOF (KOF_INT16, "%c%c", 0x00, 0x1b);
  const uint16_t spe = getResponseAsUint16(oledConfig);
  const uint8_t speMajor = spe/256;
  const uint8_t speMinor = spe%256;
  chsnprintf (&buffer[strlen(buffer)], buflen-strlen(buffer), " Spe=%d.%d", speMajor, speMinor);
}

void oledSetBaud (oledConfig *oledConfig, uint32_t baud)
{
  uint8_t baudCode ;
  uint32_t actualbaudRate; 

  struct SerialDriver *sd = (struct SerialDriver *) oledConfig->serial;

  RET_UNLESS_INIT(oledConfig);

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

  //  oledConfig->serialConfig.speed = baud < actualbaudRate ? baud : actualbaudRate; // Test avec actualbaudRate
  oledConfig->serialConfig.speed = actualbaudRate; 

  // send command, do not wait response
  if (oledConfig->deviceType == PICASO) {
    OLED_KOF (KOF_NONE, "%c%c%c%c", 0x0, 0x26, 0x0, baudCode);
    chThdSleepMilliseconds(10);
  }
  sdStop (sd);
  sdStart (sd, &(oledConfig->serialConfig));

  // wait 150ms, and wait for response at new speed
  chThdSleepMilliseconds(150);
  if (oledConfig->deviceType == PICASO) {
    oledReceiveAnswer (oledConfig, 1, __FUNCTION__, __LINE__);
  }
}

/* void oledPrintFmt (oledConfig *oledConfig, const char *fmt, ...) */
/* { */
/*   char buffer[80]; */
/*   char *curStrPtr, *token, *savePtr; */

/*   va_list ap; */
/*   RET_UNLESS_INIT(oledConfig); */

/*   va_start(ap, fmt); */
/*   chvsnprintf(buffer, sizeof(buffer), fmt, ap);  */
/*   va_end(ap); */

/*   // replace escape color sequence by color command for respective backend */
/*   // ESC c 0 à 9 : couleur index of background and foreground */
/*   for (curStrPtr=buffer; ; curStrPtr=NULL) { */
/*     if ((token = strtok_r (curStrPtr, "\033", &savePtr)) == NULL) */
/*       break; */

/*     //    DebugTrace ("Token [-1:%d] [0:%d] [1:%d]",  *(token-1), *(token), *(token+1)); */

/*     if ((token > buffer) &&  (*(token-1) == 0) &&  */
/* 	(tolower((uint32_t) (*(token++))) == 'c')) { */
/*       const int32_t colorIndex = INRANGE(0, COLOR_TABLE_SIZE-1, *(token++) - '0'); */
/*       oledUseColorIndex (oledConfig, colorIndex); */
/*       DebugTrace ("useColorIndex %d", colorIndex); */
/*     } */
/*     oledPrintBuffer (oledConfig, token); */
/*     oledConfig->curXpos += strnlen (token, sizeof(buffer)); */
/*   } */
/* } */

void oledPrintFmt (oledConfig *oledConfig, const char *fmt, ...)
{
  char buffer[80];
  char *token, *curBuf;
  bool_t lastLoop=FALSE;

  va_list ap;
  RET_UNLESS_INIT(oledConfig);

  va_start(ap, fmt);
  chvsnprintf(buffer, sizeof(buffer), fmt, ap);
  va_end(ap);

  if (buffer[0] == 0)
    return;

  const char* endPtr = &(buffer[strnlen(buffer, sizeof (buffer)) -1]);
  // replace escape color sequence by color command for respective backend
  // ESC c 0 à 9 : couleur index of background and foreground
  // replace escape n by carriage return, line feed
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
      oledConfig->curXpos += strnlen (curBuf, sizeof(buffer));
    }
    
    if (lastLoop == FALSE) {
      // next two char a color coding scheme
      if (tolower((uint32_t) (*token)) == 'c') { 
	const int32_t colorIndex = *++token - '0';
	oledUseColorIndex (oledConfig, colorIndex);
	//	DebugTrace ("useColorIndex %d", colorIndex);
	curBuf=token+1;
      } else if (tolower((uint32_t) (*token)) == 'n') { 	
	//	DebugTrace ("carriage return");
	oledGotoXY (oledConfig, 0, oledConfig->curYpos+1);
	curBuf=token+1;
      }
    }
  }
}

void oledPrintBuffer (oledConfig *oledConfig, const char *buffer)
{
  RET_UNLESS_INIT(oledConfig);

  if (oledConfig->deviceType == PICASO) {
    OLED ("%c%c%c%c%c%c", 0xff, 0xe9, 0x00, oledConfig->curYpos, 0x00, oledConfig->curXpos);
    OLED_KOF (KOF_INT16, "%c%c%s%c", 0x0, 0x18, buffer, 0x0);
  } else {
    chprintf (oledConfig->serial, buffer);
    chprintf (oledConfig->serial, "\r\n");
  }
}

void oledChangeBgColor (oledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);
  
  const uint16_t oldCol = oledConfig->bg;
  const uint16_t newCol = oledConfig->bg = colorDecTo16b(r,g,b);
  OLED ("%c%c%c%c%c%c", 0xff, 0xb4, 
	twoBytesFromWord(oldCol),
	twoBytesFromWord(newCol));
} 


void oledSetTextBgColor (oledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);

  if (oledConfig->deviceType == PICASO) {
    oledConfig->tbg[0] = mkColor24(r,g,b);
    const uint16_t bg = colorDecTo16b(r,g,b);
    OLED_KOF (KOF_INT16, "%c%c%c%c", 0xff, 0xe6,
	      twoBytesFromWord(bg));
  } else if (oledConfig->deviceType == TERM_VT100) {
    sendVt100Seq (oledConfig->serial, "48;2;%d;%d;%dm", r*255/100, g*255/100, b*255/100);
  }
}

void oledSetTextFgColor (oledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);


  if (oledConfig->deviceType == PICASO) {
  oledConfig->fg[0] =  mkColor24(r,g,b);
  const uint16_t fg = colorDecTo16b(r,g,b);
  OLED_KOF (KOF_INT16, "%c%c%c%c", 0xff, 0xe7,
	      twoBytesFromWord(fg));
  } else if (oledConfig->deviceType == TERM_VT100) {
    sendVt100Seq (oledConfig->serial, "38;2;%d;%d;%dm", r*255/100, g*255/100, b*255/100);
  }
}


void oledSetTextBgColorTable (oledConfig *oledConfig, uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);
  if (++index >= COLOR_TABLE_SIZE) return;

  oledConfig->tbg[index] = mkColor24(r,g,b);
}

void oledSetTextFgColorTable (oledConfig *oledConfig,  uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);
  if (++index >= COLOR_TABLE_SIZE) return;

  oledConfig->fg[index] = mkColor24(r,g,b);
}

void oledUseColorIndex (oledConfig *oledConfig, uint8_t index)
{
  RET_UNLESS_INIT(oledConfig);
  if (++index >= COLOR_TABLE_SIZE) return;

  if (oledConfig->deviceType == PICASO) {
    if (oledConfig->fg[0].rgb != oledConfig->fg[index].rgb)  {
      oledSetTextFgColor (oledConfig, oledConfig->fg[index].r, 
			  oledConfig->fg[index].g, oledConfig->fg[index].b);
    }
    
    if (oledConfig->tbg[0].rgb != oledConfig->tbg[index].rgb)  {
      oledSetTextBgColor (oledConfig, oledConfig->tbg[index].r, 
			  oledConfig->tbg[index].g, oledConfig->tbg[index].b);
    }
  } else {
    oledSetTextFgColor (oledConfig, oledConfig->fg[index].r, 
			oledConfig->fg[index].g, oledConfig->fg[index].b);
    oledSetTextBgColor (oledConfig, oledConfig->tbg[index].r, 
			oledConfig->tbg[index].g, oledConfig->tbg[index].b);
  }
}





void oledGotoXY (oledConfig *oledConfig, uint8_t x, uint8_t y)
{  
  RET_UNLESS_INIT(oledConfig);

   oledConfig->curXpos=x;
   oledConfig->curYpos=y;

   if (oledConfig->deviceType == TERM_VT100) {
     sendVt100Seq (oledConfig->serial, "%d;%dH", y+1,x+1);
   } 
}

void oledGotoX (oledConfig *oledConfig, uint8_t x)
{  
  RET_UNLESS_INIT(oledConfig);

  oledConfig->curXpos=x;
  if (oledConfig->deviceType == TERM_VT100) {
    sendVt100Seq (oledConfig->serial, "%d;%dH", oledConfig->curYpos+1,x+1);
  }
}

void oledGotoNextLine (oledConfig *oledConfig)
{  
  RET_UNLESS_INIT(oledConfig);

  oledConfig->curXpos=0;
  oledConfig->curYpos++;
  if (oledConfig->deviceType == TERM_VT100) {
    sendVt100Seq (oledConfig->serial, "B");
    //    chprintf (oledConfig->serial, "\r\n");
  }
}

void oledClearScreen (oledConfig *oledConfig)
{
  RET_UNLESS_INIT(oledConfig);

  if (oledConfig->deviceType == TERM_VT100) {
    sendVt100Seq (oledConfig->serial, "2J");
  } else {
    OLED ("%c%c", 0xff, 0xcd);
  }
}


void oledDrawPoint (oledConfig *oledConfig, const uint16_t x, const uint16_t y, 
		    const uint8_t index)
{
  RET_UNLESS_INIT(oledConfig);

  if (oledConfig->deviceType != PICASO) {
    return;
  }

  const uint16_t fg = fgColorIndexTo16b (oledConfig, index+1);

  OLED ("%c%c%c%c%c%c%c%c",
	0xff, 0xc1,
	twoBytesFromWord(x),
	twoBytesFromWord(y),
	twoBytesFromWord(fg));
}


void oledDrawLine (oledConfig *oledConfig, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2, 
		   const uint8_t index)
{
  RET_UNLESS_INIT(oledConfig);

  if (oledConfig->deviceType != PICASO) {
    return;
  }
  

  const uint16_t fg = fgColorIndexTo16b (oledConfig, index+1);

  OLED ("%c%c%c%c%c%c%c%c%c%c%c%c",
	0xff, 0xc8,
	twoBytesFromWord(x1),
	twoBytesFromWord(y1),
	twoBytesFromWord(x2),
	twoBytesFromWord(y2),
	twoBytesFromWord(fg));
}

void oledDrawRect (oledConfig *oledConfig, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2,
		   const bool_t filled,
		   const uint8_t index)
{
  RET_UNLESS_INIT(oledConfig);

  if (oledConfig->deviceType != PICASO) {
    return;
  }
  

  const uint16_t fg = fgColorIndexTo16b (oledConfig, index+1);

  OLED ("%c%c%c%c%c%c%c%c%c%c%c%c",
	0xff, filled ? 0xc4 : 0xc5,
	twoBytesFromWord(x1),
	twoBytesFromWord(y1),
	twoBytesFromWord(x2),
	twoBytesFromWord(y2),
	twoBytesFromWord(fg));
}

void oledEnableTouch (oledConfig *oledConfig, bool_t enable)
{
   RET_UNLESS_INIT(oledConfig);

  if (oledConfig->deviceType != PICASO) {
    return;
  }
  
  OLED ("%c%c%c%c", 0xff, 0x38, 0x00, enable ? 0x00 : 0x01);
}

static uint16_t oledTouchGet (oledConfig *oledConfig, uint16_t mode)
{
  if (oledIsInitialised(oledConfig) == FALSE) 
    return 0xff;
  
  if (oledConfig->deviceType != PICASO) {
    return 0xff;
  }
  
  OLED_KOF (KOF_INT16, "%c%c%c%c", 0xff, 0x37, twoBytesFromWord(mode));
  uint16_t ret = (oledConfig->response[1] << 8) | oledConfig->response[2];
  return ret;
}


uint16_t oledTouchGetStatus (oledConfig *oledConfig)
{
  return oledTouchGet (oledConfig, 0);
}

uint16_t oledTouchGetXcoord (oledConfig *oledConfig)
{
  return oledTouchGet (oledConfig, 1);
}

uint16_t oledTouchGetYcoord (oledConfig *oledConfig)
{
  return oledTouchGet (oledConfig, 2);
}




bool_t oledInitSdCard (oledConfig *oledConfig)
{ 
  if (oledIsInitialised(oledConfig) == FALSE) return FALSE;
  if (oledConfig->deviceType != PICASO) return FALSE;

  OLED_KOF (KOF_INT16, "%c%c", 0xff, 0x89);
  if (oledConfig->response[2] != 0) {
    // card is present, mount fat16 fs
    DebugTrace ("oledInitSdCard sd presence=%d", oledConfig->response[2]);
    OLED_KOF (KOF_INT16, "%c%c", 0xff, 0x03);
    DebugTrace ("oledInitSdCard fat16 mount=%d", oledConfig->response[2])
  }
  
  return (oledConfig->response[2] != 0);
}

void oledListSdCardDirectory (oledConfig *oledConfig)
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
      DebugTrace ("File [%d] = %s", fileNo, oledConfig->response);
    }
  }
}


void oledSetSoundVolume (oledConfig *oledConfig, uint8_t percent)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_PICASO(oledConfig);
  percent = MIN (100, percent);
  percent +=27;
  OLED ("%c%c%c%c", 0xff, 0x00, 0x00, percent);
}

void oledPlayWav (oledConfig *oledConfig, const char* fileName)
{  
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_PICASO(oledConfig);
  OLED_KOF (KOF_INT16, "%c%c%s%c", 0x0, 0x0b, fileName, 0x0);
}

uint32_t oledOpenFile (oledConfig *oledConfig, const char* fileName, uint16_t *handle)
{
  if (oledIsInitialised(oledConfig) == FALSE) return 0;
  if (oledConfig->deviceType != PICASO) return 0;

  OLED_KOF (KOF_INT16, "%c%c%s%c%c", 0x0, 0x0a, fileName, 0x0, 'r' /* READ */);
  *handle = getResponseAsUint16 (oledConfig);
  return oledGetFileError (oledConfig);
}


void oledCloseFile (oledConfig *oledConfig, const uint16_t handle)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_PICASO(oledConfig);
   const union {
     uint16_t val;
     uint8_t  buf [sizeof(uint16_t)] ;
   } hndl = {.val = handle};

   OLED_KOF (KOF_INT16, "%c%c%c%c", 0xff, 0x18,  hndl.buf[1], hndl.buf[0]);
}



void oledDisplayGci (oledConfig *oledConfig, const uint16_t handle, uint32_t offset)
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



uint16_t getResponseAsUint16 (oledConfig *oledConfig)
{
  return (oledConfig->response[1] << 8) | oledConfig->response[2];
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

static  uint16_t fgColorIndexTo16b (const oledConfig *oledConfig, const uint8_t index) {
  const Color24 fg = oledConfig->fg[index];
  
  return (colorDecTo16b(fg.r, fg.g, fg.b));
}

static void oledTrace (oledConfig *oledConfig, const char* err)
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

static uint32_t oledGetFileError  (oledConfig *oledConfig)
{
  if (oledIsInitialised(oledConfig) == FALSE) return 0;
  if (oledConfig->deviceType != PICASO) return 0;

  OLED_KOF (KOF_INT16, "%c%c", 0xff, 0x1f);

  if (oledConfig->response[2]) {
    DebugTrace ("oledGetFileError error %d", oledConfig->response[2]);
  }

  return oledConfig->response[2];
}

static bool_t oledFileSeek  (oledConfig *oledConfig, const uint16_t handle, const uint32_t offset)
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

static uint32_t oledReceiveAnswer (struct oledConfig *oc, const uint32_t size,
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
    DebugTrace ("oledReceiveAnswer ret[%d] != expectedSize[%d] @%s : line %d", ret, size, fct, line);
    oledTrace (oc, "LCD Protocol error");
  } else if (response[0] != 0x6) {
#if defined LCD_240_320 || defined LCD_240_400
    hardwareSetState (HW_uart1, FALSE);
#endif
    DebugTrace ("oledReceiveAnswer get NACK [%d] @%s : line %d XY=[%d,%d]", response[0], fct, line,
		oc->curXpos, oc->curYpos);
    oledTrace (oc, "NACK");
  } else {
#if defined LCD_240_320 || defined LCD_240_400
    hardwareSetState (HW_uart1, TRUE);
#endif
  }
  return ret;
}


static uint32_t oledSendCommand (struct oledConfig *oc, KindOfCommand kof, 
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
  chvprintf(oc->serial, fmt, ap); 
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


static void sendVt100Seq (BaseSequentialStream *serial, const char *fmt, ...)
{
  char buffer[80] = {0x1b, '['};
  va_list ap;
  
  va_start(ap, fmt);
  chvsnprintf(&(buffer[2]), sizeof (buffer)-2, fmt, ap); 
  va_end(ap);

  chprintf (serial, buffer);
}
