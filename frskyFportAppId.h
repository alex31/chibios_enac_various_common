#pragma once


/**
 * @brief  AppId code associated with an uplink telemetry channel
 * @notes  no real documentation about this one has to check betaflight and opentx source
 *         on their respective github repositories
 */
typedef enum __attribute__ ((__packed__)) {
    // BETAFLIGH IDs :
    FPORT_DATAID_NULL       = 0x0000, 
    FPORT_DATAID_SPEED      = 0x0830, // u32; ground speed (knot / 1000) 
    FPORT_DATAID_VFAS       = 0x0210, // u32; cell (or battery) voltage (volt / 100)
    FPORT_DATAID_VFAS1      = 0x0211,
    FPORT_DATAID_VFAS2      = 0x0212,
    FPORT_DATAID_VFAS3      = 0x0213,
    FPORT_DATAID_VFAS4      = 0x0214,
    FPORT_DATAID_VFAS5      = 0x0215,
    FPORT_DATAID_VFAS6      = 0x0216,
    FPORT_DATAID_VFAS7      = 0x0217,
    FPORT_DATAID_VFAS8      = 0x0218,
    FPORT_DATAID_CURRENT    = 0x0200, // u32; intensity amperage (ampere / 10)
    FPORT_DATAID_CURRENT1   = 0x0201,
    FPORT_DATAID_CURRENT2   = 0x0202,
    FPORT_DATAID_CURRENT3   = 0x0203,
    FPORT_DATAID_CURRENT4   = 0x0204,
    FPORT_DATAID_CURRENT5   = 0x0205,
    FPORT_DATAID_CURRENT6   = 0x0206,
    FPORT_DATAID_CURRENT7   = 0x0207,
    FPORT_DATAID_CURRENT8   = 0x0208,
    FPORT_DATAID_RPM        = 0x0500, // u32; rpm (rpm / 10)
    FPORT_DATAID_RPM1       = 0x0501,
    FPORT_DATAID_RPM2       = 0x0502,
    FPORT_DATAID_RPM3       = 0x0503,
    FPORT_DATAID_RPM4       = 0x0504,
    FPORT_DATAID_RPM5       = 0x0505,
    FPORT_DATAID_RPM6       = 0x0506,
    FPORT_DATAID_RPM7       = 0x0507,
    FPORT_DATAID_RPM8       = 0x0508,
    FPORT_DATAID_ALTITUDE   = 0x0100, // u32; altitude (centimetres above ground)
    FPORT_DATAID_FUEL       = 0x0600, // u32; remaining percentage OR mAh drawn
    FPORT_DATAID_ADC1       = 0xF102, // u8; (adc/19.4, clamped to 13.1V)
    FPORT_DATAID_ADC2       = 0xF103, // u8; (adc/19.4, clamped to 13.1V)
    /*
      latlong is complex : encodage for lat and long
      tmpui = abs(gpsSol.llh.lon);  // now we have unsigned value and one bit to spare
      tmpui = (tmpui + tmpui / 2) / 25 (FOR LONG ONLY: | 0x80000000);  
      if (gpsSol.llh.lon < 0) tmpui |= 0x40000000;
      
      then at each interrogation, one time is for long, next time if for lat
    */
    FPORT_DATAID_LATLONG    = 0x0800, // u32; latitude or longitude * 1e+7

    FPORT_DATAID_VARIO      = 0x0110, // s32; centimeters/seconds
    FPORT_DATAID_CELLS      = 0x0300, // u32; individual cell voltage in u32 (volt / 100)
    FPORT_DATAID_CELLS_LAST = 0x030F,
    FPORT_DATAID_HEADING    = 0x0840, // u32; (degrees / 100)
    // DIY range 0x5100 to 0x52FF
    FPORT_DATAID_CAP_USED   = 0x5250, // u32; remaining percentage OR mAh drawn in u32
    FPORT_DATAID_PITCH      = 0x5230, // u32; custom; (degree / 10)
    FPORT_DATAID_ROLL       = 0x5240, // u32; custom; (degree / 10)
    FPORT_DATAID_ACCX       = 0x0700, // u32; (G / 100)
    FPORT_DATAID_ACCY       = 0x0710, // u32; (G / 100)
    FPORT_DATAID_ACCZ       = 0x0720, // u32; (G / 100)
    FPORT_DATAID_T1         = 0x0400, // s32; temperature 1 in ° celcius
    FPORT_DATAID_T11        = 0x0401, // s32; also temperature 1 ?
    FPORT_DATAID_T2         = 0x0410, // s32; temperature 1 in ° celcius
    FPORT_DATAID_HOME_DIST  = 0x0420, // u32; distance to home in meters
    FPORT_DATAID_GPS_ALT    = 0x0820, // u32; altitude from GPS (centimetres above ground)
    FPORT_DATAID_ASPD       = 0x0A00, // u32; air speed (knot / 10) 
    FPORT_DATAID_TEMP       = 0x0B70, // s32; temperature (degree C / 100)
    FPORT_DATAID_TEMP1      = 0x0B71,
    FPORT_DATAID_TEMP2      = 0x0B72,
    FPORT_DATAID_TEMP3      = 0x0B73,
    FPORT_DATAID_TEMP4      = 0x0B74,
    FPORT_DATAID_TEMP5      = 0x0B75,
    FPORT_DATAID_TEMP6      = 0x0B76,
    FPORT_DATAID_TEMP7      = 0x0B77,
    FPORT_DATAID_TEMP8      = 0x0B78,
    FPORT_DATAID_A3_FIRST_ID = 0x0900, // u32; voltage (volt / 100)
    FPORT_DATAID_A3_LAST_ID = 0x090F,  // u32; voltage (volt / 100)
    FPORT_DATAID_A4_FIRST_ID = 0x0910, // u32; voltage (volt / 100)
    FPORT_DATAID_A4_LAST_ID = 0x091F,  // u32; voltage (volt / 100)
    FPORT_DATAID_UNDEF      = 0xFFFF,

    // OPENTX IDs (betaflight use some of theses, but not all)
    FPORT_ALT_FIRST_ID =              0x0100,
    FPORT_ALT_LAST_ID =               0x010F,
    FPORT_VARIO_FIRST_ID =            0x0110,
    FPORT_VARIO_LAST_ID =             0x011F,
    FPORT_CURR_FIRST_ID =             0x0200,
    FPORT_CURR_LAST_ID =              0x020F,
    FPORT_VFAS_FIRST_ID =             0x0210,
    FPORT_VFAS_LAST_ID =              0x021F,
    FPORT_CELLS_FIRST_ID =            0x0300,
    FPORT_CELLS_LAST_ID =             0x030F,
    FPORT_T1_FIRST_ID =               0x0400,
    FPORT_T1_LAST_ID =                0x040F,
    FPORT_T2_FIRST_ID =               0x0410,
    FPORT_T2_LAST_ID =                0x041F,
    FPORT_RPM_FIRST_ID =              0x0500,
    FPORT_RPM_LAST_ID =               0x050F,
    FPORT_FUEL_FIRST_ID =             0x0600,
    FPORT_FUEL_LAST_ID =              0x060F,
    FPORT_ACCX_FIRST_ID =             0x0700,
    FPORT_ACCX_LAST_ID =              0x070F,
    FPORT_ACCY_FIRST_ID =             0x0710,
    FPORT_ACCY_LAST_ID =              0x071F,
    FPORT_ACCZ_FIRST_ID =             0x0720,
    FPORT_ACCZ_LAST_ID =              0x072F,
    FPORT_GPS_LONG_LATI_FIRST_ID =    0x0800,
    FPORT_GPS_LONG_LATI_LAST_ID =     0x080F,
    FPORT_GPS_ALT_FIRST_ID =          0x0820,
    FPORT_GPS_ALT_LAST_ID =           0x082F,
    FPORT_GPS_SPEED_FIRST_ID =        0x0830,
    FPORT_GPS_SPEED_LAST_ID =         0x083F,
    FPORT_GPS_COURS_FIRST_ID =        0x0840,
    FPORT_GPS_COURS_LAST_ID =         0x084F,
    FPORT_GPS_TIME_DATE_FIRST_ID =    0x0850,
    FPORT_GPS_TIME_DATE_LAST_ID =     0x085F,
    FPORT_A3_FIRST_ID =               0x0900,
    FPORT_A3_LAST_ID =                0x090F,
    FPORT_A4_FIRST_ID =               0x0910,
    FPORT_A4_LAST_ID =                0x091F,
    FPORT_AIR_SPEED_FIRST_ID =        0x0A00,
    FPORT_AIR_SPEED_LAST_ID =         0x0A0F,
    FPORT_FUEL_QTY_FIRST_ID =         0x0A10,
    FPORT_FUEL_QTY_LAST_ID =          0x0A1F,
    FPORT_RBOX_BATT1_FIRST_ID =       0x0B00, // multiplexed voltage+current
    FPORT_RBOX_BATT1_LAST_ID =        0x0B0F, // multiplexed voltage+current
    FPORT_RBOX_BATT2_FIRST_ID =       0x0B10, // multiplexed voltage+current
    FPORT_RBOX_BATT2_LAST_ID =        0x0B1F, // multiplexed voltage+current
    FPORT_RBOX_STATE_FIRST_ID =       0x0B20, // multiplexed bitfield+bitfield
    FPORT_RBOX_STATE_LAST_ID =        0x0B2F, // multiplexed bitfield+bitfield
    FPORT_RBOX_CNSP_FIRST_ID =        0x0B30, // multiplexed current+current
    FPORT_RBOX_CNSP_LAST_ID =         0x0B3F, // multiplexed current+current
    FPORT_SD1_FIRST_ID =              0x0B40,
    FPORT_SD1_LAST_ID =               0x0B4F,
    FPORT_ESC_POWER_FIRST_ID =        0x0B50, // multiplexed voltage+current
    FPORT_ESC_POWER_LAST_ID =         0x0B5F, // multiplexed voltage+current
    FPORT_ESC_RPM_CONS_FIRST_ID =     0x0B60,
    FPORT_ESC_RPM_CONS_LAST_ID =      0x0B6F,
    FPORT_ESC_TEMPERATURE_FIRST_ID =  0x0B70,
    FPORT_ESC_TEMPERATURE_LAST_ID =   0x0B7F,
    FPORT_RB3040_OUTPUT_FIRST_ID =    0x0B80,
    FPORT_RB3040_OUTPUT_LAST_ID =     0x0B8F,
    FPORT_RB3040_CH1_2_FIRST_ID =     0x0B90, // multiplexed current+current
    FPORT_RB3040_CH1_2_LAST_ID =      0x0B9F, // multiplexed current+current
    FPORT_RB3040_CH3_4_FIRST_ID =     0x0BA0, // multiplexed current+current
    FPORT_RB3040_CH3_4_LAST_ID =      0x0BAF, // multiplexed current+current
    FPORT_RB3040_CH5_6_FIRST_ID =     0x0BB0, // multiplexed current+current
    FPORT_RB3040_CH5_6_LAST_ID =      0x0BBF, // multiplexed current+current
    FPORT_RB3040_CH7_8_FIRST_ID =     0x0BC0, // multiplexed current+current
    FPORT_RB3040_CH7_8_LAST_ID =      0x0BCF, // multiplexed current+current
    FPORT_X8R_FIRST_ID =              0x0C20,
    FPORT_X8R_LAST_ID =               0x0C2F,
    FPORT_S6R_FIRST_ID =              0x0C30,
    FPORT_S6R_LAST_ID =               0x0C3F,
    FPORT_GASSUIT_TEMP1_FIRST_ID =    0x0D00,
    FPORT_GASSUIT_TEMP1_LAST_ID =     0x0D0F,
    FPORT_GASSUIT_TEMP2_FIRST_ID =    0x0D10,
    FPORT_GASSUIT_TEMP2_LAST_ID =     0x0D1F,
    FPORT_GASSUIT_SPEED_FIRST_ID =    0x0D20,
    FPORT_GASSUIT_SPEED_LAST_ID =     0x0D2F,
    FPORT_GASSUIT_RES_VOL_FIRST_ID =  0x0D30,
    FPORT_GASSUIT_RES_VOL_LAST_ID =   0x0D3F,
    FPORT_GASSUIT_RES_PERC_FIRST_ID = 0x0D40,
    FPORT_GASSUIT_RES_PERC_LAST_ID =  0x0D4F,
    FPORT_GASSUIT_FLOW_FIRST_ID =     0x0D50,
    FPORT_GASSUIT_FLOW_LAST_ID =      0x0D5F,
    FPORT_GASSUIT_MAX_FLOW_FIRST_ID = 0x0D60,
    FPORT_GASSUIT_MAX_FLOW_LAST_ID =  0x0D6F,
    FPORT_GASSUIT_AVG_FLOW_FIRST_ID = 0x0D70,
    FPORT_GASSUIT_AVG_FLOW_LAST_ID =  0x0D7F,
    FPORT_SBEC_POWER_FIRST_ID =       0x0E50, // multiplexed voltage+current
    FPORT_SBEC_POWER_LAST_ID =        0x0E5F, // multiplexed voltage+current
    FPORT_DIY_FIRST_ID =              0x5100,
    FPORT_DIY_LAST_ID =               0x52FF,
    FPORT_DIY_STREAM_FIRST_ID =       0x5000,
    FPORT_DIY_STREAM_LAST_ID =        0x50FF,
    FPORT_SERVO_FIRST_ID =            0x6800, // multiplexed current+voltage+temperature+text
    FPORT_SERVO_LAST_ID =             0x680F, // multiplexed current+voltage+temperature+text
    FPORT_FACT_TEST_ID =              0xF000,
    FPORT_VALID_FRAME_RATE_ID =       0xF010,
    FPORT_RSSI_ID =                   0xF101,
    FPORT_ADC1_ID =                   0xF102,
    FPORT_ADC2_ID =                   0xF103,
    FPORT_BATT_ID =                   0xF104,
    FPORT_RAS_ID =                    0xF105,
    FPORT_XJT_VERSION_ID =            0xF106,
    FPORT_R9_PWR_ID =                 0xF107,
    FPORT_SP2UART_A_ID =              0xFD00,
    FPORT_SP2UART_B_ID =              0xFD01,
  } fportAppId;
