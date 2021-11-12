#include "/home/alex/DEV/STM32/CHIBIOS/COMMON/various/picaso4Display_ll.h"

/*
    This code has been generated from API description
    All hand modifications will be lost at next generation 
*/
    
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
api_media_readSector,
api_media_writeSector,
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
api_misc_blitComtoDisplay,
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
api_touch_get
};
enum qdsType_t {
cmd_goldelox, cmd_picaso, cmd_diablo
};

#define CMD_NOT_IMPL 0xbaba
static  const uint16_t cmdCodeByType[93][3] = {
{__builtin_bswap16(0xFFE4), __builtin_bswap16(0xFFE9),  __builtin_bswap16(0xFFF0)},
{__builtin_bswap16(0xFFFE), __builtin_bswap16(0xFFFE),  __builtin_bswap16(0xFFFE)},
{__builtin_bswap16(0x0006), __builtin_bswap16(0x0018),  __builtin_bswap16(0x0018)},
{__builtin_bswap16(0x0002), __builtin_bswap16(0x001E),  __builtin_bswap16(0x001E)},
{__builtin_bswap16(0x0001), __builtin_bswap16(0x001D),  __builtin_bswap16(0x001D)},
{__builtin_bswap16(0xFF7F), __builtin_bswap16(0xFFE7),  __builtin_bswap16(0xFFEE)},
{__builtin_bswap16(0xFF7E), __builtin_bswap16(0xFFE6),  __builtin_bswap16(0xFFED)},
{__builtin_bswap16(0xFF7D), __builtin_bswap16(0xFFE5),  __builtin_bswap16(0xFFEC)},
{__builtin_bswap16(0xFF7C), __builtin_bswap16(0xFFE4),  __builtin_bswap16(0xFFEB)},
{__builtin_bswap16(0xFF7B), __builtin_bswap16(0xFFE3),  __builtin_bswap16(0xFFEA)},
{__builtin_bswap16(0xFF7A), __builtin_bswap16(0xFFE2),  __builtin_bswap16(0xFFE9)},
{__builtin_bswap16(0xFF79), __builtin_bswap16(0xFFE1),  __builtin_bswap16(0xFFE8)},
{__builtin_bswap16(0xFF76), __builtin_bswap16(0xFFDE),  __builtin_bswap16(0xFFE5)},
{__builtin_bswap16(0xFF74), __builtin_bswap16(0xFFDC),  __builtin_bswap16(0xFFE3)},
{__builtin_bswap16(0xFF75), __builtin_bswap16(0xFFDD),  __builtin_bswap16(0xFFE4)},
{__builtin_bswap16(0xFF77), __builtin_bswap16(0xFFDF),  __builtin_bswap16(0xFFE6)},
{__builtin_bswap16(0xFF73), __builtin_bswap16(0xFFDB),  __builtin_bswap16(0xFFE2)},
{__builtin_bswap16(0xFF72), __builtin_bswap16(0xFFDA),  __builtin_bswap16(0xFFE1)},
{__builtin_bswap16(0xFFE3), __builtin_bswap16(CMD_NOT_IMPL),  __builtin_bswap16(CMD_NOT_IMPL)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD9),  __builtin_bswap16(0xFFE0)},
{__builtin_bswap16(0xFFD7), __builtin_bswap16(0xFFCD),  __builtin_bswap16(0xFF82)},
{__builtin_bswap16(0xFFBE), __builtin_bswap16(0xFFB4),  __builtin_bswap16(0xFF69)},
{__builtin_bswap16(0xFFCD), __builtin_bswap16(0xFFC3),  __builtin_bswap16(0xFF78)},
{__builtin_bswap16(0xFFCC), __builtin_bswap16(0xFFC2),  __builtin_bswap16(0xFF77)},
{__builtin_bswap16(0xFFD2), __builtin_bswap16(0xFFC8),  __builtin_bswap16(0xFF7D)},
{__builtin_bswap16(0xFFCF), __builtin_bswap16(0xFFC5),  __builtin_bswap16(0xFF7A)},
{__builtin_bswap16(0xFFCE), __builtin_bswap16(0xFFC4),  __builtin_bswap16(0xFF79)},
{__builtin_bswap16(0x0005), __builtin_bswap16(0x0015),  __builtin_bswap16(0x0015)},
{__builtin_bswap16(0x0004), __builtin_bswap16(0x0013),  __builtin_bswap16(0x0013)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0014),  __builtin_bswap16(0x0014)},
{__builtin_bswap16(0xFFC9), __builtin_bswap16(0xFFBF),  __builtin_bswap16(0xFF74)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFA9),  __builtin_bswap16(0xFF59)},
{__builtin_bswap16(0x0003), __builtin_bswap16(0x0012),  __builtin_bswap16(0x0012)},
{__builtin_bswap16(0xFFCB), __builtin_bswap16(0xFFC1),  __builtin_bswap16(0xFF76)},
{__builtin_bswap16(0xFFCA), __builtin_bswap16(0xFFC0),  __builtin_bswap16(0xFF75)},
{__builtin_bswap16(0xFFD6), __builtin_bswap16(0xFFCC),  __builtin_bswap16(0xFF81)},
{__builtin_bswap16(0xFFD4), __builtin_bswap16(0xFFCA),  __builtin_bswap16(0xFF7F)},
{__builtin_bswap16(0xFF6C), __builtin_bswap16(0xFFA2),  __builtin_bswap16(0xFF46)},
{__builtin_bswap16(0xFFBF), __builtin_bswap16(0xFFB5),  __builtin_bswap16(0xFF6A)},
{__builtin_bswap16(0xFFBC), __builtin_bswap16(0xFFB3),  __builtin_bswap16(0xFF68)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFB2),  __builtin_bswap16(0xFF67)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFB1),  __builtin_bswap16(0xFF66)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0011),  __builtin_bswap16(0x0011)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFAF),  __builtin_bswap16(0xFF5F)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFAE),  __builtin_bswap16(0xFF5E)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFAD),  __builtin_bswap16(0xFF5D)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF98),  __builtin_bswap16(0xFF3C)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF99),  __builtin_bswap16(0xFF3D)},
{__builtin_bswap16(0xFF6E), __builtin_bswap16(0xFFA4),  __builtin_bswap16(0xFF48)},
{__builtin_bswap16(0xFF67), __builtin_bswap16(0xFF9D),  __builtin_bswap16(0xFF41)},
{__builtin_bswap16(0xFF66), __builtin_bswap16(0xFF9C),  __builtin_bswap16(0xFF40)},
{__builtin_bswap16(0xFF69), __builtin_bswap16(0xFF9F),  __builtin_bswap16(0xFF43)},
{__builtin_bswap16(0xFF65), __builtin_bswap16(0xFF9B),  __builtin_bswap16(0xFF3F)},
{__builtin_bswap16(0xFF68), __builtin_bswap16(0xFF9E),  __builtin_bswap16(0xFF42)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFA0),  __builtin_bswap16(0xFF44)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFA1),  __builtin_bswap16(0xFF45)},
{__builtin_bswap16(0xFFD8), __builtin_bswap16(0xFFCE),  __builtin_bswap16(0xFF83)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFA6),  __builtin_bswap16(0xFF4A)},
{__builtin_bswap16(0xFFB1), __builtin_bswap16(0xFF89),  __builtin_bswap16(0xFF25)},
{__builtin_bswap16(0xFFB9), __builtin_bswap16(0xFF93),  __builtin_bswap16(0xFF2F)},
{__builtin_bswap16(0xFFB8), __builtin_bswap16(0xFF92),  __builtin_bswap16(0xFF2E)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0016),  __builtin_bswap16(0x0016)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0017),  __builtin_bswap16(0x0017)},
{__builtin_bswap16(0xFFB7), __builtin_bswap16(0xFF8F),  __builtin_bswap16(0xFF2B)},
{__builtin_bswap16(0xFFB6), __builtin_bswap16(0xFF8E),  __builtin_bswap16(0xFF2A)},
{__builtin_bswap16(0xFFB5), __builtin_bswap16(0xFF8D),  __builtin_bswap16(0xFF29)},
{__builtin_bswap16(0xFFB4), __builtin_bswap16(0xFF8C),  __builtin_bswap16(0xFF28)},
{__builtin_bswap16(0xFFB2), __builtin_bswap16(0xFF8A),  __builtin_bswap16(0xFF26)},
{__builtin_bswap16(0xFFB3), __builtin_bswap16(0xFF8B),  __builtin_bswap16(0xFF27)},
{__builtin_bswap16(0xFFBB), __builtin_bswap16(0xFF95),  __builtin_bswap16(0xFF31)},
{__builtin_bswap16(0xFFBA), __builtin_bswap16(0xFF94),  __builtin_bswap16(0xFF30)},
{__builtin_bswap16(0xFFF6), __builtin_bswap16(CMD_NOT_IMPL),  __builtin_bswap16(CMD_NOT_IMPL)},
{__builtin_bswap16(0xFFF4), __builtin_bswap16(CMD_NOT_IMPL),  __builtin_bswap16(CMD_NOT_IMPL)},
{__builtin_bswap16(0xFFF5), __builtin_bswap16(CMD_NOT_IMPL),  __builtin_bswap16(CMD_NOT_IMPL)},
{__builtin_bswap16(0xFFF3), __builtin_bswap16(CMD_NOT_IMPL),  __builtin_bswap16(CMD_NOT_IMPL)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0027),  __builtin_bswap16(0x0027)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0028),  __builtin_bswap16(CMD_NOT_IMPL)},
{__builtin_bswap16(0xFFD9), __builtin_bswap16(CMD_NOT_IMPL),  __builtin_bswap16(CMD_NOT_IMPL)},
{__builtin_bswap16(0xFFDA), __builtin_bswap16(CMD_NOT_IMPL),  __builtin_bswap16(CMD_NOT_IMPL)},
{__builtin_bswap16(0x000B), __builtin_bswap16(0x0026),  __builtin_bswap16(0x0026)},
{__builtin_bswap16(0x000A), __builtin_bswap16(CMD_NOT_IMPL),  __builtin_bswap16(CMD_NOT_IMPL)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF3B),  __builtin_bswap16(0xFE6D)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF24),  __builtin_bswap16(0xFE5F)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF23),  __builtin_bswap16(0xFE5E)},
{__builtin_bswap16(0x0007), __builtin_bswap16(0x001A),  __builtin_bswap16(0x001A)},
{__builtin_bswap16(0x0008), __builtin_bswap16(0x001B),  __builtin_bswap16(0x001B)},
{__builtin_bswap16(0x0009), __builtin_bswap16(0x001C),  __builtin_bswap16(0x001C)},
{__builtin_bswap16(0x000C), __builtin_bswap16(CMD_NOT_IMPL),  __builtin_bswap16(CMD_NOT_IMPL)},
{__builtin_bswap16(0x000D), __builtin_bswap16(CMD_NOT_IMPL),  __builtin_bswap16(CMD_NOT_IMPL)},
{__builtin_bswap16(0x000E), __builtin_bswap16(CMD_NOT_IMPL),  __builtin_bswap16(CMD_NOT_IMPL)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF39),  __builtin_bswap16(0xFE6A)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF38),  __builtin_bswap16(0xFE69)},
{__builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF37),  __builtin_bswap16(0xFE68)},
};

