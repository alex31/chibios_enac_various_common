/**
 * @file display4DS.h
 * @brief High-level driver for 4D Systems intelligent displays.
 * @details This file provides a generic, thread-safe API to control various 4D Systems
 * display modules (GOLDELOX, PICASO, DIABLO16, PIXXI) and also provides
 * a basic VT100 terminal mode. It abstracts the low-level serial commands
 * into a set of user-friendly functions for text rendering, drawing primitives,
 * touch input, SD card access, and audio playback.
 *
 * The driver can be configured to use either a ChibiOS standard UARTDriver
 * (`FDS_DISPLAY_USE_UART = TRUE`) or a SerialDriver (`FDS_DISPLAY_USE_SD = TRUE`).
 *
 * ### Initialization
 * 1. Call fdsStart() to initialize the driver, which performs a hardware reset,
 *    auto-detects the baud rate (starting at 9600), configures the display,
 *    and sets the final communication speed.
 *
 * ### Thread Safety
 * All public functions are thread-safe. Access to the display is serialized
 * using a mutex. For a sequence of operations, use fdsAcquireLock() and
 * fdsReleaseLock() to ensure atomicity.
 *
 * ### Color Management
 * The driver supports both direct RGB color setting and an indexed color table
 * of size `FDS_COLOR_TABLE_SIZE`. fdsUseColorIndex() selects a pre-configured
 * color from the tables for subsequent text operations.
 *
 * ### Escape Sequences in fdsPrintFmt
 * The fdsPrintFmt() function supports special escape sequences:
 * - `\cn`: (e.g., `\c0`, `\c1`...) Switches to the color profile at index \'n\'.
 * - `\n`: Moves to the next line (newline).
 * - `\t`: Inserts a horizontal tab.
 *
 * @note Most drawing and media functions are not available for the `FDS_TERM_VT100` device type.
 * @note Many advanced features are specific to PICASO, DIABLO16, or PIXXI processors
 *       and are not available on the older GOLDELOX processor. Check individual
 *       function documentation for device compatibility notes.
 */
#pragma once
#include <ch.h>
#include <hal.h>
#include "display4DS_ll.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief If set to TRUE, the driver will use a UARTDriver for communication.
 * @details If FALSE, it will use a SerialDriver.
 */
#if !defined(FDS_DISPLAY_USE_UART)
#define FDS_DISPLAY_USE_UART FALSE
#endif

/**
 * @brief If set to TRUE, the driver will use a SerialDriver for communication.
 */
#define FDS_DISPLAY_USE_SD   (!FDS_DISPLAY_USE_UART)
  
#if  FDS_DISPLAY_USE_SD
#define FDS_LINK_DRIVER SerialDriver
#else
#define FDS_LINK_DRIVER UARTDriver
#endif
  
  /** @brief Main driver structure. Its members are for internal use and should not be accessed directly. */
  typedef struct FdsDriver  FdsDriver;

  /** @brief Enumeration of supported 4D Systems display controller types. */
  enum FdsDriver_Device {
    FDS_GOLDELOX,       /**< GOLDELOX processor. */
    FDS_PICASO,         /**< PICASO processor. */
    FDS_DIABLO16,       /**< DIABLO16 processor. */
    FDS_PIXXI,          /**< PIXXI processor. */
    FDS_AUTO,           /**< Auto-detect device type during initialization. */
    FDS_TERM_VT100      /**< Basic VT100 terminal emulation mode. */
  }; 

  /** @brief Enumeration for text attributes. Can be bitwise-ORed. */
  enum FdsTextAttribute {
    FDS_RESET_ATTRIB=0, /**< Reset all attributes to default. */
    FDS_BOLD=16,        /**< Bold text. */
    FDS_ITALIC=32,      /**< Italic text. */
    FDS_INVERSE=64,     /**< Inverse (swap foreground and background). */
    FDS_UNDERLINE=128   /**< Underlined text. */
  };

  /** @brief Enumeration for screen orientation. */
  enum FdsScreenOrientation {
    FDS_LANDSCAPE=0,        /**< Landscape mode (0 degrees). Default. */
    FDS_LANDSCAPE_REVERSE,  /**< Landscape mode, flipped 180 degrees. */
    FDS_PORTRAIT,           /**< Portrait mode (rotated 90 degrees clockwise). */
    FDS_PORTRAIT_REVERSE    /**< Portrait mode, flipped 180 degrees. */
  };

