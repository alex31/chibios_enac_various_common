#pragma once
#include <ch.h>
#include <hal.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FdsDriver FdsDriver;

// line : Holds a positive value for the required line position. column :Holds a
// positive value for the required column position. Moves the text cursor to a
// screen position set by line and column parameters. The line and column
// position is calculated, based on the size and scaling factor for the
// currently selected font. When text is outputted to screen it will be
// displayed from this position. The text position could also be set with “Move
// Origin” command if required to set the text position to an exact pixel
// location. Note that lines and columns start from 0, so line 0, column 0 is
// the top left corner of the display. Available on  FDS_GOLDELOX, FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI,
bool txt_moveCursor(const FdsDriver *fds, uint16_t line, uint16_t column);

// car : Holds a positive value for the required character.
// Prints a single character to the display
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool txt_putCh(const FdsDriver *fds, uint16_t car);

// cstr : Holds a Null terminated string. NOTE: Maximum characters in the string
// is 255 for GOLDELOX or 511 for PICASO and DIABLO16 length : Length of the
// string printed Prints a string to the display and returns the pointer to the
// string. Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
// argument(s) length not returned if screen is of goldelox type
bool txt_putStr(const FdsDriver *fds, const char *cstr, uint16_t *length);

// car : The ASCII character for the width calculation.
// width : Width of a single character in pixel units.
// Calculate the width in pixel units for a character, based on the currently
// selected font. The font can be proportional or mono-spaced. If the total
// width of the character exceeds 255 pixel units, the function will return the
// 'wrapped' (modulo 256) value. Available on  FDS_GOLDELOX, FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI,
bool txt_charWidth(const FdsDriver *fds, char car, uint16_t *width);

// car : The ASCII character for the height calculation.
// height : Height of a single character in pixel units.
// Calculate the height in pixel units for a character, based on the currently
// selected font. The font can be proportional or mono-spaced. If the total
// height of the character exceeds 255 pixel units, the function will return the
// 'wrapped' (modulo 256) value. Available on  FDS_GOLDELOX, FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI,
bool txt_charHeight(const FdsDriver *fds, char car, uint16_t *height);

// Colour : colour to be set.
// oldCol : Previous Text Foreground Colour.
// Sets the text foreground colour.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
// argument(s) oldCol not returned if screen is of goldelox type
bool txt_fgColour(const FdsDriver *fds, uint16_t colour, uint16_t *oldCol);

// Colour : colour to be set.
// oldCol : Previous Text Background Colour.
// Sets the text background colour.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
// argument(s) oldCol not returned if screen is of goldelox type
bool txt_bgColour(const FdsDriver *fds, uint16_t colour, uint16_t *oldCol);

// Id : 0 for System font (Default Fonts), 7 for Media fonts
// oldFont : Previous Font ID.
// Sets the required font using its ID.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
// argument(s) oldFont not returned if screen is of goldelox type
bool txt_fontID(const FdsDriver *fds, uint16_t id, uint16_t *oldFont);

// wMultiplier : Width multiplier, 1 to 16 (Default =1)
// oldMul : Previous Multiplier value.
// Sets the text width multiplier between 1 and 16.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
// argument(s) oldMul not returned if screen is of goldelox type
bool txt_widthMult(const FdsDriver *fds, uint16_t wMultiplier,
                   uint16_t *oldMul);

// hMultiplier : Height multiplier, 1 to 16 (Default =1)
// oldMul : Previous Multiplier value.
// Sets the text height multiplier between 1 and 16.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
// argument(s) oldMul not returned if screen is of goldelox type
bool txt_heightMult(const FdsDriver *fds, uint16_t hMultiplier,
                    uint16_t *oldMul);

// xGap : pixelcount 0 to 32(Default =0)
// oldGap : Previous pixelcount value.
// Sets the pixel gap between characters (x-axis), where the gap is in pixel
// units. Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
// argument(s) oldGap not returned if screen is of goldelox type
bool txt_xgap(const FdsDriver *fds, uint16_t xGap, uint16_t *oldGap);

// yGap : pixelcount 0 to 32(Default =0)
// oldGap : Previous pixelcount value.
// Sets the pixel gap between characters (y-axis), where the gap is in pixel
// units. Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
// argument(s) oldGap not returned if screen is of goldelox type
bool txt_ygap(const FdsDriver *fds, uint16_t yGap, uint16_t *oldGap);

// mode : 1 for ON, 0 for OFF.
// oldBold : Previous Bold status.
// Sets the Bold attribute for the text.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
// argument(s) oldBold not returned if screen is of goldelox type
bool txt_bold(const FdsDriver *fds, uint16_t mode, uint16_t *oldBold);

// mode : 1 for ON, 0 for OFF.
// oldInv : Previous ‘Text Inverse’ status.
// Sets the text to be inverse.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
// argument(s) oldInv not returned if screen is of goldelox type
bool txt_inverse(const FdsDriver *fds, uint16_t mode, uint16_t *oldInv);

// mode : 1 for ON, 0 for OFF.
// oldItal : Previous Italic Text status.
// Sets the text to italic.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
// argument(s) oldItal not returned if screen is of goldelox type
bool txt_italic(const FdsDriver *fds, uint16_t mode, uint16_t *oldItal);

// mode : 1 for ON. (Opaque), 0 for OFF. (Transparent)
// oldOpa : Previous Text Opacity status.
// Selects whether or not the 'background' pixels are drawn (Default mode is
// OPAQUE with BLACK background). Available on  FDS_GOLDELOX, FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI, argument(s) oldOpa not returned if screen is of
// goldelox type
bool txt_opacity(const FdsDriver *fds, uint16_t mode, uint16_t *oldOpa);

// mode : 1 for ON, 0 for OFF.
// oldUnder : Previous Text Underline status.
// Sets the text to underlined.vThe ‘Text Underline’ attribute is cleared
// internally once the text (character or string) is displayed. Note: The “Text
// Y-gap” command is required to be at least 2 for the underline to be visible.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
// argument(s) oldUnder not returned if screen is of goldelox type
bool txt_underline(const FdsDriver *fds, uint16_t mode, uint16_t *oldUnder);

// bitfield : bold=1<<4, italic=1<<5, inverse=1<<6, underlined=1<<7 (bits 0-3
// and 8-15 are reserved) oldAttr : Previous Text Attributes status. Controls
// the following functions grouped : Text Bold, Text Italic, Text Inverse, Text
// Underlined. The Attributes are set to normal internally once the text
// (character or string) is displayed. Available on  FDS_GOLDELOX, FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI, argument(s) oldAttr not returned if screen is of
// goldelox type
bool txt_attributes(const FdsDriver *fds, uint16_t bitfield, uint16_t *oldAttr);

// function : 7 = Sets the Delay between the characters being printed through
// Put Character or Put String functions, value = 0(Default) to 255 (ms) Sets
// various parameters for the Text commands. Available on  FDS_GOLDELOX,
bool txt_set(const FdsDriver *fds, uint16_t function, uint16_t value);

// xpos : 0 for OFF (Default), 1 to N for Wrap position in Pixels.
// oldWrap : Previous wrap position
// Sets the pixel position where text wrap will occur at RHS. The feature
// automatically resets when screen mode is changed. Available on  FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI,
bool txt_wrap(const FdsDriver *fds, uint16_t xpos, uint16_t *oldWrap);

// Clears the screen using the current background colour. Brings some of the
// settings back to default such as Outline colour set to BLACK, Pen set to
// OUTLINE, Text magnifications set to 1, All origins set to 0:0. Available on
// FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_cls(const FdsDriver *fds);

// oldColor : sample colour to be changed within the clipping window, newColor :
// new colour to change all occurrences of old colour within the clipping window
// Changes all oldColour pixels to newColour within the clipping window area.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_changeColour(const FdsDriver *fds, uint16_t oldColour,
                      uint16_t newColour);

// x, y : centre of the circle, radius : radius of the circle, colour : colour
// of the circle. Draws a circle with centre point and radius using the
// specified colour. Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool gfx_circle(const FdsDriver *fds, uint16_t x, uint16_t y, uint16_t radius,
                uint16_t colour);

