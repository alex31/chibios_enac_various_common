#include <ch.h>
#include <hal.h>
#include <string.h>
#include <ctype.h>
#include "stdutil.h"
#include "printf.h"
#include "picaso4Display.h"
#include "picaso4Display_ll.h"


#define clampColor(r,v,b) ((uint16_t) ((r & 0x1f) <<11 | (v & 0x3f) << 5 | (b & 0x1f)))
#define colorDecTo16b(r,v,b) (clampColor((r*31/100), (v*63/100), (b*31/100)))
#define QDS_ACK 0x6

typedef enum {KOF_NONE, KOF_ACK, KOF_INT16, 
	      KOF_INT16LENGTH_THEN_DATA} KindOfCommand;

const uint32_t readTimout = TIME_MS2I(500);
static OledStatus oledStatus = OLED_OK;


static uint32_t oledTransmitBuffer (OledConfig *oc, const char* fct, const uint32_t line,
					 const uint8_t *outBuffer, const size_t outSize,
					 uint8_t *inBuffer, const size_t inSize);
#if PICASO_DISPLAY_USE_SD
static uint32_t oledReceiveAnswer (OledConfig *oc, uint8_t *response, const size_t size,
				   const char* fct, const uint32_t line);
static void sendVt100Seq (BaseSequentialStream *serial, const char *fmt, ...);
#else
static void sendVt100Seq (UARTDriver *serial, const char *fmt, ...);
static void oledStartReceiveAnswer (OledConfig *oc, uint8_t *response, const size_t size,
				     const char* fct, const uint32_t line);
static uint32_t oledWaitReceiveAnswer (OledConfig *oc, size_t *size,
					const char* fct, const uint32_t line);
#endif
static bool findDeviceType (OledConfig *oledConfig);

#if CH_DBG_ENABLE_ASSERTS 
#define RET_UNLESS_INIT(oledCfg)    {if (oledIsInitialised(oledCfg) == false) return ;}
#define RET_UNLESS_4DSYS(oledCfg)  {if (oledCfg->deviceType == TERM_VT100) return ;}
#define RET_UNLESS_GOLDELOX(oledCfg)  {if (oledCfg->deviceType != GOLDELOX) return ;}

#define RET_UNLESS_INIT_BOOL(oledCfg)    {if (oledIsInitialised(oledCfg) == false) return false;}
#define RET_UNLESS_4DSYS_BOOL(oledCfg)  {if (oledCfg->deviceType == TERM_VT100) return false;}
#define RET_UNLESS_GOLDELOX_BOOL(oledCfg)  {if (oledCfg->deviceType != GOLDELOX) return false;}
#else
#define RET_UNLESS_INIT(oledCfg)
#define RET_UNLESS_4DSYS(oledCfg)
#define RET_UNLESS_GOLDELOX(oledCfg)

#define RET_UNLESS_INIT_BOOL(oledCfg)
#define RET_UNLESS_4DSYS_BOOL(oledCfg)
#define RET_UNLESS_GOLDELOX_BOOL(oledCfg)
#endif

static bool oledIsInitialised (const OledConfig *oledConfig) ;
static void oledScreenSaverTimout (OledConfig *oledConfig, uint16_t timout);
static void oledPreInit (OledConfig *oledConfig, uint32_t baud);
static uint32_t oledGetFileError  (OledConfig *oledConfig);
static bool oledFileSeek  (OledConfig *oledConfig, const uint16_t handle, const uint32_t offset);
static uint16_t fgColorIndexTo16b (const OledConfig *oledConfig, const uint8_t colorIndex);
static uint16_t oledTouchGet (OledConfig *oledConfig, uint16_t mode);
static uint16_t getResponseAsUint16 (const uint8_t *buff);
#if PICASO_DISPLAY_USE_UART
static void uartStartRead (UARTDriver *serial, uint8_t *response, 
			   const size_t size);
static msg_t uartWaitReadTimeout(UARTDriver *serial, size_t *size, sysinterval_t  rTimout);
#endif

