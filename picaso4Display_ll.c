#include "/home/alex/DEV/STM32/CHIBIOS/COMMON/various/picaso4Display_ll.h"

/*
    This code has been generated from API description
    All hand modifications will be lost at next generation 
*/
static bool gfx_polyxxx(OledConfig *oledConfig, uint16_t cmd, uint16_t n, uint16_t vx[], uint16_t vy[], uint16_t color)
{
 struct {
  uint16_t cmd;
   uint16_t n;
} command = {
   .cmd = cmd,
  .n = __builtin_bswap16(n)
};	

  struct {
  uint8_t ack;
  } response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     NULL, 0);

  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) vx, sizeof(vx[0]) * n,
		     NULL, 0);

  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) vy, sizeof(vy[0]) * n,
		     NULL, 0);

  color = __builtin_bswap16(color);
  return oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &color, sizeof(color),
		     (uint8_t *) &response, sizeof(response)) != 0;
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

bool txt_moveCursor(OledConfig *oledConfig, uint16_t line, uint16_t column)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t line;
  uint16_t column;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_moveCursor][oledConfig->deviceType],
  .line = __builtin_bswap16(line) ,
  .column = __builtin_bswap16(column) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool txt_putCh(OledConfig *oledConfig, uint16_t car)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t car;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_putCh][oledConfig->deviceType],
  .car = __builtin_bswap16(car) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool txt_putStr(OledConfig *oledConfig, const char* str0)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;
  
  struct {
    uint16_t cmd;
  } __attribute__ ((__packed__)) command = {
    .cmd = cmdCodeByType[api_txt_putStr][oledConfig->deviceType],
  };	
  
  struct {
    uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     NULL, 0) ;
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
			     (uint8_t *) str0, strlen(str0) + 1,
			     (uint8_t *) &response, sizeof(response)) != 0;
  
  return stus;
}
bool txt_charWidth(OledConfig *oledConfig, char car, uint16_t *width)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  char car;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_charWidth][oledConfig->deviceType],
  .car = car 
};	

  struct {
  uint8_t ack;
  uint16_t width;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (width != NULL) 
       *width = __builtin_bswap16(response.width);
  return stus;
}

bool txt_charHeight(OledConfig *oledConfig, char car, uint16_t *height)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  char car;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_charHeight][oledConfig->deviceType],
  .car = car 
};	

  struct {
  uint8_t ack;
  uint16_t height;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (height != NULL) 
       *height = __builtin_bswap16(response.height);
  return stus;
}

