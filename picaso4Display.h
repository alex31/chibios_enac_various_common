#ifndef __PICASO4_DISPLAY_H__
#define __PICASO4_DISPLAY_H__
#include <ch.h>
#include <hal.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct OledConfig  OledConfig;
enum OledConfig_Device {PICASO, GOLDELOX, TERM_VT100}; // will have to implement DIABLO
enum OledTextAttribute {OLED_RESET_ATTRIB=0,
			  OLED_BOLD=16, OLED_ITALIC=32, OLED_INVERSE=64, OLED_UNDERLINE=128};
enum OledScreenOrientation {OLED_LANDSCAPE=0, OLED_LANDSCAPE_REVERSE, OLED_PORTRAIT, 
			    OLED_PORTRAIT_REVERSE};
#define COLOR_TABLE_SIZE 11

void oledInit (OledConfig *oledConfig,  struct SerialDriver *oled, const uint32_t baud,
	       GPIO_TypeDef *rstGpio, uint32_t rstPin, enum OledConfig_Device dev);
void   oledHardReset (OledConfig *oledConfig);
bool_t oledIsCorrectDevice (OledConfig *oledConfig);
void oledAcquireLock (OledConfig *oledConfig);
void oledReleaseLock (OledConfig *oledConfig);

// replace escape color sequence by color command for respective backend
// ESC c 0 à 9 : couleur index of background and foreground
// replace escape n by carriage return, line feed
void oledPrintFmt (OledConfig *oledConfig, const char *txt, ...);
void oledPrintBuffer (OledConfig *oledConfig, const char *buffer);
void oledGetVersion (OledConfig *oledConfig, char *buffer, const size_t buflen);
void oledChangeBgColor (OledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b);
void oledSetTextFgColor (OledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b);
void oledSetTextBgColor (OledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b);
void oledSetTextBgColorTable (OledConfig *oledConfig, uint8_t index, 
			      uint8_t r, uint8_t g, uint8_t b);
void oledSetTextFgColorTable (OledConfig *oledConfig,  uint8_t index, 
			      uint8_t r, uint8_t g, uint8_t b);
void oledUseColorIndex (OledConfig *oledConfig, uint8_t index);
void oledSetTextOpacity (OledConfig *oledConfig, bool_t opaque);
void oledSetTextAttributeMask (OledConfig *oledConfig, enum OledTextAttribute attrib);
void oledSetTextGap (OledConfig *oledConfig, uint8_t xgap, uint8_t ygap);
void oledSetScreenOrientation (OledConfig *oledConfig, enum OledScreenOrientation orientation);
void oledGotoXY (OledConfig *oledConfig, uint8_t x, uint8_t y);
void oledGotoX (OledConfig *oledConfig, uint8_t x);
void oledGotoNextLine (OledConfig *oledConfig);
void oledClearScreen (OledConfig *oledConfig);
bool_t oledInitSdCard (OledConfig *oledConfig);
void oledDrawPoint (OledConfig *oledConfig, const uint16_t x, 
		    const uint16_t y, const uint8_t index);
void oledDrawLine (OledConfig *oledConfig, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2, 
		   const uint8_t index);
void oledDrawRect (OledConfig *oledConfig, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2, 
		   const bool_t filled, const uint8_t index);
void oledEnableTouch (OledConfig *oledConfig, bool_t enable);
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
  SerialConfig serialConfig;
  Mutex omutex ;
  BaseSequentialStream *serial;
  GPIO_TypeDef *rstGpio;
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
};


#ifdef __cplusplus
}
#endif



#endif //__PICASO4_DISPLAY_H__
