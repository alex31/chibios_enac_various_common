#include <ch.h>
#include <hal.h>
#include <string.h>
#include <ctype.h>
#include "stdutil.h"
#include "printf.h"
#include "display4DS.h"
#include "display4DS_ll.h"


#define gfx_clampColor(r,v,b) ((uint16_t) ((r & 0x1f) <<11 | (v & 0x3f) << 5 | (b & 0x1f)))
#define gfx_colorDecTo16b(r,v,b) (gfx_clampColor((r*31/100), (v*63/100), (b*31/100)))
#define QDS_ACK 0x6

typedef enum {KOF_NONE, KOF_ACK, KOF_INT16, 
	      KOF_INT16LENGTH_THEN_DATA} KindOfCommand;

const uint32_t readTimout = TIME_MS2I(500);
static OledStatus fdsStatus = OLED_OK;


static uint32_t fdsTransmitBuffer (const FdsConfig *oc, const char* fct, const uint32_t line,
					 const uint8_t *outBuffer, const size_t outSize,
					 uint8_t *inBuffer, const size_t inSize);
#if PICASO_DISPLAY_USE_SD
static uint32_t fdsReceiveAnswer (const FdsConfig *oc, uint8_t *response, const size_t size,
				   const char* fct, const uint32_t line);
static void sendVt100Seq (BaseSequentialStream *serial, const char *fmt, ...);
#else
static void sendVt100Seq (UARTDriver *serial, const char *fmt, ...);
static void fdsStartReceiveAnswer (const FdsConfig *oc, uint8_t *response, const size_t size,
				     const char* fct, const uint32_t line);
static uint32_t fdsWaitReceiveAnswer (const FdsConfig *oc, size_t *size,
					const char* fct, const uint32_t line);
#endif
static bool findDeviceType (FdsConfig *fdsConfig);

#if CH_DBG_ENABLE_ASSERTS 
#define RET_UNLESS_INIT(fdsCfg)    {if (fdsIsInitialised(fdsCfg) == false) return ;}
#define RET_UNLESS_4DSYS(fdsCfg)  {if (fdsCfg->deviceType == TERM_VT100) return ;}
#define RET_UNLESS_GOLDELOX(fdsCfg)  {if (fdsCfg->deviceType != GOLDELOX) return ;}

#define RET_UNLESS_INIT_BOOL(fdsCfg)    {if (fdsIsInitialised(fdsCfg) == false) return false;}
#define RET_UNLESS_4DSYS_BOOL(fdsCfg)  {if (fdsCfg->deviceType == TERM_VT100) return false;}
#define RET_UNLESS_GOLDELOX_BOOL(fdsCfg)  {if (fdsCfg->deviceType != GOLDELOX) return false;}
#else
#define RET_UNLESS_INIT(fdsCfg)
#define RET_UNLESS_4DSYS(fdsCfg)
#define RET_UNLESS_GOLDELOX(fdsCfg)

#define RET_UNLESS_INIT_BOOL(fdsCfg)
#define RET_UNLESS_4DSYS_BOOL(fdsCfg)
#define RET_UNLESS_GOLDELOX_BOOL(fdsCfg)
#endif

static bool fdsIsInitialised (const FdsConfig *fdsConfig) ;
static void fdsScreenSaverTimout (const FdsConfig *fdsConfig, uint16_t timout);
static void fdsPreInit (FdsConfig *fdsConfig, uint32_t baud);
static uint32_t fdsGetFileError  (const FdsConfig *fdsConfig);
static bool fdsFileSeek  (const FdsConfig *fdsConfig, const uint16_t handle, const uint32_t offset);
static uint16_t fgColorIndexTo16b (const FdsConfig *fdsConfig, const uint8_t colorIndex);
static uint8_t colorDimmed (const uint8_t channel, const uint8_t luminosity);
static uint16_t fdsTouchGet (const FdsConfig *fdsConfig, uint16_t mode);
static uint16_t getResponseAsUint16 (const uint8_t *buff);
#if PICASO_DISPLAY_USE_UART
static void uartStartRead (UARTDriver *serial, uint8_t *response, 
			   const size_t size);
static msg_t uartWaitReadTimeout(UARTDriver *serial, size_t *size, sysinterval_t  rTimout);
#endif


