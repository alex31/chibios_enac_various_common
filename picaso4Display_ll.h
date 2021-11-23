#pragma once
#include <ch.h>
#include <hal.h>

typedef struct OledConfig OledConfig;

// "Line : Holds a positive value for the required line position. Column :Holds
// a positive value for the required column position." The Move Cursor command
// moves the text cursor to a screen position set by line and column parameters.
// The line and column position is calculated, based on the size and scaling
// factor for the currently selected font. When text is outputted to screen it
// will be displayed from this position. The text position could also be set
// with “Move Origin” command if required to set the text position to an exact
// pixel location. Note that lines and columns start from 0, so line 0, column 0
// is the top left corner of the display.
bool txt_moveCursor(const OledConfig *oledConfig, uint16_t line,
                    uint16_t column);

bool txt_putCh(const OledConfig *oledConfig, uint16_t car);

// gold : strlen <= 255, picaso+diablo : strlen <= 511
bool txt_putStr(const OledConfig *oledConfig, const char *cstr,
                uint16_t *length);

bool txt_charWidth(const OledConfig *oledConfig, char car, uint16_t *width);

bool txt_charHeight(const OledConfig *oledConfig, char car, uint16_t *height);

bool txt_fgColour(const OledConfig *oledConfig, uint16_t colour,
                  uint16_t *oldCol);

bool txt_bgColour(const OledConfig *oledConfig, uint16_t colour,
                  uint16_t *oldCol);

bool txt_fontID(const OledConfig *oledConfig, uint16_t id, uint16_t *oldFont);

bool txt_widthMult(const OledConfig *oledConfig, uint16_t wMultiplier,
                   uint16_t *oldMul);

bool txt_heightMult(const OledConfig *oledConfig, uint16_t hMultiplier,
                    uint16_t *oldMul);

// 00 .. 32
bool txt_xgap(const OledConfig *oledConfig, uint16_t xGap, uint16_t *oldGap);

// 00 .. 32
bool txt_ygap(const OledConfig *oledConfig, uint16_t yGap, uint16_t *oldGap);

// 0 .. 1
bool txt_bold(const OledConfig *oledConfig, uint16_t mode, uint16_t *oldBold);

// 0 .. 1
bool txt_inverse(const OledConfig *oledConfig, uint16_t mode, uint16_t *oldInv);

// 0 .. 1
bool txt_italic(const OledConfig *oledConfig, uint16_t mode, uint16_t *oldItal);

// 0 .. 1
bool txt_opacity(const OledConfig *oledConfig, uint16_t mode, uint16_t *oldOpa);

// 0 .. 1
bool txt_underline(const OledConfig *oledConfig, uint16_t mode,
                   uint16_t *oldUnder);

// bold:1<<4, italic:1<<5, inverse:1<<6, underlined1<<7
bool txt_attributes(const OledConfig *oledConfig, uint16_t bitfield,
                    uint16_t *oldAttr);

// function=[7: text print delay], delai=[0 .. 255 ms]
bool txt_set(const OledConfig *oledConfig, uint16_t function, uint16_t value);

// 0:off, 1..N:wrap position
bool txt_wrap(const OledConfig *oledConfig, uint16_t xpos, uint16_t *oldWrap);

bool gfx_cls(const OledConfig *oledConfig);

bool gfx_changeColour(const OledConfig *oledConfig, uint16_t oldColor,
                      uint16_t newColor);

bool gfx_circle(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                uint16_t radius, uint16_t colour);

bool gfx_circleFilled(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                      uint16_t radius, uint16_t colour);

bool gfx_line(const OledConfig *oledConfig, uint16_t x1, uint16_t y1,
              uint16_t x2, uint16_t y2, uint16_t colour);

bool gfx_rectangle(const OledConfig *oledConfig, uint16_t tlx, uint16_t tly,
                   uint16_t brx, uint16_t bry, uint16_t colour);

bool gfx_rectangleFilled(const OledConfig *oledConfig, uint16_t tlx,
                         uint16_t tly, uint16_t brx, uint16_t bry,
                         uint16_t colour);

bool gfx_polyline(const OledConfig *oledConfig, uint16_t n, const uint16_t vx[],
                  const uint16_t vy[], uint16_t color);

bool gfx_polygon(const OledConfig *oledConfig, uint16_t n, const uint16_t vx[],
                 const uint16_t vy[], uint16_t color);

bool gfx_polygonFilled(const OledConfig *oledConfig, uint16_t n,
                       const uint16_t vx[], const uint16_t vy[],
                       uint16_t color);

bool gfx_triangle(const OledConfig *oledConfig, uint16_t x1, uint16_t y1,
                  uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
                  uint16_t colour);