void txt_moveCursor(OledConfig *oledConfig, uint16_t line, uint16_t column)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t line;
  uint16_t column;
} command = {
  .cmd = cmdCodeByType[api_txt_moveCursor][oledConfig->deviceType],
  .line = line,
  .column = column
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void txt_putCh(OledConfig *oledConfig, uint16_t car)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t car;
} command = {
  .cmd = cmdCodeByType[api_txt_putCh][oledConfig->deviceType],
  .car = car
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void txt_putStr(OledConfig *oledConfig, char* str0)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  char* str0;
} command = {
  .cmd = cmdCodeByType[api_txt_putStr][oledConfig->deviceType],
  .str0 = str0
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void txt_charWidth(OledConfig *oledConfig, char car, int16_t *width)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  char car;
} command = {
  .cmd = cmdCodeByType[api_txt_charWidth][oledConfig->deviceType],
  .car = car
};	

  struct {
  uint8_t ack;
  int16_t width;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *width = response.width;

}

void txt_charHeight(OledConfig *oledConfig, char car, int16_t *height)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  char car;
} command = {
  .cmd = cmdCodeByType[api_txt_charHeight][oledConfig->deviceType],
  .car = car
};	

  struct {
  uint8_t ack;
  int16_t height;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *height = response.height;

}

