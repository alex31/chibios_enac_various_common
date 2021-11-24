#pragma once
#include <ch.h>
#include <hal.h>

typedef struct OledConfig OledConfig;

// line : Holds a positive value for the required line position. column :Holds a
// positive value for the required column position. Moves the text cursor to a
// screen position set by line and column parameters. The line and column
// position is calculated, based on the size and scaling factor for the
// currently selected font. When text is outputted to screen it will be
// displayed from this position. The text position could also be set with “Move
// Origin” command if required to set the text position to an exact pixel
// location. Note that lines and columns start from 0, so line 0, column 0 is
// the top left corner of the display.
bool txt_moveCursor(const OledConfig *oledConfig, uint16_t line,
                    uint16_t column);

// car : Holds a positive value for the required character.
// Prints a single character to the display
bool txt_putCh(const OledConfig *oledConfig, uint16_t car);

// cstr : Holds a Null terminated string. NOTE: Maximum characters in the string
// is 255 for GOLDELOX or 511 for PICASO and DIABLO16 length : Length of the
// string printed Prints a string to the display and returns the pointer to the
// string. argument(s) length not returned if screen is of goldelox type
bool txt_putStr(const OledConfig *oledConfig, const char *cstr,
                uint16_t *length);

// car : The ASCII character for the width calculation.
// width : Width of a single character in pixel units.
// Calculate the width in pixel units for a character, based on the currently
// selected font. The font can be proportional or mono-spaced. If the total
// width of the character exceeds 255 pixel units, the function will return the
// 'wrapped' (modulo 256) value.
bool txt_charWidth(const OledConfig *oledConfig, char car, uint16_t *width);

// car : The ASCII character for the height calculation.
// height : Height of a single character in pixel units.
// Calculate the height in pixel units for a character, based on the currently
// selected font. The font can be proportional or mono-spaced. If the total
// height of the character exceeds 255 pixel units, the function will return the
// 'wrapped' (modulo 256) value.
bool txt_charHeight(const OledConfig *oledConfig, char car, uint16_t *height);

// Colour : colour to be set.
// oldCol : Previous Text Foreground Colour.
// Sets the text foreground colour.
// argument(s) oldCol not returned if screen is of goldelox type
bool txt_fgColour(const OledConfig *oledConfig, uint16_t colour,
                  uint16_t *oldCol);

// Colour : colour to be set.
// oldCol : Previous Text Background Colour.
// Sets the text background colour.
// argument(s) oldCol not returned if screen is of goldelox type
bool txt_bgColour(const OledConfig *oledConfig, uint16_t colour,
                  uint16_t *oldCol);

// Id : 0 for System font (Default Fonts), 7 for Media fonts
// oldFont : Previous Font ID.
// Sets the required font using its ID.
// argument(s) oldFont not returned if screen is of goldelox type
bool txt_fontID(const OledConfig *oledConfig, uint16_t id, uint16_t *oldFont);

// wMultiplier : Width multiplier, 1 to 16 (Default =1)
// oldMul : Previous Multiplier value.
// Sets the text width multiplier between 1 and 16.
// argument(s) oldMul not returned if screen is of goldelox type
bool txt_widthMult(const OledConfig *oledConfig, uint16_t wMultiplier,
                   uint16_t *oldMul);

// hMultiplier : Height multiplier, 1 to 16 (Default =1)
// oldMul : Previous Multiplier value.
// Sets the text height multiplier between 1 and 16.
// argument(s) oldMul not returned if screen is of goldelox type
bool txt_heightMult(const OledConfig *oledConfig, uint16_t hMultiplier,
                    uint16_t *oldMul);

// xGap : pixelcount 0 to 32(Default =0)
// oldGap : Previous pixelcount value.
// Sets the pixel gap between characters (x-axis), where the gap is in pixel
// units. argument(s) oldGap not returned if screen is of goldelox type
bool txt_xgap(const OledConfig *oledConfig, uint16_t xGap, uint16_t *oldGap);

// yGap : pixelcount 0 to 32(Default =0)
// oldGap : Previous pixelcount value.
// Sets the pixel gap between characters (y-axis), where the gap is in pixel
// units. argument(s) oldGap not returned if screen is of goldelox type
bool txt_ygap(const OledConfig *oledConfig, uint16_t yGap, uint16_t *oldGap);

// mode : 1 for ON, 0 for OFF.
// oldBold : Previous Bold status.
// Sets the Bold attribute for the text.
// argument(s) oldBold not returned if screen is of goldelox type
bool txt_bold(const OledConfig *oledConfig, uint16_t mode, uint16_t *oldBold);

