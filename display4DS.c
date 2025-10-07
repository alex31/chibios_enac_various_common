#include <ch.h>
#include <hal.h>
#include <string.h>
#include <ctype.h>
#include "stdutil.h"
#include "printf.h"
#include "display4DS.h"
#include "display4DS_ll.h"


#define QDS_ACK 0x6

typedef enum {KOF_NONE, KOF_ACK, KOF_INT16, 
	      KOF_INT16LENGTH_THEN_DATA} KindOfCommand;

const uint32_t readTimout = TIME_MS2I(500);
static FdsStatus fdsStatus = FDS_OK;


static uint32_t fdsTransmitBuffer (const FdsDriver *oc, const char* fct, const uint32_t line,
					 const uint8_t *outBuffer, const size_t outSize,
					 uint8_t *inBuffer, const size_t inSize);
#if FDS_DISPLAY_USE_SD
static uint32_t fdsReceiveAnswer (const FdsDriver *oc, uint8_t *response, const size_t size,
				   const char* fct, const uint32_t line);
static void sendVt100Seq (BaseSequentialStream *serial, const char *fmt, ...);
#else
static void sendVt100Seq (UARTDriver *serial, const char *fmt, ...);
static void fdsStartReceiveAnswer (const FdsDriver *oc, uint8_t *response, const size_t size,
				     const char* fct, const uint32_t line);
static uint32_t fdsWaitReceiveAnswer (const FdsDriver *oc, size_t *size,
					const char* fct, const uint32_t line);
#endif
static bool findDeviceType (FdsDriver *fdsDriver);

#if CH_DBG_ENABLE_ASSERTS 
#define RET_UNLESS_INIT(fdsCfg)    {if (fdsIsInitialised(fdsCfg) == false) return ;}
#define RET_UNLESS_4DSYS(fdsCfg)  {if (fdsCfg->deviceType == FDS_TERM_VT100) return ;}
#define RET_UNLESS_GOLDELOX(fdsCfg)  {if (fdsCfg->deviceType != FDS_GOLDELOX) return ;}

#define RET_UNLESS_INIT_BOOL(fdsCfg)    {if (fdsIsInitialised(fdsCfg) == false) return false;}
#define RET_UNLESS_4DSYS_BOOL(fdsCfg)  {if (fdsCfg->deviceType == FDS_TERM_VT100) return false;}
#define RET_UNLESS_GOLDELOX_BOOL(fdsCfg)  {if (fdsCfg->deviceType != FDS_GOLDELOX) return false;}
#else
#define RET_UNLESS_INIT(fdsCfg)
#define RET_UNLESS_4DSYS(fdsCfg)
#define RET_UNLESS_GOLDELOX(fdsCfg)

#define RET_UNLESS_INIT_BOOL(fdsCfg)
#define RET_UNLESS_4DSYS_BOOL(fdsCfg)
#define RET_UNLESS_GOLDELOX_BOOL(fdsCfg)
#endif

static bool fdsIsInitialised (const FdsDriver *fdsDriver) ;
static void fdsScreenSaverTimout (const FdsDriver *fdsDriver, uint16_t timout);
static void fdsPreInit (FdsDriver *fdsDriver, uint32_t baud);
static uint32_t fdsGetFileError  (const FdsDriver *fdsDriver);
static bool fdsFileSeek  (const FdsDriver *fdsDriver, const uint16_t handle, const uint32_t offset);
static uint16_t fgColorIndexTo16b (const FdsDriver *fdsDriver, const uint8_t colorIndex);
static uint8_t colorDimmed (const uint8_t channel, const uint8_t luminosity);
static uint16_t fdsTouchGet (const FdsDriver *fdsDriver, uint16_t mode);
static uint16_t getResponseAsUint16 (const uint8_t *buff);
#if FDS_DISPLAY_USE_UART
static void uartStartRead (UARTDriver *serial, uint8_t *response, 
			   const size_t size);
static msg_t uartWaitReadTimeout(UARTDriver *serial, size_t *size, sysinterval_t  rTimout);
#endif


static uint8_t colorDimmed (const uint8_t channel, const uint8_t luminosity)
{
  return ((uint32_t) channel * luminosity) / 100U;
}