void txt_Fgcolour(OledConfig *oledConfig, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_txt_Fgcolour][oledConfig->deviceType],
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void txt_Bgcolour(OledConfig *oledConfig, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_txt_Bgcolour][oledConfig->deviceType],
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void txt_fontID(OledConfig *oledConfig, uint16_t id)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t id;
} command = {
  .cmd = cmdCodeByType[api_txt_fontID][oledConfig->deviceType],
  .id = id
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void txt_widthMult(OledConfig *oledConfig, uint16_t wMultiplier)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t wMultiplier;
} command = {
  .cmd = cmdCodeByType[api_txt_widthMult][oledConfig->deviceType],
  .wMultiplier = wMultiplier
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void txt_heightMult(OledConfig *oledConfig, uint16_t hMultiplier)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t hMultiplier;
} command = {
  .cmd = cmdCodeByType[api_txt_heightMult][oledConfig->deviceType],
  .hMultiplier = hMultiplier
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void txt_xgap(OledConfig *oledConfig, uint16_t xGap)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t xGap;
} command = {
  .cmd = cmdCodeByType[api_txt_xgap][oledConfig->deviceType],
  .xGap = xGap
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void txt_ygap(OledConfig *oledConfig, uint16_t yGap)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t yGap;
} command = {
  .cmd = cmdCodeByType[api_txt_ygap][oledConfig->deviceType],
  .yGap = yGap
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void txt_bold(OledConfig *oledConfig, uint16_t mode)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t mode;
} command = {
  .cmd = cmdCodeByType[api_txt_bold][oledConfig->deviceType],
  .mode = mode
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void txt_inverse(OledConfig *oledConfig, uint16_t mode)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t mode;
} command = {
  .cmd = cmdCodeByType[api_txt_inverse][oledConfig->deviceType],
  .mode = mode
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void txt_italic(OledConfig *oledConfig, uint16_t mode)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t mode;
} command = {
  .cmd = cmdCodeByType[api_txt_italic][oledConfig->deviceType],
  .mode = mode
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void txt_opacity(OledConfig *oledConfig, uint16_t mode)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t mode;
} command = {
  .cmd = cmdCodeByType[api_txt_opacity][oledConfig->deviceType],
  .mode = mode
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void txt_underline(OledConfig *oledConfig, uint16_t mode)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t mode;
} command = {
  .cmd = cmdCodeByType[api_txt_underline][oledConfig->deviceType],
  .mode = mode
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void txt_attributes(OledConfig *oledConfig, uint16_t bitfield)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t bitfield;
} command = {
  .cmd = cmdCodeByType[api_txt_attributes][oledConfig->deviceType],
  .bitfield = bitfield
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void txt_set(OledConfig *oledConfig, uint16_t function, uint16_t value)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t function;
  uint16_t value;
} command = {
  .cmd = cmdCodeByType[api_txt_set][oledConfig->deviceType],
  .function = function,
  .value = value
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function txt_set unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void txt_wrap(OledConfig *oledConfig, uint16_t xpos)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t xpos;
} command = {
  .cmd = cmdCodeByType[api_txt_wrap][oledConfig->deviceType],
  .xpos = xpos
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function txt_wrap unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_cls(OledConfig *oledConfig)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  
} command = {
  .cmd = cmdCodeByType[api_gfx_cls][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_changeColour(OledConfig *oledConfig, uint16_t oldColor, uint16_t newColor)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t oldColor;
  uint16_t newColor;
} command = {
  .cmd = cmdCodeByType[api_gfx_changeColour][oledConfig->deviceType],
  .oldColor = oldColor,
  .newColor = newColor
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_circle(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t radius, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
  uint16_t radius;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_gfx_circle][oledConfig->deviceType],
  .x = x,
  .y = y,
  .radius = radius,
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_circleFilled(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t radius, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
  uint16_t radius;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_gfx_circleFilled][oledConfig->deviceType],
  .x = x,
  .y = y,
  .radius = radius,
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_line(OledConfig *oledConfig, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t x1;
  uint16_t y1;
  uint16_t x2;
  uint16_t y2;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_gfx_line][oledConfig->deviceType],
  .x1 = x1,
  .y1 = y1,
  .x2 = x2,
  .y2 = y2,
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_rectangle(OledConfig *oledConfig, uint16_t tlx, uint16_t tly, uint16_t brx, uint16_t bry, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t tlx;
  uint16_t tly;
  uint16_t brx;
  uint16_t bry;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_gfx_rectangle][oledConfig->deviceType],
  .tlx = tlx,
  .tly = tly,
  .brx = brx,
  .bry = bry,
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_rectangleFilled(OledConfig *oledConfig, uint16_t tlx, uint16_t tly, uint16_t brx, uint16_t bry, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t tlx;
  uint16_t tly;
  uint16_t brx;
  uint16_t bry;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_gfx_rectangleFilled][oledConfig->deviceType],
  .tlx = tlx,
  .tly = tly,
  .brx = brx,
  .bry = bry,
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_polyline(OledConfig *oledConfig, uint16_t n, uint16_t vx, uint16_t vy, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t n;
  uint16_t vx;
  uint16_t vy;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_gfx_polyline][oledConfig->deviceType],
  .n = n,
  .vx = vx,
  .vy = vy,
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_polygon(OledConfig *oledConfig, uint16_t n, uint16_t vx, uint16_t vy, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t n;
  uint16_t vx;
  uint16_t vy;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_gfx_polygon][oledConfig->deviceType],
  .n = n,
  .vx = vx,
  .vy = vy,
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_polygonFilled(OledConfig *oledConfig, uint16_t n, uint16_t vx, uint16_t vy, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t n;
  uint16_t vx;
  uint16_t vy;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_gfx_polygonFilled][oledConfig->deviceType],
  .n = n,
  .vx = vx,
  .vy = vy,
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_polygonFilled unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_triangle(OledConfig *oledConfig, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t x1;
  uint16_t y1;
  uint16_t x2;
  uint16_t y2;
  uint16_t x3;
  uint16_t y3;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_gfx_triangle][oledConfig->deviceType],
  .x1 = x1,
  .y1 = y1,
  .x2 = x2,
  .y2 = y2,
  .x3 = x3,
  .y3 = y3,
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_triangleFilled(OledConfig *oledConfig, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t x1;
  uint16_t y1;
  uint16_t x2;
  uint16_t y2;
  uint16_t x3;
  uint16_t y3;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_gfx_triangleFilled][oledConfig->deviceType],
  .x1 = x1,
  .y1 = y1,
  .x2 = x2,
  .y2 = y2,
  .x3 = x3,
  .y3 = y3,
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_triangleFilled unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_orbit(OledConfig *oledConfig, uint16_t angle, uint16_t distance, int16_t *Xdist, int16_t *Ydist)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t angle;
  uint16_t distance;
} command = {
  .cmd = cmdCodeByType[api_gfx_orbit][oledConfig->deviceType],
  .angle = angle,
  .distance = distance
};	

  struct {
  uint8_t ack;
  int16_t Xdist;
  int16_t Ydist;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *Xdist = response.Xdist;;
    *Ydist = response.Ydist;

}

void gfx_putPixel(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_gfx_putPixel][oledConfig->deviceType],
  .x = x,
  .y = y,
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_getPixel(OledConfig *oledConfig, uint16_t x, uint16_t y, int16_t *color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
} command = {
  .cmd = cmdCodeByType[api_gfx_getPixel][oledConfig->deviceType],
  .x = x,
  .y = y
};	

  struct {
  uint8_t ack;
  int16_t color;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *color = response.color;

}

void gfx_moveTo(OledConfig *oledConfig, uint16_t x, uint16_t y)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
} command = {
  .cmd = cmdCodeByType[api_gfx_moveTo][oledConfig->deviceType],
  .x = x,
  .y = y
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_lineTo(OledConfig *oledConfig, uint16_t x, uint16_t y)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
} command = {
  .cmd = cmdCodeByType[api_gfx_lineTo][oledConfig->deviceType],
  .x = x,
  .y = y
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_clipping(OledConfig *oledConfig, uint16_t mode)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t mode;
} command = {
  .cmd = cmdCodeByType[api_gfx_clipping][oledConfig->deviceType],
  .mode = mode
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_clipWindow(OledConfig *oledConfig, uint16_t tlx, uint16_t tly, uint16_t brx, uint16_t bry)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t tlx;
  uint16_t tly;
  uint16_t brx;
  uint16_t bry;
} command = {
  .cmd = cmdCodeByType[api_gfx_clipWindow][oledConfig->deviceType],
  .tlx = tlx,
  .tly = tly,
  .brx = brx,
  .bry = bry
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_setClipRegion(OledConfig *oledConfig)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  
} command = {
  .cmd = cmdCodeByType[api_gfx_setClipRegion][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_ellipse(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t xrad, uint16_t yrad, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
  uint16_t xrad;
  uint16_t yrad;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_gfx_ellipse][oledConfig->deviceType],
  .x = x,
  .y = y,
  .xrad = xrad,
  .yrad = yrad,
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_ellipse unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_ellipseFilled(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t xrad, uint16_t yrad, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
  uint16_t xrad;
  uint16_t yrad;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_gfx_ellipseFilled][oledConfig->deviceType],
  .x = x,
  .y = y,
  .xrad = xrad,
  .yrad = yrad,
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_ellipseFilled unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_button(OledConfig *oledConfig, uint16_t state, uint16_t x, uint16_t y, uint16_t buttoncolor, uint16_t txtcolor, uint16_t font, uint16_t twtWidth, uint16_t twtHeight, char* str0)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

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
  char* str0;
} command = {
  .cmd = cmdCodeByType[api_gfx_button][oledConfig->deviceType],
  .state = state,
  .x = x,
  .y = y,
  .buttoncolor = buttoncolor,
  .txtcolor = txtcolor,
  .font = font,
  .twtWidth = twtWidth,
  .twtHeight = twtHeight,
  .str0 = str0
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_button unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_panel(OledConfig *oledConfig, uint16_t state, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t state;
  uint16_t x;
  uint16_t y;
  uint16_t width;
  uint16_t height;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_gfx_panel][oledConfig->deviceType],
  .state = state,
  .x = x,
  .y = y,
  .width = width,
  .height = height,
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_panel unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_slider(OledConfig *oledConfig, uint16_t mode, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color, uint16_t scale, uint16_t value)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

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
} command = {
  .cmd = cmdCodeByType[api_gfx_slider][oledConfig->deviceType],
  .mode = mode,
  .x1 = x1,
  .y1 = y1,
  .x2 = x2,
  .y2 = y2,
  .color = color,
  .scale = scale,
  .value = value
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_slider unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_screenCopyPaste(OledConfig *oledConfig, uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t width, uint16_t height)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t xs;
  uint16_t ys;
  uint16_t xd;
  uint16_t yd;
  uint16_t width;
  uint16_t height;
} command = {
  .cmd = cmdCodeByType[api_gfx_screenCopyPaste][oledConfig->deviceType],
  .xs = xs,
  .ys = ys,
  .xd = xd,
  .yd = yd,
  .width = width,
  .height = height
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_screenCopyPaste unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_bevelShadow(OledConfig *oledConfig, uint16_t value, int16_t *status)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t value;
} command = {
  .cmd = cmdCodeByType[api_gfx_bevelShadow][oledConfig->deviceType],
  .value = value
};	

  struct {
  uint8_t ack;
  int16_t status;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_bevelShadow unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *status = response.status;

}

void gfx_bevelWidth(OledConfig *oledConfig, uint16_t value, int16_t *status)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t value;
} command = {
  .cmd = cmdCodeByType[api_gfx_bevelWidth][oledConfig->deviceType],
  .value = value
};	

  struct {
  uint8_t ack;
  int16_t status;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_bevelWidth unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *status = response.status;

}

