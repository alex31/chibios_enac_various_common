#include <ch.h>
#include <hal.h>
#include "stdutil.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>  // For memcpy
#include "printf.h"
#include "ssd1306.h"

/***
 *      __  __
 *     |  \/  |__ _ __ _ _ ___ ___
 *     | |\/| / _` / _| '_/ _ (_-<
 *     |_|  |_\__,_\__|_| \___/__/
 *
 */
#define SCREEN_BUFFER_SIZE   			(SSD1306_WIDTH * SSD1306_HEIGHT / 8)
#define CIRCLE_APPROXIMATION_SEGMENTS	36

/***
 *     +-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+
 *     |S|S|D|1|3|0|6| |O|p|.|C|o|d|e|s|
 *     +-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+
 */
// Fundamental Command Table
#define CMD_CONTRAST_CONTROL				0x81
#define CMD_DISPLAY_FOLLOW_RAM				0xA4
#define CMD_DISPLAY_IGNORE_RAM_ALL_ON		0xA5
#define CMD_NORMAL_DISPLAY					0xA6
#define CMD_INVERSE_DISPLAY					0xA7
#define CMD_DISPLAY_ON						0xAF
#define CMD_DISPLAY_OFF						0xAE

//Scrolling Command Table

// Addressing Setting Command Table
#define CMD_MEMORY_ADDRESSING_MODE			0x20
	#define DATA_MEMORY_ADDRESSING_MODE_HORIZ	0x00
	#define DATA_MEMORY_ADDRESSING_MODE_VERT	0x01
	#define DATA_MEMORY_ADDRESSING_MODE_PAGE	0x02
#define CMD_PAGE_START_ADDRESS_0			0xB0

//Hardware Configuration Command Table
#define CMD_DISPLAY_START_LINE_0			0x40
#define CMD_SEGMENT_REMAP_COL0_SEG0			0xA0
#define CMD_SEGMENT_REMAP_COL127_SEG0		0xA1
#define CMD_MULTIPLEX_RATIO					0xA8
#define CMD_COM_SCAN_DIRECTION_NORMAL		0xC0
#define CMD_COM_SCAN_DIRECTION_REMAPPED		0xC8
#define CMD_DISPLAY_OFFSET					0xD3
#define CMD_COM_PIN_CONFIG					0xDA

// Timing & Driving Scheme Setting Command Table
#define CMD_DISPLAY_CLOCK_RATIO_FREQ		0xD5
#define CMD_PRE_CHARGE_PERIOD				0xD9
#define CMD_VCOMH_DESELECT_LEVEL			0xDB

// Charge Pump Command Table
#define CMD_CHARGE_PUMP_SETTING				0x8D
	#define DATA_CHARGE_PUMP_ENABLE				0x14
	#define DATA_CHARGE_PUMP_DISABLE			0x10

/***
 *     __   __        _      _    _           ___ _     _          _
 *     \ \ / /_ _ _ _(_)__ _| |__| |___ ___  / __| |___| |__  __ _| |___ ___
 *      \ V / _` | '_| / _` | '_ \ / -_|_-< | (_ | / _ \ '_ \/ _` | / -_|_-<
 *       \_/\__,_|_| |_\__,_|_.__/_\___/__/  \___|_\___/_.__/\__,_|_\___/__/
 *
 */
static uint8_t ScreenBuffer[SCREEN_BUFFER_SIZE]; // Screenbuffer
static SSD1306Pixel cursor; // coordonnees x & y en pixel du curseur de texte
MUTEX_DECL(i2cMtx);

/***
 *      ___     _          _
 *     | _ \_ _(_)_ ____ _| |_ ___
 *     |  _/ '_| \ V / _` |  _/ -_)
 *     |_| |_| |_|\_/\__,_|\__\___|
 *
 */
// Send buffer to SSD1306 over I2C
void ssd1306_SendI2c(const uint8_t *buffer, size_t buff_size)
{
	chMtxLock(&i2cMtx);

	msg_t status = i2cMasterTransmitTimeout(&SSD1306_I2CD, SSD1306_I2C_ADDR, buffer, buff_size, NULL, 0, 100);
	if (status != MSG_OK) {DebugTrace("SSD1306 I2C write error %ld", status);}

	chMtxUnlock(&i2cMtx);
}

// Send a byte to the command register
void ssd1306_WriteCommand(uint8_t byte)
{
	const uint8_t writeBuffer[2] = {0x00, byte};

	ssd1306_SendI2c(writeBuffer, sizeof(writeBuffer));
}

// Convert Degrees to Radians
static float ssd1306_DegToRad(float angle)
{
	return angle * 3.14f / 180.0f;
}

