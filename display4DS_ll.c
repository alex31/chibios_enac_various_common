#include "display4DS_ll.h"

/*
    This code has been generated from API description
    All hand modifications will be lost at next generation
*/
static bool gfx_polyxxx(const FdsDriver *fds, uint16_t cmd, uint16_t n,
                        const uint16_t vx[], const uint16_t vy[],
                        uint16_t color) {
  struct {
    uint16_t cmd;
    uint16_t n;
  } command1 = {.cmd = cmd, .n = __builtin_bswap16(n)};

  struct {
    uint8_t ack;
  } response;

  fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                    sizeof(command1), NULL, 0);

  fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)vx,
                    sizeof(vx[0]) * n, NULL, 0);

  fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)vy,
                    sizeof(vy[0]) * n, NULL, 0);

  color = __builtin_bswap16(color);
  return fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&color,
                           sizeof(color), (uint8_t *)&response,
                           sizeof(response)) != 0;
}

bool txt_moveCursor(const FdsDriver *fds, uint16_t line, uint16_t column) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFE4), __builtin_bswap16(0xFFE9),
      __builtin_bswap16(0xFFF0), __builtin_bswap16(0xFFE9)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t line;
    uint16_t column;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .line = __builtin_bswap16(line),
              .column = __builtin_bswap16(column)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool txt_putCh(const FdsDriver *fds, uint16_t car) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFFE), __builtin_bswap16(0xFFFE),
      __builtin_bswap16(0xFFFE), __builtin_bswap16(0xFFFE)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t car;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .car = __builtin_bswap16(car)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool txt_putStr(const FdsDriver *fds, const char *cstr, uint16_t *length) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0x0006), __builtin_bswap16(0x0018),
      __builtin_bswap16(0x0018), __builtin_bswap16(0x0018)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t length;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(
             fds, __FUNCTION__, __LINE__, (uint8_t *)cstr, strlen(cstr) + 1,
             (uint8_t *)&response,
             sizeof(response) -
                 (fds->deviceType == FDS_GOLDELOX ? sizeof(*length) : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (length != NULL) *length = __builtin_bswap16(response.length);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_charWidth(const FdsDriver *fds, char car, uint16_t *width) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0x0002), __builtin_bswap16(0x001E),
      __builtin_bswap16(0x001E), __builtin_bswap16(0x001E)};

  bool stus = false;
  struct {
    uint16_t cmd;
    char car;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .car = __builtin_bswap16(car)};

  struct {
    uint8_t ack;
    uint16_t width;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (width != NULL) *width = __builtin_bswap16(response.width);

  return stus && (response.ack == QDS_ACK);
}

bool txt_charHeight(const FdsDriver *fds, char car, uint16_t *height) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0x0001), __builtin_bswap16(0x001D),
      __builtin_bswap16(0x001D), __builtin_bswap16(0x001D)};

  bool stus = false;
  struct {
    uint16_t cmd;
    char car;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .car = __builtin_bswap16(car)};

  struct {
    uint8_t ack;
    uint16_t height;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (height != NULL) *height = __builtin_bswap16(response.height);

  return stus && (response.ack == QDS_ACK);
}

bool txt_fgColour(const FdsDriver *fds, uint16_t colour, uint16_t *oldCol) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF7F), __builtin_bswap16(0xFFE7),
      __builtin_bswap16(0xFFEE), __builtin_bswap16(0xFFE7)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t colour;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
    uint16_t oldCol;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(
             fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) -
                 (fds->deviceType == FDS_GOLDELOX ? sizeof(*oldCol) : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldCol != NULL) *oldCol = __builtin_bswap16(response.oldCol);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_bgColour(const FdsDriver *fds, uint16_t colour, uint16_t *oldCol) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF7E), __builtin_bswap16(0xFFE6),
      __builtin_bswap16(0xFFED), __builtin_bswap16(0xFFE6)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t colour;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
    uint16_t oldCol;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(
             fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) -
                 (fds->deviceType == FDS_GOLDELOX ? sizeof(*oldCol) : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldCol != NULL) *oldCol = __builtin_bswap16(response.oldCol);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_fontID(const FdsDriver *fds, uint16_t id, uint16_t *oldFont) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF7D), __builtin_bswap16(0xFFE5),
      __builtin_bswap16(0xFFEC), __builtin_bswap16(0xFFE5)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t id;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .id = __builtin_bswap16(id)};

  struct {
    uint8_t ack;
    uint16_t oldFont;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(
             fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) -
                 (fds->deviceType == FDS_GOLDELOX ? sizeof(*oldFont) : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldFont != NULL) *oldFont = __builtin_bswap16(response.oldFont);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_widthMult(const FdsDriver *fds, uint16_t wMultiplier,
                   uint16_t *oldMul) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF7C), __builtin_bswap16(0xFFE4),
      __builtin_bswap16(0xFFEB), __builtin_bswap16(0xFFE4)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t wMultiplier;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .wMultiplier = __builtin_bswap16(wMultiplier)};

  struct {
    uint8_t ack;
    uint16_t oldMul;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(
             fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) -
                 (fds->deviceType == FDS_GOLDELOX ? sizeof(*oldMul) : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldMul != NULL) *oldMul = __builtin_bswap16(response.oldMul);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_heightMult(const FdsDriver *fds, uint16_t hMultiplier,
                    uint16_t *oldMul) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF7B), __builtin_bswap16(0xFFE3),
      __builtin_bswap16(0xFFEA), __builtin_bswap16(0xFFE3)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t hMultiplier;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .hMultiplier = __builtin_bswap16(hMultiplier)};

  struct {
    uint8_t ack;
    uint16_t oldMul;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(
             fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) -
                 (fds->deviceType == FDS_GOLDELOX ? sizeof(*oldMul) : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldMul != NULL) *oldMul = __builtin_bswap16(response.oldMul);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_xgap(const FdsDriver *fds, uint16_t xGap, uint16_t *oldGap) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF7A), __builtin_bswap16(0xFFE2),
      __builtin_bswap16(0xFFE9), __builtin_bswap16(0xFFE2)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t xGap;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .xGap = __builtin_bswap16(xGap)};

  struct {
    uint8_t ack;
    uint16_t oldGap;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(
             fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) -
                 (fds->deviceType == FDS_GOLDELOX ? sizeof(*oldGap) : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldGap != NULL) *oldGap = __builtin_bswap16(response.oldGap);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_ygap(const FdsDriver *fds, uint16_t yGap, uint16_t *oldGap) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF79), __builtin_bswap16(0xFFE1),
      __builtin_bswap16(0xFFE8), __builtin_bswap16(0xFFE1)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t yGap;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .yGap = __builtin_bswap16(yGap)};

  struct {
    uint8_t ack;
    uint16_t oldGap;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(
             fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) -
                 (fds->deviceType == FDS_GOLDELOX ? sizeof(*oldGap) : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldGap != NULL) *oldGap = __builtin_bswap16(response.oldGap);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_bold(const FdsDriver *fds, uint16_t mode, uint16_t *oldBold) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF76), __builtin_bswap16(0xFFDE),
      __builtin_bswap16(0xFFE5), __builtin_bswap16(0xFFDE)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldBold;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(
             fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) -
                 (fds->deviceType == FDS_GOLDELOX ? sizeof(*oldBold) : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldBold != NULL) *oldBold = __builtin_bswap16(response.oldBold);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_inverse(const FdsDriver *fds, uint16_t mode, uint16_t *oldInv) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF74), __builtin_bswap16(0xFFDC),
      __builtin_bswap16(0xFFE3), __builtin_bswap16(0xFFDC)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldInv;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(
             fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) -
                 (fds->deviceType == FDS_GOLDELOX ? sizeof(*oldInv) : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldInv != NULL) *oldInv = __builtin_bswap16(response.oldInv);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_italic(const FdsDriver *fds, uint16_t mode, uint16_t *oldItal) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF75), __builtin_bswap16(0xFFDD),
      __builtin_bswap16(0xFFE4), __builtin_bswap16(0xFFDD)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldItal;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(
             fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) -
                 (fds->deviceType == FDS_GOLDELOX ? sizeof(*oldItal) : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldItal != NULL) *oldItal = __builtin_bswap16(response.oldItal);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_opacity(const FdsDriver *fds, uint16_t mode, uint16_t *oldOpa) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF77), __builtin_bswap16(0xFFDF),
      __builtin_bswap16(0xFFE6), __builtin_bswap16(0xFFDF)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldOpa;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(
             fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) -
                 (fds->deviceType == FDS_GOLDELOX ? sizeof(*oldOpa) : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldOpa != NULL) *oldOpa = __builtin_bswap16(response.oldOpa);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_underline(const FdsDriver *fds, uint16_t mode, uint16_t *oldUnder) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF73), __builtin_bswap16(0xFFDB),
      __builtin_bswap16(0xFFE2), __builtin_bswap16(0xFFDB)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldUnder;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response) - (fds->deviceType == FDS_GOLDELOX
                                                   ? sizeof(*oldUnder)
                                                   : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldUnder != NULL) *oldUnder = __builtin_bswap16(response.oldUnder);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_attributes(const FdsDriver *fds, uint16_t bitfield,
                    uint16_t *oldAttr) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF72), __builtin_bswap16(0xFFDA),
      __builtin_bswap16(0xFFE1), __builtin_bswap16(0xFFDA)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t bitfield;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .bitfield = __builtin_bswap16(bitfield)};

  struct {
    uint8_t ack;
    uint16_t oldAttr;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(
             fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) -
                 (fds->deviceType == FDS_GOLDELOX ? sizeof(*oldAttr) : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldAttr != NULL) *oldAttr = __builtin_bswap16(response.oldAttr);
  }

  return stus && (response.ack == QDS_ACK);
}