static uint8_t colorDimmed (const uint8_t channel, const uint8_t luminosity)
{
  return ((uint32_t) channel * luminosity) / 100U;
}


static void fdsPreInit (FdsConfig *fdsConfig, uint32_t baud)
{
  fdsConfig->bg = gfx_colorDecTo16b(0,0,0);
  fdsConfig->tbg[0] = mkColor24 (0,0,0);
  fdsConfig->fg[0] = mkColor24(50,50,50);
  fdsConfig->colIdx = 0;
  fdsConfig->curXpos = 0;
  fdsConfig->curYpos = 0;
  fdsConfig->luminosity = 100U;
  fdsConfig->serial = NULL;

  // initial USART conf only for 4D system screen
  // 9600 bauds because of broken auto baud feature on some screen model
  memset(&fdsConfig->serialConfig, 0, sizeof(fdsConfig->serialConfig));
  fdsConfig->serialConfig.speed = baud;
  fdsConfig->serialConfig.cr1 = 0;
  fdsConfig->serialConfig.cr2 = USART_CR2_STOP1_BITS | USART_CR2_LINEN;
  fdsConfig->serialConfig.cr3 = 0;
}

bool fdsStart (FdsConfig *fdsConfig,  LINK_DRIVER *fds, const uint32_t baud,
		ioline_t rstLine, enum FdsConfig_Device dev)
{
  fdsConfig->rstLine = rstLine;
  fdsConfig->deviceType = dev;

  fdsHardReset(fdsConfig);


  fdsPreInit(fdsConfig,
	       fdsConfig->deviceType == TERM_VT100 ? baud : 9600);
  chMtxObjectInit(&(fdsConfig->omutex));
#if PICASO_DISPLAY_USE_SD
  fdsConfig->serial = (BaseSequentialStream *) fds;
  sdStart(fds, &(fdsConfig->serialConfig));
#else
  fdsConfig->serial = fds;
  uartStart(fds, &(fdsConfig->serialConfig));
#endif  
  chThdSleepMilliseconds(10);


  // test is no error on kind of device : picaso, goldelox, diablo ...
  if (!fdsIsCorrectDevice(fdsConfig)) {
    DebugTrace("Error incorrect device type @ %s:%d", __FUNCTION__, __LINE__);
    return false;
  }

  // opaque background
  fdsClearScreen(fdsConfig);
  fdsSetTextOpacity(fdsConfig, true);

  // disable screensaver : 0 is special value for disabling screensaver
  // since fds has remanance problem, we activate screensaver after 20 seconds
  fdsScreenSaverTimout(fdsConfig, 20000);
  
  // use greater speed
  if ((fdsConfig->deviceType != TERM_VT100) && (baud != 9600))
    return fdsSetBaud(fdsConfig, baud);
  return true;
}


void fdsHardReset (FdsConfig *fdsConfig)
{

  RET_UNLESS_4DSYS(fdsConfig);

  palClearLine(fdsConfig->rstLine);
  chThdSleepMilliseconds(10);
  palSetLine(fdsConfig->rstLine);
  chThdSleepMilliseconds(3000);
}


bool fdsIsInitialised (const FdsConfig *fdsConfig)
{
  return (fdsConfig->serial != NULL);
}

void fdsAcquireLock (FdsConfig *fdsConfig)
{ 
  RET_UNLESS_INIT(fdsConfig);
  chMtxLock(&(fdsConfig->omutex));
}

void fdsReleaseLock (FdsConfig *fdsConfig)
{ 
  RET_UNLESS_INIT(fdsConfig);
  (void) fdsConfig;
  chMtxUnlock(&(fdsConfig->omutex));
}


