#pragma once
#include <ch.h>
#include <hal.h>

typedef struct FdsConfig FdsConfig;

// line : Holds a positive value for the required line position. column :Holds a
// positive value for the required column position. Moves the text cursor to a
// screen position set by line and column parameters. The line and column
// position is calculated, based on the size and scaling factor for the
// currently selected font. When text is outputted to screen it will be
// displayed from this position. The text position could also be set with “Move
// Origin” command if required to set the text position to an exact pixel
// location. Note that lines and columns start from 0, so line 0, column 0 is
// the top left corner of the display.
bool txt_moveCursor(const FdsConfig *fdsConfig, uint16_t line, uint16_t column);

// car : Holds a positive value for the required character.
// Prints a single character to the display
bool txt_putCh(const FdsConfig *fdsConfig, uint16_t car);

// cstr : Holds a Null terminated string. NOTE: Maximum characters in the string
// is 255 for GOLDELOX or 511 for PICASO and DIABLO16 length : Length of the
// string printed Prints a string to the display and returns the pointer to the
// string. argument(s) length not returned if screen is of goldelox type
bool txt_putStr(const FdsConfig *fdsConfig, const char *cstr, uint16_t *length);

// car : The ASCII character for the width calculation.
// width : Width of a single character in pixel units.
// Calculate the width in pixel units for a character, based on the currently
// selected font. The font can be proportional or mono-spaced. If the total
// width of the character exceeds 255 pixel units, the function will return the
// 'wrapped' (modulo 256) value.
bool txt_charWidth(const FdsConfig *fdsConfig, char car, uint16_t *width);

// car : The ASCII character for the height calculation.
// height : Height of a single character in pixel units.
// Calculate the height in pixel units for a character, based on the currently
// selected font. The font can be proportional or mono-spaced. If the total
// height of the character exceeds 255 pixel units, the function will return the
// 'wrapped' (modulo 256) value.
bool txt_charHeight(const FdsConfig *fdsConfig, char car, uint16_t *height);

// Colour : colour to be set.
// oldCol : Previous Text Foreground Colour.
// Sets the text foreground colour.
// argument(s) oldCol not returned if screen is of goldelox type
bool txt_fgColour(const FdsConfig *fdsConfig, uint16_t colour,
                  uint16_t *oldCol);

// Colour : colour to be set.
// oldCol : Previous Text Background Colour.
// Sets the text background colour.
// argument(s) oldCol not returned if screen is of goldelox type
bool txt_bgColour(const FdsConfig *fdsConfig, uint16_t colour,
                  uint16_t *oldCol);

// Id : 0 for System font (Default Fonts), 7 for Media fonts
// oldFont : Previous Font ID.
// Sets the required font using its ID.
// argument(s) oldFont not returned if screen is of goldelox type
bool txt_fontID(const FdsConfig *fdsConfig, uint16_t id, uint16_t *oldFont);

// wMultiplier : Width multiplier, 1 to 16 (Default =1)
// oldMul : Previous Multiplier value.
// Sets the text width multiplier between 1 and 16.
// argument(s) oldMul not returned if screen is of goldelox type
bool txt_widthMult(const FdsConfig *fdsConfig, uint16_t wMultiplier,
                   uint16_t *oldMul);

// hMultiplier : Height multiplier, 1 to 16 (Default =1)
// oldMul : Previous Multiplier value.
// Sets the text height multiplier between 1 and 16.
// argument(s) oldMul not returned if screen is of goldelox type
bool txt_heightMult(const FdsConfig *fdsConfig, uint16_t hMultiplier,
                    uint16_t *oldMul);

// xGap : pixelcount 0 to 32(Default =0)
// oldGap : Previous pixelcount value.
// Sets the pixel gap between characters (x-axis), where the gap is in pixel
// units. argument(s) oldGap not returned if screen is of goldelox type
bool txt_xgap(const FdsConfig *fdsConfig, uint16_t xGap, uint16_t *oldGap);

// yGap : pixelcount 0 to 32(Default =0)
// oldGap : Previous pixelcount value.
// Sets the pixel gap between characters (y-axis), where the gap is in pixel
// units. argument(s) oldGap not returned if screen is of goldelox type
bool txt_ygap(const FdsConfig *fdsConfig, uint16_t yGap, uint16_t *oldGap);

// mode : 1 for ON, 0 for OFF.
// oldBold : Previous Bold status.
// Sets the Bold attribute for the text.
// argument(s) oldBold not returned if screen is of goldelox type
bool txt_bold(const FdsConfig *fdsConfig, uint16_t mode, uint16_t *oldBold);

// mode : 1 for ON, 0 for OFF.
// oldInv : Previous ‘Text Inverse’ status.
// Sets the text to be inverse.
// argument(s) oldInv not returned if screen is of goldelox type
bool txt_inverse(const FdsConfig *fdsConfig, uint16_t mode, uint16_t *oldInv);

// mode : 1 for ON, 0 for OFF.
// oldItal : Previous Italic Text status.
// Sets the text to italic.
// argument(s) oldItal not returned if screen is of goldelox type
bool txt_italic(const FdsConfig *fdsConfig, uint16_t mode, uint16_t *oldItal);

// mode : 1 for ON. (Opaque), 0 for OFF. (Transparent)
// oldOpa : Previous Text Opacity status.
// Selects whether or not the 'background' pixels are drawn (Default mode is
// OPAQUE with BLACK background). argument(s) oldOpa not returned if screen is
// of goldelox type
bool txt_opacity(const FdsConfig *fdsConfig, uint16_t mode, uint16_t *oldOpa);