static void oledPreInit (OledConfig *oledConfig, uint32_t baud)
{
  oledConfig->bg = colorDecTo16b(0,0,0);
  oledConfig->tbg[0] = mkColor24 (0,0,0);
  oledConfig->fg[0] = mkColor24(50,50,50);
  oledConfig->tbgIdx = oledConfig->fgIdx = 0;
  oledConfig->curXpos = 0;
  oledConfig->curYpos = 0;
  oledConfig->serial = NULL;

  // initial USART conf only for 4D system screen
  // 9600 bauds because of broken auto baud feature on some screen model
  memset(&oledConfig->serialConfig, 0, sizeof(oledConfig->serialConfig));
  oledConfig->serialConfig.speed = baud;
  oledConfig->serialConfig.cr1 = 0;
  oledConfig->serialConfig.cr2 = USART_CR2_STOP1_BITS | USART_CR2_LINEN;
  oledConfig->serialConfig.cr3 = 0;
}

bool oledStart (OledConfig *oledConfig,  LINK_DRIVER *oled, const uint32_t baud,
		ioline_t rstLine, enum OledConfig_Device dev)
{
  oledConfig->rstLine = rstLine;
  oledConfig->deviceType = dev;

  oledHardReset(oledConfig);


  oledPreInit(oledConfig,
	       oledConfig->deviceType == TERM_VT100 ? baud : 9600);
  chMtxObjectInit(&(oledConfig->omutex));
#if PICASO_DISPLAY_USE_SD
  oledConfig->serial = (BaseSequentialStream *) oled;
  sdStart(oled, &(oledConfig->serialConfig));
#else
  oledConfig->serial = oled;
  uartStart(oled, &(oledConfig->serialConfig));
#endif  
  chThdSleepMilliseconds(10);


  // test is no error on kind of device : picaso, goldelox, diablo ...
  if (!oledIsCorrectDevice(oledConfig)) {
    DebugTrace("Error incorrect device type @ %s:%d", __FUNCTION__, __LINE__);
    return false;
  }

  // opaque background
  oledClearScreen(oledConfig);
  oledSetTextOpacity(oledConfig, true);

  // disable screensaver : 0 is special value for disabling screensaver
  // since oled has remanance problem, we activate screensaver after 20 seconds
  oledScreenSaverTimout(oledConfig, 20000);
  
  // use greater speed
  if ((oledConfig->deviceType != TERM_VT100) && (baud != 9600))
    return oledSetBaud(oledConfig, baud);
  return true;
}


void oledHardReset (OledConfig *oledConfig)
{

  RET_UNLESS_4DSYS(oledConfig);

  palClearLine(oledConfig->rstLine);
  chThdSleepMilliseconds(10);
  palSetLine(oledConfig->rstLine);
  chThdSleepMilliseconds(3000);
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
  chMtxUnlock(&(oledConfig->omutex));
}


void oledGetVersion (OledConfig *oledConfig, char *buffer, const size_t buflen)
{
  RET_UNLESS_4DSYS(oledConfig);

   // get display model
  uint16_t modelLen = buflen;
  sys_getModel(oledConfig, &modelLen, buffer);

  if (modelLen > buflen) {
    DebugTrace("warning %s need bigger buffer [%u instead %u] to store returned string",
	       __FUNCTION__, modelLen, buflen);
  }

  // get Pmmc version
  uint16_t pmmc=0;
  sys_getPmmC(oledConfig, &pmmc);
  const uint8_t pmmcMajor = (uint8_t) (pmmc>>8);
  const uint8_t pmmcMinor =  (uint8_t) (pmmc & 0xff);
  chsnprintf(&buffer[strlen(buffer)], buflen-strlen(buffer), " Pmmc=%d.%d", pmmcMajor, pmmcMinor);

  // get SPE version
  uint16_t spe=0;
  sys_getVersion(oledConfig, &spe);
  const uint8_t speMajor = (uint8_t) (spe >> 8);
  const uint8_t speMinor = (uint8_t) (spe  & 0xff);
  chsnprintf(&buffer[strlen(buffer)], buflen-strlen(buffer), " Spe=%d.%d", speMajor, speMinor);

  // string terminaison
  buffer[buflen-1]=0;
}