static void fdsPreInit (FdsDriver *fdsDriver, uint32_t baud)
{
  fdsDriver->bg = fds_colorDecTo16b(0,0,0);
  fdsDriver->tbg[0] = mkColor24 (0,0,0);
  fdsDriver->fg[0] = mkColor24(50,50,50);
  fdsDriver->colIdx = 0;
  fdsDriver->curXpos = 0;
  fdsDriver->curYpos = 0;
  fdsDriver->luminosity = 100U;
  fdsDriver->serial = NULL;

  // initial USART conf only for 4D system screen
  // 9600 bauds because of broken auto baud feature on some screen model
  memset(&fdsDriver->serialConfig, 0, sizeof(fdsDriver->serialConfig));
  fdsDriver->serialConfig.speed = baud;
  fdsDriver->serialConfig.cr1 = 0;
  fdsDriver->serialConfig.cr2 = USART_CR2_STOP1_BITS | USART_CR2_LINEN;
  fdsDriver->serialConfig.cr3 = 0;
}

bool fdsStart (FdsDriver *fdsDriver,  FDS_LINK_DRIVER *fds, const uint32_t baud,
		ioline_t rstLine, enum FdsDriver_Device dev)
{
  fdsDriver->rstLine = rstLine;
  fdsDriver->deviceType = dev;

  fdsHardReset(fdsDriver);


  fdsPreInit(fdsDriver,
	       fdsDriver->deviceType == FDS_TERM_VT100 ? baud : 9600);
  chMtxObjectInit(&(fdsDriver->omutex));
#if FDS_DISPLAY_USE_SD
  fdsDriver->serial = (BaseSequentialStream *) fds;
  sdStart(fds, &(fdsDriver->serialConfig));
#else
  fdsDriver->serial = fds;
  uartStart(fds, &(fdsDriver->serialConfig));
#endif  
  chThdSleepMilliseconds(10);


  // test is no error on kind of device : picaso, goldelox, diablo ...
  if (!fdsIsCorrectDevice(fdsDriver)) {
    DebugTrace("Error incorrect device type @ %s:%d", __FUNCTION__, __LINE__);
    return false;
  }

  // opaque background
  fdsClearScreen(fdsDriver);
  fdsSetTextOpacity(fdsDriver, true);

  // disable screensaver : 0 is special value for disabling screensaver
  // since fds has remanance problem, we activate screensaver after 20 seconds
  fdsScreenSaverTimout(fdsDriver, 20000);
  
  // use greater speed
  if ((fdsDriver->deviceType != FDS_TERM_VT100) && (baud != 9600))
    return fdsSetBaud(fdsDriver, baud);
  return true;
}


void fdsHardReset (FdsDriver *fdsDriver)
{

  RET_UNLESS_4DSYS(fdsDriver);

  palClearLine(fdsDriver->rstLine);
  chThdSleepMilliseconds(10);
  palSetLine(fdsDriver->rstLine);
  chThdSleepMilliseconds(3000);
}


bool fdsIsInitialised (const FdsDriver *fdsDriver)
{
  return (fdsDriver->serial != NULL);
}

void fdsAcquireLock (FdsDriver *fdsDriver)
{ 
  RET_UNLESS_INIT(fdsDriver);
  chMtxLock(&(fdsDriver->omutex));
}

void fdsReleaseLock (FdsDriver *fdsDriver)
{ 
  RET_UNLESS_INIT(fdsDriver);
  (void) fdsDriver;
  chMtxUnlock(&(fdsDriver->omutex));
}


void fdsGetVersion (FdsDriver *fdsDriver, char *buffer, const size_t buflen)
{
  RET_UNLESS_4DSYS(fdsDriver);

   // get display model
  uint16_t modelLen = buflen;
  sys_getModel(fdsDriver, &modelLen, buffer);

  if (modelLen > buflen) {
    DebugTrace("warning %s need bigger buffer [%u instead %u] to store returned string",
	       __FUNCTION__, modelLen, buflen);
  }

  // get Pmmc version
  uint16_t pmmc=0;
  sys_getPmmC(fdsDriver, &pmmc);
  const uint8_t pmmcMajor = (uint8_t) (pmmc>>8);
  const uint8_t pmmcMinor =  (uint8_t) (pmmc & 0xff);
  chsnprintf(&buffer[strlen(buffer)], buflen-strlen(buffer), " Pmmc=%d.%d", pmmcMajor, pmmcMinor);

  // get SPE version
  uint16_t spe=0;
  sys_getVersion(fdsDriver, &spe);
  const uint8_t speMajor = (uint8_t) (spe >> 8);
  const uint8_t speMinor = (uint8_t) (spe  & 0xff);
  chsnprintf(&buffer[strlen(buffer)], buflen-strlen(buffer), " Spe=%d.%d", speMajor, speMinor);

  // string terminaison
  buffer[buflen-1]=0;
}


