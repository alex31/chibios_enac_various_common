#ifndef __PICASO4_DISPLAY_H__
#define __PICASO4_DISPLAY_H__
#include <ch.h>
#include <hal.h>

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

typedef struct OledConfig  OledConfig;
  enum OledConfig_Device {PICASO=0, GOLDELOX, DIABLO16, TERM_VT100}; 
  enum OledTextAttribute {OLED_RESET_ATTRIB=0,
    OLED_BOLD=16, OLED_ITALIC=32, OLED_INVERSE=64, OLED_UNDERLINE=128};
  enum OledScreenOrientation {OLED_LANDSCAPE=0, OLED_LANDSCAPE_REVERSE,
    OLED_PORTRAIT,    OLED_PORTRAIT_REVERSE};
typedef enum {OLED_OK,
	      OLED_ERROR} OledStatus;
#define COLOR_TABLE_SIZE 11

typedef struct {
    uint16_t x;
    uint16_t y;
} PolyPoint;


// enforce the use of oledStart over oledInit
void __attribute__((deprecated)) oledInit (OledConfig *oledConfig,  LINK_DRIVER *oled,
					   const uint32_t baud,     ioportid_t rstGpio, uint32_t rstPin,
					   enum OledConfig_Device dev);
static inline void oledStart (OledConfig *oledConfig,  LINK_DRIVER *oled, const uint32_t baud,
	        ioline_t reset, enum OledConfig_Device dev)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  oledInit (oledConfig, oled, baud, PAL_PORT(reset), PAL_PAD(reset), dev);
#pragma GCC diagnostic pop  
}
  
void   oledHardReset (OledConfig *oledConfig);
bool oledIsCorrectDevice (OledConfig *oledConfig);
void oledAcquireLock (OledConfig *oledConfig);
void oledReleaseLock (OledConfig *oledConfig);

// replace escape color sequence by color command for respective backend
// ESC c 0 à 9 : couleur index of background and foreground
// replace escape n by carriage return, line feed
void oledPrintFmt (OledConfig *oledConfig, const char *txt, ...)
  __attribute__ ((format (printf, 2, 3)));
;
void oledPrintBuffer (OledConfig *oledConfig, const char *buffer);
void oledGetVersion (OledConfig *oledConfig, char *buffer, const size_t buflen);
void oledChangeBgColor (OledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b);
void oledSetTextFgColor (OledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b);
void oledSetTextBgColor (OledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b);
void oledSetTextBgColorTable (OledConfig *oledConfig, uint8_t colorIndex, 
			      uint8_t r, uint8_t g, uint8_t b);
void oledSetTextFgColorTable (OledConfig *oledConfig,  uint8_t colorIndex, 
			      uint8_t r, uint8_t g, uint8_t b);
void oledUseColorIndex (OledConfig *oledConfig, uint8_t colorIndex);
void oledSetTextOpacity (OledConfig *oledConfig, bool opaque);
void oledSetTextAttributeMask (OledConfig *oledConfig, enum OledTextAttribute attrib);
void oledSetTextGap (OledConfig *oledConfig, uint8_t xgap, uint8_t ygap);
void oledSetTextSizeMultiplier (OledConfig *oledConfig, uint8_t xmul, uint8_t ymul);
void oledSetScreenOrientation (OledConfig *oledConfig, enum OledScreenOrientation orientation);
void oledGotoXY (OledConfig *oledConfig, uint8_t x, uint8_t y);
void oledGotoX (OledConfig *oledConfig, uint8_t x);
uint8_t oledGetX  (const OledConfig *oledConfig);
uint8_t oledGetY  (const OledConfig *oledConfig);
void oledGotoNextLine (OledConfig *oledConfig);
void oledClearScreen (OledConfig *oledConfig);
bool oledInitSdCard (OledConfig *oledConfig);
void oledDrawPoint (OledConfig *oledConfig, const uint16_t x, 
		    const uint16_t y, const uint8_t colorIndex);
void oledDrawLine (OledConfig *oledConfig, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2, 
		   const uint8_t colorIndex);
void oledDrawRect (OledConfig *oledConfig, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2, 
		   const bool filled, const uint8_t colorIndex);
void oledDrawPolyLine (OledConfig *oledConfig, 
		       const uint16_t len,
		       const PolyPoint * const pp,
		       const uint8_t colorIndex);
void oledScreenCopyPaste (OledConfig *oledConfig, 
			  const uint16_t xs, const uint16_t ys, 
			  const uint16_t xd, const uint16_t yd,
			  const uint16_t width, const uint16_t height);
void oledEnableTouch (OledConfig *oledConfig, bool enable);
uint16_t oledTouchGetStatus (OledConfig *oledConfig);
uint16_t oledTouchGetXcoord (OledConfig *oledConfig);
uint16_t oledTouchGetYcoord (OledConfig *oledConfig);
void oledListSdCardDirectory (OledConfig *oledConfig);
void oledSetSoundVolume (OledConfig *oledConfig, uint8_t percent);
void oledPlayWav (OledConfig *oledConfig, const char* fileName);
//void oledPlayBeep (OledConfig *oledConfig, uint8_t note, uint16_t duration);
uint32_t oledOpenFile  (OledConfig *oledConfig, const char* fileName, uint16_t *handle);
void oledCloseFile (OledConfig *oledConfig, const uint16_t handle);
void oledDisplayGci  (OledConfig *oledConfig, const uint16_t handle, uint32_t offset);
void oledSetBaud (OledConfig *oledConfig, uint32_t baud);
OledStatus oledGetStatus(void);

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



struct OledConfig {
#if PICASO_DISPLAY_USE_SD
  SerialConfig serialConfig;
  BaseSequentialStream *serial;
#else
  UARTConfig serialConfig;
  UARTDriver *serial;
#endif
  mutex_t omutex ;
  ioportid_t rstGpio;
  uint32_t rstPin;
  enum OledConfig_Device deviceType;
  // =============
  uint16_t bg;
  Color24 tbg[COLOR_TABLE_SIZE]; 
  Color24 fg[COLOR_TABLE_SIZE]; 
  uint8_t tbgIdx; 
  uint8_t fgIdx; 
  uint8_t curXpos; 
  uint8_t curYpos;   
  uint8_t response[16];
#if PICASO_DISPLAY_USE_UART
 char sendBuffer[80];
#endif
};


#ifdef __cplusplus
}
#endif



#endif //__PICASO4_DISPLAY_H__