bool txt_set(const FdsDriver *fds, uint16_t function, uint16_t value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFE3), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t function;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .function = __builtin_bswap16(function),
              .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function txt_set unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool txt_wrap(const FdsDriver *fds, uint16_t xpos, uint16_t *oldWrap) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD9),
      __builtin_bswap16(0xFFE0), __builtin_bswap16(0xFFD9)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t xpos;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .xpos = __builtin_bswap16(xpos)};

  struct {
    uint8_t ack;
    uint16_t oldWrap;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function txt_wrap unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (oldWrap != NULL) *oldWrap = __builtin_bswap16(response.oldWrap);

  return stus && (response.ack == QDS_ACK);
}

bool gfx_cls(const FdsDriver *fds) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFD7), __builtin_bswap16(0xFFCD),
      __builtin_bswap16(0xFF82), __builtin_bswap16(0xFFCD)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_changeColour(const FdsDriver *fds, uint16_t oldColour,
                      uint16_t newColour) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFBE), __builtin_bswap16(0xFFB4),
      __builtin_bswap16(0xFF69), __builtin_bswap16(0xFFB4)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t oldColour;
    uint16_t newColour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .oldColour = __builtin_bswap16(oldColour),
              .newColour = __builtin_bswap16(newColour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_circle(const FdsDriver *fds, uint16_t x, uint16_t y, uint16_t radius,
                uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFCD), __builtin_bswap16(0xFFC3),
      __builtin_bswap16(0xFF78), __builtin_bswap16(0xFFC3)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t radius;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .radius = __builtin_bswap16(radius),
              .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_circleFilled(const FdsDriver *fds, uint16_t x, uint16_t y,
                      uint16_t radius, uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFCC), __builtin_bswap16(0xFFC2),
      __builtin_bswap16(0xFF77), __builtin_bswap16(0xFFC2)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t radius;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .radius = __builtin_bswap16(radius),
              .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_line(const FdsDriver *fds, uint16_t x1, uint16_t y1, uint16_t x2,
              uint16_t y2, uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFD2), __builtin_bswap16(0xFFC8),
      __builtin_bswap16(0xFF7D), __builtin_bswap16(0xFFC8)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .x1 = __builtin_bswap16(x1),
              .y1 = __builtin_bswap16(y1),
              .x2 = __builtin_bswap16(x2),
              .y2 = __builtin_bswap16(y2),
              .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_rectangle(const FdsDriver *fds, uint16_t tlx, uint16_t tly,
                   uint16_t brx, uint16_t bry, uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFCF), __builtin_bswap16(0xFFC5),
      __builtin_bswap16(0xFF7A), __builtin_bswap16(0xFFC5)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t tlx;
    uint16_t tly;
    uint16_t brx;
    uint16_t bry;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .tlx = __builtin_bswap16(tlx),
              .tly = __builtin_bswap16(tly),
              .brx = __builtin_bswap16(brx),
              .bry = __builtin_bswap16(bry),
              .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_rectangleFilled(const FdsDriver *fds, uint16_t tlx, uint16_t tly,
                         uint16_t brx, uint16_t bry, uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFCE), __builtin_bswap16(0xFFC4),
      __builtin_bswap16(0xFF79), __builtin_bswap16(0xFFC4)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t tlx;
    uint16_t tly;
    uint16_t brx;
    uint16_t bry;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .tlx = __builtin_bswap16(tlx),
              .tly = __builtin_bswap16(tly),
              .brx = __builtin_bswap16(brx),
              .bry = __builtin_bswap16(bry),
              .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_polyline(const FdsDriver *fds, uint16_t n, const uint16_t vx[],
                  const uint16_t vy[], uint16_t color) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0x0005), __builtin_bswap16(0x0015),
      __builtin_bswap16(0x0015), __builtin_bswap16(0x0015)};

  return gfx_polyxxx(fds, cmds[fds->deviceType], n, vx, vy, color);
}
bool gfx_polygon(const FdsDriver *fds, uint16_t n, const uint16_t vx[],
                 const uint16_t vy[], uint16_t color) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0x0004), __builtin_bswap16(0x0013),
      __builtin_bswap16(0x0013), __builtin_bswap16(0x0013)};

  return gfx_polyxxx(fds, cmds[fds->deviceType], n, vx, vy, color);
}
bool gfx_polygonFilled(const FdsDriver *fds, uint16_t n, const uint16_t vx[],
                       const uint16_t vy[], uint16_t color) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0014),
      __builtin_bswap16(0x0014), __builtin_bswap16(0x0014)};
  osalDbgAssert(cmds[fds->deviceType] != CMD_NOT_IMPL,
                "function gfx_polygonFilled unimplemented for this screen");
  return gfx_polyxxx(fds, cmds[fds->deviceType], n, vx, vy, color);
}
bool gfx_triangle(const FdsDriver *fds, uint16_t x1, uint16_t y1, uint16_t x2,
                  uint16_t y2, uint16_t x3, uint16_t y3, uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFC9), __builtin_bswap16(0xFFBF),
      __builtin_bswap16(0xFF74), __builtin_bswap16(0xFFBF)};

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
  command1 = {.cmd = cmds[fds->deviceType],
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

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_triangleFilled(const FdsDriver *fds, uint16_t x1, uint16_t y1,
                        uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
                        uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFA9),
      __builtin_bswap16(0xFF59), __builtin_bswap16(0xFFA9)};

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
  command1 = {.cmd = cmds[fds->deviceType],
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
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_orbit(const FdsDriver *fds, uint16_t angle, uint16_t distance,
               uint16_t *Xdist, uint16_t *Ydist) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0x0003), __builtin_bswap16(0x0012),
      __builtin_bswap16(0x0012), __builtin_bswap16(0x0012)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t angle;
    uint16_t distance;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .angle = __builtin_bswap16(angle),
              .distance = __builtin_bswap16(distance)};

  struct {
    uint8_t ack;
    uint16_t Xdist;
    uint16_t Ydist;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (Xdist != NULL) *Xdist = __builtin_bswap16(response.Xdist);
  ;
  if (Ydist != NULL) *Ydist = __builtin_bswap16(response.Ydist);

  return stus && (response.ack == QDS_ACK);
}