void gfx_bGcolour(OledConfig *oledConfig, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_gfx_bGcolour][oledConfig->deviceType],
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_outlineColour(OledConfig *oledConfig, uint16_t color)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_gfx_outlineColour][oledConfig->deviceType],
  .color = color
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_contrast(OledConfig *oledConfig, uint16_t contrast)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t contrast;
} command = {
  .cmd = cmdCodeByType[api_gfx_contrast][oledConfig->deviceType],
  .contrast = contrast
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_frameDelay(OledConfig *oledConfig, uint16_t delayMsec)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t delayMsec;
} command = {
  .cmd = cmdCodeByType[api_gfx_frameDelay][oledConfig->deviceType],
  .delayMsec = delayMsec
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_linePattern(OledConfig *oledConfig, uint16_t pattern)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t pattern;
} command = {
  .cmd = cmdCodeByType[api_gfx_linePattern][oledConfig->deviceType],
  .pattern = pattern
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_screenMode(OledConfig *oledConfig, uint16_t mode)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t mode;
} command = {
  .cmd = cmdCodeByType[api_gfx_screenMode][oledConfig->deviceType],
  .mode = mode
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_transparency(OledConfig *oledConfig, uint16_t mode, int16_t *previous)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t mode;
} command = {
  .cmd = cmdCodeByType[api_gfx_transparency][oledConfig->deviceType],
  .mode = mode
};	

  struct {
  uint8_t ack;
  int16_t previous;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_transparency unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *previous = response.previous;

}