void fdsGetVersion (FdsConfig *fdsConfig, char *buffer, const size_t buflen)
{
  RET_UNLESS_4DSYS(fdsConfig);

   // get display model
  uint16_t modelLen = buflen;
  sys_getModel(fdsConfig, &modelLen, buffer);

  if (modelLen > buflen) {
    DebugTrace("warning %s need bigger buffer [%u instead %u] to store returned string",
	       __FUNCTION__, modelLen, buflen);
  }

  // get Pmmc version
  uint16_t pmmc=0;
  sys_getPmmC(fdsConfig, &pmmc);
  const uint8_t pmmcMajor = (uint8_t) (pmmc>>8);
  const uint8_t pmmcMinor =  (uint8_t) (pmmc & 0xff);
  chsnprintf(&buffer[strlen(buffer)], buflen-strlen(buffer), " Pmmc=%d.%d", pmmcMajor, pmmcMinor);

  // get SPE version
  uint16_t spe=0;
  sys_getVersion(fdsConfig, &spe);
  const uint8_t speMajor = (uint8_t) (spe >> 8);
  const uint8_t speMinor = (uint8_t) (spe  & 0xff);
  chsnprintf(&buffer[strlen(buffer)], buflen-strlen(buffer), " Spe=%d.%d", speMajor, speMinor);

  // string terminaison
  buffer[buflen-1]=0;
}


bool fdsSetBaud (FdsConfig *fdsConfig, uint32_t baud)
{
  uint32_t actualbaudRate; 
  uint16_t baudCode = 0;

  LINK_DRIVER *sd = (LINK_DRIVER *) fdsConfig->serial;

  RET_UNLESS_INIT_BOOL(fdsConfig);
  
  switch (fdsConfig->deviceType) {
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
    fdsConfig->serialConfig.speed = actualbaudRate; 
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
    fdsConfig->serialConfig.speed = actualbaudRate; 
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
    fdsConfig->serialConfig.speed = actualbaudRate; 
  }
    break;

  default:
    fdsConfig->serialConfig.speed = baud;
    break;
  }

  misc_setbaudWait(fdsConfig, baudCode);
  chThdSleepMilliseconds(10);
#if PICASO_DISPLAY_USE_SD
  sdStop(sd);
  sdStart(sd, &(fdsConfig->serialConfig));
#else
  uartStop(sd);
  uartStart(sd, &(fdsConfig->serialConfig));
#endif
  // wait for response at new speed
  RET_UNLESS_4DSYS_BOOL(fdsConfig);

  struct {
   uint8_t ack;
 } response;
 return fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
			   NULL, 0,
			   (uint8_t *) &response, sizeof(response)) == 1; 
}


bool fdsPrintFmt (FdsConfig *fdsConfig, const char *fmt, ...)
{
  char buffer[120];
  char *token, *curBuf;
  bool lastLoop = false;
  bool ret = false;

  va_list ap;
  RET_UNLESS_INIT_BOOL(fdsConfig);

  va_start(ap, fmt);
  chvsnprintf(buffer, sizeof(buffer), fmt, ap);
  va_end(ap);

  if (buffer[0] == 0)
    return true;

  if (buffer[1] == 0) {
    return fdsPrintBuffer(fdsConfig, buffer);
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
      ret = fdsPrintBuffer(fdsConfig, curBuf);
      fdsConfig->curXpos =  (uint8_t) (fdsConfig->curXpos + strnlen(curBuf, sizeof(buffer)));
    }
    
    if (lastLoop == false) {
      // next two char a color coding scheme
      if (tolower((uint32_t) (*token)) == 'c') { 
	const int32_t colorIndex = *++token - '0';
	fdsUseColorIndex(fdsConfig, (uint8_t) colorIndex);
	//	DebugTrace ("useColorIndex %d", colorIndex);
	curBuf=token+1;
      } else if (tolower((uint32_t) (*token)) == 'n') { 	
	//	DebugTrace ("carriage return");
	fdsGotoXY(fdsConfig, 0,  (uint8_t) (fdsConfig->curYpos+1));
	curBuf=token+1;
      } else if (tolower((uint32_t) (*token)) == 't') { 	
	//	DebugTrace ("tabulation");
	const uint8_t tabLength =  (uint8_t) (8-(fdsConfig->curXpos%8));
	char space[8] = {[0 ... 7] = ' '};
	space[tabLength] = 0;
	ret = fdsPrintBuffer(fdsConfig, space);
	fdsGotoX(fdsConfig, (uint8_t) (fdsConfig->curXpos + tabLength));
	curBuf=token+1;
      }
    }
  }
  return ret;
}