bool oledSetBaud (OledConfig *oledConfig, uint32_t baud)
{
  uint32_t actualbaudRate; 
  uint16_t baudCode = 0;

  LINK_DRIVER *sd = (LINK_DRIVER *) oledConfig->serial;

  RET_UNLESS_INIT_BOOL(oledConfig);
  
  switch (oledConfig->deviceType) {
  case DIABLO16: {
    switch(baud) {
    case 110    : baudCode = 0x0;  actualbaudRate = 110; break;
    case 300    : baudCode = 0x1;  actualbaudRate = 300; break;
    case 600    : baudCode = 0x2;  actualbaudRate = 600; break;
    case 1200   : baudCode = 0x3;  actualbaudRate = 1200; break;
    case 2400   : baudCode = 0x4;  actualbaudRate = 2401; break;
    case 4800   : baudCode = 0x5;  actualbaudRate = 4802; break;
    case 9600   : baudCode = 0x6;  actualbaudRate = 9615; break;
    case 14400  : baudCode = 0x7;  actualbaudRate = 14439; break;
    case 19200  : baudCode = 0x8;  actualbaudRate = 19273; break;
    case 31250  : baudCode = 0x9;  actualbaudRate = 31250; break;
    case 38400  : baudCode = 0xa;  actualbaudRate = 38717; break;
    case 56000  : baudCode = 0xb;  actualbaudRate = 56090; break;
    case 57600  : baudCode = 0xc;  actualbaudRate = 58333; break;
    case 115200 : baudCode = 0xd;  actualbaudRate = 118243; break;
    case 128000 : baudCode = 0xe;  actualbaudRate = 128676; break;
    case 256000 : baudCode = 0xf;  actualbaudRate = 257353; break;
    case 300000 : baudCode = 0x10; actualbaudRate = 312500; break;
    case 375000 : baudCode = 0x11; actualbaudRate = 397727; break;
    case 500000 : baudCode = 0x12; actualbaudRate = 546875; break;
    case 600000 : baudCode = 0x13; actualbaudRate = 625000; break;
    default : return false;
    }
    oledConfig->serialConfig.speed = actualbaudRate; 
  }
    break;

  case PICASO: {
    switch(baud) {
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
    default : return false;
    }
    oledConfig->serialConfig.speed = actualbaudRate; 
  }
    break;
  case GOLDELOX: {
    switch(baud) {
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
    default : return false;
    }
    oledConfig->serialConfig.speed = actualbaudRate; 
  }
    break;

  default:
    oledConfig->serialConfig.speed = baud;
    break;
  }

  misc_setbaudWait(oledConfig, baudCode);
  chThdSleepMilliseconds(10);
#if PICASO_DISPLAY_USE_SD
  sdStop(sd);
  sdStart(sd, &(oledConfig->serialConfig));
#else
  uartStop(sd);
  uartStart(sd, &(oledConfig->serialConfig));
#endif
  // wait for response at new speed
  RET_UNLESS_4DSYS_BOOL(oledConfig);

  struct {
   uint8_t ack;
 } response;
 return oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
			   NULL, 0,
			   (uint8_t *) &response, sizeof(response)) == 1; 
}