void gfx_transparentColour(OledConfig *oledConfig, uint16_t color, int16_t *previous)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t color;
} command = {
  .cmd = cmdCodeByType[api_gfx_transparentColour][oledConfig->deviceType],
  .color = color
};	

  struct {
  uint8_t ack;
  int16_t previous;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_transparentColour unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *previous = response.previous;

}

void gfx_set(OledConfig *oledConfig, uint16_t function, uint16_t mode)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t function;
  uint16_t mode;
} command = {
  .cmd = cmdCodeByType[api_gfx_set][oledConfig->deviceType],
  .function = function,
  .mode = mode
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void gfx_get(OledConfig *oledConfig, uint16_t mode, int16_t *value)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t mode;
} command = {
  .cmd = cmdCodeByType[api_gfx_get][oledConfig->deviceType],
  .mode = mode
};	

  struct {
  uint8_t ack;
  int16_t value;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_get unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *value = response.value;

}

void media_init(OledConfig *oledConfig, int16_t *value)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  
} command = {
  .cmd = cmdCodeByType[api_media_init][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  int16_t value;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *value = response.value;

}

void media_setAdd(OledConfig *oledConfig, uint16_t hiAddr, uint16_t loAddr)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t hiAddr;
  uint16_t loAddr;
} command = {
  .cmd = cmdCodeByType[api_media_setAdd][oledConfig->deviceType],
  .hiAddr = hiAddr,
  .loAddr = loAddr
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void media_setSector(OledConfig *oledConfig, uint16_t hiAddr, uint16_t loAddr)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t hiAddr;
  uint16_t loAddr;
} command = {
  .cmd = cmdCodeByType[api_media_setSector][oledConfig->deviceType],
  .hiAddr = hiAddr,
  .loAddr = loAddr
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void media_readSector(OledConfig *oledConfig, uint8_t *sector)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  
} command = {
  .cmd = cmdCodeByType[api_media_readSector][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  uint8_t sector;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function media_readSector unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *sector = response.sector;

}

void media_writeSector(OledConfig *oledConfig, uint8_t sector)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint8_t sector;
} command = {
  .cmd = cmdCodeByType[api_media_writeSector][oledConfig->deviceType],
  .sector = sector
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function media_writeSector unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void media_readByte(OledConfig *oledConfig, int16_t *value)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  
} command = {
  .cmd = cmdCodeByType[api_media_readByte][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  int16_t value;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *value = response.value;

}

void media_readWord(OledConfig *oledConfig, int16_t *value)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  
} command = {
  .cmd = cmdCodeByType[api_media_readWord][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  int16_t value;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *value = response.value;

}

void media_writeByte(OledConfig *oledConfig, uint16_t value, int16_t *status)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t value;
} command = {
  .cmd = cmdCodeByType[api_media_writeByte][oledConfig->deviceType],
  .value = value
};	

  struct {
  uint8_t ack;
  int16_t status;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *status = response.status;

}