// x, y : centre of the circle, radius : radius of the circle, colour : colour
// of the circle. Draws a solid circle with centre point and radius using the
// specified colour. Outline colour can be specified with the “Outline Colour”
// command, if set to 0, no outline is drawn. Available on  FDS_GOLDELOX,
// FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_circleFilled(const FdsDriver *fds, uint16_t x, uint16_t y,
                      uint16_t radius, uint16_t colour);

// x1,y1 : starting coordinates of the line, x2,y2 : ending coordinates of the
// line, colour : colour of the line. Draws a line from x1,y1 to x2,y2 using the
// specified colour. The line is drawn using the current object colour. The
// current origin is not altered. The line may be tessellated with the “Line
// Pattern” command. Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool gfx_line(const FdsDriver *fds, uint16_t x1, uint16_t y1, uint16_t x2,
              uint16_t y2, uint16_t colour);

// tlx,tly : top left corner of the rectangle, brx,bry : bottom right corner of
// the rectangle, colour : colour of the rectangle. Draws a rectangle from x1,
// y1 to x2, y2 using the specified colour. The line may be tessellated with the
// “Line Pattern” command. Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool gfx_rectangle(const FdsDriver *fds, uint16_t tlx, uint16_t tly,
                   uint16_t brx, uint16_t bry, uint16_t colour);

// tlx,tly : top left corner of the rectangle, brx,bry : bottom right corner of
// the rectangle, colour : colour of the rectangle. Draws a Filled rectangle
// from x1, y1 to x2, y2 using the specified colour. The line may be tessellated
// with the “Line Pattern” command. Outline colour can be specified with the
// “Outline Colour” command, if set to 0, no outline is drawn. Available on
// FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_rectangleFilled(const FdsDriver *fds, uint16_t tlx, uint16_t tly,
                         uint16_t brx, uint16_t bry, uint16_t colour);

// n : number of elements in the x and y arrays specifying the vertices for the
// polyline, vx[n] : array of elements for the x coordinates of the vertices,
// vy[n] : array of elements for the y coordinates of the vertices, colour :
// colour of the polyline. Plots lines between points specified by a pair of
// arrays using the specified colour. The lines may be tessellated with the
// “Line Pattern” command Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool gfx_polyline(const FdsDriver *fds, uint16_t n, const uint16_t vx[],
                  const uint16_t vy[], uint16_t color);

// n : number of elements in the x and y arrays specifying the vertices for the
// polygon, vx[n] : array of elements for the x coordinates of the vertices,
// vy[n] : array of elements for the y coordinates of the vertices, colour :
// colour of the polygon. Plots lines between points specified by a pair of
// arrays using the specified colour. The last point is drawn back to the first
// point, completing the polygon. The lines may be tessellated with “Line
// Pattern” command. Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool gfx_polygon(const FdsDriver *fds, uint16_t n, const uint16_t vx[],
                 const uint16_t vy[], uint16_t color);

// n : number of elements in the x and y arrays specifying the vertices for the
// polygon, vx[n] : array of elements for the x coordinates of the vertices,
// vy[n] : array of elements for the y coordinates of the vertices, colour :
// colour of the polygon. Draws a solid Polygon between specified vertices using
// the specified colour. The last point is drawn back to the first point,
// completing the polygon. Vertices must be a minimum of 3 and can be specified
// in any fashion. Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_polygonFilled(const FdsDriver *fds, uint16_t n, const uint16_t vx[],
                       const uint16_t vy[], uint16_t color);

// x1,y1 : first vertice of the triangle, x2,y2 : second vertice of the
// triangle, x3,y3 : third vertice of the triangle, colour : colour of the
// triangle. Draws a triangle outline between specified vertices using the
// specified colour. The line may be tessellated with the “Line Pattern”
// command. Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_triangle(const FdsDriver *fds, uint16_t x1, uint16_t y1, uint16_t x2,
                  uint16_t y2, uint16_t x3, uint16_t y3, uint16_t colour);

// x1,y1 : first vertice of the triangle, x2,y2 : second vertice of the
// triangle, x3,y3 : third vertice of the triangle, colour : colour of the
// triangle. Draws a solid triangle between specified vertices using the
// specified colour. Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_triangleFilled(const FdsDriver *fds, uint16_t x1, uint16_t y1,
                        uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
                        uint16_t colour);

// angle : angle from the origin to the remote point. The angle is specified in
// degrees, distance : distance from the origin to the remote point in pixel
// units. Xdist,Ydist : coordinates from the current origin. Calculates the
// coordinates of a distant point relative to the current origin, where the only
// known parameters are the angle and the distance from the current origin.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_orbit(const FdsDriver *fds, uint16_t angle, uint16_t distance,
               uint16_t *Xdist, uint16_t *Ydist);

// x, y : pixel coordinates, colour : colour of the pixel
// Draws a pixel at specified coordinates using the specified colour.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_putPixel(const FdsDriver *fds, uint16_t x, uint16_t y,
                  uint16_t colour);

// x, y : pixel coordinates
// colour : 16bit colour of the pixel.
// Reads the colour value of the pixel at the specified coordinate.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_getPixel(const FdsDriver *fds, uint16_t x, uint16_t y,
                  uint16_t *colour);

// x,y : coordinates of the new origin.
// Moves the origin to a new position.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_moveTo(const FdsDriver *fds, uint16_t x, uint16_t y);

// x,y : line end coordinates.
// Draws a line from the current origin to a new position. The Origin is then
// set to the new position. The line is drawn using the current object colour,
// using the “Set Graphics Parameters” – “Object Colour” command. The line may
// be tessellated with the “Line Pattern” command. Available on  FDS_GOLDELOX,
// FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_lineTo(const FdsDriver *fds, uint16_t x, uint16_t y);

// mode : 0 = Clipping Disabled, 1 = Clipping Enabled
// Enables or Disables the ability for Clipping to be used.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_clipping(const FdsDriver *fds, uint16_t mode);

// tlx,tly :  top left corner of the clipping window, brx,bry : bottom right
// corner of the clipping window. Specifies a clipping window region on the
// screen such that any objects and text placed onto the screen will be clipped
// and displayed only within that region. For the clipping window to take
// effect, the clipping setting must be enabled separately using the “Clipping”
// command Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_clipWindow(const FdsDriver *fds, uint16_t tlx, uint16_t tly,
                    uint16_t brx, uint16_t bry);

// Forces the clip region to the extent of the last text that was printed, or
// the last image that was shown. Available on  FDS_GOLDELOX, FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI,
bool gfx_setClipRegion(const FdsDriver *fds);

// x,y : center of ellipse, xrad : x-radius of ellipse, yrad : y-radius of
// ellipse, colour : colour of the ellipse. Plots a coloured Ellipse on the
// screen at centre x, y with x-radius = xrad and y-radius = yrad. Available on
// FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_ellipse(const FdsDriver *fds, uint16_t x, uint16_t y, uint16_t xrad,
                 uint16_t yrad, uint16_t colour);

// x,y : center of ellipse, xrad : x-radius of ellipse, yrad : y-radius of
// ellipse, colour : colour of the ellipse. Plots a solid coloured Ellipse on
// the screen at centre x, y with x-radius = xrad and y-radius = yrad. Available
// on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_ellipseFilled(const FdsDriver *fds, uint16_t x, uint16_t y,
                       uint16_t xrad, uint16_t yrad, uint16_t colour);

// "state : appearance of button, 0 = depressed
//  1 = raised, x,y : top left corner position, buttoncolour : button colour,
//  txtcolour : text colour, font : font ID, txtWidth : text width (font width
//  multiplier, minimum must be 1) , txtHeight : teext height (font width
//  multiplier, minimum must be 1), cstr : text string, printable ASCII
//  character set Null terminated. May have \n characters to create a multiline
//  button." Draws a 3 dimensional Text Button at location defined by
//  parameters. Size depends on font, width, height and length of text. Can
//  contain multiple lines of text by having \n character in the string. In this
//  case, the widest text in the string sets the overall width, and height is
//  set by the number of text lines. In case of multiple lines, each line is
//  left justified. To centre or right justify text, prepare string according to
//  requirements.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_button(const FdsDriver *fds, uint16_t state, uint16_t x, uint16_t y,
                uint16_t buttoncolour, uint16_t txtcolour, uint16_t font,
                uint16_t txtWidth, uint16_t txtHeight, const char *cstr);