bool gfx_putPixel(const FdsDriver *fds, uint16_t x, uint16_t y,
                  uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFCB), __builtin_bswap16(0xFFC1),
      __builtin_bswap16(0xFF76), __builtin_bswap16(0xFFC1)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_getPixel(const FdsDriver *fds, uint16_t x, uint16_t y,
                  uint16_t *colour) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFCA), __builtin_bswap16(0xFFC0),
      __builtin_bswap16(0xFF75), __builtin_bswap16(0xFFC0)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .x = __builtin_bswap16(x),
                                            .y = __builtin_bswap16(y)};

  struct {
    uint8_t ack;
    uint16_t colour;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (colour != NULL) *colour = __builtin_bswap16(response.colour);

  return stus && (response.ack == QDS_ACK);
}

bool gfx_moveTo(const FdsDriver *fds, uint16_t x, uint16_t y) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFD6), __builtin_bswap16(0xFFCC),
      __builtin_bswap16(0xFF81), __builtin_bswap16(0xFFCC)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .x = __builtin_bswap16(x),
                                            .y = __builtin_bswap16(y)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_lineTo(const FdsDriver *fds, uint16_t x, uint16_t y) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFD4), __builtin_bswap16(0xFFCA),
      __builtin_bswap16(0xFF7F), __builtin_bswap16(0xFFCA)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .x = __builtin_bswap16(x),
                                            .y = __builtin_bswap16(y)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_clipping(const FdsDriver *fds, uint16_t mode) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF6C), __builtin_bswap16(0xFFA2),
      __builtin_bswap16(0xFF46), __builtin_bswap16(0xFFA2)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_clipWindow(const FdsDriver *fds, uint16_t tlx, uint16_t tly,
                    uint16_t brx, uint16_t bry) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFBF), __builtin_bswap16(0xFFB5),
      __builtin_bswap16(0xFF6A), __builtin_bswap16(0xFFB5)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t tlx;
    uint16_t tly;
    uint16_t brx;
    uint16_t bry;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .tlx = __builtin_bswap16(tlx),
                                            .tly = __builtin_bswap16(tly),
                                            .brx = __builtin_bswap16(brx),
                                            .bry = __builtin_bswap16(bry)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_setClipRegion(const FdsDriver *fds) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFBC), __builtin_bswap16(0xFFB3),
      __builtin_bswap16(0xFF68), __builtin_bswap16(0xFFB3)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_ellipse(const FdsDriver *fds, uint16_t x, uint16_t y, uint16_t xrad,
                 uint16_t yrad, uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFB2),
      __builtin_bswap16(0xFF67), __builtin_bswap16(0xFFB2)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t xrad;
    uint16_t yrad;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
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
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_ellipseFilled(const FdsDriver *fds, uint16_t x, uint16_t y,
                       uint16_t xrad, uint16_t yrad, uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFB1),
      __builtin_bswap16(0xFF66), __builtin_bswap16(0xFFB1)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t xrad;
    uint16_t yrad;
    uint16_t colour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
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
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_button(const FdsDriver *fds, uint16_t state, uint16_t x, uint16_t y,
                uint16_t buttoncolour, uint16_t txtcolour, uint16_t font,
                uint16_t txtWidth, uint16_t txtHeight, const char *cstr) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0011),
      __builtin_bswap16(0x0011), __builtin_bswap16(0x0011)};

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
  command1 = {.cmd = cmds[fds->deviceType],
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
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)cstr,
                           strlen(cstr) + 1, (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_panel(const FdsDriver *fds, uint16_t state, uint16_t x, uint16_t y,
               uint16_t width, uint16_t height, uint16_t colour) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFAF),
      __builtin_bswap16(0xFF5F), __builtin_bswap16(0xFFAF)};

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
  command1 = {.cmd = cmds[fds->deviceType],
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
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_slider(const FdsDriver *fds, uint16_t mode, uint16_t tlx, uint16_t tly,
                uint16_t brx, uint16_t bry, uint16_t colour, uint16_t scale,
                int16_t value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFAE),
      __builtin_bswap16(0xFF5E), __builtin_bswap16(0xFFAE)};

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
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
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
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_screenCopyPaste(const FdsDriver *fds, uint16_t xs, uint16_t ys,
                         uint16_t xd, uint16_t yd, uint16_t width,
                         uint16_t height) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFAD),
      __builtin_bswap16(0xFF5D), __builtin_bswap16(0xFFAD)};

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
  command1 = {.cmd = cmds[fds->deviceType],
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
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_bevelShadow(const FdsDriver *fds, uint16_t value, uint16_t *oldBevel) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF98),
      __builtin_bswap16(0xFF3C), __builtin_bswap16(0xFF98)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
    uint16_t oldBevel;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_bevelShadow unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (oldBevel != NULL) *oldBevel = __builtin_bswap16(response.oldBevel);

  return stus && (response.ack == QDS_ACK);
}

bool gfx_bevelWidth(const FdsDriver *fds, uint16_t value, uint16_t *oldWidth) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF99),
      __builtin_bswap16(0xFF3D), __builtin_bswap16(0xFF99)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
    uint16_t oldWidth;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_bevelWidth unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (oldWidth != NULL) *oldWidth = __builtin_bswap16(response.oldWidth);

  return stus && (response.ack == QDS_ACK);
}

bool gfx_bgCcolour(const FdsDriver *fds, uint16_t colour, uint16_t *oldCol) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF6E), __builtin_bswap16(0xFFA4),
      __builtin_bswap16(0xFF48), __builtin_bswap16(0xFFA4)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t colour;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
    uint16_t oldCol;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(
             fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) -
                 (fds->deviceType == FDS_GOLDELOX ? sizeof(*oldCol) : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldCol != NULL) *oldCol = __builtin_bswap16(response.oldCol);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_outlineColour(const FdsDriver *fds, uint16_t colour,
                       uint16_t *oldCol) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF67), __builtin_bswap16(0xFF9D),
      __builtin_bswap16(0xFF41), __builtin_bswap16(0xFF9D)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t colour;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
    uint16_t oldCol;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(
             fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) -
                 (fds->deviceType == FDS_GOLDELOX ? sizeof(*oldCol) : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldCol != NULL) *oldCol = __builtin_bswap16(response.oldCol);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_contrast(const FdsDriver *fds, uint16_t contrast,
                  uint16_t *oldContrast) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF66), __builtin_bswap16(0xFF9C),
      __builtin_bswap16(0xFF40), __builtin_bswap16(0xFF9C)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t contrast;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .contrast = __builtin_bswap16(contrast)};

  struct {
    uint8_t ack;
    uint16_t oldContrast;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response) - (fds->deviceType == FDS_GOLDELOX
                                                   ? sizeof(*oldContrast)
                                                   : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldContrast != NULL)
      *oldContrast = __builtin_bswap16(response.oldContrast);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_frameDelay(const FdsDriver *fds, uint16_t delayMsec,
                    uint16_t *oldDelay) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF69), __builtin_bswap16(0xFF9F),
      __builtin_bswap16(0xFF43), __builtin_bswap16(0xFF9F)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t delayMsec;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .delayMsec = __builtin_bswap16(delayMsec)};

  struct {
    uint8_t ack;
    uint16_t oldDelay;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response) - (fds->deviceType == FDS_GOLDELOX
                                                   ? sizeof(*oldDelay)
                                                   : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldDelay != NULL) *oldDelay = __builtin_bswap16(response.oldDelay);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_linePattern(const FdsDriver *fds, uint16_t pattern,
                     uint16_t *oldPattern) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF65), __builtin_bswap16(0xFF9B),
      __builtin_bswap16(0xFF3F), __builtin_bswap16(0xFF9B)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t pattern;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .pattern = __builtin_bswap16(pattern)};

  struct {
    uint8_t ack;
    uint16_t oldPattern;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response) - (fds->deviceType == FDS_GOLDELOX
                                                   ? sizeof(*oldPattern)
                                                   : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldPattern != NULL)
      *oldPattern = __builtin_bswap16(response.oldPattern);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_screenMode(const FdsDriver *fds, uint16_t mode, uint16_t *oldMode) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFF68), __builtin_bswap16(0xFF9E),
      __builtin_bswap16(0xFF42), __builtin_bswap16(0xFF9E)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldMode;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(
             fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
             sizeof(command1), (uint8_t *)&response,
             sizeof(response) -
                 (fds->deviceType == FDS_GOLDELOX ? sizeof(*oldMode) : 0)) != 0;

  if (fds->deviceType != FDS_GOLDELOX) {
    if (oldMode != NULL) *oldMode = __builtin_bswap16(response.oldMode);
  }

  return stus && (response.ack == QDS_ACK);
}

