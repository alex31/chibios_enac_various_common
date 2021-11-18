#pragma once
#include <ch.h>
#include <hal.h>

typedef struct OledConfig OledConfig;

bool txt_moveCursor(OledConfig *oledConfig, uint16_t line, uint16_t column);

bool txt_putCh(OledConfig *oledConfig, uint16_t car);

// gold : strlen <= 255, picaso+diablo : strlen <= 511
bool txt_putStr(OledConfig *oledConfig, const char *cstr, uint16_t *length);

bool txt_charWidth(OledConfig *oledConfig, char car, uint16_t *width);

bool txt_charHeight(OledConfig *oledConfig, char car, uint16_t *height);

bool txt_fgColour(OledConfig *oledConfig, uint16_t colour, uint16_t *oldCol);

bool txt_bgColour(OledConfig *oledConfig, uint16_t colour, uint16_t *oldCol);

bool txt_fontID(OledConfig *oledConfig, uint16_t id, uint16_t *oldFont);

bool txt_widthMult(OledConfig *oledConfig, uint16_t wMultiplier,
                   uint16_t *oldMul);

bool txt_heightMult(OledConfig *oledConfig, uint16_t hMultiplier,
                    uint16_t *oldMul);

// 00 .. 32
bool txt_xgap(OledConfig *oledConfig, uint16_t xGap, uint16_t *oldGap);

// 00 .. 32
bool txt_ygap(OledConfig *oledConfig, uint16_t yGap, uint16_t *oldGap);

// 0 .. 1
bool txt_bold(OledConfig *oledConfig, uint16_t mode, uint16_t *oldBold);

// 0 .. 1
bool txt_inverse(OledConfig *oledConfig, uint16_t mode, uint16_t *oldInv);

// 0 .. 1
bool txt_italic(OledConfig *oledConfig, uint16_t mode, uint16_t *oldItal);

// 0 .. 1
bool txt_opacity(OledConfig *oledConfig, uint16_t mode, uint16_t *oldOpa);

// 0 .. 1
bool txt_underline(OledConfig *oledConfig, uint16_t mode, uint16_t *oldUnder);

// bold:1<<4, italic:1<<5, inverse:1<<6, underlined1<<7
bool txt_attributes(OledConfig *oledConfig, uint16_t bitfield,
                    uint16_t *oldAttr);

// function=[7: text print delay], delai=[0 .. 255 ms]
bool txt_set(OledConfig *oledConfig, uint16_t function, uint16_t value);

// 0:off, 1..N:wrap position
bool txt_wrap(OledConfig *oledConfig, uint16_t xpos, uint16_t *oldWrap);

bool gfx_cls(OledConfig *oledConfig);

bool gfx_changeColour(OledConfig *oledConfig, uint16_t oldColor,
                      uint16_t newColor);

bool gfx_circle(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t radius,
                uint16_t colour);

bool gfx_circleFilled(OledConfig *oledConfig, uint16_t x, uint16_t y,
                      uint16_t radius, uint16_t colour);

bool gfx_line(OledConfig *oledConfig, uint16_t x1, uint16_t y1, uint16_t x2,
              uint16_t y2, uint16_t colour);

bool gfx_rectangle(OledConfig *oledConfig, uint16_t tlx, uint16_t tly,
                   uint16_t brx, uint16_t bry, uint16_t colour);

bool gfx_rectangleFilled(OledConfig *oledConfig, uint16_t tlx, uint16_t tly,
                         uint16_t brx, uint16_t bry, uint16_t colour);

bool gfx_polyline(OledConfig *oledConfig, uint16_t n, uint16_t vx[],
                  uint16_t vy[], uint16_t color);

bool gfx_polygon(OledConfig *oledConfig, uint16_t n, uint16_t vx[],
                 uint16_t vy[], uint16_t color);

bool gfx_polygonFilled(OledConfig *oledConfig, uint16_t n, uint16_t vx[],
                       uint16_t vy[], uint16_t color);

bool gfx_triangle(OledConfig *oledConfig, uint16_t x1, uint16_t y1, uint16_t x2,
                  uint16_t y2, uint16_t x3, uint16_t y3, uint16_t colour);

bool gfx_triangleFilled(OledConfig *oledConfig, uint16_t x1, uint16_t y1,
                        uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
                        uint16_t colour);

bool gfx_orbit(OledConfig *oledConfig, uint16_t angle, uint16_t distance,
               uint16_t *Xdist, uint16_t *Ydist);