// "state : appearance of panel, 0 = recessed
//  1 = raised, x,y : top left corner position of the panel, width : width of
//  the panel ,height : height of the panel, colour : colour of the panel."
// Draws a 3 dimensional rectangular panel
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_panel(const FdsDriver *fds, uint16_t state, uint16_t x, uint16_t y,
               uint16_t width, uint16_t height, uint16_t colour);

// mode : 0 = Indented, 1 = Raised, 2 = Hidden (background colour), tlx,tly :
// top left corner position, brx,bry : bottom right corner position ,colour :
// colour of the slider, scale : full scale range of the slider for the thumb (0
// to n), value : relative position of the thumb on the slider bar Draws a
// vertical or horizontal slider bar. Mode operation rules : if width > height
// slider is assumed to be horizontal, if height <= width slider is assumed to
// be vertical, If value parameter is positive, thumb is set to the position
// that is the proportion of value to the scale parameter. If value parameter is
// negative, thumb is driven to the graphics position set by the ABSolute of
// value. Thumb colour is determine by the “Set Graphics Parameters” – “Object
// Colour” command, however, if the current object colour is BLACK, a darkened
// shade of the colour parameter is used for the thumb . Available on
// FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_slider(const FdsDriver *fds, uint16_t mode, uint16_t tlx, uint16_t tly,
                uint16_t brx, uint16_t bry, uint16_t colour, uint16_t scale,
                int16_t value);

// xs, ys : position of the top left corner of the area to be copied (source),
// xd, yd : position of the top left corner of where the paste is to be made
// (destination), width : width of the copied area, height : height of the
// copied area. Copies an area of the screen from xs, ys of size given by width
// and height parameters and pastes it to another location determined by xd, yd.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_screenCopyPaste(const FdsDriver *fds, uint16_t xs, uint16_t ys,
                         uint16_t xd, uint16_t yd, uint16_t width,
                         uint16_t height);

// value : number of pixel, 0 = No Bevel Shadow, 1-4 = Number of Pixels Deep
// (default = 3) oldBevel : previous bevel shadow status. Changes the graphics
// “Draw Button” commands bevel shadow depth Available on  FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI,
bool gfx_bevelShadow(const FdsDriver *fds, uint16_t value, uint16_t *oldBevel);

// value : 0 = No Bevel, 1-15 = Number of Pixels Wide (Default = 2)
// oldWidth : previous bevel width status.
// Changes the graphics “Draw Button” commands bevel width
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_bevelWidth(const FdsDriver *fds, uint16_t value, uint16_t *oldWidth);

// colour : colour to be set (0-65535 or HEX 0x0000-0xFFFF)
// oldCol : previous background Colour.
// Sets the screen background colour
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
// argument(s) oldCol not returned if screen is of goldelox type
bool gfx_bgCcolour(const FdsDriver *fds, uint16_t colour, uint16_t *oldCol);

// colour : colour to be set (0-65535 or HEX 0x0000-0xFFFF), set to 0 for no
// effect oldCol : previous outline Colour. Sets the outline colour for
// rectangles and circles. Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI, argument(s) oldCol not returned if screen is of goldelox type
bool gfx_outlineColour(const FdsDriver *fds, uint16_t colour, uint16_t *oldCol);

// contrast : 0 = Display OFF, 1 - 15 = Contrast Level
// oldContrast : previous Contrast value.
// Sets the contrast of the display, or turns it On/Off depending on display
// model Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
// argument(s) oldContrast not returned if screen is of goldelox type
bool gfx_contrast(const FdsDriver *fds, uint16_t contrast,
                  uint16_t *oldContrast);

// delayMsec : delay in milliseconds (0-255)
// oldDelay : previous Frame Delay value.
// Sets the inter frame delay for the “Media Video” command
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
// argument(s) oldDelay not returned if screen is of goldelox type
bool gfx_frameDelay(const FdsDriver *fds, uint16_t delayMsec,
                    uint16_t *oldDelay);

// pattern : 0 = all line pixels are on (Default), 0-65535 (or HEX
// 0x0000-0xFFFF) = number of bits in the line are turned off to form a pattern
// oldPattern : previous Line Pattern value.
// Sets the line draw pattern for line drawing. If set to zero, lines are solid,
// else each '1' bit represents a pixel that is turned off. Available on
// FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI, argument(s) oldPattern not
// returned if screen is of goldelox type
bool gfx_linePattern(const FdsDriver *fds, uint16_t pattern,
                     uint16_t *oldPattern);

// mode : 0 = Landscape, 1 = Landscape reverse, 2 = Portrait, 3 = Portrait
// reverse oldMode : previous Screen Mode value. Alters the graphics orientation
// (Landscape, Landscape reverse, Portrait or Portrait reverse) Available on
// FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI, argument(s) oldMode not
// returned if screen is of goldelox type
bool gfx_screenMode(const FdsDriver *fds, uint16_t mode, uint16_t *oldMode);

// mode : 0 = Transparency OFF, 1 = Transparency ON
// oldMode : previous Transparency value.
// Turns the transparency ON or OFF. Transparency is automatically turned OFF
// after the next image or video command. Available on  FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI,
bool gfx_transparency(const FdsDriver *fds, uint16_t mode, uint16_t *oldMode);

// colour : colour to make transparent (0-65535 or HEX 0x0000-0xFFFF)
// oldColor : previous Transparent Colour value.
// Alters the colour that needs to be made transparent.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_transparentColour(const FdsDriver *fds, uint16_t colour,
                           uint16_t *oldColor);

// function, value : see « Serial Commands Reference Manual » of appropriate
// display processor Sets various parameters for the graphics gommands.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool gfx_set(const FdsDriver *fds, uint16_t function, uint16_t value);

// mode : 0 = Current orientations maximum X value (X_MAX), 1 = Current
// orientations maximum Y value (Y_MAX), 2 = Left location of last Object, 3 =
// Top location of Object, 4 = Right location of last Object, 5 = Bottom
// location of Object value : Mode=0: horizontal resolution - 1, Mode=1:
// vertical resolution - 1, Mode=2: left location of the last drawn object,
// Mode=3: top location of the last drawn object, Mode=4: right location of the
// last drawn object, Mode=5: bottom location of the last drawn object. Returns
// various graphics parameters to the caller. Available on  FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI,
bool gfx_get(const FdsDriver *fds, uint16_t mode, uint16_t *value);

// handle: A pointer to the memory block for widget variable utilization,
// params: A pointer to the memory block holding the widget parameters The Draw
// Scale command draws the Scale PmmC widget on the screen. Available on
// FDS_PIXXI,
bool gfx_scale(const FdsDriver *fds, uint16_t handle, uint16_t param);

// options: Bevel direction (0 – Inwards, 1 – Outwards) Additional bit for
// filling panel with fill color (0x8000 - PANEL2_FILLED), x & y: Specifies the
// Top-Left X-position, Top-Left Y-position, width: Specifies the panel width,
// height: Specifies the panel height, width1: Outer bevel offset, width2: Inner
// bevel offset, maincolour: Main bevel colour, shadowcolour: Shadow bevel
// colour, fcolour: Fill colour
//  The Draw Panel 2 command draws the Panel 2 PmmC widget on the screen.
// Available on  FDS_PIXXI,
bool gfx_pannel2(const FdsDriver *fds, uint16_t options, uint16_t x, uint16_t y,
                 uint16_t width, uint16_t height, uint16_t width1,
                 uint16_t width2, uint16_t maincolour, uint16_t shadowcolour,
                 uint16_t fcolour);

// value: A value (usually a constant) specifying the current frame of the
// widget, handle: A pointer to the memory block for widget variable
// utilization, params: A pointer to the memory block holding the widget
// parameters The Draw Button4 command draws the Button4 PmmC widget on the
// screen. Available on  FDS_PIXXI,
bool gfx_button4(const FdsDriver *fds, uint16_t value, uint16_t handle,
                 uint16_t params);

