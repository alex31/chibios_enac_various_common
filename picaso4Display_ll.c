#include "/home/alex/DEV/STM32/CHIBIOS/COMMON/various/picaso4Display_ll.h"

/*
    This code has been generated from API description
    All hand modifications will be lost at next generation
*/
static bool gfx_polyxxx(OledConfig *oledConfig, uint16_t cmd, uint16_t n,
                        uint16_t vx[], uint16_t vy[], uint16_t color) {
  struct {
    uint16_t cmd;
    uint16_t n;
  } command1 = {.cmd = cmd, .n = __builtin_bswap16(n)};

  struct {
    uint8_t ack;
  } response;

  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                     sizeof(command1), NULL, 0);

  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__, (uint8_t *)vx,
                     sizeof(vx[0]) * n, NULL, 0);

  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__, (uint8_t *)vy,
                     sizeof(vy[0]) * n, NULL, 0);

  color = __builtin_bswap16(color);
  return oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&color, sizeof(color),
                            (uint8_t *)&response, sizeof(response)) != 0;
}

enum apiEntries_t {
  api_txt_moveCursor,
  api_txt_putCh,
  api_txt_putStr,
  api_txt_charWidth,
  api_txt_charHeight,
  api_txt_Fgcolour,
  api_txt_Bgcolour,
  api_txt_fontID,
  api_txt_widthMult,
  api_txt_heightMult,
  api_txt_xgap,
  api_txt_ygap,
  api_txt_bold,
  api_txt_inverse,
  api_txt_italic,
  api_txt_opacity,
  api_txt_underline,
  api_txt_attributes,
  api_txt_set,
  api_txt_wrap,
  api_gfx_cls,
  api_gfx_changeColour,
  api_gfx_circle,
  api_gfx_circleFilled,
  api_gfx_line,
  api_gfx_rectangle,
  api_gfx_rectangleFilled,
  api_gfx_polyline,
  api_gfx_polygon,
  api_gfx_polygonFilled,
  api_gfx_triangle,
  api_gfx_triangleFilled,
  api_gfx_orbit,
  api_gfx_putPixel,
  api_gfx_getPixel,
  api_gfx_moveTo,
  api_gfx_lineTo,
  api_gfx_clipping,
  api_gfx_clipWindow,
  api_gfx_setClipRegion,
  api_gfx_ellipse,
  api_gfx_ellipseFilled,
  api_gfx_button,
  api_gfx_panel,
  api_gfx_slider,
  api_gfx_screenCopyPaste,
  api_gfx_bevelShadow,
  api_gfx_bevelWidth,
  api_gfx_bGcolour,
  api_gfx_outlineColour,
  api_gfx_contrast,
  api_gfx_frameDelay,
  api_gfx_linePattern,
  api_gfx_screenMode,
  api_gfx_transparency,
  api_gfx_transparentColour,
  api_gfx_set,
  api_gfx_get,
  api_media_init,
  api_media_setAdd,
  api_media_setSector,
  api_media_readByte,
  api_media_readWord,
  api_media_writeByte,
  api_media_writeWord,
  api_media_flush,
  api_media_image,
  api_media_video,
  api_media_videoFrame,
  api_misc_peekB,
  api_misc_pokeB,
  api_misc_peekW,
  api_misc_pokeW,
  api_misc_peekM,
  api_misc_pokeM,
  api_misc_joystick,
  api_misc_beep,
  api_misc_setbaudWait,
  api_sys_sleep,
  api_sys_memFree,
  api_sys_memHeap,
  api_sys_getModel,
  api_sys_getVersion,
  api_sys_getPmmC,
  api_misc_screenSaverTimeout,
  api_misc_screenSaverSpeed,
  api_misc_screenSaverMode,
  api_touch_detectRegion,
  api_touch_set,
  api_touch_get,
  api_file_error,
  api_file_count,
  api_file_dir,
  api_file_findFirst,
  api_file_findFirstRet,
  api_file_findNext,
  api_file_findNextRet,
  api_file_exists,
  api_file_open,
  api_file_close,
  api_file_read,
  api_file_seek,
  api_file_index,
  api_file_tell,
  api_file_write,
  api_file_size,
  api_file_image,
  api_file_screenCapture,
  api_file_putC,
  api_file_getC,
  api_file_putW,
  api_file_getW,
  api_file_putS,
  api_file_getS,
  api_file_erase,
  api_file_rewind,
  api_file_loadFunction,
  api_file_loadImageControl,
  api_file_mount,
  api_file_unmount,
  api_file_playWAV,
  api_file_writeString,
  api_snd_volume,
  api_snd_pitch,
  api_snd_bufSize,
  api_snd_stop,
  api_snd_pause,
  api_snd_continue,
  api_snd_playing,
  api_img_setPosition,
  api_img_enable,
  api_img_disable,
  api_img_darken,
  api_img_lighten,
  api_img_setWord,
  api_img_getWord,
  api_img_show,
  api_img_setAttributes,
  api_img_clearAttributes,
  api_img_touched,
  api_img_blitComtoDisplay,
  api_bus_in,
  api_bus_out,
  api_bus_read,
  api_bus_set,
  api_bus_write,
  api_pin_hi,
  api_pin_lo,
  api_pin_read,
  api_pin_set
};
enum qdsType_t { cmd_goldelox, cmd_picaso, cmd_diablo };

