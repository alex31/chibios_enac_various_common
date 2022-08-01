#pragma once
#include "ch.h"


#ifdef __cplusplus
extern "C" {
#endif

  



#include "ssd1306_fonts.h"
#include "ssd1306_conf.h"

/***
 *      _____
 *     |_   _|  _ _ __  ___ ___
 *       | || || | '_ \/ -_|_-<
 *       |_| \_, | .__/\___/__/
 *           |__/|_|
 */
// screen colors
typedef enum {
    BLACK = 0x00, // Black color, no pixel
    WHITE = 0x01  // Pixel is set. Color depends on OLED
} SSD1306Color;

// Struct to store pixel coordinates
typedef struct {
    uint16_t x;
    uint16_t y;
} SSD1306Pixel;

/***
 *      ___             _   _
 *     | __|  _ _ _  __| |_(_)___ _ _  ___
 *     | _| || | ' \/ _|  _| / _ \ ' \(_-<
 *     |_| \_,_|_||_\__|\__|_\___/_||_/__/
 *
 */
/***
 *     +-+-+-+-+-+
 *     |U|t|i|l|s|
 *     +-+-+-+-+-+
 */
void ssd1306_Init(void);
void ssd1306_SetDisplayOn(void);
void ssd1306_SetDisplayOff(void);
void ssd1306_UpdateScreen(void); // MAJ ecran avec buffer ecran
void ssd1306_MoveCursor(uint8_t x, uint8_t y);
void ssd1306_Fill(SSD1306Color color);
void ssd1306_SetContrast(const uint8_t value); // Contrast increases as value increases. (RESET = 7Fh)

/***
 *     +-+-+-+-+
 *     |T|e|x|t|
 *     +-+-+-+-+
 */
void ssd1306_WriteChar(uint8_t character, const GFXfont font, SSD1306Color color);
void ssd1306_WriteString(const char* str, const GFXfont font, SSD1306Color color);
void ssd1306_WriteFmt(const GFXfont font, SSD1306Color color, const char *fmt, ...);

/***
 *     +-+-+-+-+-+-+-+-+
 *     |G|r|a|p|h|i|c|s|
 *     +-+-+-+-+-+-+-+-+
 */
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306Color color);
void ssd1306_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306Color color);
void ssd1306_DrawArc(uint8_t centerX, uint8_t centerY, uint8_t radius,
		     uint16_t startAngle, uint16_t sweepAngle, SSD1306Color color);
void ssd1306_DrawCircle(uint8_t centerX, uint8_t centerY, uint8_t radius, SSD1306Color color);
void ssd1306_DrawPolyline(const SSD1306Pixel *vertexTable, uint16_t size, SSD1306Color color);
void ssd1306_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306Color color);
void ssd1306_DrawBitmap(uint8_t x, uint8_t y, const unsigned char* bitmap, uint8_t w, uint8_t h, SSD1306Color color);

#ifdef __cplusplus
}
#endif