// mode : 1 for ON, 0 for OFF.
// oldInv : Previous ‘Text Inverse’ status.
// Sets the text to be inverse.
// argument(s) oldInv not returned if screen is of goldelox type
bool txt_inverse(const OledConfig *oledConfig, uint16_t mode, uint16_t *oldInv);

// mode : 1 for ON, 0 for OFF.
// oldItal : Previous Italic Text status.
// Sets the text to italic.
// argument(s) oldItal not returned if screen is of goldelox type
bool txt_italic(const OledConfig *oledConfig, uint16_t mode, uint16_t *oldItal);

// mode : 1 for ON. (Opaque), 0 for OFF. (Transparent)
// oldOpa : Previous Text Opacity status.
// Selects whether or not the 'background' pixels are drawn (Default mode is
// OPAQUE with BLACK background). argument(s) oldOpa not returned if screen is
// of goldelox type
bool txt_opacity(const OledConfig *oledConfig, uint16_t mode, uint16_t *oldOpa);

// mode : 1 for ON, 0 for OFF.
// oldUnder : Previous Text Underline status.
// Sets the text to underlined.vThe ‘Text Underline’ attribute is cleared
// internally once the text (character or string) is displayed. Note: The “Text
// Y-gap” command is required to be at least 2 for the underline to be visible.
bool txt_underline(const OledConfig *oledConfig, uint16_t mode,
                   uint16_t *oldUnder);

// bitfield : bold=1<<4, italic=1<<5, inverse=1<<6, underlined=1<<7 (bits 0-3
// and 8-15 are reserved) oldAttr : Previous Text Attributes status. Controls
// the following functions grouped : Text Bold, Text Italic, Text Inverse, Text
// Underlined. The Attributes are set to normal internally once the text
// (character or string) is displayed. argument(s) oldAttr not returned if
// screen is of goldelox type
bool txt_attributes(const OledConfig *oledConfig, uint16_t bitfield,
                    uint16_t *oldAttr);

// function : 7 = Sets the Delay between the characters being printed through
// Put Character or Put String functions, value = 0(Default) to 255 (ms) Sets
// various parameters for the Text commands.
bool txt_set(const OledConfig *oledConfig, uint16_t function, uint16_t value);

// xpos : 0 for OFF (Default), 1 to N for Wrap position in Pixels.
// oldWrap : Previous wrap position
// Sets the pixel position where text wrap will occur at RHS. The feature
// automatically resets when screen mode is changed.
bool txt_wrap(const OledConfig *oledConfig, uint16_t xpos, uint16_t *oldWrap);

// Clears the screen using the current background colour. Brings some of the
// settings back to default such as Outline colour set to BLACK, Pen set to
// OUTLINE, Text magnifications set to 1, All origins set to 0:0.
bool gfx_cls(const OledConfig *oledConfig);

// oldColor : sample colour to be changed within the clipping window, newColor :
// new colour to change all occurrences of old colour within the clipping window
// Changes all oldColour pixels to newColour within the clipping window area.
bool gfx_changeColour(const OledConfig *oledConfig, uint16_t oldColour,
                      uint16_t newColour);

// x, y : centre of the circle, radius : radius of the circle, colour : colour
// of the circle. Draws a circle with centre point and radius using the
// specified colour.
bool gfx_circle(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                uint16_t radius, uint16_t colour);

// x, y : centre of the circle, radius : radius of the circle, colour : colour
// of the circle. Draws a solid circle with centre point and radius using the
// specified colour. Outline colour can be specified with the “Outline Colour”
// command, if set to 0, no outline is drawn.
bool gfx_circleFilled(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                      uint16_t radius, uint16_t colour);

// x1,y1 : starting coordinates of the line, x2,y2 : ending coordinates of the
// line, colour : colour of the line. Draws a line from x1,y1 to x2,y2 using the
// specified colour. The line is drawn using the current object colour. The
// current origin is not altered. The line may be tessellated with the “Line
// Pattern” command.
bool gfx_line(const OledConfig *oledConfig, uint16_t x1, uint16_t y1,
              uint16_t x2, uint16_t y2, uint16_t colour);

// tlx,tly : top left corner of the rectangle, brx,bry : bottom right corner of
// the rectangle, colour : colour of the rectangle. Draws a rectangle from x1,
// y1 to x2, y2 using the specified colour. The line may be tessellated with the
// “Line Pattern” command.
bool gfx_rectangle(const OledConfig *oledConfig, uint16_t tlx, uint16_t tly,
                   uint16_t brx, uint16_t bry, uint16_t colour);