bool txt_Fgcolour(OledConfig *oledConfig, uint16_t color, uint16_t *oldCol)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t color;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_Fgcolour][oledConfig->deviceType],
  .color = __builtin_bswap16(color) 
};	

  struct {
  uint8_t ack;
  uint16_t oldCol;
  } __attribute__ ((__packed__)) response;
  
   if (oledConfig->deviceType == GOLDELOX)
    stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response) - sizeof(*oldCol)) != 0;
   else
    stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool txt_Bgcolour(OledConfig *oledConfig, uint16_t color, uint16_t *oldCol)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t color;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_Bgcolour][oledConfig->deviceType],
  .color = __builtin_bswap16(color) 
};	

  struct {
  uint8_t ack;
  uint16_t oldCol;
  } __attribute__ ((__packed__)) response;
  
   if (oledConfig->deviceType == GOLDELOX)
    stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response) - sizeof(*oldCol)) != 0;
   else
    stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool txt_fontID(OledConfig *oledConfig, uint16_t id)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t id;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_fontID][oledConfig->deviceType],
  .id = __builtin_bswap16(id) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool txt_widthMult(OledConfig *oledConfig, uint16_t wMultiplier, uint16_t *oldMul)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t wMultiplier;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_widthMult][oledConfig->deviceType],
  .wMultiplier = __builtin_bswap16(wMultiplier) 
};	

  struct {
  uint8_t ack;
  uint16_t oldMul;
  } __attribute__ ((__packed__)) response;
  
   if (oledConfig->deviceType == GOLDELOX)
    stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response) - sizeof(*oldMul)) != 0;
   else
    stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool txt_heightMult(OledConfig *oledConfig, uint16_t hMultiplier, uint16_t *oldMul)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t hMultiplier;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_heightMult][oledConfig->deviceType],
  .hMultiplier = __builtin_bswap16(hMultiplier) 
};	

  struct {
  uint8_t ack;
  uint16_t oldMul;
  } __attribute__ ((__packed__)) response;
  
   if (oledConfig->deviceType == GOLDELOX)
    stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response) - sizeof(*oldMul)) != 0;
   else
    stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool txt_xgap(OledConfig *oledConfig, uint16_t xGap, uint16_t *oldGap)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t xGap;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_xgap][oledConfig->deviceType],
  .xGap = __builtin_bswap16(xGap) 
};	

  struct {
  uint8_t ack;
  uint16_t oldGap;
  } __attribute__ ((__packed__)) response;
  
   if (oledConfig->deviceType == GOLDELOX)
    stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response) - sizeof(*oldGap)) != 0;
   else
    stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool txt_ygap(OledConfig *oledConfig, uint16_t yGap, uint16_t *oldGap)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t yGap;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_ygap][oledConfig->deviceType],
  .yGap = __builtin_bswap16(yGap) 
};	

  struct {
  uint8_t ack;
  uint16_t oldGap;
  } __attribute__ ((__packed__)) response;
  
   if (oledConfig->deviceType == GOLDELOX)
    stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response) - sizeof(*oldGap)) != 0;
   else
    stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool txt_bold(OledConfig *oledConfig, uint16_t mode, uint16_t *oldBold)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t mode;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_bold][oledConfig->deviceType],
  .mode = __builtin_bswap16(mode) 
};	

  struct {
  uint8_t ack;
  uint16_t oldBold;
  } __attribute__ ((__packed__)) response;
  
   if (oledConfig->deviceType == GOLDELOX)
    stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response) - sizeof(*oldBold)) != 0;
   else
    stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool txt_inverse(OledConfig *oledConfig, uint16_t mode, uint16_t *oldInv)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t mode;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_inverse][oledConfig->deviceType],
  .mode = __builtin_bswap16(mode) 
};	

  struct {
  uint8_t ack;
  uint16_t oldInv;
  } __attribute__ ((__packed__)) response;
  
   if (oledConfig->deviceType == GOLDELOX)
    stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response) - sizeof(*oldInv)) != 0;
   else
    stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool txt_italic(OledConfig *oledConfig, uint16_t mode, uint16_t *oldItal)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t mode;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_italic][oledConfig->deviceType],
  .mode = __builtin_bswap16(mode) 
};	

  struct {
  uint8_t ack;
  uint16_t oldItal;
  } __attribute__ ((__packed__)) response;
  
   if (oledConfig->deviceType == GOLDELOX)
    stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response) - sizeof(*oldItal)) != 0;
   else
    stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool txt_opacity(OledConfig *oledConfig, uint16_t mode, uint16_t *oldOpa)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t mode;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_opacity][oledConfig->deviceType],
  .mode = __builtin_bswap16(mode) 
};	

  struct {
  uint8_t ack;
  uint16_t oldOpa;
  } __attribute__ ((__packed__)) response;
  
   if (oledConfig->deviceType == GOLDELOX)
    stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response) - sizeof(*oldOpa)) != 0;
   else
    stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool txt_underline(OledConfig *oledConfig, uint16_t mode, uint16_t *oldUnder)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t mode;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_underline][oledConfig->deviceType],
  .mode = __builtin_bswap16(mode) 
};	

  struct {
  uint8_t ack;
  uint16_t oldUnder;
  } __attribute__ ((__packed__)) response;
  
   if (oledConfig->deviceType == GOLDELOX)
    stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response) - sizeof(*oldUnder)) != 0;
   else
    stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool txt_attributes(OledConfig *oledConfig, uint16_t bitfield, uint16_t *oldAttr)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t bitfield;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_attributes][oledConfig->deviceType],
  .bitfield = __builtin_bswap16(bitfield) 
};	

  struct {
  uint8_t ack;
  uint16_t oldAttr;
  } __attribute__ ((__packed__)) response;
  
   if (oledConfig->deviceType == GOLDELOX)
    stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response) - sizeof(*oldAttr)) != 0;
   else
    stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool txt_set(OledConfig *oledConfig, uint16_t function, uint16_t value)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t function;
  uint16_t value;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_set][oledConfig->deviceType],
  .function = __builtin_bswap16(function) ,
  .value = __builtin_bswap16(value) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function txt_set unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool txt_wrap(OledConfig *oledConfig, uint16_t xpos)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t xpos;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_txt_wrap][oledConfig->deviceType],
  .xpos = __builtin_bswap16(xpos) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function txt_wrap unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_cls(OledConfig *oledConfig)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_cls][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_changeColour(OledConfig *oledConfig, uint16_t oldColor, uint16_t newColor)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t oldColor;
  uint16_t newColor;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_changeColour][oledConfig->deviceType],
  .oldColor = __builtin_bswap16(oldColor) ,
  .newColor = __builtin_bswap16(newColor) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_circle(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t radius, uint16_t color)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
  uint16_t radius;
  uint16_t color;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_circle][oledConfig->deviceType],
  .x = __builtin_bswap16(x) ,
  .y = __builtin_bswap16(y) ,
  .radius = __builtin_bswap16(radius) ,
  .color = __builtin_bswap16(color) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_circleFilled(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t radius, uint16_t color)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
  uint16_t radius;
  uint16_t color;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_circleFilled][oledConfig->deviceType],
  .x = __builtin_bswap16(x) ,
  .y = __builtin_bswap16(y) ,
  .radius = __builtin_bswap16(radius) ,
  .color = __builtin_bswap16(color) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_line(OledConfig *oledConfig, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
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
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_line][oledConfig->deviceType],
  .x1 = __builtin_bswap16(x1) ,
  .y1 = __builtin_bswap16(y1) ,
  .x2 = __builtin_bswap16(x2) ,
  .y2 = __builtin_bswap16(y2) ,
  .color = __builtin_bswap16(color) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_rectangle(OledConfig *oledConfig, uint16_t tlx, uint16_t tly, uint16_t brx, uint16_t bry, uint16_t color)
{
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
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_rectangle][oledConfig->deviceType],
  .tlx = __builtin_bswap16(tlx) ,
  .tly = __builtin_bswap16(tly) ,
  .brx = __builtin_bswap16(brx) ,
  .bry = __builtin_bswap16(bry) ,
  .color = __builtin_bswap16(color) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_rectangleFilled(OledConfig *oledConfig, uint16_t tlx, uint16_t tly, uint16_t brx, uint16_t bry, uint16_t color)
{
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
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_rectangleFilled][oledConfig->deviceType],
  .tlx = __builtin_bswap16(tlx) ,
  .tly = __builtin_bswap16(tly) ,
  .brx = __builtin_bswap16(brx) ,
  .bry = __builtin_bswap16(bry) ,
  .color = __builtin_bswap16(color) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_polyline(OledConfig *oledConfig, uint16_t n, uint16_t vx[], uint16_t vy[], uint16_t color)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  
  return gfx_polyxxx(oledConfig,
	      cmdCodeByType[api_gfx_polyline][oledConfig->deviceType],
	      n, vx, vy, color);

}
bool gfx_polygon(OledConfig *oledConfig, uint16_t n, uint16_t vx[], uint16_t vy[], uint16_t color)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  
  return gfx_polyxxx(oledConfig,
	      cmdCodeByType[api_gfx_polyline][oledConfig->deviceType],
	      n, vx, vy, color);

}
bool gfx_polygonFilled(OledConfig *oledConfig, uint16_t n, uint16_t vx[], uint16_t vy[], uint16_t color)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  osalDbgAssert(cmdCodeByType[api_gfx_polyline][oledConfig->deviceType] != CMD_NOT_IMPL, "function gfx_polygonFilled unimplemented for this screen");
  return gfx_polyxxx(oledConfig,
	      cmdCodeByType[api_gfx_polyline][oledConfig->deviceType],
	      n, vx, vy, color);

}
bool gfx_triangle(OledConfig *oledConfig, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color)
{
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
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_triangle][oledConfig->deviceType],
  .x1 = __builtin_bswap16(x1) ,
  .y1 = __builtin_bswap16(y1) ,
  .x2 = __builtin_bswap16(x2) ,
  .y2 = __builtin_bswap16(y2) ,
  .x3 = __builtin_bswap16(x3) ,
  .y3 = __builtin_bswap16(y3) ,
  .color = __builtin_bswap16(color) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_triangleFilled(OledConfig *oledConfig, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color)
{
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
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_triangleFilled][oledConfig->deviceType],
  .x1 = __builtin_bswap16(x1) ,
  .y1 = __builtin_bswap16(y1) ,
  .x2 = __builtin_bswap16(x2) ,
  .y2 = __builtin_bswap16(y2) ,
  .x3 = __builtin_bswap16(x3) ,
  .y3 = __builtin_bswap16(y3) ,
  .color = __builtin_bswap16(color) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_triangleFilled unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_orbit(OledConfig *oledConfig, uint16_t angle, uint16_t distance, uint16_t *Xdist, uint16_t *Ydist)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t angle;
  uint16_t distance;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_orbit][oledConfig->deviceType],
  .angle = __builtin_bswap16(angle) ,
  .distance = __builtin_bswap16(distance) 
};	

  struct {
  uint8_t ack;
  uint16_t Xdist;
  uint16_t Ydist;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (Xdist != NULL) 
       *Xdist = __builtin_bswap16(response.Xdist);;
    if (Ydist != NULL) 
       *Ydist = __builtin_bswap16(response.Ydist);
  return stus;
}