#define CMD_NOT_IMPL 0xbaba
static const uint16_t cmdCodeByType[150][3] = {
    {__builtin_bswap16(0xFFE4), __builtin_bswap16(0xFFE9),
     __builtin_bswap16(0xFFF0)},
    {__builtin_bswap16(0xFFFE), __builtin_bswap16(0xFFFE),
     __builtin_bswap16(0xFFFE)},
    {__builtin_bswap16(0x0006), __builtin_bswap16(0x0018),
     __builtin_bswap16(0x0018)},
    {__builtin_bswap16(0x0002), __builtin_bswap16(0x001E),
     __builtin_bswap16(0x001E)},
    {__builtin_bswap16(0x0001), __builtin_bswap16(0x001D),
     __builtin_bswap16(0x001D)},
    {__builtin_bswap16(0xFF7F), __builtin_bswap16(0xFFE7),
     __builtin_bswap16(0xFFEE)},
    {__builtin_bswap16(0xFF7E), __builtin_bswap16(0xFFE6),
     __builtin_bswap16(0xFFED)},
    {__builtin_bswap16(0xFF7D), __builtin_bswap16(0xFFE5),
     __builtin_bswap16(0xFFEC)},
    {__builtin_bswap16(0xFF7C), __builtin_bswap16(0xFFE4),
     __builtin_bswap16(0xFFEB)},
    {__builtin_bswap16(0xFF7B), __builtin_bswap16(0xFFE3),
     __builtin_bswap16(0xFFEA)},
    {__builtin_bswap16(0xFF7A), __builtin_bswap16(0xFFE2),
     __builtin_bswap16(0xFFE9)},
    {__builtin_bswap16(0xFF79), __builtin_bswap16(0xFFE1),
     __builtin_bswap16(0xFFE8)},
    {__builtin_bswap16(0xFF76), __builtin_bswap16(0xFFDE),
     __builtin_bswap16(0xFFE5)},
    {__builtin_bswap16(0xFF74), __builtin_bswap16(0xFFDC),
     __builtin_bswap16(0xFFE3)},
    {__builtin_bswap16(0xFF75), __builtin_bswap16(0xFFDD),
     __builtin_bswap16(0xFFE4)},
    {__builtin_bswap16(0xFF77), __builtin_bswap16(0xFFDF),
     __builtin_bswap16(0xFFE6)},
    {__builtin_bswap16(0xFF73), __builtin_bswap16(0xFFDB),
     __builtin_bswap16(0xFFE2)},
    {__builtin_bswap16(0xFF72), __builtin_bswap16(0xFFDA),
     __builtin_bswap16(0xFFE1)},
    {__builtin_bswap16(0xFFE3), __builtin_bswap16(CMD_NOT_IMPL),
     __builtin_bswap16(CMD_NOT_IMPL)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD9),
     __builtin_bswap16(0xFFE0)},
    {__builtin_bswap16(0xFFD7), __builtin_bswap16(0xFFCD),
     __builtin_bswap16(0xFF82)},
    {__builtin_bswap16(0xFFBE), __builtin_bswap16(0xFFB4),
     __builtin_bswap16(0xFF69)},
    {__builtin_bswap16(0xFFCD), __builtin_bswap16(0xFFC3),
     __builtin_bswap16(0xFF78)},
    {__builtin_bswap16(0xFFCC), __builtin_bswap16(0xFFC2),
     __builtin_bswap16(0xFF77)},
    {__builtin_bswap16(0xFFD2), __builtin_bswap16(0xFFC8),
     __builtin_bswap16(0xFF7D)},
    {__builtin_bswap16(0xFFCF), __builtin_bswap16(0xFFC5),
     __builtin_bswap16(0xFF7A)},
    {__builtin_bswap16(0xFFCE), __builtin_bswap16(0xFFC4),
     __builtin_bswap16(0xFF79)},
    {__builtin_bswap16(0x0005), __builtin_bswap16(0x0015),
     __builtin_bswap16(0x0015)},
    {__builtin_bswap16(0x0004), __builtin_bswap16(0x0013),
     __builtin_bswap16(0x0013)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0014),
     __builtin_bswap16(0x0014)},
    {__builtin_bswap16(0xFFC9), __builtin_bswap16(0xFFBF),
     __builtin_bswap16(0xFF74)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFA9),
     __builtin_bswap16(0xFF59)},
    {__builtin_bswap16(0x0003), __builtin_bswap16(0x0012),
     __builtin_bswap16(0x0012)},
    {__builtin_bswap16(0xFFCB), __builtin_bswap16(0xFFC1),
     __builtin_bswap16(0xFF76)},
    {__builtin_bswap16(0xFFCA), __builtin_bswap16(0xFFC0),
     __builtin_bswap16(0xFF75)},
    {__builtin_bswap16(0xFFD6), __builtin_bswap16(0xFFCC),
     __builtin_bswap16(0xFF81)},
    {__builtin_bswap16(0xFFD4), __builtin_bswap16(0xFFCA),
     __builtin_bswap16(0xFF7F)},
    {__builtin_bswap16(0xFF6C), __builtin_bswap16(0xFFA2),
     __builtin_bswap16(0xFF46)},
    {__builtin_bswap16(0xFFBF), __builtin_bswap16(0xFFB5),
     __builtin_bswap16(0xFF6A)},
    {__builtin_bswap16(0xFFBC), __builtin_bswap16(0xFFB3),
     __builtin_bswap16(0xFF68)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFB2),
     __builtin_bswap16(0xFF67)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFB1),
     __builtin_bswap16(0xFF66)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0011),
     __builtin_bswap16(0x0011)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFAF),
     __builtin_bswap16(0xFF5F)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFAE),
     __builtin_bswap16(0xFF5E)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFAD),
     __builtin_bswap16(0xFF5D)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF98),
     __builtin_bswap16(0xFF3C)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF99),
     __builtin_bswap16(0xFF3D)},
    {__builtin_bswap16(0xFF6E), __builtin_bswap16(0xFFA4),
     __builtin_bswap16(0xFF48)},
    {__builtin_bswap16(0xFF67), __builtin_bswap16(0xFF9D),
     __builtin_bswap16(0xFF41)},
    {__builtin_bswap16(0xFF66), __builtin_bswap16(0xFF9C),
     __builtin_bswap16(0xFF40)},
    {__builtin_bswap16(0xFF69), __builtin_bswap16(0xFF9F),
     __builtin_bswap16(0xFF43)},
    {__builtin_bswap16(0xFF65), __builtin_bswap16(0xFF9B),
     __builtin_bswap16(0xFF3F)},
    {__builtin_bswap16(0xFF68), __builtin_bswap16(0xFF9E),
     __builtin_bswap16(0xFF42)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFA0),
     __builtin_bswap16(0xFF44)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFA1),
     __builtin_bswap16(0xFF45)},
    {__builtin_bswap16(0xFFD8), __builtin_bswap16(0xFFCE),
     __builtin_bswap16(0xFF83)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFA6),
     __builtin_bswap16(0xFF4A)},
    {__builtin_bswap16(0xFFB1), __builtin_bswap16(0xFF89),
     __builtin_bswap16(0xFF25)},
    {__builtin_bswap16(0xFFB9), __builtin_bswap16(0xFF93),
     __builtin_bswap16(0xFF2F)},
    {__builtin_bswap16(0xFFB8), __builtin_bswap16(0xFF92),
     __builtin_bswap16(0xFF2E)},
    {__builtin_bswap16(0xFFB7), __builtin_bswap16(0xFF8F),
     __builtin_bswap16(0xFF2B)},
    {__builtin_bswap16(0xFFB6), __builtin_bswap16(0xFF8E),
     __builtin_bswap16(0xFF2A)},
    {__builtin_bswap16(0xFFB5), __builtin_bswap16(0xFF8D),
     __builtin_bswap16(0xFF29)},
    {__builtin_bswap16(0xFFB4), __builtin_bswap16(0xFF8C),
     __builtin_bswap16(0xFF28)},
    {__builtin_bswap16(0xFFB2), __builtin_bswap16(0xFF8A),
     __builtin_bswap16(0xFF26)},
    {__builtin_bswap16(0xFFB3), __builtin_bswap16(0xFF8B),
     __builtin_bswap16(0xFF27)},
    {__builtin_bswap16(0xFFBB), __builtin_bswap16(0xFF95),
     __builtin_bswap16(0xFF31)},
    {__builtin_bswap16(0xFFBA), __builtin_bswap16(0xFF94),
     __builtin_bswap16(0xFF30)},
    {__builtin_bswap16(0xFFF6), __builtin_bswap16(CMD_NOT_IMPL),
     __builtin_bswap16(CMD_NOT_IMPL)},
    {__builtin_bswap16(0xFFF4), __builtin_bswap16(CMD_NOT_IMPL),
     __builtin_bswap16(CMD_NOT_IMPL)},
    {__builtin_bswap16(0xFFF5), __builtin_bswap16(CMD_NOT_IMPL),
     __builtin_bswap16(CMD_NOT_IMPL)},
    {__builtin_bswap16(0xFFF3), __builtin_bswap16(CMD_NOT_IMPL),
     __builtin_bswap16(CMD_NOT_IMPL)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0027),
     __builtin_bswap16(0x0027)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0028),
     __builtin_bswap16(0x0028)},
    {__builtin_bswap16(0xFFD9), __builtin_bswap16(CMD_NOT_IMPL),
     __builtin_bswap16(CMD_NOT_IMPL)},
    {__builtin_bswap16(0xFFDA), __builtin_bswap16(CMD_NOT_IMPL),
     __builtin_bswap16(CMD_NOT_IMPL)},
    {__builtin_bswap16(0x000B), __builtin_bswap16(0x0026),
     __builtin_bswap16(0x0026)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF3B),
     __builtin_bswap16(0xFE6D)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF24),
     __builtin_bswap16(0xFE5F)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF23),
     __builtin_bswap16(0xFE5E)},
    {__builtin_bswap16(0x0007), __builtin_bswap16(0x001A),
     __builtin_bswap16(0x001A)},
    {__builtin_bswap16(0x0008), __builtin_bswap16(0x001B),
     __builtin_bswap16(0x001B)},
    {__builtin_bswap16(0x0009), __builtin_bswap16(0x001C),
     __builtin_bswap16(0x001C)},
    {__builtin_bswap16(0x000C), __builtin_bswap16(CMD_NOT_IMPL),
     __builtin_bswap16(CMD_NOT_IMPL)},
    {__builtin_bswap16(0x000D), __builtin_bswap16(CMD_NOT_IMPL),
     __builtin_bswap16(CMD_NOT_IMPL)},
    {__builtin_bswap16(0x000E), __builtin_bswap16(CMD_NOT_IMPL),
     __builtin_bswap16(CMD_NOT_IMPL)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF39),
     __builtin_bswap16(0xFE6A)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF38),
     __builtin_bswap16(0xFE69)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF37),
     __builtin_bswap16(0xFE68)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF1F),
     __builtin_bswap16(0xFE58)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0001),
     __builtin_bswap16(0x0001)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0002),
     __builtin_bswap16(0x0002)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0006),
     __builtin_bswap16(0x0006)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0024),
     __builtin_bswap16(0x0024)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF1B),
     __builtin_bswap16(0xFE54)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0025),
     __builtin_bswap16(0x0025)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0005),
     __builtin_bswap16(0x0005)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x000A),
     __builtin_bswap16(0x000A)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF18),
     __builtin_bswap16(0xFE51)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x000C),
     __builtin_bswap16(0x000C)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF16),
     __builtin_bswap16(0xFE4F)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF15),
     __builtin_bswap16(0xFE4E)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x000F),
     __builtin_bswap16(0x000F)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0010),
     __builtin_bswap16(0x0010)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x000E),
     __builtin_bswap16(0x000E)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF11),
     __builtin_bswap16(0xFE4A)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF10),
     __builtin_bswap16(0xFE49)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x001F),
     __builtin_bswap16(0x001F)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF0E),
     __builtin_bswap16(0xFE47)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF0D),
     __builtin_bswap16(0xFE46)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF0C),
     __builtin_bswap16(0xFE45)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0020),
     __builtin_bswap16(0x0020)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0007),
     __builtin_bswap16(0x0007)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0003),
     __builtin_bswap16(0x0003)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF08),
     __builtin_bswap16(0xFE41)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0008),
     __builtin_bswap16(0x0008)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0009),
     __builtin_bswap16(0x0009)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF03),
     __builtin_bswap16(0xFE3C)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF02),
     __builtin_bswap16(0xFE3B)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x000B),
     __builtin_bswap16(0x000B)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0021),
     __builtin_bswap16(0x0021)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF00),
     __builtin_bswap16(0xFE35)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEFF),
     __builtin_bswap16(0xFE34)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEFE),
     __builtin_bswap16(0xFE33)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEFD),
     __builtin_bswap16(0xFE32)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEFC),
     __builtin_bswap16(0xFE31)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEFB),
     __builtin_bswap16(0xFE30)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEFA),
     __builtin_bswap16(0xFE2F)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF4E),
     __builtin_bswap16(0xFE8A)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF4D),
     __builtin_bswap16(0xFE89)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF4C),
     __builtin_bswap16(0xFE88)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF4B),
     __builtin_bswap16(0xFE87)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF4A),
     __builtin_bswap16(0xFE86)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF49),
     __builtin_bswap16(0xFE85)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF48),
     __builtin_bswap16(0xFE84)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF47),
     __builtin_bswap16(0xFE83)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF46),
     __builtin_bswap16(0xFE82)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF45),
     __builtin_bswap16(0xFE81)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF44),
     __builtin_bswap16(0xFE80)},
    {__builtin_bswap16(0x000A), __builtin_bswap16(0x0023),
     __builtin_bswap16(0x0023)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD3),
     __builtin_bswap16(CMD_NOT_IMPL)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD2),
     __builtin_bswap16(CMD_NOT_IMPL)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFCF),
     __builtin_bswap16(0xFF86)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD1),
     __builtin_bswap16(CMD_NOT_IMPL)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD0),
     __builtin_bswap16(0xFF87)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD6),
     __builtin_bswap16(0xFF8F)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD5),
     __builtin_bswap16(0xFF8E)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD4),
     __builtin_bswap16(0xFF8C)},
    {__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD7),
     __builtin_bswap16(0xFF90)},
};