// tlx,tly : top left corner of the rectangle, brx,bry : bottom right corner of
// the rectangle, colour : colour of the rectangle. Draws a Filled rectangle
// from x1, y1 to x2, y2 using the specified colour. The line may be tessellated
// with the “Line Pattern” command. Outline colour can be specified with the
// “Outline Colour” command, if set to 0, no outline is drawn.
bool gfx_rectangleFilled(const OledConfig *oledConfig, uint16_t tlx,
                         uint16_t tly, uint16_t brx, uint16_t bry,
                         uint16_t colour);

// n : number of elements in the x and y arrays specifying the vertices for the
// polyline, vx[n] : array of elements for the x coordinates of the vertices,
// vy[n] : array of elements for the y coordinates of the vertices, colour :
// colour of the polyline. Plots lines between points specified by a pair of
// arrays using the specified colour. The lines may be tessellated with the
// “Line Pattern” command
bool gfx_polyline(const OledConfig *oledConfig, uint16_t n, const uint16_t vx[],
                  const uint16_t vy[], uint16_t color);

// n : number of elements in the x and y arrays specifying the vertices for the
// polygon, vx[n] : array of elements for the x coordinates of the vertices,
// vy[n] : array of elements for the y coordinates of the vertices, colour :
// colour of the polygon. Plots lines between points specified by a pair of
// arrays using the specified colour. The last point is drawn back to the first
// point, completing the polygon. The lines may be tessellated with “Line
// Pattern” command.
bool gfx_polygon(const OledConfig *oledConfig, uint16_t n, const uint16_t vx[],
                 const uint16_t vy[], uint16_t color);

// n : number of elements in the x and y arrays specifying the vertices for the
// polygon, vx[n] : array of elements for the x coordinates of the vertices,
// vy[n] : array of elements for the y coordinates of the vertices, colour :
// colour of the polygon. Draws a solid Polygon between specified vertices using
// the specified colour. The last point is drawn back to the first point,
// completing the polygon. Vertices must be a minimum of 3 and can be specified
// in any fashion.
bool gfx_polygonFilled(const OledConfig *oledConfig, uint16_t n,
                       const uint16_t vx[], const uint16_t vy[],
                       uint16_t color);

// x1,y1 : first vertice of the triangle, x2,y2 : second vertice of the
// triangle, x3,y3 : third vertice of the triangle, colour : colour of the
// triangle. Draws a triangle outline between specified vertices using the
// specified colour. The line may be tessellated with the “Line Pattern”
// command.
bool gfx_triangle(const OledConfig *oledConfig, uint16_t x1, uint16_t y1,
                  uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
                  uint16_t colour);

// x1,y1,x2,y2,x3,y3,colour
//
bool gfx_triangleFilled(const OledConfig *oledConfig, uint16_t x1, uint16_t y1,
                        uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
                        uint16_t colour);

//
bool gfx_orbit(const OledConfig *oledConfig, uint16_t angle, uint16_t distance,
               uint16_t *Xdist, uint16_t *Ydist);

//
bool gfx_putPixel(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                  uint16_t colour);

//
bool gfx_getPixel(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                  uint16_t *colour);

//
bool gfx_moveTo(const OledConfig *oledConfig, uint16_t x, uint16_t y);

//
bool gfx_lineTo(const OledConfig *oledConfig, uint16_t x, uint16_t y);

// 0 .. 1
//
bool gfx_clipping(const OledConfig *oledConfig, uint16_t mode);

//
bool gfx_clipWindow(const OledConfig *oledConfig, uint16_t tlx, uint16_t tly,
                    uint16_t brx, uint16_t bry);

//
bool gfx_setClipRegion(const OledConfig *oledConfig);

//
bool gfx_ellipse(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                 uint16_t xrad, uint16_t yrad, uint16_t colour);

//
bool gfx_ellipseFilled(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                       uint16_t xrad, uint16_t yrad, uint16_t colour);

//
bool gfx_button(const OledConfig *oledConfig, uint16_t state, uint16_t x,
                uint16_t y, uint16_t buttoncolour, uint16_t txtcolour,
                uint16_t font, uint16_t twtWidth, uint16_t twtHeight,
                const char *cstr);

//
bool gfx_panel(const OledConfig *oledConfig, uint16_t state, uint16_t x,
               uint16_t y, uint16_t width, uint16_t height, uint16_t colour);

