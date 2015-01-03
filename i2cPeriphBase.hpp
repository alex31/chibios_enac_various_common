#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <initializer_list>
#include <tuple>
#include <ch.h>
#include <hal.h>
#include "stdutil.h"
#include "globalVar.h"
#include "stdutil++.hpp"


/*




  class I2cPeriphBase : classe de base. 

  °  classe interface pour les type de peripherique : PressureSensor, IMU, gyro etc
  °  classe pour gerer le timing de la recuperation des infos sur le periphérique

  class SensorProbeRate : classe de base pour la frequence de collection des données

  classe pour un peripherique particulier qui dérive de I2cPeriphBase et 
  eventuellement d'une classe abstraite de pression, imu, etc etc
  et de SensorProbeRate



  template	
  class I2cChannel
  avec toutes les references sur les I2cPeriph* passés en variadic template et stoqués
  dans un std::tuple constexpr 


*/

using namespace std;



struct I2cChannel  {

  enum class PalMode {INPUT, OUTPUT, I2C};
  
 
  I2CDriver		&driver;
  const char		*name;
  const I2CConfig	i2ccfg;
  GPIO_TypeDef  * const sdaGpio;
  GPIO_TypeDef  * const sclGpio;
  const uint8_t         sdaPin;
  const uint8_t         sclPin;
  const uint8_t	        alternateFunction;

  void configureMode (const enum  PalMode mode) {
    switch (mode) {
    case PalMode::OUTPUT: 
      palSetPadMode (sdaGpio, sdaPin, PAL_MODE_OUTPUT_PUSHPULL); 
      palSetPadMode (sclGpio, sclPin, PAL_MODE_OUTPUT_PUSHPULL); 
       break;
      
     case PalMode::INPUT:  
      palSetPadMode (sdaGpio, sdaPin, PAL_MODE_INPUT); 
      palSetPadMode (sclGpio, sclPin, PAL_MODE_INPUT); 
       break;
     case PalMode::I2C:  
       palSetPadMode (sclGpio, sclPin, 
		      PAL_MODE_ALTERNATE(alternateFunction) | PAL_STM32_OTYPE_OPENDRAIN |
		      PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUDR_PULLUP);
       
       palSetPadMode (sdaGpio, sdaPin, 
		      PAL_MODE_ALTERNATE(alternateFunction) | PAL_STM32_OTYPE_OPENDRAIN |
		      PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUDR_PULLUP);
      
       break;
    }
  }


  bool_t unhang (void) {
    bool_t sdaReleased;
    bool_t currentInput;

    palSetPadMode (sdaGpio, sdaPin, PAL_MODE_INPUT); 
    sdaReleased = (palReadPad (sdaGpio, sdaPin) == 1);
    if (sdaReleased) 
      goto end;
    
    palSetPadMode (sclGpio, sclPin, PAL_MODE_INPUT);
    currentInput = palReadPad (sclGpio, sclPin);
    if (currentInput) 
      palSetPad (sclGpio, sclPin);
    else
      palClearPad (sclGpio, sclPin);
    
    palSetPadMode (sclGpio, sclPin, PAL_MODE_OUTPUT_PUSHPULL);
    
    for (uint8_t i=0; i<=8; i++) {
      halPolledDelay (US2ST(10)) ; // 10µs : 100 khz
      palTogglePad (sclGpio, sclPin);
      halPolledDelay (US2ST(10)) ; // 10µs : 100 khz
      palTogglePad (sclGpio, sclPin);
      halPolledDelay (US2ST(10)) ; // 10µs : 100 khz
      
      sdaReleased = (palReadPad (sdaGpio, sdaPin) == 1);
      if (sdaReleased) 
	break;
    }
    
  end:
    //    DebugTrace ("Unhang %s %s", name, sdaReleased ? "OK" : "Fail");
    if (not sdaReleased) {
      DebugTrace ("Unhang %s FAIL");
    }
    return sdaReleased; 
  }