// value: A value (usually a constant) specifying the current frame of the
// widget, handle: A pointer to the memory block for widget variable
// utilization, params: A pointer to the memory block holding the widget
// parameters The Draw Switch command draws the Switch PmmC widget on the
// screen. Available on  FDS_PIXXI,
bool gfx_switch(const FdsDriver *fds, uint16_t value, uint16_t handle,
                uint16_t params);

// value: A value (usually a constant) specifying the current frame of the
// widget, handle: A pointer to the memory block for widget variable
// utilization, params: A pointer to the memory block holding the widget
// parameters The Draw Slider5 command draws the Slider5 PmmC widget on the
// screen. Available on  FDS_PIXXI,
bool gfx_slider5(const FdsDriver *fds, uint16_t value, uint16_t handle,
                 uint16_t params);

// value: A value (usually a constant) specifying the current frame of the
// widget, handle: A pointer to the memory block for widget variable
// utilization, params: A pointer to the memory block holding the widget
// parameters The Draw Dial command draws the Dial PmmC widget on the screen.
// Available on  FDS_PIXXI,
bool gfx_dial(const FdsDriver *fds, uint16_t value, uint16_t handle,
              uint16_t params);

// value: A value (usually a constant) specifying the current frame of the
// widget, handle: A pointer to the memory block for widget variable
// utilization, params: A pointer to the memory block holding the widget
// parameters The Draw Led command draws the Led PmmC widget on the screen.
// Available on  FDS_PIXXI,
bool gfx_led(const FdsDriver *fds, uint16_t value, uint16_t handle,
             uint16_t params);

// value: A value (usually a constant) specifying the current frame of the
// widget, handle: A pointer to the memory block for widget variable
// utilization, params: A pointer to the memory block holding the widget
// parameters The Draw Gauge command draws the Gauge PmmC widget on the screen.
// Available on  FDS_PIXXI,
bool gfx_gauge(const FdsDriver *fds, uint16_t value, uint16_t handle,
               uint16_t params);

// value: A value (usually a constant) specifying the current frame of the
// widget, handle: A pointer to the memory block for widget variable
// utilization, params: A pointer to the memory block holding the widget
// parameters The Draw Angular Meter command draws the Angular Meter PmmC widget
// on the screen. Available on  FDS_PIXXI,
bool gfx_angularMeter(const FdsDriver *fds, uint16_t value, uint16_t handle,
                      uint16_t params);

// x & y: Specifies the top left corner position of the digit on the screen,
// digitSize: Specifies the size of the digit, onColour: Specifies the colour of
// the activate LED segments, offColour: Specifies the colour of the inactive
// LED segments, value: Value to be displayed by the widget The Draw Led Digit
// command draws the Led Digit PmmC widget on the screen. Available on
// FDS_PIXXI,
bool gfx_ledGigit(const FdsDriver *fds, uint16_t x, uint16_t y,
                  uint16_t digitSize, uint16_t onColour, uint16_t offColour,
                  uint16_t value);

// value: A value (usually a constant) specifying the current frame of the
// widget, handle: A pointer to the memory block for widget variable
// utilization, params: A pointer to the memory block holding the widget
// parameters The Draw Led Digits command draws the Led Digits PmmC widget on
// the screen. Available on  FDS_PIXXI,
bool gfx_ledDigits(const FdsDriver *fds, uint16_t value, uint16_t handle,
                   uint16_t params);

// value : Non-Zero = card is present and successfully initialised, 0 = no card
// is present or not able to initialise. Initialises a uSD/SD/SDHC memory card
// for further operations. SD card is connected to the SPI of the processor
// display. Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool media_init(const FdsDriver *fds, uint16_t *value);

// hiAddr : high word (upper 2 bytes) of a 4 byte media memory byte address
// location, loAddr : low word (lower 2 bytes) of a 4 byte media memory byte
// address location. Sets the media memory internal Address pointer for access
// at a non-sector aligned byte address. Available on  FDS_GOLDELOX, FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI,
bool media_setAdd(const FdsDriver *fds, uint16_t hiAddr, uint16_t loAddr);

// hiAddr : high word (upper 2 bytes) of a 4 byte media memory byte address
// location, loAddr : low word (lower 2 bytes) of a 4 byte media memory byte
// address location. Sets the media memory internal Address pointer for sector
// access. Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool media_setSector(const FdsDriver *fds, uint16_t hiAddr, uint16_t loAddr);

// value : byte value in the LSB.
// Returns the byte value from the current media address, set by the “Set Byte
// Address” command. The internal byte address will then be internally
// incremented by one. Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool media_readByte(const FdsDriver *fds, uint16_t *value);

// value : word value
// Returns the word value (2 bytes) from the current media address, set by the
// “Set Byte Address” command. The internal byte address will then be internally
// incremented by two. If the address is not aligned, the word will still be
// read correctly. Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool media_readWord(const FdsDriver *fds, uint16_t *value);

// s : byte value, in the LSB, to be written at the current byte address
// location. status : Non Zero = successful media response, 0 = attempt failed.
// Writes a byte to the current media address that was initially set with the
// “Set Sector Address” command. See « Serial Commands Reference Manual » for
// more information. Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool media_writeByte(const FdsDriver *fds, uint16_t s, uint16_t *status);

// value : 16 bit word to be written at the current media address location.
// status : Non Zero = successful media response, 0 = attempt failed.
// Writes a word to the current media address that was initially set with the
// “Set Sector Address” command. See « Serial Commands Reference Manual » for
// more information. Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool media_writeWord(const FdsDriver *fds, uint16_t value, uint16_t *status);

// status : Non Zero = successful media response, 0 = attempt failed.
// After writing any data to a sector, the Flush Media command should be called
// to ensure that the current sector that is being written is correctly stored
// back to the media else write operations may be unpredictable. Available on
// FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool media_flush(const FdsDriver *fds, uint16_t *status);

// x,y : top left position where the image will be displayed.
// Displays an image from the media storage at the specified co-ordinates. The
// image address is previously specified with the “Set Byte Address” command or
// “Set Sector Address” command. If the image is shown partially off screen, it
// may not be displayed correctly. Available on  FDS_GOLDELOX, FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI,
bool media_image(const FdsDriver *fds, uint16_t x, uint16_t y);

// x,y : top left position where the video clip will be displayed.
// Displays a video clip from the media storage device at the specified
// co-ordinates. The video address location in the media is previously specified
// with the “Set Byte Address” or “Set Sector Address” commands. If the video is
// shown partially off screen, it may not be displayed correctly. Note that
// showing a video blocks all other processes until the video has finished
// showing. See the “Display Video Frame” command for alternatives. Available on
// FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool media_video(const FdsDriver *fds, uint16_t x, uint16_t y);

// x,y : top left position of the video frame to be displayed, frameNumber :
// required frame number to be displayed. Displays a video from the media
// storage device at the specified co-ordinates. The video address is previously
// specified with the “Set Byte Address” command or “Set Sector Address”
// command. If the video is shown partially off it may not be displayed
// correctly. The frames can be shown in any order. This function gives a great
// flexibility for showing various icons from an image strip, as well as showing
// videos while doing other tasks. Available on  FDS_GOLDELOX, FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI,
bool media_videoFrame(const FdsDriver *fds, uint16_t x, uint16_t y,
                      int16_t frameNumber);

// eveReg : byte register address
// value : register value in the LSB.
// Returns the EVE System Byte Register value in the lower byte. See EVE System
// Registers Memory Map in « Serial Commands Reference Manual ». Available on
// FDS_GOLDELOX,
bool misc_peekB(const FdsDriver *fds, uint16_t eveReg, uint16_t *value);

// eveReg : byte register address, value : register value in the LSB.
// Sets the EVE System Byte Register value in the lower byte. See EVE System
// Registers Memory Map in « Serial Commands Reference Manual ». Available on
// FDS_GOLDELOX,
bool misc_pokeB(const FdsDriver *fds, uint16_t eveReg, uint16_t value);

// eveReg : word register address
// value : registre value
// Returns the EVE System Word Register value. See EVE System Registers Memory
// Map in « Serial Commands Reference Manual ». Available on  FDS_GOLDELOX,
bool misc_peekW(const FdsDriver *fds, uint16_t eveReg, uint16_t *value);