bool gfx_putPixel(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t color)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
  uint16_t color;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_putPixel][oledConfig->deviceType],
  .x = __builtin_bswap16(x) ,
  .y = __builtin_bswap16(y) ,
  .color = __builtin_bswap16(color) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_getPixel(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t *color)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_getPixel][oledConfig->deviceType],
  .x = __builtin_bswap16(x) ,
  .y = __builtin_bswap16(y) 
};	

  struct {
  uint8_t ack;
  uint16_t color;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (color != NULL) 
       *color = __builtin_bswap16(response.color);
  return stus;
}

bool gfx_moveTo(OledConfig *oledConfig, uint16_t x, uint16_t y)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_moveTo][oledConfig->deviceType],
  .x = __builtin_bswap16(x) ,
  .y = __builtin_bswap16(y) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_lineTo(OledConfig *oledConfig, uint16_t x, uint16_t y)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_lineTo][oledConfig->deviceType],
  .x = __builtin_bswap16(x) ,
  .y = __builtin_bswap16(y) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_clipping(OledConfig *oledConfig, uint16_t mode)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t mode;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_clipping][oledConfig->deviceType],
  .mode = __builtin_bswap16(mode) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_clipWindow(OledConfig *oledConfig, uint16_t tlx, uint16_t tly, uint16_t brx, uint16_t bry)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t tlx;
  uint16_t tly;
  uint16_t brx;
  uint16_t bry;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_clipWindow][oledConfig->deviceType],
  .tlx = __builtin_bswap16(tlx) ,
  .tly = __builtin_bswap16(tly) ,
  .brx = __builtin_bswap16(brx) ,
  .bry = __builtin_bswap16(bry) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_setClipRegion(OledConfig *oledConfig)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_setClipRegion][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_ellipse(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t xrad, uint16_t yrad, uint16_t color)
{
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
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_ellipse][oledConfig->deviceType],
  .x = __builtin_bswap16(x) ,
  .y = __builtin_bswap16(y) ,
  .xrad = __builtin_bswap16(xrad) ,
  .yrad = __builtin_bswap16(yrad) ,
  .color = __builtin_bswap16(color) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_ellipse unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_ellipseFilled(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t xrad, uint16_t yrad, uint16_t color)
{
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
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_ellipseFilled][oledConfig->deviceType],
  .x = __builtin_bswap16(x) ,
  .y = __builtin_bswap16(y) ,
  .xrad = __builtin_bswap16(xrad) ,
  .yrad = __builtin_bswap16(yrad) ,
  .color = __builtin_bswap16(color) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_ellipseFilled unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_button(OledConfig *oledConfig, uint16_t state, uint16_t x, uint16_t y,
		uint16_t buttoncolor, uint16_t txtcolor, uint16_t font,
		uint16_t twtWidth, uint16_t twtHeight, char* cstr)
{
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
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_button][oledConfig->deviceType],
  .state = __builtin_bswap16(state) ,
  .x = __builtin_bswap16(x) ,
  .y = __builtin_bswap16(y) ,
  .buttoncolor = __builtin_bswap16(buttoncolor) ,
  .txtcolor = __builtin_bswap16(txtcolor) ,
  .font = __builtin_bswap16(font) ,
  .twtWidth = __builtin_bswap16(twtWidth) ,
  .twtHeight = __builtin_bswap16(twtHeight)
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_button unimplemented for this screen");
  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     NULL, 0) ;
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
			     (uint8_t *) cstr, strlen(cstr) + 1,
			     (uint8_t *) &response, sizeof(response)) != 0;
 
  return stus;
}
bool gfx_panel(OledConfig *oledConfig, uint16_t state, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
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
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_panel][oledConfig->deviceType],
  .state = __builtin_bswap16(state) ,
  .x = __builtin_bswap16(x) ,
  .y = __builtin_bswap16(y) ,
  .width = __builtin_bswap16(width) ,
  .height = __builtin_bswap16(height) ,
  .color = __builtin_bswap16(color) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_panel unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_slider(OledConfig *oledConfig, uint16_t mode, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color, uint16_t scale, uint16_t value)
{
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
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_slider][oledConfig->deviceType],
  .mode = __builtin_bswap16(mode) ,
  .x1 = __builtin_bswap16(x1) ,
  .y1 = __builtin_bswap16(y1) ,
  .x2 = __builtin_bswap16(x2) ,
  .y2 = __builtin_bswap16(y2) ,
  .color = __builtin_bswap16(color) ,
  .scale = __builtin_bswap16(scale) ,
  .value = __builtin_bswap16(value) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_slider unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_screenCopyPaste(OledConfig *oledConfig, uint16_t xs, uint16_t ys, uint16_t xd, uint16_t yd, uint16_t width, uint16_t height)
{
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
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_screenCopyPaste][oledConfig->deviceType],
  .xs = __builtin_bswap16(xs) ,
  .ys = __builtin_bswap16(ys) ,
  .xd = __builtin_bswap16(xd) ,
  .yd = __builtin_bswap16(yd) ,
  .width = __builtin_bswap16(width) ,
  .height = __builtin_bswap16(height) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_screenCopyPaste unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_bevelShadow(OledConfig *oledConfig, uint16_t value, uint16_t *status)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t value;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_bevelShadow][oledConfig->deviceType],
  .value = __builtin_bswap16(value) 
};	

  struct {
  uint8_t ack;
  uint16_t status;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_bevelShadow unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (status != NULL) 
       *status = __builtin_bswap16(response.status);
  return stus;
}