void media_writeWord(OledConfig *oledConfig, uint16_t value, int16_t *status)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t value;
} command = {
  .cmd = cmdCodeByType[api_media_writeWord][oledConfig->deviceType],
  .value = value
};	

  struct {
  uint8_t ack;
  int16_t status;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *status = response.status;

}

void media_flush(OledConfig *oledConfig, int16_t *status)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  
} command = {
  .cmd = cmdCodeByType[api_media_flush][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  int16_t status;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *status = response.status;

}

void media_image(OledConfig *oledConfig, uint16_t x, uint16_t y)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
} command = {
  .cmd = cmdCodeByType[api_media_image][oledConfig->deviceType],
  .x = x,
  .y = y
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void media_video(OledConfig *oledConfig, uint16_t x, uint16_t y)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
} command = {
  .cmd = cmdCodeByType[api_media_video][oledConfig->deviceType],
  .x = x,
  .y = y
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void media_videoFrame(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t frameNumber)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
  uint16_t frameNumber;
} command = {
  .cmd = cmdCodeByType[api_media_videoFrame][oledConfig->deviceType],
  .x = x,
  .y = y,
  .frameNumber = frameNumber
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void misc_peekB(OledConfig *oledConfig, uint16_t eveRegIndex, int16_t *value)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t eveRegIndex;
} command = {
  .cmd = cmdCodeByType[api_misc_peekB][oledConfig->deviceType],
  .eveRegIndex = eveRegIndex
};	

  struct {
  uint8_t ack;
  int16_t value;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_peekB unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *value = response.value;

}