// Normalize degree to [0;360]
static uint16_t ssd1306_NormalizeTo0_360(uint16_t angle)
{
	uint16_t locAngle;
	if(angle <= 360) locAngle = angle;
	else
	{
		locAngle = angle % 360;
		locAngle = ((angle != 0) ? angle : 360);
	}
	return locAngle;
}

/***
 *      ___      _
 *     |   \ _ _(_)_ _____ _ _
 *     | |) | '_| \ V / -_) '_|
 *     |___/|_| |_|\_/\___|_|
 *
 */
/***
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |I|n|i|t|i|a|l|i|s|a|t|i|o|n|
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
void ssd1306_Init(void) {

    // Wait for the screen to boot
    chThdSleepMilliseconds(100);

	//=== Set multiplex ratio ===
	#if (SSD1306_HEIGHT == 128)
		ssd1306_WriteCommand(0xFF); // Found in the Luma Python lib for SH1106.
	#else
		ssd1306_WriteCommand(CMD_MULTIPLEX_RATIO); // multiplex ratio(15 to 64)
	#endif
	#if (SSD1306_HEIGHT == 32)
		ssd1306_WriteCommand(0x1F); // multiplex ratio(15 to 64)
	#elif (SSD1306_HEIGHT == 64)
		ssd1306_WriteCommand(0x3F); // multiplex ratio(15 to 64)
	#elif (SSD1306_HEIGHT == 128)
		ssd1306_WriteCommand(0x3F); // multiplex ratio(15 to 64), Seems to work for 128px high displays too.
	#else
	#error "Only 32, 64, or 128 lines of height are supported!"
	#endif

	//=== Set Display offset ===
	ssd1306_WriteCommand(CMD_DISPLAY_OFFSET);
	ssd1306_WriteCommand(0x00); // no offset

	//=== Set Display Start Line ===
    ssd1306_WriteCommand(CMD_DISPLAY_START_LINE_0); // RAM Display start line (0-63)

    //=== Set Segment re-map ===
#ifdef SSD1306_MIRROR_VERT
    ssd1306_WriteCommand(CMD_SEGMENT_REMAP_COL0_SEG0); // Mirror horizontally
#else
    ssd1306_WriteCommand(CMD_SEGMENT_REMAP_COL127_SEG0); //
#endif

    //=== Set COM Output Scan Direction ===
#ifdef SSD1306_MIRROR_HORIZ
    ssd1306_WriteCommand(CMD_COM_SCAN_DIRECTION_NORMAL); // Mirror vertically
#else
    ssd1306_WriteCommand(CMD_COM_SCAN_DIRECTION_REMAPPED); //Set COM Output Scan Direction
#endif

    //=== Set COM Pins hardware configuration
    ssd1306_WriteCommand(CMD_COM_PIN_CONFIG);
#if (SSD1306_HEIGHT == 32)
    ssd1306_WriteCommand(0x02);
#elif (SSD1306_HEIGHT == 64)
    ssd1306_WriteCommand(0x12);
#elif (SSD1306_HEIGHT == 128)
    ssd1306_WriteCommand(0x12);
#else
#error "Only 32, 64, or 128 lines of height are supported!"
#endif

    //=== Set Memory Addressing Mode ===
    ssd1306_WriteCommand(CMD_MEMORY_ADDRESSING_MODE); //
    ssd1306_WriteCommand(DATA_MEMORY_ADDRESSING_MODE_HORIZ);
    ssd1306_WriteCommand(CMD_PAGE_START_ADDRESS_0); //Set Page Start Address for Page Addressing Mode,0-7

    //=== Set Contrast Control ===
    ssd1306_SetContrast(0xFF);

    //=== Disable Entire Display On ===
    ssd1306_WriteCommand(CMD_DISPLAY_FOLLOW_RAM); //Output follows RAM content

    // === Set Normal/Inverse Color Display ===
#ifdef SSD1306_INVERSE_COLOR
    ssd1306_WriteCommand(CMD_INVERSE_DISPLAY);
#else
    ssd1306_WriteCommand(CMD_NORMAL_DISPLAY);
#endif

    //=== Set Osc Frequency ===
    ssd1306_WriteCommand(CMD_DISPLAY_CLOCK_RATIO_FREQ); // set clock divide ratio/oscillator frequency
    ssd1306_WriteCommand(0xF0); // divide ratio

    //=== Set pre-charge period ===
    ssd1306_WriteCommand(CMD_PRE_CHARGE_PERIOD);
    ssd1306_WriteCommand(0x22);

    //=== Set VCOMH Deselect Level ===
    ssd1306_WriteCommand(CMD_VCOMH_DESELECT_LEVEL); //--set vcomh
    ssd1306_WriteCommand(0x20); //0x20,0.77xVcc

    //=== Enable charge pump regulator ===
    // Charge Pump Enable sequence
    ssd1306_WriteCommand(CMD_CHARGE_PUMP_SETTING);
    ssd1306_WriteCommand(DATA_CHARGE_PUMP_ENABLE);
    ssd1306_SetDisplayOn();


    //=== Set Higher/Lower Column Start Address for Page Addressing Mode ===
    ssd1306_WriteCommand(0x00); // Lower Column Start Address 0
    ssd1306_WriteCommand(0x10); // Higher Column Start Address 0

    //=== Cleanup Screen ===
    ssd1306_Fill(BLACK); // Clear screen
    ssd1306_UpdateScreen(); // Flush buffer to screen

    // raz cordonnees curseur de texte
    cursor.x = 0;
    cursor.y = 0;
}


/***
 *     +-+-+-+-+-+-+-+-+-+-+-+ +-+-+-+-+-+
 *     |R|e|m|p|l|i|s|s|a|g|e| |e|c|r|a|n|
 *     +-+-+-+-+-+-+-+-+-+-+-+ +-+-+-+-+-+
 */