bool gfx_bevelWidth(OledConfig *oledConfig, uint16_t value, uint16_t *status)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t value;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_bevelWidth][oledConfig->deviceType],
  .value = __builtin_bswap16(value) 
};	

  struct {
  uint8_t ack;
  uint16_t status;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_bevelWidth unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (status != NULL) 
       *status = __builtin_bswap16(response.status);
  return stus;
}

bool gfx_bGcolour(OledConfig *oledConfig, uint16_t color)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t color;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_bGcolour][oledConfig->deviceType],
  .color = __builtin_bswap16(color) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_outlineColour(OledConfig *oledConfig, uint16_t color)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t color;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_outlineColour][oledConfig->deviceType],
  .color = __builtin_bswap16(color) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_contrast(OledConfig *oledConfig, uint16_t contrast)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t contrast;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_contrast][oledConfig->deviceType],
  .contrast = __builtin_bswap16(contrast) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_frameDelay(OledConfig *oledConfig, uint16_t delayMsec)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t delayMsec;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_frameDelay][oledConfig->deviceType],
  .delayMsec = __builtin_bswap16(delayMsec) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_linePattern(OledConfig *oledConfig, uint16_t pattern)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t pattern;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_linePattern][oledConfig->deviceType],
  .pattern = __builtin_bswap16(pattern) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_screenMode(OledConfig *oledConfig, uint16_t mode, uint16_t *oldMode)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t mode;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_screenMode][oledConfig->deviceType],
  .mode = __builtin_bswap16(mode) 
};	

  struct {
  uint8_t ack;
  uint16_t oldMode;
  } __attribute__ ((__packed__)) response;
  
   if (oledConfig->deviceType == GOLDELOX)
    stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response) - sizeof(*oldMode)) != 0;
   else
    stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_transparency(OledConfig *oledConfig, uint16_t mode, uint16_t *previous)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t mode;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_transparency][oledConfig->deviceType],
  .mode = __builtin_bswap16(mode) 
};	

  struct {
  uint8_t ack;
  uint16_t previous;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_transparency unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (previous != NULL) 
       *previous = __builtin_bswap16(response.previous);
  return stus;
}

