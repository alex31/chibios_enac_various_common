#include "../display4DS_ll.h"

/*
    This code has been generated from API description
    All hand modifications will be lost at next generation
*/
static bool gfx_polyxxx(const FdsConfig *fdsConfig, uint16_t cmd, uint16_t n,
                        const uint16_t vx[], const uint16_t vy[],
                        uint16_t color) {
  struct {
    uint16_t cmd;
    uint16_t n;
  } command1 = {.cmd = cmd, .n = __builtin_bswap16(n)};

  struct {
    uint8_t ack;
  } response;

  fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                    sizeof(command1), NULL, 0);

  fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)vx,
                    sizeof(vx[0]) * n, NULL, 0);

  fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)vy,
                    sizeof(vy[0]) * n, NULL, 0);

  color = __builtin_bswap16(color);
  return fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&color,
                           sizeof(color), (uint8_t *)&response,
                           sizeof(response)) != 0;
}

bool txt_moveCursor(const FdsConfig *fdsConfig, uint16_t line,
                    uint16_t column) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFE4),
                                          __builtin_bswap16(0xFFE9),
                                          __builtin_bswap16(0xFFF0)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t line;
    uint16_t column;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .line = __builtin_bswap16(line),
              .column = __builtin_bswap16(column)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool txt_putCh(const FdsConfig *fdsConfig, uint16_t car) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFFE),
                                          __builtin_bswap16(0xFFFE),
                                          __builtin_bswap16(0xFFFE)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t car;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .car = __builtin_bswap16(car)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool txt_putStr(const FdsConfig *fdsConfig, const char *cstr,
                uint16_t *length) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0x0006),
                                          __builtin_bswap16(0x0018),
                                          __builtin_bswap16(0x0018)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t length;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)cstr,
                           strlen(cstr) + 1, (uint8_t *)&response,
                           sizeof(response) - (fdsConfig->deviceType == GOLDELOX
                                                   ? sizeof(*length)
                                                   : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (length != NULL) *length = __builtin_bswap16(response.length);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_charWidth(const FdsConfig *fdsConfig, char car, uint16_t *width) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0x0002),
                                          __builtin_bswap16(0x001E),
                                          __builtin_bswap16(0x001E)};

  bool stus = false;
  struct {
    uint16_t cmd;
    char car;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .car = __builtin_bswap16(car)};

  struct {
    uint8_t ack;
    uint16_t width;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (width != NULL) *width = __builtin_bswap16(response.width);

  return stus && (response.ack == QDS_ACK);
}

bool txt_charHeight(const FdsConfig *fdsConfig, char car, uint16_t *height) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0x0001),
                                          __builtin_bswap16(0x001D),
                                          __builtin_bswap16(0x001D)};

  bool stus = false;
  struct {
    uint16_t cmd;
    char car;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .car = __builtin_bswap16(car)};

  struct {
    uint8_t ack;
    uint16_t height;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (height != NULL) *height = __builtin_bswap16(response.height);

  return stus && (response.ack == QDS_ACK);
}

bool txt_fgColour(const FdsConfig *fdsConfig, uint16_t colour,
                  uint16_t *oldCol) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF7F),
                                          __builtin_bswap16(0xFFE7),
                                          __builtin_bswap16(0xFFEE)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t colour;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType], .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
    uint16_t oldCol;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(
          fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (fdsConfig->deviceType == GOLDELOX ? sizeof(*oldCol) : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldCol != NULL) *oldCol = __builtin_bswap16(response.oldCol);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_bgColour(const FdsConfig *fdsConfig, uint16_t colour,
                  uint16_t *oldCol) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF7E),
                                          __builtin_bswap16(0xFFE6),
                                          __builtin_bswap16(0xFFED)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t colour;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType], .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
    uint16_t oldCol;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(
          fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (fdsConfig->deviceType == GOLDELOX ? sizeof(*oldCol) : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldCol != NULL) *oldCol = __builtin_bswap16(response.oldCol);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_fontID(const FdsConfig *fdsConfig, uint16_t id, uint16_t *oldFont) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF7D),
                                          __builtin_bswap16(0xFFE5),
                                          __builtin_bswap16(0xFFEC)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t id;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType], .id = __builtin_bswap16(id)};

  struct {
    uint8_t ack;
    uint16_t oldFont;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(
          fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (fdsConfig->deviceType == GOLDELOX ? sizeof(*oldFont) : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldFont != NULL) *oldFont = __builtin_bswap16(response.oldFont);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_widthMult(const FdsConfig *fdsConfig, uint16_t wMultiplier,
                   uint16_t *oldMul) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF7C),
                                          __builtin_bswap16(0xFFE4),
                                          __builtin_bswap16(0xFFEB)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t wMultiplier;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .wMultiplier = __builtin_bswap16(wMultiplier)};

  struct {
    uint8_t ack;
    uint16_t oldMul;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(
          fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (fdsConfig->deviceType == GOLDELOX ? sizeof(*oldMul) : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldMul != NULL) *oldMul = __builtin_bswap16(response.oldMul);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_heightMult(const FdsConfig *fdsConfig, uint16_t hMultiplier,
                    uint16_t *oldMul) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF7B),
                                          __builtin_bswap16(0xFFE3),
                                          __builtin_bswap16(0xFFEA)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t hMultiplier;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .hMultiplier = __builtin_bswap16(hMultiplier)};

  struct {
    uint8_t ack;
    uint16_t oldMul;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(
          fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (fdsConfig->deviceType == GOLDELOX ? sizeof(*oldMul) : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldMul != NULL) *oldMul = __builtin_bswap16(response.oldMul);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_xgap(const FdsConfig *fdsConfig, uint16_t xGap, uint16_t *oldGap) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF7A),
                                          __builtin_bswap16(0xFFE2),
                                          __builtin_bswap16(0xFFE9)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t xGap;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .xGap = __builtin_bswap16(xGap)};

  struct {
    uint8_t ack;
    uint16_t oldGap;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(
          fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (fdsConfig->deviceType == GOLDELOX ? sizeof(*oldGap) : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldGap != NULL) *oldGap = __builtin_bswap16(response.oldGap);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_ygap(const FdsConfig *fdsConfig, uint16_t yGap, uint16_t *oldGap) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF79),
                                          __builtin_bswap16(0xFFE1),
                                          __builtin_bswap16(0xFFE8)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t yGap;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .yGap = __builtin_bswap16(yGap)};

  struct {
    uint8_t ack;
    uint16_t oldGap;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(
          fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (fdsConfig->deviceType == GOLDELOX ? sizeof(*oldGap) : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldGap != NULL) *oldGap = __builtin_bswap16(response.oldGap);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_bold(const FdsConfig *fdsConfig, uint16_t mode, uint16_t *oldBold) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF76),
                                          __builtin_bswap16(0xFFDE),
                                          __builtin_bswap16(0xFFE5)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldBold;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(
          fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (fdsConfig->deviceType == GOLDELOX ? sizeof(*oldBold) : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldBold != NULL) *oldBold = __builtin_bswap16(response.oldBold);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_inverse(const FdsConfig *fdsConfig, uint16_t mode, uint16_t *oldInv) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF74),
                                          __builtin_bswap16(0xFFDC),
                                          __builtin_bswap16(0xFFE3)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldInv;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(
          fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (fdsConfig->deviceType == GOLDELOX ? sizeof(*oldInv) : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldInv != NULL) *oldInv = __builtin_bswap16(response.oldInv);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_italic(const FdsConfig *fdsConfig, uint16_t mode, uint16_t *oldItal) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF75),
                                          __builtin_bswap16(0xFFDD),
                                          __builtin_bswap16(0xFFE4)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldItal;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(
          fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (fdsConfig->deviceType == GOLDELOX ? sizeof(*oldItal) : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldItal != NULL) *oldItal = __builtin_bswap16(response.oldItal);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_opacity(const FdsConfig *fdsConfig, uint16_t mode, uint16_t *oldOpa) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF77),
                                          __builtin_bswap16(0xFFDF),
                                          __builtin_bswap16(0xFFE6)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldOpa;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(
          fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (fdsConfig->deviceType == GOLDELOX ? sizeof(*oldOpa) : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldOpa != NULL) *oldOpa = __builtin_bswap16(response.oldOpa);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_underline(const FdsConfig *fdsConfig, uint16_t mode,
                   uint16_t *oldUnder) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF73),
                                          __builtin_bswap16(0xFFDB),
                                          __builtin_bswap16(0xFFE2)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldUnder;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(
          fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (fdsConfig->deviceType == GOLDELOX ? sizeof(*oldUnder) : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldUnder != NULL) *oldUnder = __builtin_bswap16(response.oldUnder);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_attributes(const FdsConfig *fdsConfig, uint16_t bitfield,
                    uint16_t *oldAttr) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF72),
                                          __builtin_bswap16(0xFFDA),
                                          __builtin_bswap16(0xFFE1)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t bitfield;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .bitfield = __builtin_bswap16(bitfield)};

  struct {
    uint8_t ack;
    uint16_t oldAttr;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(
          fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (fdsConfig->deviceType == GOLDELOX ? sizeof(*oldAttr) : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldAttr != NULL) *oldAttr = __builtin_bswap16(response.oldAttr);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_set(const FdsConfig *fdsConfig, uint16_t function, uint16_t value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFE3),
                                          __builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t function;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .function = __builtin_bswap16(function),
              .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function txt_set unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool txt_wrap(const FdsConfig *fdsConfig, uint16_t xpos, uint16_t *oldWrap) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFD9),
                                          __builtin_bswap16(0xFFE0)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t xpos;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .xpos = __builtin_bswap16(xpos)};

  struct {
    uint8_t ack;
    uint16_t oldWrap;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function txt_wrap unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (oldWrap != NULL) *oldWrap = __builtin_bswap16(response.oldWrap);

  return stus && (response.ack == QDS_ACK);
}