bool oledPrintFmt (OledConfig *oledConfig, const char *fmt, ...)
{
  char buffer[120];
  char *token, *curBuf;
  bool lastLoop = false;
  bool ret = false;

  va_list ap;
  RET_UNLESS_INIT_BOOL(oledConfig);

  va_start(ap, fmt);
  chvsnprintf(buffer, sizeof(buffer), fmt, ap);
  va_end(ap);

  if (buffer[0] == 0)
    return true;

  if (buffer[1] == 0) {
    return oledPrintBuffer(oledConfig, buffer);
  }
  
  const char* endPtr = &(buffer[strnlen(buffer, sizeof(buffer)) -1]);
  // replace escape color sequence by color command for respective backend
  // ESC c 0 à 9 : couleur index of background and foreground
  // replace escape n by carriage return, line feed
  // replace escape t by horizontal tabulation
  for (curBuf=buffer;(curBuf<endPtr) && (lastLoop == false);) {
    token = index(curBuf, 033);
    if (token == NULL) {
      // on peut imprimer les derniers caractères et terminer
      lastLoop = true;
    } else {
      // token pointe sur le char d'echappement ESC
      // on met un caractère de fin de chaine à la place
      *token++ =0;
    }
    
    if (*curBuf != 0) {
      ret = oledPrintBuffer(oledConfig, curBuf);
      oledConfig->curXpos =  (uint8_t) (oledConfig->curXpos + strnlen(curBuf, sizeof(buffer)));
    }
    
    if (lastLoop == false) {
      // next two char a color coding scheme
      if (tolower((uint32_t) (*token)) == 'c') { 
	const int32_t colorIndex = *++token - '0';
	oledUseColorIndex(oledConfig, (uint8_t) colorIndex);
	//	DebugTrace ("useColorIndex %d", colorIndex);
	curBuf=token+1;
      } else if (tolower((uint32_t) (*token)) == 'n') { 	
	//	DebugTrace ("carriage return");
	oledGotoXY(oledConfig, 0,  (uint8_t) (oledConfig->curYpos+1));
	curBuf=token+1;
      } else if (tolower((uint32_t) (*token)) == 't') { 	
	//	DebugTrace ("tabulation");
	const uint8_t tabLength =  (uint8_t) (8-(oledConfig->curXpos%8));
	char space[8] = {[0 ... 7] = ' '};
	space[tabLength] = 0;
	ret = oledPrintBuffer(oledConfig, space);
	oledGotoX(oledConfig, (uint8_t) (oledConfig->curXpos + tabLength));
	curBuf=token+1;
      }
    }
  }
  return ret;
}

bool oledPrintBuffer (OledConfig *oledConfig, const char *buffer)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  bool ret = false;
  switch(oledConfig->deviceType) {
  case GOLDELOX :
  case PICASO :
  case DIABLO16 : 
    ret = txt_moveCursor(oledConfig, oledConfig->curYpos, oledConfig->curXpos);
    if (ret ==  true)
      ret = txt_putStr(oledConfig, buffer, NULL);
    break;
  case TERM_VT100 : 
    sendVt100Seq(oledConfig->serial, "%d;%dH",  oledConfig->curYpos+1, oledConfig->curXpos+1);
    #if PICASO_DISPLAY_USE_SD
    directchprintf(oledConfig->serial, buffer);
#else
    size_t length = strnlen(buffer, sizeof(buffer));
    uartSendTimeout(oledConfig->serial, &length, buffer, TIME_INFINITE);
#endif
    break;
  default: osalSysHalt("incorrect oledConfig->deviceType");
  }
  return ret;
}

void oledChangeBgColor (OledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);
  
  const uint16_t oldCol = oledConfig->bg;
  const uint16_t newCol = oledConfig->bg = colorDecTo16b(r,g,b);
  RET_UNLESS_4DSYS(oledConfig);
  gfx_changeColour(oledConfig, oldCol, newCol);
} 


void oledSetTextBgColor (OledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);

  oledConfig->tbg[0] = mkColor24(r,g,b);
  switch(oledConfig->deviceType) {
  case GOLDELOX :
  case PICASO : 
  case DIABLO16 :
    txt_Bgcolour(oledConfig,  colorDecTo16b(r,g,b), NULL);
    break;
  case TERM_VT100 : 
    sendVt100Seq(oledConfig->serial, "48;2;%d;%d;%dm", r*255/100, g*255/100, b*255/100);
    break;
  default: osalSysHalt("incorrect oledConfig->deviceType");
  } 
}