// eveReg : word register address, value : register value
// Sets the EVE System Word Register value. See EVE System Registers Memory Map
// in « Serial Commands Reference Manual ». Available on  FDS_GOLDELOX,
bool misc_pokeW(const FdsDriver *fds, uint16_t eveReg, uint16_t value);

// address : address to be peeked
// value : contents of the specified memory address.
// Returns the word contents of a specified memory address. This command would
// normally be used to read the contents of File and/or ImageControl handles.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool misc_peekM(const FdsDriver *fds, uint16_t address, uint16_t *value);

// address : address to be poked, value : data to be poked into the address
// Sets the word contents of a specified memory address. This command would
// normally be used to alter the contents of File and/or ImageControl handles.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool misc_pokeM(const FdsDriver *fds, uint16_t address, uint16_t value);

// value : value of the Joystick position, 0=Released, 1=Up , 2=Left , 3=Down ,
// 4=Right, 5=Press (5 position switch implementation). Return the value of the
// joystick position. Refer to the processor datasheet. Available on
// FDS_GOLDELOX,
bool misc_joystick(const FdsDriver *fds, uint16_t *value);

// note : frequency of the note (0-64), duration_ms : time in milliseconds that
// the note will be played for. Produce a single musical note for the required
// duration through IO2. Available on  FDS_GOLDELOX,
bool misc_beep(const FdsDriver *fds, uint16_t note, uint16_t duration_ms);

// index : baud rate index value. See index table in « Serial Commands Reference
// Manual » Set the required baud rate. Please refer to the « Serial Commands
// Reference Manual ». Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool misc_setbaudWait(const FdsDriver *fds, int16_t index);

// duration_s : Sleep duration (0-65535) approximately in seconds (in sleep
// mode, timing is controlled by an RC oscillator, so is not totally accurate)
// ack,duration : remaining time when touch screen is touched, else returns
// zero. "Puts display and processor into low power mode for a period of time.
// If ""duration"" is zero, goes into sleep mode forever and needs power cycling
// to re-initialize. If ""duration"" is 1 to 65535, display will sleep for that
// period of time, or will be woken when touch screen is touched. Returns
// remaining time when screen was touched. When returning from sleep mode,
// display and processor are restored from low power mode." Available on
// FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool sys_sleep(const FdsDriver *fds, uint16_t duration_s, uint16_t *duration);

// handle : pointer to the memory block.
// status : Non Zero = operation successful, 0 = attempt failed.
// "Releases the memory space used by the the ""Load Image Control"" and ""File
// Load Function"" commands." Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool sys_memFree(const FdsDriver *fds, uint16_t handle, uint16_t *status);

// avail : largest available memory chunk of the heap.
// Returns byte size of the largest chunk of memory available in the heap.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool sys_memHeap(const FdsDriver *fds, uint16_t *avail);

// n : number of characters in the model name to return, str[n] : display
// module’s model name (without NULL terminator). Returns the Display Model in
// the form of a string without Null terminator. Available on  FDS_GOLDELOX,
// FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool sys_getModel(const FdsDriver *fds, uint16_t *n, char *str);

// version : SPE Version installed on the module.
// Returns the SPE Version installed on the module.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool sys_getVersion(const FdsDriver *fds, uint16_t *version);

// version : PmmC Version installed on the module.
// Returns the PmmC Version installed on the module.
// Available on  FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool sys_getPmmC(const FdsDriver *fds, uint16_t *version);

// timout_ms : Screen saver timeout in milliseconds (1 to 65535, 0 disables the
// screen saver). Set the screen saver Timeout. Default screen saver timeout
// settings could be adjusted. This feature is display dependent, please refer
// to module's datasheet. Available on  FDS_GOLDELOX,
bool misc_screenSaverTimeout(const FdsDriver *fds, uint16_t timout_ms);

// speed : screen saver speed. Range is display dependant. Please refer to
// module datasheet Set the screen saver speed. Default screen saver speed
// settings could be adjusted. This feature is display dependent, please refer
// to module datasheet. Available on  FDS_GOLDELOX,
bool misc_screenSaverSpeed(const FdsDriver *fds, uint16_t speed);

// mode : screen saver scroll direction. Display dependant, please refer to
// module datasheet. Set the screen saver scroll direction. This feature is
// display dependent, please refer to module's datasheet. Available on
// FDS_GOLDELOX,
bool misc_screenSaverMode(const FdsDriver *fds, uint16_t mode);

// tlx,tly : position of the top left corner of the region, brx,bry : position
// of the bottom right corner of the region. Specifies a touch detect region on
// the screen. This setting will filter out any touch activity outside the
// region and only touch activity within that region will be reported by the
// status poll “Touch Get” command Available on  FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool touch_detectRegion(const FdsDriver *fds, uint16_t tlx, uint16_t tly,
                        uint16_t brx, uint16_t bry);

// mode : 0=Enables and initialises Touch Screen hardware, 1=Disables the Touch
// Screen (Touch Screen task runs in the background, disabling it when not in
// use free up extra CPU cycles), 2 = reset the current active region to default
// (= full screen area) Sets various Touch Screen related parameters. Available
// on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool touch_set(const FdsDriver *fds, uint16_t mode);

// mode : 0 = get Status, 1 = get X coordinates, 2 = get Y coordinates.
// value : mode0 = returns the various states of the touch screen
// (0=invalid/notouch, 1=press, 2=release, 3=moving), mode1 = returns the X
// coordinates of the touch reported by mode0, mode2 = returns the Y coordinates
// of the touch reported by mode0. Returns various touch screen parameters,
// based on the touch detect region set by the “Touch Detect Region” command.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool touch_get(const FdsDriver *fds, uint16_t mode, uint16_t *value);

// errno : error number (0 = no error).
// Returns the most recent error code. See error table in « Serial Commands
// Reference Manual » Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_error(const FdsDriver *fds, uint16_t *errno);

// filename : Name of the file(s) for search (passed as a string). Filename must
// be 8.3 format. count : number of files that match the criteria. Returns
// number of files found that match the criteria. The wild card character '*'
// matches up with any combination of allowable characters and '?' matches up
// with any single allowable character. Available on  FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool file_count(const FdsDriver *fds, const char *filename, uint16_t *count);

// filename : Name of the file(s) for search (passed as a string). Filename must
// be 8.3 format. count : number of files that match the criteria. Lists the
// stream of file names that agree with the search key on the Display Screen.
// The wild card character ‘*’ matches up with any combination of allowable
// characters and ‘?’ matches up with any single allowable character. Note:
// “Find First File and Report” and “Find Next File and Report” are recommended
// alternatives Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_dir(const FdsDriver *fds, const char *filename, uint16_t *count);

// filename : Name of the file(s) for search (passed as a string). Filename must
// be 8.3 format. status : 1 = at least one file exists that satisfies the
// criteria, 0 = no file satisfies the criteria. Returns 1 if at least one file
// exists that satisfies the file argument. Wildcards are usually used so if the
// “Find First File” command returns true, further tests can be made using the
// “Find Next File” command to find all the files that match the wildcard class.
// Notes : filename is printed on the screen. “Find First File and Report” and
// “Find Next File and Report” are recommended alternatives in order to return
// the responses. Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_findFirst(const FdsDriver *fds, const char *filename,
                    uint16_t *status);

// filename : Name of the file(s) for search (passed as a string). Filename must
// be 8.3 format. length : Length of the filename string, str[n] : Filename if
// it exists. Filename string is not NULL terminated. Returns length of filename
// and filename if at least 1 file exists that matches the criteria. Wildcards
// are usually used so further tests can be made using “Find Next File” or “Find
// Next File and Report” commands to find all the files that match the wildcard
// class. Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_findFirstRet(const FdsDriver *fds, const char *filename,
                       uint16_t *length, char *str);

// status : 1 = at least one file exists that satisfies the criteria, 0 = no
// file satisfies the criteria. Returns 1 if more file exists that satisfies the
// file argument that was given for the “Find First File” or “Find First File
// and Report” commands. Wildcards must be used for the “Find First File” or
// “Find First File and Report” commands else this function will always return
// zero as the only occurrence will have already been found. Note : filename is
// printed on the screen. Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_findNext(const FdsDriver *fds, uint16_t *status);