bool txt_moveCursor(OledConfig *oledConfig, uint16_t line, uint16_t column) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t line;
    uint16_t column;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_moveCursor][oledConfig->deviceType],
              .line = __builtin_bswap16(line),
              .column = __builtin_bswap16(column)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool txt_putCh(OledConfig *oledConfig, uint16_t car) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t car;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_putCh][oledConfig->deviceType],
              .car = __builtin_bswap16(car)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool txt_putStr(OledConfig *oledConfig, const char *cstr, uint16_t *length) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_txt_putStr][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t length;
  } __attribute__((__packed__)) response;

  stus =
      oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                         (uint8_t *)&command1, sizeof(command1), NULL, 0) != 0;
  stus =
      oledTransmitBuffer(
          oledConfig, __FUNCTION__, __LINE__, (uint8_t *)cstr, strlen(cstr) + 1,
          (uint8_t *)&response,
          sizeof(response) -
              (oledConfig->deviceType == GOLDELOX ? sizeof(*length) : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (length != NULL) *length = __builtin_bswap16(response.length);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_charWidth(OledConfig *oledConfig, char car, uint16_t *width) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    char car;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_charWidth][oledConfig->deviceType],
              .car = __builtin_bswap16(car)};

  struct {
    uint8_t ack;
    uint16_t width;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (width != NULL) *width = __builtin_bswap16(response.width);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_charHeight(OledConfig *oledConfig, char car, uint16_t *height) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    char car;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_charHeight][oledConfig->deviceType],
              .car = __builtin_bswap16(car)};

  struct {
    uint8_t ack;
    uint16_t height;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (height != NULL) *height = __builtin_bswap16(response.height);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_Fgcolour(OledConfig *oledConfig, uint16_t color, uint16_t *oldCol) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t color;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_Fgcolour][oledConfig->deviceType],
              .color = __builtin_bswap16(color)};

  struct {
    uint8_t ack;
    uint16_t oldCol;
  } __attribute__((__packed__)) response;

  stus =
      oledTransmitBuffer(
          oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (oledConfig->deviceType == GOLDELOX ? sizeof(*oldCol) : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldCol != NULL) *oldCol = __builtin_bswap16(response.oldCol);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_Bgcolour(OledConfig *oledConfig, uint16_t color, uint16_t *oldCol) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t color;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_Bgcolour][oledConfig->deviceType],
              .color = __builtin_bswap16(color)};

  struct {
    uint8_t ack;
    uint16_t oldCol;
  } __attribute__((__packed__)) response;

  stus =
      oledTransmitBuffer(
          oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (oledConfig->deviceType == GOLDELOX ? sizeof(*oldCol) : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldCol != NULL) *oldCol = __builtin_bswap16(response.oldCol);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_fontID(OledConfig *oledConfig, uint16_t id, uint16_t *oldFont) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t id;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_fontID][oledConfig->deviceType],
              .id = __builtin_bswap16(id)};

  struct {
    uint8_t ack;
    uint16_t oldFont;
  } __attribute__((__packed__)) response;

  stus =
      oledTransmitBuffer(
          oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (oledConfig->deviceType == GOLDELOX ? sizeof(*oldFont) : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldFont != NULL) *oldFont = __builtin_bswap16(response.oldFont);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_widthMult(OledConfig *oledConfig, uint16_t wMultiplier,
                   uint16_t *oldMul) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t wMultiplier;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_widthMult][oledConfig->deviceType],
              .wMultiplier = __builtin_bswap16(wMultiplier)};

  struct {
    uint8_t ack;
    uint16_t oldMul;
  } __attribute__((__packed__)) response;

  stus =
      oledTransmitBuffer(
          oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (oledConfig->deviceType == GOLDELOX ? sizeof(*oldMul) : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldMul != NULL) *oldMul = __builtin_bswap16(response.oldMul);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_heightMult(OledConfig *oledConfig, uint16_t hMultiplier,
                    uint16_t *oldMul) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t hMultiplier;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_heightMult][oledConfig->deviceType],
              .hMultiplier = __builtin_bswap16(hMultiplier)};

  struct {
    uint8_t ack;
    uint16_t oldMul;
  } __attribute__((__packed__)) response;

  stus =
      oledTransmitBuffer(
          oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (oledConfig->deviceType == GOLDELOX ? sizeof(*oldMul) : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldMul != NULL) *oldMul = __builtin_bswap16(response.oldMul);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_xgap(OledConfig *oledConfig, uint16_t xGap, uint16_t *oldGap) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t xGap;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_xgap][oledConfig->deviceType],
              .xGap = __builtin_bswap16(xGap)};

  struct {
    uint8_t ack;
    uint16_t oldGap;
  } __attribute__((__packed__)) response;

  stus =
      oledTransmitBuffer(
          oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (oledConfig->deviceType == GOLDELOX ? sizeof(*oldGap) : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldGap != NULL) *oldGap = __builtin_bswap16(response.oldGap);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_ygap(OledConfig *oledConfig, uint16_t yGap, uint16_t *oldGap) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t yGap;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_ygap][oledConfig->deviceType],
              .yGap = __builtin_bswap16(yGap)};

  struct {
    uint8_t ack;
    uint16_t oldGap;
  } __attribute__((__packed__)) response;

  stus =
      oledTransmitBuffer(
          oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (oledConfig->deviceType == GOLDELOX ? sizeof(*oldGap) : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldGap != NULL) *oldGap = __builtin_bswap16(response.oldGap);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_bold(OledConfig *oledConfig, uint16_t mode, uint16_t *oldBold) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_bold][oledConfig->deviceType],
              .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldBold;
  } __attribute__((__packed__)) response;

  stus =
      oledTransmitBuffer(
          oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (oledConfig->deviceType == GOLDELOX ? sizeof(*oldBold) : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldBold != NULL) *oldBold = __builtin_bswap16(response.oldBold);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_inverse(OledConfig *oledConfig, uint16_t mode, uint16_t *oldInv) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_inverse][oledConfig->deviceType],
              .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldInv;
  } __attribute__((__packed__)) response;

  stus =
      oledTransmitBuffer(
          oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (oledConfig->deviceType == GOLDELOX ? sizeof(*oldInv) : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldInv != NULL) *oldInv = __builtin_bswap16(response.oldInv);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_italic(OledConfig *oledConfig, uint16_t mode, uint16_t *oldItal) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_italic][oledConfig->deviceType],
              .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldItal;
  } __attribute__((__packed__)) response;

  stus =
      oledTransmitBuffer(
          oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (oledConfig->deviceType == GOLDELOX ? sizeof(*oldItal) : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldItal != NULL) *oldItal = __builtin_bswap16(response.oldItal);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_opacity(OledConfig *oledConfig, uint16_t mode, uint16_t *oldOpa) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_opacity][oledConfig->deviceType],
              .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldOpa;
  } __attribute__((__packed__)) response;

  stus =
      oledTransmitBuffer(
          oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (oledConfig->deviceType == GOLDELOX ? sizeof(*oldOpa) : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldOpa != NULL) *oldOpa = __builtin_bswap16(response.oldOpa);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_underline(OledConfig *oledConfig, uint16_t mode, uint16_t *oldUnder) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_underline][oledConfig->deviceType],
              .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldUnder;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(
             oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) - (oledConfig->deviceType == GOLDELOX
                                     ? sizeof(*oldUnder)
                                     : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldUnder != NULL) *oldUnder = __builtin_bswap16(response.oldUnder);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_attributes(OledConfig *oledConfig, uint16_t bitfield,
                    uint16_t *oldAttr) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t bitfield;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_attributes][oledConfig->deviceType],
              .bitfield = __builtin_bswap16(bitfield)};

  struct {
    uint8_t ack;
    uint16_t oldAttr;
  } __attribute__((__packed__)) response;

  stus =
      oledTransmitBuffer(
          oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (oledConfig->deviceType == GOLDELOX ? sizeof(*oldAttr) : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldAttr != NULL) *oldAttr = __builtin_bswap16(response.oldAttr);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_set(OledConfig *oledConfig, uint16_t function, uint16_t value) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t function;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_set][oledConfig->deviceType],
              .function = __builtin_bswap16(function),
              .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function txt_set unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool txt_wrap(OledConfig *oledConfig, uint16_t xpos, uint16_t *oldWrap) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t xpos;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_txt_wrap][oledConfig->deviceType],
              .xpos = __builtin_bswap16(xpos)};

  struct {
    uint8_t ack;
    uint16_t oldWrap;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function txt_wrap unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldWrap != NULL) *oldWrap = __builtin_bswap16(response.oldWrap);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_cls(OledConfig *oledConfig) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_gfx_cls][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_changeColour(OledConfig *oledConfig, uint16_t oldColor,
                      uint16_t newColor) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t oldColor;
    uint16_t newColor;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_gfx_changeColour][oledConfig->deviceType],
      .oldColor = __builtin_bswap16(oldColor),
      .newColor = __builtin_bswap16(newColor)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_circle(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t radius,
                uint16_t color) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t radius;
    uint16_t color;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_circle][oledConfig->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .radius = __builtin_bswap16(radius),
              .color = __builtin_bswap16(color)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_circleFilled(OledConfig *oledConfig, uint16_t x, uint16_t y,
                      uint16_t radius, uint16_t color) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t radius;
    uint16_t color;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_gfx_circleFilled][oledConfig->deviceType],
      .x = __builtin_bswap16(x),
      .y = __builtin_bswap16(y),
      .radius = __builtin_bswap16(radius),
      .color = __builtin_bswap16(color)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_line(OledConfig *oledConfig, uint16_t x1, uint16_t y1, uint16_t x2,
              uint16_t y2, uint16_t color) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
    uint16_t color;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_line][oledConfig->deviceType],
              .x1 = __builtin_bswap16(x1),
              .y1 = __builtin_bswap16(y1),
              .x2 = __builtin_bswap16(x2),
              .y2 = __builtin_bswap16(y2),
              .color = __builtin_bswap16(color)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_rectangle(OledConfig *oledConfig, uint16_t tlx, uint16_t tly,
                   uint16_t brx, uint16_t bry, uint16_t color) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t tlx;
    uint16_t tly;
    uint16_t brx;
    uint16_t bry;
    uint16_t color;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_rectangle][oledConfig->deviceType],
              .tlx = __builtin_bswap16(tlx),
              .tly = __builtin_bswap16(tly),
              .brx = __builtin_bswap16(brx),
              .bry = __builtin_bswap16(bry),
              .color = __builtin_bswap16(color)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_rectangleFilled(OledConfig *oledConfig, uint16_t tlx, uint16_t tly,
                         uint16_t brx, uint16_t bry, uint16_t color) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t tlx;
    uint16_t tly;
    uint16_t brx;
    uint16_t bry;
    uint16_t color;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_gfx_rectangleFilled][oledConfig->deviceType],
      .tlx = __builtin_bswap16(tlx),
      .tly = __builtin_bswap16(tly),
      .brx = __builtin_bswap16(brx),
      .bry = __builtin_bswap16(bry),
      .color = __builtin_bswap16(color)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_polyline(OledConfig *oledConfig, uint16_t n, uint16_t vx[],
                  uint16_t vy[], uint16_t color) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);

  return gfx_polyxxx(oledConfig,
                     cmdCodeByType[api_gfx_polyline][oledConfig->deviceType], n,
                     vx, vy, color);
}
bool gfx_polygon(OledConfig *oledConfig, uint16_t n, uint16_t vx[],
                 uint16_t vy[], uint16_t color) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);

  return gfx_polyxxx(oledConfig,
                     cmdCodeByType[api_gfx_polygon][oledConfig->deviceType], n,
                     vx, vy, color);
}
bool gfx_polygonFilled(OledConfig *oledConfig, uint16_t n, uint16_t vx[],
                       uint16_t vy[], uint16_t color) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  osalDbgAssert(cmdCodeByType[api_gfx_polygonFilled][oledConfig->deviceType] !=
                    CMD_NOT_IMPL,
                "function gfx_polygonFilled unimplemented for this screen");
  return gfx_polyxxx(
      oledConfig, cmdCodeByType[api_gfx_polygonFilled][oledConfig->deviceType],
      n, vx, vy, color);
}
bool gfx_triangle(OledConfig *oledConfig, uint16_t x1, uint16_t y1, uint16_t x2,
                  uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
    uint16_t x3;
    uint16_t y3;
    uint16_t color;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_triangle][oledConfig->deviceType],
              .x1 = __builtin_bswap16(x1),
              .y1 = __builtin_bswap16(y1),
              .x2 = __builtin_bswap16(x2),
              .y2 = __builtin_bswap16(y2),
              .x3 = __builtin_bswap16(x3),
              .y3 = __builtin_bswap16(y3),
              .color = __builtin_bswap16(color)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_triangleFilled(OledConfig *oledConfig, uint16_t x1, uint16_t y1,
                        uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
                        uint16_t color) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
    uint16_t x3;
    uint16_t y3;
    uint16_t color;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_gfx_triangleFilled][oledConfig->deviceType],
      .x1 = __builtin_bswap16(x1),
      .y1 = __builtin_bswap16(y1),
      .x2 = __builtin_bswap16(x2),
      .y2 = __builtin_bswap16(y2),
      .x3 = __builtin_bswap16(x3),
      .y3 = __builtin_bswap16(y3),
      .color = __builtin_bswap16(color)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_triangleFilled unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_orbit(OledConfig *oledConfig, uint16_t angle, uint16_t distance,
               uint16_t *Xdist, uint16_t *Ydist) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t angle;
    uint16_t distance;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_orbit][oledConfig->deviceType],
              .angle = __builtin_bswap16(angle),
              .distance = __builtin_bswap16(distance)};

  struct {
    uint8_t ack;
    uint16_t Xdist;
    uint16_t Ydist;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (Xdist != NULL) *Xdist = __builtin_bswap16(response.Xdist);
    ;
    if (Ydist != NULL) *Ydist = __builtin_bswap16(response.Ydist);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_putPixel(OledConfig *oledConfig, uint16_t x, uint16_t y,
                  uint16_t color) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t color;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_putPixel][oledConfig->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .color = __builtin_bswap16(color)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_getPixel(OledConfig *oledConfig, uint16_t x, uint16_t y,
                  uint16_t *color) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_getPixel][oledConfig->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y)};

  struct {
    uint8_t ack;
    uint16_t color;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (color != NULL) *color = __builtin_bswap16(response.color);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_moveTo(OledConfig *oledConfig, uint16_t x, uint16_t y) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_moveTo][oledConfig->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_lineTo(OledConfig *oledConfig, uint16_t x, uint16_t y) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_lineTo][oledConfig->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_clipping(OledConfig *oledConfig, uint16_t mode) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_clipping][oledConfig->deviceType],
              .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_clipWindow(OledConfig *oledConfig, uint16_t tlx, uint16_t tly,
                    uint16_t brx, uint16_t bry) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t tlx;
    uint16_t tly;
    uint16_t brx;
    uint16_t bry;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_clipWindow][oledConfig->deviceType],
              .tlx = __builtin_bswap16(tlx),
              .tly = __builtin_bswap16(tly),
              .brx = __builtin_bswap16(brx),
              .bry = __builtin_bswap16(bry)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_setClipRegion(OledConfig *oledConfig) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_gfx_setClipRegion][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_ellipse(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t xrad,
                 uint16_t yrad, uint16_t color) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t xrad;
    uint16_t yrad;
    uint16_t color;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_ellipse][oledConfig->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .xrad = __builtin_bswap16(xrad),
              .yrad = __builtin_bswap16(yrad),
              .color = __builtin_bswap16(color)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_ellipse unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_ellipseFilled(OledConfig *oledConfig, uint16_t x, uint16_t y,
                       uint16_t xrad, uint16_t yrad, uint16_t color) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t xrad;
    uint16_t yrad;
    uint16_t color;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_gfx_ellipseFilled][oledConfig->deviceType],
      .x = __builtin_bswap16(x),
      .y = __builtin_bswap16(y),
      .xrad = __builtin_bswap16(xrad),
      .yrad = __builtin_bswap16(yrad),
      .color = __builtin_bswap16(color)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_ellipseFilled unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_button(OledConfig *oledConfig, uint16_t state, uint16_t x, uint16_t y,
                uint16_t buttoncolor, uint16_t txtcolor, uint16_t font,
                uint16_t twtWidth, uint16_t twtHeight, const char *cstr) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t state;
    uint16_t x;
    uint16_t y;
    uint16_t buttoncolor;
    uint16_t txtcolor;
    uint16_t font;
    uint16_t twtWidth;
    uint16_t twtHeight;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_button][oledConfig->deviceType],
              .state = __builtin_bswap16(state),
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .buttoncolor = __builtin_bswap16(buttoncolor),
              .txtcolor = __builtin_bswap16(txtcolor),
              .font = __builtin_bswap16(font),
              .twtWidth = __builtin_bswap16(twtWidth),
              .twtHeight = __builtin_bswap16(twtHeight)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_button unimplemented for this screen");
  stus =
      oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                         (uint8_t *)&command1, sizeof(command1), NULL, 0) != 0;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__, (uint8_t *)cstr,
                            strlen(cstr) + 1, (uint8_t *)&response,
                            sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_panel(OledConfig *oledConfig, uint16_t state, uint16_t x, uint16_t y,
               uint16_t width, uint16_t height, uint16_t color) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t state;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t color;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_panel][oledConfig->deviceType],
              .state = __builtin_bswap16(state),
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .width = __builtin_bswap16(width),
              .height = __builtin_bswap16(height),
              .color = __builtin_bswap16(color)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_panel unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_slider(OledConfig *oledConfig, uint16_t mode, uint16_t x1, uint16_t y1,
                uint16_t x2, uint16_t y2, uint16_t color, uint16_t scale,
                uint16_t value) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
    uint16_t color;
    uint16_t scale;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_slider][oledConfig->deviceType],
              .mode = __builtin_bswap16(mode),
              .x1 = __builtin_bswap16(x1),
              .y1 = __builtin_bswap16(y1),
              .x2 = __builtin_bswap16(x2),
              .y2 = __builtin_bswap16(y2),
              .color = __builtin_bswap16(color),
              .scale = __builtin_bswap16(scale),
              .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_slider unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_screenCopyPaste(OledConfig *oledConfig, uint16_t xs, uint16_t ys,
                         uint16_t xd, uint16_t yd, uint16_t width,
                         uint16_t height) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t xs;
    uint16_t ys;
    uint16_t xd;
    uint16_t yd;
    uint16_t width;
    uint16_t height;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_gfx_screenCopyPaste][oledConfig->deviceType],
      .xs = __builtin_bswap16(xs),
      .ys = __builtin_bswap16(ys),
      .xd = __builtin_bswap16(xd),
      .yd = __builtin_bswap16(yd),
      .width = __builtin_bswap16(width),
      .height = __builtin_bswap16(height)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_screenCopyPaste unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_bevelShadow(OledConfig *oledConfig, uint16_t value,
                     uint16_t *oldStatus) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_bevelShadow][oledConfig->deviceType],
              .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
    uint16_t oldStatus;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_bevelShadow unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldStatus != NULL) *oldStatus = __builtin_bswap16(response.oldStatus);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_bevelWidth(OledConfig *oledConfig, uint16_t value,
                    uint16_t *oldWidth) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_bevelWidth][oledConfig->deviceType],
              .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
    uint16_t oldWidth;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_bevelWidth unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldWidth != NULL) *oldWidth = __builtin_bswap16(response.oldWidth);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_bGcolour(OledConfig *oledConfig, uint16_t color, uint16_t *oldCol) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t color;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_bGcolour][oledConfig->deviceType],
              .color = __builtin_bswap16(color)};

  struct {
    uint8_t ack;
    uint16_t oldCol;
  } __attribute__((__packed__)) response;

  stus =
      oledTransmitBuffer(
          oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (oledConfig->deviceType == GOLDELOX ? sizeof(*oldCol) : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldCol != NULL) *oldCol = __builtin_bswap16(response.oldCol);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_outlineColour(OledConfig *oledConfig, uint16_t color,
                       uint16_t *oldCol) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t color;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_gfx_outlineColour][oledConfig->deviceType],
      .color = __builtin_bswap16(color)};

  struct {
    uint8_t ack;
    uint16_t oldCol;
  } __attribute__((__packed__)) response;

  stus =
      oledTransmitBuffer(
          oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (oledConfig->deviceType == GOLDELOX ? sizeof(*oldCol) : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldCol != NULL) *oldCol = __builtin_bswap16(response.oldCol);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_contrast(OledConfig *oledConfig, uint16_t contrast,
                  uint16_t *oldContrast) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t contrast;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_contrast][oledConfig->deviceType],
              .contrast = __builtin_bswap16(contrast)};

  struct {
    uint8_t ack;
    uint16_t oldContrast;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(
             oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) - (oledConfig->deviceType == GOLDELOX
                                     ? sizeof(*oldContrast)
                                     : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldContrast != NULL)
      *oldContrast = __builtin_bswap16(response.oldContrast);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_frameDelay(OledConfig *oledConfig, uint16_t delayMsec,
                    uint16_t *oldDelay) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t delayMsec;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_frameDelay][oledConfig->deviceType],
              .delayMsec = __builtin_bswap16(delayMsec)};

  struct {
    uint8_t ack;
    uint16_t oldDelay;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(
             oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) - (oledConfig->deviceType == GOLDELOX
                                     ? sizeof(*oldDelay)
                                     : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldDelay != NULL) *oldDelay = __builtin_bswap16(response.oldDelay);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_linePattern(OledConfig *oledConfig, uint16_t pattern,
                     uint16_t *oldPattern) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t pattern;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_linePattern][oledConfig->deviceType],
              .pattern = __builtin_bswap16(pattern)};

  struct {
    uint8_t ack;
    uint16_t oldPattern;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(
             oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) - (oledConfig->deviceType == GOLDELOX
                                     ? sizeof(*oldPattern)
                                     : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldPattern != NULL)
      *oldPattern = __builtin_bswap16(response.oldPattern);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_screenMode(OledConfig *oledConfig, uint16_t mode, uint16_t *oldMode) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_screenMode][oledConfig->deviceType],
              .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldMode;
  } __attribute__((__packed__)) response;

  stus =
      oledTransmitBuffer(
          oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (oledConfig->deviceType == GOLDELOX ? sizeof(*oldMode) : 0)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldMode != NULL) *oldMode = __builtin_bswap16(response.oldMode);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_transparency(OledConfig *oledConfig, uint16_t mode,
                      uint16_t *previous) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_gfx_transparency][oledConfig->deviceType],
      .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t previous;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_transparency unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (previous != NULL) *previous = __builtin_bswap16(response.previous);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_transparentColour(OledConfig *oledConfig, uint16_t color,
                           uint16_t *previous) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t color;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_gfx_transparentColour][oledConfig->deviceType],
      .color = __builtin_bswap16(color)};

  struct {
    uint8_t ack;
    uint16_t previous;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_transparentColour unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (previous != NULL) *previous = __builtin_bswap16(response.previous);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_set(OledConfig *oledConfig, uint16_t function, uint16_t mode) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t function;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_set][oledConfig->deviceType],
              .function = __builtin_bswap16(function),
              .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_get(OledConfig *oledConfig, uint16_t mode, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_gfx_get][oledConfig->deviceType],
              .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_get unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (value != NULL) *value = __builtin_bswap16(response.value);
  }

  return stus && (response.ack == QDS_ACK);
}