void oledSetTextFgColor (OledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(oledConfig);
  
  oledConfig->fg[0] = mkColor24(r,g,b);
  switch(oledConfig->deviceType) {
  case GOLDELOX :
  case PICASO : 
  case DIABLO16 :
    txt_Fgcolour(oledConfig,  colorDecTo16b(r,g,b), NULL);
    break;
  case TERM_VT100 : 
    sendVt100Seq(oledConfig->serial, "38;2;%d;%d;%dm", r*255/100, g*255/100, b*255/100);
    break;
  default: osalSysHalt("incorrect oledConfig->deviceType");
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
      oledSetTextFgColor(oledConfig, oledConfig->fg[colorIndex].r, 
			  oledConfig->fg[colorIndex].g, oledConfig->fg[colorIndex].b);
    }
    
    if (oledConfig->tbg[0].rgb != oledConfig->tbg[colorIndex].rgb)  {
      oledSetTextBgColor(oledConfig, oledConfig->tbg[colorIndex].r, 
			  oledConfig->tbg[colorIndex].g, oledConfig->tbg[colorIndex].b);
    }
  } else {
    oledSetTextFgColor(oledConfig, oledConfig->fg[colorIndex].r, 
			oledConfig->fg[colorIndex].g, oledConfig->fg[colorIndex].b);
    oledSetTextBgColor(oledConfig, oledConfig->tbg[colorIndex].r, 
			oledConfig->tbg[colorIndex].g, oledConfig->tbg[colorIndex].b);
  }
}


void oledSetTextOpacity (OledConfig *oledConfig, bool opaque)
{
  if (oledConfig->deviceType == GOLDELOX)
    txt_opacity(oledConfig, opaque, NULL);
}


void oledSetTextAttributeMask (OledConfig *oledConfig, enum OledTextAttribute attrib)
{
  txt_attributes(oledConfig, attrib, NULL);
}

void oledSetTextGap (OledConfig *oledConfig, uint8_t xgap, uint8_t ygap)
{
  txt_xgap(oledConfig, xgap, NULL);
  txt_ygap(oledConfig, ygap, NULL);
}

void oledSetTextSizeMultiplier (OledConfig *oledConfig, uint8_t xmul, uint8_t ymul)
{
  txt_widthMult(oledConfig, xmul, NULL);
  txt_heightMult(oledConfig, ymul, NULL);
}

void oledSetScreenOrientation (OledConfig *oledConfig, enum OledScreenOrientation orientation)
{
  gfx_screenMode(oledConfig, orientation, NULL);
}


void oledGotoXY (OledConfig *oledConfig, uint8_t x, uint8_t y)
{  
  RET_UNLESS_INIT(oledConfig);

   oledConfig->curXpos=x;
   oledConfig->curYpos=y;

   if (oledConfig->deviceType == TERM_VT100) {
     sendVt100Seq(oledConfig->serial, "%d;%dH", y+1,x+1);
   } 
}

void oledGotoX (OledConfig *oledConfig, uint8_t x)
{  
  RET_UNLESS_INIT(oledConfig);

  oledConfig->curXpos=x;
  if (oledConfig->deviceType == TERM_VT100) {
    sendVt100Seq(oledConfig->serial, "%d;%dH", oledConfig->curYpos+1,x+1);
  }
}

uint8_t oledGetX  (const OledConfig *oledConfig)
{  
  if (oledIsInitialised(oledConfig) == false)
    return 0;
  
  return oledConfig->curXpos;
}

uint8_t oledGetY  (const OledConfig *oledConfig)
{  
  if (oledIsInitialised(oledConfig) == false)
    return 0;
  
  return oledConfig->curYpos;
}

void oledGotoNextLine (OledConfig *oledConfig)
{  
  RET_UNLESS_INIT(oledConfig);

  oledConfig->curXpos=0;
  oledConfig->curYpos++;
  if (oledConfig->deviceType == TERM_VT100) {
    sendVt100Seq(oledConfig->serial, "B");
    //    chprintf(oledConfig->serial, "\r\n");
  }
}


bool oledIsCorrectDevice (OledConfig *oledConfig)
{
  if (oledIsInitialised(oledConfig) == false) 
    return false;

  if (oledConfig->deviceType == TERM_VT100) 
    return true;

  if (oledConfig->deviceType == AUTO_4DS) 
    return findDeviceType(oledConfig);
  else
    return (gfx_cls(oledConfig) && gfx_cls(oledConfig));
}

static bool findDeviceType (OledConfig *oledConfig)
{
  if (oledIsInitialised(oledConfig) == false) 
    return false;

  if (oledConfig->deviceType == TERM_VT100) 
    return true;

  for (enum OledConfig_Device type=GOLDELOX; type <= DIABLO16; type++) {
    oledConfig->deviceType = type;
    if (oledIsCorrectDevice(oledConfig)) {
      DebugTrace("find correct device type = %u", type);
      return true;
    } else {
      oledHardReset(oledConfig);
    }
  }
  return false;
}