bool gfx_putPixel(OledConfig *oledConfig, uint16_t x, uint16_t y,
                  uint16_t colour);

bool gfx_getPixel(OledConfig *oledConfig, uint16_t x, uint16_t y,
                  uint16_t *colour);

bool gfx_moveTo(OledConfig *oledConfig, uint16_t x, uint16_t y);

bool gfx_lineTo(OledConfig *oledConfig, uint16_t x, uint16_t y);

// 0 .. 1
bool gfx_clipping(OledConfig *oledConfig, uint16_t mode);

bool gfx_clipWindow(OledConfig *oledConfig, uint16_t tlx, uint16_t tly,
                    uint16_t brx, uint16_t bry);

bool gfx_setClipRegion(OledConfig *oledConfig);

bool gfx_ellipse(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t xrad,
                 uint16_t yrad, uint16_t colour);

bool gfx_ellipseFilled(OledConfig *oledConfig, uint16_t x, uint16_t y,
                       uint16_t xrad, uint16_t yrad, uint16_t colour);

bool gfx_button(OledConfig *oledConfig, uint16_t state, uint16_t x, uint16_t y,
                uint16_t buttoncolour, uint16_t txtcolour, uint16_t font,
                uint16_t twtWidth, uint16_t twtHeight, const char *cstr);

bool gfx_panel(OledConfig *oledConfig, uint16_t state, uint16_t x, uint16_t y,
               uint16_t width, uint16_t height, uint16_t colour);

bool gfx_slider(OledConfig *oledConfig, uint16_t mode, uint16_t x1, uint16_t y1,
                uint16_t x2, uint16_t y2, uint16_t colour, uint16_t scale,
                int16_t value);

bool gfx_screenCopyPaste(OledConfig *oledConfig, uint16_t xs, uint16_t ys,
                         uint16_t xd, uint16_t yd, uint16_t width,
                         uint16_t height);

// number of pixel 0..4
bool gfx_bevelShadow(OledConfig *oledConfig, uint16_t value,
                     uint16_t *oldBevel);

// 0..15
bool gfx_bevelWidth(OledConfig *oledConfig, uint16_t value, uint16_t *oldWidth);

bool gfx_bgCcolour(OledConfig *oledConfig, uint16_t colour, uint16_t *oldCol);

bool gfx_outlineColour(OledConfig *oledConfig, uint16_t colour,
                       uint16_t *oldCol);

// 0..15
bool gfx_contrast(OledConfig *oledConfig, uint16_t contrast,
                  uint16_t *oldContrast);

bool gfx_frameDelay(OledConfig *oledConfig, uint16_t delayMsec,
                    uint16_t *oldDelay);

bool gfx_linePattern(OledConfig *oledConfig, uint16_t pattern,
                     uint16_t *oldPattern);

// 0 = LANDSCAPE, 1 = LANDSCAPE REVERSE, 2=portrait, 3=portrait_reverse
bool gfx_screenMode(OledConfig *oledConfig, uint16_t mode, uint16_t *oldMode);

// 0..1
bool gfx_transparency(OledConfig *oledConfig, uint16_t mode, uint16_t *oldMode);

bool gfx_transparentColour(OledConfig *oledConfig, uint16_t colour,
                           uint16_t *oldColor);

// see manual
bool gfx_set(OledConfig *oledConfig, uint16_t function, uint16_t value);

bool gfx_get(OledConfig *oledConfig, uint16_t mode, uint16_t *value);

bool media_init(OledConfig *oledConfig, uint16_t *value);

bool media_setAdd(OledConfig *oledConfig, uint16_t hiAddr, uint16_t loAddr);

bool media_setSector(OledConfig *oledConfig, uint16_t hiAddr, uint16_t loAddr,
                     uint16_t *value);

bool media_readByte(OledConfig *oledConfig, uint16_t *value);

bool media_readWord(OledConfig *oledConfig, uint16_t *value);

bool media_writeByte(OledConfig *oledConfig, uint16_t value, uint16_t *status);

bool media_writeWord(OledConfig *oledConfig, uint16_t value, uint16_t *status);

bool media_flush(OledConfig *oledConfig, uint16_t *status);

bool media_image(OledConfig *oledConfig, uint16_t x, uint16_t y);

bool media_video(OledConfig *oledConfig, uint16_t x, uint16_t y);

bool media_videoFrame(OledConfig *oledConfig, uint16_t x, uint16_t y,
                      int16_t frameNumber);

bool misc_peekB(OledConfig *oledConfig, uint16_t eveReg, uint16_t index,
                uint16_t *value);