bool fdsPrintBuffer (FdsConfig *fdsConfig, const char *buffer)
{
  RET_UNLESS_INIT_BOOL(fdsConfig);
  bool ret = false;
  switch(fdsConfig->deviceType) {
  case GOLDELOX :
  case PICASO :
  case DIABLO16 : 
    ret = txt_moveCursor(fdsConfig, fdsConfig->curYpos, fdsConfig->curXpos);
    if (ret ==  true)
      ret = txt_putStr(fdsConfig, buffer, NULL);
    break;
  case TERM_VT100 : 
    sendVt100Seq(fdsConfig->serial, "%d;%dH",  fdsConfig->curYpos+1, fdsConfig->curXpos+1);
    #if PICASO_DISPLAY_USE_SD
    directchprintf(fdsConfig->serial, buffer);
#else
    size_t length = strnlen(buffer, sizeof(buffer));
    uartSendTimeout(fdsConfig->serial, &length, buffer, TIME_INFINITE);
#endif
    break;
  default: osalSysHalt("incorrect fdsConfig->deviceType");
  }
  return ret;
}

void fdsChangeBgColor (FdsConfig *fdsConfig, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(fdsConfig);
  
  const uint16_t oldCol = fdsConfig->bg;
  const uint16_t newCol = fdsConfig->bg = gfx_colorDecTo16b(r,g,b);
  RET_UNLESS_4DSYS(fdsConfig);
  gfx_changeColour(fdsConfig, oldCol, newCol);
} 


void fdsSetTextBgColor (FdsConfig *fdsConfig, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(fdsConfig);

  fdsConfig->tbg[0] = mkColor24(r,g,b);
  switch(fdsConfig->deviceType) {
  case GOLDELOX :
  case PICASO : 
  case DIABLO16 :
    txt_bgColour(fdsConfig,  gfx_colorDecTo16b(r,g,b), NULL);
    break;
  case TERM_VT100 : 
    sendVt100Seq(fdsConfig->serial, "48;2;%d;%d;%dm", r*255/100, g*255/100, b*255/100);
    break;
  default: osalSysHalt("incorrect fdsConfig->deviceType");
  } 
}

void fdsSetTextFgColor (FdsConfig *fdsConfig, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(fdsConfig);
  
  fdsConfig->fg[0] = mkColor24(r,g,b);
  switch(fdsConfig->deviceType) {
  case GOLDELOX :
  case PICASO : 
  case DIABLO16 :
    txt_fgColour(fdsConfig,  gfx_colorDecTo16b(r,g,b), NULL);
    break;
  case TERM_VT100 : 
    sendVt100Seq(fdsConfig->serial, "38;2;%d;%d;%dm", r*255/100, g*255/100, b*255/100);
    break;
  default: osalSysHalt("incorrect fdsConfig->deviceType");
  } 
}



void fdsSetTextBgColorTable (FdsConfig *fdsConfig, uint8_t colorIndex, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(fdsConfig);
  if (++colorIndex >= COLOR_TABLE_SIZE) return;

  fdsConfig->tbg[colorIndex] = mkColor24(r,g,b);
}

void fdsSetTextFgColorTable (FdsConfig *fdsConfig,  uint8_t colorIndex, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(fdsConfig);
  if (++colorIndex >= COLOR_TABLE_SIZE) return;

  fdsConfig->fg[colorIndex] = mkColor24(r,g,b);
}

void fdsUseColorIndex (FdsConfig *fdsConfig, uint8_t colorIndex)
{
  RET_UNLESS_INIT(fdsConfig);
  if (++colorIndex >= COLOR_TABLE_SIZE) return;
  fdsConfig->colIdx = colorIndex;
  if (fdsConfig->deviceType != TERM_VT100) {
    if (fdsConfig->fg[0].rgb != fdsConfig->fg[colorIndex].rgb)  {
      fdsSetTextFgColor(fdsConfig, colorDimmed(fdsConfig->fg[colorIndex].r, fdsConfig->luminosity), 
			 colorDimmed(fdsConfig->fg[colorIndex].g, fdsConfig->luminosity),
			 colorDimmed(fdsConfig->fg[colorIndex].b, fdsConfig->luminosity));
    }
    
    if (fdsConfig->tbg[0].rgb != fdsConfig->tbg[colorIndex].rgb)  {
      fdsSetTextBgColor(fdsConfig, colorDimmed(fdsConfig->tbg[colorIndex].r, fdsConfig->luminosity), 
			 colorDimmed(fdsConfig->tbg[colorIndex].g, fdsConfig->luminosity),
			 colorDimmed(fdsConfig->tbg[colorIndex].b, fdsConfig->luminosity));
    }
  } else {
    fdsSetTextFgColor(fdsConfig, colorDimmed(fdsConfig->fg[colorIndex].r, fdsConfig->luminosity), 
		       colorDimmed(fdsConfig->fg[colorIndex].g, fdsConfig->luminosity),
		       colorDimmed(fdsConfig->fg[colorIndex].b, fdsConfig->luminosity));
    fdsSetTextBgColor(fdsConfig, colorDimmed(fdsConfig->tbg[colorIndex].r, fdsConfig->luminosity), 
		       colorDimmed(fdsConfig->tbg[colorIndex].g, fdsConfig->luminosity),
		       colorDimmed(fdsConfig->tbg[colorIndex].b, fdsConfig->luminosity));
  }
}