//
bool gfx_slider(const OledConfig *oledConfig, uint16_t mode, uint16_t x1,
                uint16_t y1, uint16_t x2, uint16_t y2, uint16_t colour,
                uint16_t scale, int16_t value);

//
bool gfx_screenCopyPaste(const OledConfig *oledConfig, uint16_t xs, uint16_t ys,
                         uint16_t xd, uint16_t yd, uint16_t width,
                         uint16_t height);

// number of pixel 0..4
//
bool gfx_bevelShadow(const OledConfig *oledConfig, uint16_t value,
                     uint16_t *oldBevel);

// 0..15
//
bool gfx_bevelWidth(const OledConfig *oledConfig, uint16_t value,
                    uint16_t *oldWidth);

//
// argument(s) oldCol not returned if screen is of goldelox type
bool gfx_bgCcolour(const OledConfig *oledConfig, uint16_t colour,
                   uint16_t *oldCol);

//
// argument(s) oldCol not returned if screen is of goldelox type
bool gfx_outlineColour(const OledConfig *oledConfig, uint16_t colour,
                       uint16_t *oldCol);

// 0..15
//
// argument(s) oldContrast not returned if screen is of goldelox type
bool gfx_contrast(const OledConfig *oledConfig, uint16_t contrast,
                  uint16_t *oldContrast);

//
// argument(s) oldDelay not returned if screen is of goldelox type
bool gfx_frameDelay(const OledConfig *oledConfig, uint16_t delayMsec,
                    uint16_t *oldDelay);

//
// argument(s) oldPattern not returned if screen is of goldelox type
bool gfx_linePattern(const OledConfig *oledConfig, uint16_t pattern,
                     uint16_t *oldPattern);

// 0 = LANDSCAPE, 1 = LANDSCAPE REVERSE, 2=portrait, 3=portrait_reverse
//
// argument(s) oldMode not returned if screen is of goldelox type
bool gfx_screenMode(const OledConfig *oledConfig, uint16_t mode,
                    uint16_t *oldMode);

// 0..1
//
bool gfx_transparency(const OledConfig *oledConfig, uint16_t mode,
                      uint16_t *oldMode);

//
bool gfx_transparentColour(const OledConfig *oledConfig, uint16_t colour,
                           uint16_t *oldColor);

// see manual
//
bool gfx_set(const OledConfig *oledConfig, uint16_t function, uint16_t value);

//
bool gfx_get(const OledConfig *oledConfig, uint16_t mode, uint16_t *value);

//
// argument(s) value not returned if screen is of goldelox type
bool media_init(const OledConfig *oledConfig, uint16_t *value);

//
bool media_setAdd(const OledConfig *oledConfig, uint16_t hiAddr,
                  uint16_t loAddr);

//
// argument(s) value not returned if screen is of goldelox type
bool media_setSector(const OledConfig *oledConfig, uint16_t hiAddr,
                     uint16_t loAddr, uint16_t *value);

//
bool media_readByte(const OledConfig *oledConfig, uint16_t *value);

//
bool media_readWord(const OledConfig *oledConfig, uint16_t *value);

//
bool media_writeByte(const OledConfig *oledConfig, uint16_t value,
                     uint16_t *status);

//
bool media_writeWord(const OledConfig *oledConfig, uint16_t value,
                     uint16_t *status);

//
bool media_flush(const OledConfig *oledConfig, uint16_t *status);

//
bool media_image(const OledConfig *oledConfig, uint16_t x, uint16_t y);

//
bool media_video(const OledConfig *oledConfig, uint16_t x, uint16_t y);

//
bool media_videoFrame(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                      int16_t frameNumber);

//
bool misc_peekB(const OledConfig *oledConfig, uint16_t eveReg, uint16_t index,
                uint16_t *value);

//
bool misc_pokeB(const OledConfig *oledConfig, uint16_t eveReg, uint16_t index,
                uint16_t value);

//
bool misc_peekW(const OledConfig *oledConfig, uint16_t eveReg, uint16_t index,
                uint16_t *value);

//
bool misc_pokeW(const OledConfig *oledConfig, uint16_t eveReg, uint16_t index,
                uint16_t value);

//
bool misc_peekM(const OledConfig *oledConfig, uint16_t address,
                uint16_t *value);

//
bool misc_pokeM(const OledConfig *oledConfig, uint16_t address, uint16_t value);

//
bool misc_joystick(const OledConfig *oledConfig, uint16_t *value);

