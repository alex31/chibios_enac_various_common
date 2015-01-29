#pragma once

#include "i2cPeriphBase.hpp"


using namespace std;



// namespace Example {
//   typedef const std::tuple<std::array<uint8_t, 3>, std::array<uint8_t, 3>> initSequence_type;
//   typedef const std::tuple<std::array<uint8_t, 2>> devidSequence_type;
//   extern constexpr initSequence_type initSequence {{0x10, 1,2}, {0x10, 1,2}};
//   extern constexpr devidSequence_type devidSequence {{0x15, 1}};
//   extern constexpr const char* example = {"example"};
// }

// template<const char* const &_name,
// 	 const I2cChannel &_i2cd>
// class I2cPeriphExample : public I2cPeriphBase<_name,
// 					      _i2cd, 
// 					      0x10, 10, 2, 
// 					      std::array<float, 2>, 
// 					      Example::initSequence_type,
// 					      Example::devidSequence_type,
// 					      Example::initSequence,
// 					      Example::devidSequence>
// {
//  public:
//   //  virtual const  std::array<float, 2> getValue (void) const {return {0,0};}
// };



// MPL3115A2
namespace MPL3115A2 {
  using initSequence_type =  const std::tuple<std::array<uint8_t, 2>, std::array<uint8_t, 2>>;
  using devidSequence_type = const std::array<uint8_t, 1>;

  constexpr uint8_t i2cAddr =  0x60;
  constexpr uint8_t  OVERSAMPLING = 0x0 << 3;
  constexpr std::array<uint8_t, 2> activeMode = {0x26, 0x01 | OVERSAMPLING};
  constexpr std::array<uint8_t, 2> standbyMode = {0x26, 0x0 | OVERSAMPLING};
  constexpr std::array<uint8_t, 2> oneShotMode = {0x26, 0x2 | OVERSAMPLING};
  constexpr std::array<uint8_t, 2> continuousMode = {0x26, 0x2 | OVERSAMPLING};
  constexpr std::array<uint8_t, 2> enableEvent = {0x13, 0x07};
  constexpr std::array<uint8_t, 1> statusReg = {0x00};
  constexpr std::array<uint8_t, 1> pressureReg = {0x01};

  extern constexpr initSequence_type initSequence {enableEvent, standbyMode};
  extern constexpr devidSequence_type devidSequence {0x0c};
  extern constexpr const char* baro = {"baro mpl"};
}

template<const char* const &_name,
	 const I2cChannel &_i2cd>
class I2cBaroMPL3115A2 : public I2cPeriphBase<_name,
					      _i2cd, 
					      MPL3115A2::i2cAddr,
					      /*MPL3115A2::pressureReg[0]*/ 0x01, 
					      4, 
					      std::array<float, 1>, 
					      MPL3115A2::initSequence_type,
					      MPL3115A2::devidSequence_type,
					      MPL3115A2::initSequence,
					      MPL3115A2::devidSequence>
{
 public:
  using I2cBaroMPL3115A2::I2cPeriphBase::registerValues;
  using I2cBaroMPL3115A2::I2cPeriphBase::status;
  //  using I2cBaroMPL3115A2::I2cPeriphBase::_registerBaseAddr;
  //  using I2cBaroMPL3115A2::I2cPeriphBase::_i2cAddr;


  // copy raw data from i2c peripheral
  static void  cacheRegisters (void) {
    bool_t  notReady;
    i2cAcquireBus(&_i2cd.driver);
    i2cMasterTransmitTimeout (&_i2cd.driver, MPL3115A2::i2cAddr,
			      MPL3115A2::oneShotMode.data(), MPL3115A2::oneShotMode.size(),
			      nullptr, 0, 100);
    do {
      i2cMasterTransmitTimeout (&_i2cd.driver, MPL3115A2::i2cAddr,
				MPL3115A2::statusReg.data(), MPL3115A2::statusReg.size(),
				registerValues.data(), 2, 100);
      notReady = !(registerValues[0] & 1<<2);
      if (notReady) {
	chThdSleepMilliseconds(2);
      }
    } while (notReady);
    
    status = i2cMasterTransmitTimeout (&_i2cd.driver, MPL3115A2::i2cAddr,
				       MPL3115A2::pressureReg.data(), MPL3115A2::pressureReg.size(),
				       registerValues.data(), registerValues.size(), 100);
    i2cReleaseBus(&_i2cd.driver);
    if (status != RDY_OK) {						
      DebugTrace ("I2C error read/write on %s, bus %s, status =%s", _name, _i2cd.name,
		  status == RDY_RESET ?			
		  "RDY_RESET" : "RDY_TIMEOUT");				
      
    }
  }
  
  
  static const  std::array<float, 1> getValue (void)  {
    const int32_t *rawB = (int32_t *) registerValues.data();
    //const int32_t *rawB = (int32_t *) getRawValues().data();
    const uint32_t swapVal = (SWAP_ENDIAN32(*rawB<<8)) ;
    return {swapVal/6400.0f};
  }

};



namespace MPU6050 {
  using initSequence_type = const std::tuple<std::array<uint8_t, 2>, 
					     std::array<uint8_t, 2>,
					     std::array<uint8_t, 2>,
					     std::array<uint8_t, 2>,
					     std::array<uint8_t, 2>>;
  using devidSequence_type = const std::array<uint8_t, 1>;
  constexpr uint8_t i2cAddr =  0x69;
  constexpr std::array<uint8_t, 2> activeMode = {0x6b, 0x1};
  constexpr std::array<uint8_t, 2> sampleDiv8 = {0x19, 0x7};
  constexpr std::array<uint8_t, 2> noLowPassFilter = {0x1a, 0x0};
  constexpr std::array<uint8_t, 2> initBypass1 = {0x37, 0x2};
  constexpr std::array<uint8_t, 2> initBypass2 = {0x6a, 0x0};
  constexpr std::array<uint8_t, 2> initMaster1 = {0x37, 0x0};
  constexpr std::array<uint8_t, 2> initMaster2 = {0x6a, 0x20};
  constexpr std::array<uint8_t, 2> initMaster3 = {0x24, 0x0d};
  constexpr std::array<uint8_t, 2> initMaster4 = {0x67, 0x0};