bool gfx_cls(const FdsConfig *fdsConfig) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFD7),
                                          __builtin_bswap16(0xFFCD),
                                          __builtin_bswap16(0xFF82)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_changeColour(const FdsConfig *fdsConfig, uint16_t oldColour,
                      uint16_t newColour) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFBE),
                                          __builtin_bswap16(0xFFB4),
                                          __builtin_bswap16(0xFF69)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t oldColour;
    uint16_t newColour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .oldColour = __builtin_bswap16(oldColour),
              .newColour = __builtin_bswap16(newColour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_circle(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                uint16_t radius, uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFCD),
                                          __builtin_bswap16(0xFFC3),
                                          __builtin_bswap16(0xFF78)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t radius;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .radius = __builtin_bswap16(radius),
              .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_circleFilled(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                      uint16_t radius, uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFCC),
                                          __builtin_bswap16(0xFFC2),
                                          __builtin_bswap16(0xFF77)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t radius;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .radius = __builtin_bswap16(radius),
              .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_line(const FdsConfig *fdsConfig, uint16_t x1, uint16_t y1, uint16_t x2,
              uint16_t y2, uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFD2),
                                          __builtin_bswap16(0xFFC8),
                                          __builtin_bswap16(0xFF7D)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .x1 = __builtin_bswap16(x1),
              .y1 = __builtin_bswap16(y1),
              .x2 = __builtin_bswap16(x2),
              .y2 = __builtin_bswap16(y2),
              .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_rectangle(const FdsConfig *fdsConfig, uint16_t tlx, uint16_t tly,
                   uint16_t brx, uint16_t bry, uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFCF),
                                          __builtin_bswap16(0xFFC5),
                                          __builtin_bswap16(0xFF7A)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t tlx;
    uint16_t tly;
    uint16_t brx;
    uint16_t bry;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .tlx = __builtin_bswap16(tlx),
              .tly = __builtin_bswap16(tly),
              .brx = __builtin_bswap16(brx),
              .bry = __builtin_bswap16(bry),
              .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_rectangleFilled(const FdsConfig *fdsConfig, uint16_t tlx, uint16_t tly,
                         uint16_t brx, uint16_t bry, uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFCE),
                                          __builtin_bswap16(0xFFC4),
                                          __builtin_bswap16(0xFF79)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t tlx;
    uint16_t tly;
    uint16_t brx;
    uint16_t bry;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .tlx = __builtin_bswap16(tlx),
              .tly = __builtin_bswap16(tly),
              .brx = __builtin_bswap16(brx),
              .bry = __builtin_bswap16(bry),
              .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_polyline(const FdsConfig *fdsConfig, uint16_t n, const uint16_t vx[],
                  const uint16_t vy[], uint16_t color) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0x0005),
                                          __builtin_bswap16(0x0015),
                                          __builtin_bswap16(0x0015)};

  return gfx_polyxxx(fdsConfig, cmds[fdsConfig->deviceType], n, vx, vy, color);
}
bool gfx_polygon(const FdsConfig *fdsConfig, uint16_t n, const uint16_t vx[],
                 const uint16_t vy[], uint16_t color) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0x0004),
                                          __builtin_bswap16(0x0013),
                                          __builtin_bswap16(0x0013)};

  return gfx_polyxxx(fdsConfig, cmds[fdsConfig->deviceType], n, vx, vy, color);
}
bool gfx_polygonFilled(const FdsConfig *fdsConfig, uint16_t n,
                       const uint16_t vx[], const uint16_t vy[],
                       uint16_t color) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0014),
                                          __builtin_bswap16(0x0014)};
  osalDbgAssert(cmds[fdsConfig->deviceType] != CMD_NOT_IMPL,
                "function gfx_polygonFilled unimplemented for this screen");
  return gfx_polyxxx(fdsConfig, cmds[fdsConfig->deviceType], n, vx, vy, color);
}
bool gfx_triangle(const FdsConfig *fdsConfig, uint16_t x1, uint16_t y1,
                  uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
                  uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFC9),
                                          __builtin_bswap16(0xFFBF),
                                          __builtin_bswap16(0xFF74)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
    uint16_t x3;
    uint16_t y3;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .x1 = __builtin_bswap16(x1),
              .y1 = __builtin_bswap16(y1),
              .x2 = __builtin_bswap16(x2),
              .y2 = __builtin_bswap16(y2),
              .x3 = __builtin_bswap16(x3),
              .y3 = __builtin_bswap16(y3),
              .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_triangleFilled(const FdsConfig *fdsConfig, uint16_t x1, uint16_t y1,
                        uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
                        uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFA9),
                                          __builtin_bswap16(0xFF59)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
    uint16_t x3;
    uint16_t y3;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .x1 = __builtin_bswap16(x1),
              .y1 = __builtin_bswap16(y1),
              .x2 = __builtin_bswap16(x2),
              .y2 = __builtin_bswap16(y2),
              .x3 = __builtin_bswap16(x3),
              .y3 = __builtin_bswap16(y3),
              .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_triangleFilled unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_orbit(const FdsConfig *fdsConfig, uint16_t angle, uint16_t distance,
               uint16_t *Xdist, uint16_t *Ydist) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0x0003),
                                          __builtin_bswap16(0x0012),
                                          __builtin_bswap16(0x0012)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t angle;
    uint16_t distance;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .angle = __builtin_bswap16(angle),
              .distance = __builtin_bswap16(distance)};

  struct {
    uint8_t ack;
    uint16_t Xdist;
    uint16_t Ydist;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (Xdist != NULL) *Xdist = __builtin_bswap16(response.Xdist);
  ;
  if (Ydist != NULL) *Ydist = __builtin_bswap16(response.Ydist);

  return stus && (response.ack == QDS_ACK);
}