bool gfx_transparency(const FdsDriver *fds, uint16_t mode, uint16_t *oldMode) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFA0),
      __builtin_bswap16(0xFF44), __builtin_bswap16(0xFFA0)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t oldMode;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_transparency unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (oldMode != NULL) *oldMode = __builtin_bswap16(response.oldMode);

  return stus && (response.ack == QDS_ACK);
}

bool gfx_transparentColour(const FdsDriver *fds, uint16_t colour,
                           uint16_t *oldColor) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFA1),
      __builtin_bswap16(0xFF45), __builtin_bswap16(0xFFA1)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t colour;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .colour = __builtin_bswap16(colour)};

  struct {
    uint8_t ack;
    uint16_t oldColor;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_transparentColour unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (oldColor != NULL) *oldColor = __builtin_bswap16(response.oldColor);

  return stus && (response.ack == QDS_ACK);
}

bool gfx_set(const FdsDriver *fds, uint16_t function, uint16_t value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFD8), __builtin_bswap16(0xFFCE),
      __builtin_bswap16(0xFF83), __builtin_bswap16(0xFFCE)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t function;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .function = __builtin_bswap16(function),
              .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_get(const FdsDriver *fds, uint16_t mode, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFA6),
      __builtin_bswap16(0xFF4A), __builtin_bswap16(0xFFA6)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_get unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool gfx_scale(const FdsDriver *fds, uint16_t handle, uint16_t param) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEAA)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t param;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .param = __builtin_bswap16(param)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_scale unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_pannel2(const FdsDriver *fds, uint16_t options, uint16_t x, uint16_t y,
                 uint16_t width, uint16_t height, uint16_t width1,
                 uint16_t width2, uint16_t maincolour, uint16_t shadowcolour,
                 uint16_t fcolour) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEC7)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t options;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t width1;
    uint16_t width2;
    uint16_t maincolour;
    uint16_t shadowcolour;
    uint16_t fcolour;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .options = __builtin_bswap16(options),
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .width = __builtin_bswap16(width),
              .height = __builtin_bswap16(height),
              .width1 = __builtin_bswap16(width1),
              .width2 = __builtin_bswap16(width2),
              .maincolour = __builtin_bswap16(maincolour),
              .shadowcolour = __builtin_bswap16(shadowcolour),
              .fcolour = __builtin_bswap16(fcolour)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_pannel2 unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_button4(const FdsDriver *fds, uint16_t value, uint16_t handle,
                 uint16_t params) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEAC)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
    uint16_t handle;
    uint16_t params;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .value = __builtin_bswap16(value),
              .handle = __builtin_bswap16(handle),
              .params = __builtin_bswap16(params)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_button4 unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_switch(const FdsDriver *fds, uint16_t value, uint16_t handle,
                uint16_t params) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEAD)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
    uint16_t handle;
    uint16_t params;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .value = __builtin_bswap16(value),
              .handle = __builtin_bswap16(handle),
              .params = __builtin_bswap16(params)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_switch unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_slider5(const FdsDriver *fds, uint16_t value, uint16_t handle,
                 uint16_t params) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEAF)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
    uint16_t handle;
    uint16_t params;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .value = __builtin_bswap16(value),
              .handle = __builtin_bswap16(handle),
              .params = __builtin_bswap16(params)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_slider5 unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_dial(const FdsDriver *fds, uint16_t value, uint16_t handle,
              uint16_t params) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEC5)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
    uint16_t handle;
    uint16_t params;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .value = __builtin_bswap16(value),
              .handle = __builtin_bswap16(handle),
              .params = __builtin_bswap16(params)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_dial unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_led(const FdsDriver *fds, uint16_t value, uint16_t handle,
             uint16_t params) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEAB)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
    uint16_t handle;
    uint16_t params;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .value = __builtin_bswap16(value),
              .handle = __builtin_bswap16(handle),
              .params = __builtin_bswap16(params)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_led unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_gauge(const FdsDriver *fds, uint16_t value, uint16_t handle,
               uint16_t params) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEC4)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
    uint16_t handle;
    uint16_t params;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .value = __builtin_bswap16(value),
              .handle = __builtin_bswap16(handle),
              .params = __builtin_bswap16(params)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_gauge unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_angularMeter(const FdsDriver *fds, uint16_t value, uint16_t handle,
                      uint16_t params) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEC8)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
    uint16_t handle;
    uint16_t params;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .value = __builtin_bswap16(value),
              .handle = __builtin_bswap16(handle),
              .params = __builtin_bswap16(params)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_angularMeter unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_ledGigit(const FdsDriver *fds, uint16_t x, uint16_t y,
                  uint16_t digitSize, uint16_t onColour, uint16_t offColour,
                  uint16_t value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEC1)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t digitSize;
    uint16_t onColour;
    uint16_t offColour;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .digitSize = __builtin_bswap16(digitSize),
              .onColour = __builtin_bswap16(onColour),
              .offColour = __builtin_bswap16(offColour),
              .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_ledGigit unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool gfx_ledDigits(const FdsDriver *fds, uint16_t value, uint16_t handle,
                   uint16_t params) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEC2)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
    uint16_t handle;
    uint16_t params;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .value = __builtin_bswap16(value),
              .handle = __builtin_bswap16(handle),
              .params = __builtin_bswap16(params)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function gfx_ledDigits unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool media_init(const FdsDriver *fds, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFB1), __builtin_bswap16(0xFF89),
      __builtin_bswap16(0xFF25), __builtin_bswap16(0xFF89)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool media_setAdd(const FdsDriver *fds, uint16_t hiAddr, uint16_t loAddr) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFB9), __builtin_bswap16(0xFF93),
      __builtin_bswap16(0xFF2F), __builtin_bswap16(0xFF93)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t hiAddr;
    uint16_t loAddr;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .hiAddr = __builtin_bswap16(hiAddr),
              .loAddr = __builtin_bswap16(loAddr)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool media_setSector(const FdsDriver *fds, uint16_t hiAddr, uint16_t loAddr) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFB8), __builtin_bswap16(0xFF92),
      __builtin_bswap16(0xFF2E), __builtin_bswap16(0xFF92)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t hiAddr;
    uint16_t loAddr;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .hiAddr = __builtin_bswap16(hiAddr),
              .loAddr = __builtin_bswap16(loAddr)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool media_readByte(const FdsDriver *fds, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFB7), __builtin_bswap16(0xFF8F),
      __builtin_bswap16(0xFF2B), __builtin_bswap16(0xFF8F)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool media_readWord(const FdsDriver *fds, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFB6), __builtin_bswap16(0xFF8E),
      __builtin_bswap16(0xFF2A), __builtin_bswap16(0xFF8E)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool media_writeByte(const FdsDriver *fds, uint16_t s, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFB5), __builtin_bswap16(0xFF8D),
      __builtin_bswap16(0xFF29), __builtin_bswap16(0xFF8D)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t s;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .s = __builtin_bswap16(s)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool media_writeWord(const FdsDriver *fds, uint16_t value, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFB4), __builtin_bswap16(0xFF8C),
      __builtin_bswap16(0xFF28), __builtin_bswap16(0xFF8C)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool media_flush(const FdsDriver *fds, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFB2), __builtin_bswap16(0xFF8A),
      __builtin_bswap16(0xFF26), __builtin_bswap16(0xFF8A)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool media_image(const FdsDriver *fds, uint16_t x, uint16_t y) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFB3), __builtin_bswap16(0xFF8B),
      __builtin_bswap16(0xFF27), __builtin_bswap16(0xFF8B)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .x = __builtin_bswap16(x),
                                            .y = __builtin_bswap16(y)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool media_video(const FdsDriver *fds, uint16_t x, uint16_t y) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFBB), __builtin_bswap16(0xFF95),
      __builtin_bswap16(0xFF31), __builtin_bswap16(0xFF95)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .x = __builtin_bswap16(x),
                                            .y = __builtin_bswap16(y)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool media_videoFrame(const FdsDriver *fds, uint16_t x, uint16_t y,
                      int16_t frameNumber) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFBA), __builtin_bswap16(0xFF94),
      __builtin_bswap16(0xFF30), __builtin_bswap16(0xFF94)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    int16_t frameNumber;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .frameNumber = __builtin_bswap16(frameNumber)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_peekB(const FdsDriver *fds, uint16_t eveReg, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFF6), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t eveReg;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .eveReg = __builtin_bswap16(eveReg)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_peekB  unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool misc_pokeB(const FdsDriver *fds, uint16_t eveReg, uint16_t value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFF4), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t eveReg;
    uint16_t value;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .eveReg = __builtin_bswap16(eveReg),
                                            .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_pokeB  unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_peekW(const FdsDriver *fds, uint16_t eveReg, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFF5), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t eveReg;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .eveReg = __builtin_bswap16(eveReg)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_peekW  unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool misc_pokeW(const FdsDriver *fds, uint16_t eveReg, uint16_t value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFF3), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t eveReg;
    uint16_t value;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .eveReg = __builtin_bswap16(eveReg),
                                            .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_pokeW unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_peekM(const FdsDriver *fds, uint16_t address, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0027),
      __builtin_bswap16(0x0027), __builtin_bswap16(0x0027)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t address;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .address = __builtin_bswap16(address)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_peekM  unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool misc_pokeM(const FdsDriver *fds, uint16_t address, uint16_t value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0028),
      __builtin_bswap16(0x0028), __builtin_bswap16(0x0028)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t address;
    uint16_t value;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .address = __builtin_bswap16(address),
              .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_pokeM unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_joystick(const FdsDriver *fds, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFD9), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_joystick unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool misc_beep(const FdsDriver *fds, uint16_t note, uint16_t duration_ms) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0xFFDA), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t note;
    uint16_t duration_ms;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .note = __builtin_bswap16(note),
              .duration_ms = __builtin_bswap16(duration_ms)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_beep unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_setbaudWait(const FdsDriver *fds, int16_t index) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0x000B), __builtin_bswap16(0x0026),
      __builtin_bswap16(0x0026), __builtin_bswap16(0x0026)};

  bool stus = false;
  struct {
    uint16_t cmd;
    int16_t index;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .index = __builtin_bswap16(index)};

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), NULL, 0) == 0;
  return stus;
}