bool media_init(OledConfig *oledConfig, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_media_init][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (value != NULL) *value = __builtin_bswap16(response.value);
  }

  return stus && (response.ack == QDS_ACK);
}

bool media_setAdd(OledConfig *oledConfig, uint16_t hiAddr, uint16_t loAddr) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t hiAddr;
    uint16_t loAddr;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_media_setAdd][oledConfig->deviceType],
              .hiAddr = __builtin_bswap16(hiAddr),
              .loAddr = __builtin_bswap16(loAddr)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool media_setSector(OledConfig *oledConfig, uint16_t hiAddr, uint16_t loAddr) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t hiAddr;
    uint16_t loAddr;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_media_setSector][oledConfig->deviceType],
              .hiAddr = __builtin_bswap16(hiAddr),
              .loAddr = __builtin_bswap16(loAddr)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool media_readByte(OledConfig *oledConfig, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_media_readByte][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (value != NULL) *value = __builtin_bswap16(response.value);
  }

  return stus && (response.ack == QDS_ACK);
}

bool media_readWord(OledConfig *oledConfig, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_media_readWord][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (value != NULL) *value = __builtin_bswap16(response.value);
  }

  return stus && (response.ack == QDS_ACK);
}

bool media_writeByte(OledConfig *oledConfig, uint16_t value, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_media_writeByte][oledConfig->deviceType],
              .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool media_writeWord(OledConfig *oledConfig, uint16_t value, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_media_writeWord][oledConfig->deviceType],
              .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool media_flush(OledConfig *oledConfig, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_media_flush][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool media_image(OledConfig *oledConfig, uint16_t x, uint16_t y) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_media_image][oledConfig->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool media_video(OledConfig *oledConfig, uint16_t x, uint16_t y) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_media_video][oledConfig->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool media_videoFrame(OledConfig *oledConfig, uint16_t x, uint16_t y,
                      uint16_t frameNumber) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t frameNumber;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_media_videoFrame][oledConfig->deviceType],
      .x = __builtin_bswap16(x),
      .y = __builtin_bswap16(y),
      .frameNumber = __builtin_bswap16(frameNumber)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_peekB(OledConfig *oledConfig, uint16_t eveRegIndex, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t eveRegIndex;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_misc_peekB][oledConfig->deviceType],
              .eveRegIndex = __builtin_bswap16(eveRegIndex)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_peekB  unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (value != NULL) *value = __builtin_bswap16(response.value);
  }

  return stus && (response.ack == QDS_ACK);
}