void oledClearScreen (OledConfig *oledConfig)
{
  RET_UNLESS_INIT(oledConfig);

  if (oledConfig->deviceType == TERM_VT100) {
    sendVt100Seq(oledConfig->serial, "2J");
  } else {
    gfx_cls(oledConfig);
  }
}


void oledDrawPoint (OledConfig *oledConfig, const uint16_t x, const uint16_t y, 
		    const uint8_t colorIndex)
{
  RET_UNLESS_INIT(oledConfig);

  const uint16_t fg = fgColorIndexTo16b(oledConfig, (uint8_t) (colorIndex+1));
  gfx_putPixel(oledConfig, x, y, fg);
}


void oledDrawLine (OledConfig *oledConfig, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2, 
		   const uint8_t colorIndex)
{
  RET_UNLESS_INIT(oledConfig);

  const uint16_t fg = fgColorIndexTo16b(oledConfig, (uint8_t) (colorIndex+1));
  gfx_line(oledConfig, x1, y1, x2, y2, fg);
}

void oledDrawRect (OledConfig *oledConfig, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2,
		   const bool filled,
		   const uint8_t colorIndex)
{
  RET_UNLESS_INIT(oledConfig);

  const uint16_t fg = fgColorIndexTo16b(oledConfig, (uint8_t) (colorIndex+1));
  if (filled) 
    gfx_rectangle(oledConfig, x1, y1, x2, y2, fg);
  else
    gfx_rectangleFilled(oledConfig, x1, y1, x2, y2, fg);
 }

void oledDrawPolyLine (OledConfig *oledConfig, 
		       const uint16_t len,
		       const PolyPoint * const pp,
		       const uint8_t colorIndex)
  
{
  RET_UNLESS_INIT(oledConfig);
  struct {
    uint16_t vx[len];
    uint16_t vy[len];
    uint16_t color;
  } command;

  for (size_t i=0; i<len; i++) {
    command.vx[i] = __builtin_bswap16(pp[i].x);
    command.vy[i] = __builtin_bswap16(pp[i].y);
  }
  command.color =  fgColorIndexTo16b(oledConfig, (uint8_t) (colorIndex+1));
  gfx_polyline(oledConfig, len, command.vx, command.vy, command.color);
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
  gfx_screenCopyPaste(oledConfig, xs, ys, xd, yd, width, height);
}


void oledEnableTouch (OledConfig *oledConfig, bool enable)
{
   touch_set(oledConfig, enable ? 0x00 : 0x01);
}

static uint16_t oledTouchGet (OledConfig *oledConfig, uint16_t mode)
{
  if (oledIsInitialised(oledConfig) == false) 
    return 0xff;
  uint16_t value;
  
  touch_get(oledConfig, mode, &value);
  return value;
}


uint16_t oledTouchGetStatus (OledConfig *oledConfig)
{
  return oledTouchGet(oledConfig, 0);
}

uint16_t oledTouchGetXcoord (OledConfig *oledConfig)
{
  return oledTouchGet(oledConfig, 1);
}

uint16_t oledTouchGetYcoord (OledConfig *oledConfig)
{
  return oledTouchGet(oledConfig, 2);
}




bool oledInitSdCard (OledConfig *oledConfig)
{ 
  if (oledIsInitialised(oledConfig) == false) return false;

  uint16_t status;
  media_init(oledConfig, &status);

  if (status != 0) {
    // card is present, mount fat16 fs
    DebugTrace ("oledInitSdCard sd presence=%d", status);
    file_mount(oledConfig, &status);
    DebugTrace("oledInitSdCard fat16 mount=%d", status);
  }
  
  return (status != 0);
}