// length : Length of the filename string, str[n] : Filename if it exists.
// Filename string is not NULL terminated. "Returns length of filename and
// filename if at least 1 file exists that matches criteria given for the “Find
// First File” or “Find First File and Report” commands. Wildcards must be used
// for “Find First File” or “Find First File and Report” commands else this
// function will always return zero as the only occurrence will have already
// been found. Wildcards are usually used, so if the “Find First File” or “Find
// First File and Report” commands return the stringlength and filename, further
// tests can be made using ""Find Next File and Report"" command to find all the
// files that match the wildcard class." Available on  FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool file_findNextRet(const FdsDriver *fds, uint16_t *length, char *str);

// filename : Name of the file(s) for search (passed as a string). Filename must
// be 8.3 format. status : 1 = File found, 0 = File not found. Tests for the
// existence of the file provided with the search key. Available on  FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI,
bool file_exists(const FdsDriver *fds, const char *filename, uint16_t *status);

// filename : Name of the file(s) for search (passed as a string). Filename must
// be 8.3 format. Mode : ‘r’ or ‘w’ or ‘a’ (read write, append) handle : return
// handle if file exists, else internal file error is set Returns handle if file
// exists. The file ‘handle’ that is created is now used as reference for
// ‘filename’ for further file commands such as “File Close”, etc. For File
// Write and File Append modes ('w' and 'a') the file is created if it does not
// exist. Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_open(const FdsDriver *fds, const char *filename, char mode,
               uint16_t *handle);

// handle :  The file handle that was created by the “File Open” command which
// is now used as reference ‘handle’ for the filename, for further file
// functions such as in this function to close the file. Status : 1 = file
// closed, 0 = file not closed The File Close command will close the previously
// opened file. Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_close(const FdsDriver *fds, uint16_t handle, uint16_t *status);

// size : Number of bytes to be read. handle : The handle that references the
// file to be read. count : Returns the number of bytes read. data : Data read
// from the file Returns the number of bytes specified by ‘size’ from the file
// referenced by ‘handle’. Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_read(const FdsDriver *fds, uint16_t size, uint16_t handle,
               uint16_t *n, char *str);

// "handle The handle that references the file 
// HiWord Contains the upper 16bits of the memory pointer into the file. LoWord
// Contains the lower 16bits of the memory pointer into the file." Status : 1 =
// success, 0 = error The File Seek command places the file pointer at the
// required position in a file that has been opened in 'r' (read) or 'a'
// (append) mode. Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_seek(const FdsDriver *fds, uint16_t handle, uint16_t hiWord,
               uint16_t loWord, uint16_t *status);

// "Handle : The handle that references the file 
//  HiSize :  Contains the upper 16bits of the size of the file records
//  LoSize : Contains the lower 16bits of the size of the file records 
// recordnum : The index of the required record"
// Status : 1 = success, 0 = error
// Places the file pointer at the position in a file that has been opened in 'r'
// (read) or 'a' (append) mode. In append mode, File Index does not expand a
// filesize, instead, the file pointer (handle) is set to the end position of
// the file, e.g. assuming the record size is 100 bytes, the File Index command
// with HiSize = 0, LoSize = 100 and recordnum = 22 will set the file position
// to 2200 for the file handle, so subsequent data may be read from that
// position onwards with “Read Character from the File”, “Read Word from the
// File”, “Read String from the File” commands or an image can be displayed with
// the “Display Image (FAT)” command. Available on  FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool file_index(const FdsDriver *fds, uint16_t handle, uint16_t hiWord,
                uint16_t loWord, uint16_t recordNum, uint16_t *status);

// Handle : The handle that references the file
// "Status : 1 = success, 0 = error 
//  HiWord Contains the upper 16bits of the value of the pointer 
//  LoWord  Contains the lower 16bits of the value of the pointer"
// The File Tell command returns the current value of the file pointer.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_tell(const FdsDriver *fds, uint16_t handle, uint16_t *status,
               uint16_t *hiWord, uint16_t *loWord);

// size : Number of bytes to be written. Maximum that can be written at one time
// is 512 bytes.source :  String of Data without Null terminator. Handle :  The
// handle that references the file to write. count  Returns the number of bytes
// written. write chunk of data to file Available on  FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool file_write(const FdsDriver *fds, uint16_t size, const uint8_t *source,
                uint16_t handle, uint16_t *count);

// Handle : The handle that references the file
// "Status : 1 = success, 0 = error 
//  HiWord Contains the upper 16bits of the file size 
//  LoWord  Contains the lower 16bits of the file size"
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_size(const FdsDriver *fds, uint16_t handle, uint16_t *status,
               uint16_t *hiWord, uint16_t *loWord);

// "x : X-position of the image to be displayed 
//  y : Y-position of the image to be displayed 
//  handle : The handle that references the file containing the image(s)."
// error  Returns a copy of the File Error, see the “File Error” command
// Display an image from the file stream at screen location specified by x, y
// (top left corner).If there is more than 1 image in the file, it can be
// accessed with the “File Seek” command Available on  FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool file_image(const FdsDriver *fds, uint16_t x, uint16_t y, uint16_t handle,
                uint16_t *errno);

// "X : X-position of the image to be captured 
//  y :  Y-position of the image to be captured 
//  width : Width of the area to be captured 
// height : Height of the area to be captured 
//  handle : The handle that references the file to store the image(s)"
// status :  0: If the operation was successful
// "The Screen Capture command saves an image of the screen shot to file at the
// current file position.The image can later be displayed with the “Display
// Image (FAT)” command. The file may beopened in append mode to accumulate
// multiple images. Later, the images can be displayed with the “File Seek”
// command. The image is saved from x, y (with respect to top left corner),and
// the capture area is determined by ""width"" and ""height""." Available on
// FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_screenCapture(const FdsDriver *fds, uint16_t x, uint16_t y,
                        uint16_t width, uint16_t height, uint16_t handle,
                        uint16_t *status);

// car: data byte (in the LSB) about to be written, handle: The handle that
// references the file to be written to. status: returns the number of bytes
// written successfully "This function writes the byte specified by ""char"" to
// the file, at the position indicated by the associated file-position pointer
// (set by the “File Seek” or “File Index” commands) and advances the pointer
// appropriately (incremented by 1). The file must be previously opened with 'w'
// (write) or 'a' (append) modes." Available on  FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool file_putC(const FdsDriver *fds, uint16_t car, uint16_t handle,
               uint16_t *status);

// handle: the handle that references the file to be read from
// car: returns the data byte read from the file in the LSB
// The Read Character from the File command reads a byte from the file, at the
// position indicated by the associated file-position pointer (set by the “File
// Seek” or “File Index”commands) and advances the pointer appropriately
// (incremented by 1). The file must be previously opened with 'r' (read) mode.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_getC(const FdsDriver *fds, uint16_t handle, uint16_t *car);

// word: word about to be written, handle: The handle that references the file
// to be written to. status: returns the number of bytes written successfully
// "This function writes the word specified by ""word"" to the file, at the
// position indicated by the associated file-position pointer (set by the “File
// Seek” or “File Index” commands) and advances the pointer appropriately
// (incremented by 2). The file must be previously opened with 'w' (write) or
// 'a' (append) modes." Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_putW(const FdsDriver *fds, uint16_t word, uint16_t handle,
               uint16_t *status);

// handle: the handle that references the file to be read from
// word: returns the word read from the file in the LSB
// The Read Word from the File command reads a word from the file, at the
// position indicated by the associated file-position pointer (set by the “File
// Seek” or “File Index”commands) and advances the pointer appropriately
// (incremented by 2). The file must be previously opened with 'r' (read) mode.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_getW(const FdsDriver *fds, uint16_t handle, uint16_t *word);

// data: a Null terminated string to be written to the file,handle: the handle
// that references the file to be written to count: returns the number of
// characters written (excluding the null terminator) This function writes a
// null terminated string to the file, at the position indicated by the
// associated file-position pointer (set by the “File Seek” or “File Index”
// commands) and advances the pointer appropriately. The file must be previously
// opened with 'w' (write) or 'a' (append) modes. Available on  FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI,
bool file_putS(const FdsDriver *fds, const char *cstr, uint16_t *count);