bool misc_pokeB(OledConfig *oledConfig, uint16_t eveRegIndex, uint16_t value) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t eveRegIndex;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_misc_pokeB][oledConfig->deviceType],
              .eveRegIndex = __builtin_bswap16(eveRegIndex),
              .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_pokeB  unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_peekW(OledConfig *oledConfig, uint16_t eveRegIndex, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t eveRegIndex;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_misc_peekW][oledConfig->deviceType],
              .eveRegIndex = __builtin_bswap16(eveRegIndex)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_peekW  unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (value != NULL) *value = __builtin_bswap16(response.value);
  }

  return stus && (response.ack == QDS_ACK);
}

bool misc_pokeW(OledConfig *oledConfig, uint16_t eveRegIndex, uint16_t value) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t eveRegIndex;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_misc_pokeW][oledConfig->deviceType],
              .eveRegIndex = __builtin_bswap16(eveRegIndex),
              .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_pokeW unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_peekM(OledConfig *oledConfig, uint16_t address, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t address;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_misc_peekM][oledConfig->deviceType],
              .address = __builtin_bswap16(address)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_peekM  unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (value != NULL) *value = __builtin_bswap16(response.value);
  }

  return stus && (response.ack == QDS_ACK);
}

bool misc_pokeM(OledConfig *oledConfig, uint16_t address, uint16_t value) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t address;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_misc_pokeM][oledConfig->deviceType],
              .address = __builtin_bswap16(address),
              .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_pokeM unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_joystick(OledConfig *oledConfig, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_misc_joystick][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_joystick unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (value != NULL) *value = __builtin_bswap16(response.value);
  }

  return stus && (response.ack == QDS_ACK);
}