bool gfx_transparentColour(OledConfig *oledConfig, uint16_t color, uint16_t *previous)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t color;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_transparentColour][oledConfig->deviceType],
  .color = __builtin_bswap16(color) 
};	

  struct {
  uint8_t ack;
  uint16_t previous;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_transparentColour unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (previous != NULL) 
       *previous = __builtin_bswap16(response.previous);
  return stus;
}

bool gfx_set(OledConfig *oledConfig, uint16_t function, uint16_t mode)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t function;
  uint16_t mode;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_set][oledConfig->deviceType],
  .function = __builtin_bswap16(function) ,
  .mode = __builtin_bswap16(mode) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool gfx_get(OledConfig *oledConfig, uint16_t mode, uint16_t *value)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t mode;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_gfx_get][oledConfig->deviceType],
  .mode = __builtin_bswap16(mode) 
};	

  struct {
  uint8_t ack;
  uint16_t value;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function gfx_get unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (value != NULL) 
       *value = __builtin_bswap16(response.value);
  return stus;
}

bool media_init(OledConfig *oledConfig, uint16_t *value)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_media_init][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  uint16_t value;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (value != NULL) 
       *value = __builtin_bswap16(response.value);
  return stus;
}

bool media_setAdd(OledConfig *oledConfig, uint16_t hiAddr, uint16_t loAddr)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t hiAddr;
  uint16_t loAddr;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_media_setAdd][oledConfig->deviceType],
  .hiAddr = __builtin_bswap16(hiAddr) ,
  .loAddr = __builtin_bswap16(loAddr) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool media_setSector(OledConfig *oledConfig, uint16_t hiAddr, uint16_t loAddr)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t hiAddr;
  uint16_t loAddr;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_media_setSector][oledConfig->deviceType],
  .hiAddr = __builtin_bswap16(hiAddr) ,
  .loAddr = __builtin_bswap16(loAddr) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool media_readSector(OledConfig *oledConfig, uint8_t *sector)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_media_readSector][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  uint8_t sector;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function media_readSector unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (sector != NULL) 
       *sector = __builtin_bswap16(response.sector);
  return stus;
}