  bool_t start (void) {
    bool_t res = unhang();
    configureMode (PalMode::I2C);
    i2cStart(&driver, &i2ccfg);
    return res;
  }

  bool_t   resetBus (void) {
  i2cStop (&driver);
  return start();
}


} ;




#include "i2cConf.hpp"

#if STM32_I2C_USE_I2C1 && ! defined I2C_CHANNEL_1
#error "if STM32_I2C_USE_I2C1 is defined in mcuconf.h, I2C_CHANNEL_1 should be defined in i2cConf.h"
#endif

#if STM32_I2C_USE_I2C2 && ! defined I2C_CHANNEL_2
#error "if STM32_I2C_USE_I2C2 is defined in mcuconf.h, I2C_CHANNEL_2 should be defined in i2cConf.h"
#endif

#if STM32_I2C_USE_I2C3 && ! defined I2C_CHANNEL_3
#error "if STM32_I2C_USE_I2C3 is defined in mcuconf.h, I2C_CHANNEL_3 should be defined in i2cConf.h"
#endif


template<const char* const &_name,
	 const I2cChannel &_i2cd,
	 uint8_t _i2cAddr, uint32_t _registerBaseAddr, 
	 uint32_t _registerBufferSize, typename values_type,
         typename init_sequence_type,
         typename devid_sequence_type,
	 init_sequence_type& _init_sequence,
	 devid_sequence_type& _devid_sequence>
class I2cPeriphBase
{
protected:
  static msg_t		status;

  struct i2cWrite_functor  {
    template<typename T>
    void operator () (T&& buffer)
    {
      if (status == RDY_OK) {
	if (buffer.size() != 0) {
	  DebugTrace ("aquire bus");
	  i2cAcquireBus(&_i2cd.driver);
	  DebugTrace ("bus aquired");
	  
	  status = i2cMasterTransmitTimeout (&_i2cd.driver, _i2cAddr,
					     buffer.data(), buffer.size(),
					     NULL, 0, 100);
	  DebugTrace ("i2cWrite_functor periph=%s len=%d [0x%x, 0x%x]", _name, buffer.size(),
		      buffer[0], buffer[1]);
	  i2cReleaseBus(&_i2cd.driver);
	  if (status != RDY_OK) {						
	    DebugTrace ("I2C error write on %s, bus %s, status =%s", _name, _i2cd.name,
			status == RDY_RESET ?			
			"RDY_RESET" : "RDY_TIMEOUT");				
	    
	  }
	}
      }
    }
  };
  
  // struct i2cReadWrite_functor  {
  //   template<typename T>
  //   void operator () (T&& buffer)
  //   {
  //     i2cAcquireBus(&_i2cd.driver);
  //     msg_t _status = i2cMasterTransmitTimeout (&_i2cd.driver, _i2cAddr,
  // 						buffer.data(), buffer.size(),
  // 						registerValues.data(), registerValues.size(), 100);
  //     i2cReleaseBus(&_i2cd.driver);
  //     if (_status != RDY_OK) {						
  // 	DebugTrace ("I2C error read/write on %s, bus %s, status =%s", _name, _i2cd.name,
  // 		    status == RDY_RESET ?			
  // 		    "RDY_RESET" : "RDY_TIMEOUT");				
	
