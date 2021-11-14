#pragma once
#include <ch.h>
#include <hal.h>
    

bool txt_moveCursor(OledConfig *oledConfig, uint16_t line, uint16_t column);

bool txt_putCh(OledConfig *oledConfig, uint16_t car);

// gold:strlen<=255,picaso+diablo:strlen<=511
bool txt_putStr(OledConfig *oledConfig, const char* str0);

bool txt_charWidth(OledConfig *oledConfig, char car, uint16_t *width);

bool txt_charHeight(OledConfig *oledConfig, char car, uint16_t *height);

bool txt_Fgcolour(OledConfig *oledConfig, uint16_t color, uint16_t *oldCol);

bool txt_Bgcolour(OledConfig *oledConfig, uint16_t color, uint16_t *oldCol);

bool txt_fontID(OledConfig *oledConfig, uint16_t id);

bool txt_widthMult(OledConfig *oledConfig, uint16_t wMultiplier, uint16_t *oldMul);

bool txt_heightMult(OledConfig *oledConfig, uint16_t hMultiplier, uint16_t *oldMul);

// 00..32
bool txt_xgap(OledConfig *oledConfig, uint16_t xGap, uint16_t *oldGap);

// 00..32
bool txt_ygap(OledConfig *oledConfig, uint16_t yGap, uint16_t *oldGap);

// 0..1
bool txt_bold(OledConfig *oledConfig, uint16_t mode, uint16_t *oldBold);

// 0..1
bool txt_inverse(OledConfig *oledConfig, uint16_t mode, uint16_t *oldInv);

// 0..1
bool txt_italic(OledConfig *oledConfig, uint16_t mode, uint16_t *oldItal);

// 0..1
bool txt_opacity(OledConfig *oledConfig, uint16_t mode, uint16_t *oldOpa);

// 0..1
bool txt_underline(OledConfig *oledConfig, uint16_t mode, uint16_t *oldUnder);

// bold:1<<4,italic:1<<5,inverse:1<<6,underlined1<<7
bool txt_attributes(OledConfig *oledConfig, uint16_t bitfield, uint16_t *oldAttr);

// function=[7:textprintdelay],delai=[0..255ms]
bool txt_set(OledConfig *oledConfig, uint16_t function, uint16_t value);

// 0:off,1..N:wrapposition
bool txt_wrap(OledConfig *oledConfig, uint16_t xpos);

bool gfx_cls(OledConfig *oledConfig);

bool gfx_changeColour(OledConfig *oledConfig, uint16_t oldColor, uint16_t newColor);

bool gfx_circle(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t radius, uint16_t color);

bool gfx_circleFilled(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t radius, uint16_t color);

bool gfx_line(OledConfig *oledConfig, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);

bool gfx_rectangle(OledConfig *oledConfig, uint16_t tlx, uint16_t tly, uint16_t brx, uint16_t bry, uint16_t color);

bool gfx_rectangleFilled(OledConfig *oledConfig, uint16_t tlx, uint16_t tly, uint16_t brx, uint16_t bry, uint16_t color);

bool gfx_polyline(OledConfig *oledConfig, uint16_t n, uint16_t vx[], uint16_t vy[], uint16_t color);

bool gfx_polygon(OledConfig *oledConfig, uint16_t n, uint16_t vx[], uint16_t vy[], uint16_t color);

bool gfx_polygonFilled(OledConfig *oledConfig, uint16_t n, uint16_t vx[], uint16_t vy[], uint16_t color);

bool gfx_triangle(OledConfig *oledConfig, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color);

bool gfx_triangleFilled(OledConfig *oledConfig, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color);

bool gfx_orbit(OledConfig *oledConfig, uint16_t angle, uint16_t distance, uint16_t *Xdist, uint16_t *Ydist);

bool gfx_putPixel(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t color);

bool gfx_getPixel(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t *color);

bool gfx_moveTo(OledConfig *oledConfig, uint16_t x, uint16_t y);

bool gfx_lineTo(OledConfig *oledConfig, uint16_t x, uint16_t y);

// 0..1
bool gfx_clipping(OledConfig *oledConfig, uint16_t mode);

bool gfx_clipWindow(OledConfig *oledConfig, uint16_t tlx, uint16_t tly, uint16_t brx, uint16_t bry);

bool gfx_setClipRegion(OledConfig *oledConfig);

bool gfx_ellipse(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t xrad, uint16_t yrad, uint16_t color);

bool gfx_ellipseFilled(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t xrad, uint16_t yrad, uint16_t color);

bool gfx_button(OledConfig *oledConfig, uint16_t state, uint16_t x, uint16_t y,
uint16_t buttoncolor, uint16_t txtcolor, uint16_t font,
uint16_t twtWidth, uint16_t twtHeight, char* cstr);
;