bool misc_pokeB(OledConfig *oledConfig, uint16_t eveReg, uint16_t index,
                uint16_t value);

bool misc_peekW(OledConfig *oledConfig, uint16_t eveReg, uint16_t index,
                uint16_t *value);

bool misc_pokeW(OledConfig *oledConfig, uint16_t eveReg, uint16_t index,
                uint16_t value);

bool misc_peekM(OledConfig *oledConfig, uint16_t address, uint16_t *value);

bool misc_pokeM(OledConfig *oledConfig, uint16_t address, uint16_t value);

bool misc_joystick(OledConfig *oledConfig, uint16_t *value);

// note=[0..64]
bool misc_beep(OledConfig *oledConfig, uint16_t note, uint16_t duration_ms);

// The response will be 0x06 at the new baud rate set, 100ms after the command
// is sent
bool misc_setbaudWait(OledConfig *oledConfig, int16_t index);

bool sys_sleep(OledConfig *oledConfig, uint16_t duration_s, uint16_t *duration);

bool sys_memFree(OledConfig *oledConfig, uint16_t handle, uint16_t *status);

bool sys_memHeap(OledConfig *oledConfig, uint16_t *avail);

bool sys_getModel(OledConfig *oledConfig, uint16_t *n, char *str);

bool sys_getVersion(OledConfig *oledConfig, uint16_t *version);

bool sys_getPmmC(OledConfig *oledConfig, uint16_t *version);

// 1..65535
bool misc_screenSaverTimeout(OledConfig *oledConfig, uint16_t timout_ms);

// see reference manuel
bool misc_screenSaverSpeed(OledConfig *oledConfig, uint16_t speed_index);

// see reference manuel
bool misc_screenSaverMode(OledConfig *oledConfig, uint16_t mode);

bool touch_detectRegion(OledConfig *oledConfig, uint16_t x1, uint16_t y1,
                        uint16_t x2, uint16_t y2);

// 0..2
bool touch_set(OledConfig *oledConfig, uint16_t mode);

// 0..2
bool touch_get(OledConfig *oledConfig, uint16_t mode, uint16_t *value);

bool file_error(OledConfig *oledConfig, uint16_t *errno);

bool file_count(OledConfig *oledConfig, const char *filename, uint16_t *count);

bool file_dir(OledConfig *oledConfig, const char *filename, uint16_t *count);

bool file_findFirst(OledConfig *oledConfig, const char *filename,
                    uint16_t *status);

bool file_findFirstRet(OledConfig *oledConfig, const char *filename,
                       uint16_t *n, char *str);

bool file_findNext(OledConfig *oledConfig, uint16_t *status);

bool file_findNextRet(OledConfig *oledConfig, uint16_t *n, char *str);

bool file_exists(OledConfig *oledConfig, const char *filename,
                 uint16_t *status);

bool file_open(OledConfig *oledConfig, const char *filename, char mode,
               uint16_t *handle);

bool file_close(OledConfig *oledConfig, uint16_t handle, uint16_t *status);

bool file_read(OledConfig *oledConfig, uint16_t size, uint16_t handle,
               uint16_t *n, char *str);

bool file_seek(OledConfig *oledConfig, uint16_t handle, uint16_t hiWord,
               uint16_t loWord, uint16_t *status);

bool file_index(OledConfig *oledConfig, uint16_t handle, uint16_t hiWord,
                uint16_t loWord, uint16_t recordNum, uint16_t *status);

bool file_tell(OledConfig *oledConfig, uint16_t handle, uint16_t *status,
               uint16_t *hiWord, uint16_t *loWord);

bool file_write(OledConfig *oledConfig, uint16_t size, uint16_t source,
                uint16_t handle, uint16_t *count);

bool file_size(OledConfig *oledConfig, uint16_t handle, uint16_t *status,
               uint16_t *hiWord, uint16_t *loWord);

bool file_image(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t handle,
                uint16_t *errno);

bool file_screenCapture(OledConfig *oledConfig, uint16_t x, uint16_t y,
                        uint16_t width, uint16_t height, uint16_t handle,
                        uint16_t *status);

bool file_putC(OledConfig *oledConfig, uint16_t car, uint16_t handle,
               uint16_t *status);

bool file_getC(OledConfig *oledConfig, uint16_t handle, uint16_t *car);

bool file_putW(OledConfig *oledConfig, uint16_t word, uint16_t handle,
               uint16_t *status);

bool file_getW(OledConfig *oledConfig, uint16_t handle, uint16_t *word);

