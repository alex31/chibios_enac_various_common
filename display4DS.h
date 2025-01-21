#pragma once
#include <ch.h>
#include <hal.h>
#include "display4DS_ll.h"

#ifdef __cplusplus
extern "C" {
#endif
  
#if !defined(FDS_DISPLAY_USE_UART)
#define FDS_DISPLAY_USE_UART FALSE
#endif
#define FDS_DISPLAY_USE_SD   (!FDS_DISPLAY_USE_UART)
  
#if  FDS_DISPLAY_USE_SD
#define FDS_LINK_DRIVER SerialDriver
#else
#define FDS_LINK_DRIVER UARTDriver
#endif
  
  typedef struct FdsDriver  FdsDriver;
  enum FdsDriver_Device {FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI, FDS_AUTO,
    FDS_TERM_VT100}; 
  enum FdsTextAttribute {FDS_RESET_ATTRIB=0,
    FDS_BOLD=16, FDS_ITALIC=32, FDS_INVERSE=64, FDS_UNDERLINE=128};
  enum FdsScreenOrientation {FDS_LANDSCAPE=0, FDS_LANDSCAPE_REVERSE,
    FDS_PORTRAIT, FDS_PORTRAIT_REVERSE};
typedef enum {FDS_NOTOUCH, FDS_PRESS, FDS_RELEASE, FDS_MOVING} FdsTouchStatus;

typedef enum {FDS_OK,
	      FDS_ERROR} FdsStatus;
#define COLOR_TABLE_SIZE 11U

typedef struct {
    uint16_t x;
    uint16_t y;
} PolyPoint;

  
#define CMD_NOT_IMPL 0xbaba

  
bool fdsStart (FdsDriver *fdsDriver,  FDS_LINK_DRIVER *fds, const uint32_t baud,
		ioline_t reset, enum FdsDriver_Device dev);
void fdsHardReset (FdsDriver *fdsDriver);
bool fdsIsCorrectDevice (FdsDriver *fdsDriver);
void fdsAcquireLock (FdsDriver *fdsDriver);
void fdsReleaseLock (FdsDriver *fdsDriver);

// replace escape color sequence by color command for respective backend
// ESC c 0 à 9 : couleur index of background and foreground
// replace escape n by carriage return, line feed
bool fdsPrintFmt (FdsDriver *fdsDriver, const char *txt, ...)
  __attribute__ ((format (printf, 2, 3)));
;
bool fdsPrintBuffer (FdsDriver *fdsDriver, const char *buffer);
void fdsGetVersion (FdsDriver *fdsDriver, char *buffer, const size_t buflen);
void fdsChangeBgColor (FdsDriver *fdsDriver, uint8_t r, uint8_t g, uint8_t b);
void fdsSetTextFgColor (FdsDriver *fdsDriver, uint8_t r, uint8_t g, uint8_t b);
void fdsSetTextBgColor (FdsDriver *fdsDriver, uint8_t r, uint8_t g, uint8_t b);
void fdsSetTextBgColorTable (FdsDriver *fdsDriver, uint8_t colorIndex, 
			      uint8_t r, uint8_t g, uint8_t b);
void fdsSetTextFgColorTable (FdsDriver *fdsDriver,  uint8_t colorIndex, 
			      uint8_t r, uint8_t g, uint8_t b);
void fdsUseColorIndex (FdsDriver *fdsDriver, uint8_t colorIndex);
void fdsSetTextOpacity (FdsDriver *fdsDriver, bool opaque);
void fdsSetTextAttributeMask (FdsDriver *fdsDriver, enum FdsTextAttribute attrib);
void fdsSetTextGap (FdsDriver *fdsDriver, uint8_t xgap, uint8_t ygap);
void fdsSetLuminosity (FdsDriver *fdsDriver, uint8_t luminosity);
void fdsSetTextSizeMultiplier (FdsDriver *fdsDriver, uint8_t xmul, uint8_t ymul);
void fdsSetScreenOrientation (FdsDriver *fdsDriver, enum FdsScreenOrientation orientation);
void fdsGotoXY (FdsDriver *fdsDriver, uint8_t x, uint8_t y);
void fdsGotoX (FdsDriver *fdsDriver, uint8_t x);
uint8_t fdsGetX (const FdsDriver *fdsDriver);
uint8_t fdsGetY (const FdsDriver *fdsDriver);
void fdsGotoNextLine (FdsDriver *fdsDriver);
void fdsClearScreen (FdsDriver *fdsDriver);
bool fdsInitSdCard (FdsDriver *fdsDriver);
void fdsDrawPoint (FdsDriver *fdsDriver, const uint16_t x, 
		    const uint16_t y, const uint8_t colorIndex);
void fdsDrawLine (FdsDriver *fdsDriver, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2, 
		   const uint8_t colorIndex);
void fdsDrawRect (FdsDriver *fdsDriver, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2, 
		   const bool filled, const uint8_t colorIndex);
void fdsDrawPolyLine (FdsDriver *fdsDriver, 
		       const uint16_t len,
		       const PolyPoint * const pp,
		       const uint8_t colorIndex);
void fdsScreenCopyPaste (FdsDriver *fdsDriver, 
			  const uint16_t xs, const uint16_t ys, 
			  const uint16_t xd, const uint16_t yd,
			  const uint16_t width, const uint16_t height);
void fdsEnableTouch (FdsDriver *fdsDriver, bool enable);
FdsTouchStatus fdsTouchGetStatus (FdsDriver *fdsDriver);
uint16_t fdsTouchGetXcoord (FdsDriver *fdsDriver);
uint16_t fdsTouchGetYcoord (FdsDriver *fdsDriver);
void fdsListSdCardDirectory (FdsDriver *fdsDriver);
void fdsSetSoundVolume (FdsDriver *fdsDriver, uint8_t percent);
void fdsPlayWav (FdsDriver *fdsDriver, const char* fileName);
//void fdsPlayBeep (FdsDriver *fdsDriver, uint8_t note, uint16_t duration);
uint32_t fdsOpenFile  (FdsDriver *fdsDriver, const char* fileName, uint16_t *handle);
void fdsCloseFile (FdsDriver *fdsDriver, const uint16_t handle);
void fdsDisplayGci  (FdsDriver *fdsDriver, const uint16_t handle, uint32_t offset);
bool fdsCallFunction(FdsDriver *fdsDriver, uint16_t handle, uint16_t *retVal, const size_t numArgs, ...);
bool fdsFileRun(FdsDriver *fdsDriver, const char *filename, uint16_t *retVal, const size_t numArgs, ...);
bool fdsFileExec(FdsDriver *fdsDriver, const char *filename, uint16_t *retVal, const size_t numArgs, ...);
bool fdsSetBaud (FdsDriver *fdsDriver, uint32_t baud);
FdsStatus fdsGetStatus(void);
#define fds_clampColor(r,v,b) ((uint16_t) ((r & 0x1f) <<11 | (v & 0x3f) << 5 | (b & 0x1f)))
#define fds_colorDecTo16b(r,v,b) (fds_clampColor((r*31/100), (v*63/100), (b*31/100)))

typedef union  {
  struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };
  uint32_t rgb:24;
} Color24;

  
static inline Color24 mkColor24 (uint8_t r, uint8_t g, uint8_t b) {
  Color24 ret = {{.r=r, .g=g, .b=b}};
  return ret;
}

/*
#                 _ __           _                    _                   
#                | '_ \         (_)                  | |                  
#                | |_) |  _ __   _   __   __   __ _  | |_     ___         
#                | .__/  | '__| | |  \ \ / /  / _` | | __|   / _ \        
#                | |     | |    | |   \ V /  | (_| | \ |_   |  __/        
#                |_|     |_|    |_|    \_/    \__,_|  \__|   \___|        
*/



struct FdsDriver {
#if FDS_DISPLAY_USE_SD
  SerialConfig serialConfig;
  BaseSequentialStream *serial;
#else
  UARTConfig serialConfig;
  UARTDriver *serial;
#endif
  mutex_t omutex ;
  //  ioportid_t rstGpio;
  //  uint32_t rstPin;
  ioline_t rstLine;
  enum FdsDriver_Device deviceType;
  // =============
  uint16_t bg;
  Color24 tbg[COLOR_TABLE_SIZE]; 
  Color24 fg[COLOR_TABLE_SIZE];
  uint8_t colIdx;
  uint8_t curXpos; 
  uint8_t curYpos;
  uint8_t luminosity;
};


#ifdef __cplusplus
}
#endif