bool gfx_putPixel(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                  uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFCB),
                                          __builtin_bswap16(0xFFC1),
                                          __builtin_bswap16(0xFF76)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_getPixel(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                  uint16_t *colour) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFCA),
                                          __builtin_bswap16(0xFFC0),
                                          __builtin_bswap16(0xFF75)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .x = __builtin_bswap16(x),
                                            .y = __builtin_bswap16(y)};

  struct {
    uint8_t ack;
    uint16_t colour;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (colour != NULL) *colour = __builtin_bswap16(response.colour);

  return stus && (response.ack == QDS_ACK);
}

bool gfx_moveTo(const FdsConfig *fdsConfig, uint16_t x, uint16_t y) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFD6),
                                          __builtin_bswap16(0xFFCC),
                                          __builtin_bswap16(0xFF81)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .x = __builtin_bswap16(x),
                                            .y = __builtin_bswap16(y)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_lineTo(const FdsConfig *fdsConfig, uint16_t x, uint16_t y) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFD4),
                                          __builtin_bswap16(0xFFCA),
                                          __builtin_bswap16(0xFF7F)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .x = __builtin_bswap16(x),
                                            .y = __builtin_bswap16(y)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_clipping(const FdsConfig *fdsConfig, uint16_t mode) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF6C),
                                          __builtin_bswap16(0xFFA2),
                                          __builtin_bswap16(0xFF46)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_clipWindow(const FdsConfig *fdsConfig, uint16_t tlx, uint16_t tly,
                    uint16_t brx, uint16_t bry) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFBF),
                                          __builtin_bswap16(0xFFB5),
                                          __builtin_bswap16(0xFF6A)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t tlx;
    uint16_t tly;
    uint16_t brx;
    uint16_t bry;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .tlx = __builtin_bswap16(tlx),
                                            .tly = __builtin_bswap16(tly),
                                            .brx = __builtin_bswap16(brx),
                                            .bry = __builtin_bswap16(bry)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_setClipRegion(const FdsConfig *fdsConfig) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFBC),
                                          __builtin_bswap16(0xFFB3),
                                          __builtin_bswap16(0xFF68)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_ellipse(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                 uint16_t xrad, uint16_t yrad, uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFB2),
                                          __builtin_bswap16(0xFF67)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t xrad;
    uint16_t yrad;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .xrad = __builtin_bswap16(xrad),
              .yrad = __builtin_bswap16(yrad),
              .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_ellipse unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_ellipseFilled(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                       uint16_t xrad, uint16_t yrad, uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFB1),
                                          __builtin_bswap16(0xFF66)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t xrad;
    uint16_t yrad;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .xrad = __builtin_bswap16(xrad),
              .yrad = __builtin_bswap16(yrad),
              .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_ellipseFilled unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_button(const FdsConfig *fdsConfig, uint16_t state, uint16_t x,
                uint16_t y, uint16_t buttoncolour, uint16_t txtcolour,
                uint16_t font, uint16_t txtWidth, uint16_t txtHeight,
                const char *cstr) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0011),
                                          __builtin_bswap16(0x0011)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t state;
    uint16_t x;
    uint16_t y;
    uint16_t buttoncolour;
    uint16_t txtcolour;
    uint16_t font;
    uint16_t txtWidth;
    uint16_t txtHeight;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .state = __builtin_bswap16(state),
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .buttoncolour = __builtin_bswap16(buttoncolour),
              .txtcolour = __builtin_bswap16(txtcolour),
              .font = __builtin_bswap16(font),
              .txtWidth = __builtin_bswap16(txtWidth),
              .txtHeight = __builtin_bswap16(txtHeight)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_button unimplemented for this screen");
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)cstr,
                           strlen(cstr) + 1, (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_panel(const FdsConfig *fdsConfig, uint16_t state, uint16_t x,
               uint16_t y, uint16_t width, uint16_t height, uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFAF),
                                          __builtin_bswap16(0xFF5F)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t state;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .state = __builtin_bswap16(state),
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .width = __builtin_bswap16(width),
              .height = __builtin_bswap16(height),
              .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_panel unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_slider(const FdsConfig *fdsConfig, uint16_t mode, uint16_t tlx,
                uint16_t tly, uint16_t brx, uint16_t bry, uint16_t colour,
                uint16_t scale, int16_t value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFAE),
                                          __builtin_bswap16(0xFF5E)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
    uint16_t tlx;
    uint16_t tly;
    uint16_t brx;
    uint16_t bry;
    uint16_t colour;
    uint16_t scale;
    int16_t value;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .mode = __builtin_bswap16(mode),
                                            .tlx = __builtin_bswap16(tlx),
                                            .tly = __builtin_bswap16(tly),
                                            .brx = __builtin_bswap16(brx),
                                            .bry = __builtin_bswap16(bry),
                                            .colour = __builtin_bswap16(colour),
                                            .scale = __builtin_bswap16(scale),
                                            .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_slider unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_screenCopyPaste(const FdsConfig *fdsConfig, uint16_t xs, uint16_t ys,
                         uint16_t xd, uint16_t yd, uint16_t width,
                         uint16_t height) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFAD),
                                          __builtin_bswap16(0xFF5D)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t xs;
    uint16_t ys;
    uint16_t xd;
    uint16_t yd;
    uint16_t width;
    uint16_t height;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
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
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_bevelShadow(const FdsConfig *fdsConfig, uint16_t value,
                     uint16_t *oldBevel) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF98),
                                          __builtin_bswap16(0xFF3C)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
    uint16_t oldBevel;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_bevelShadow unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (oldBevel != NULL) *oldBevel = __builtin_bswap16(response.oldBevel);

  return stus && (response.ack == QDS_ACK);
}