bool gfx_panel(OledConfig *oledConfig, uint16_t state, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

bool gfx_slider(OledConfig *oledConfig, uint16_t mode, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color, uint16_t scale, uint16_t value);

bool gfx_screenCopyPaste(OledConfig *oledConfig, uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t width, uint16_t height);

// 0..4
bool gfx_bevelShadow(OledConfig *oledConfig, uint16_t value, uint16_t *status);

// 0..15
bool gfx_bevelWidth(OledConfig *oledConfig, uint16_t value, uint16_t *status);

bool gfx_bGcolour(OledConfig *oledConfig, uint16_t color);

bool gfx_outlineColour(OledConfig *oledConfig, uint16_t color);

// 0..15
bool gfx_contrast(OledConfig *oledConfig, uint16_t contrast);

bool gfx_frameDelay(OledConfig *oledConfig, uint16_t delayMsec);

bool gfx_linePattern(OledConfig *oledConfig, uint16_t pattern);

// 0=LANDSCAPE,1=LANDSCAPEREVERSE,2=portrait,3=portrait_reverse
bool gfx_screenMode(OledConfig *oledConfig, uint16_t mode, uint16_t *oldMode);

// 0..1
bool gfx_transparency(OledConfig *oledConfig, uint16_t mode, uint16_t *previous);

bool gfx_transparentColour(OledConfig *oledConfig, uint16_t color, uint16_t *previous);

// seemanual
bool gfx_set(OledConfig *oledConfig, uint16_t function, uint16_t mode);

bool gfx_get(OledConfig *oledConfig, uint16_t mode, uint16_t *value);

bool media_init(OledConfig *oledConfig, uint16_t *value);

bool media_setAdd(OledConfig *oledConfig, uint16_t hiAddr, uint16_t loAddr);

bool media_setSector(OledConfig *oledConfig, uint16_t hiAddr, uint16_t loAddr);

bool media_readSector(OledConfig *oledConfig, uint8_t *sector);

bool media_writeSector(OledConfig *oledConfig, uint8_t sector);

bool media_readByte(OledConfig *oledConfig, uint16_t *value);

bool media_readWord(OledConfig *oledConfig, uint16_t *value);

bool media_writeByte(OledConfig *oledConfig, uint16_t value, uint16_t *status);

bool media_writeWord(OledConfig *oledConfig, uint16_t value, uint16_t *status);

bool media_flush(OledConfig *oledConfig, uint16_t *status);

bool media_image(OledConfig *oledConfig, uint16_t x, uint16_t y);

bool media_video(OledConfig *oledConfig, uint16_t x, uint16_t y);

bool media_videoFrame(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t frameNumber);

bool misc_peekB(OledConfig *oledConfig, uint16_t eveRegIndex, uint16_t *value);

bool misc_pokeB(OledConfig *oledConfig, uint16_t eveRegIndex, uint16_t value);

bool misc_peekW(OledConfig *oledConfig, uint16_t eveRegIndex, uint16_t *value);

bool misc_pokeW(OledConfig *oledConfig, uint16_t eveRegIndex, uint16_t value);

bool misc_peekM(OledConfig *oledConfig, uint16_t address, uint16_t *value);

bool misc_pokeM(OledConfig *oledConfig, uint16_t address, uint16_t value);

bool misc_joystick(OledConfig *oledConfig, uint16_t *value);

// note=[0..64]
bool misc_beep(OledConfig *oledConfig, uint16_t note, uint16_t duration_ms);

// Theresponsewillbe0x06atthenewbaudrateset,100msafterthecommandissent
bool misc_setbaudWait(OledConfig *oledConfig, uint16_t index);

bool misc_blitComtoDisplay(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t data);

bool sys_sleep(OledConfig *oledConfig, uint16_t duration_s, uint16_t *duration);

bool sys_memFree(OledConfig *oledConfig, uint16_t handle, uint16_t *value);

bool sys_memHeap(OledConfig *oledConfig, uint16_t *avail);

bool sys_getModel(OledConfig *oledConfig, uint16_t n, char *str);

bool sys_getVersion(OledConfig *oledConfig, uint16_t *version);

bool sys_getPmmC(OledConfig *oledConfig, uint16_t *version);

// 1..65535
bool misc_screenSaverTimeout(OledConfig *oledConfig, uint16_t timout_ms);

// seereferencemanuel
bool misc_screenSaverSpeed(OledConfig *oledConfig, uint16_t speed_index);

// seereferencemanuel
bool misc_screenSaverMode(OledConfig *oledConfig, uint16_t mode);

bool touch_detectRegion(OledConfig *oledConfig, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

// 0..2
bool touch_set(OledConfig *oledConfig, uint16_t mode);

// 0..2
bool touch_get(OledConfig *oledConfig, uint16_t mode, uint16_t *value);