void ssd1306_Fill(SSD1306Color color)
{
	for(uint32_t i = 0; i < sizeof(ScreenBuffer); i++)
	{
		ScreenBuffer[i] = (color == BLACK) ? 0x00 : 0xFF;
	}
}

/***
 *     +-+-+-+ +-+-+-+-+-+-+-+-+-+
 *     |M|A|J| |a|f|f|i|c|h|a|g|e|
 *     +-+-+-+ +-+-+-+-+-+-+-+-+-+
 */
// Update display with screenbuffer content
void ssd1306_UpdateScreen(void) {


	// Write data to each page of RAM. Number of pages depends on the screen height:
    // 32px = 4 pages, 64px = 8 pages, 128px = 16 pages
	uint8_t writeBuffer[SSD1306_WIDTH + 1];
	writeBuffer[0] = 0x40;

    for(uint8_t pageIndex = 0; pageIndex < (SSD1306_HEIGHT / 8); pageIndex++)
    {
        ssd1306_WriteCommand(0xB0 + pageIndex); // Set the current RAM page address.
        ssd1306_WriteCommand(0x00);
        ssd1306_WriteCommand(0x10);
		memcpy(writeBuffer + 1, &ScreenBuffer[SSD1306_WIDTH * pageIndex], SSD1306_WIDTH);

		ssd1306_SendI2c(writeBuffer, sizeof(writeBuffer));
    }

}

/***
 *     +-+-+-+-+-+-+ +-+-+-+-+-+
 *     |D|e|s|s|i|n| |p|i|x|e|l|
 *     +-+-+-+-+-+-+ +-+-+-+-+-+
 */