  // extern constexpr initSequence_type initSequence {activeMode, sampleDiv8, noLowPassFilter, 
  // 						   initBypass1, initBypass2};
  extern constexpr initSequence_type initSequence {activeMode, sampleDiv8, noLowPassFilter, 
						   initBypass1, initBypass2};
  extern constexpr devidSequence_type devidSequence {0x75};
  extern constexpr const char* imu = {"imu mpl"};
}

template<const char* const &_name,
	 const I2cChannel &_i2cd>
class I2cImuMPU6050 : public I2cPeriphBase<_name,
					   _i2cd, 
					   MPU6050::i2cAddr,
					   0x3a, 
					   15, 
					   std::tuple <float,
							std::array<float, 3>,
							std::array<float, 3>>, 
					   MPU6050::initSequence_type,
					   MPU6050::devidSequence_type,
					   MPU6050::initSequence,
					   MPU6050::devidSequence>
{
 public:
  using I2cImuMPU6050::I2cPeriphBase::registerValues;
  using I2cImuMPU6050::I2cPeriphBase::status;

  
  
  static const std::tuple <float,
			   std::array<float, 3>,
			   std::array<float, 3>>  getValue (void)  {
    
    const decltype(registerValues) &rv = registerValues;
 
    float temp = ( ((int16_t) ((rv[7]<<8) | rv[8])) / 340.0f) + 36.53f;

    std::array<float, 3> acc;
    for (uint32_t i=0; i< acc.size(); i++) {
      acc[i] =  ((int16_t) ((rv[(i*2)+1]<<8) | rv[(i*2)+2])) / 16384.0f; // POUR MODE 2G
    }
    std::array<float, 3> gyro;
    for (uint32_t i=0; i< 3; i++) {
      gyro[i] =  ((int16_t) ((rv[(i*2)+9]<<8) | rv[(i*2)+10])) / 131.0f; // POUR MODE 250°/s
    }
    return std::make_tuple (temp, acc, gyro);
  }

};


namespace HMC5883L {
  using initSequence_type = const std::tuple<std::array<uint8_t, 4>>;
  using devidSequence_type =  const std::array<uint8_t, 1>;

  constexpr uint8_t i2cAddr =  0x1e;

  // sample average 8, gain (+/-  1.30 G)
  constexpr std::array<uint8_t, 4> sampleAvg8_minGain = {0x0, 0x6c, 0x20, 0x0}; 

  // no sample average, max gain (+/- 0.88 G)
  constexpr std::array<uint8_t, 4> sampleAvg1_maxGain = {0x0, 0x18, 0x00, 0x0}; 

  extern constexpr initSequence_type initSequence {sampleAvg1_maxGain};
  extern constexpr devidSequence_type devidSequence {0x0a};
  extern constexpr const char* compass = {"compass hmc"};
}

template<const char* const &_name,
	 const I2cChannel &_i2cd>
class I2cCompassHMC5883L : public I2cPeriphBase<_name,
						_i2cd, 
					      HMC5883L::i2cAddr,
					      /*HMC5883L::pressureReg[0]*/ 0x03, 
					      6, 
					      std::array<float, 3>, 
					      HMC5883L::initSequence_type,
					      HMC5883L::devidSequence_type,
					      HMC5883L::initSequence,
					      HMC5883L::devidSequence>
{
 public:
  using I2cCompassHMC5883L::I2cPeriphBase::registerValues;
  using I2cCompassHMC5883L::I2cPeriphBase::status;
  //  using I2cCompassHMC5883L::I2cPeriphBase::_registerBaseAddr;
  //  using I2cCompassHMC5883L::I2cPeriphBase::_i2cAddr;


  static const  std::array<float, 3> getValue (void)  {
    std::array<float, 3> mag;
    const int16_t *rawB = (int16_t *) registerValues.data();

    mag[0] = SWAP_ENDIAN16(rawB[0]) / 1.37f;
    mag[2] = SWAP_ENDIAN16(rawB[1]) / 1.37f; // HMC5883L returns values in x,z,y order
    mag[1] = SWAP_ENDIAN16(rawB[2]) / 1.37f;

    
    return mag;
  }

};


#ifdef USE_24AA02
namespace 24AA02 {
  using initSequence_type = const std::tuple<std::array<uint8_t, 0>>;
  using devidSequence_type =  const std::array<uint8_t, 0>;

  const initSequence_type initSequence = {};
  const devidSequence_type devidSequence = {};

  constexpr uint8_t i2cAddr =  0b1010000;

  extern constexpr const char* eeprom = {"eeprom 24AA02"};
}

template<const char* const &_name,
	 const I2cChannel &_i2cd>
class Eeprom24AA02 : public I2cPeriphBase<_name,
						_i2cd, 
					      24AA02::i2cAddr,
					      /*24AA02::pressureReg[0]*/ 0x03, 
					      0, 
					      std::array<uint8_t, 8>, 
					      24AA02::initSequence_type,
					      24AA02::devidSequence_type,
					      24AA02::initSequence,
					      24AA02::devidSequence>
{
 public:

  static const  void getBuffer (const uint8_t address, uint8_t *buffer, uint8_t bufferSize)  {
    (void) address;
    (void) buffer;
    (void) bufferSize;
  }

};
#endif