/** @brief Enumeration for touch screen status. */
typedef enum {
    FDS_NOTOUCH, /**< No touch activity. */
    FDS_PRESS,   /**< The screen was just pressed. */
    FDS_RELEASE, /**< The screen was just released. */
    FDS_MOVING   /**< A touch-and-drag motion is in progress. */
} FdsTouchStatus;

/** @brief General status return values for driver functions. */
typedef enum {
    FDS_OK,    /**< Operation successful. */
    FDS_ERROR  /**< An error occurred during the last serial transaction. */
} FdsStatus;

/** @brief Union to represent a 24-bit color. */
typedef union  {
  struct {
    uint8_t r; /**< Red component. */
    uint8_t g; /**< Green component. */
    uint8_t b; /**< Blue component. */
  };
  uint32_t rgb:24; /**< 24-bit packed color. */
} Color24;

/** @brief Represents a foreground/background color pair. */
typedef struct {
    Color24 fg; /**< Foreground color. */
    Color24 bg; /**< Background color. */
} FdsColor;

/** 
 * @brief Size of the indexed color palette. 
 * @details This defines the number of user-configurable color palette entries, indexed from 0.
 *          The "current" color is stored separately and is not part of this palette.
 */
#ifndef FDS_PALETTE_SIZE
#define FDS_PALETTE_SIZE 10U
#endif

/** @brief Structure to represent a 2D point for polygon drawing. */
typedef struct {
    uint16_t x; /**< X coordinate. */
    uint16_t y; /**< Y coordinate. */
} PolyPoint;

  
/** @brief Internal marker for commands not implemented on a specific device. */
#define CMD_NOT_IMPL 0xbaba

  
/**
 * @brief Starts and initializes the 4D Systems display driver.
 * @details This function performs a hardware reset, initializes the serial interface
 *          at a safe 9600 baud, attempts to identify the display controller,
 *          and then sets the final, higher communication baud rate.
 * @param[in] fdsDriver Pointer to the FdsDriver structure to initialize.
 * @param[in] fds       Pointer to the link driver instance (UARTDriver or SerialDriver).
 * @param[in] baud      The desired final baud rate for communication.
 * @param[in] reset     The I/O line connected to the display\'s reset pin.
 * @param[in] dev       The type of display device. Use FDS_AUTO for auto-detection.
 * @return              `true` on successful initialization, `false` on failure (e.g., incorrect device).
 */
bool fdsStart (FdsDriver *fdsDriver,  FDS_LINK_DRIVER *fds, const uint32_t baud,
		ioline_t reset, enum FdsDriver_Device dev);

/**
 * @brief Performs a hardware reset of the display.
 * @details Toggles the reset line and waits for the display to reboot (approx. 3 seconds).
 *          This function does nothing if the device type is `FDS_TERM_VT100`.
 * @param[in] fdsDriver Pointer to the initialized FdsDriver structure.
 */
void fdsHardReset (FdsDriver *fdsDriver);

/**
 * @brief Verifies communication with the display by sending a clear screen command.
 * @param[in] fdsDriver Pointer to the initialized FdsDriver structure.
 * @return              `true` if the device acknowledges the command, `false` otherwise.
 */
bool fdsIsCorrectDevice (FdsDriver *fdsDriver);

/**
 * @brief Acquires the mutex for exclusive access to the display.
 * @details Use this to group multiple display operations into an atomic sequence.
 * @param[in] fdsDriver Pointer to the initialized FdsDriver structure.
 */