  // 	status |= _status;
  //     }
  //   }
  // };

public:
  using registerBuffer_t = std::array<uint8_t, _registerBufferSize>;
  // return name
  static constexpr char * getName (void) {return _name;}
  // return first peripheral register address
  static constexpr uint32_t getRegisterBaseAddr (void) {return _registerBaseAddr;}
  // return peripheral register length 
  static constexpr uint32_t getRawValuesSize (void) {return _registerBufferSize;} 
  // return i2c address in 7 bits form (must be uniq on an i2c bus)
  static constexpr uint8_t getI2cAddr (void)  {return _i2cAddr;} 
  // return reference to raw values for debug
  static const registerBuffer_t &  getRawValues (void)  {return registerValues;} 
  // copy raw data from i2c peripheral
  static  msg_t  initPeripheral (void) {
    
    // // use chibios i2c api here to send init sequence
    // constexpr const uint8_t &i2cRegAddr = std::get<0>(_init_sequence);
    // constexpr typename std::tuple_element<1,init_sequence_type>::type &buffer = 
    //   std::get<1>(_init_sequence);
    // printf ("init sequence i2cRegAddr = %u\n", i2cRegAddr);
    // for (auto &i : buffer) {
    //   printf ("init sequence : %d\n", i);
    // }
    status = RDY_OK;
    for_each_in_tuple(_init_sequence, i2cWrite_functor());

    return status;
  }

  // copy raw data from i2c peripheral
  static void  cacheRegisters (void) {
    if (status == RDY_OK) {
      constexpr array<uint8_t, 1> buffer = {_registerBaseAddr};
      i2cAcquireBus(&_i2cd.driver);
      status = i2cMasterTransmitTimeout (&_i2cd.driver, _i2cAddr,
					 buffer.data(), buffer.size(),
					 registerValues.data(), registerValues.size(), 100);
      i2cReleaseBus(&_i2cd.driver);
      if (status != RDY_OK) {						
	DebugTrace ("I2C error read/write on %s, bus %s, status =%s", _name, _i2cd.name,
		    status == RDY_RESET ?			
		    "RDY_RESET" : "RDY_TIMEOUT");				
	
      }
    }
  }
  
  // get dev id from  i2c peripheral
  static uint8_t getDevId(void) {
    std::array<uint8_t, 2> devidResponse = {0xff, 0xff};
    i2cAcquireBus(&_i2cd.driver);
    status = i2cMasterTransmitTimeout (&_i2cd.driver, _i2cAddr,
				       _devid_sequence.data(), _devid_sequence.size(),
				       devidResponse.data(), devidResponse.size(), 100);
    i2cReleaseBus(&_i2cd.driver);
    if (status != RDY_OK) {						
      DebugTrace ("getDevId: I2C error read/write on %s, bus %s, status =%s", _name, _i2cd.name,
		  status == RDY_RESET ?			
		  "RDY_RESET" : "RDY_TIMEOUT");				
      
    }
    return devidResponse[0];
  }
    
    
  // convert from raw register to values
  //  virtual const values_type   getValue (void) const = 0; 

  void printInfo (void) const{
    printf ("i2cAddr=%d\n", _i2cAddr);
  }

  bool getInitStatus (void) {return initOk;}

protected: 
  static uint8_t		initOk;
  static registerBuffer_t	registerValues;
  
  
} __attribute__((packed));


// BEGIN STATIC MEMBER OF TEMPLATE DECLARATION
#define TPLT_I2cPeriphBase(...) template<const char* const &_name,	\
 	 const I2cChannel &_i2cd, \
 	 uint8_t _i2cAddr, uint32_t _registerBaseAddr,  \
 	 uint32_t _registerBufferSize, typename values_type, \
	 typename init_sequence_type, \
	 typename devid_sequence_type, \
 	 init_sequence_type& _init_sequence, \
 	 devid_sequence_type& _devid_sequence> \
__VA_ARGS__ I2cPeriphBase<_name, \
		    _i2cd, \
		    _i2cAddr, _registerBaseAddr,  \
		    _registerBufferSize,  values_type, \
		    init_sequence_type, \
		    devid_sequence_type, \
		    _init_sequence, \
		    _devid_sequence> 


TPLT_I2cPeriphBase(msg_t)::status = RDY_OK;
TPLT_I2cPeriphBase(uint8_t)::initOk = false;
TPLT_I2cPeriphBase(std::array<uint8_t, _registerBufferSize>)::registerValues;
// END STATIC MEMBER OF TEMPLATE DECLARATION