bool gfx_bevelWidth(const FdsConfig *fdsConfig, uint16_t value,
                    uint16_t *oldWidth) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF99),
                                          __builtin_bswap16(0xFF3D)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
    uint16_t oldWidth;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_bevelWidth unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (oldWidth != NULL) *oldWidth = __builtin_bswap16(response.oldWidth);

  return stus && (response.ack == QDS_ACK);
}

bool gfx_bgCcolour(const FdsConfig *fdsConfig, uint16_t colour,
                   uint16_t *oldCol) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF6E),
                                          __builtin_bswap16(0xFFA4),
                                          __builtin_bswap16(0xFF48)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t colour;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType], .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
    uint16_t oldCol;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(
          fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (fdsConfig->deviceType == GOLDELOX ? sizeof(*oldCol) : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldCol != NULL) *oldCol = __builtin_bswap16(response.oldCol);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_outlineColour(const FdsConfig *fdsConfig, uint16_t colour,
                       uint16_t *oldCol) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF67),
                                          __builtin_bswap16(0xFF9D),
                                          __builtin_bswap16(0xFF41)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t colour;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType], .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
    uint16_t oldCol;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(
          fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (fdsConfig->deviceType == GOLDELOX ? sizeof(*oldCol) : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldCol != NULL) *oldCol = __builtin_bswap16(response.oldCol);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_contrast(const FdsConfig *fdsConfig, uint16_t contrast,
                  uint16_t *oldContrast) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF66),
                                          __builtin_bswap16(0xFF9C),
                                          __builtin_bswap16(0xFF40)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t contrast;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .contrast = __builtin_bswap16(contrast)};

  struct {
    uint8_t ack;
    uint16_t oldContrast;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), (uint8_t *)&response,
                        sizeof(response) - (fdsConfig->deviceType == GOLDELOX
                                                ? sizeof(*oldContrast)
                                                : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldContrast != NULL)
      *oldContrast = __builtin_bswap16(response.oldContrast);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_frameDelay(const FdsConfig *fdsConfig, uint16_t delayMsec,
                    uint16_t *oldDelay) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF69),
                                          __builtin_bswap16(0xFF9F),
                                          __builtin_bswap16(0xFF43)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t delayMsec;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .delayMsec = __builtin_bswap16(delayMsec)};

  struct {
    uint8_t ack;
    uint16_t oldDelay;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(
          fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (fdsConfig->deviceType == GOLDELOX ? sizeof(*oldDelay) : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldDelay != NULL) *oldDelay = __builtin_bswap16(response.oldDelay);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_linePattern(const FdsConfig *fdsConfig, uint16_t pattern,
                     uint16_t *oldPattern) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF65),
                                          __builtin_bswap16(0xFF9B),
                                          __builtin_bswap16(0xFF3F)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t pattern;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .pattern = __builtin_bswap16(pattern)};

  struct {
    uint8_t ack;
    uint16_t oldPattern;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), (uint8_t *)&response,
                        sizeof(response) - (fdsConfig->deviceType == GOLDELOX
                                                ? sizeof(*oldPattern)
                                                : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldPattern != NULL)
      *oldPattern = __builtin_bswap16(response.oldPattern);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_screenMode(const FdsConfig *fdsConfig, uint16_t mode,
                    uint16_t *oldMode) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFF68),
                                          __builtin_bswap16(0xFF9E),
                                          __builtin_bswap16(0xFF42)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldMode;
  } __attribute__((__packed__)) response;

  stus =
      fdsTransmitBuffer(
          fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
          sizeof(command1), (uint8_t *)&response,
          sizeof(response) -
              (fdsConfig->deviceType == GOLDELOX ? sizeof(*oldMode) : 0)) != 0;

  if (fdsConfig->deviceType != GOLDELOX) {
    if (oldMode != NULL) *oldMode = __builtin_bswap16(response.oldMode);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_transparency(const FdsConfig *fdsConfig, uint16_t mode,
                      uint16_t *oldMode) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFA0),
                                          __builtin_bswap16(0xFF44)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldMode;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_transparency unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (oldMode != NULL) *oldMode = __builtin_bswap16(response.oldMode);

  return stus && (response.ack == QDS_ACK);
}

bool gfx_transparentColour(const FdsConfig *fdsConfig, uint16_t colour,
                           uint16_t *oldColor) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFA1),
                                          __builtin_bswap16(0xFF45)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t colour;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType], .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
    uint16_t oldColor;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_transparentColour unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (oldColor != NULL) *oldColor = __builtin_bswap16(response.oldColor);

  return stus && (response.ack == QDS_ACK);
}