// note=[0..64]
//
bool misc_beep(const OledConfig *oledConfig, uint16_t note,
               uint16_t duration_ms);

// The response will be 0x06 at the new baud rate set, 100ms after the command
// is sent
//
bool misc_setbaudWait(const OledConfig *oledConfig, int16_t index);

//
bool sys_sleep(const OledConfig *oledConfig, uint16_t duration_s,
               uint16_t *duration);

//
bool sys_memFree(const OledConfig *oledConfig, uint16_t handle,
                 uint16_t *status);

//
bool sys_memHeap(const OledConfig *oledConfig, uint16_t *avail);

//
bool sys_getModel(const OledConfig *oledConfig, uint16_t *n, char *str);

//
bool sys_getVersion(const OledConfig *oledConfig, uint16_t *version);

//
bool sys_getPmmC(const OledConfig *oledConfig, uint16_t *version);

// 1..65535
//
bool misc_screenSaverTimeout(const OledConfig *oledConfig, uint16_t timout_ms);

// see reference manuel
//
bool misc_screenSaverSpeed(const OledConfig *oledConfig, uint16_t speed_index);

// see reference manuel
//
bool misc_screenSaverMode(const OledConfig *oledConfig, uint16_t mode);

//
bool touch_detectRegion(const OledConfig *oledConfig, uint16_t x1, uint16_t y1,
                        uint16_t x2, uint16_t y2);

// 0..2
//
bool touch_set(const OledConfig *oledConfig, uint16_t mode);

// 0..2
//
bool touch_get(const OledConfig *oledConfig, uint16_t mode, uint16_t *value);

//
bool file_error(const OledConfig *oledConfig, uint16_t *errno);

//
bool file_count(const OledConfig *oledConfig, const char *filename,
                uint16_t *count);

//
bool file_dir(const OledConfig *oledConfig, const char *filename,
              uint16_t *count);

//
bool file_findFirst(const OledConfig *oledConfig, const char *filename,
                    uint16_t *status);

//
bool file_findFirstRet(const OledConfig *oledConfig, const char *filename,
                       uint16_t *n, char *str);

//
bool file_findNext(const OledConfig *oledConfig, uint16_t *status);

//
bool file_findNextRet(const OledConfig *oledConfig, uint16_t *n, char *str);

//
bool file_exists(const OledConfig *oledConfig, const char *filename,
                 uint16_t *status);

//
bool file_open(const OledConfig *oledConfig, const char *filename, char mode,
               uint16_t *handle);

//
bool file_close(const OledConfig *oledConfig, uint16_t handle,
                uint16_t *status);

//
bool file_read(const OledConfig *oledConfig, uint16_t size, uint16_t handle,
               uint16_t *n, char *str);

//
bool file_seek(const OledConfig *oledConfig, uint16_t handle, uint16_t hiWord,
               uint16_t loWord, uint16_t *status);

//
bool file_index(const OledConfig *oledConfig, uint16_t handle, uint16_t hiWord,
                uint16_t loWord, uint16_t recordNum, uint16_t *status);

//
bool file_tell(const OledConfig *oledConfig, uint16_t handle, uint16_t *status,
               uint16_t *hiWord, uint16_t *loWord);

//
bool file_write(const OledConfig *oledConfig, uint16_t size, uint16_t source,
                uint16_t handle, uint16_t *count);

//
bool file_size(const OledConfig *oledConfig, uint16_t handle, uint16_t *status,
               uint16_t *hiWord, uint16_t *loWord);

//
bool file_image(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                uint16_t handle, uint16_t *errno);

//
bool file_screenCapture(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                        uint16_t width, uint16_t height, uint16_t handle,
                        uint16_t *status);

//
bool file_putC(const OledConfig *oledConfig, uint16_t car, uint16_t handle,
               uint16_t *status);

//
bool file_getC(const OledConfig *oledConfig, uint16_t handle, uint16_t *car);

//
bool file_putW(const OledConfig *oledConfig, uint16_t word, uint16_t handle,
               uint16_t *status);

//
bool file_getW(const OledConfig *oledConfig, uint16_t handle, uint16_t *word);

//
bool file_putS(const OledConfig *oledConfig, const char *cstr, uint16_t *count);

//
bool file_getS(const OledConfig *oledConfig, uint16_t size, uint16_t handle,
               uint16_t *n, char *str);

//
bool file_erase(const OledConfig *oledConfig, const char *filename,
                uint16_t *status);