void fdsAcquireLock (FdsDriver *fdsDriver);

/**
 * @brief Releases the mutex, allowing other threads to access the display.
 * @param[in] fdsDriver Pointer to the initialized FdsDriver structure.
 */
void fdsReleaseLock (FdsDriver *fdsDriver);

/**
 * @brief Prints formatted text to the display at the current cursor position.
 * @details This function works like `printf` and supports special escape sequences:
 *          - `\cn`: (e.g. `\c0`, `\c1`, ...) Sets the text color using a pre-defined index.
 *          - `\n`: Moves the cursor to the start of the next line.
 *          - `\t`: Moves the cursor to the next tab stop (8 characters wide).
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] txt       The `printf`-style format string.
 * @param[in] ...       Variable arguments for the format string.
 * @return              `true` on success, `false` on failure.
 */
bool fdsPrintFmt (FdsDriver *fdsDriver, const char *txt, ...)
  __attribute__ ((format (printf, 2, 3)));



/**
 * @brief Prints string with escape sequence to the display at the current cursor position.
 * @details This function supports special escape sequences:
 *          - `\cn`: (e.g. `\c0`, `\c1`, ...) Sets the text color using a pre-defined index.
 *          - `\n`: Moves the cursor to the start of the next line.
 *          - `\t`: Moves the cursor to the next tab stop (8 characters wide).
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] buffer    The null-terminated string to print.
 * @return              `true` on success, `false` on failure.
 */
bool fdsPrintStr (FdsDriver *fdsConfig, const char *str);



/**
 * @brief Prints string to the display at the current cursor position.
 * @details This function does *NOT* supports escape sequences
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] buffer    The null-terminated string to print.
 * @return              `true` on success, `false` on failure.
 */
bool fdsPrintBuffer (FdsDriver *fdsDriver, const char *buffer);

/**
 * @brief Gets hardware and firmware version information from the display.
 * @param[in]  fdsDriver Pointer to the FdsDriver structure.
 * @param[out] buffer    Buffer to store the formatted version string.
 * @param[in]  buflen    Length of the buffer.
 */
void fdsGetVersion (FdsDriver *fdsDriver, char *buffer, const size_t buflen);

/**
 * @brief Changes the screen\'s solid background color.
 * @note This is a slow operation as it replaces every pixel of the old color.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] r         Red component (0-100).
 * @param[in] g         Green component (0-100).
 * @param[in] b         Blue component (0-100).
 */
void fdsChangeBgColor (FdsDriver *fdsDriver, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Sets the current foreground color for text.
 * @details This color is used for subsequent text operations.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] r         Red component (0-100).
 * @param[in] g         Green component (0-100).
 * @param[in] b         Blue component (0-100).
 */
void fdsSetTextFgColor (FdsDriver *fdsDriver, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Sets the current background color for text.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] r         Red component (0-100).
 * @param[in] g         Green component (0-100).
 * @param[in] b         Blue component (0-100).
 */