bool gfx_set(const FdsConfig *fdsConfig, uint16_t function, uint16_t value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFD8),
                                          __builtin_bswap16(0xFFCE),
                                          __builtin_bswap16(0xFF83)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t function;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .function = __builtin_bswap16(function),
              .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_get(const FdsConfig *fdsConfig, uint16_t mode, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFA6),
                                          __builtin_bswap16(0xFF4A)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_get unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool media_init(const FdsConfig *fdsConfig, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFB1),
                                          __builtin_bswap16(0xFF89),
                                          __builtin_bswap16(0xFF25)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool media_setAdd(const FdsConfig *fdsConfig, uint16_t hiAddr,
                  uint16_t loAddr) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFB9),
                                          __builtin_bswap16(0xFF93),
                                          __builtin_bswap16(0xFF2F)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t hiAddr;
    uint16_t loAddr;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .hiAddr = __builtin_bswap16(hiAddr),
              .loAddr = __builtin_bswap16(loAddr)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool media_setSector(const FdsConfig *fdsConfig, uint16_t hiAddr,
                     uint16_t loAddr) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFB8),
                                          __builtin_bswap16(0xFF92),
                                          __builtin_bswap16(0xFF2E)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t hiAddr;
    uint16_t loAddr;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .hiAddr = __builtin_bswap16(hiAddr),
              .loAddr = __builtin_bswap16(loAddr)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool media_readByte(const FdsConfig *fdsConfig, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFB7),
                                          __builtin_bswap16(0xFF8F),
                                          __builtin_bswap16(0xFF2B)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool media_readWord(const FdsConfig *fdsConfig, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFB6),
                                          __builtin_bswap16(0xFF8E),
                                          __builtin_bswap16(0xFF2A)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool media_writeByte(const FdsConfig *fdsConfig, uint16_t s, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFB5),
                                          __builtin_bswap16(0xFF8D),
                                          __builtin_bswap16(0xFF29)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t s;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType], .s = __builtin_bswap16(s)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool media_writeWord(const FdsConfig *fdsConfig, uint16_t value,
                     uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFB4),
                                          __builtin_bswap16(0xFF8C),
                                          __builtin_bswap16(0xFF28)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool media_flush(const FdsConfig *fdsConfig, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFB2),
                                          __builtin_bswap16(0xFF8A),
                                          __builtin_bswap16(0xFF26)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool media_image(const FdsConfig *fdsConfig, uint16_t x, uint16_t y) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFB3),
                                          __builtin_bswap16(0xFF8B),
                                          __builtin_bswap16(0xFF27)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .x = __builtin_bswap16(x),
                                            .y = __builtin_bswap16(y)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool media_video(const FdsConfig *fdsConfig, uint16_t x, uint16_t y) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFBB),
                                          __builtin_bswap16(0xFF95),
                                          __builtin_bswap16(0xFF31)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .x = __builtin_bswap16(x),
                                            .y = __builtin_bswap16(y)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool media_videoFrame(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                      int16_t frameNumber) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFBA),
                                          __builtin_bswap16(0xFF94),
                                          __builtin_bswap16(0xFF30)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    int16_t frameNumber;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .frameNumber = __builtin_bswap16(frameNumber)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_peekB(const FdsConfig *fdsConfig, uint16_t eveReg, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFF6),
                                          __builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t eveReg;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType], .eveReg = __builtin_bswap16(eveReg)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_peekB  unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool misc_pokeB(const FdsConfig *fdsConfig, uint16_t eveReg, uint16_t value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFF4),
                                          __builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t eveReg;
    uint16_t value;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .eveReg = __builtin_bswap16(eveReg),
                                            .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_pokeB  unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_peekW(const FdsConfig *fdsConfig, uint16_t eveReg, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFF5),
                                          __builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t eveReg;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType], .eveReg = __builtin_bswap16(eveReg)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_peekW  unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool misc_pokeW(const FdsConfig *fdsConfig, uint16_t eveReg, uint16_t value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFF3),
                                          __builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t eveReg;
    uint16_t value;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .eveReg = __builtin_bswap16(eveReg),
                                            .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_pokeW unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_peekM(const FdsConfig *fdsConfig, uint16_t address, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0027),
                                          __builtin_bswap16(0x0027)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t address;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .address = __builtin_bswap16(address)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_peekM  unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool misc_pokeM(const FdsConfig *fdsConfig, uint16_t address, uint16_t value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0028),
                                          __builtin_bswap16(0x0028)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t address;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .address = __builtin_bswap16(address),
              .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_pokeM unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_joystick(const FdsConfig *fdsConfig, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFD9),
                                          __builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_joystick unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool misc_beep(const FdsConfig *fdsConfig, uint16_t note,
               uint16_t duration_ms) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0xFFDA),
                                          __builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t note;
    uint16_t duration_ms;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .note = __builtin_bswap16(note),
              .duration_ms = __builtin_bswap16(duration_ms)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_beep unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_setbaudWait(const FdsConfig *fdsConfig, int16_t index) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0x000B),
                                          __builtin_bswap16(0x0026),
                                          __builtin_bswap16(0x0026)};

  bool stus = false;
  struct {
    uint16_t cmd;
    int16_t index;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .index = __builtin_bswap16(index)};

  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), NULL, 0) == 0;
  return stus;
}

bool sys_sleep(const FdsConfig *fdsConfig, uint16_t duration_s,
               uint16_t *duration) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF3B),
                                          __builtin_bswap16(0xFE6D)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t duration_s;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .duration_s = __builtin_bswap16(duration_s)};

  struct {
    uint8_t ack;
    uint16_t duration;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function sys_sleep unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (duration != NULL) *duration = __builtin_bswap16(response.duration);

  return stus && (response.ack == QDS_ACK);
}

bool sys_memFree(const FdsConfig *fdsConfig, uint16_t handle,
                 uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF24),
                                          __builtin_bswap16(0xFE5F)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType], .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function sys_memFree unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool sys_memHeap(const FdsConfig *fdsConfig, uint16_t *avail) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF23),
                                          __builtin_bswap16(0xFE5E)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t avail;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function sys_memHeap unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (avail != NULL) *avail = __builtin_bswap16(response.avail);

  return stus && (response.ack == QDS_ACK);
}

bool sys_getModel(const FdsConfig *fdsConfig, uint16_t *n, char *str) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0x0007),
                                          __builtin_bswap16(0x001A),
                                          __builtin_bswap16(0x001A)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), (uint8_t *)str, 3) == 3;
  if ((!stus) || (str[0] != QDS_ACK)) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
    return false;
  }
  const size_t dynSize = MIN(getResponseAsUint16((uint8_t *)str), (*n) - 1);
  (*n) = dynSize;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, NULL, 0,
                           (uint8_t *)str, dynSize) == dynSize;
  if (!stus) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
  } else {
    str[dynSize] = 0;
  }

  return stus;
}

bool sys_getVersion(const FdsConfig *fdsConfig, uint16_t *version) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0x0008),
                                          __builtin_bswap16(0x001B),
                                          __builtin_bswap16(0x001B)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t version;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (version != NULL) *version = __builtin_bswap16(response.version);

  return stus && (response.ack == QDS_ACK);
}

bool sys_getPmmC(const FdsConfig *fdsConfig, uint16_t *version) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0x0009),
                                          __builtin_bswap16(0x001C),
                                          __builtin_bswap16(0x001C)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t version;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (version != NULL) *version = __builtin_bswap16(response.version);

  return stus && (response.ack == QDS_ACK);
}