void oledListSdCardDirectory (OledConfig *oledConfig)
{
  RET_UNLESS_INIT(oledConfig);
  bool remainFile = true;
  uint32_t fileNo = 0;
  char fileName[24];
  uint16_t strLen = sizeof(fileName);
  
  while (remainFile) {
    strLen = sizeof(fileName);
    if (fileNo++ == 0) 
      // find first file and report
      file_findFirstRet(oledConfig, "*.*", &strLen, fileName);
    else
      // find next file and report
      file_findNextRet(oledConfig, &strLen, fileName);
    
    if (strLen) {
      DebugTrace ("File [%lu] = %s", fileNo, fileName);
    }
  }
}


void oledSetSoundVolume (OledConfig *oledConfig, uint8_t percent)
{
  percent = MIN (100, percent);
  percent = (uint8_t) (percent + 27);
  snd_volume(oledConfig, percent);
}

void oledPlayWav (OledConfig *oledConfig, const char* fileName)
{  
  file_playWAV(oledConfig, fileName, NULL);
}

// No buzzer on our OLED-96-G2
/* void oledPlayBeep (OledConfig *oledConfig, uint8_t note, uint16_t duration) */
/* {   */
/*   RET_UNLESS_INIT(oledConfig); */
/*   RET_UNLESS_GOLDELOX(oledConfig); */
/*   OLED("%c%c%c%c%c%c", 0xff, 0xda,  */
/* 	0, note, twoBytesFromWord(duration)); */
/* } */

uint32_t oledOpenFile (OledConfig *oledConfig, const char* fileName, uint16_t *handle)
{
  if (oledIsInitialised(oledConfig) == false) return 0;

  file_open(oledConfig, fileName, 'r', handle);
  return oledGetFileError(oledConfig);
}


void oledCloseFile (OledConfig *oledConfig, const uint16_t handle)
{
  file_close(oledConfig, handle, NULL);
}



