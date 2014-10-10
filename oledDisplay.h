#ifndef __OLED_DISPLAY_H__
#define __OLED_DISPLAY_H__
#include <ch.h>
#include <hal.h>


typedef struct oledConfig  oledConfig;
#define COLOR_TABLE_SIZE 8

void oledInit (oledConfig *oledConfig,  struct SerialDriver *oled, const uint32_t baud,
	       GPIO_TypeDef *rstGpio, uint32_t rstPin);
void oledHardReset (oledConfig *oledConfig);

void oledAcquireLock (oledConfig *oledConfig);
void oledReleaseLock (oledConfig *oledConfig);
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
void oledGotoX (oledConfig *oledConfig, uint8_t x);
void oledGotoNextLine (oledConfig *oledConfig);
void oledClearScreen (oledConfig *oledConfig);
void oledInitSdCard (oledConfig *oledConfig);
void oledListSdCardDirectory (oledConfig *oledConfig);
void oledSound (oledConfig *oledConfig, uint16_t freq, uint16_t duration);
void oledSetSoundVolume (oledConfig *oledConfig, uint8_t percent);
void oledPlayWav (oledConfig *oledConfig, const char* fileName);
void oledDisplayCgi  (oledConfig *oledConfig, const char* fileName, uint32_t offset);



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
  // =============
  uint16_t bg;
  uint16_t tbg[COLOR_TABLE_SIZE]; 
  uint16_t fg[COLOR_TABLE_SIZE]; 
  uint8_t tbgIdx; 
  uint8_t fgIdx; 
  uint8_t curXpos; 
  uint8_t curYpos;   
  uint8_t response[16];
};







#endif //__OLED_DISPLAY_H__