void fdsSetTextOpacity (FdsConfig *fdsConfig, bool opaque)
{
  if (fdsConfig->deviceType == GOLDELOX)
    txt_opacity(fdsConfig, opaque, NULL);
}


void fdsSetTextAttributeMask (FdsConfig *fdsConfig, enum OledTextAttribute attrib)
{
  txt_attributes(fdsConfig, attrib, NULL);
}

void fdsSetTextGap (FdsConfig *fdsConfig, uint8_t xgap, uint8_t ygap)
{
  txt_xgap(fdsConfig, xgap, NULL);
  txt_ygap(fdsConfig, ygap, NULL);
}

void fdsSetTextSizeMultiplier (FdsConfig *fdsConfig, uint8_t xmul, uint8_t ymul)
{
  txt_widthMult(fdsConfig, xmul, NULL);
  txt_heightMult(fdsConfig, ymul, NULL);
}

void fdsSetScreenOrientation (FdsConfig *fdsConfig, enum OledScreenOrientation orientation)
{
  gfx_screenMode(fdsConfig, orientation, NULL);
}


void fdsGotoXY (FdsConfig *fdsConfig, uint8_t x, uint8_t y)
{  
  RET_UNLESS_INIT(fdsConfig);

   fdsConfig->curXpos=x;
   fdsConfig->curYpos=y;

   if (fdsConfig->deviceType == TERM_VT100) {
     sendVt100Seq(fdsConfig->serial, "%d;%dH", y+1,x+1);
   } 
}

void fdsGotoX (FdsConfig *fdsConfig, uint8_t x)
{  
  RET_UNLESS_INIT(fdsConfig);

  fdsConfig->curXpos=x;
  if (fdsConfig->deviceType == TERM_VT100) {
    sendVt100Seq(fdsConfig->serial, "%d;%dH", fdsConfig->curYpos+1,x+1);
  }
}

uint8_t fdsGetX  (const FdsConfig *fdsConfig)
{  
  if (fdsIsInitialised(fdsConfig) == false)
    return 0;
  
  return fdsConfig->curXpos;
}

uint8_t fdsGetY  (const FdsConfig *fdsConfig)
{  
  if (fdsIsInitialised(fdsConfig) == false)
    return 0;
  
  return fdsConfig->curYpos;
}

void fdsGotoNextLine (FdsConfig *fdsConfig)
{  
  RET_UNLESS_INIT(fdsConfig);

  fdsConfig->curXpos=0;
  fdsConfig->curYpos++;
  if (fdsConfig->deviceType == TERM_VT100) {
    sendVt100Seq(fdsConfig->serial, "B");
    //    chprintf(fdsConfig->serial, "\r\n");
  }
}

void fdsSetLuminosity (FdsConfig *fdsConfig, uint8_t luminosity)
{
  fdsConfig->luminosity = luminosity;
  //  fdsUseColorIndex(fdsConfig, fdsConfig->colIdx);
}


bool fdsIsCorrectDevice (FdsConfig *fdsConfig)
{
  if (fdsIsInitialised(fdsConfig) == false) 
    return false;

  if (fdsConfig->deviceType == TERM_VT100) 
    return true;

  if (fdsConfig->deviceType == AUTO_4DS) 
    return findDeviceType(fdsConfig);
  else
    return (gfx_cls(fdsConfig) && gfx_cls(fdsConfig));
}