bool media_writeSector(OledConfig *oledConfig, uint8_t sector)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint8_t sector;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_media_writeSector][oledConfig->deviceType],
  .sector = sector 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function media_writeSector unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool media_readByte(OledConfig *oledConfig, uint16_t *value)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_media_readByte][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  uint16_t value;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (value != NULL) 
       *value = __builtin_bswap16(response.value);
  return stus;
}

bool media_readWord(OledConfig *oledConfig, uint16_t *value)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_media_readWord][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  uint16_t value;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (value != NULL) 
       *value = __builtin_bswap16(response.value);
  return stus;
}

bool media_writeByte(OledConfig *oledConfig, uint16_t value, uint16_t *status)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t value;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_media_writeByte][oledConfig->deviceType],
  .value = __builtin_bswap16(value) 
};	

  struct {
  uint8_t ack;
  uint16_t status;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (status != NULL) 
       *status = __builtin_bswap16(response.status);
  return stus;
}

bool media_writeWord(OledConfig *oledConfig, uint16_t value, uint16_t *status)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t value;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_media_writeWord][oledConfig->deviceType],
  .value = __builtin_bswap16(value) 
};	

  struct {
  uint8_t ack;
  uint16_t status;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (status != NULL) 
       *status = __builtin_bswap16(response.status);
  return stus;
}

bool media_flush(OledConfig *oledConfig, uint16_t *status)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_media_flush][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  uint16_t status;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (status != NULL) 
       *status = __builtin_bswap16(response.status);
  return stus;
}

bool media_image(OledConfig *oledConfig, uint16_t x, uint16_t y)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_media_image][oledConfig->deviceType],
  .x = __builtin_bswap16(x) ,
  .y = __builtin_bswap16(y) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool media_video(OledConfig *oledConfig, uint16_t x, uint16_t y)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_media_video][oledConfig->deviceType],
  .x = __builtin_bswap16(x) ,
  .y = __builtin_bswap16(y) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool media_videoFrame(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t frameNumber)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
  uint16_t frameNumber;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_media_videoFrame][oledConfig->deviceType],
  .x = __builtin_bswap16(x) ,
  .y = __builtin_bswap16(y) ,
  .frameNumber = __builtin_bswap16(frameNumber) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool misc_peekB(OledConfig *oledConfig, uint16_t eveRegIndex, uint16_t *value)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t eveRegIndex;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_misc_peekB][oledConfig->deviceType],
  .eveRegIndex = __builtin_bswap16(eveRegIndex) 
};	

  struct {
  uint8_t ack;
  uint16_t value;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_peekB unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (value != NULL) 
       *value = __builtin_bswap16(response.value);
  return stus;
}