bool sys_sleep(const FdsDriver *fds, uint16_t duration_s, uint16_t *duration) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF3B),
      __builtin_bswap16(0xFE6D), __builtin_bswap16(0xFF3B)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t duration_s;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .duration_s = __builtin_bswap16(duration_s)};

  struct {
    uint8_t ack;
    uint16_t duration;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function sys_sleep unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (duration != NULL) *duration = __builtin_bswap16(response.duration);

  return stus && (response.ack == QDS_ACK);
}

bool sys_memFree(const FdsDriver *fds, uint16_t handle, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF24),
      __builtin_bswap16(0xFE5F), __builtin_bswap16(0xFF24)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function sys_memFree unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool sys_memHeap(const FdsDriver *fds, uint16_t *avail) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF23),
      __builtin_bswap16(0xFE5E), __builtin_bswap16(0xFF23)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t avail;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function sys_memHeap unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (avail != NULL) *avail = __builtin_bswap16(response.avail);

  return stus && (response.ack == QDS_ACK);
}

bool sys_getModel(const FdsDriver *fds, uint16_t *n, char *str) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0x0007), __builtin_bswap16(0x001A),
      __builtin_bswap16(0x001A), __builtin_bswap16(0x001A)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)str, 3) == 3;
  if ((!stus) || (str[0] != QDS_ACK)) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
    return false;
  }
  const size_t dynSize = MIN(getResponseAsUint16((uint8_t *)str), (*n) - 1);
  (*n) = dynSize;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, NULL, 0, (uint8_t *)str,
                           dynSize) == dynSize;
  if (!stus) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
  } else {
    str[dynSize] = 0;
  }

  return stus;
}

bool sys_getVersion(const FdsDriver *fds, uint16_t *version) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0x0008), __builtin_bswap16(0x001B),
      __builtin_bswap16(0x001B), __builtin_bswap16(0x001B)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t version;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (version != NULL) *version = __builtin_bswap16(response.version);

  return stus && (response.ack == QDS_ACK);
}

bool sys_getPmmC(const FdsDriver *fds, uint16_t *version) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0x0009), __builtin_bswap16(0x001C),
      __builtin_bswap16(0x001C), __builtin_bswap16(0x001C)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t version;
  } __attribute__((__packed__)) response;

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (version != NULL) *version = __builtin_bswap16(response.version);

  return stus && (response.ack == QDS_ACK);
}

bool misc_screenSaverTimeout(const FdsDriver *fds, uint16_t timout_ms) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0x000C), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t timout_ms;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .timout_ms = __builtin_bswap16(timout_ms)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(
      command1.cmd != CMD_NOT_IMPL,
      "function misc_screenSaverTimeout unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_screenSaverSpeed(const FdsDriver *fds, uint16_t speed) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0x000D), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t speed;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .speed = __builtin_bswap16(speed)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_screenSaverSpeed unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool misc_screenSaverMode(const FdsDriver *fds, uint16_t mode) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0x000E), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function misc_screenSaverMode unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool touch_detectRegion(const FdsDriver *fds, uint16_t tlx, uint16_t tly,
                        uint16_t brx, uint16_t bry) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF39),
      __builtin_bswap16(0xFE6A), __builtin_bswap16(0xFF39)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t tlx;
    uint16_t tly;
    uint16_t brx;
    uint16_t bry;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .tlx = __builtin_bswap16(tlx),
                                            .tly = __builtin_bswap16(tly),
                                            .brx = __builtin_bswap16(brx),
                                            .bry = __builtin_bswap16(bry)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function touch_detectRegion unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool touch_set(const FdsDriver *fds, uint16_t mode) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF38),
      __builtin_bswap16(0xFE69), __builtin_bswap16(0xFF38)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function touch_set unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool touch_get(const FdsDriver *fds, uint16_t mode, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF37),
      __builtin_bswap16(0xFE68), __builtin_bswap16(0xFF37)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .mode = __builtin_bswap16(mode)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function touch_get unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool file_error(const FdsDriver *fds, uint16_t *errno) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF1F),
      __builtin_bswap16(0xFE58), __builtin_bswap16(0xFF1F)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t errno;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_error unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (errno != NULL) *errno = __builtin_bswap16(response.errno);

  return stus && (response.ack == QDS_ACK);
}