bool fdsSetBaud (FdsDriver *fdsDriver, uint32_t baud)
{
  uint32_t actualbaudRate; 
  uint16_t baudCode = 0;

  FDS_LINK_DRIVER *sd = (FDS_LINK_DRIVER *) fdsDriver->serial;

  RET_UNLESS_INIT_BOOL(fdsDriver);
  
  switch (fdsDriver->deviceType) {
  case FDS_DIABLO16: {
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
    fdsDriver->serialConfig.speed = actualbaudRate; 
  }
    break;

  case FDS_PIXXI:
  case FDS_PICASO: {
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
    fdsDriver->serialConfig.speed = actualbaudRate; 
  }
    break;
  case FDS_GOLDELOX: {
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
    fdsDriver->serialConfig.speed = actualbaudRate; 
  }
    break;

  default:
    fdsDriver->serialConfig.speed = baud;
    break;
  }

  misc_setbaudWait(fdsDriver, baudCode);
  chThdSleepMilliseconds(10);
#if FDS_DISPLAY_USE_SD
  sdStop(sd);
  sdStart(sd, &(fdsDriver->serialConfig));
#else
  uartStop(sd);
  uartStart(sd, &(fdsDriver->serialConfig));
#endif
  // wait for response at new speed
  RET_UNLESS_4DSYS_BOOL(fdsDriver);

  struct {
   uint8_t ack;
 } response;
 return fdsTransmitBuffer(fdsDriver, __FUNCTION__, __LINE__,
			   NULL, 0,
			   (uint8_t *) &response, sizeof(response)) == 1; 
}


bool fdsPrintFmt (FdsDriver *fdsDriver, const char *fmt, ...)
{
  char buffer[120];
  char *token, *curBuf;
  bool lastLoop = false;
  bool ret = false;

  va_list ap;
  RET_UNLESS_INIT_BOOL(fdsDriver);

  va_start(ap, fmt);
  chvsnprintf(buffer, sizeof(buffer), fmt, ap);
  va_end(ap);

  if (buffer[0] == 0)
    return true;

  if (buffer[1] == 0) {
    return fdsPrintBuffer(fdsDriver, buffer);
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
      ret = fdsPrintBuffer(fdsDriver, curBuf);
      fdsDriver->curXpos =  (uint8_t) (fdsDriver->curXpos + strnlen(curBuf, sizeof(buffer)));
    }
    
    if (lastLoop == false) {
      // next two char a color coding scheme
      if (tolower((uint32_t) (*token)) == 'c') { 
	const int32_t colorIndex = *++token - '0';
	fdsUseColorIndex(fdsDriver, (uint8_t) colorIndex);
	//	DebugTrace ("useColorIndex %d", colorIndex);
	curBuf=token+1;
      } else if (tolower((uint32_t) (*token)) == 'n') { 	
	//	DebugTrace ("carriage return");
	fdsGotoXY(fdsDriver, 0,  (uint8_t) (fdsDriver->curYpos+1));
	curBuf=token+1;
      } else if (tolower((uint32_t) (*token)) == 't') { 	
	//	DebugTrace ("tabulation");
	const uint8_t tabLength =  (uint8_t) (8-(fdsDriver->curXpos%8));
	char space[8] = {[0 ... 7] = ' '};
	space[tabLength] = 0;
	ret = fdsPrintBuffer(fdsDriver, space);
	fdsGotoX(fdsDriver, (uint8_t) (fdsDriver->curXpos + tabLength));
	curBuf=token+1;
      }
    }
  }
  return ret;
}