static bool findDeviceType (FdsConfig *fdsConfig)
{
  if (fdsIsInitialised(fdsConfig) == false) 
    return false;

  if (fdsConfig->deviceType == TERM_VT100) 
    return true;

  for (enum FdsConfig_Device type=GOLDELOX; type <= DIABLO16; type++) {
    fdsConfig->deviceType = type;
    if (fdsIsCorrectDevice(fdsConfig)) {
      DebugTrace("find correct device type = %u", type);
      return true;
    } else {
      fdsHardReset(fdsConfig);
    }
  }
  return false;
}

void fdsClearScreen (FdsConfig *fdsConfig)
{
  RET_UNLESS_INIT(fdsConfig);

  if (fdsConfig->deviceType == TERM_VT100) {
    sendVt100Seq(fdsConfig->serial, "2J");
  } else {
    gfx_cls(fdsConfig);
  }
}


void fdsDrawPoint (FdsConfig *fdsConfig, const uint16_t x, const uint16_t y, 
		    const uint8_t colorIndex)
{
  RET_UNLESS_INIT(fdsConfig);

  const uint16_t fg = fgColorIndexTo16b(fdsConfig, (uint8_t) (colorIndex+1));
  gfx_putPixel(fdsConfig, x, y, fg);
}


void fdsDrawLine (FdsConfig *fdsConfig, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2, 
		   const uint8_t colorIndex)
{
  RET_UNLESS_INIT(fdsConfig);

  const uint16_t fg = fgColorIndexTo16b(fdsConfig, (uint8_t) (colorIndex+1));
  gfx_line(fdsConfig, x1, y1, x2, y2, fg);
}

void fdsDrawRect (FdsConfig *fdsConfig, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2,
		   const bool filled,
		   const uint8_t colorIndex)
{
  RET_UNLESS_INIT(fdsConfig);

  const uint16_t fg = fgColorIndexTo16b(fdsConfig, (uint8_t) (colorIndex+1));
  if (filled) 
    gfx_rectangle(fdsConfig, x1, y1, x2, y2, fg);
  else
    gfx_rectangleFilled(fdsConfig, x1, y1, x2, y2, fg);
 }

void fdsDrawPolyLine (FdsConfig *fdsConfig, 
		       const uint16_t len,
		       const PolyPoint * const pp,
		       const uint8_t colorIndex)
  
{
  RET_UNLESS_INIT(fdsConfig);
  struct {
    uint16_t vx[len];
    uint16_t vy[len];
    uint16_t color;
  } command;

  for (size_t i=0; i<len; i++) {
    command.vx[i] = __builtin_bswap16(pp[i].x);
    command.vy[i] = __builtin_bswap16(pp[i].y);
  }
  command.color =  fgColorIndexTo16b(fdsConfig, (uint8_t) (colorIndex+1));
  gfx_polyline(fdsConfig, len, command.vx, command.vy, command.color);
}


/*
  API available for PICASO and DIABLO, not for GOLDELOX 

 */
void fdsScreenCopyPaste (FdsConfig *fdsConfig, 
			  const uint16_t xs, const uint16_t ys, 
			  const uint16_t xd, const uint16_t yd,
			  const uint16_t width, const uint16_t height)
{
  RET_UNLESS_INIT(fdsConfig);
  gfx_screenCopyPaste(fdsConfig, xs, ys, xd, yd, width, height);
}


void fdsEnableTouch (FdsConfig *fdsConfig, bool enable)
{
   touch_set(fdsConfig, enable ? 0x00 : 0x01);
}

static uint16_t fdsTouchGet (const FdsConfig *fdsConfig, uint16_t mode)
{
  if (fdsIsInitialised(fdsConfig) == false) 
    return 0xff;
  uint16_t value;
  
  touch_get(fdsConfig, mode, &value);
  return value;
}


uint16_t fdsTouchGetStatus (FdsConfig *fdsConfig)
{
  return fdsTouchGet(fdsConfig, 0);
}

uint16_t fdsTouchGetXcoord (FdsConfig *fdsConfig)
{
  return fdsTouchGet(fdsConfig, 1);
}

