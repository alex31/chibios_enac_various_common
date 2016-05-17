#include <ch.h>
#include <hal.h>
#include "stdutil.h"

#if defined I2C_USE_MS45XX && I2C_USE_MS45XX != 0
#include "i2cPeriphMS45XX.h"

/** MS45xx pressure range in psi.
 * The sensor is available in 1, 2, 5, 15, 30, 50, 100, 150 psi ranges.
 * 1 psi max range should be ~100m/s max airspeed.
 */
#ifndef MS45XX_PRESSURE_RANGE
#define MS45XX_PRESSURE_RANGE 1.0f
#endif

/** MS45xx output Type.
 * 0 = Output Type A with 10% to 90%
 * 1 = Output Type B with 5% to 95%
 */
#ifndef MS45XX_OUTPUT_TYPE
#define MS45XX_OUTPUT_TYPE 0
#endif

/** Conversion factor from psi to Pa */
#define PSI_TO_PA 6894.75729f

#if MS45XX_OUTPUT_TYPE == 0
/* Offset and scaling for OUTPUT TYPE A:
 * p_raw = (0.8*16383)/ (Pmax - Pmin) * (pressure - Pmin) + 0.1*16383
 * For differential sensors Pmax = MS45XX_PRESSURE_RANGE = -Pmin.
 *
 * p_diff = (p_raw - 0.1*16383) * 2*RANGE/(0.8*16383) - RANGE
 * p_diff = p_raw * 2*RANGE/(0.8*16383) - (RANGE + (0.1 * 16383) * 2*RANGE/(0.8*16383)
 * p_diff = p_raw * 2*RANGE/(0.8*16383) - (1.25 * RANGE)
 * p_diff = p_raw * scale - offset
 * then convert to Pascal
 */
#ifndef MS45XX_PRESSURE_SCALE
#define MS45XX_PRESSURE_SCALE (2 * MS45XX_PRESSURE_RANGE / (0.8f * 16383) * PSI_TO_PA)
#endif
#ifndef MS45XX_PRESSURE_OFFSET
#define MS45XX_PRESSURE_OFFSET (1.25f * MS45XX_PRESSURE_RANGE * PSI_TO_PA)
#endif
#else
/* Offset and scaling for OUTPUT TYPE B:
 * p_raw = (0.9*16383)/ (Pmax - Pmin) * (pressure - Pmin) + 0.05*16383
 */
#ifndef MS45XX_PRESSURE_SCALE
#define MS45XX_PRESSURE_SCALE (2 * MS45XX_PRESSURE_RANGE / (0.9f * 16383) * PSI_TO_PA)
#endif
#ifndef MS45XX_PRESSURE_OFFSET
#define MS45XX_PRESSURE_OFFSET ((1.0f + 0.1f / 0.9f) * MS45XX_PRESSURE_RANGE  * PSI_TO_PA)
#endif
#endif

#ifndef MS45XX_AIRSPEED_SCALE
#define MS45XX_AIRSPEED_SCALE 1.6327f
#endif


static bool ms45XX_calc(MS45XXData *ms);


msg_t MS45XX_init (MS45XXData *ms, I2CDriver *i2cd, const uint8_t i2cAddr)
{
  ms->i2cd = i2cd;
  ms->i2cAddr = i2cAddr;
  ms->diff_pressure = 0;
  ms->temperature = 0;
  ms->airspeed = 0.f;
  ms->pressure_scale = MS45XX_PRESSURE_SCALE;
  ms->pressure_offset = MS45XX_PRESSURE_OFFSET;
  ms->airspeed_scale = MS45XX_AIRSPEED_SCALE;

  return MSG_OK;
}




msg_t MS45XX_getVal  (MS45XXData *ms, float *temp, float *pressure)
{
  msg_t status;
  *temp = 0.0f;
  *pressure = 0.0f;
  
  i2cAcquireBus(ms->i2cd);
  I2C_READ (ms->i2cd, ms->i2cAddr, ms->raw);
  i2cReleaseBus(ms->i2cd);
  
  /* if value is zero, it was read to soon and is invalid, back to idle */
  if (ms45XX_calc (ms) != true)
    return MSG_RESET;
  
  *temp = ms->temperature / 10.0f;
  *pressure = ms->diff_pressure ;
  return MSG_OK;
}




static bool ms45XX_calc(MS45XXData *ms)
{
  
  /* 2 MSB of data are status bits, 0 = good data, 2 = already fetched, 3 = fault */
  uint8_t status = (0xC0 & ms->raw[0]) >> 6;

  if (status == 0) {
    /* 14bit raw pressure */
    uint16_t p_raw = 0x3FFF & (((uint16_t)(ms->raw[0]) << 8) |
			       (uint16_t)(ms->raw[1]));
    /* Output is proportional to the difference between Port 1 and Port 2. Output
     * swings positive when Port 1> Port 2. Output is 50% of total counts
     * when Port 1=Port 2.
     * p_diff = p_raw * scale - offset
     */
    ms->diff_pressure = p_raw * ms->pressure_scale - ms->pressure_offset;
     
    /* 11bit raw temperature, 5 LSB bits not used */
    uint16_t temp_raw = 0xFFE0 & (((uint16_t)(ms->raw[2]) << 8) |
				  (uint16_t)(ms->raw[3]));
    temp_raw = temp_raw >> 5;
    /* 0 = -50degC, 20147 = 150degC
     * ms45xx_temperature in 0.1 deg Celcius
     */
    ms->temperature = ((uint32_t)temp_raw * 2000) / 2047 - 500;
    
    // Compute airspeed
    ms->airspeed = sqrtf(MAX(ms->diff_pressure * ms->airspeed_scale, 0));
    return true;
  }
  return false;
}


#endif