bool fdsPrintBuffer (FdsDriver *fdsDriver, const char *buffer)
{
  RET_UNLESS_INIT_BOOL(fdsDriver);
  bool ret = false;
  switch(fdsDriver->deviceType) {
  case FDS_GOLDELOX :
  case FDS_PIXXI:
  case FDS_PICASO :
  case FDS_DIABLO16 : 
    ret = txt_moveCursor(fdsDriver, fdsDriver->curYpos, fdsDriver->curXpos);
    if (ret ==  true)
      ret = txt_putStr(fdsDriver, buffer, NULL);
    break;
  case FDS_TERM_VT100 : 
    sendVt100Seq(fdsDriver->serial, "%d;%dH",  fdsDriver->curYpos+1, fdsDriver->curXpos+1);
    #if FDS_DISPLAY_USE_SD
    directchprintf(fdsDriver->serial, buffer);
#else
    size_t length = strnlen(buffer, sizeof(buffer));
    uartSendTimeout(fdsDriver->serial, &length, buffer, TIME_INFINITE);
#endif
    break;
  default: osalSysHalt("incorrect fdsDriver->deviceType");
  }
  return ret;
}

void fdsChangeBgColor (FdsDriver *fdsDriver, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(fdsDriver);
  
  const uint16_t oldCol = fdsDriver->bg;
  const uint16_t newCol = fdsDriver->bg = fds_colorDecTo16b(r,g,b);
  RET_UNLESS_4DSYS(fdsDriver);
  gfx_changeColour(fdsDriver, oldCol, newCol);
} 


void fdsSetTextBgColor (FdsDriver *fdsDriver, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(fdsDriver);

  fdsDriver->tbg[0] = mkColor24(r,g,b);
  switch(fdsDriver->deviceType) {
  case FDS_GOLDELOX :
  case FDS_PIXXI:
  case FDS_PICASO : 
  case FDS_DIABLO16 :
    txt_bgColour(fdsDriver,  fds_colorDecTo16b(r,g,b), NULL);
    break;
  case FDS_TERM_VT100 : 
    sendVt100Seq(fdsDriver->serial, "48;2;%d;%d;%dm", r*255/100, g*255/100, b*255/100);
    break;
  default: osalSysHalt("incorrect fdsDriver->deviceType");
  } 
}

void fdsSetTextFgColor (FdsDriver *fdsDriver, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(fdsDriver);
  
  fdsDriver->fg[0] = mkColor24(r,g,b);
  switch(fdsDriver->deviceType) {
  case FDS_GOLDELOX :
  case FDS_PIXXI:
  case FDS_PICASO : 
  case FDS_DIABLO16 :
    txt_fgColour(fdsDriver,  fds_colorDecTo16b(r,g,b), NULL);
    break;
  case FDS_TERM_VT100 : 
    sendVt100Seq(fdsDriver->serial, "38;2;%d;%d;%dm", r*255/100, g*255/100, b*255/100);
    break;
  default: osalSysHalt("incorrect fdsDriver->deviceType");
  } 
}



void fdsSetTextBgColorTable (FdsDriver *fdsDriver, uint8_t colorIndex, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(fdsDriver);
  if (++colorIndex >= COLOR_TABLE_SIZE) return;

  fdsDriver->tbg[colorIndex] = mkColor24(r,g,b);
}

void fdsSetTextFgColorTable (FdsDriver *fdsDriver,  uint8_t colorIndex, uint8_t r, uint8_t g, uint8_t b)
{
  RET_UNLESS_INIT(fdsDriver);
  if (++colorIndex >= COLOR_TABLE_SIZE) return;

  fdsDriver->fg[colorIndex] = mkColor24(r,g,b);
}