bool misc_screenSaverTimeout(const FdsConfig *fdsConfig, uint16_t timout_ms) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0x000C),
                                          __builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t timout_ms;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .timout_ms = __builtin_bswap16(timout_ms)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(
      command1.cmd != CMD_NOT_IMPL,
      "function misc_screenSaverTimeout unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_screenSaverSpeed(const FdsConfig *fdsConfig, uint16_t speed) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0x000D),
                                          __builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t speed;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .speed = __builtin_bswap16(speed)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_screenSaverSpeed unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_screenSaverMode(const FdsConfig *fdsConfig, uint16_t mode) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0x000E),
                                          __builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_screenSaverMode unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool touch_detectRegion(const FdsConfig *fdsConfig, uint16_t tlx, uint16_t tly,
                        uint16_t brx, uint16_t bry) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF39),
                                          __builtin_bswap16(0xFE6A)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t tlx;
    uint16_t tly;
    uint16_t brx;
    uint16_t bry;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .tlx = __builtin_bswap16(tlx),
                                            .tly = __builtin_bswap16(tly),
                                            .brx = __builtin_bswap16(brx),
                                            .bry = __builtin_bswap16(bry)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function touch_detectRegion unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool touch_set(const FdsConfig *fdsConfig, uint16_t mode) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF38),
                                          __builtin_bswap16(0xFE69)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function touch_set unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool touch_get(const FdsConfig *fdsConfig, uint16_t mode, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF37),
                                          __builtin_bswap16(0xFE68)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function touch_get unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool file_error(const FdsConfig *fdsConfig, uint16_t *errno) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF1F),
                                          __builtin_bswap16(0xFE58)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t errno;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_error unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (errno != NULL) *errno = __builtin_bswap16(response.errno);

  return stus && (response.ack == QDS_ACK);
}

bool file_count(const FdsConfig *fdsConfig, const char *filename,
                uint16_t *count) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0001),
                                          __builtin_bswap16(0x0001)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t count;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_count unimplemented for this screen");
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)filename, strlen(filename) + 1,
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (count != NULL) *count = __builtin_bswap16(response.count);

  return stus && (response.ack == QDS_ACK);
}

bool file_dir(const FdsConfig *fdsConfig, const char *filename,
              uint16_t *count) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0002),
                                          __builtin_bswap16(0x0002)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t count;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_dir unimplemented for this screen");
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)filename, strlen(filename) + 1,
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (count != NULL) *count = __builtin_bswap16(response.count);

  return stus && (response.ack == QDS_ACK);
}

bool file_findFirst(const FdsConfig *fdsConfig, const char *filename,
                    uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0006),
                                          __builtin_bswap16(0x0006)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_findFirst unimplemented for this screen");
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)filename, strlen(filename) + 1,
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_findFirstRet(const FdsConfig *fdsConfig, const char *filename,
                       uint16_t *length, char *str) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0024),
                                          __builtin_bswap16(0x0024)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), NULL, 0) != 0;
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)filename,
                        strlen(filename) + 1, (uint8_t *)str, 3) == 3;
  if ((!stus) || (str[0] != QDS_ACK)) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
    return false;
  }
  const size_t dynSize =
      MIN(getResponseAsUint16((uint8_t *)str), (*length) - 1);
  (*length) = dynSize;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, NULL, 0,
                           (uint8_t *)str, dynSize) == dynSize;
  if (!stus) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
  } else {
    str[dynSize] = 0;
  }

  return stus;
}

bool file_findNext(const FdsConfig *fdsConfig, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF1B),
                                          __builtin_bswap16(0xFE54)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_findNext unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_findNextRet(const FdsConfig *fdsConfig, uint16_t *length, char *str) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0025),
                                          __builtin_bswap16(0x0025)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), (uint8_t *)str, 3) == 3;
  if ((!stus) || (str[0] != QDS_ACK)) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
    return false;
  }
  const size_t dynSize =
      MIN(getResponseAsUint16((uint8_t *)str), (*length) - 1);
  (*length) = dynSize;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, NULL, 0,
                           (uint8_t *)str, dynSize) == dynSize;
  if (!stus) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
  } else {
    str[dynSize] = 0;
  }

  return stus;
}

bool file_exists(const FdsConfig *fdsConfig, const char *filename,
                 uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0005),
                                          __builtin_bswap16(0x0005)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_exists unimplemented for this screen");
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)filename, strlen(filename) + 1,
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_open(const FdsConfig *fdsConfig, const char *filename, char mode,
               uint16_t *handle) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x000A),
                                          __builtin_bswap16(0x000A)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
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
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), NULL, 0) != 0;
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)filename,
                        strlen(filename) + 1, NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command2, sizeof(command2),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (handle != NULL) *handle = __builtin_bswap16(response.handle);

  return stus && (response.ack == QDS_ACK);
}

bool file_close(const FdsConfig *fdsConfig, uint16_t handle, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF18),
                                          __builtin_bswap16(0xFE51)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType], .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_close unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_read(const FdsConfig *fdsConfig, uint16_t size, uint16_t handle,
               uint16_t *n, char *str) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x000C),
                                          __builtin_bswap16(0x000C)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t size;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .size = __builtin_bswap16(size),
              .handle = __builtin_bswap16(handle)};

  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), (uint8_t *)str, 3) == 3;
  if ((!stus) || (str[0] != QDS_ACK)) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
    return false;
  }
  const size_t dynSize = MIN(getResponseAsUint16((uint8_t *)str), (*n) - 1);
  (*n) = dynSize;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, NULL, 0,
                           (uint8_t *)str, dynSize) == dynSize;
  if (!stus) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
  } else {
    str[dynSize] = 0;
  }

  return stus;
}

bool file_seek(const FdsConfig *fdsConfig, uint16_t handle, uint16_t hiWord,
               uint16_t loWord, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF16),
                                          __builtin_bswap16(0xFE4F)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t hiWord;
    uint16_t loWord;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .handle = __builtin_bswap16(handle),
              .hiWord = __builtin_bswap16(hiWord),
              .loWord = __builtin_bswap16(loWord)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_seek unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_index(const FdsConfig *fdsConfig, uint16_t handle, uint16_t hiWord,
                uint16_t loWord, uint16_t recordNum, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF15),
                                          __builtin_bswap16(0xFE4E)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t hiWord;
    uint16_t loWord;
    uint16_t recordNum;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
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
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_tell(const FdsConfig *fdsConfig, uint16_t handle, uint16_t *status,
               uint16_t *hiWord, uint16_t *loWord) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x000F),
                                          __builtin_bswap16(0x000F)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType], .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
    uint16_t hiWord;
    uint16_t loWord;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_tell unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);
  ;
  if (hiWord != NULL) *hiWord = __builtin_bswap16(response.hiWord);
  ;
  if (loWord != NULL) *loWord = __builtin_bswap16(response.loWord);

  return stus && (response.ack == QDS_ACK);
}

bool file_write(const FdsConfig *fdsConfig, uint16_t size,
                const uint8_t *source, uint16_t handle, uint16_t *count) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0010),
                                          __builtin_bswap16(0x0010)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t size;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .size = __builtin_bswap16(size)};

  struct {
    uint16_t handle;
  } __attribute__((__packed__))
  command2 = {.handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t count;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_write unimplemented for this screen");
  fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                    sizeof(command1), NULL, 0);
  fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, source, size, NULL, 0);
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command2, sizeof(command2),
                           (uint8_t *)&response, sizeof(response));

  if (count != NULL) *count = __builtin_bswap16(response.count);

  return stus && (response.ack == QDS_ACK);
}