bool gfx_triangleFilled(const OledConfig *oledConfig, uint16_t x1, uint16_t y1,
                        uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
                        uint16_t colour);

bool gfx_orbit(const OledConfig *oledConfig, uint16_t angle, uint16_t distance,
               uint16_t *Xdist, uint16_t *Ydist);

bool gfx_putPixel(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                  uint16_t colour);

bool gfx_getPixel(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                  uint16_t *colour);

bool gfx_moveTo(const OledConfig *oledConfig, uint16_t x, uint16_t y);

bool gfx_lineTo(const OledConfig *oledConfig, uint16_t x, uint16_t y);

// 0 .. 1
bool gfx_clipping(const OledConfig *oledConfig, uint16_t mode);

bool gfx_clipWindow(const OledConfig *oledConfig, uint16_t tlx, uint16_t tly,
                    uint16_t brx, uint16_t bry);

bool gfx_setClipRegion(const OledConfig *oledConfig);

bool gfx_ellipse(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                 uint16_t xrad, uint16_t yrad, uint16_t colour);

bool gfx_ellipseFilled(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                       uint16_t xrad, uint16_t yrad, uint16_t colour);

bool gfx_button(const OledConfig *oledConfig, uint16_t state, uint16_t x,
                uint16_t y, uint16_t buttoncolour, uint16_t txtcolour,
                uint16_t font, uint16_t twtWidth, uint16_t twtHeight,
                const char *cstr);

bool gfx_panel(const OledConfig *oledConfig, uint16_t state, uint16_t x,
               uint16_t y, uint16_t width, uint16_t height, uint16_t colour);

bool gfx_slider(const OledConfig *oledConfig, uint16_t mode, uint16_t x1,
                uint16_t y1, uint16_t x2, uint16_t y2, uint16_t colour,
                uint16_t scale, int16_t value);

bool gfx_screenCopyPaste(const OledConfig *oledConfig, uint16_t xs, uint16_t ys,
                         uint16_t xd, uint16_t yd, uint16_t width,
                         uint16_t height);

// number of pixel 0..4
bool gfx_bevelShadow(const OledConfig *oledConfig, uint16_t value,
                     uint16_t *oldBevel);

// 0..15
bool gfx_bevelWidth(const OledConfig *oledConfig, uint16_t value,
                    uint16_t *oldWidth);

bool gfx_bgCcolour(const OledConfig *oledConfig, uint16_t colour,
                   uint16_t *oldCol);

bool gfx_outlineColour(const OledConfig *oledConfig, uint16_t colour,
                       uint16_t *oldCol);

// 0..15
bool gfx_contrast(const OledConfig *oledConfig, uint16_t contrast,
                  uint16_t *oldContrast);

bool gfx_frameDelay(const OledConfig *oledConfig, uint16_t delayMsec,
                    uint16_t *oldDelay);

bool gfx_linePattern(const OledConfig *oledConfig, uint16_t pattern,
                     uint16_t *oldPattern);

// 0 = LANDSCAPE, 1 = LANDSCAPE REVERSE, 2=portrait, 3=portrait_reverse
bool gfx_screenMode(const OledConfig *oledConfig, uint16_t mode,
                    uint16_t *oldMode);

// 0..1
bool gfx_transparency(const OledConfig *oledConfig, uint16_t mode,
                      uint16_t *oldMode);

bool gfx_transparentColour(const OledConfig *oledConfig, uint16_t colour,
                           uint16_t *oldColor);

// see manual
bool gfx_set(const OledConfig *oledConfig, uint16_t function, uint16_t value);

bool gfx_get(const OledConfig *oledConfig, uint16_t mode, uint16_t *value);

bool media_init(const OledConfig *oledConfig, uint16_t *value);

bool media_setAdd(const OledConfig *oledConfig, uint16_t hiAddr,
                  uint16_t loAddr);

bool media_setSector(const OledConfig *oledConfig, uint16_t hiAddr,
                     uint16_t loAddr, uint16_t *value);

bool media_readByte(const OledConfig *oledConfig, uint16_t *value);

bool media_readWord(const OledConfig *oledConfig, uint16_t *value);

bool media_writeByte(const OledConfig *oledConfig, uint16_t value,
                     uint16_t *status);

bool media_writeWord(const OledConfig *oledConfig, uint16_t value,
                     uint16_t *status);

bool media_flush(const OledConfig *oledConfig, uint16_t *status);

bool media_image(const OledConfig *oledConfig, uint16_t x, uint16_t y);

bool media_video(const OledConfig *oledConfig, uint16_t x, uint16_t y);

bool media_videoFrame(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                      int16_t frameNumber);