void fdsUseColorIndex (FdsDriver *fdsDriver, uint8_t colorIndex)
{
  RET_UNLESS_INIT(fdsDriver);
  if (++colorIndex >= COLOR_TABLE_SIZE) return;
  fdsDriver->colIdx = colorIndex;
  if (fdsDriver->deviceType != FDS_TERM_VT100) {
    if (fdsDriver->fg[0].rgb != fdsDriver->fg[colorIndex].rgb)  {
      fdsSetTextFgColor(fdsDriver, colorDimmed(fdsDriver->fg[colorIndex].r, fdsDriver->luminosity), 
			 colorDimmed(fdsDriver->fg[colorIndex].g, fdsDriver->luminosity),
			 colorDimmed(fdsDriver->fg[colorIndex].b, fdsDriver->luminosity));
    }
    
    if (fdsDriver->tbg[0].rgb != fdsDriver->tbg[colorIndex].rgb)  {
      fdsSetTextBgColor(fdsDriver, colorDimmed(fdsDriver->tbg[colorIndex].r, fdsDriver->luminosity), 
			 colorDimmed(fdsDriver->tbg[colorIndex].g, fdsDriver->luminosity),
			 colorDimmed(fdsDriver->tbg[colorIndex].b, fdsDriver->luminosity));
    }
  } else {
    fdsSetTextFgColor(fdsDriver, colorDimmed(fdsDriver->fg[colorIndex].r, fdsDriver->luminosity), 
		       colorDimmed(fdsDriver->fg[colorIndex].g, fdsDriver->luminosity),
		       colorDimmed(fdsDriver->fg[colorIndex].b, fdsDriver->luminosity));
    fdsSetTextBgColor(fdsDriver, colorDimmed(fdsDriver->tbg[colorIndex].r, fdsDriver->luminosity), 
		       colorDimmed(fdsDriver->tbg[colorIndex].g, fdsDriver->luminosity),
		       colorDimmed(fdsDriver->tbg[colorIndex].b, fdsDriver->luminosity));
  }
}


void fdsSetTextOpacity (FdsDriver *fdsDriver, bool opaque)
{
  if (fdsDriver->deviceType == FDS_GOLDELOX)
    txt_opacity(fdsDriver, opaque, NULL);
}


void fdsSetTextAttributeMask (FdsDriver *fdsDriver, enum FdsTextAttribute attrib)
{
  txt_attributes(fdsDriver, attrib, NULL);
}

void fdsSetTextGap (FdsDriver *fdsDriver, uint8_t xgap, uint8_t ygap)
{
  txt_xgap(fdsDriver, xgap, NULL);
  txt_ygap(fdsDriver, ygap, NULL);
}

void fdsSetTextSizeMultiplier (FdsDriver *fdsDriver, uint8_t xmul, uint8_t ymul)
{
  txt_widthMult(fdsDriver, xmul, NULL);
  txt_heightMult(fdsDriver, ymul, NULL);
}

void fdsSetScreenOrientation (FdsDriver *fdsDriver, enum FdsScreenOrientation orientation)
{
  gfx_screenMode(fdsDriver, orientation, NULL);
}


void fdsGotoXY (FdsDriver *fdsDriver, uint8_t x, uint8_t y)
{  
  RET_UNLESS_INIT(fdsDriver);

   fdsDriver->curXpos=x;
   fdsDriver->curYpos=y;

   if (fdsDriver->deviceType == FDS_TERM_VT100) {
     sendVt100Seq(fdsDriver->serial, "%d;%dH", y+1,x+1);
   } 
}

void fdsGotoX (FdsDriver *fdsDriver, uint8_t x)
{  
  RET_UNLESS_INIT(fdsDriver);

  fdsDriver->curXpos=x;
  if (fdsDriver->deviceType == FDS_TERM_VT100) {
    sendVt100Seq(fdsDriver->serial, "%d;%dH", fdsDriver->curYpos+1,x+1);
  }
}

uint8_t fdsGetX  (const FdsDriver *fdsDriver)
{  
  if (fdsIsInitialised(fdsDriver) == false)
    return 0;
  
  return fdsDriver->curXpos;
}

uint8_t fdsGetY  (const FdsDriver *fdsDriver)
{  
  if (fdsIsInitialised(fdsDriver) == false)
    return 0;
  
  return fdsDriver->curYpos;
}

void fdsGotoNextLine (FdsDriver *fdsDriver)
{  
  RET_UNLESS_INIT(fdsDriver);

  fdsDriver->curXpos=0;
  fdsDriver->curYpos++;
  if (fdsDriver->deviceType == FDS_TERM_VT100) {
    sendVt100Seq(fdsDriver->serial, "B");
    //    chprintf(fdsDriver->serial, "\r\n");
  }
}

void fdsSetLuminosity (FdsDriver *fdsDriver, uint8_t luminosity)
{
  fdsDriver->luminosity = luminosity;
  //  fdsUseColorIndex(fdsDriver, fdsDriver->colIdx);
}