//
bool file_rewind(const OledConfig *oledConfig, uint16_t handle,
                 uint16_t *status);

//
bool file_loadFunction(const OledConfig *oledConfig, const char *filename,
                       uint16_t *pointer);

//
bool file_callFunction(const OledConfig *oledConfig, uint16_t handle,
                       uint16_t n, const uint16_t *args, uint16_t *value);

//
bool file_run(const OledConfig *oledConfig, const char *filename, uint16_t n,
              const uint16_t *args, uint16_t *value);

//
bool file_exec(const OledConfig *oledConfig, const char *filename, uint16_t n,
               const uint16_t *args, uint16_t *value);

//
bool file_loadImageControl(const OledConfig *oledConfig, const char *filename1,
                           const char *filename2, uint16_t mode,
                           uint16_t *handle);

//
bool file_mount(const OledConfig *oledConfig, uint16_t *status);

//
bool file_unmount(const OledConfig *oledConfig);

//
bool file_playWAV(const OledConfig *oledConfig, const char *filename,
                  uint16_t *status);

//
bool file_writeString(const OledConfig *oledConfig, uint16_t handle,
                      const char *cstr, uint16_t *pointer);

// level is 0 – 127
//
bool snd_volume(const OledConfig *oledConfig, uint16_t level);

// rate is 4000 – 65535
//
bool snd_pitch(const OledConfig *oledConfig, uint16_t rate, uint16_t *oldRate);

// 0 .. 2
//
bool snd_bufSize(const OledConfig *oledConfig, uint16_t bufferSize);

//
bool snd_stop(const OledConfig *oledConfig);

//
bool snd_pause(const OledConfig *oledConfig);

//
bool snd_continue(const OledConfig *oledConfig);

//
bool snd_playing(const OledConfig *oledConfig, uint16_t *togo);

//
bool img_setPosition(const OledConfig *oledConfig, uint16_t handle,
                     int16_t index, uint16_t xpos, uint16_t ypos,
                     uint16_t *status);

//
bool img_enable(const OledConfig *oledConfig, uint16_t handle, int16_t index,
                uint16_t *status);

//
bool img_disable(const OledConfig *oledConfig, uint16_t handle, int16_t index,
                 uint16_t *status);

//
bool img_darken(const OledConfig *oledConfig, uint16_t handle, int16_t index,
                uint16_t *status);

//
bool img_lighten(const OledConfig *oledConfig, uint16_t handle, int16_t index,
                 uint16_t *status);

//
bool img_setWord(const OledConfig *oledConfig, uint16_t handle, int16_t index,
                 uint16_t offset, uint16_t value, uint16_t *status);

//
bool img_getWord(const OledConfig *oledConfig, uint16_t handle, int16_t index,
                 uint16_t offset, uint16_t *value);

//
bool img_show(const OledConfig *oledConfig, uint16_t handle, int16_t index,
              uint16_t *status);

//
bool img_setAttributes(const OledConfig *oledConfig, uint16_t handle,
                       int16_t index, uint16_t value, uint16_t *status);

//
bool img_clearAttributes(const OledConfig *oledConfig, uint16_t handle,
                         int16_t index, uint16_t value, uint16_t *status);

//
bool img_touched(const OledConfig *oledConfig, uint16_t handle, int16_t index,
                 int16_t *value);

//
bool img_blitComtoDisplay(const OledConfig *oledConfig, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height,
                          const uint16_t *data);

//
bool bus_in(const OledConfig *oledConfig, uint16_t *busState);

//
bool bus_out(const OledConfig *oledConfig, uint16_t busState);

//
bool bus_read(const OledConfig *oledConfig, uint16_t *busState);

//
bool bus_set(const OledConfig *oledConfig, uint16_t dirMask);

//
bool bus_write(const OledConfig *oledConfig, uint16_t bitfield);

//
bool pin_hi(const OledConfig *oledConfig, uint16_t pin, uint16_t *status);

//
bool pin_lo(const OledConfig *oledConfig, uint16_t pin, uint16_t *status);

//
bool pin_read(const OledConfig *oledConfig, uint16_t pin, uint16_t *status);

// Pin=1..7 mode = 0 .. 1 (see reference manuel)
//
bool pin_set_picaso(const OledConfig *oledConfig, uint16_t mode, uint16_t pin,
                    uint16_t *status);

// Pin = 1..16 mode = 0..6 (see reference manuel)
//
bool pin_set_diablo(const OledConfig *oledConfig, uint16_t mode, uint16_t pin,
                    uint16_t *status);