void misc_pokeB(OledConfig *oledConfig, uint16_t eveRegIndex, uint16_t value)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t eveRegIndex;
  uint16_t value;
} command = {
  .cmd = cmdCodeByType[api_misc_pokeB][oledConfig->deviceType],
  .eveRegIndex = eveRegIndex,
  .value = value
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_pokeB unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void misc_peekW(OledConfig *oledConfig, uint16_t eveRegIndex, int16_t *value)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t eveRegIndex;
} command = {
  .cmd = cmdCodeByType[api_misc_peekW][oledConfig->deviceType],
  .eveRegIndex = eveRegIndex
};	

  struct {
  uint8_t ack;
  int16_t value;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_peekW unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *value = response.value;

}

void misc_pokeW(OledConfig *oledConfig, uint16_t eveRegIndex, uint16_t value)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t eveRegIndex;
  uint16_t value;
} command = {
  .cmd = cmdCodeByType[api_misc_pokeW][oledConfig->deviceType],
  .eveRegIndex = eveRegIndex,
  .value = value
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_pokeW unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void misc_peekM(OledConfig *oledConfig, uint16_t address, int16_t *value)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t address;
} command = {
  .cmd = cmdCodeByType[api_misc_peekM][oledConfig->deviceType],
  .address = address
};	

  struct {
  uint8_t ack;
  int16_t value;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_peekM unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *value = response.value;

}