bool file_count(const FdsDriver *fds, const char *filename, uint16_t *count) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0001),
      __builtin_bswap16(0x0001), __builtin_bswap16(0x0001)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t count;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_count unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)filename,
                           strlen(filename) + 1, (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (count != NULL) *count = __builtin_bswap16(response.count);

  return stus && (response.ack == QDS_ACK);
}

bool file_dir(const FdsDriver *fds, const char *filename, uint16_t *count) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0002),
      __builtin_bswap16(0x0002), __builtin_bswap16(0x0002)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t count;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_dir unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)filename,
                           strlen(filename) + 1, (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (count != NULL) *count = __builtin_bswap16(response.count);

  return stus && (response.ack == QDS_ACK);
}

bool file_findFirst(const FdsDriver *fds, const char *filename,
                    uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0006),
      __builtin_bswap16(0x0006), __builtin_bswap16(0x0006)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_findFirst unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)filename,
                           strlen(filename) + 1, (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_findFirstRet(const FdsDriver *fds, const char *filename,
                       uint16_t *length, char *str) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0024),
      __builtin_bswap16(0x0024), __builtin_bswap16(0x0024)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)filename,
                           strlen(filename) + 1, (uint8_t *)str, 3) == 3;
  if ((!stus) || (str[0] != QDS_ACK)) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
    return false;
  }
  const size_t dynSize =
      MIN(getResponseAsUint16((uint8_t *)str), (*length) - 1);
  (*length) = dynSize;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, NULL, 0, (uint8_t *)str,
                           dynSize) == dynSize;
  if (!stus) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
  } else {
    str[dynSize] = 0;
  }

  return stus;
}

bool file_findNext(const FdsDriver *fds, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF1B),
      __builtin_bswap16(0xFE54), __builtin_bswap16(0xFF1B)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_findNext unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_findNextRet(const FdsDriver *fds, uint16_t *length, char *str) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0025),
      __builtin_bswap16(0x0025), __builtin_bswap16(0x0025)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)str, 3) == 3;
  if ((!stus) || (str[0] != QDS_ACK)) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
    return false;
  }
  const size_t dynSize =
      MIN(getResponseAsUint16((uint8_t *)str), (*length) - 1);
  (*length) = dynSize;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, NULL, 0, (uint8_t *)str,
                           dynSize) == dynSize;
  if (!stus) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
  } else {
    str[dynSize] = 0;
  }

  return stus;
}

bool file_exists(const FdsDriver *fds, const char *filename, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0005),
      __builtin_bswap16(0x0005), __builtin_bswap16(0x0005)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_exists unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)filename,
                           strlen(filename) + 1, (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_open(const FdsDriver *fds, const char *filename, char mode,
               uint16_t *handle) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x000A),
      __builtin_bswap16(0x000A), __builtin_bswap16(0x000A)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
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
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)filename,
                           strlen(filename) + 1, NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command2,
                           sizeof(command2), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (handle != NULL) *handle = __builtin_bswap16(response.handle);

  return stus && (response.ack == QDS_ACK);
}

bool file_close(const FdsDriver *fds, uint16_t handle, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF18),
      __builtin_bswap16(0xFE51), __builtin_bswap16(0xFF18)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_close unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_read(const FdsDriver *fds, uint16_t size, uint16_t handle,
               uint16_t *n, char *str) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x000C),
      __builtin_bswap16(0x000C), __builtin_bswap16(0x000C)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t size;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .size = __builtin_bswap16(size),
              .handle = __builtin_bswap16(handle)};

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)str, 3) == 3;
  if ((!stus) || (str[0] != QDS_ACK)) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
    return false;
  }
  const size_t dynSize = MIN(getResponseAsUint16((uint8_t *)str), (*n) - 1);
  (*n) = dynSize;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, NULL, 0, (uint8_t *)str,
                           dynSize) == dynSize;
  if (!stus) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
  } else {
    str[dynSize] = 0;
  }

  return stus;
}

bool file_seek(const FdsDriver *fds, uint16_t handle, uint16_t hiWord,
               uint16_t loWord, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF16),
      __builtin_bswap16(0xFE4F), __builtin_bswap16(0xFF16)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t hiWord;
    uint16_t loWord;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .handle = __builtin_bswap16(handle),
              .hiWord = __builtin_bswap16(hiWord),
              .loWord = __builtin_bswap16(loWord)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_seek unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_index(const FdsDriver *fds, uint16_t handle, uint16_t hiWord,
                uint16_t loWord, uint16_t recordNum, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF15),
      __builtin_bswap16(0xFE4E), __builtin_bswap16(0xFF15)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t hiWord;
    uint16_t loWord;
    uint16_t recordNum;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
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
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_tell(const FdsDriver *fds, uint16_t handle, uint16_t *status,
               uint16_t *hiWord, uint16_t *loWord) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x000F),
      __builtin_bswap16(0x000F), __builtin_bswap16(0x000F)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
    uint16_t hiWord;
    uint16_t loWord;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_tell unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);
  ;
  if (hiWord != NULL) *hiWord = __builtin_bswap16(response.hiWord);
  ;
  if (loWord != NULL) *loWord = __builtin_bswap16(response.loWord);

  return stus && (response.ack == QDS_ACK);
}

bool file_write(const FdsDriver *fds, uint16_t size, const uint8_t *source,
                uint16_t handle, uint16_t *count) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0010),
      __builtin_bswap16(0x0010), __builtin_bswap16(0x0010)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t size;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .size = __builtin_bswap16(size)};

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
  fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                    sizeof(command1), NULL, 0);
  fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, source, size, NULL, 0);
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command2,
                           sizeof(command2), (uint8_t *)&response,
                           sizeof(response));

  if (count != NULL) *count = __builtin_bswap16(response.count);

  return stus && (response.ack == QDS_ACK);
}

bool file_size(const FdsDriver *fds, uint16_t handle, uint16_t *status,
               uint16_t *hiWord, uint16_t *loWord) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x000E),
      __builtin_bswap16(0x000E), __builtin_bswap16(0x000E)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
    uint16_t hiWord;
    uint16_t loWord;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_size unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);
  ;
  if (hiWord != NULL) *hiWord = __builtin_bswap16(response.hiWord);
  ;
  if (loWord != NULL) *loWord = __builtin_bswap16(response.loWord);

  return stus && (response.ack == QDS_ACK);
}

bool file_image(const FdsDriver *fds, uint16_t x, uint16_t y, uint16_t handle,
                uint16_t *errno) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF11),
      __builtin_bswap16(0xFE4A), __builtin_bswap16(0xFF11)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t errno;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_image unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (errno != NULL) *errno = __builtin_bswap16(response.errno);

  return stus && (response.ack == QDS_ACK);
}