bool fdsIsCorrectDevice (FdsDriver *fdsDriver)
{
  if (fdsIsInitialised(fdsDriver) == false) 
    return false;

  if (fdsDriver->deviceType == FDS_TERM_VT100) 
    return true;

  if (fdsDriver->deviceType == FDS_AUTO) 
    return findDeviceType(fdsDriver);
  else
    return (gfx_cls(fdsDriver) && gfx_cls(fdsDriver));
}

static bool findDeviceType (FdsDriver *fdsDriver)
{
  if (fdsIsInitialised(fdsDriver) == false) 
    return false;

  if (fdsDriver->deviceType == FDS_TERM_VT100) 
    return true;

  for (enum FdsDriver_Device type=FDS_GOLDELOX; type <= FDS_DIABLO16; type++) {
    fdsDriver->deviceType = type;
    if (fdsIsCorrectDevice(fdsDriver)) {
      DebugTrace("find correct device type = %u", type);
      return true;
    } else {
      fdsHardReset(fdsDriver);
    }
  }
  return false;
}

void fdsClearScreen (FdsDriver *fdsDriver)
{
  RET_UNLESS_INIT(fdsDriver);

  if (fdsDriver->deviceType == FDS_TERM_VT100) {
    sendVt100Seq(fdsDriver->serial, "2J");
  } else {
    gfx_cls(fdsDriver);
  }
}


void fdsDrawPoint (FdsDriver *fdsDriver, const uint16_t x, const uint16_t y, 
		    const uint8_t colorIndex)
{
  RET_UNLESS_INIT(fdsDriver);

  const uint16_t fg = fgColorIndexTo16b(fdsDriver, (uint8_t) (colorIndex+1));
  gfx_putPixel(fdsDriver, x, y, fg);
}


void fdsDrawLine (FdsDriver *fdsDriver, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2, 
		   const uint8_t colorIndex)
{
  RET_UNLESS_INIT(fdsDriver);

  const uint16_t fg = fgColorIndexTo16b(fdsDriver, (uint8_t) (colorIndex+1));
  gfx_line(fdsDriver, x1, y1, x2, y2, fg);
}

void fdsDrawRect (FdsDriver *fdsDriver, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2,
		   const bool filled,
		   const uint8_t colorIndex)
{
  RET_UNLESS_INIT(fdsDriver);

  const uint16_t fg = fgColorIndexTo16b(fdsDriver, (uint8_t) (colorIndex+1));
  if (filled) 
    gfx_rectangleFilled(fdsDriver, x1, y1, x2, y2, fg);
  else
    gfx_rectangle(fdsDriver, x1, y1, x2, y2, fg);
 }

void fdsDrawPolyLine (FdsDriver *fdsDriver, 
		      uint16_t len,
		      const PolyPoint * const pp,
		      const uint8_t colorIndex)
{
  RET_UNLESS_INIT(fdsDriver);
  if (len > 300) len = 300;
  struct {
    uint16_t vx[len];
    uint16_t vy[len];
    uint16_t color;
  } command;

  for (size_t i=0; i<len; i++) {
    command.vx[i] = __builtin_bswap16(pp[i].x);
    command.vy[i] = __builtin_bswap16(pp[i].y);
  }
  command.color =  fgColorIndexTo16b(fdsDriver, (uint8_t) (colorIndex+1));
  gfx_polyline(fdsDriver, len, command.vx, command.vy, command.color);
}


/*
  API available for PICASO and DIABLO, not for GOLDELOX 

 */
void fdsScreenCopyPaste (FdsDriver *fdsDriver, 
			  const uint16_t xs, const uint16_t ys, 
			  const uint16_t xd, const uint16_t yd,
			  const uint16_t width, const uint16_t height)
{
  RET_UNLESS_INIT(fdsDriver);
  gfx_screenCopyPaste(fdsDriver, xs, ys, xd, yd, width, height);
}


void fdsEnableTouch (FdsDriver *fdsDriver, bool enable)
{
   touch_set(fdsDriver, enable ? 0x00 : 0x01);
}

static uint16_t fdsTouchGet (const FdsDriver *fdsDriver, uint16_t mode)
{
  if (fdsIsInitialised(fdsDriver) == false) 
    return 0xff;
  uint16_t value = 0;
  
  touch_get(fdsDriver, mode, &value);
  return value;
}


FdsTouchStatus fdsTouchGetStatus (FdsDriver *fdsDriver)
{
  return fdsTouchGet(fdsDriver, 0);
}