bool misc_beep(OledConfig *oledConfig, uint16_t note, uint16_t duration_ms) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t note;
    uint16_t duration_ms;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_misc_beep][oledConfig->deviceType],
              .note = __builtin_bswap16(note),
              .duration_ms = __builtin_bswap16(duration_ms)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_beep unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_setbaudWait(OledConfig *oledConfig, uint16_t index) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t index;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_misc_setbaudWait][oledConfig->deviceType],
      .index = __builtin_bswap16(index)};

  stus =
      oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                         (uint8_t *)&command1, sizeof(command1), NULL, 0) == 0;
  return stus;
}

bool sys_sleep(OledConfig *oledConfig, uint16_t duration_s,
               uint16_t *duration) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t duration_s;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_sys_sleep][oledConfig->deviceType],
              .duration_s = __builtin_bswap16(duration_s)};

  struct {
    uint8_t ack;
    uint16_t duration;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function sys_sleep unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (duration != NULL) *duration = __builtin_bswap16(response.duration);
  }

  return stus && (response.ack == QDS_ACK);
}

bool sys_memFree(OledConfig *oledConfig, uint16_t handle, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_sys_memFree][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function sys_memFree unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (value != NULL) *value = __builtin_bswap16(response.value);
  }

  return stus && (response.ack == QDS_ACK);
}

bool sys_memHeap(OledConfig *oledConfig, uint16_t *avail) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_sys_memHeap][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t avail;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function sys_memHeap unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (avail != NULL) *avail = __builtin_bswap16(response.avail);
  }

  return stus && (response.ack == QDS_ACK);
}

bool sys_getModel(OledConfig *oledConfig, uint16_t *n, char *str) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_sys_getModel][oledConfig->deviceType],
  };

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)str, 3) == 3;
  if ((!stus) || (str[0] != QDS_ACK)) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
    return false;
  }
  const size_t dynSize = MIN(getResponseAsUint16((uint8_t *)str), (*n) - 1);
  *n = dynSize;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__, NULL, 0,
                            (uint8_t *)str, dynSize) == dynSize;
  if (!stus) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
  } else {
    str[dynSize] = 0;
  }

  return stus;
}

bool sys_getVersion(OledConfig *oledConfig, uint16_t *version) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_sys_getVersion][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t version;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (version != NULL) *version = __builtin_bswap16(response.version);
  }

  return stus && (response.ack == QDS_ACK);
}

bool sys_getPmmC(OledConfig *oledConfig, uint16_t *version) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_sys_getPmmC][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t version;
  } __attribute__((__packed__)) response;

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (version != NULL) *version = __builtin_bswap16(response.version);
  }

  return stus && (response.ack == QDS_ACK);
}

bool misc_screenSaverTimeout(OledConfig *oledConfig, uint16_t timout_ms) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t timout_ms;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_misc_screenSaverTimeout][oledConfig->deviceType],
      .timout_ms = __builtin_bswap16(timout_ms)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(
      command1.cmd != CMD_NOT_IMPL,
      "function misc_screenSaverTimeout unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_screenSaverSpeed(OledConfig *oledConfig, uint16_t speed_index) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t speed_index;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_misc_screenSaverSpeed][oledConfig->deviceType],
      .speed_index = __builtin_bswap16(speed_index)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_screenSaverSpeed unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_screenSaverMode(OledConfig *oledConfig, uint16_t mode) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_misc_screenSaverMode][oledConfig->deviceType],
      .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_screenSaverMode unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool touch_detectRegion(OledConfig *oledConfig, uint16_t x1, uint16_t y1,
                        uint16_t x2, uint16_t y2) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_touch_detectRegion][oledConfig->deviceType],
      .x1 = __builtin_bswap16(x1),
      .y1 = __builtin_bswap16(y1),
      .x2 = __builtin_bswap16(x2),
      .y2 = __builtin_bswap16(y2)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function touch_detectRegion unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool touch_set(OledConfig *oledConfig, uint16_t mode) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_touch_set][oledConfig->deviceType],
              .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function touch_set unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool touch_get(OledConfig *oledConfig, uint16_t mode, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_touch_get][oledConfig->deviceType],
              .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function touch_get unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (value != NULL) *value = __builtin_bswap16(response.value);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_error(OledConfig *oledConfig, uint16_t *errno) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_error][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t errno;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_error unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (errno != NULL) *errno = __builtin_bswap16(response.errno);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_count(OledConfig *oledConfig, const char *filename, uint16_t *count) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_count][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t count;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_count unimplemented for this screen");
  stus =
      oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                         (uint8_t *)&command1, sizeof(command1), NULL, 0) != 0;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)filename, strlen(filename) + 1,
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (count != NULL) *count = __builtin_bswap16(response.count);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_dir(OledConfig *oledConfig, const char *filename, uint16_t *count) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_dir][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t count;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_dir unimplemented for this screen");
  stus =
      oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                         (uint8_t *)&command1, sizeof(command1), NULL, 0) != 0;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)filename, strlen(filename) + 1,
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (count != NULL) *count = __builtin_bswap16(response.count);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_findFirst(OledConfig *oledConfig, const char *filename,
                    uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_findFirst][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_findFirst unimplemented for this screen");
  stus =
      oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                         (uint8_t *)&command1, sizeof(command1), NULL, 0) != 0;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)filename, strlen(filename) + 1,
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_findFirstRet(OledConfig *oledConfig, const char *filename,
                       uint16_t *n, char *str) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_findFirstRet][oledConfig->deviceType],
  };

  stus =
      oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                         (uint8_t *)&command1, sizeof(command1), NULL, 0) != 0;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)filename, strlen(filename) + 1,
                            (uint8_t *)str, 3) == 3;
  if ((!stus) || (str[0] != QDS_ACK)) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
    return false;
  }
  const size_t dynSize = MIN(getResponseAsUint16((uint8_t *)str), (*n) - 1);
  *n = dynSize;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__, NULL, 0,
                            (uint8_t *)str, dynSize) == dynSize;
  if (!stus) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
  } else {
    str[dynSize] = 0;
  }

  return stus;
}