void oledDisplayGci (OledConfig *oledConfig, const uint16_t handle, uint32_t offset)
{
  uint16_t errno;
  if (oledFileSeek (oledConfig, handle, offset)) {
    file_image(oledConfig, 0, 0, handle, &errno);
     if (errno) {
       DebugTrace ("oledDisplayGci error %u", errno);
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

static uint16_t getResponseAsUint16 (const uint8_t *buffer)
{
  return __builtin_bswap16(*(uint16_t *) (buffer+1));
}

static  uint16_t fgColorIndexTo16b (const OledConfig *oledConfig, const uint8_t colorIndex) {
  const Color24 fg = oledConfig->fg[colorIndex];
  
  return (colorDecTo16b(fg.r, fg.g, fg.b));
}

static void oledScreenSaverTimout (OledConfig *oledConfig, uint16_t timout)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_GOLDELOX(oledConfig);

  misc_screenSaverTimeout(oledConfig, timout);
}

static uint32_t oledGetFileError  (OledConfig *oledConfig)
{
  if (oledIsInitialised(oledConfig) == false) return 0;
  uint16_t errno;

  file_error(oledConfig, &errno);
  return errno;
}

static bool oledFileSeek  (OledConfig *oledConfig, const uint16_t handle, const uint32_t offset)
{
  if (oledIsInitialised(oledConfig) == false) return false;

  const union {
    uint32_t val;
    struct {
      uint16_t lo;
      uint16_t hi;
    };
  } ofst = {.val = offset};
  uint16_t status;
  file_seek(oledConfig, handle, ofst.hi, ofst.lo, &status);
  
  return status == 1;
}

#if PICASO_DISPLAY_USE_SD
static uint32_t oledReceiveAnswer (OledConfig *oc, uint8_t *response, const size_t size,
				   const char* fct, const uint32_t line)
{
  (void) fct;
  (void) line;

  BaseChannel *serial =  (BaseChannel *)  oc->serial;
  

  const uint32_t ret = chnReadTimeout(serial, response, size, readTimout);

  if (ret != size) {
    DebugTrace ("oledReceiveAnswer ret[%lu] != expectedSize[%u] @%s : line %lu", ret, size, fct, line);
    oledStatus = OLED_ERROR;
  } else {
    oledStatus = OLED_OK;
  }
  return ret;
}
#endif

#if PICASO_DISPLAY_USE_UART

static void oledStartReceiveAnswer (OledConfig *oc, uint8_t *response, const size_t size,
				    const char* fct, const uint32_t line)
{
  (void) fct;
  (void) line;

  UARTDriver *serial = oc->serial;
  response[0] = 0;

  uartStartRead(serial, response, size);
}

static uint32_t oledWaitReceiveAnswer (OledConfig *oc, size_t *size,
				       const char* fct, const uint32_t line)

{
  (void) fct;
  (void) line;

  UARTDriver *serial = oc->serial;

  const size_t ask = *size;
  msg_t status = uartWaitReadTimeout(serial, size, readTimout);
  const size_t ret = *size;

  if (status != MSG_OK) {
    DebugTrace ("oledWaitReceiveAnswer ask[%u] != read[%u] @%s : line %lu", ask,
		ret, fct, line);
    oledStatus = OLED_ERROR;
  } else {
    oledStatus = OLED_OK;
  }
  return ret;
}
#endif


static uint32_t oledTransmitBuffer (OledConfig *oc, const char* fct, const uint32_t line,
				    const uint8_t *outBuffer, const size_t outSize,
				    uint8_t *inBuffer, const size_t inSize)
{
  uint32_t ret = 0;

#if PICASO_DISPLAY_USE_SD
  BaseChannel * serial =  (BaseChannel *)  oc->serial;
  if (inSize != 0)
    chnReadTimeout (serial, inBuffer, inSize, TIME_IMMEDIATE);
#else
  size_t inSizeRw = inSize;
#endif
  
  // send command
#if PICASO_DISPLAY_USE_SD
  if (outSize != 0)
    streamWrite(oc->serial, outBuffer, outSize);
#else
  if (inSize != 0) {
    oledStartReceiveAnswer(oc, inBuffer, inSizeRw, fct, line);
  }
  size_t length = outSize;
  if (outSize != 0) {
    uartSendTimeout(oc->serial, &length, outBuffer, TIME_INFINITE);
  }
#endif
  
  // get response
  if (inSize == 0)
    return 0;

#if PICASO_DISPLAY_USE_SD
  ret = oledReceiveAnswer(oc, inBuffer, inSize, fct, line);
#else
  ret = oledWaitReceiveAnswer(oc, &inSizeRw, fct, line);
#endif
  
  return ret;
}


OledStatus oledGetStatus(void)
{
  return oledStatus;
}

#if PICASO_DISPLAY_USE_SD
static void sendVt100Seq (BaseSequentialStream *serial, const char *fmt, ...)
{
  char buffer[80] = {0x1b, '['};
  va_list ap;
  
  va_start(ap, fmt);
  chvsnprintf(&(buffer[2]), sizeof (buffer)-2, fmt, ap); 
  va_end(ap);

  directchprintf (serial, buffer);
}
#else
static void sendVt100Seq (UARTDriver *serial, const char *fmt, ...)
{
  char buffer[80] = {0x1b, '['};
  va_list ap;
  
  va_start(ap, fmt);
  size_t length =  chvsnprintf(&(buffer[2]), sizeof (buffer)-2, fmt, ap); 
  va_end(ap);

  uartSendTimeout(serial, &length, buffer, TIME_INFINITE);
}


static void uartStartRead (UARTDriver *serial, uint8_t *response, 
		    const size_t size)
{
  osalDbgCheck((serial != NULL) && (size != 0U) && (response != NULL));
  
  osalSysLock();
  osalDbgAssert(serial->state == UART_READY, "is active");
  osalDbgAssert(serial->rxstate != UART_RX_ACTIVE, "rx active");
  
  /* Receive start.*/
  uart_lld_start_receive(serial, size, response);
  serial->rxstate = UART_RX_ACTIVE;
  osalSysUnlock();
}


static msg_t uartWaitReadTimeout(UARTDriver *serial, size_t *size, sysinterval_t  rTimout)
{
  msg_t msg;

  osalDbgCheck(serial != NULL);
  osalSysLock();
  
  msg = osalThreadSuspendTimeoutS(&serial->threadrx, rTimout);
  if (msg != MSG_OK) {
    *size -= uartStopReceiveI(serial);
  }
  osalSysUnlock();
  
  return msg;
}
#endif

#include "picaso4Display_ll.c"