uint16_t fdsTouchGetYcoord (FdsConfig *fdsConfig)
{
  return fdsTouchGet(fdsConfig, 2);
}




bool fdsInitSdCard (FdsConfig *fdsConfig)
{ 
  if (fdsIsInitialised(fdsConfig) == false) return false;

  uint16_t status;
  media_init(fdsConfig, &status);

  if (status != 0) {
    // card is present, mount fat16 fs
    DebugTrace ("fdsInitSdCard sd presence=%d", status);
    file_mount(fdsConfig, &status);
    DebugTrace("fdsInitSdCard fat16 mount=%d", status);
  }
  
  return (status != 0);
}

void fdsListSdCardDirectory (FdsConfig *fdsConfig)
{
  RET_UNLESS_INIT(fdsConfig);
  bool remainFile = true;
  uint32_t fileNo = 0;
  char fileName[24];
  uint16_t strLen = sizeof(fileName);
  
  while (remainFile) {
    strLen = sizeof(fileName);
    if (fileNo++ == 0) 
      // find first file and report
      file_findFirstRet(fdsConfig, "*.*", &strLen, fileName);
    else
      // find next file and report
      file_findNextRet(fdsConfig, &strLen, fileName);
    
    if (strLen) {
      DebugTrace ("File [%lu] = %s", fileNo, fileName);
    }
  }
}


void fdsSetSoundVolume (FdsConfig *fdsConfig, uint8_t percent)
{
  percent = MIN (100, percent);
  percent = (uint8_t) (percent + 27);
  snd_volume(fdsConfig, percent);
}

void fdsPlayWav (FdsConfig *fdsConfig, const char* fileName)
{  
  file_playWAV(fdsConfig, fileName, NULL);
}

// No buzzer on our OLED-96-G2
/* void fdsPlayBeep (FdsConfig *fdsConfig, uint8_t note, uint16_t duration) */
/* {   */
/*   RET_UNLESS_INIT(fdsConfig); */
/*   RET_UNLESS_GOLDELOX(fdsConfig); */
/*   OLED("%c%c%c%c%c%c", 0xff, 0xda,  */
/* 	0, note, twoBytesFromWord(duration)); */
/* } */

uint32_t fdsOpenFile (FdsConfig *fdsConfig, const char* fileName, uint16_t *handle)
{
  if (fdsIsInitialised(fdsConfig) == false) return 0;

  file_open(fdsConfig, fileName, 'r', handle);
  return fdsGetFileError(fdsConfig);
}


void fdsCloseFile (FdsConfig *fdsConfig, const uint16_t handle)
{
  file_close(fdsConfig, handle, NULL);
}



void fdsDisplayGci (FdsConfig *fdsConfig, const uint16_t handle, uint32_t offset)
{
  uint16_t errno;
  if (fdsFileSeek (fdsConfig, handle, offset)) {
    file_image(fdsConfig, 0, 0, handle, &errno);
     if (errno) {
       DebugTrace ("fdsDisplayGci error %u", errno);
     }
  } else {
    DebugTrace ("fdsDisplayGci fdsFileSeek error");
  }
}

/* va_list argp; */
/* va_start(argp, format); */
/* char char_to_print = va_arg(argp, int); */

bool fdsCallFunction(FdsConfig *fdsConfig, uint16_t handle, uint16_t *retVal, const size_t numArgs, ...)
{
    uint16_t args[numArgs];
    va_list argp;
    va_start(argp, numArgs);
    for (size_t i=0; i< numArgs; i++) {
      args[i] = va_arg(argp, int);
    } 
    return file_callFunction(fdsConfig, handle, numArgs, args, retVal);
}

bool fdsFileRun(FdsConfig *fdsConfig, const char *filename, uint16_t *retVal, const size_t numArgs, ...)
{
    uint16_t args[numArgs];
    va_list argp;
    va_start(argp, numArgs);
    for (size_t i=0; i< numArgs; i++) {
      args[i] = va_arg(argp, int);
    } 
    return file_run(fdsConfig, filename, numArgs, args, retVal);
}

bool fdsFileExec(FdsConfig *fdsConfig, const char *filename, uint16_t *retVal, const size_t numArgs, ...)
{
    uint16_t args[numArgs];
    va_list argp;
    va_start(argp, numArgs);
    for (size_t i=0; i< numArgs; i++) {
      args[i] = va_arg(argp, int);
    } 
    return file_exec(fdsConfig, filename, numArgs, args, retVal);
}

