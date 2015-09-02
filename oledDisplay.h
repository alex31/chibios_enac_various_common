#ifndef __OLED_DISPLAY_H__
#define __OLED_DISPLAY_H__
#include <ch.h>
#include <hal.h>


typedef struct OledConfig  OledConfig;
#define COLOR_TABLE_SIZE 8

void oledInit (OledConfig *oledConfig,  struct SerialDriver *oled, const uint32_t baud,
	       GPIO_TypeDef *rstGpio, uint32_t rstPin);
void oledHardReset (OledConfig *oledConfig);

void oledAcquireLock (OledConfig *oledConfig);
void oledReleaseLock (OledConfig *oledConfig);
void oledPrintFmt (OledConfig *oledConfig, const char *txt, ...);
void oledPrintBuffer (OledConfig *oledConfig, const char *buffer);
void oledPrintVersion (OledConfig *oledConfig);
void oledChangeBgColor (OledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b);
void oledSetTextFgColor (OledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b);
void oledSetTextBgColor (OledConfig *oledConfig, uint8_t r, uint8_t g, uint8_t b);
void oledSetTextBgColorTable (OledConfig *oledConfig, uint8_t index, 
			      uint8_t r, uint8_t g, uint8_t b);
void oledSetTextFgColorTable (OledConfig *oledConfig,  uint8_t index, 
			      uint8_t r, uint8_t g, uint8_t b);
void oledUseColorIndex (OledConfig *oledConfig, uint8_t index);
void oledGotoXY (OledConfig *oledConfig, uint8_t x, uint8_t y);
void oledGotoX (OledConfig *oledConfig, uint8_t x);
void oledGotoNextLine (OledConfig *oledConfig);
void oledClearScreen (OledConfig *oledConfig);
void oledInitSdCard (OledConfig *oledConfig);
void oledListSdCardDirectory (OledConfig *oledConfig);
void oledSound (OledConfig *oledConfig, uint16_t freq, uint16_t duration);
void oledSetSoundVolume (OledConfig *oledConfig, uint8_t percent);
void oledPlayWav (OledConfig *oledConfig, const char* fileName);
void oledDisplayCgi  (OledConfig *oledConfig, const char* fileName, uint32_t offset);



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