uint16_t fdsTouchGetXcoord (FdsDriver *fdsDriver)
{
  return fdsTouchGet(fdsDriver, 1);
}

uint16_t fdsTouchGetYcoord (FdsDriver *fdsDriver)
{
  return fdsTouchGet(fdsDriver, 2);
}




bool fdsInitSdCard (FdsDriver *fdsDriver)
{ 
  if (fdsIsInitialised(fdsDriver) == false) return false;

  uint16_t status = 0;
  media_init(fdsDriver, &status);

  if (status != 0) {
    // card is present, mount fat16 fs
    DebugTrace ("fdsInitSdCard sd presence=%d", status);
    file_mount(fdsDriver, &status);
    DebugTrace("fdsInitSdCard fat16 mount=%d", status);
  }
  
  return (status != 0);
}

void fdsListSdCardDirectory (FdsDriver *fdsDriver)
{
  RET_UNLESS_INIT(fdsDriver);
  bool remainFile = true;
  uint32_t fileNo = 0;
  char fileName[24];
  uint16_t strLen = sizeof(fileName);
  
  while (remainFile) {
    strLen = sizeof(fileName);
    if (fileNo++ == 0) 
      // find first file and report
      file_findFirstRet(fdsDriver, "*.*", &strLen, fileName);
    else
      // find next file and report
      file_findNextRet(fdsDriver, &strLen, fileName);
    
    if (strLen) {
      DebugTrace ("File [%lu] = %s", fileNo, fileName);
    }
  }
}


void fdsSetSoundVolume (FdsDriver *fdsDriver, uint8_t percent)
{
  percent = MIN (100, percent);
  percent = (uint8_t) (percent + 27);
  snd_volume(fdsDriver, percent);
}

void fdsPlayWav (FdsDriver *fdsDriver, const char* fileName)
{  
  file_playWAV(fdsDriver, fileName, NULL);
}

// No buzzer on our OLED-96-G2
/* void fdsPlayBeep (FdsDriver *fdsDriver, uint8_t note, uint16_t duration) */
/* {   */
/*   RET_UNLESS_INIT(fdsDriver); */
/*   RET_UNLESS_GOLDELOX(fdsDriver); */
/*   OLED("%c%c%c%c%c%c", 0xff, 0xda,  */
/* 	0, note, twoBytesFromWord(duration)); */
/* } */

uint32_t fdsOpenFile (FdsDriver *fdsDriver, const char* fileName, uint16_t *handle)
{
  if (fdsIsInitialised(fdsDriver) == false) return 0;

  file_open(fdsDriver, fileName, 'r', handle);
  return fdsGetFileError(fdsDriver);
}


void fdsCloseFile (FdsDriver *fdsDriver, const uint16_t handle)
{
  file_close(fdsDriver, handle, NULL);
}