bool file_findNext(OledConfig *oledConfig, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_findNext][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_findNext unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_findNextRet(OledConfig *oledConfig, uint16_t *n, char *str) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_findNextRet][oledConfig->deviceType],
  };

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)str, 3) == 3;
  if ((!stus) || (str[0] != QDS_ACK)) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
    return false;
  }
  const size_t dynSize = MIN(getResponseAsUint16((uint8_t *)str), (*n) - 1);
  *n = dynSize;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__, NULL, 0,
                            (uint8_t *)str, dynSize) == dynSize;
  if (!stus) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
  } else {
    str[dynSize] = 0;
  }

  return stus;
}

bool file_exists(OledConfig *oledConfig, const char *filename,
                 uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_exists][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_exists unimplemented for this screen");
  stus =
      oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                         (uint8_t *)&command1, sizeof(command1), NULL, 0) != 0;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)filename, strlen(filename) + 1,
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_open(OledConfig *oledConfig, const char *filename, char mode,
               uint16_t *handle) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_open][oledConfig->deviceType],
  };
  struct {
    char mode;
  } __attribute__((__packed__)) command2 = {.mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t handle;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_open unimplemented for this screen");
  stus =
      oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                         (uint8_t *)&command1, sizeof(command1), NULL, 0) != 0;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)filename, strlen(filename) + 1, NULL,
                            0) != 0;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command2, sizeof(command2),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (handle != NULL) *handle = __builtin_bswap16(response.handle);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_close(OledConfig *oledConfig, uint16_t handle, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_file_close][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_close unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_read(OledConfig *oledConfig, uint16_t size, uint16_t handle,
               uint16_t *n, char *str) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t size;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_file_read][oledConfig->deviceType],
              .size = __builtin_bswap16(size),
              .handle = __builtin_bswap16(handle)};

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)str, 3) == 3;
  if ((!stus) || (str[0] != QDS_ACK)) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
    return false;
  }
  const size_t dynSize = MIN(getResponseAsUint16((uint8_t *)str), (*n) - 1);
  *n = dynSize;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__, NULL, 0,
                            (uint8_t *)str, dynSize) == dynSize;
  if (!stus) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
  } else {
    str[dynSize] = 0;
  }

  return stus;
}

bool file_seek(OledConfig *oledConfig, uint16_t handle, uint16_t hiWord,
               uint16_t loWord, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t hiWord;
    uint16_t loWord;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_file_seek][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle),
              .hiWord = __builtin_bswap16(hiWord),
              .loWord = __builtin_bswap16(loWord)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_seek unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_index(OledConfig *oledConfig, uint16_t handle, uint16_t hiWord,
                uint16_t loWord, uint16_t recordNum, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t hiWord;
    uint16_t loWord;
    uint16_t recordNum;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_file_index][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle),
              .hiWord = __builtin_bswap16(hiWord),
              .loWord = __builtin_bswap16(loWord),
              .recordNum = __builtin_bswap16(recordNum)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_index unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_tell(OledConfig *oledConfig, uint16_t handle, uint16_t *status,
               uint16_t *hiWord, uint16_t *loWord) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_file_tell][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
    uint16_t hiWord;
    uint16_t loWord;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_tell unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
    ;
    if (hiWord != NULL) *hiWord = __builtin_bswap16(response.hiWord);
    ;
    if (loWord != NULL) *loWord = __builtin_bswap16(response.loWord);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_write(OledConfig *oledConfig, uint16_t size, uint16_t source,
                uint16_t handle, uint16_t *count) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t size;
    uint16_t source;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_file_write][oledConfig->deviceType],
              .size = __builtin_bswap16(size),
              .source = __builtin_bswap16(source),
              .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t count;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_write unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (count != NULL) *count = __builtin_bswap16(response.count);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_size(OledConfig *oledConfig, uint16_t handle, uint16_t *status,
               uint16_t *hiWord, uint16_t *loWord) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_file_size][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
    uint16_t hiWord;
    uint16_t loWord;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_size unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
    ;
    if (hiWord != NULL) *hiWord = __builtin_bswap16(response.hiWord);
    ;
    if (loWord != NULL) *loWord = __builtin_bswap16(response.loWord);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_image(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t handle,
                uint16_t *errno) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_file_image][oledConfig->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t errno;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_image unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (errno != NULL) *errno = __builtin_bswap16(response.errno);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_screenCapture(OledConfig *oledConfig, uint16_t x, uint16_t y,
                        uint16_t width, uint16_t height, uint16_t handle,
                        uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_screenCapture][oledConfig->deviceType],
      .x = __builtin_bswap16(x),
      .y = __builtin_bswap16(y),
      .width = __builtin_bswap16(width),
      .height = __builtin_bswap16(height),
      .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_screenCapture unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_putC(OledConfig *oledConfig, uint16_t car, uint16_t handle,
               uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t car;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_file_putC][oledConfig->deviceType],
              .car = __builtin_bswap16(car),
              .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_putC unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_getC(OledConfig *oledConfig, uint16_t handle, uint16_t *car) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_file_getC][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t car;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_getC unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (car != NULL) *car = __builtin_bswap16(response.car);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_putW(OledConfig *oledConfig, uint16_t word, uint16_t handle,
               uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t word;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_file_putW][oledConfig->deviceType],
              .word = __builtin_bswap16(word),
              .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_putW unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_getW(OledConfig *oledConfig, uint16_t handle, uint16_t *word) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_file_getW][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t word;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_getW unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (word != NULL) *word = __builtin_bswap16(response.word);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_putS(OledConfig *oledConfig, const char *cstr, uint16_t *count) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_putS][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t count;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_putS unimplemented for this screen");
  stus =
      oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                         (uint8_t *)&command1, sizeof(command1), NULL, 0) != 0;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__, (uint8_t *)cstr,
                            strlen(cstr) + 1, (uint8_t *)&response,
                            sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (count != NULL) *count = __builtin_bswap16(response.count);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_getS(OledConfig *oledConfig, uint16_t size, uint16_t handle,
               uint16_t *n, char *str) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t size;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_file_getS][oledConfig->deviceType],
              .size = __builtin_bswap16(size),
              .handle = __builtin_bswap16(handle)};

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)str, 3) == 3;
  if ((!stus) || (str[0] != QDS_ACK)) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
    return false;
  }
  const size_t dynSize = MIN(getResponseAsUint16((uint8_t *)str), (*n) - 1);
  *n = dynSize;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__, NULL, 0,
                            (uint8_t *)str, dynSize) == dynSize;
  if (!stus) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
  } else {
    str[dynSize] = 0;
  }

  return stus;
}

