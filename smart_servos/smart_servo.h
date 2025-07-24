#pragma once
#include <ch.h>
#include <hal.h>
#include "stdutil++.hpp"
#include <etl/span.h>

constexpr size_t MAX_DATA_LEN = 15;
constexpr uint8_t BROADCAST_ID = 254;

constexpr size_t TX_BUF_LEN = 10U * MAX_DATA_LEN;

/*
  FF	Header 1	
FF	Header 2	
01	ID	
05	Length (= 5)	Error + 2 Data + Checksum
00	Error	OK
AB	Data[0] = PosL	
03	Data[1] = PosH	
7C	Checksum
 */
typedef struct __attribute__((packed)) {
    uint16_t STX;
  union {
    struct {
      uint8_t id;
      uint8_t len;
      union {
	uint8_t instruction;        // order  : -> servo
	uint8_t error;		    // response : <- servo
      };
      uint8_t params_and_crc[TX_BUF_LEN + 1U];
    };
    uint8_t raw4[4];
  };
} servo_msg_t;


class SmartServo {
public:

	enum RotationDirection{
		Counterclockwise = 0,
		Clockwise = 1
	};

    typedef struct {
        uint8_t id;
        uint8_t reg;
        uint8_t len;
        uint8_t data[MAX_DATA_LEN];
    } record_t;


    enum Status {
        OK = 0,
        // dynamixel status packet errors
        INPUT_VOLTAGE_ERROR 	= 1<<0,
        ANGLE_LIMIT_ERROR   	= 1<<1,
        OVERHEATING_ERROR   	= 1<<2,
        RANGE_ERROR         	= 1<<3,
        CHECKSUM_ERROR      	= 1<<4,
        OVERLOAD_ERROR      	= 1<<5,
        INSTRUCTION_ERROR   	= 1<<6,

        // errors from this lib
        INVALID_PARAMS      	= 1<<8,
        TX_BUFFER_OVERFLOW	= 1<<9,
        STATUS_TIMEOUT          = 1<<10,
        HETEROGENEOUS_BAUDRATES = 1<<11,
	ALREADY_IN_USE		= 1<<12
    };

  SmartServo(UARTDriver* s): response_level(RL_NORMAL), uartd(s){
    static bool used = false;
    // if we went several servobus on several USART then we should go
    // for refactorisation : template<UARTDriver&> SmartServo and derived class, keeping
    // struct and type definition outside 
    chDbgAssert(used == false, "this is singleton class, static dma_buff cannot be shared");
    used = true;
  }
  virtual ~SmartServo() = default;
  void init();

  void setSerialBaudrate(uint32_t speed);
  uint32_t getSerialBaudrate();
  virtual SmartServo::Status setBaudrate(uint8_t id, uint32_t speed) = 0;
  virtual SmartServo::Status setID(uint8_t id, uint8_t newID) = 0;
  virtual SmartServo::Status move(uint8_t id, uint16_t position, bool reg_write=false) = 0;
  virtual SmartServo::Status moveSpeed(uint8_t id, uint16_t position, uint16_t speed, bool reg_write=false) = 0;
  virtual SmartServo::Status setEndless(uint8_t id, bool status) = 0;
  virtual SmartServo::Status turn(uint8_t id, RotationDirection direction, uint16_t speed) = 0;
  virtual SmartServo::Status setTorque(uint8_t id, uint16_t torque) = 0;
  virtual SmartServo::Status torqueEnable(uint8_t id, bool enable) = 0;
  virtual SmartServo::Status setLimits(uint8_t id, uint16_t minAngle, uint16_t maxAngle) = 0;

  virtual int readPosition(uint8_t id) = 0;
    
  /**
   * Send ping to servo.
   */
  Status ping(uint8_t id);

  /**
   * Reset servo to factorey default values
   */
  Status reset(uint8_t id);


  servo_msg_t* getStatus() {return &servo_status;}

  Status detectBaudrate(etl::span<const uint32_t> baudrates);
  Status detectBaudrate(std::initializer_list<uint32_t> baudrates);

  // void setResponseLevel(uint8_t rl) {response_level = (ResponseLevel)rl;}

protected:

  Status detectBaudrateBroadcast(etl::span<const uint32_t> baudrates);
  Status detectBaudrateUnicast(etl::span<const uint32_t> baudrates);
  /**
   * Read @record.len bytes from servo @record.id, starting at address record.reg.
   */
  Status read(record_t* record);

  /**
   * Write data to servo.
   * @param is_reg_write Perform a registered write. The servo then wait for ACTION to execute the command.
   */
  Status write(record_t* record, bool is_reg_write=false);

  /**
   * Execute command given with reg_write
   */
  Status action(uint8_t id);

  /**
   * Sync write: write to multiples servos with a single packet.
   * All records must share the same register address and the same data lenght.
   * Data can differ for each ID.
   */
  Status sync_write(record_t* records, size_t nb_records);

  // Write one byte to register
  Status writeRegister(uint8_t id, uint8_t reg, uint8_t value);

  enum ResponseLevel {
    RL_SILENT = 0,
    RL_NORMAL = 1,
  };

  enum ResponseLevel response_level;

private:

  Status readStatus(size_t paramlen);
    

  enum Instruction {
    SMART_SERVO_PING = 0x01,
    SMART_SERVO_READ = 0x02,
    SMART_SERVO_WRITE = 0x03,
    SMART_SERVO_REG_WRITE = 0x04,
    SMART_SERVO_ACTION = 0x05,
    SMART_SERVO_RESET = 0x06,
    //SMART_SERVO_REBOOT = 0x08,
    SMART_SERVO_SYNC_WRITE = 0x83,
    //SMART_SERVO_BULK_READ = 0x92,
  };

    




  UARTDriver* uartd;


  static servo_msg_t servo_msg;
  static servo_msg_t servo_status;

};