void fdsDisplayGci (FdsDriver *fdsDriver, const uint16_t handle, uint32_t offset)
{
  uint16_t errno = 0;
  if (fdsFileSeek (fdsDriver, handle, offset)) {
    file_image(fdsDriver, 0, 0, handle, &errno);
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

bool fdsCallFunction(FdsDriver *fdsDriver, uint16_t handle, uint16_t *retVal, size_t numArgs, ...)
{
    if (numArgs > 128) numArgs = 128;
    uint16_t args[numArgs];
    va_list argp;
    va_start(argp, numArgs);
    for (size_t i=0; i< numArgs; i++) {
      args[i] = va_arg(argp, int);
    }
    va_end(argp);
    return file_callFunction(fdsDriver, handle, numArgs, args, retVal);
}

bool fdsFileRun(FdsDriver *fdsDriver, const char *filename, uint16_t *retVal, size_t numArgs, ...)
{
    if (numArgs > 128) numArgs = 128;
    uint16_t args[numArgs];
    va_list argp;
    va_start(argp, numArgs);
    for (size_t i=0; i< numArgs; i++) {
      args[i] = va_arg(argp, int);
    }
    va_end(argp);
    return file_run(fdsDriver, filename, numArgs, args, retVal);
}

bool fdsFileExec(FdsDriver *fdsDriver, const char *filename, uint16_t *retVal, size_t numArgs, ...)
{
    if (numArgs > 128) numArgs = 128;
    uint16_t args[numArgs];
    va_list argp;
    va_start(argp, numArgs);
    for (size_t i=0; i< numArgs; i++) {
      args[i] = va_arg(argp, int);
    }
    va_end(argp);
    return file_exec(fdsDriver, filename, numArgs, args, retVal);
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

static  uint16_t fgColorIndexTo16b (const FdsDriver *fdsDriver, const uint8_t colorIndex) {
  const Color24 fg = fdsDriver->fg[colorIndex];
  
  return (fds_colorDecTo16b(fg.r, fg.g, fg.b));
}

static void fdsScreenSaverTimout (const FdsDriver *fdsDriver, uint16_t timout)
{
  RET_UNLESS_INIT(fdsDriver);
  RET_UNLESS_GOLDELOX(fdsDriver);

  misc_screenSaverTimeout(fdsDriver, timout);
}

static uint32_t fdsGetFileError  (const FdsDriver *fdsDriver)
{
  if (fdsIsInitialised(fdsDriver) == false) return 0;
  uint16_t errno = 0;

  file_error(fdsDriver, &errno);
  return errno;
}

static bool fdsFileSeek  (const FdsDriver *fdsDriver, const uint16_t handle, const uint32_t offset)
{
  if (fdsIsInitialised(fdsDriver) == false) return false;

  const union {
    uint32_t val;
    struct {
      uint16_t lo;
      uint16_t hi;
    };
  } ofst = {.val = offset};
  uint16_t status = 0;
  file_seek(fdsDriver, handle, ofst.hi, ofst.lo, &status);
  
  return status == 1;
}

#if FDS_DISPLAY_USE_SD
static uint32_t fdsReceiveAnswer (const FdsDriver *oc, uint8_t *response, const size_t size,
				   const char* fct, const uint32_t line)
{
  (void) fct;
  (void) line;

  BaseChannel *serial =  (BaseChannel *)  oc->serial;
  

  const uint32_t ret = chnReadTimeout(serial, response, size, readTimout);

  if (ret != size) {
    DebugTrace ("fdsReceiveAnswer ret[%lu] != expectedSize[%u] @%s : line %lu", ret, size, fct, line);
    fdsStatus = FDS_ERROR;
  } else {
    fdsStatus = FDS_OK;
  }
  return ret;
}
#endif

#if FDS_DISPLAY_USE_UART

static void fdsStartReceiveAnswer (const FdsDriver *oc, uint8_t *response, const size_t size,
				    const char* fct, const uint32_t line)
{
  (void) fct;
  (void) line;

  UARTDriver *serial = oc->serial;
  response[0] = 0;

  uartStartRead(serial, response, size);
}

static uint32_t fdsWaitReceiveAnswer (const FdsDriver *oc, size_t *size,
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
    fdsStatus = FDS_ERROR;
  } else {
    fdsStatus = FDS_OK;
  }
  return ret;
}
#endif


static uint32_t fdsTransmitBuffer (const FdsDriver *oc, const char* fct, const uint32_t line,
				    const uint8_t *outBuffer, const size_t outSize,
				    uint8_t *inBuffer, const size_t inSize)
{
  uint32_t ret = 0;

#if FDS_DISPLAY_USE_SD
  BaseChannel * serial =  (BaseChannel *)  oc->serial;
  if (inSize != 0)
    chnReadTimeout (serial, inBuffer, inSize, TIME_IMMEDIATE);
#else
  size_t inSizeRw = inSize;
#endif
  
  // send command
#if FDS_DISPLAY_USE_SD
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

#if FDS_DISPLAY_USE_SD
  ret = fdsReceiveAnswer(oc, inBuffer, inSize, fct, line);
#else
  ret = fdsWaitReceiveAnswer(oc, &inSizeRw, fct, line);
#endif
  
  return ret;
}


FdsStatus fdsGetStatus(void)
{
  return fdsStatus;
}

#if FDS_DISPLAY_USE_SD
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
  if (serial->rxstate != UART_RX_ACTIVE) {
    osalSysUnlock();
    return MSG_RESET;
  }
					
  msg = osalThreadSuspendTimeoutS(&serial->threadrx, rTimout);
  if (msg != MSG_OK) {
    *size -= uartStopReceiveI(serial);
  }
  osalSysUnlock();
  
  return msg;
}
#endif

#include "display4DS_ll.c"