bool misc_peekB(const OledConfig *oledConfig, uint16_t eveReg, uint16_t index,
                uint16_t *value);

bool misc_pokeB(const OledConfig *oledConfig, uint16_t eveReg, uint16_t index,
                uint16_t value);

bool misc_peekW(const OledConfig *oledConfig, uint16_t eveReg, uint16_t index,
                uint16_t *value);

bool misc_pokeW(const OledConfig *oledConfig, uint16_t eveReg, uint16_t index,
                uint16_t value);

bool misc_peekM(const OledConfig *oledConfig, uint16_t address,
                uint16_t *value);

bool misc_pokeM(const OledConfig *oledConfig, uint16_t address, uint16_t value);

bool misc_joystick(const OledConfig *oledConfig, uint16_t *value);

// note=[0..64]
bool misc_beep(const OledConfig *oledConfig, uint16_t note,
               uint16_t duration_ms);

// The response will be 0x06 at the new baud rate set, 100ms after the command
// is sent
bool misc_setbaudWait(const OledConfig *oledConfig, int16_t index);

bool sys_sleep(const OledConfig *oledConfig, uint16_t duration_s,
               uint16_t *duration);

bool sys_memFree(const OledConfig *oledConfig, uint16_t handle,
                 uint16_t *status);

bool sys_memHeap(const OledConfig *oledConfig, uint16_t *avail);

bool sys_getModel(const OledConfig *oledConfig, uint16_t *n, char *str);

bool sys_getVersion(const OledConfig *oledConfig, uint16_t *version);

bool sys_getPmmC(const OledConfig *oledConfig, uint16_t *version);

// 1..65535
bool misc_screenSaverTimeout(const OledConfig *oledConfig, uint16_t timout_ms);

// see reference manuel
bool misc_screenSaverSpeed(const OledConfig *oledConfig, uint16_t speed_index);

// see reference manuel
bool misc_screenSaverMode(const OledConfig *oledConfig, uint16_t mode);

bool touch_detectRegion(const OledConfig *oledConfig, uint16_t x1, uint16_t y1,
                        uint16_t x2, uint16_t y2);

// 0..2
bool touch_set(const OledConfig *oledConfig, uint16_t mode);

// 0..2
bool touch_get(const OledConfig *oledConfig, uint16_t mode, uint16_t *value);

bool file_error(const OledConfig *oledConfig, uint16_t *errno);

bool file_count(const OledConfig *oledConfig, const char *filename,
                uint16_t *count);

bool file_dir(const OledConfig *oledConfig, const char *filename,
              uint16_t *count);

bool file_findFirst(const OledConfig *oledConfig, const char *filename,
                    uint16_t *status);

bool file_findFirstRet(const OledConfig *oledConfig, const char *filename,
                       uint16_t *n, char *str);

bool file_findNext(const OledConfig *oledConfig, uint16_t *status);

bool file_findNextRet(const OledConfig *oledConfig, uint16_t *n, char *str);

bool file_exists(const OledConfig *oledConfig, const char *filename,
                 uint16_t *status);

bool file_open(const OledConfig *oledConfig, const char *filename, char mode,
               uint16_t *handle);

bool file_close(const OledConfig *oledConfig, uint16_t handle,
                uint16_t *status);

bool file_read(const OledConfig *oledConfig, uint16_t size, uint16_t handle,
               uint16_t *n, char *str);

bool file_seek(const OledConfig *oledConfig, uint16_t handle, uint16_t hiWord,
               uint16_t loWord, uint16_t *status);

bool file_index(const OledConfig *oledConfig, uint16_t handle, uint16_t hiWord,
                uint16_t loWord, uint16_t recordNum, uint16_t *status);

bool file_tell(const OledConfig *oledConfig, uint16_t handle, uint16_t *status,
               uint16_t *hiWord, uint16_t *loWord);

bool file_write(const OledConfig *oledConfig, uint16_t size, uint16_t source,
                uint16_t handle, uint16_t *count);

bool file_size(const OledConfig *oledConfig, uint16_t handle, uint16_t *status,
               uint16_t *hiWord, uint16_t *loWord);

bool file_image(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                uint16_t handle, uint16_t *errno);

bool file_screenCapture(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                        uint16_t width, uint16_t height, uint16_t handle,
                        uint16_t *status);

bool file_putC(const OledConfig *oledConfig, uint16_t car, uint16_t handle,
               uint16_t *status);

bool file_getC(const OledConfig *oledConfig, uint16_t handle, uint16_t *car);

bool file_putW(const OledConfig *oledConfig, uint16_t word, uint16_t handle,
               uint16_t *status);