// mode : 1 for ON, 0 for OFF.
// oldUnder : Previous Text Underline status.
// Sets the text to underlined.vThe ‘Text Underline’ attribute is cleared
// internally once the text (character or string) is displayed. Note: The “Text
// Y-gap” command is required to be at least 2 for the underline to be visible.
// argument(s) oldUnder not returned if screen is of goldelox type
bool txt_underline(const FdsConfig *fdsConfig, uint16_t mode,
                   uint16_t *oldUnder);

// bitfield : bold=1<<4, italic=1<<5, inverse=1<<6, underlined=1<<7 (bits 0-3
// and 8-15 are reserved) oldAttr : Previous Text Attributes status. Controls
// the following functions grouped : Text Bold, Text Italic, Text Inverse, Text
// Underlined. The Attributes are set to normal internally once the text
// (character or string) is displayed. argument(s) oldAttr not returned if
// screen is of goldelox type
bool txt_attributes(const FdsConfig *fdsConfig, uint16_t bitfield,
                    uint16_t *oldAttr);

// function : 7 = Sets the Delay between the characters being printed through
// Put Character or Put String functions, value = 0(Default) to 255 (ms) Sets
// various parameters for the Text commands.
bool txt_set(const FdsConfig *fdsConfig, uint16_t function, uint16_t value);

// xpos : 0 for OFF (Default), 1 to N for Wrap position in Pixels.
// oldWrap : Previous wrap position
// Sets the pixel position where text wrap will occur at RHS. The feature
// automatically resets when screen mode is changed.
bool txt_wrap(const FdsConfig *fdsConfig, uint16_t xpos, uint16_t *oldWrap);

// Clears the screen using the current background colour. Brings some of the
// settings back to default such as Outline colour set to BLACK, Pen set to
// OUTLINE, Text magnifications set to 1, All origins set to 0:0.
bool gfx_cls(const FdsConfig *fdsConfig);

// oldColor : sample colour to be changed within the clipping window, newColor :
// new colour to change all occurrences of old colour within the clipping window
// Changes all oldColour pixels to newColour within the clipping window area.
bool gfx_changeColour(const FdsConfig *fdsConfig, uint16_t oldColour,
                      uint16_t newColour);

// x, y : centre of the circle, radius : radius of the circle, colour : colour
// of the circle. Draws a circle with centre point and radius using the
// specified colour.
bool gfx_circle(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                uint16_t radius, uint16_t colour);

// x, y : centre of the circle, radius : radius of the circle, colour : colour
// of the circle. Draws a solid circle with centre point and radius using the
// specified colour. Outline colour can be specified with the “Outline Colour”
// command, if set to 0, no outline is drawn.
bool gfx_circleFilled(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                      uint16_t radius, uint16_t colour);

// x1,y1 : starting coordinates of the line, x2,y2 : ending coordinates of the
// line, colour : colour of the line. Draws a line from x1,y1 to x2,y2 using the
// specified colour. The line is drawn using the current object colour. The
// current origin is not altered. The line may be tessellated with the “Line
// Pattern” command.
bool gfx_line(const FdsConfig *fdsConfig, uint16_t x1, uint16_t y1, uint16_t x2,
              uint16_t y2, uint16_t colour);

// tlx,tly : top left corner of the rectangle, brx,bry : bottom right corner of
// the rectangle, colour : colour of the rectangle. Draws a rectangle from x1,
// y1 to x2, y2 using the specified colour. The line may be tessellated with the
// “Line Pattern” command.
bool gfx_rectangle(const FdsConfig *fdsConfig, uint16_t tlx, uint16_t tly,
                   uint16_t brx, uint16_t bry, uint16_t colour);

// tlx,tly : top left corner of the rectangle, brx,bry : bottom right corner of
// the rectangle, colour : colour of the rectangle. Draws a Filled rectangle
// from x1, y1 to x2, y2 using the specified colour. The line may be tessellated
// with the “Line Pattern” command. Outline colour can be specified with the
// “Outline Colour” command, if set to 0, no outline is drawn.
bool gfx_rectangleFilled(const FdsConfig *fdsConfig, uint16_t tlx, uint16_t tly,
                         uint16_t brx, uint16_t bry, uint16_t colour);

// n : number of elements in the x and y arrays specifying the vertices for the
// polyline, vx[n] : array of elements for the x coordinates of the vertices,
// vy[n] : array of elements for the y coordinates of the vertices, colour :
// colour of the polyline. Plots lines between points specified by a pair of
// arrays using the specified colour. The lines may be tessellated with the
// “Line Pattern” command
bool gfx_polyline(const FdsConfig *fdsConfig, uint16_t n, const uint16_t vx[],
                  const uint16_t vy[], uint16_t color);

// n : number of elements in the x and y arrays specifying the vertices for the
// polygon, vx[n] : array of elements for the x coordinates of the vertices,
// vy[n] : array of elements for the y coordinates of the vertices, colour :
// colour of the polygon. Plots lines between points specified by a pair of
// arrays using the specified colour. The last point is drawn back to the first
// point, completing the polygon. The lines may be tessellated with “Line
// Pattern” command.
bool gfx_polygon(const FdsConfig *fdsConfig, uint16_t n, const uint16_t vx[],
                 const uint16_t vy[], uint16_t color);

// n : number of elements in the x and y arrays specifying the vertices for the
// polygon, vx[n] : array of elements for the x coordinates of the vertices,
// vy[n] : array of elements for the y coordinates of the vertices, colour :
// colour of the polygon. Draws a solid Polygon between specified vertices using
// the specified colour. The last point is drawn back to the first point,
// completing the polygon. Vertices must be a minimum of 3 and can be specified
// in any fashion.
bool gfx_polygonFilled(const FdsConfig *fdsConfig, uint16_t n,
                       const uint16_t vx[], const uint16_t vy[],
                       uint16_t color);

