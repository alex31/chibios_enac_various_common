#ifndef __PICASO4_DISPLAY_H__
#define __PICASO4_DISPLAY_H__
#include <ch.h>
#include <hal.h>




typedef struct oledConfig  oledConfig;
enum OledConfig_Device {PICASO, TERM_VT100};
#define COLOR_TABLE_SIZE 11

void oledInit (oledConfig *oledConfig,  struct SerialDriver *oled, const uint32_t baud,
	       GPIO_TypeDef *rstGpio, uint32_t rstPin, enum OledConfig_Device dev);
void oledHardReset (oledConfig *oledConfig);

void oledAcquireLock (oledConfig *oledConfig);
void oledReleaseLock (oledConfig *oledConfig);

// replace escape color sequence by color command for respective backend
// ESC c 0 à 9 : couleur index of background and foreground
// replace escape n by carriage return, line feed
void oledPrintFmt (oledConfig *oledConfig, const char *txt, ...);
void oledPrintBuffer (oledConfig *oledConfig, const char *buffer);
void oledPrintVersion (oledConfig *oledConfig);
void oledChangeBgColor (oledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b);
void oledSetTextFgColor (oledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b);
void oledSetTextBgColor (oledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b);
void oledSetTextBgColorTable (oledConfig *oledConfig, uint8_t index, 
			      uint8_t r, uint8_t g, uint8_t b);
void oledSetTextFgColorTable (oledConfig *oledConfig,  uint8_t index, 
			      uint8_t r, uint8_t g, uint8_t b);
void oledUseColorIndex (oledConfig *oledConfig, uint8_t index);
void oledGotoXY (oledConfig *oledConfig, uint8_t x, uint8_t y);
void oledGotoNextLine (oledConfig *oledConfig);
void oledClearScreen (oledConfig *oledConfig);
bool_t oledInitSdCard (oledConfig *oledConfig);
void oledDrawPoint (oledConfig *oledConfig, const uint16_t x, 
		    const uint16_t y, const uint8_t index);
void oledDrawLine (oledConfig *oledConfig, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2, 
		   const uint8_t index);
void oledDrawRect (oledConfig *oledConfig, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2, 
		   const bool_t filled, const uint8_t index);
void oledEnableTouch (oledConfig *oledConfig, bool_t enable);
uint16_t oledTouchGetStatus (oledConfig *oledConfig);
uint16_t oledTouchGetXcoord (oledConfig *oledConfig);
uint16_t oledTouchGetYcoord (oledConfig *oledConfig);
void oledListSdCardDirectory (oledConfig *oledConfig);
void oledSetSoundVolume (oledConfig *oledConfig, uint8_t percent);
void oledPlayWav (oledConfig *oledConfig, const char* fileName);
uint32_t oledOpenFile  (oledConfig *oledConfig, const char* fileName, uint16_t *handle);
void oledCloseFile (oledConfig *oledConfig, const uint16_t handle);
void oledDisplayGci  (oledConfig *oledConfig, const uint16_t handle, uint32_t offset);
uint16_t getResponseAsUint16 (oledConfig *oledConfig);

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



struct oledConfig {
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






#endif //__PICASO4_DISPLAY_H__