// Draw one pixel in the screenbuffer at x,y coordinate using color
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306Color color) {

	if(x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return; // do nothing if outside buffer
   
    // Draw pixel
    if(color == WHITE)
    {
        ScreenBuffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    }
    else
    {
        ScreenBuffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

/***
 *     +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+-+
 *     |E|c|r|i|t|u|r|e| |c|a|r|a|c|t|e|r|e|
 *     +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+-+
 */
// ecrit un caractere dans le buffer d'ecran a la position courante du curseur
void ssd1306_WriteChar(uint8_t character, const GFXfont font, SSD1306Color color)
{
	(void)color;

	// ignore silencieusement les caracteres hors de la fonte
	if ((character < font.first) || (character > font.last)) return;

	// calcul offset par rapport au premier caractere de la fonte
	const uint16_t charOffset = character - font.first;
	const uint16_t bitMapOffset = font.glyph[charOffset].bitmapOffset;
//	DebugTrace ("bitMapOffset = %d", bitMapOffset);

	const uint16_t charWidth = font.glyph[charOffset].width;
//	DebugTrace ("charWidth = %d", charWidth);
	const uint16_t charHeight = font.glyph[charOffset].height;
//	DebugTrace ("charHeight = %d", charHeight);

	const int8_t xOffset = font.glyph[charOffset].xOffset;
//	DebugTrace ("xOffset = %d", xOffset);
	const int8_t yOffset = font.glyph[charOffset].yOffset;
//	DebugTrace ("yOffset = %d", yOffset);

	SSD1306Pixel localCursor = {cursor.x + xOffset , cursor.y + yOffset};
//	DebugTrace ("cursor : x=%d y=%d", localCursor.x, localCursor.y);

	for(uint16_t y = 0 ; y < charHeight ; y++) // boucle ligne
	{
		for(uint16_t x = 0; x < charWidth ; x++) // boucle colonne
		{
			const uint32_t pixelIndex = x + (charWidth * y); // calcul de l'index de balayage du bitmap
			const uint32_t byteIndex = (pixelIndex / 8) + bitMapOffset; // calcul de l'index de l'octet du bitmap a analyser
			const uint8_t bitMask = 0x80 >> (pixelIndex & 0x07); // masque du bit a recuperer dans l'octet du bitmap
			const bool pixel = font.bitmap[byteIndex] & bitMask; // recuperation de la valeur du bit/pixel a afficher
			//ssd1306_DrawPixel(localCursor.x + x, localCursor.y + y, (SSD1306Color)pixel); // affichage du pixel
			if (pixel)
			{
				ssd1306_DrawPixel(localCursor.x + x, localCursor.y + y, (SSD1306Color)color);
			}
			else
			{
				ssd1306_DrawPixel(localCursor.x + x, localCursor.y + y, (SSD1306Color)!color);
			}
		}
	}
	cursor.x += font.glyph[charOffset].xAdvance;
}

/***
 *     +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+
 *     |E|c|r|i|t|u|r|e| |c|h|a|i|n|e|
 *     +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+
 */
// Write full string to screenbuffer
void ssd1306_WriteString(const char* str, const GFXfont font, SSD1306Color color) {

    while (*str) // Write until null-byte
    {
        ssd1306_WriteChar(*str, font, color);
        str++; // Next char
    }
}

/***
 *     +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+
 *     |E|c|r|i|t|u|r|e| |c|h|a|i|n|e|
 *     +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+
 */
// Write full string with printf formating to screenbuffer
void ssd1306_WriteFmt(const GFXfont font, SSD1306Color color, const char *fmt, ...)
{
   char buffer[40];
   va_list ap;
   va_start(ap, fmt);
   chvsnprintf(buffer, sizeof(buffer), fmt, ap);
   va_end(ap);
   ssd1306_WriteString(buffer, font, color);
}
/***
 *     +-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+
 *     |D|e|p|l|a|c|e| |C|u|r|s|e|u|r|
 *     +-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+
 */
// Move cursor
void ssd1306_MoveCursor(uint8_t x, uint8_t y)
{
  cursor.x = x;
  cursor.y = y;
}

/***
 *     +-+-+-+-+-+-+ +-+-+-+-+-+
 *     |D|e|s|s|i|n| |L|i|g|n|e|
 *     +-+-+-+-+-+-+ +-+-+-+-+-+
 */
// Draw line by Bresenhem's algorithm
void ssd1306_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306Color color) {

	const int32_t deltaX = abs(x2 - x1);
	const int32_t deltaY = abs(y2 - y1);
	const int32_t signX = ((x1 < x2) ? 1 : -1);
	const int32_t signY = ((y1 < y2) ? 1 : -1);
	int32_t error = deltaX - deltaY;
	int32_t error2;

	ssd1306_DrawPixel(x2, y2, color);
	while((x1 != x2) || (y1 != y2))
	{
		ssd1306_DrawPixel(x1, y1, color);
		error2 = error * 2;
		if(error2 > -deltaY)
		{
			error -= deltaY;
			x1 += signX;
		}
		// else nothing to do

		if(error2 < deltaX)
		{
			error += deltaX;
			y1 += signY;
		}
		// else nothing to do

	}
}

/***
 *     +-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+-+
 *     |D|e|s|s|i|n| |P|o|l|y|l|i|g|n|e|
 *     +-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+-+
 */
void ssd1306_DrawPolyline(const SSD1306Pixel *vertexTable, uint16_t size, SSD1306Color color)
{
	for(uint16_t index = 1; index < size; index++)
	{
		ssd1306_DrawLine(vertexTable[index - 1].x, vertexTable[index - 1].y, vertexTable[index].x, vertexTable[index].y, color);
	}
}

/***
 *     +-+-+-+-+-+-+ +-+-+-+
 *     |D|e|s|s|i|n| |A|r|c|
 *     +-+-+-+-+-+-+ +-+-+-+
 */
// Draw angle beginning from 4 quart of trigonometric circle (3pi/2)
// startAngle & sweepAngle in degree
void ssd1306_DrawArc(uint8_t centerX, uint8_t centerY, uint8_t radius, uint16_t startAngle, uint16_t sweepAngle, SSD1306Color color) {

    const uint32_t locSweep = ssd1306_NormalizeTo0_360(sweepAngle);
    const uint32_t approxSegments = (locSweep * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    const float approxDegree = locSweep / (float)approxSegments;
    uint32_t count = (ssd1306_NormalizeTo0_360(startAngle) * CIRCLE_APPROXIMATION_SEGMENTS) / 360;
    float rad;
	uint8_t xp1, xp2, yp1, yp2;

    while(count < approxSegments)
    {
        rad = ssd1306_DegToRad(count * approxDegree);
        xp1 = centerX + (int8_t)(sin(rad) * radius);
        yp1 = centerY + (int8_t)(cos(rad) * radius);
        count++;
        if(count != approxSegments)
        {
            rad = ssd1306_DegToRad(count * approxDegree);
        }
        else
        {            
            rad = ssd1306_DegToRad(locSweep);
        }
        xp2 = centerX + (int8_t)(sin(rad) * radius);
        yp2 = centerY + (int8_t)(cos(rad) * radius);
        ssd1306_DrawLine(xp1, yp1, xp2, yp2, color);
    }
}

/***
 *     +-+-+-+-+-+-+ +-+-+-+-+-+-+
 *     |D|e|s|s|i|n| |C|e|r|c|l|e|
 *     +-+-+-+-+-+-+ +-+-+-+-+-+-+
 */
// Draw circle by Bresenhem's algorithm
void ssd1306_DrawCircle(uint8_t centerX, uint8_t centerY, uint8_t radius, SSD1306Color color) {
	int32_t currentX = -radius;
	int32_t currentY = 0;
	int32_t err = 2 - 2 * radius;
	int32_t err2;

	if (centerX >= SSD1306_WIDTH || centerY >= SSD1306_HEIGHT) return;

    do
    {
		ssd1306_DrawPixel(centerX - currentX, centerY + currentY, color);
		ssd1306_DrawPixel(centerX + currentX, centerY + currentY, color);
		ssd1306_DrawPixel(centerX + currentX, centerY - currentY, color);
		ssd1306_DrawPixel(centerX - currentX, centerY - currentY, color);
		err2 = err;
        if (err2 <= currentY)
        {
            currentY++;
            err = err + (currentY * 2 + 1);
            if(-currentX == currentY && err2 <= currentX)
            {
            	err2 = 0;
            }
        }

        if(err2 > currentX)
        {
        	currentX++;
        	err = err + (currentX * 2 + 1);
        }

    }
    while(currentX <= 0);
}

/***
 *     +-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+-+
 *     |D|e|s|s|i|n| |R|e|c|t|a|n|g|l|e|
 *     +-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+-+
 */
//Draw rectangle
void ssd1306_DrawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306Color color) {
  ssd1306_DrawLine(x1,y1,x2,y1,color);
  ssd1306_DrawLine(x2,y1,x2,y2,color);
  ssd1306_DrawLine(x2,y2,x1,y2,color);
  ssd1306_DrawLine(x1,y2,x1,y1,color);
}

/***
 *     +-+-+-+-+-+-+ +-+-+-+-+-+-+
 *     |D|e|s|s|i|n| |B|i|t|m|a|p|
 *     +-+-+-+-+-+-+ +-+-+-+-+-+-+
 */
//Draw bitmap - ported from the ADAFruit GFX library
void ssd1306_DrawBitmap(uint8_t x, uint8_t y, const unsigned char* bitmap, uint8_t w, uint8_t h, SSD1306Color color)
{
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;

    for (uint8_t j = 0; j < h; j++, y++) {
        for (uint8_t i = 0; i < w; i++) {
            if (i & 7)
                byte <<= 1;
            else
                byte = (*(const unsigned char *)(&bitmap[j * byteWidth + i / 8]));
            if (byte & 0x80)
                ssd1306_DrawPixel(x + i, y, color);
        }
    }
}

/***
 *     +-+-+-+-+-+-+-+-+-+
 *     |C|o|n|t|r|a|s|t|e|
 *     +-+-+-+-+-+-+-+-+-+
 */
void ssd1306_SetContrast(const uint8_t value) {
    ssd1306_WriteCommand(CMD_CONTRAST_CONTROL);
    ssd1306_WriteCommand(value);
}

/***
 *     +-+-+-+-+-+-+-+ +-+-+-+-+-+-+
 *     |D|i|s|p|l|a|y| |O|N|/|O|F|F|
 *     +-+-+-+-+-+-+-+ +-+-+-+-+-+-+
 */
void ssd1306_SetDisplayOn(void)
{
	ssd1306_WriteCommand(CMD_DISPLAY_ON);
}

void ssd1306_SetDisplayOff(void)
{
	ssd1306_WriteCommand(CMD_DISPLAY_OFF);
}