bool misc_pokeB(OledConfig *oledConfig, uint16_t eveRegIndex, uint16_t value)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t eveRegIndex;
  uint16_t value;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_misc_pokeB][oledConfig->deviceType],
  .eveRegIndex = __builtin_bswap16(eveRegIndex) ,
  .value = __builtin_bswap16(value) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_pokeB unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool misc_peekW(OledConfig *oledConfig, uint16_t eveRegIndex, uint16_t *value)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t eveRegIndex;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_misc_peekW][oledConfig->deviceType],
  .eveRegIndex = __builtin_bswap16(eveRegIndex) 
};	

  struct {
  uint8_t ack;
  uint16_t value;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_peekW unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (value != NULL) 
       *value = __builtin_bswap16(response.value);
  return stus;
}

bool misc_pokeW(OledConfig *oledConfig, uint16_t eveRegIndex, uint16_t value)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t eveRegIndex;
  uint16_t value;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_misc_pokeW][oledConfig->deviceType],
  .eveRegIndex = __builtin_bswap16(eveRegIndex) ,
  .value = __builtin_bswap16(value) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_pokeW unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool misc_peekM(OledConfig *oledConfig, uint16_t address, uint16_t *value)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t address;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_misc_peekM][oledConfig->deviceType],
  .address = __builtin_bswap16(address) 
};	

  struct {
  uint8_t ack;
  uint16_t value;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_peekM unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (value != NULL) 
       *value = __builtin_bswap16(response.value);
  return stus;
}

bool misc_pokeM(OledConfig *oledConfig, uint16_t address, uint16_t value)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t address;
  uint16_t value;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_misc_pokeM][oledConfig->deviceType],
  .address = __builtin_bswap16(address) ,
  .value = __builtin_bswap16(value) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_pokeM unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool misc_joystick(OledConfig *oledConfig, uint16_t *value)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_misc_joystick][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  uint16_t value;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_joystick unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (value != NULL) 
       *value = __builtin_bswap16(response.value);
  return stus;
}

bool misc_beep(OledConfig *oledConfig, uint16_t note, uint16_t duration_ms)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t note;
  uint16_t duration_ms;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_misc_beep][oledConfig->deviceType],
  .note = __builtin_bswap16(note) ,
  .duration_ms = __builtin_bswap16(duration_ms) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_beep unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool misc_setbaudWait(OledConfig *oledConfig, uint16_t index)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t index;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_misc_setbaudWait][oledConfig->deviceType],
  .index = __builtin_bswap16(index) 
};	
    stus = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
		     (uint8_t *) &command, sizeof(command),
		     NULL, 0) == 0; 
  return stus;
}

bool misc_blitComtoDisplay(OledConfig *oledConfig, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t data)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t x;
  uint16_t y;
  uint16_t width;
  uint16_t height;
  uint16_t data;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_misc_blitComtoDisplay][oledConfig->deviceType],
  .x = __builtin_bswap16(x) ,
  .y = __builtin_bswap16(y) ,
  .width = __builtin_bswap16(width) ,
  .height = __builtin_bswap16(height) ,
  .data = __builtin_bswap16(data) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_blitComtoDisplay unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool sys_sleep(OledConfig *oledConfig, uint16_t duration_s, uint16_t *duration)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t duration_s;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_sys_sleep][oledConfig->deviceType],
  .duration_s = __builtin_bswap16(duration_s) 
};	

  struct {
  uint8_t ack;
  uint16_t duration;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function sys_sleep unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (duration != NULL) 
       *duration = __builtin_bswap16(response.duration);
  return stus;
}

bool sys_memFree(OledConfig *oledConfig, uint16_t handle, uint16_t *value)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t handle;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_sys_memFree][oledConfig->deviceType],
  .handle = __builtin_bswap16(handle) 
};	

  struct {
  uint8_t ack;
  uint16_t value;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function sys_memFree unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (value != NULL) 
       *value = __builtin_bswap16(response.value);
  return stus;
}