void fdsSetTextBgColor (FdsDriver *fdsDriver, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Sets the background color for a specific entry in the color palette.
 * @param[in] fdsDriver   Pointer to the FdsDriver structure.
 * @param[in] paletteIndex The index in the palette to set (0 to FDS_PALETTE_SIZE - 1).
 * @param[in] r           Red component (0-100).
 * @param[in] g           Green component (0-100).
 * @param[in] b           Blue component (0-100).
 */
void fdsSetPaletteBgColor (FdsDriver *fdsDriver, uint8_t paletteIndex, 
			      uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Sets the foreground color for a specific entry in the color palette.
 * @param[in] fdsDriver   Pointer to the FdsDriver structure.
 * @param[in] paletteIndex The index in the palette to set (0 to FDS_PALETTE_SIZE - 1).
 * @param[in] r           Red component (0-100).
 * @param[in] g           Green component (0-100).
 * @param[in] b           Blue component (0-100).
 */
void fdsSetPaletteFgColor (FdsDriver *fdsDriver,  uint8_t paletteIndex, 
			      uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Selects a color from the palette to become the new current color.
 * @param[in] fdsDriver   Pointer to the FdsDriver structure.
 * @param[in] paletteIndex  The palette index to use (0 to FDS_PALETTE_SIZE - 1).
 */
void fdsUsePaletteIndex (FdsDriver *fdsDriver, uint8_t paletteIndex);

/**
 * @brief Sets text opacity.
 * @note Only available on GOLDELOX displays.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] opaque    If `true`, text has a solid background. If `false`, it\'s transparent.
 */
void fdsSetTextOpacity (FdsDriver *fdsDriver, bool opaque);

/**
 * @brief Sets text attributes like bold, italic, etc.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] attrib    A bitmask of `FdsTextAttribute` values.
 */
void fdsSetTextAttributeMask (FdsDriver *fdsDriver, enum FdsTextAttribute attrib);

/**
 * @brief Sets the gap between characters and lines.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] xgap      Horizontal gap in pixels (0-32).
 * @param[in] ygap      Vertical gap in pixels (0-32).
 */
void fdsSetTextGap (FdsDriver *fdsDriver, uint8_t xgap, uint8_t ygap);

/**
 * @brief Adjusts the display\'s backlight luminosity by scaling current colors.
 * @details This function does not send a command. It modifies the color values
 *          in the driver\'s state, which are then used for subsequent color-setting
 *          commands. Call `fdsUsePaletteIndex` to apply the new luminosity.
 * @param[in] fdsDriver   Pointer to the FdsDriver structure.
 * @param[in] luminosity  Luminosity level (0-100).
 */
void fdsSetLuminosity (FdsDriver *fdsDriver, uint8_t luminosity);

/**
 * @brief Sets the text size multiplier.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] xmul      Horizontal multiplier (1-16).
 * @param[in] ymul      Vertical multiplier (1-16).
 */
void fdsSetTextSizeMultiplier (FdsDriver *fdsDriver, uint8_t xmul, uint8_t ymul);

/**
 * @brief Sets the screen orientation.
 * @param[in] fdsDriver   Pointer to the FdsDriver structure.
 * @param[in] orientation The desired screen orientation from `FdsScreenOrientation`.
 */
void fdsSetScreenOrientation (FdsDriver *fdsDriver, enum FdsScreenOrientation orientation);

/**
 * @brief Moves the text cursor to a specific character row and column.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] x         Column position (0-based).
 * @param[in] y         Row position (0-based).
 */
void fdsGotoXY (FdsDriver *fdsDriver, uint8_t x, uint8_t y);

/**
 * @brief Moves the text cursor to a specific character column on the current row.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] x         Column position (0-based).
 */
void fdsGotoX (FdsDriver *fdsDriver, uint8_t x);

/**
 * @brief Gets the current column (X) of the text cursor.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @return              The current column.
 */
uint8_t fdsGetX (const FdsDriver *fdsDriver);

/**
 * @brief Gets the current row (Y) of the text cursor.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @return              The current row.
 */
uint8_t fdsGetY (const FdsDriver *fdsDriver);

/**
 * @brief Moves the cursor to the beginning of the next line.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 */
void fdsGotoNextLine (FdsDriver *fdsDriver);

/**
 * @brief Clears the entire screen to the current background color.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 */
void fdsClearScreen (FdsDriver *fdsDriver);

/**
 * @brief Initializes the SD card on the display module.
 * @note Available on PICASO, DIABLO16, and PIXXI displays.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @return              `true` on success, `false` on failure.
 */
bool fdsInitSdCard (FdsDriver *fdsDriver);

/**
 * @brief Draws a single pixel.
 * @param[in] fdsDriver   Pointer to the FdsDriver structure.
 * @param[in] x           X coordinate.
 * @param[in] y           Y coordinate.
 * @param[in] colorIndex  Index of the color to use from the color palette.
 */
void fdsDrawPoint (FdsDriver *fdsDriver, const uint16_t x, 
		    const uint16_t y, const uint8_t colorIndex);

/**
 * @brief Draws a line between two points.
 * @param[in] fdsDriver   Pointer to the FdsDriver structure.
 * @param[in] x1          Starting X coordinate.
 * @param[in] y1          Starting Y coordinate.
 * @param[in] x2          Ending X coordinate.
 * @param[in] y2          Ending Y coordinate.
 * @param[in] colorIndex  Index of the color to use from the color palette.
 */
void fdsDrawLine (FdsDriver *fdsDriver, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2, 
		   const uint8_t colorIndex);

/**
 * @brief Draws a rect3angle.
 * @param[in] fdsDriver   Pointer to the FdsDriver structure.
 * @param[in] x1          Top-left X coordinate.
 * @param[in] y1          Top-left Y coordinate.
 * @param[in] x2          Bottom-right X coordinate.
 * @param[in] y2          Bottom-right Y coordinate.
 * @param[in] filled      If `true`, the rectangle is filled (currently draws outline due to bug).
 * @param[in] colorIndex  Index of the color to use from the color palette.
 */
void fdsDrawRect (FdsDriver *fdsDriver, 
		   const uint16_t x1, const uint16_t y1, 
		   const uint16_t x2, const uint16_t y2, 
		   const bool filled, const uint8_t colorIndex);

/**
 * @brief Draws a polyline (a series of connected lines).
 * @param[in] fdsDriver   Pointer to the FdsDriver structure.
 * @param[in] len         Number of points in the polyline (max 300).
 * @param[in] pp          Pointer to an array of `PolyPoint` structures.
 * @param[in] colorIndex  Index of the color to use from the color palette.
 */
void fdsDrawPolyLine (FdsDriver *fdsDriver, 
		       const uint16_t len,
		       const PolyPoint * const pp,
		       const uint8_t colorIndex);

/**
 * @brief Copies a rectangular area of the screen to another location.
 * @note Available on PICASO, DIABLO16, and PIXXI displays. Not available on GOLDELOX.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] xs        Source top-left X coordinate.
 * @param[in] ys        Source top-left Y coordinate.
 * @param[in] xd        Destination top-left X coordinate.
 * @param[in] yd        Destination top-left Y coordinate.
 * @param[in] width     Width of the area to copy.
 * @param[in] height    Height of the area to copy.
 */
void fdsScreenCopyPaste (FdsDriver *fdsDriver, 
			  const uint16_t xs, const uint16_t ys, 
			  const uint16_t xd, const uint16_t yd,
			  const uint16_t width, const uint16_t height);

/**
 * @brief Enables or disables touch screen reporting.
 * @note Available on PICASO, DIABLO16, and PIXXI displays.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] enable    `true` to enable, `false` to disable.
 */
void fdsEnableTouch (FdsDriver *fdsDriver, bool enable);

/**
 * @brief Gets the current status of the touch screen.
 * @note Available on PICASO, DIABLO16, and PIXXI displays.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @return              The current touch status (`FdsTouchStatus`).
 */
FdsTouchStatus fdsTouchGetStatus (FdsDriver *fdsDriver);

/**
 * @brief Gets the X coordinate of the last touch event.
 * @note Available on PICASO, DIABLO16, and PIXXI displays.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @return              The X coordinate in pixels.
 */
uint16_t fdsTouchGetXcoord (FdsDriver *fdsDriver);

/**
 * @brief Gets the Y coordinate of the last touch event.
 * @note Available on PICASO, DIABLO16, and PIXXI displays.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @return              The Y coordinate in pixels.
 */
uint16_t fdsTouchGetYcoord (FdsDriver *fdsDriver);

/**
 * @brief Lists the contents of the SD card\'s root directory to the debug trace.
 * @note Available on PICASO, DIABLO16, and PIXXI displays.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 */
void fdsListSdCardDirectory (FdsDriver *fdsDriver);

/**
 * @brief Sets the volume for the onboard audio amplifier.
 * @note Available on PICASO, DIABLO16, and PIXXI displays.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] percent   Volume level (0-100).
 */
void fdsSetSoundVolume (FdsDriver *fdsDriver, uint8_t percent);

/**
 * @brief Plays a WAV file from the SD card.
 * @note Available on PICASO, DIABLO16, and PIXXI displays.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] fileName  Name of the WAV file to play (8.3 format).
 */
void fdsPlayWav (FdsDriver *fdsDriver, const char* fileName);
//void fdsPlayBeep (FdsDriver *fdsDriver, uint8_t note, uint16_t duration);

/**
 * @brief Opens a file on the SD card for reading.
 * @note Available on PICASO, DIABLO16, and PIXXI displays.
 * @param[in]  fdsDriver Pointer to the FdsDriver structure.
 * @param[in]  fileName  Name of the file to open (8.3 format).
 * @param[out] handle    Pointer to store the file handle on success.
 * @return               File error code (0 on success).
 */
uint32_t fdsOpenFile  (FdsDriver *fdsDriver, const char* fileName, uint16_t *handle);

/**
 * @brief Closes a previously opened file.
 * @note Available on PICASO, DIABLO16, and PIXXI displays.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] handle    The handle of the file to close.
 */
void fdsCloseFile (FdsDriver *fdsDriver, const uint16_t handle);

/**
 * @brief Displays a GCI (Genie Command Image) from an open file.
 * @note Available on PICASO, DIABLO16, and PIXXI displays.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] handle    Handle of the open file containing the GCI data.
 * @param[in] offset    Offset within the file where the GCI data starts.
 */
void fdsDisplayGci  (FdsDriver *fdsDriver, const uint16_t handle, uint32_t offset);

/**
 * @brief Calls a function in a pre-loaded 4DGL application.
 * @note Available on PICASO, DIABLO16, and PIXXI displays.
 * @param[in]  fdsDriver Pointer to the FdsDriver structure.
 * @param[in]  handle    Handle of the loaded application (from `file_loadFunction`).
 * @param[out] retVal    Pointer to store the return value from the 4DGL function.
 * @param[in]  numArgs   Number of arguments to pass (max 128).
 * @param[in]  ...       Variable arguments (as `int`).
 * @return               `true` on success, `false` on failure.
 */
bool fdsCallFunction(FdsDriver *fdsDriver, uint16_t handle, uint16_t *retVal, const size_t numArgs, ...);

/**
 * @brief Loads and runs a 4DGL application from a file, waiting for it to complete.
 * @note Available on PICASO, DIABLO16, and PIXXI displays.
 * @param[in]  fdsDriver Pointer to the FdsDriver structure.
 * @param[in]  filename  Name of the application file (.4XE).
 * @param[out] retVal    Pointer to store the return value from the application.
 * @param[in]  numArgs   Number of arguments to pass (max 128).
 * @param[in]  ...       Variable arguments (as `int`).
 * @return               `true` on success, `false` on failure.
 */
bool fdsFileRun(FdsDriver *fdsDriver, const char *filename, uint16_t *retVal, const size_t numArgs, ...);

/**
 * @brief Loads and executes a 4DGL application from a file. Similar to fdsFileRun.
 * @note Available on PICASO, DIABLO16, and PIXXI displays.
 * @param[in]  fdsDriver Pointer to the FdsDriver structure.
 * @param[in]  filename  Name of the application file (.4XE).
 * @param[out] retVal    Pointer to store the return value from the application.
 * @param[in]  numArgs   Number of arguments to pass (max 128).
 * @param[in]  ...       Variable arguments (as `int`).
 * @return               `true` on success, `false` on failure.
 */
bool fdsFileExec(FdsDriver *fdsDriver, const char *filename, uint16_t *retVal, const size_t numArgs, ...);

/**
 * @brief Changes the serial communication baud rate.
 * @details Sends the command to the display, then reconfigures the host\'s serial driver.
 * @param[in] fdsDriver Pointer to the FdsDriver structure.
 * @param[in] baud      The new baud rate.
 * @return              `true` if the display acknowledged the change at the new speed, `false` otherwise.
 */
bool fdsSetBaud (FdsDriver *fdsDriver, uint32_t baud);

/**
 * @brief Gets the status of the last serial transaction.
 * @return `FDS_OK` or `FDS_ERROR`.
 */
FdsStatus fdsGetStatus(void);

/**
 * @brief Macro to clamp RGB components and pack them into a 16-bit color value (5-6-5 format).
 * @param[in] r Red component (0-31).
 * @param[in] v Green component (0-63).
 * @param[in] b Blue component (0-31).
 * @return A 16-bit color value.
 */
#define fds_clampColor(r,v,b) ((uint16_t) ((r & 0x1f) <<11 | (v & 0x3f) << 5 | (b & 0x1f)))

/**
 * @brief Macro to convert 0-100 percentage-based RGB values to a 16-bit color value.
 * @param[in] r Red component (0-100).
 * @param[in] v Green component (0-100).
 * @param[in] b Blue component (0-100).
 * @return A 16-bit color value.
 */
#define fds_colorDecTo16b(r,v,b) (fds_clampColor((r*31/100), (v*63/100), (b*31/100)))

  
/**
 * @brief Creates a Color24 structure from RGB components.
 * @param[in] r Red component (0-255).
 * @param[in] g Green component (0-255).
 * @param[in] b Blue component (0-255).
 * @return A `Color24` structure.
 */
static inline Color24 mkColor24 (uint8_t r, uint8_t g, uint8_t b) {
  Color24 ret = {{.r=r, .g=g, .b=b}};
  return ret;
}

/*
#                 _____           _                    _                   
#                |  __ \         (_)                  | |                  
#                | |__) |  _ __   _   __   __   __ _  | |_     ___         
#                |  ___/  | '__| | |  \ \ / /  / _` | | __|   / _ \        
#                | |      | |    | |   \ V /  | (_| | \ |_   |  __/        
#                |_|      |_|    |_|    \_/    \__,_|  \__|   \___|        
*/


/**
 * @brief The main driver instance structure.
 * @details This structure holds the state and configuration for a display instance.
 * Its fields are for internal use only and should not be modified directly.
 */
struct FdsDriver {
#if FDS_DISPLAY_USE_SD
  SerialConfig serialConfig;      /**< @internal Serial driver configuration. */
  BaseSequentialStream *serial;   /**< @internal Pointer to the serial stream. */
#else
  UARTConfig serialConfig;        /**< @internal UART driver configuration. */
  UARTDriver *serial;             /**< @internal Pointer to the UART driver. */
#endif
  mutex_t omutex ;                /**< @internal Mutex for thread-safe access. */
  ioline_t rstLine;               /**< @internal Reset I/O line. */
  enum FdsDriver_Device deviceType; /**< @internal Type of the display controller. */
  // =============
  uint16_t screenBackgroundColor; /**< @internal Global screen background color (16-bit). Used by fdsChangeBgColor. */
  FdsColor currentColor;          /**< @internal Current fg/bg color for text and drawing. */
  FdsColor colorPalette[FDS_PALETTE_SIZE]; /**< @internal Indexed color palette. */
  uint8_t colIdx;                 /**< @internal Current palette index. */
  uint8_t curXpos;                /**< @internal Current cursor X position (character column). */
  uint8_t curYpos;                /**< @internal Current cursor Y position (character row). */
  uint8_t luminosity;             /**< @internal Current backlight luminosity percentage. */
};


#ifdef __cplusplus
}
#endif
