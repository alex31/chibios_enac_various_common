#include "dshot_erps.h"

static const uint8_t gcrNibble[16] = {
  0x19, 0x1B, 0x12, 0x13, 0x1D, 0x15, 0x16, 0x17,
  0x1A, 0x09, 0x0A, 0x0B, 0x1E, 0x0D, 0x0E, 0x0F};

static const uint8_t grcNibbleInv[32] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0x9, 0xa, 0xb, 0xff, 0xd, 0xe, 0xf, 
  0xff, 0xff, 0x2, 0x3, 0xff, 0x5, 0x6, 0x7, 
  0xff, 0x0, 0x8, 0x1, 0xff, 0x4, 0xc, 0xff};

static uint8_t crc4(uint16_t val);
static DshotEPeriodPacket eperiodToPacked(const uint32_t eperiod);
static uint32_t greyEncode(uint32_t num);
static uint32_t greyDecode(const uint32_t num);
static uint32_t grcEncode(uint32_t from);
static uint32_t grcDecode(uint32_t from);
static uint32_t eperiodEncode(const uint16_t eperiod);
static uint32_t eperiodDecode(const uint32_t frame);



static  void setFromEperiod(DshotErps *derpsp, uint32_t eperiod);
static  uint32_t getEperiod(const DshotErps *derpsp);
static  void frameToPacket(DshotErps *derpsp);
static  void packetToFrame(DshotErps *derpsp);
//static  void setCrc4(DshotErps *derpsp);
//static uint32_t packedToEperiod(const DshotEPeriodPacket pk);






const DshotErps* DshotErpsSetFromFrame(DshotErps *derpsp, uint32_t frame)
{
  derpsp->ef = frame;
  frameToPacket(derpsp);
  return derpsp;
}

const DshotErps* DshotErpsSetFromRpm(DshotErps *derpsp, uint32_t rpm)
{
  uint32_t eperiod = ((uint32_t) 60e6f) / rpm;
  setFromEperiod(derpsp, eperiod);
  return derpsp;
}

uint32_t DshotErpsGetRpm(const DshotErps *derpsp)
{
  return ((uint32_t) 60e6f) / getEperiod(derpsp);
}

bool DshotErpsCheckCrc4(const DshotErps *derpsp)
{
   return (crc4(derpsp->ep.rawFrame) == derpsp->ep.crc);
}






static uint8_t crc4(uint16_t val)
{
  val >>= 4;
  return ~(val ^ (val >> 4) ^ (val >> 8)) & 0x0F;
}


static DshotEPeriodPacket eperiodToPacked(const uint32_t eperiod)
{
  DshotEPeriodPacket p;
  const uint32_t exponent = eperiod >> 9;
  
  if (exponent > 128) {
    p = (DshotEPeriodPacket) {.crc = 0, .mantisse = 511, .exponent = 7};
  } else {
    p.exponent = 32U - __builtin_clz(exponent);
    p.mantisse = eperiod >> p.exponent;
  }
  p.crc = crc4(p.rawFrame);
  return p;
}

static uint32_t greyEncode(uint32_t num)
{
  num ^= (num >> 16);
  num ^= (num >>  8);
  num ^= (num >>  4);
  num ^= (num >>  2);
  num ^= (num >>  1);
  return num;
}

static uint32_t greyDecode(const uint32_t num)
{
  return num ^ (num >> 1);
}

static uint32_t grcEncode(uint32_t from)
{
  uint32_t ret = 0;
  for (size_t i = 0; i < 4; i++) {
    //   printf("nibble %u from = 0x%x to = 0x%x\n", 
    //   i, from & 0xf, gcrNibble[from & 0xf]);
    ret |= (gcrNibble[from & 0xf] << (i*5));
    from >>= 4;
  }
  return ret;
}

static uint32_t grcDecode(uint32_t from)
{
  uint32_t ret = 0;
  for (size_t i = 0; i < 4; i++) {
    const uint32_t nibble = grcNibbleInv[from & 0x1f];
    if (nibble == 0xff) {
      ret = 0x0;
      break;
    }
    //   printf("nibble %u from = 0x%x to = 0x%x\n", 
    //   i, from & 0xf, gcrNibble[from & 0xf]);
    ret |= (nibble << (i * 4));
    from >>= 5;
  }
  return ret;
}

static uint32_t eperiodEncode(const uint16_t eperiod)
{
  return greyEncode(grcEncode(eperiod));
}

static uint32_t eperiodDecode(const uint32_t frame)
{
  return grcDecode(greyDecode(frame));
}

static  void setFromEperiod(DshotErps *derpsp, uint32_t eperiod)
{
  derpsp->ep = eperiodToPacked(eperiod);
  packetToFrame(derpsp);
}

static  uint32_t getEperiod(const DshotErps *derpsp)
{
  return derpsp->ep.mantisse << derpsp->ep.exponent;
}

static  void frameToPacket(DshotErps *derpsp)
{
  derpsp->ep.rawFrame = eperiodDecode(derpsp->ef);
}

static  void packetToFrame(DshotErps *derpsp)
{
  derpsp->ef = eperiodEncode(derpsp->ep.rawFrame);
}

/*
static  void setCrc4(DshotErps *derpsp)
{
  derpsp->ep.crc = crc4(derpsp->ep.rawFrame);
}

static uint32_t packedToEperiod(const DshotEPeriodPacket pk)
{
   return pk.mantisse << pk.exponent;
}
*/