bool file_putS(OledConfig *oledConfig, const char *cstr, uint16_t *count);

bool file_getS(OledConfig *oledConfig, uint16_t size, uint16_t handle,
               uint16_t *n, char *str);

bool file_erase(OledConfig *oledConfig, const char *filename, uint16_t *status);

bool file_rewind(OledConfig *oledConfig, uint16_t handle, uint16_t *status);

bool file_loadFunction(OledConfig *oledConfig, const char *filename,
                       uint16_t *pointer);

bool file_callFunction(OledConfig *oledConfig, uint16_t handle, uint16_t n,
                       uint16_t *args, uint16_t *value);

bool file_run(OledConfig *oledConfig, const char *filename, uint16_t n,
              uint16_t *args, uint16_t *value);

bool file_exec(OledConfig *oledConfig, const char *filename, uint16_t n,
               uint16_t *args, uint16_t *value);

bool file_loadImageControl(OledConfig *oledConfig, const char *filename1,
                           const char *filename2, uint16_t mode,
                           uint16_t *handle);

bool file_mount(OledConfig *oledConfig, uint16_t *status);

bool file_unmount(OledConfig *oledConfig);

bool file_playWAV(OledConfig *oledConfig, const char *filename,
                  uint16_t *status);

bool file_writeString(OledConfig *oledConfig, uint16_t handle, const char *cstr,
                      uint16_t *pointer);

// level is 0 – 127
bool snd_volume(OledConfig *oledConfig, uint16_t level);

// rate is 4000 – 65535
bool snd_pitch(OledConfig *oledConfig, uint16_t rate, uint16_t *oldRate);

// 0 .. 2
bool snd_bufSize(OledConfig *oledConfig, uint16_t bufferSize);

bool snd_stop(OledConfig *oledConfig);

bool snd_pause(OledConfig *oledConfig);

bool snd_continue(OledConfig *oledConfig);

bool snd_playing(OledConfig *oledConfig, uint16_t *togo);

bool img_setPosition(OledConfig *oledConfig, uint16_t handle, int16_t index,
                     uint16_t xpos, uint16_t ypos, uint16_t *status);

bool img_enable(OledConfig *oledConfig, uint16_t handle, int16_t index,
                uint16_t *status);

bool img_disable(OledConfig *oledConfig, uint16_t handle, int16_t index,
                 uint16_t *status);

bool img_darken(OledConfig *oledConfig, uint16_t handle, int16_t index,
                uint16_t *status);

bool img_lighten(OledConfig *oledConfig, uint16_t handle, int16_t index,
                 uint16_t *status);

bool img_setWord(OledConfig *oledConfig, uint16_t handle, int16_t index,
                 uint16_t offset, uint16_t value, uint16_t *status);

bool img_getWord(OledConfig *oledConfig, uint16_t handle, int16_t index,
                 uint16_t offset, uint16_t *value);

bool img_show(OledConfig *oledConfig, uint16_t handle, int16_t index,
              uint16_t *status);

bool img_setAttributes(OledConfig *oledConfig, uint16_t handle, int16_t index,
                       uint16_t value, uint16_t *status);

bool img_clearAttributes(OledConfig *oledConfig, uint16_t handle, int16_t index,
                         uint16_t value, uint16_t *status);

bool img_touched(OledConfig *oledConfig, uint16_t handle, int16_t index,
                 int16_t *value);

bool img_blitComtoDisplay(OledConfig *oledConfig, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height,
                          const uint16_t *data);

bool bus_in(OledConfig *oledConfig, uint16_t *busState);

bool bus_out(OledConfig *oledConfig, uint16_t busState);

bool bus_read(OledConfig *oledConfig, uint16_t *busState);

bool bus_set(OledConfig *oledConfig, uint16_t dirMask);

bool bus_write(OledConfig *oledConfig, uint16_t bitfield);

bool pin_hi(OledConfig *oledConfig, uint16_t pin, uint16_t *status);

bool pin_lo(OledConfig *oledConfig, uint16_t pin, uint16_t *status);

bool pin_read(OledConfig *oledConfig, uint16_t pin, uint16_t *status);

// Pin=1..7 mode = 0 .. 1 (see reference manuel)
bool pin_set_picaso(OledConfig *oledConfig, uint16_t mode, uint16_t pin,
                    uint16_t *status);

// Pin = 1..16 mode = 0..6 (see reference manuel)
bool pin_set_diablo(OledConfig *oledConfig, uint16_t mode, uint16_t pin,
                    uint16_t *status);