bool file_screenCapture(const FdsDriver *fds, uint16_t x, uint16_t y,
                        uint16_t width, uint16_t height, uint16_t handle,
                        uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF10),
      __builtin_bswap16(0xFE49), __builtin_bswap16(0xFF10)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
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
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_putC(const FdsDriver *fds, uint16_t car, uint16_t handle,
               uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x001F),
      __builtin_bswap16(0x001F), __builtin_bswap16(0x001F)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t car;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .car = __builtin_bswap16(car),
              .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_putC unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_getC(const FdsDriver *fds, uint16_t handle, uint16_t *car) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF0E),
      __builtin_bswap16(0xFE47), __builtin_bswap16(0xFF0E)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t car;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_getC unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (car != NULL) *car = __builtin_bswap16(response.car);

  return stus && (response.ack == QDS_ACK);
}

bool file_putW(const FdsDriver *fds, uint16_t word, uint16_t handle,
               uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF0D),
      __builtin_bswap16(0xFE46), __builtin_bswap16(0xFF0D)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t word;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .word = __builtin_bswap16(word),
              .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_putW unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_getW(const FdsDriver *fds, uint16_t handle, uint16_t *word) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF0C),
      __builtin_bswap16(0xFE45), __builtin_bswap16(0xFF0C)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t word;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_getW unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (word != NULL) *word = __builtin_bswap16(response.word);

  return stus && (response.ack == QDS_ACK);
}

bool file_putS(const FdsDriver *fds, const char *cstr, uint16_t *count) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0020),
      __builtin_bswap16(0x0020), __builtin_bswap16(0x0020)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t count;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_putS unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)cstr,
                           strlen(cstr) + 1, (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (count != NULL) *count = __builtin_bswap16(response.count);

  return stus && (response.ack == QDS_ACK);
}

bool file_getS(const FdsDriver *fds, uint16_t size, uint16_t handle,
               uint16_t *n, char *str) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0007),
      __builtin_bswap16(0x0007), __builtin_bswap16(0x0007)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t size;
    uint16_t handle;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .size = __builtin_bswap16(size),
              .handle = __builtin_bswap16(handle)};

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)str, 3) == 3;
  if ((!stus) || (str[0] != QDS_ACK)) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
    return false;
  }
  const size_t dynSize = MIN(getResponseAsUint16((uint8_t *)str), (*n) - 1);
  (*n) = dynSize;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, NULL, 0, (uint8_t *)str,
                           dynSize) == dynSize;
  if (!stus) {
    DebugTrace("%s error ", __FUNCTION__);
    str[0] = 0;
  } else {
    str[dynSize] = 0;
  }

  return stus;
}

bool file_erase(const FdsDriver *fds, const char *filename, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0003),
      __builtin_bswap16(0x0003), __builtin_bswap16(0x0003)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_erase unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)filename,
                           strlen(filename) + 1, (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_rewind(const FdsDriver *fds, uint16_t handle, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF08),
      __builtin_bswap16(0xFE41), __builtin_bswap16(0xFF08)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_rewind unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_loadFunction(const FdsDriver *fds, const char *filename,
                       uint16_t *pointer) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0008),
      __builtin_bswap16(0x0008), __builtin_bswap16(0x0008)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t pointer;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_loadFunction unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)filename,
                           strlen(filename) + 1, (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (pointer != NULL) *pointer = __builtin_bswap16(response.pointer);

  return stus && (response.ack == QDS_ACK);
}

bool file_callFunction(const FdsDriver *fds, uint16_t handle, uint16_t n,
                       const uint16_t *args, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0019),
      __builtin_bswap16(0x0019), __builtin_bswap16(0x0019)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    uint16_t n;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .n = __builtin_bswap16(n)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_callFunction unimplemented for this screen");
  fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                    sizeof(command1), NULL, 0);
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)args,
                           n * sizeof(*args), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool file_run(const FdsDriver *fds, const char *filename, uint16_t n,
              const uint16_t *args, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x000D),
      __builtin_bswap16(0x000D), __builtin_bswap16(0x000D)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
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
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)filename,
                           strlen(filename) + 1, NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command2,
                           sizeof(command2), NULL, 0);
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)args,
                           n * sizeof(*args), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool file_exec(const FdsDriver *fds, const char *filename, uint16_t n,
               const uint16_t *args, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0004),
      __builtin_bswap16(0x0004), __builtin_bswap16(0x0004)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
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
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)filename,
                           strlen(filename) + 1, NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command2,
                           sizeof(command2), NULL, 0);
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)args,
                           n * sizeof(*args), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool file_loadImageControl(const FdsDriver *fds, const char *filename1,
                           const char *filename2, uint16_t mode,
                           uint16_t *handle) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0009),
      __builtin_bswap16(0x0009), __builtin_bswap16(0x0009)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
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
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)filename1,
                           strlen(filename1) + 1, NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)filename2,
                           strlen(filename2) + 1, NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command2,
                           sizeof(command2), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (handle != NULL) *handle = __builtin_bswap16(response.handle);

  return stus && (response.ack == QDS_ACK);
}

bool file_mount(const FdsDriver *fds, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF03),
      __builtin_bswap16(0xFE3C), __builtin_bswap16(0xFF03)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_mount unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_unmount(const FdsDriver *fds) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF02),
      __builtin_bswap16(0xFE3B), __builtin_bswap16(0xFF02)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_unmount unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool file_playWAV(const FdsDriver *fds, const char *filename,
                  uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x000B),
      __builtin_bswap16(0x000B), __builtin_bswap16(0x000B)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_playWAV unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)filename,
                           strlen(filename) + 1, (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool file_writeString(const FdsDriver *fds, uint16_t handle, const char *cstr,
                      uint16_t *pointer) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0x0021),
      __builtin_bswap16(0x0021), __builtin_bswap16(0x0021)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .handle = __builtin_bswap16(handle)};

  struct {
    uint8_t ack;
    uint16_t pointer;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function file_writeString unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), NULL, 0) != 0;
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)cstr,
                           strlen(cstr) + 1, (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (pointer != NULL) *pointer = __builtin_bswap16(response.pointer);

  return stus && (response.ack == QDS_ACK);
}

bool snd_volume(const FdsDriver *fds, uint16_t level) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF00),
      __builtin_bswap16(0xFE35), __builtin_bswap16(0xFF00)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t level;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .level = __builtin_bswap16(level)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_volume unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool snd_pitch(const FdsDriver *fds, uint16_t rate, uint16_t *oldRate) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEFF),
      __builtin_bswap16(0xFE34), __builtin_bswap16(0xFEFF)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t rate;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .rate = __builtin_bswap16(rate)};

  struct {
    uint8_t ack;
    uint16_t oldRate;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_pitch unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (oldRate != NULL) *oldRate = __builtin_bswap16(response.oldRate);

  return stus && (response.ack == QDS_ACK);
}

bool snd_bufSize(const FdsDriver *fds, uint16_t bufferSize) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEFE),
      __builtin_bswap16(0xFE33), __builtin_bswap16(0xFEFE)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t bufferSize;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .bufferSize = __builtin_bswap16(bufferSize)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_bufSize unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool snd_stop(const FdsDriver *fds) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEFD),
      __builtin_bswap16(0xFE32), __builtin_bswap16(0xFEFD)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_stop unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool snd_pause(const FdsDriver *fds) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEFC),
      __builtin_bswap16(0xFE31), __builtin_bswap16(0xFEFC)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_pause unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool snd_continue(const FdsDriver *fds) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEFB),
      __builtin_bswap16(0xFE30), __builtin_bswap16(0xFEFB)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_continue unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool snd_playing(const FdsDriver *fds, uint16_t *togo) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFEFA),
      __builtin_bswap16(0xFE2F), __builtin_bswap16(0xFEFA)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t togo;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function snd_playing unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (togo != NULL) *togo = __builtin_bswap16(response.togo);

  return stus && (response.ack == QDS_ACK);
}

