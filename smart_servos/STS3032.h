#pragma once
#include "smart_servo.h"

class STS3032: public SmartServo {

public:

  enum Baudrate {
    BD_1M = 0,
    BD_500K = 1,
    BD_250K = 2,
    BD_128K = 3,
    BD_115200 = 4,  //115107
    BD_76800 = 5,   //76923
    BD_57600 = 6,   //57553.9
    BD_38400 = 7    // 38461.5
  };

  struct StateVector {
    float    position = {};  // [-1 .. 1]
    float    load = {}; // [0 ..1]
    float    voltage = {}; // volts
    float    current = {}; // amp
    uint16_t speed = {};  // step/s
    uint16_t status = STATUS_TIMEOUT;
    uint8_t  temperature = {};
    bool     moving = {};   
  };

  STS3032(UARTDriver* s): SmartServo(s) {}

  SmartServo::Status setBaudrate(uint8_t id, uint32_t speed) override;
    

  SmartServo::Status setID(uint8_t id, uint8_t newID) override;

  SmartServo::Status move(uint8_t id, uint16_t position, bool reg_write=false) override;
  SmartServo::Status speedLimit(uint8_t id, uint16_t speed, bool reg_write=false);
  SmartServo::Status moveSpeed(uint8_t id, uint16_t position, uint16_t speed, bool reg_write=false) override;
  SmartServo::Status setEndless(uint8_t id, bool status) override;
  SmartServo::Status turn(uint8_t id, RotationDirection direction, uint16_t speed) override;
  SmartServo::Status setTorque(uint8_t id, uint16_t torque) override;
  SmartServo::Status torqueEnable(uint8_t id, bool enable) override;
  SmartServo::Status setLimits(uint8_t id, uint16_t minAngle, uint16_t maxAngle) override;
  SmartServo::Status setResolution(uint8_t id, uint8_t resolution);

  SmartServo::Status setMultiturn(uint8_t id, uint8_t factor);
    
  // true for locking, false for unlocking
  SmartServo::Status lock_eprom(uint8_t id, bool lock);

  int readPosition(uint8_t id) override;
  STS3032::StateVector readStates (uint8_t id);

  // Read response level and update smart_servo accordingly.
  int readResponseLevel(uint8_t id);

private:
  SmartServo::Status writeRegisterEPROM(uint8_t id, uint8_t reg, uint8_t value);

  enum eRegister {
    //-------EPROM--------
    R_FirmwareMajorVersion = 0x01,
    R_FirmwareMinorVersion = 0x02,
    R_ServoMajorVersion = 0x03,
    R_ServoMinorVersion = 0x04,
    //-------EPROM--------
    R_ServoID    = 0x05,
    R_BaudRate = 0x06,
    R_returnDelayTime = 0x07,

    // - 0: reply only for READ and PING instructions
    // - 1: reply for all instructions
    R_ResponseStatusLevel = 0x08,

    // Min angle limit. [0-4094].(2 bytes)
    // Set to 0 for multiturn position control.
    R_MinAngleLimit = 0x09,

    // Max angle limit. [0-4095]. (2 bytes)
    // Set to 0 for multiturn position control.
    R_MaxAngleLimit = 0x0B,

    // Maximum operating temperature, in °C.  [0-100]°C
    R_MaxTempLimit = 0x0D,
    R_MaxInputVoltage = 0x0E,   // unit: 0.1V
    R_MinInputVoltage = 0x0F,   // unit: 0.1V

    // 0.1% of max torque. [0-1000]. (2 bytes)
    R_MaxTorque = 0x10,

    //R_HighVoltageFlagBit = 0x12,

    // set status flag bits to enable protection for these errors.
    R_UnloadCondition = 0x13,

    // set status flag bits to enable LED flashing alarm for these errors.
    R_LEDAlarmCondition = 0x14,


    R_P = 0x15,
    R_D = 0x16,
    R_I = 0x17,
    R_MinStartupForce = 0x18,    // 2 bytes [0-1000]

    R_CW_DeadZone   = 0x1A,     // [0-32] steps
    R_CCW_DeadZone  = 0x1B,     // [0-32] steps

    // Protection current in the [0-511] range. 6.5mA steps. (2 bytes)
    R_ProtectionCurrent = 0x1C,

    /** Angular resolution in [1-100] range.
     * Lowering resolution increase the number of control turns.
     */
    R_AngularResolution = 0x1E,

    /** Offset angle. (2 bytes)
     * bit 11 is the direction
     * bits[10:0] in the range 0-2047
     */
    R_PositionCorrection = 0x1F,

    /** Operation mode: [0-3]
     *  - 0: position servo mode.
     *  - 1: constant speed mode. Controlled by 0x2E (goal speed)
     *  - 2: PWM open-loop regulation. Controlled by 0x2C (goal time)
     *  - 3: Step servo mode. The number of step progress is represented by parameter 0x2a, and the highest bit 15 is the direction bit. (???)
     */
    R_OperationMode  = 0x21,

    // torque after entering overload protection. Unit: 1% of max torque
    R_ProtectiveTorque = 0x22,

    // Time for which load output can exceed overload torque. Unit: 10ms.
    R_ProtectionTime = 0x23,

    // torque threshold starting protective time meter. Unit: 1% of max torque.
    R_OverloadTorque = 0x24,

    // In speed mode (mode 1), proportionnal gain.
    R_Speed_P = 0x25,

    // ??? diff with R_ProtectionTime ?
    R_OverCurrentProtectionTime = 0x26,

    // In speed mode (mode 1), integral gain.
    R_Speed_I = 0x27,

    //--------------------SRAM-----------------------------

    /** - 0: turn off torque output
     *  - 1: turn on torque output
     *  - 128: current position correction is 2048 (???)
     */
    R_TorqueEnable = 0x28,

    // unit: 100step/s^2
    R_Acceleration = 0x29,

    R_GoalPosition = 0x2A,    //  (2 bytes)

    // unit: 0.1% [0-1000] range. (2 bytes)
    R_RunningTime = 0x2C,

    // Nb of steps/second. 50 steps / second = 0.732 RPM. (2 bytes)
    R_RunningSpeed = 0x2E,

    // unit: 0.1% (or 1% ?) of max torque
    R_TorqueLimit = 0x30,

    /** 
     * - 0: unlock EPROM
     * - 1: lock EPROM
     */
    R_Lock = 0x37,

    //-----------------SRAM-------------------

    // unit: step. (2 bytes)
    R_CurrentPosition = 0x38,

    // unit: step/s. (2 bytes)
    R_CurrentSpeed = 0x3A,

    //  motor drive duty cycle. unit: 0.1%. (2 bytes)
    R_CurrentLoad = 0x3C,

    // unit: 0.1V
    R_CurrentVoltage = 0x3E,

    // unit: °C
    R_CurrentTemperature = 0x3F,

    R_AsynchronousWriteFlag = 0x40,
        
    // Status flags
    R_ServoStatus = 0x41,
        
    // 1 if the servo is moving, 0 if servo is stopped
    R_Moving = 0x42,

    // Current. unit: 6.5mA. (2 bytes)
    R_CurrentCurrent = 0x45,
  };

};

extern STS3032 sts3032;
