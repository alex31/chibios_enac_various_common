#ifndef __PICASO4_DISPLAY_H__
#define __PICASO4_DISPLAY_H__
#include <ch.h>
#include <hal.h>
#include "display4DS_ll.h"
#ifdef __cplusplus
extern "C" {
#endif


#if !defined(PICASO_DISPLAY_USE_UART)
#define PICASO_DISPLAY_USE_UART FALSE
#endif


#define PICASO_DISPLAY_USE_SD   (!PICASO_DISPLAY_USE_UART)

#if  PICASO_DISPLAY_USE_SD
#define LINK_DRIVER SerialDriver
#else
#define LINK_DRIVER UARTDriver
#endif

typedef struct FdsConfig  FdsConfig;
  enum FdsConfig_Device {GOLDELOX, PICASO, DIABLO16, AUTO_4DS, TERM_VT100}; 
  enum OledTextAttribute {OLED_RESET_ATTRIB=0,
    OLED_BOLD=16, OLED_ITALIC=32, OLED_INVERSE=64, OLED_UNDERLINE=128};
  enum OledScreenOrientation {OLED_LANDSCAPE=0, OLED_LANDSCAPE_REVERSE,
    OLED_PORTRAIT, OLED_PORTRAIT_REVERSE};
typedef enum {OLED_OK,
	      OLED_ERROR} OledStatus;
#define COLOR_TABLE_SIZE 11U

typedef struct {
    uint16_t x;
    uint16_t y;
} PolyPoint;

  
#define gfx_clampColor(r,v,b) ((uint16_t) ((r & 0x1f) <<11 | (v & 0x3f) << 5 | (b & 0x1f)))
#define gfx_colorDecTo16b(r,v,b) (gfx_clampColor((r*31/100), (v*63/100), (b*31/100)))
#define CMD_NOT_IMPL 0xbaba

bool fdsStart (FdsConfig *fdsConfig,  LINK_DRIVER *fds, const uint32_t baud,
		ioline_t reset, enum FdsConfig_Device dev);
void fdsHardReset (FdsConfig *fdsConfig);
bool fdsIsCorrectDevice (FdsConfig *fdsConfig);
void fdsAcquireLock (FdsConfig *fdsConfig);
void fdsReleaseLock (FdsConfig *fdsConfig);

// replace escape color sequence by color command for respective backend
// ESC c 0 à 9 : couleur index of background and foreground
// replace escape n by carriage return, line feed
bool fdsPrintFmt (FdsConfig *fdsConfig, const char *txt, ...)
  __attribute__ ((format (printf, 2, 3)));
;
bool fdsPrintBuffer (FdsConfig *fdsConfig, const char *buffer);
void fdsGetVersion (FdsConfig *fdsConfig, char *buffer, const size_t buflen);
void fdsChangeBgColor (FdsConfig *fdsConfig, uint8_t r, uint8_t g, uint8_t b);
void fdsSetTextFgColor (FdsConfig *fdsConfig, uint8_t r, uint8_t g, uint8_t b);
void fdsSetTextBgColor (FdsConfig *fdsConfig, uint8_t r, uint8_t g, uint8_t b);
void fdsSetTextBgColorTable (FdsConfig *fdsConfig, uint8_t colorIndex, 
			      uint8_t r, uint8_t g, uint8_t b);
void fdsSetTextFgColorTable (FdsConfig *fdsConfig,  uint8_t colorIndex, 
			      uint8_t r, uint8_t g, uint8_t b);
void fdsUseColorIndex (FdsConfig *fdsConfig, uint8_t colorIndex);
void fdsSetTextOpacity (FdsConfig *fdsConfig, bool opaque);
void fdsSetTextAttributeMask (FdsConfig *fdsConfig, enum OledTextAttribute attrib);
void fdsSetTextGap (FdsConfig *fdsConfig, uint8_t xgap, uint8_t ygap);
void fdsSetLuminosity (FdsConfig *fdsConfig, uint8_t luminosity);
void fdsSetTextSizeMultiplier (FdsConfig *fdsConfig, uint8_t xmul, uint8_t ymul);
void fdsSetScreenOrientation (FdsConfig *fdsConfig, enum OledScreenOrientation orientation);
void fdsGotoXY (FdsConfig *fdsConfig, uint8_t x, uint8_t y);
void fdsGotoX (FdsConfig *fdsConfig, uint8_t x);
uint8_t fdsGetX (const FdsConfig *fdsConfig);
uint8_t fdsGetY (const FdsConfig *fdsConfig);
void fdsGotoNextLine (FdsConfig *fdsConfig);
void fdsClearScreen (FdsConfig *fdsConfig);
bool fdsInitSdCard (FdsConfig *fdsConfig);
void fdsDrawPoint (FdsConfig *fdsConfig, const uint16_t x, 
		    const uint16_t y, const uint8_t colorIndex);
void fdsDrawLine (FdsConfig *fdsConfig, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2, 
		   const uint8_t colorIndex);
void fdsDrawRect (FdsConfig *fdsConfig, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2, 
		   const bool filled, const uint8_t colorIndex);
void fdsDrawPolyLine (FdsConfig *fdsConfig, 
		       const uint16_t len,
		       const PolyPoint * const pp,
		       const uint8_t colorIndex);
void fdsScreenCopyPaste (FdsConfig *fdsConfig, 
			  const uint16_t xs, const uint16_t ys, 
			  const uint16_t xd, const uint16_t yd,
			  const uint16_t width, const uint16_t height);
void fdsEnableTouch (FdsConfig *fdsConfig, bool enable);
uint16_t fdsTouchGetStatus (FdsConfig *fdsConfig);
uint16_t fdsTouchGetXcoord (FdsConfig *fdsConfig);
uint16_t fdsTouchGetYcoord (FdsConfig *fdsConfig);
void fdsListSdCardDirectory (FdsConfig *fdsConfig);
void fdsSetSoundVolume (FdsConfig *fdsConfig, uint8_t percent);
void fdsPlayWav (FdsConfig *fdsConfig, const char* fileName);
//void fdsPlayBeep (FdsConfig *fdsConfig, uint8_t note, uint16_t duration);
uint32_t fdsOpenFile  (FdsConfig *fdsConfig, const char* fileName, uint16_t *handle);
void fdsCloseFile (FdsConfig *fdsConfig, const uint16_t handle);
void fdsDisplayGci  (FdsConfig *fdsConfig, const uint16_t handle, uint32_t offset);
bool fdsCallFunction(FdsConfig *fdsConfig, uint16_t handle, uint16_t *retVal, const size_t numArgs, ...);
bool fdsFileRun(FdsConfig *fdsConfig, const char *filename, uint16_t *retVal, const size_t numArgs, ...);
bool fdsFileExec(FdsConfig *fdsConfig, const char *filename, uint16_t *retVal, const size_t numArgs, ...);
bool fdsSetBaud (FdsConfig *fdsConfig, uint32_t baud);
OledStatus fdsGetStatus(void);

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



struct FdsConfig {
#if PICASO_DISPLAY_USE_SD
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
  enum FdsConfig_Device deviceType;
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



#endif //__PICASO4_DISPLAY_H__