bool file_getW(const OledConfig *oledConfig, uint16_t handle, uint16_t *word);

bool file_putS(const OledConfig *oledConfig, const char *cstr, uint16_t *count);

bool file_getS(const OledConfig *oledConfig, uint16_t size, uint16_t handle,
               uint16_t *n, char *str);

bool file_erase(const OledConfig *oledConfig, const char *filename,
                uint16_t *status);

bool file_rewind(const OledConfig *oledConfig, uint16_t handle,
                 uint16_t *status);

bool file_loadFunction(const OledConfig *oledConfig, const char *filename,
                       uint16_t *pointer);

bool file_callFunction(const OledConfig *oledConfig, uint16_t handle,
                       uint16_t n, const uint16_t *args, uint16_t *value);

bool file_run(const OledConfig *oledConfig, const char *filename, uint16_t n,
              const uint16_t *args, uint16_t *value);

bool file_exec(const OledConfig *oledConfig, const char *filename, uint16_t n,
               const uint16_t *args, uint16_t *value);

bool file_loadImageControl(const OledConfig *oledConfig, const char *filename1,
                           const char *filename2, uint16_t mode,
                           uint16_t *handle);

bool file_mount(const OledConfig *oledConfig, uint16_t *status);

bool file_unmount(const OledConfig *oledConfig);

bool file_playWAV(const OledConfig *oledConfig, const char *filename,
                  uint16_t *status);

bool file_writeString(const OledConfig *oledConfig, uint16_t handle,
                      const char *cstr, uint16_t *pointer);

// level is 0 – 127
bool snd_volume(const OledConfig *oledConfig, uint16_t level);

// rate is 4000 – 65535
bool snd_pitch(const OledConfig *oledConfig, uint16_t rate, uint16_t *oldRate);

// 0 .. 2
bool snd_bufSize(const OledConfig *oledConfig, uint16_t bufferSize);

bool snd_stop(const OledConfig *oledConfig);

bool snd_pause(const OledConfig *oledConfig);

bool snd_continue(const OledConfig *oledConfig);

bool snd_playing(const OledConfig *oledConfig, uint16_t *togo);

bool img_setPosition(const OledConfig *oledConfig, uint16_t handle,
                     int16_t index, uint16_t xpos, uint16_t ypos,
                     uint16_t *status);

bool img_enable(const OledConfig *oledConfig, uint16_t handle, int16_t index,
                uint16_t *status);

bool img_disable(const OledConfig *oledConfig, uint16_t handle, int16_t index,
                 uint16_t *status);

bool img_darken(const OledConfig *oledConfig, uint16_t handle, int16_t index,
                uint16_t *status);

bool img_lighten(const OledConfig *oledConfig, uint16_t handle, int16_t index,
                 uint16_t *status);

bool img_setWord(const OledConfig *oledConfig, uint16_t handle, int16_t index,
                 uint16_t offset, uint16_t value, uint16_t *status);

bool img_getWord(const OledConfig *oledConfig, uint16_t handle, int16_t index,
                 uint16_t offset, uint16_t *value);

bool img_show(const OledConfig *oledConfig, uint16_t handle, int16_t index,
              uint16_t *status);

bool img_setAttributes(const OledConfig *oledConfig, uint16_t handle,
                       int16_t index, uint16_t value, uint16_t *status);

bool img_clearAttributes(const OledConfig *oledConfig, uint16_t handle,
                         int16_t index, uint16_t value, uint16_t *status);

bool img_touched(const OledConfig *oledConfig, uint16_t handle, int16_t index,
                 int16_t *value);

bool img_blitComtoDisplay(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height,
                          const uint16_t *data);

bool bus_in(const OledConfig *oledConfig, uint16_t *busState);

bool bus_out(const OledConfig *oledConfig, uint16_t busState);

bool bus_read(const OledConfig *oledConfig, uint16_t *busState);

bool bus_set(const OledConfig *oledConfig, uint16_t dirMask);

bool bus_write(const OledConfig *oledConfig, uint16_t bitfield);

bool pin_hi(const OledConfig *oledConfig, uint16_t pin, uint16_t *status);

bool pin_lo(const OledConfig *oledConfig, uint16_t pin, uint16_t *status);

bool pin_read(const OledConfig *oledConfig, uint16_t pin, uint16_t *status);

// Pin=1..7 mode = 0 .. 1 (see reference manuel)
bool pin_set_picaso(const OledConfig *oledConfig, uint16_t mode, uint16_t pin,
                    uint16_t *status);

// Pin = 1..16 mode = 0..6 (see reference manuel)
bool pin_set_diablo(const OledConfig *oledConfig, uint16_t mode, uint16_t pin,
                    uint16_t *status);