bool file_erase(OledConfig *oledConfig, const char *filename,
                uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_erase][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_erase unimplemented for this screen");
  stus =
      oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                         (uint8_t *)&command1, sizeof(command1), NULL, 0) != 0;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)filename, strlen(filename) + 1,
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_rewind(OledConfig *oledConfig, uint16_t handle, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_file_rewind][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_rewind unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_loadFunction(OledConfig *oledConfig, const char *fnname,
                       uint16_t *pointer) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_loadFunction][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t pointer;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_loadFunction unimplemented for this screen");
  stus =
      oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                         (uint8_t *)&command1, sizeof(command1), NULL, 0) != 0;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)fnname, strlen(fnname) + 1,
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (pointer != NULL) *pointer = __builtin_bswap16(response.pointer);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_loadImageControl(OledConfig *oledConfig, const char *filename1,
                           const char *filename2, uint16_t mode,
                           uint16_t *handle) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_loadImageControl][oledConfig->deviceType],
  };
  struct {
    uint16_t mode;
  } __attribute__((__packed__)) command2 = {.mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t handle;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_loadImageControl unimplemented for this screen");
  stus =
      oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                         (uint8_t *)&command1, sizeof(command1), NULL, 0) != 0;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)filename1, strlen(filename1) + 1, NULL,
                            0) != 0;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)filename2, strlen(filename2) + 1, NULL,
                            0) != 0;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command2, sizeof(command2),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (handle != NULL) *handle = __builtin_bswap16(response.handle);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_mount(OledConfig *oledConfig, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_mount][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_mount unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_unmount(OledConfig *oledConfig) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_unmount][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_unmount unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool file_playWAV(OledConfig *oledConfig, const char *wavname,
                  uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_playWAV][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_playWAV unimplemented for this screen");
  stus =
      oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                         (uint8_t *)&command1, sizeof(command1), NULL, 0) != 0;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)wavname, strlen(wavname) + 1,
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool file_writeString(OledConfig *oledConfig, uint16_t handle, const char *cstr,
                      uint16_t *pointer) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_file_writeString][oledConfig->deviceType],
      .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t pointer;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_writeString unimplemented for this screen");
  stus =
      oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                         (uint8_t *)&command1, sizeof(command1), NULL, 0) != 0;
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__, (uint8_t *)cstr,
                            strlen(cstr) + 1, (uint8_t *)&response,
                            sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (pointer != NULL) *pointer = __builtin_bswap16(response.pointer);
  }

  return stus && (response.ack == QDS_ACK);
}

bool snd_volume(OledConfig *oledConfig, uint16_t level) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t level;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_snd_volume][oledConfig->deviceType],
              .level = __builtin_bswap16(level)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_volume unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool snd_pitch(OledConfig *oledConfig, uint16_t rate, uint16_t *oldRate) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t rate;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_snd_pitch][oledConfig->deviceType],
              .rate = __builtin_bswap16(rate)};

  struct {
    uint8_t ack;
    uint16_t oldRate;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_pitch unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (oldRate != NULL) *oldRate = __builtin_bswap16(response.oldRate);
  }

  return stus && (response.ack == QDS_ACK);
}

bool snd_bufSize(OledConfig *oledConfig, uint16_t bufferSize) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t bufferSize;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_snd_bufSize][oledConfig->deviceType],
              .bufferSize = __builtin_bswap16(bufferSize)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_bufSize unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool snd_stop(OledConfig *oledConfig) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_snd_stop][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_stop unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool snd_pause(OledConfig *oledConfig) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_snd_pause][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_pause unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool snd_continue(OledConfig *oledConfig) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_snd_continue][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_continue unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool snd_playing(OledConfig *oledConfig, uint16_t *togo) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_snd_playing][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t togo;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_playing unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (togo != NULL) *togo = __builtin_bswap16(response.togo);
  }

  return stus && (response.ack == QDS_ACK);
}

bool img_setPosition(OledConfig *oledConfig, uint16_t handle, uint16_t index,
                     uint16_t xpos, uint16_t ypos, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t index;
    uint16_t xpos;
    uint16_t ypos;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_img_setPosition][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle),
              .index = __builtin_bswap16(index),
              .xpos = __builtin_bswap16(xpos),
              .ypos = __builtin_bswap16(ypos)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_setPosition unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool img_enable(OledConfig *oledConfig, uint16_t handle, uint16_t index,
                uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t index;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_img_enable][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle),
              .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_enable  unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool img_disable(OledConfig *oledConfig, uint16_t handle, uint16_t index,
                 uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t index;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_img_disable][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle),
              .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_disable unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool img_darken(OledConfig *oledConfig, uint16_t handle, uint16_t index,
                uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t index;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_img_darken][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle),
              .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_darken unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool img_lighten(OledConfig *oledConfig, uint16_t handle, uint16_t index,
                 uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t index;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_img_lighten][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle),
              .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_lighten unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool img_setWord(OledConfig *oledConfig, uint16_t handle, uint16_t index,
                 uint16_t offset, uint16_t value, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t index;
    uint16_t offset;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_img_setWord][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle),
              .index = __builtin_bswap16(index),
              .offset = __builtin_bswap16(offset),
              .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_setWord unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool img_getWord(OledConfig *oledConfig, uint16_t handle, uint16_t index,
                 uint16_t offset, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t index;
    uint16_t offset;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_img_getWord][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle),
              .index = __builtin_bswap16(index),
              .offset = __builtin_bswap16(offset)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_getWord unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (value != NULL) *value = __builtin_bswap16(response.value);
  }

  return stus && (response.ack == QDS_ACK);
}

bool img_show(OledConfig *oledConfig, uint16_t handle, uint16_t index,
              uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t index;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_img_show][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle),
              .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_show unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool img_setAttributes(OledConfig *oledConfig, uint16_t handle, uint16_t index,
                       uint16_t value, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t index;
    uint16_t value;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_img_setAttributes][oledConfig->deviceType],
      .handle = __builtin_bswap16(handle),
      .index = __builtin_bswap16(index),
      .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_setAttributes unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool img_clearAttributes(OledConfig *oledConfig, uint16_t handle,
                         uint16_t index, uint16_t value, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t index;
    uint16_t value;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_img_clearAttributes][oledConfig->deviceType],
      .handle = __builtin_bswap16(handle),
      .index = __builtin_bswap16(index),
      .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_clearAttributes unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool img_touched(OledConfig *oledConfig, uint16_t handle, uint16_t index,
                 uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t index;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_img_touched][oledConfig->deviceType],
              .handle = __builtin_bswap16(handle),
              .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_touched unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool img_blitComtoDisplay(OledConfig *oledConfig, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height,
                          const uint16_t *data) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_img_blitComtoDisplay][oledConfig->deviceType],
      .x = __builtin_bswap16(x),
      .y = __builtin_bswap16(y),
      .width = __builtin_bswap16(width),
      .height = __builtin_bswap16(height)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_blitComtoDisplay unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                     sizeof(command1), NULL, 0);

  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__, (uint8_t *)data,
                            sizeof(*data) * width * height,
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus;
}

bool bus_in(OledConfig *oledConfig, uint16_t *busState) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_bus_in][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t busState;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function bus_in unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (busState != NULL) *busState = __builtin_bswap16(response.busState);
  }

  return stus && (response.ack == QDS_ACK);
}

bool bus_out(OledConfig *oledConfig, uint16_t busState) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t busState;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_bus_out][oledConfig->deviceType],
              .busState = __builtin_bswap16(busState)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function bus_out unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool bus_read(OledConfig *oledConfig, uint16_t *busState) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmdCodeByType[api_bus_read][oledConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t busState;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function bus_read unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (busState != NULL) *busState = __builtin_bswap16(response.busState);
  }

  return stus && (response.ack == QDS_ACK);
}

bool bus_set(OledConfig *oledConfig, uint16_t dirMask) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t dirMask;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_bus_set][oledConfig->deviceType],
              .dirMask = __builtin_bswap16(dirMask)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function bus_set unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool bus_write(OledConfig *oledConfig, uint16_t mask) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mask;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_bus_write][oledConfig->deviceType],
              .mask = __builtin_bswap16(mask)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function bus_write unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool pin_hi(OledConfig *oledConfig, uint16_t pin, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t pin;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_pin_hi][oledConfig->deviceType],
              .pin = __builtin_bswap16(pin)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function pin_hi unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool pin_lo(OledConfig *oledConfig, uint16_t pin, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t pin;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_pin_lo][oledConfig->deviceType],
              .pin = __builtin_bswap16(pin)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function pin_lo unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool pin_read(OledConfig *oledConfig, uint16_t pin, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t pin;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_pin_read][oledConfig->deviceType],
              .pin = __builtin_bswap16(pin)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function pin_read unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}

bool pin_set(OledConfig *oledConfig, uint16_t mode, uint16_t pin,
             uint16_t *status) {
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
    uint16_t pin;
  } __attribute__((__packed__))
  command1 = {.cmd = cmdCodeByType[api_pin_set][oledConfig->deviceType],
              .mode = __builtin_bswap16(mode),
              .pin = __builtin_bswap16(pin)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function pin_set unimplemented for this screen");
  stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
                            (uint8_t *)&command1, sizeof(command1),
                            (uint8_t *)&response, sizeof(response)) != 0;

  if (oledConfig->deviceType != GOLDELOX) {
    if (status != NULL) *status = __builtin_bswap16(response.status);
  }

  return stus && (response.ack == QDS_ACK);
}