bool sys_memHeap(OledConfig *oledConfig, uint16_t *avail)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_sys_memHeap][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  uint16_t avail;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function sys_memHeap unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (avail != NULL) 
       *avail = __builtin_bswap16(response.avail);
  return stus;
}

    bool sys_getModel(OledConfig *oledConfig, uint16_t n, char *str)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);

 struct {
  uint16_t cmd;
  
 } __attribute__ ((__packed__)) command = {
   .cmd = cmdCodeByType[api_sys_getModel][oledConfig->deviceType]
 };	

 struct {
   uint8_t ack;
   uint16_t n;
 } __attribute__ ((__packed__)) response;
 
 size_t ret = oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
				 (uint8_t *) &command, sizeof(command),
				 (uint8_t *) &response, sizeof(response));
 if (ret != sizeof(response)) {
   DebugTrace("sys_getModel error ret=%u instead of %u", ret, sizeof(response));
   str[0] = 0;
   return false;
 }
 return oledGetDynamicString(oledConfig, __FUNCTION__, __LINE__,
			     __builtin_bswap16(response.n),
			     (uint8_t *) str, n);
}

bool sys_getVersion(OledConfig *oledConfig, uint16_t *version)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_sys_getVersion][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  uint16_t version;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (version != NULL) 
       *version = __builtin_bswap16(response.version);
  return stus;
}

bool sys_getPmmC(OledConfig *oledConfig, uint16_t *version)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_sys_getPmmC][oledConfig->deviceType]
  
};	

  struct {
  uint8_t ack;
  uint16_t version;
  } __attribute__ ((__packed__)) response;
  
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (version != NULL) 
       *version = __builtin_bswap16(response.version);
  return stus;
}

bool misc_screenSaverTimeout(OledConfig *oledConfig, uint16_t timout_ms)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t timout_ms;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_misc_screenSaverTimeout][oledConfig->deviceType],
  .timout_ms = __builtin_bswap16(timout_ms) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_screenSaverTimeout unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool misc_screenSaverSpeed(OledConfig *oledConfig, uint16_t speed_index)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t speed_index;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_misc_screenSaverSpeed][oledConfig->deviceType],
  .speed_index = __builtin_bswap16(speed_index) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_screenSaverSpeed unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool misc_screenSaverMode(OledConfig *oledConfig, uint16_t mode)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t mode;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_misc_screenSaverMode][oledConfig->deviceType],
  .mode = __builtin_bswap16(mode) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function misc_screenSaverMode unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool touch_detectRegion(OledConfig *oledConfig, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t x1;
  uint16_t y1;
  uint16_t x2;
  uint16_t y2;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_touch_detectRegion][oledConfig->deviceType],
  .x1 = __builtin_bswap16(x1) ,
  .y1 = __builtin_bswap16(y1) ,
  .x2 = __builtin_bswap16(x2) ,
  .y2 = __builtin_bswap16(y2) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function touch_detectRegion unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool touch_set(OledConfig *oledConfig, uint16_t mode)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t mode;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_touch_set][oledConfig->deviceType],
  .mode = __builtin_bswap16(mode) 
};	

  struct {
  uint8_t ack;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function touch_set unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;

  return stus;
}

bool touch_get(OledConfig *oledConfig, uint16_t mode, uint16_t *value)
{
  RET_UNLESS_INIT_BOOL(oledConfig);
  RET_UNLESS_4DSYS_BOOL(oledConfig);
  bool stus = false;

 struct {
  uint16_t cmd;
  uint16_t mode;
} __attribute__ ((__packed__)) command = {
  .cmd = cmdCodeByType[api_touch_get][oledConfig->deviceType],
  .mode = __builtin_bswap16(mode) 
};	

  struct {
  uint8_t ack;
  uint16_t value;
  } __attribute__ ((__packed__)) response;
  osalDbgAssert(command.cmd != CMD_NOT_IMPL, "function touch_get unimplemented for this screen");
  stus =  oledTransmitBuffer(oledConfig, __FUNCTION__, __LINE__,
  		      (uint8_t *) &command, sizeof(command),
		      (uint8_t *) &response, sizeof(response)) != 0;
  if (value != NULL) 
       *value = __builtin_bswap16(response.value);
  return stus;
}