/*
#                 _ __           _                    _                   
#                | '_ \         (_)                  | |                  
#                | |_) |  _ __   _   __   __   __ _  | |_     ___         
#                | .__/  | '__| | |  \ \ / /  / _` | | __|   / _ \        
#                | |     | |    | |   \ V /  | (_| | \ |_   |  __/        
#                |_|     |_|    |_|    \_/    \__,_|  \__|   \___|        
*/
/* static bool fds (uint32_t respLen, const char *fmt, ...) */
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

static  uint16_t fgColorIndexTo16b (const FdsConfig *fdsConfig, const uint8_t colorIndex) {
  const Color24 fg = fdsConfig->fg[colorIndex];
  
  return (gfx_colorDecTo16b(fg.r, fg.g, fg.b));
}

static void fdsScreenSaverTimout (const FdsConfig *fdsConfig, uint16_t timout)
{
  RET_UNLESS_INIT(fdsConfig);
  RET_UNLESS_GOLDELOX(fdsConfig);

  misc_screenSaverTimeout(fdsConfig, timout);
}

static uint32_t fdsGetFileError  (const FdsConfig *fdsConfig)
{
  if (fdsIsInitialised(fdsConfig) == false) return 0;
  uint16_t errno;

  file_error(fdsConfig, &errno);
  return errno;
}

static bool fdsFileSeek  (const FdsConfig *fdsConfig, const uint16_t handle, const uint32_t offset)
{
  if (fdsIsInitialised(fdsConfig) == false) return false;

  const union {
    uint32_t val;
    struct {
      uint16_t lo;
      uint16_t hi;
    };
  } ofst = {.val = offset};
  uint16_t status;
  file_seek(fdsConfig, handle, ofst.hi, ofst.lo, &status);
  
  return status == 1;
}

#if PICASO_DISPLAY_USE_SD
static uint32_t fdsReceiveAnswer (const FdsConfig *oc, uint8_t *response, const size_t size,
				   const char* fct, const uint32_t line)
{
  (void) fct;
  (void) line;

  BaseChannel *serial =  (BaseChannel *)  oc->serial;
  

  const uint32_t ret = chnReadTimeout(serial, response, size, readTimout);

  if (ret != size) {
    DebugTrace ("fdsReceiveAnswer ret[%lu] != expectedSize[%u] @%s : line %lu", ret, size, fct, line);
    fdsStatus = OLED_ERROR;
  } else {
    fdsStatus = OLED_OK;
  }
  return ret;
}
#endif

#if PICASO_DISPLAY_USE_UART

static void fdsStartReceiveAnswer (const FdsConfig *oc, uint8_t *response, const size_t size,
				    const char* fct, const uint32_t line)
{
  (void) fct;
  (void) line;

  UARTDriver *serial = oc->serial;
  response[0] = 0;

  uartStartRead(serial, response, size);
}

static uint32_t fdsWaitReceiveAnswer (const FdsConfig *oc, size_t *size,
				       const char* fct, const uint32_t line)

{
  (void) fct;
  (void) line;

  UARTDriver *serial = oc->serial;

  const size_t ask = *size;
  msg_t status = uartWaitReadTimeout(serial, size, readTimout);
  const size_t ret = *size;

  if (status != MSG_OK) {
    DebugTrace ("fdsWaitReceiveAnswer ask[%u] != read[%u] @%s : line %lu", ask,
		ret, fct, line);
    fdsStatus = OLED_ERROR;
  } else {
    fdsStatus = OLED_OK;
  }
  return ret;
}
#endif


static uint32_t fdsTransmitBuffer (const FdsConfig *oc, const char* fct, const uint32_t line,
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
    fdsStartReceiveAnswer(oc, inBuffer, inSizeRw, fct, line);
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
  ret = fdsReceiveAnswer(oc, inBuffer, inSize, fct, line);
#else
  ret = fdsWaitReceiveAnswer(oc, &inSizeRw, fct, line);
#endif
  
  return ret;
}


OledStatus fdsGetStatus(void)
{
  return fdsStatus;
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

#include "display4DS_ll.c"