bool img_setPosition(const FdsDriver *fds, uint16_t handle, int16_t index,
                     uint16_t xpos, uint16_t ypos, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF4E),
      __builtin_bswap16(0xFE8A), __builtin_bswap16(0xFF4E)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
    uint16_t xpos;
    uint16_t ypos;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
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
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_enable(const FdsDriver *fds, uint16_t handle, int16_t index,
                uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF4D),
      __builtin_bswap16(0xFE89), __builtin_bswap16(0xFF4D)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_enable  unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_disable(const FdsDriver *fds, uint16_t handle, int16_t index,
                 uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF4C),
      __builtin_bswap16(0xFE88), __builtin_bswap16(0xFF4C)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_disable unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_darken(const FdsDriver *fds, uint16_t handle, int16_t index,
                uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF4B),
      __builtin_bswap16(0xFE87), __builtin_bswap16(0xFF4B)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_darken unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_lighten(const FdsDriver *fds, uint16_t handle, int16_t index,
                 uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF4A),
      __builtin_bswap16(0xFE86), __builtin_bswap16(0xFF4A)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_lighten unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_setWord(const FdsDriver *fds, uint16_t handle, int16_t index,
                 uint16_t offset, uint16_t value, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF49),
      __builtin_bswap16(0xFE85), __builtin_bswap16(0xFF49)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
    uint16_t offset;
    uint16_t value;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
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
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_getWord(const FdsDriver *fds, uint16_t handle, int16_t index,
                 uint16_t offset, uint16_t *value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF48),
      __builtin_bswap16(0xFE84), __builtin_bswap16(0xFF48)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
    uint16_t offset;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .handle = __builtin_bswap16(handle),
              .index = __builtin_bswap16(index),
              .offset = __builtin_bswap16(offset)};

  struct {
    uint8_t ack;
    uint16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_getWord unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool img_show(const FdsDriver *fds, uint16_t handle, int16_t index,
              uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF47),
      __builtin_bswap16(0xFE83), __builtin_bswap16(0xFF47)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_show unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_setAttributes(const FdsDriver *fds, uint16_t handle, int16_t index,
                       uint16_t value, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF46),
      __builtin_bswap16(0xFE82), __builtin_bswap16(0xFF46)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
    uint16_t value;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .index = __builtin_bswap16(index),
                                            .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_setAttributes unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_clearAttributes(const FdsDriver *fds, uint16_t handle, int16_t index,
                         uint16_t value, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF45),
      __builtin_bswap16(0xFE81), __builtin_bswap16(0xFF45)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
    uint16_t value;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .index = __builtin_bswap16(index),
                                            .value = __builtin_bswap16(value)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_clearAttributes unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool img_touched(const FdsDriver *fds, uint16_t handle, int16_t index,
                 int16_t *value) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFF44),
      __builtin_bswap16(0xFE80), __builtin_bswap16(0xFF44)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t handle;
    int16_t index;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .handle = __builtin_bswap16(handle),
                                            .index = __builtin_bswap16(index)};

  struct {
    uint8_t ack;
    int16_t value;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_touched unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (value != NULL) *value = __builtin_bswap16(response.value);

  return stus && (response.ack == QDS_ACK);
}

bool img_blitComtoDisplay(const FdsDriver *fds, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height,
                          const uint16_t *data) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(0x000A), __builtin_bswap16(0x0023),
      __builtin_bswap16(0x0023), __builtin_bswap16(0x0023)};
  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType],
              .x = __builtin_bswap16(x),
              .y = __builtin_bswap16(y),
              .width = __builtin_bswap16(width),
              .height = __builtin_bswap16(height)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function img_blitComtoDisplay unimplemented for this screen");
  fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                    sizeof(command1), NULL, 0);

  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)data,
                           sizeof(*data) * width * height, (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus;
}

bool bus_in(const FdsDriver *fds, uint16_t *busState) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD3),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD3)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t busState;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function bus_in unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (busState != NULL) *busState = __builtin_bswap16(response.busState);

  return stus && (response.ack == QDS_ACK);
}

bool bus_out(const FdsDriver *fds, uint16_t busState) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD2),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD2)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t busState;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .busState = __builtin_bswap16(busState)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function bus_out unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool bus_read(const FdsDriver *fds, uint16_t *busState) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFCF),
      __builtin_bswap16(0xFF86), __builtin_bswap16(0xFFCF)};

  bool stus = false;
  struct {
    uint16_t cmd;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType],
  };

  struct {
    uint8_t ack;
    uint16_t busState;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function bus_read unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (busState != NULL) *busState = __builtin_bswap16(response.busState);

  return stus && (response.ack == QDS_ACK);
}

bool bus_set(const FdsDriver *fds, uint16_t dirMask) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD1),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD1)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t dirMask;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .dirMask = __builtin_bswap16(dirMask)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function bus_set unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool bus_write(const FdsDriver *fds, uint16_t bitfield) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD0),
      __builtin_bswap16(0xFF87), __builtin_bswap16(0xFFD0)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t bitfield;
  } __attribute__((__packed__)) command1 = {
      .cmd = cmds[fds->deviceType], .bitfield = __builtin_bswap16(bitfield)};

  struct {
    uint8_t ack;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function bus_write unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  return stus && (response.ack == QDS_ACK);
}

bool pin_hi(const FdsDriver *fds, uint16_t pin, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD6),
      __builtin_bswap16(0xFF8F), __builtin_bswap16(0xFFD6)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t pin;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .pin = __builtin_bswap16(pin)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function pin_hi unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool pin_lo(const FdsDriver *fds, uint16_t pin, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD5),
      __builtin_bswap16(0xFF8E), __builtin_bswap16(0xFFD5)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t pin;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .pin = __builtin_bswap16(pin)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function pin_lo unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool pin_read(const FdsDriver *fds, uint16_t pin, uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD4),
      __builtin_bswap16(0xFF8C), __builtin_bswap16(0xFFD4)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t pin;
  } __attribute__((__packed__))
  command1 = {.cmd = cmds[fds->deviceType], .pin = __builtin_bswap16(pin)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function pin_read unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool pin_set_picaso(const FdsDriver *fds, uint16_t mode, uint16_t pin,
                    uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD7),
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(0xFFD7)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
    uint16_t pin;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .mode = __builtin_bswap16(mode),
                                            .pin = __builtin_bswap16(pin)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function pin_set_picaso unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}

bool pin_set_diablo(const FdsDriver *fds, uint16_t mode, uint16_t pin,
                    uint16_t *status) {
  RET_UNLESS_INIT_BOOL(fds);
  RET_UNLESS_4DSYS_BOOL(fds);
  static const uint16_t cmds[FDS_AUTO] = {
      __builtin_bswap16(CMD_NOT_IMPL), __builtin_bswap16(CMD_NOT_IMPL),
      __builtin_bswap16(0xFF90), __builtin_bswap16(CMD_NOT_IMPL)};

  bool stus = false;
  struct {
    uint16_t cmd;
    uint16_t mode;
    uint16_t pin;
  } __attribute__((__packed__)) command1 = {.cmd = cmds[fds->deviceType],
                                            .mode = __builtin_bswap16(mode),
                                            .pin = __builtin_bswap16(pin)};

  struct {
    uint8_t ack;
    uint16_t status;
  } __attribute__((__packed__)) response;
  osalDbgAssert(command1.cmd != CMD_NOT_IMPL,
                "function pin_set_diablo unimplemented for this screen");
  stus = fdsTransmitBuffer(fds, __FUNCTION__, __LINE__, (uint8_t *)&command1,
                           sizeof(command1), (uint8_t *)&response,
                           sizeof(response)) != 0;

  if (status != NULL) *status = __builtin_bswap16(response.status);

  return stus && (response.ack == QDS_ACK);
}