bool file_size(const FdsConfig *fdsConfig, uint16_t handle, uint16_t *status,
               uint16_t *hiWord, uint16_t *loWord) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x000E),
                                          __builtin_bswap16(0x000E)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType], .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
    uint16_t hiWord;
    uint16_t loWord;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_size unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);
  ;
  if (hiWord != NULL) *hiWord = __builtin_bswap16(response.hiWord);
  ;
  if (loWord != NULL) *loWord = __builtin_bswap16(response.loWord);

  return stus && (response.ack == QDS_ACK);
}

bool file_image(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                uint16_t handle, uint16_t *errno) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF11),
                                          __builtin_bswap16(0xFE4A)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t errno;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_image unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (errno != NULL) *errno = __builtin_bswap16(response.errno);

  return stus && (response.ack == QDS_ACK);
}

bool file_screenCapture(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                        uint16_t width, uint16_t height, uint16_t handle,
                        uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF10),
                                          __builtin_bswap16(0xFE49)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
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
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_putC(const FdsConfig *fdsConfig, uint16_t car, uint16_t handle,
               uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x001F),
                                          __builtin_bswap16(0x001F)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t car;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .car = __builtin_bswap16(car),
              .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_putC unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_getC(const FdsConfig *fdsConfig, uint16_t handle, uint16_t *car) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF0E),
                                          __builtin_bswap16(0xFE47)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType], .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t car;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_getC unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (car != NULL) *car = __builtin_bswap16(response.car);

  return stus && (response.ack == QDS_ACK);
}

bool file_putW(const FdsConfig *fdsConfig, uint16_t word, uint16_t handle,
               uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF0D),
                                          __builtin_bswap16(0xFE46)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t word;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .word = __builtin_bswap16(word),
              .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_putW unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_getW(const FdsConfig *fdsConfig, uint16_t handle, uint16_t *word) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF0C),
                                          __builtin_bswap16(0xFE45)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType], .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t word;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_getW unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (word != NULL) *word = __builtin_bswap16(response.word);

  return stus && (response.ack == QDS_ACK);
}

bool file_putS(const FdsConfig *fdsConfig, const char *cstr, uint16_t *count) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0020),
                                          __builtin_bswap16(0x0020)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t count;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_putS unimplemented for this screen");
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)cstr,
                           strlen(cstr) + 1, (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (count != NULL) *count = __builtin_bswap16(response.count);

  return stus && (response.ack == QDS_ACK);
}

bool file_getS(const FdsConfig *fdsConfig, uint16_t size, uint16_t handle,
               uint16_t *n, char *str) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0007),
                                          __builtin_bswap16(0x0007)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t size;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .size = __builtin_bswap16(size),
              .handle = __builtin_bswap16(handle)};

  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), (uint8_t *)str, 3) == 3;
  if ((!stus) || (str[0] != QDS_ACK)) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
    return false;
  }
  const size_t dynSize = MIN(getResponseAsUint16((uint8_t *)str), (*n) - 1);
  (*n) = dynSize;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, NULL, 0,
                           (uint8_t *)str, dynSize) == dynSize;
  if (!stus) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
  } else {
    str[dynSize] = 0;
  }

  return stus;
}

bool file_erase(const FdsConfig *fdsConfig, const char *filename,
                uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0003),
                                          __builtin_bswap16(0x0003)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_erase unimplemented for this screen");
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)filename, strlen(filename) + 1,
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_rewind(const FdsConfig *fdsConfig, uint16_t handle,
                 uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF08),
                                          __builtin_bswap16(0xFE41)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType], .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_rewind unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_loadFunction(const FdsConfig *fdsConfig, const char *filename,
                       uint16_t *pointer) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0008),
                                          __builtin_bswap16(0x0008)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t pointer;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_loadFunction unimplemented for this screen");
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)filename, strlen(filename) + 1,
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (pointer != NULL) *pointer = __builtin_bswap16(response.pointer);

  return stus && (response.ack == QDS_ACK);
}

bool file_callFunction(const FdsConfig *fdsConfig, uint16_t handle, uint16_t n,
                       const uint16_t *args, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0019),
                                          __builtin_bswap16(0x0019)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t n;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .n = __builtin_bswap16(n)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_callFunction unimplemented for this screen");
  fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                    sizeof(command1), NULL, 0);
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)args,
                           n * sizeof(*args), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool file_run(const FdsConfig *fdsConfig, const char *filename, uint16_t n,
              const uint16_t *args, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x000D),
                                          __builtin_bswap16(0x000D)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };
  struct {
    uint16_t n;
  } __attribute__((__packed__)) command2 = {.n = __builtin_bswap16(n)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_run unimplemented for this screen");
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), NULL, 0) != 0;
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)filename,
                        strlen(filename) + 1, NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command2, sizeof(command2), NULL, 0);
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)args,
                           n * sizeof(*args), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool file_exec(const FdsConfig *fdsConfig, const char *filename, uint16_t n,
               const uint16_t *args, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0004),
                                          __builtin_bswap16(0x0004)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };
  struct {
    uint16_t n;
  } __attribute__((__packed__)) command2 = {.n = __builtin_bswap16(n)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_run unimplemented for this screen");
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), NULL, 0) != 0;
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)filename,
                        strlen(filename) + 1, NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command2, sizeof(command2), NULL, 0);
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)args,
                           n * sizeof(*args), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool file_loadImageControl(const FdsConfig *fdsConfig, const char *filename1,
                           const char *filename2, uint16_t mode,
                           uint16_t *handle) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0009),
                                          __builtin_bswap16(0x0009)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
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
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), NULL, 0) != 0;
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)filename1,
                        strlen(filename1) + 1, NULL, 0) != 0;
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)filename2,
                        strlen(filename2) + 1, NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command2, sizeof(command2),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (handle != NULL) *handle = __builtin_bswap16(response.handle);

  return stus && (response.ack == QDS_ACK);
}

bool file_mount(const FdsConfig *fdsConfig, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF03),
                                          __builtin_bswap16(0xFE3C)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_mount unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_unmount(const FdsConfig *fdsConfig) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF02),
                                          __builtin_bswap16(0xFE3B)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_unmount unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool file_playWAV(const FdsConfig *fdsConfig, const char *filename,
                  uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x000B),
                                          __builtin_bswap16(0x000B)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_playWAV unimplemented for this screen");
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)filename, strlen(filename) + 1,
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_writeString(const FdsConfig *fdsConfig, uint16_t handle,
                      const char *cstr, uint16_t *pointer) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0x0021),
                                          __builtin_bswap16(0x0021)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType], .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t pointer;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_writeString unimplemented for this screen");
  stus =
      fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                        sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)cstr,
                           strlen(cstr) + 1, (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (pointer != NULL) *pointer = __builtin_bswap16(response.pointer);

  return stus && (response.ack == QDS_ACK);
}