// x1,y1 : first vertice of the triangle, x2,y2 : second vertice of the
// triangle, x3,y3 : third vertice of the triangle, colour : colour of the
// triangle. Draws a triangle outline between specified vertices using the
// specified colour. The line may be tessellated with the “Line Pattern”
// command.
bool gfx_triangle(const FdsConfig *fdsConfig, uint16_t x1, uint16_t y1,
                  uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
                  uint16_t colour);

// x1,y1 : first vertice of the triangle, x2,y2 : second vertice of the
// triangle, x3,y3 : third vertice of the triangle, colour : colour of the
// triangle. Draws a solid triangle between specified vertices using the
// specified colour.
bool gfx_triangleFilled(const FdsConfig *fdsConfig, uint16_t x1, uint16_t y1,
                        uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
                        uint16_t colour);

// angle : angle from the origin to the remote point. The angle is specified in
// degrees, distance : distance from the origin to the remote point in pixel
// units. Xdist,Ydist : coordinates from the current origin. Calculates the
// coordinates of a distant point relative to the current origin, where the only
// known parameters are the angle and the distance from the current origin.
bool gfx_orbit(const FdsConfig *fdsConfig, uint16_t angle, uint16_t distance,
               uint16_t *Xdist, uint16_t *Ydist);

// x, y : pixel coordinates, colour : colour of the pixel
// Draws a pixel at specified coordinates using the specified colour.
bool gfx_putPixel(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                  uint16_t colour);

// x, y : pixel coordinates
// colour : 16bit colour of the pixel.
// Reads the colour value of the pixel at the specified coordinate.
bool gfx_getPixel(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                  uint16_t *colour);

// x,y : coordinates of the new origin.
// Moves the origin to a new position.
bool gfx_moveTo(const FdsConfig *fdsConfig, uint16_t x, uint16_t y);

// x,y : line end coordinates.
// Draws a line from the current origin to a new position. The Origin is then
// set to the new position. The line is drawn using the current object colour,
// using the “Set Graphics Parameters” – “Object Colour” command. The line may
// be tessellated with the “Line Pattern” command.
bool gfx_lineTo(const FdsConfig *fdsConfig, uint16_t x, uint16_t y);

// mode : 0 = Clipping Disabled, 1 = Clipping Enabled
// Enables or Disables the ability for Clipping to be used.
bool gfx_clipping(const FdsConfig *fdsConfig, uint16_t mode);

// tlx,tly :  top left corner of the clipping window, brx,bry : bottom right
// corner of the clipping window. Specifies a clipping window region on the
// screen such that any objects and text placed onto the screen will be clipped
// and displayed only within that region. For the clipping window to take
// effect, the clipping setting must be enabled separately using the “Clipping”
// command
bool gfx_clipWindow(const FdsConfig *fdsConfig, uint16_t tlx, uint16_t tly,
                    uint16_t brx, uint16_t bry);

// Forces the clip region to the extent of the last text that was printed, or
// the last image that was shown.
bool gfx_setClipRegion(const FdsConfig *fdsConfig);

// x,y : center of ellipse, xrad : x-radius of ellipse, yrad : y-radius of
// ellipse, colour : colour of the ellipse. Plots a coloured Ellipse on the
// screen at centre x, y with x-radius = xrad and y-radius = yrad.
bool gfx_ellipse(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                 uint16_t xrad, uint16_t yrad, uint16_t colour);