// size: the maximum number of bytes to be read from the file handle: the handle
// that references the file to be read from count: returns the number of
// characters read from file (excluding the null teminator),data:  returns the
// string read from the file excluding the Null terminator "This function reads
// a line of text from a file at the current file position indicated by the
// associated file-position pointer (set by the “File Seek” or “File Index”
// commands) and advances the pointer appropriately. Characters are read until
// either a newline or an EOF is received or until the specified maximum
// ""size"" is reached. In all cases, the string is null terminated. The file
// must be previously opened with 'r' (read) mode." Available on  FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI,
bool file_getS(const FdsDriver *fds, uint16_t size, uint16_t handle,
               uint16_t *n, char *str);

// filename: name of the file to be erased (passed as a string). Filename must
// be 8.3 format status: 1=success, 0=failure "This function erases a file on
// the disk. Note: If the function fails, the appropriate error number is set in
// the “File Error” command and will usually be error 19, ""failure during FILE
// search""" Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_erase(const FdsDriver *fds, const char *filename, uint16_t *status);

// handle: the handle that references the file
// status: 1=success, 0=failure
// The File Rewind command resets the file pointer to the beginning of a file
// that has been opened in 'r' (read), 'w', or 'a' (append) mode Available on
// FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_rewind(const FdsDriver *fds, uint16_t handle, uint16_t *status);

// filename:Name of the 4DGL function (filename.4FN) or application program
// (filename.4XE) that is about to be loaded into RAM in 8.3 format pointer:
// Returns a pointer to the memory allocation where the function has been loaded
// from file which can be then used as a function call The File Load Function
// command allocates the RAM area to the 4FN or 4XE program, load it from the
// uSD card in to the RAM and return a function pointer to the allocation. The
// function can then be invoked just like any other function would be called via
// a function pointer using the “File Call Function” commands. The 4FN or 4XE
// program may be discarded at any time when no longer required, thus freeing
// its memory resources. The loaded function can be discarded with the “Memory
// Free” command. Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_loadFunction(const FdsDriver *fds, const char *filename,
                       uint16_t *pointer);

// handle: The file handle that was created by the “File Load Function” n:
// Number of arguments args: optionals arguments value:  Returns the value from
// main in the called function Call the function previously loaded through “File
// Load Function”. The loaded function can be discarded with the “Memory Free”
// command. Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_callFunction(const FdsDriver *fds, uint16_t handle, uint16_t n,
                       const uint16_t *args, uint16_t *value);

// filename: A 4FN or a 4XE file is an executable file generated when a 4DGL
// file is compiled, provided in 8.3 format n: number of arguments args:
// optional arguments value: Returns the value from the called program
//  The File Run command will load the 4FN or 4XE program from the uSD card in
//  to the RAM and execute it. Once the program is called, the Host must wait
//  until the program finished execution. Any attempt to send further commands
//  while the 4FN or 4XE file is executing can cause the module to reset or
//  respond with erroneous data.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_run(const FdsDriver *fds, const char *filename, uint16_t n,
              const uint16_t *args, uint16_t *value);

// filename: A 4FN or a 4XE file is an executable file generated when a 4DGL
// file is compiled, provided in 8.3 format n: number of arguments args:
// optional arguments value: Returns the value from the called program
//  The File Run command will load the 4FN or 4XE program from the uSD card in
//  to the RAM and execute it. Once the program is called, the Host must wait
//  until the program finished execution. Any attempt to send further commands
//  while the 4FN or 4XE file is executing can cause the module to reset or
//  respond with erroneous data.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_exec(const FdsDriver *fds, const char *filename, uint16_t n,
               const uint16_t *args, uint16_t *value);

// "filename1: The control list filename ""*.dat"". filename2: The image
// filename ""*.gci"". both Created from Graphics Composer, 8.3 format. mode: [0
// .. 2] (see the documentation for details)" handle:  handle (pointer to the
// memory allocation) to the image control list that has been created. Returns
// NULL if function fails. Reads a control file to create an image list. The GCI
// file may contain images, videos or animations built through the Graphics
// Composer Software tool.The GCI file is created by selecting the GCI – FAT
// Selected Folder option in the Built Option type. See the Graphics Composer
// User Guide for further details on the Graphics Composer. Available on
// FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_loadImageControl(const FdsDriver *fds, const char *filename1,
                           const char *filename2, uint16_t mode,
                           uint16_t *handle);

// status: Non-zero: If the operation successful. 0: if the attempt failed
// Starts up the FAT16 disk file services and allocates a small 32 byte control
// block for subsequent use. When you open a file using the “File Open” command
// a further 512 + 44 = 556 bytes are attached to the FAT16 file control block.
// When you close a file using the “File Close” command, the 556 byte allocation
// is released leaving the 32 byte file control block. The File Mount command
// must be called before any other FAT16 file related functions can be used. The
// control block and all FAT16 file resources are completely released with the
// “File Unmount” command. Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_mount(const FdsDriver *fds, uint16_t *status);

// The “File Unmount” command releases any buffers for FAT16 and unmount the
// Disk FileSystem. This function is to be called to close the FAT16 file
// system. Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_unmount(const FdsDriver *fds);

// filename: Name of the wav file to be opened and played. Filename must be 8.3
// format. status:  If there are no errors, returns number of blocks to play (1
// to 32767) If errors occurred, the following is returned 6 : can’t play this
// rate 5 : no data chunk found in first sector 4 : no format data 3 : no wave
// chunk signature 2 : bad wave file format 1 : file not found Available on
// FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_playWAV(const FdsDriver *fds, const char *filename, uint16_t *status);

// handle: A string pointer to the memory area where the string is to be loaded.
// The  first string would start with handle = 0, next one would use the handle
// = string pointer returned from the execution of the Write string earlier.
// string: A Null terminated string which is to be passed to the Child (4XE or
// 4FN) program. pointer: Returns a pointer to the memory allocation where the
// string has been loaded.
//  Load the Memory space with the string to be used by the “File Call
//  Function”, “File Run” and “File Execute” commands as an argument. The Memory
//  Space for the “Read String for 4XE/4FN File” command or “Load String for
//  4XE/4FN File” command is pre-allocated memory, 512 bytes in size. It doesn’t
//  need to be released.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool file_writeString(const FdsDriver *fds, uint16_t handle, const char *cstr,
                      uint16_t *pointer);

// level: is 0 – 127
// Set the sound playback volume. Var must be in the range from 8 (min volume)
// to 127 (max volume). If var is less than 8, volume is set to 8, and if var >
// 127 it is set to 127. Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool snd_volume(const FdsDriver *fds, uint16_t level);

// rate:  Sample's playback rate. Minimum is 4KHz. Range is, 4000 – 65535
// oldRate: Returns sample's original sample rate.
// Sets the samples playback rate to a different frequency. Setting pitch to
// zero restores the original sample rate. Available on  FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI,
bool snd_pitch(const FdsDriver *fds, uint16_t rate, uint16_t *oldRate);

// buffersize:Specifies the buffer size. 0 = 1024 bytes (default), 1 = 2048
// bytes, 2 = 4096 bytes Specify the memory chunk size for the wavefile buffer,
// default size 1024 bytes. Depending on the sample size, memory constraints,
// and the sample quality, it may be beneficial to change the buffer size from
// the default size of 1024 bytes. This command is for control of a wav buffer,
// see the “Play WAV File” command Available on  FDS_PICASO, FDS_DIABLO16,
// FDS_PIXXI,
bool snd_bufSize(const FdsDriver *fds, uint16_t bufferSize);

// Stop any sound that is currently playing, releasing buffers and closing any
// open WAV file. This command is for control of a wav buffer, see the “Play WAV
// File” command Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool snd_stop(const FdsDriver *fds);

// Pause any sound that is currently playing. This command is for control of a
// wav buffer, see the “Play WAV File” command Available on  FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI,
bool snd_pause(const FdsDriver *fds);

//  Resume any sound that is currently paused by the “Sound Pause” command. This
//  command is for control of a wav buffer, see the “Play WAV File” command
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool snd_continue(const FdsDriver *fds);

// togo:  Number of 512 byte blocks to go.
// Returns 0 if sound has finished playing, else return number of 512 byte
// blocks to go. This command is for control of a wav buffer, see the “Play WAV
// File” command Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool snd_playing(const FdsDriver *fds, uint16_t *togo);