bool snd_volume(const FdsConfig *fdsConfig, uint16_t level) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF00),
                                          __builtin_bswap16(0xFE35)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t level;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .level = __builtin_bswap16(level)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_volume unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool snd_pitch(const FdsConfig *fdsConfig, uint16_t rate, uint16_t *oldRate) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFEFF),
                                          __builtin_bswap16(0xFE34)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t rate;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .rate = __builtin_bswap16(rate)};

  struct {
    uint8_t ack;
    uint16_t oldRate;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_pitch unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (oldRate != NULL) *oldRate = __builtin_bswap16(response.oldRate);

  return stus && (response.ack == QDS_ACK);
}

bool snd_bufSize(const FdsConfig *fdsConfig, uint16_t bufferSize) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFEFE),
                                          __builtin_bswap16(0xFE33)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t bufferSize;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .bufferSize = __builtin_bswap16(bufferSize)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_bufSize unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool snd_stop(const FdsConfig *fdsConfig) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFEFD),
                                          __builtin_bswap16(0xFE32)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_stop unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool snd_pause(const FdsConfig *fdsConfig) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFEFC),
                                          __builtin_bswap16(0xFE31)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_pause unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool snd_continue(const FdsConfig *fdsConfig) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFEFB),
                                          __builtin_bswap16(0xFE30)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_continue unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool snd_playing(const FdsConfig *fdsConfig, uint16_t *togo) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFEFA),
                                          __builtin_bswap16(0xFE2F)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t togo;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_playing unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (togo != NULL) *togo = __builtin_bswap16(response.togo);

  return stus && (response.ack == QDS_ACK);
}

bool img_setPosition(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
                     uint16_t xpos, uint16_t ypos, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF4E),
                                          __builtin_bswap16(0xFE8A)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
    uint16_t xpos;
    uint16_t ypos;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
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
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_enable(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
                uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF4D),
                                          __builtin_bswap16(0xFE89)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_enable  unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_disable(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
                 uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF4C),
                                          __builtin_bswap16(0xFE88)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_disable unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_darken(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
                uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF4B),
                                          __builtin_bswap16(0xFE87)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_darken unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_lighten(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
                 uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF4A),
                                          __builtin_bswap16(0xFE86)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_lighten unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_setWord(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
                 uint16_t offset, uint16_t value, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF49),
                                          __builtin_bswap16(0xFE85)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
    uint16_t offset;
    uint16_t value;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
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
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_getWord(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
                 uint16_t offset, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF48),
                                          __builtin_bswap16(0xFE84)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
    uint16_t offset;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .handle = __builtin_bswap16(handle),
              .index = __builtin_bswap16(index),
              .offset = __builtin_bswap16(offset)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_getWord unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool img_show(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
              uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF47),
                                          __builtin_bswap16(0xFE83)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_show unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_setAttributes(const FdsConfig *fdsConfig, uint16_t handle,
                       int16_t index, uint16_t value, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF46),
                                          __builtin_bswap16(0xFE82)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
    uint16_t value;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .index = __builtin_bswap16(index),
                                            .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_setAttributes unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_clearAttributes(const FdsConfig *fdsConfig, uint16_t handle,
                         int16_t index, uint16_t value, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF45),
                                          __builtin_bswap16(0xFE81)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
    uint16_t value;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .index = __builtin_bswap16(index),
                                            .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_clearAttributes unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_touched(const FdsConfig *fdsConfig, uint16_t handle, int16_t index,
                 int16_t *value) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF44),
                                          __builtin_bswap16(0xFE80)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    int16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_touched unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool img_blitComtoDisplay(const FdsConfig *fdsConfig, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height,
                          const uint16_t *data) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(0x000A),
                                          __builtin_bswap16(0x0023),
                                          __builtin_bswap16(0x0023)};
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .width = __builtin_bswap16(width),
              .height = __builtin_bswap16(height)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_blitComtoDisplay unimplemented for this screen");
  fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                    sizeof(command1), NULL, 0);

  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__, (uint8_t *)data,
                           sizeof(*data) * width * height, (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus;
}

bool bus_in(const FdsConfig *fdsConfig, uint16_t *busState) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFD3),
                                          __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t busState;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function bus_in unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (busState != NULL) *busState = __builtin_bswap16(response.busState);

  return stus && (response.ack == QDS_ACK);
}

bool bus_out(const FdsConfig *fdsConfig, uint16_t busState) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFD2),
                                          __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t busState;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .busState = __builtin_bswap16(busState)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function bus_out unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool bus_read(const FdsConfig *fdsConfig, uint16_t *busState) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFCF),
                                          __builtin_bswap16(0xFF86)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fdsConfig->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t busState;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function bus_read unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (busState != NULL) *busState = __builtin_bswap16(response.busState);

  return stus && (response.ack == QDS_ACK);
}

bool bus_set(const FdsConfig *fdsConfig, uint16_t dirMask) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFD1),
                                          __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t dirMask;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .dirMask = __builtin_bswap16(dirMask)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function bus_set unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool bus_write(const FdsConfig *fdsConfig, uint16_t bitfield) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFD0),
                                          __builtin_bswap16(0xFF87)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t bitfield;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fdsConfig->deviceType],
              .bitfield = __builtin_bswap16(bitfield)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function bus_write unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool pin_hi(const FdsConfig *fdsConfig, uint16_t pin, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFD6),
                                          __builtin_bswap16(0xFF8F)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t pin;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .pin = __builtin_bswap16(pin)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function pin_hi unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool pin_lo(const FdsConfig *fdsConfig, uint16_t pin, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFD5),
                                          __builtin_bswap16(0xFF8E)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t pin;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .pin = __builtin_bswap16(pin)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function pin_lo unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool pin_read(const FdsConfig *fdsConfig, uint16_t pin, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFD4),
                                          __builtin_bswap16(0xFF8C)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t pin;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .pin = __builtin_bswap16(pin)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function pin_read unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool pin_set_picaso(const FdsConfig *fdsConfig, uint16_t mode, uint16_t pin,
                    uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFFD7),
                                          __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
    uint16_t pin;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .mode = __builtin_bswap16(mode),
                                            .pin = __builtin_bswap16(pin)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function pin_set_picaso unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool pin_set_diablo(const FdsConfig *fdsConfig, uint16_t mode, uint16_t pin,
                    uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fdsConfig);
  RET_UNLESS_4DSYS_BOOL(fdsConfig);
  static const uint16_t cmds[AUTO_4DS] = {__builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(CMD_NOT_IMPL),
                                          __builtin_bswap16(0xFF90)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
    uint16_t pin;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fdsConfig->deviceType],
                                            .mode = __builtin_bswap16(mode),
                                            .pin = __builtin_bswap16(pin)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function pin_set_diablo unimplemented for this screen");
  stus = fdsTransmitBuffer(fdsConfig, __FUNCTION__, __LINE__,
                           (uint8_t *)&command1, sizeof(command1),
                           (uint8_t *)&response, sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}