// x,y : center of ellipse, xrad : x-radius of ellipse, yrad : y-radius of
// ellipse, colour : colour of the ellipse. Plots a solid coloured Ellipse on
// the screen at centre x, y with x-radius = xrad and y-radius = yrad.
bool gfx_ellipseFilled(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
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
bool gfx_button(const FdsConfig *fdsConfig, uint16_t state, uint16_t x,
                uint16_t y, uint16_t buttoncolour, uint16_t txtcolour,
                uint16_t font, uint16_t txtWidth, uint16_t txtHeight,
                const char *cstr);

// "state : appearance of panel, 0 = recessed
//  1 = raised, x,y : top left corner position of the panel, width : width of
//  the panel ,height : height of the panel, colour : colour of the panel."
// Draws a 3 dimensional rectangular panel
bool gfx_panel(const FdsConfig *fdsConfig, uint16_t state, uint16_t x,
               uint16_t y, uint16_t width, uint16_t height, uint16_t colour);

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
// shade of the colour parameter is used for the thumb .
bool gfx_slider(const FdsConfig *fdsConfig, uint16_t mode, uint16_t tlx,
                uint16_t tly, uint16_t brx, uint16_t bry, uint16_t colour,
                uint16_t scale, int16_t value);

// xs, ys : position of the top left corner of the area to be copied (source),
// xd, yd : position of the top left corner of where the paste is to be made
// (destination), width : width of the copied area, height : height of the
// copied area. Copies an area of the screen from xs, ys of size given by width
// and height parameters and pastes it to another location determined by xd, yd.
bool gfx_screenCopyPaste(const FdsConfig *fdsConfig, uint16_t xs, uint16_t ys,
                         uint16_t xd, uint16_t yd, uint16_t width,
                         uint16_t height);

// value : number of pixel, 0 = No Bevel Shadow, 1-4 = Number of Pixels Deep
// (default = 3) oldBevel : previous bevel shadow status. Changes the graphics
// “Draw Button” commands bevel shadow depth
bool gfx_bevelShadow(const FdsConfig *fdsConfig, uint16_t value,
                     uint16_t *oldBevel);

// value : 0 = No Bevel, 1-15 = Number of Pixels Wide (Default = 2)
// oldWidth : previous bevel width status.
// Changes the graphics “Draw Button” commands bevel width
bool gfx_bevelWidth(const FdsConfig *fdsConfig, uint16_t value,
                    uint16_t *oldWidth);

// colour : colour to be set (0-65535 or HEX 0x0000-0xFFFF)
// oldCol : previous background Colour.
// Sets the screen background colour
// argument(s) oldCol not returned if screen is of goldelox type
bool gfx_bgCcolour(const FdsConfig *fdsConfig, uint16_t colour,
                   uint16_t *oldCol);

// colour : colour to be set (0-65535 or HEX 0x0000-0xFFFF), set to 0 for no
// effect oldCol : previous outline Colour. Sets the outline colour for
// rectangles and circles. argument(s) oldCol not returned if screen is of
// goldelox type
bool gfx_outlineColour(const FdsConfig *fdsConfig, uint16_t colour,
                       uint16_t *oldCol);

// contrast : 0 = Display OFF, 1 - 15 = Contrast Level
// oldContrast : previous Contrast value.
// Sets the contrast of the display, or turns it On/Off depending on display
// model argument(s) oldContrast not returned if screen is of goldelox type
bool gfx_contrast(const FdsConfig *fdsConfig, uint16_t contrast,
                  uint16_t *oldContrast);

// delayMsec : delay in milliseconds (0-255)
// oldDelay : previous Frame Delay value.
// Sets the inter frame delay for the “Media Video” command
// argument(s) oldDelay not returned if screen is of goldelox type
bool gfx_frameDelay(const FdsConfig *fdsConfig, uint16_t delayMsec,
                    uint16_t *oldDelay);

// pattern : 0 = all line pixels are on (Default), 0-65535 (or HEX
// 0x0000-0xFFFF) = number of bits in the line are turned off to form a pattern
// oldPattern : previous Line Pattern value.
// Sets the line draw pattern for line drawing. If set to zero, lines are solid,
// else each '1' bit represents a pixel that is turned off. argument(s)
// oldPattern not returned if screen is of goldelox type
bool gfx_linePattern(const FdsConfig *fdsConfig, uint16_t pattern,
                     uint16_t *oldPattern);

// mode : 0 = Landscape, 1 = Landscape reverse, 2 = Portrait, 3 = Portrait
// reverse oldMode : previous Screen Mode value. Alters the graphics orientation
// (Landscape, Landscape reverse, Portrait or Portrait reverse) argument(s)
// oldMode not returned if screen is of goldelox type
bool gfx_screenMode(const FdsConfig *fdsConfig, uint16_t mode,
                    uint16_t *oldMode);

// mode : 0 = Transparency OFF, 1 = Transparency ON
// oldMode : previous Transparency value.
// Turns the transparency ON or OFF. Transparency is automatically turned OFF
// after the next image or video command.
bool gfx_transparency(const FdsConfig *fdsConfig, uint16_t mode,
                      uint16_t *oldMode);

// colour : colour to make transparent (0-65535 or HEX 0x0000-0xFFFF)
// oldColor : previous Transparent Colour value.
// Alters the colour that needs to be made transparent.
bool gfx_transparentColour(const FdsConfig *fdsConfig, uint16_t colour,
                           uint16_t *oldColor);

// function, value : see « Serial Commands Reference Manual » of appropriate
// display processor Sets various parameters for the graphics gommands.
bool gfx_set(const FdsConfig *fdsConfig, uint16_t function, uint16_t value);

// mode : 0 = Current orientations maximum X value (X_MAX), 1 = Current
// orientations maximum Y value (Y_MAX), 2 = Left location of last Object, 3 =
// Top location of Object, 4 = Right location of last Object, 5 = Bottom
// location of Object value : Mode=0: horizontal resolution - 1, Mode=1:
// vertical resolution - 1, Mode=2: left location of the last drawn object,
// Mode=3: top location of the last drawn object, Mode=4: right location of the
// last drawn object, Mode=5: bottom location of the last drawn object. Returns
// various graphics parameters to the caller.
bool gfx_get(const FdsConfig *fdsConfig, uint16_t mode, uint16_t *value);

// value : Non-Zero = card is present and successfully initialised, 0 = no card
// is present or not able to initialise. Initialises a uSD/SD/SDHC memory card
// for further operations. SD card is connected to the SPI of the processor
// display.
bool media_init(const FdsConfig *fdsConfig, uint16_t *value);

// hiAddr : high word (upper 2 bytes) of a 4 byte media memory byte address
// location, loAddr : low word (lower 2 bytes) of a 4 byte media memory byte
// address location. Sets the media memory internal Address pointer for access
// at a non-sector aligned byte address.
bool media_setAdd(const FdsConfig *fdsConfig, uint16_t hiAddr, uint16_t loAddr);

// hiAddr : high word (upper 2 bytes) of a 4 byte media memory byte address
// location, loAddr : low word (lower 2 bytes) of a 4 byte media memory byte
// address location. Sets the media memory internal Address pointer for sector
// access.
bool media_setSector(const FdsConfig *fdsConfig, uint16_t hiAddr,
                     uint16_t loAddr);

// value : byte value in the LSB.
// Returns the byte value from the current media address, set by the “Set Byte
// Address” command. The internal byte address will then be internally
// incremented by one.
bool media_readByte(const FdsConfig *fdsConfig, uint16_t *value);

// value : word value
// Returns the word value (2 bytes) from the current media address, set by the
// “Set Byte Address” command. The internal byte address will then be internally
// incremented by two. If the address is not aligned, the word will still be
// read correctly.
bool media_readWord(const FdsConfig *fdsConfig, uint16_t *value);

// s : byte value, in the LSB, to be written at the current byte address
// location. status : Non Zero = successful media response, 0 = attempt failed.
// Writes a byte to the current media address that was initially set with the
// “Set Sector Address” command. See « Serial Commands Reference Manual » for
// more information.
bool media_writeByte(const FdsConfig *fdsConfig, uint16_t s, uint16_t *status);

// value : 16 bit word to be written at the current media address location.
// status : Non Zero = successful media response, 0 = attempt failed.
// Writes a word to the current media address that was initially set with the
// “Set Sector Address” command. See « Serial Commands Reference Manual » for
// more information.
bool media_writeWord(const FdsConfig *fdsConfig, uint16_t value,
                     uint16_t *status);

// status : Non Zero = successful media response, 0 = attempt failed.
// After writing any data to a sector, the Flush Media command should be called
// to ensure that the current sector that is being written is correctly stored
// back to the media else write operations may be unpredictable.
bool media_flush(const FdsConfig *fdsConfig, uint16_t *status);

// x,y : top left position where the image will be displayed.
// Displays an image from the media storage at the specified co-ordinates. The
// image address is previously specified with the “Set Byte Address” command or
// “Set Sector Address” command. If the image is shown partially off screen, it
// may not be displayed correctly.
bool media_image(const FdsConfig *fdsConfig, uint16_t x, uint16_t y);

// x,y : top left position where the video clip will be displayed.
// Displays a video clip from the media storage device at the specified
// co-ordinates. The video address location in the media is previously specified
// with the “Set Byte Address” or “Set Sector Address” commands. If the video is
// shown partially off screen, it may not be displayed correctly. Note that
// showing a video blocks all other processes until the video has finished
// showing. See the “Display Video Frame” command for alternatives.
bool media_video(const FdsConfig *fdsConfig, uint16_t x, uint16_t y);

// x,y : top left position of the video frame to be displayed, frameNumber :
// required frame number to be displayed. Displays a video from the media
// storage device at the specified co-ordinates. The video address is previously
// specified with the “Set Byte Address” command or “Set Sector Address”
// command. If the video is shown partially off it may not be displayed
// correctly. The frames can be shown in any order. This function gives a great
// flexibility for showing various icons from an image strip, as well as showing
// videos while doing other tasks.
bool media_videoFrame(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                      int16_t frameNumber);

// eveReg : byte register address
// value : register value in the LSB.
// Returns the EVE System Byte Register value in the lower byte. See EVE System
// Registers Memory Map in « Serial Commands Reference Manual ».
bool misc_peekB(const FdsConfig *fdsConfig, uint16_t eveReg, uint16_t *value);

// eveReg : byte register address, value : register value in the LSB.
// Sets the EVE System Byte Register value in the lower byte. See EVE System
// Registers Memory Map in « Serial Commands Reference Manual ».
bool misc_pokeB(const FdsConfig *fdsConfig, uint16_t eveReg, uint16_t value);

// eveReg : word register address
// value : registre value
// Returns the EVE System Word Register value. See EVE System Registers Memory
// Map in « Serial Commands Reference Manual ».
bool misc_peekW(const FdsConfig *fdsConfig, uint16_t eveReg, uint16_t *value);

// eveReg : word register address, value : register value
// Sets the EVE System Word Register value. See EVE System Registers Memory Map
// in « Serial Commands Reference Manual ».
bool misc_pokeW(const FdsConfig *fdsConfig, uint16_t eveReg, uint16_t value);

// address : address to be peeked
// value : contents of the specified memory address.
// Returns the word contents of a specified memory address. This command would
// normally be used to read the contents of File and/or ImageControl handles.
bool misc_peekM(const FdsConfig *fdsConfig, uint16_t address, uint16_t *value);

// address : address to be poked, value : data to be poked into the address
// Sets the word contents of a specified memory address. This command would
// normally be used to alter the contents of File and/or ImageControl handles.
bool misc_pokeM(const FdsConfig *fdsConfig, uint16_t address, uint16_t value);

// value : value of the Joystick position, 0=Released, 1=Up , 2=Left , 3=Down ,
// 4=Right, 5=Press (5 position switch implementation). Return the value of the
// joystick position. Refer to the processor datasheet.
bool misc_joystick(const FdsConfig *fdsConfig, uint16_t *value);

// note : frequency of the note (0-64), duration_ms : time in milliseconds that
// the note will be played for. Produce a single musical note for the required
// duration through IO2.
bool misc_beep(const FdsConfig *fdsConfig, uint16_t note, uint16_t duration_ms);

// index : baud rate index value. See index table in « Serial Commands Reference
// Manual » Set the required baud rate. Please refer to the « Serial Commands
// Reference Manual ».
bool misc_setbaudWait(const FdsConfig *fdsConfig, int16_t index);

// duration_s : Sleep duration (0-65535) approximately in seconds (in sleep
// mode, timing is controlled by an RC oscillator, so is not totally accurate)
// ack,duration : remaining time when touch screen is touched, else returns
// zero. "Puts display and processor into low power mode for a period of time.
// If ""duration"" is zero, goes into sleep mode forever and needs power cycling
// to re-initialize. If ""duration"" is 1 to 65535, display will sleep for that
// period of time, or will be woken when touch screen is touched. Returns
// remaining time when screen was touched. When returning from sleep mode,
// display and processor are restored from low power mode."
bool sys_sleep(const FdsConfig *fdsConfig, uint16_t duration_s,
               uint16_t *duration);

// handle : pointer to the memory block.
// status : Non Zero = operation successful, 0 = attempt failed.
// "Releases the memory space used by the the ""Load Image Control"" and ""File
// Load Function"" commands."
bool sys_memFree(const FdsConfig *fdsConfig, uint16_t handle, uint16_t *status);

// avail : largest available memory chunk of the heap.
// Returns byte size of the largest chunk of memory available in the heap.
bool sys_memHeap(const FdsConfig *fdsConfig, uint16_t *avail);

// n : number of characters in the model name to return, str[n] : display
// module’s model name (without NULL terminator). Returns the Display Model in
// the form of a string without Null terminator.
bool sys_getModel(const FdsConfig *fdsConfig, uint16_t *n, char *str);

// version : SPE Version installed on the module.
// Returns the SPE Version installed on the module.
bool sys_getVersion(const FdsConfig *fdsConfig, uint16_t *version);

// version : PmmC Version installed on the module.
// Returns the PmmC Version installed on the module.
bool sys_getPmmC(const FdsConfig *fdsConfig, uint16_t *version);

// timout_ms : Screen saver timeout in milliseconds (1 to 65535, 0 disables the
// screen saver). Set the screen saver Timeout. Default screen saver timeout
// settings could be adjusted. This feature is display dependent, please refer
// to module's datasheet.
bool misc_screenSaverTimeout(const FdsConfig *fdsConfig, uint16_t timout_ms);

// speed : screen saver speed. Range is display dependant. Please refer to
// module datasheet Set the screen saver speed. Default screen saver speed
// settings could be adjusted. This feature is display dependent, please refer
// to module datasheet.
bool misc_screenSaverSpeed(const FdsConfig *fdsConfig, uint16_t speed);

// mode : screen saver scroll direction. Display dependant, please refer to
// module datasheet. Set the screen saver scroll direction. This feature is
// display dependent, please refer to module's datasheet.
bool misc_screenSaverMode(const FdsConfig *fdsConfig, uint16_t mode);

// tlx,tly : position of the top left corner of the region, brx,bry : position
// of the bottom right corner of the region. Specifies a touch detect region on
// the screen. This setting will filter out any touch activity outside the
// region and only touch activity within that region will be reported by the
// status poll “Touch Get” command
bool touch_detectRegion(const FdsConfig *fdsConfig, uint16_t tlx, uint16_t tly,
                        uint16_t brx, uint16_t bry);

// mode : 0=Enables and initialises Touch Screen hardware, 1=Disables the Touch
// Screen (Touch Screen task runs in the background, disabling it when not in
// use free up extra CPU cycles), 2 = reset the current active region to default
// (= full screen area) Sets various Touch Screen related parameters.
bool touch_set(const FdsConfig *fdsConfig, uint16_t mode);

// mode : 0 = get Status, 1 = get X coordinates, 2 = get Y coordinates.
// value : mode0 = returns the various states of the touch screen
// (0=invalid/notouch, 1=press, 2=release, 3=moving), mode1 = returns the X
// coordinates of the touch reported by mode0, mode2 = returns the Y coordinates
// of the touch reported by mode0. Returns various touch screen parameters,
// based on the touch detect region set by the “Touch Detect Region” command.
bool touch_get(const FdsConfig *fdsConfig, uint16_t mode, uint16_t *value);

// errno : error number (0 = no error).
// Returns the most recent error code. See error table in « Serial Commands
// Reference Manual »
bool file_error(const FdsConfig *fdsConfig, uint16_t *errno);

// filename : Name of the file(s) for search (passed as a string). Filename must
// be 8.3 format. count : number of files that match the criteria. Returns
// number of files found that match the criteria. The wild card character '*'
// matches up with any combination of allowable characters and '?' matches up
// with any single allowable character.
bool file_count(const FdsConfig *fdsConfig, const char *filename,
                uint16_t *count);

// filename : Name of the file(s) for search (passed as a string). Filename must
// be 8.3 format. count : number of files that match the criteria. Lists the
// stream of file names that agree with the search key on the Display Screen.
// The wild card character ‘*’ matches up with any combination of allowable
// characters and ‘?’ matches up with any single allowable character. Note:
// “Find First File and Report” and “Find Next File and Report” are recommended
// alternatives
bool file_dir(const FdsConfig *fdsConfig, const char *filename,
              uint16_t *count);

// filename : Name of the file(s) for search (passed as a string). Filename must
// be 8.3 format. status : 1 = at least one file exists that satisfies the
// criteria, 0 = no file satisfies the criteria. Returns 1 if at least one file
// exists that satisfies the file argument. Wildcards are usually used so if the
// “Find First File” command returns true, further tests can be made using the
// “Find Next File” command to find all the files that match the wildcard class.
// Notes : filename is printed on the screen. “Find First File and Report” and
// “Find Next File and Report” are recommended alternatives in order to return
// the responses.
bool file_findFirst(const FdsConfig *fdsConfig, const char *filename,
                    uint16_t *status);

// filename : Name of the file(s) for search (passed as a string). Filename must
// be 8.3 format. length : Length of the filename string, str[n] : Filename if
// it exists. Filename string is not NULL terminated. Returns length of filename
// and filename if at least 1 file exists that matches the criteria. Wildcards
// are usually used so further tests can be made using “Find Next File” or “Find
// Next File and Report” commands to find all the files that match the wildcard
// class.
bool file_findFirstRet(const FdsConfig *fdsConfig, const char *filename,
                       uint16_t *length, char *str);

// status : 1 = at least one file exists that satisfies the criteria, 0 = no
// file satisfies the criteria. Returns 1 if more file exists that satisfies the
// file argument that was given for the “Find First File” or “Find First File
// and Report” commands. Wildcards must be used for the “Find First File” or
// “Find First File and Report” commands else this function will always return
// zero as the only occurrence will have already been found. Note : filename is
// printed on the screen.
bool file_findNext(const FdsConfig *fdsConfig, uint16_t *status);

// length : Length of the filename string, str[n] : Filename if it exists.
// Filename string is not NULL terminated. "Returns length of filename and
// filename if at least 1 file exists that matches criteria given for the “Find
// First File” or “Find First File and Report” commands. Wildcards must be used
// for “Find First File” or “Find First File and Report” commands else this
// function will always return zero as the only occurrence will have already
// been found. Wildcards are usually used, so if the “Find First File” or “Find
// First File and Report” commands return the stringlength and filename, further
// tests can be made using ""Find Next File and Report"" command to find all the
// files that match the wildcard class."
bool file_findNextRet(const FdsConfig *fdsConfig, uint16_t *length, char *str);

// filename : Name of the file(s) for search (passed as a string). Filename must
// be 8.3 format. status : 1 = File found, 0 = File not found. Tests for the
// existence of the file provided with the search key.
bool file_exists(const FdsConfig *fdsConfig, const char *filename,
                 uint16_t *status);

// filename : Name of the file(s) for search (passed as a string). Filename must
// be 8.3 format. Mode : ‘r’ or ‘w’ or ‘a’ (read write, append) handle : return
// handle if file exists, else internal file error is set Returns handle if file
// exists. The file ‘handle’ that is created is now used as reference for
// ‘filename’ for further file commands such as “File Close”, etc. For File
// Write and File Append modes ('w' and 'a') the file is created if it does not
// exist.
bool file_open(const FdsConfig *fdsConfig, const char *filename, char mode,
               uint16_t *handle);

// handle :  The file handle that was created by the “File Open” command which
// is now used as reference ‘handle’ for the filename, for further file
// functions such as in this function to close the file. Status : 1 = file
// closed, 0 = file not closed The File Close command will close the previously
// opened file.
bool file_close(const FdsConfig *fdsConfig, uint16_t handle, uint16_t *status);

// size : Number of bytes to be read. handle : The handle that references the
// file to be read. count : Returns the number of bytes read. data : Data read
// from the file Returns the number of bytes specified by ‘size’ from the file
// referenced by ‘handle’.
bool file_read(const FdsConfig *fdsConfig, uint16_t size, uint16_t handle,
               uint16_t *n, char *str);

// "handle The handle that references the file 
// HiWord Contains the upper 16bits of the memory pointer into the file. LoWord
// Contains the lower 16bits of the memory pointer into the file." Status : 1 =
// success, 0 = error The File Seek command places the file pointer at the
// required position in a file that has been opened in 'r' (read) or 'a'
// (append) mode.
bool file_seek(const FdsConfig *fdsConfig, uint16_t handle, uint16_t hiWord,
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
// the “Display Image (FAT)” command.
bool file_index(const FdsConfig *fdsConfig, uint16_t handle, uint16_t hiWord,
                uint16_t loWord, uint16_t recordNum, uint16_t *status);

// Handle : The handle that references the file
// "Status : 1 = success, 0 = error 
//  HiWord Contains the upper 16bits of the value of the pointer 
//  LoWord  Contains the lower 16bits of the value of the pointer"
// The File Tell command returns the current value of the file pointer.
bool file_tell(const FdsConfig *fdsConfig, uint16_t handle, uint16_t *status,
               uint16_t *hiWord, uint16_t *loWord);

// size : Number of bytes to be written. Maximum that can be written at one time
// is 512 bytes.source :  String of Data without Null terminator. Handle :  The
// handle that references the file to write. count  Returns the number of bytes
// written. write chunk of data to file
bool file_write(const FdsConfig *fdsConfig, uint16_t size,
                const uint8_t *source, uint16_t handle, uint16_t *count);

// Handle : The handle that references the file
// "Status : 1 = success, 0 = error 
//  HiWord Contains the upper 16bits of the file size 
//  LoWord  Contains the lower 16bits of the file size"
bool file_size(const FdsConfig *fdsConfig, uint16_t handle, uint16_t *status,
               uint16_t *hiWord, uint16_t *loWord);

// "x : X-position of the image to be displayed 
//  y : Y-position of the image to be displayed 
//  handle : The handle that references the file containing the image(s)."
// error  Returns a copy of the File Error, see the “File Error” command
// Display an image from the file stream at screen location specified by x, y
// (top left corner).If there is more than 1 image in the file, it can be
// accessed with the “File Seek” command
bool file_image(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                uint16_t handle, uint16_t *errno);

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
// the capture area is determined by ""width"" and ""height""."
bool file_screenCapture(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                        uint16_t width, uint16_t height, uint16_t handle,
                        uint16_t *status);

// car: data byte (in the LSB) about to be written, handle: The handle that
// references the file to be written to. status: returns the number of bytes
// written successfully This function writes the byte specified by "char" to the
// file, at the position indicated by the associated file-position pointer (set
// by the “File Seek” or “File Index” commands) and advances the pointer
// appropriately (incremented by 1). The file must be previously opened with 'w'
// (write) or 'a' (append) modes.
bool file_putC(const FdsConfig *fdsConfig, uint16_t car, uint16_t handle,
               uint16_t *status);

// handle: the handle that references the file to be read from
// car: returns the data byte read from the file in the LSB
// The Read Character from the File command reads a byte from the file, at the
// position indicated by the associated file-position pointer (set by the “File
// Seek” or “File Index”commands) and advances the pointer appropriately
// (incremented by 1). The file must be previously opened with 'r' (read) mode.
bool file_getC(const FdsConfig *fdsConfig, uint16_t handle, uint16_t *car);

// word: word about to be written, handle: The handle that references the file
// to be written to. status: returns the number of bytes written successfully
// This function writes the word specified by "word" to the file, at the
// position indicated by the associated file-position pointer (set by the “File
// Seek” or “File Index” commands) and advances the pointer appropriately
// (incremented by 2). The file must be previously opened with 'w' (write) or
// 'a' (append) modes.
bool file_putW(const FdsConfig *fdsConfig, uint16_t word, uint16_t handle,
               uint16_t *status);

// handle: the handle that references the file to be read from
// word: returns the word read from the file in the LSB
// The Read Word from the File command reads a word from the file, at the
// position indicated by the associated file-position pointer (set by the “File
// Seek” or “File Index”commands) and advances the pointer appropriately
// (incremented by 2). The file must be previously opened with 'r' (read) mode.
bool file_getW(const FdsConfig *fdsConfig, uint16_t handle, uint16_t *word);

// data: a Null terminated string to be written to the file,handle: the handle
// that references the file to be written to count: returns the number of
// characters written (excluding the null terminator) This function writes a
// null terminated string to the file, at the position indicated by the
// associated file-position pointer (set by the “File Seek” or “File Index”
// commands) and advances the pointer appropriately. The file must be previously
// opened with 'w' (write) or 'a' (append) modes.
bool file_putS(const FdsConfig *fdsConfig, const char *cstr, uint16_t *count);

// size: the maximum number of bytes to be read from the file handle: the handle
// that references the file to be read from count: returns the number of
// characters read from file (excluding the null teminator),data:  returns the
// string read from the file excluding the Null terminator This function reads a
// line of text from a file at the current file position indicated by the
// associated file-position pointer (set by the “File Seek” or “File Index”
// commands) and advances the pointer appropriately. Characters are read until
// either a newline or an EOF is received or until the specified maximum "size"
// is reached. In all cases, the string is null terminated. The file must be
// previously opened with 'r' (read) mode.
bool file_getS(const FdsConfig *fdsConfig, uint16_t size, uint16_t handle,
               uint16_t *n, char *str);

// filename: name of the file to be erased (passed as a string). Filename must
// be 8.3 format status: 1=success, 0=failure This function erases a file on the
// disk. Note: If the function fails, the appropriate error number is set in the
// “File Error” command and will usually be error 19, "failure during FILE
// search"
bool file_erase(const FdsConfig *fdsConfig, const char *filename,
                uint16_t *status);

// handle: the handle that references the file
// status: 1=success, 0=failure
// The File Rewind command resets the file pointer to the beginning of a file
// that has been opened in 'r' (read), 'w', or 'a' (append) mode
bool file_rewind(const FdsConfig *fdsConfig, uint16_t handle, uint16_t *status);

bool file_loadFunction(const FdsConfig *fdsConfig, const char *filename,
                       uint16_t *pointer);

bool file_callFunction(const FdsConfig *fdsConfig, uint16_t handle, uint16_t n,
                       const uint16_t *args, uint16_t *value);

bool file_run(const FdsConfig *fdsConfig, const char *filename, uint16_t n,
              const uint16_t *args, uint16_t *value);

bool file_exec(const FdsConfig *fdsConfig, const char *filename, uint16_t n,
               const uint16_t *args, uint16_t *value);

bool file_loadImageControl(const FdsConfig *fdsConfig, const char *filename1,
                           const char *filename2, uint16_t mode,
                           uint16_t *handle);

bool file_mount(const FdsConfig *fdsConfig, uint16_t *status);

bool file_unmount(const FdsConfig *fdsConfig);

bool file_playWAV(const FdsConfig *fdsConfig, const char *filename,
                  uint16_t *status);

bool file_writeString(const FdsConfig *fdsConfig, uint16_t handle,
                      const char *cstr, uint16_t *pointer);

// level is 0 – 127
bool snd_volume(const FdsConfig *fdsConfig, uint16_t level);

// rate is 4000 – 65535
bool snd_pitch(const FdsConfig *fdsConfig, uint16_t rate, uint16_t *oldRate);

// 0 .. 2
bool snd_bufSize(const FdsConfig *fdsConfig, uint16_t bufferSize);

bool snd_stop(const FdsConfig *fdsConfig);

bool snd_pause(const FdsConfig *fdsConfig);

bool snd_continue(const FdsConfig *fdsConfig);

bool snd_playing(const FdsConfig *fdsConfig, uint16_t *togo);

bool img_setPosition(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
                     uint16_t xpos, uint16_t ypos, uint16_t *status);

bool img_enable(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
                uint16_t *status);

bool img_disable(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
                 uint16_t *status);

bool img_darken(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
                uint16_t *status);

bool img_lighten(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
                 uint16_t *status);

bool img_setWord(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
                 uint16_t offset, uint16_t value, uint16_t *status);

bool img_getWord(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
                 uint16_t offset, uint16_t *value);

bool img_show(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
              uint16_t *status);

bool img_setAttributes(const FdsConfig *fdsConfig, uint16_t handle,
                       int16_t index, uint16_t value, uint16_t *status);

bool img_clearAttributes(const FdsConfig *fdsConfig, uint16_t handle,
                         int16_t index, uint16_t value, uint16_t *status);

bool img_touched(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
                 int16_t *value);

bool img_blitComtoDisplay(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height,
                          const uint16_t *data);

bool bus_in(const FdsConfig *fdsConfig, uint16_t *busState);

bool bus_out(const FdsConfig *fdsConfig, uint16_t busState);

bool bus_read(const FdsConfig *fdsConfig, uint16_t *busState);

bool bus_set(const FdsConfig *fdsConfig, uint16_t dirMask);

bool bus_write(const FdsConfig *fdsConfig, uint16_t bitfield);

bool pin_hi(const FdsConfig *fdsConfig, uint16_t pin, uint16_t *status);

bool pin_lo(const FdsConfig *fdsConfig, uint16_t pin, uint16_t *status);

bool pin_read(const FdsConfig *fdsConfig, uint16_t pin, uint16_t *status);

// Pin=1..7 mode = 0 .. 1 (see reference manuel)
bool pin_set_picaso(const FdsConfig *fdsConfig, uint16_t mode, uint16_t pin,
                    uint16_t *status);

// Pin = 1..16 mode = 0..6 (see reference manuel)
bool pin_set_diablo(const FdsConfig *fdsConfig, uint16_t mode, uint16_t pin,
                    uint16_t *status);