// handle:Pointer to the Image List., index: Index of the images in the list,
// xpos: Top left horizontal screen position where image is to be displayed.,
// ypos: Top left vertical screen position where image is to be displayed.
// status: 1-> If the operation successful. 0 -> if the attempt failed.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool img_setPosition(const FdsDriver *fds, uint16_t handle, int16_t index,
                     uint16_t xpos, uint16_t ypos, uint16_t *status);

// handle:Pointer to the Image List., index: Index of the images in the list.
// status: 1 -> If the operation successfull. 0 -> if the attempt failed
// This command requires that an image control has been created with the “Load
// ImageControl” command. Enables a selected image in the image list. Returns
// TRUE if index was ok and function was successful. This is the default state
// so when the “Show Image” command is called, all the images in the list will
// be shown. To enable all of the images in the list at the same time set index
// to -1. To enable a selected image, use the image index number. Available on
// FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool img_enable(const FdsDriver *fds, uint16_t handle, int16_t index,
                uint16_t *status);

// handle:Pointer to the Image List., index: Index of the images in the list.
// status: 1 -> If the operation successfull. 0 -> if the attempt failed
// This function requires that an image control has been created with the “Load
// ImageControl” command. Disables an image in the image list. Returns TRUE if
// index was ok and function was successful. Use this function to turn off an
// image so that when the “Show Image” command is called the selected image in
// the list will not be shown. To disable all of the images in the list at the
// same time set index to -1. Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool img_disable(const FdsDriver *fds, uint16_t handle, int16_t index,
                 uint16_t *status);

// handle:Pointer to the Image List., index: Index of the images in the list.
// status: 1 -> If the operation successfull. 0 -> if the attempt failed
// This function requires that an image control has been created with the “Load
// ImageControl” command. Darken an image in the image list. Returns TRUE if
// index was ok and function was successful. Use this function to darken an
// image so that when the “Show Image” command is called the control will take
// effect. To darken all of the images in the list at the same time set index to
// -1. Note: This feature will take effect one time only and when the “Show
// Image” command is called again the darkened image will revert back to normal.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool img_darken(const FdsDriver *fds, uint16_t handle, int16_t index,
                uint16_t *status);

// handle:Pointer to the Image List., index: Index of the images in the list.
// status: 1 -> If the operation successfull. 0 -> if the attempt failed
//  This function requires that an image control has been created with the “Load
//  ImageControl” command. Lighten an image in the image list. Returns TRUE if
//  index was ok and function was successful. Use this function to lighten an
//  image so that when the “Show Image” command is called the control will take
//  effect. To lighten all of the images in the list at the same time set index
//  to -1. Note: This feature will take effect one time only and when the “Show
//  Image” command is called again the lightened image will revert back to
//  normal.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool img_lighten(const FdsDriver *fds, uint16_t handle, int16_t index,
                 uint16_t *status);

// handle:Pointer to the Image List., index: Index of the images in the list.
// offset:Offset of the required word in the image entry.2 -> xpos, 3 -> ypos, 6
// -> flags, 7 -> delay, 9 -> index. value: The word to be written to the entry.
// status:  1 -> If the operation successfull. 0 -> if the attempt failed
// This function requires that an image control has been created with the “Load
// Image Control” command. Set image parameters in an image entry. Note: The
// “Show Image” command will now show an error box for out of range video
// frames. Also, if frame is set to -1, just a rectangle will be drawn in
// background colour to blank an image. It applies to PmmC R29 or above.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool img_setWord(const FdsDriver *fds, uint16_t handle, int16_t index,
                 uint16_t offset, uint16_t value, uint16_t *status);

// handle:Pointer to the Image List., index: Index of the images in the list.
// offset:Offset of the required word in the image entry.2 -> xpos, 3 -> ypos, 6
// -> flags, 7 -> delay, 9 -> index. value : value corresponding to the entrie
// This function requires that an image control has been created with the “Load
// Image Control” command. Returns the image parameters in an image entry.
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool img_getWord(const FdsDriver *fds, uint16_t handle, int16_t index,
                 uint16_t offset, uint16_t *value);

// handle:Pointer to the Image List., index: Index of the images in the list.
// status:  1 -> If the operation successfull. 0 -> if the attempt failed
// This function requires that an image control has been created with the “Load
// ImageControl” command. Enable the displaying of the image entry in the image
// control. Returns a non-zero value if successful but return value is usually
// ignored. Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool img_show(const FdsDriver *fds, uint16_t handle, int16_t index,
              uint16_t *status);

// handle:Pointer to the Image List., index: Index of the images in the list.,
// value: Refer to the Image Attribute Flags in the description status: status:
// 1 -> If the operation successfull. 0 -> if the attempt failed
//  see the documentation for detailled explanation
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool img_setAttributes(const FdsDriver *fds, uint16_t handle, int16_t index,
                       uint16_t value, uint16_t *status);

// handle:Pointer to the Image List., index: Index of the images in the list.,
// value: Refer to the Image Attribute Flags in the description status: status:
// 1 -> If the operation successfull. 0 -> if the attempt failed
//  see the documentation for detailled explanation
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool img_clearAttributes(const FdsDriver *fds, uint16_t handle, int16_t index,
                         uint16_t value, uint16_t *status);

// handle:Pointer to the Image List., index: Index of the images in the list
// value: Returns image index if image touched, -1 if image not touched.
// This command requires that an image control has been created with the “Load
// ImageControl” command. Returns index if image touched or returns -1 image not
// touched. If index is passed as -1 the command tests all images and returns -1
// if image not touched or returns index. Available on  FDS_PICASO,
// FDS_DIABLO16, FDS_PIXXI,
bool img_touched(const FdsDriver *fds, uint16_t handle, int16_t index,
                 int16_t *value);

// x,y: Specifies the horizontal and vertical position of the top-left corner of
// the x, y image to be displayed. width: width of the image to be displayed.
// height: height of the image to be displayed. data[width*height]:
// pixel1...pixeln 16 bit pixel data to be plotted on the Display screen. 16 bit
// = 5bit Red, 6bit Green, 5bit Blue This command will BLIT (Block Image
// Transfer) 16 bit pixel data from the Com port on to the screen. Available on
// FDS_GOLDELOX, FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool img_blitComtoDisplay(const FdsDriver *fds, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height,
                          const uint16_t *data);

//  see detailed 4dsystem documentation
// Available on  FDS_PICASO, FDS_PIXXI,
bool bus_in(const FdsDriver *fds, uint16_t *busState);

//  see detailed 4dsystem documentation
// Available on  FDS_PICASO, FDS_PIXXI,
bool bus_out(const FdsDriver *fds, uint16_t busState);

//  see detailed 4dsystem documentation
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool bus_read(const FdsDriver *fds, uint16_t *busState);

//  see detailed 4dsystem documentation
// Available on  FDS_PICASO, FDS_PIXXI,
bool bus_set(const FdsDriver *fds, uint16_t dirMask);

//  see detailed 4dsystem documentation
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool bus_write(const FdsDriver *fds, uint16_t bitfield);

//  see detailed 4dsystem documentation
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool pin_hi(const FdsDriver *fds, uint16_t pin, uint16_t *status);

//  see detailed 4dsystem documentation
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool pin_lo(const FdsDriver *fds, uint16_t pin, uint16_t *status);

//  see detailed 4dsystem documentation
// Available on  FDS_PICASO, FDS_DIABLO16, FDS_PIXXI,
bool pin_read(const FdsDriver *fds, uint16_t pin, uint16_t *status);

// Pin=1..7 mode = 0 .. 1 (see reference manuel)
//  see detailed 4dsystem documentation
// Available on  FDS_PICASO, FDS_PIXXI,
bool pin_set_picaso(const FdsDriver *fds, uint16_t mode, uint16_t pin,
                    uint16_t *status);

// Pin = 1..16 mode = 0..6 (see reference manuel)
//  see detailed 4dsystem documentation
// Available on  FDS_DIABLO16,
bool pin_set_diablo(const FdsDriver *fds, uint16_t mode, uint16_t pin,
                    uint16_t *status);

#ifdef __cplusplus
}
#endif