void misc_pokeM(OledConfig *oledConfig, uint16_t address, uint16_t value)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t address;
  uint16_t value;
} command = {
  .cmd = cmdCodeByType[api_misc_pokeM][oledConfig->deviceType],
  .address = address,
  .value = value
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_pokeM unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void misc_joystick(OledConfig *oledConfig, int16_t *value)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  
} command = {
  .cmd = cmdCodeByType[api_misc_joystick][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  int16_t value;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_joystick unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *value = response.value;

}

void misc_beep(OledConfig *oledConfig, uint16_t note, uint16_t duration_ms)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t note;
  uint16_t duration_ms;
} command = {
  .cmd = cmdCodeByType[api_misc_beep][oledConfig->deviceType],
  .note = note,
  .duration_ms = duration_ms
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_beep unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void misc_setbaudWait(OledConfig *oledConfig, uint16_t index)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t index;
} command = {
  .cmd = cmdCodeByType[api_misc_setbaudWait][oledConfig->deviceType],
  .index = index
};	
oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     NULL, 0, false); 

}

void misc_blitComtoDisplay(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t data)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
  uint16_t width;
  uint16_t height;
  uint16_t data;
} command = {
  .cmd = cmdCodeByType[api_misc_blitComtoDisplay][oledConfig->deviceType],
  .x = x,
  .y = y,
  .width = width,
  .height = height,
  .data = data
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_blitComtoDisplay unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void sys_sleep(OledConfig *oledConfig, uint16_t duration_s, int16_t *duration)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t duration_s;
} command = {
  .cmd = cmdCodeByType[api_sys_sleep][oledConfig->deviceType],
  .duration_s = duration_s
};	

  struct {
  uint8_t ack;
  int16_t duration;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function sys_sleep unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *duration = response.duration;

}

void sys_memFree(OledConfig *oledConfig, uint16_t handle, int16_t *value)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t handle;
} command = {
  .cmd = cmdCodeByType[api_sys_memFree][oledConfig->deviceType],
  .handle = handle
};	

  struct {
  uint8_t ack;
  int16_t value;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function sys_memFree unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *value = response.value;

}

void sys_memHeap(OledConfig *oledConfig, int16_t *avail)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  
} command = {
  .cmd = cmdCodeByType[api_sys_memHeap][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  int16_t avail;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function sys_memHeap unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *avail = response.avail;

}

void sys_getModel(OledConfig *oledConfig, int16_t *count, int16_t *str)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  
} command = {
  .cmd = cmdCodeByType[api_sys_getModel][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  int16_t count;
  int16_t str;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *count = response.count;;
    *str = response.str;

}

void sys_getVersion(OledConfig *oledConfig, int16_t *version)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  
} command = {
  .cmd = cmdCodeByType[api_sys_getVersion][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  int16_t version;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *version = response.version;

}

void sys_getPmmC(OledConfig *oledConfig, int16_t *version)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  
} command = {
  .cmd = cmdCodeByType[api_sys_getPmmC][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  int16_t version;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *version = response.version;

}

void misc_screenSaverTimeout(OledConfig *oledConfig, uint16_t timout_ms)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t timout_ms;
} command = {
  .cmd = cmdCodeByType[api_misc_screenSaverTimeout][oledConfig->deviceType],
  .timout_ms = timout_ms
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_screenSaverTimeout unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void misc_screenSaverSpeed(OledConfig *oledConfig, uint16_t speed_index)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t speed_index;
} command = {
  .cmd = cmdCodeByType[api_misc_screenSaverSpeed][oledConfig->deviceType],
  .speed_index = speed_index
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_screenSaverSpeed unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void misc_screenSaverMode(OledConfig *oledConfig, uint16_t mode)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t mode;
} command = {
  .cmd = cmdCodeByType[api_misc_screenSaverMode][oledConfig->deviceType],
  .mode = mode
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_screenSaverMode unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void touch_detectRegion(OledConfig *oledConfig, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t x1;
  uint16_t y1;
  uint16_t x2;
  uint16_t y2;
} command = {
  .cmd = cmdCodeByType[api_touch_detectRegion][oledConfig->deviceType],
  .x1 = x1,
  .y1 = y1,
  .x2 = x2,
  .y2 = y2
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function touch_detectRegion unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void touch_set(OledConfig *oledConfig, uint16_t mode)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t mode;
} command = {
  .cmd = cmdCodeByType[api_touch_set][oledConfig->deviceType],
  .mode = mode
};	

  struct {
  uint8_t ack;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function touch_set unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);


}

void touch_get(OledConfig *oledConfig, uint16_t mode, int16_t *value)
{
  RET_UNLESS_INIT(oledConfig);
  RET_UNLESS_4DSYS(oledConfig);

 struct {
  uint16_t cmd;
  uint16_t mode;
} command = {
  .cmd = cmdCodeByType[api_touch_get][oledConfig->deviceType],
  .mode = mode
};	

  struct {
  uint8_t ack;
  int16_t value;
  } response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function touch_get unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     (uint8_t *) &response, sizeof(response), false);
  *value = response.value;

}

