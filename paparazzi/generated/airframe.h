/* This file has been generated from /home/alex/DEV/PAPARAZZI/paparazzi/conf/airframes/ENAC/fixed-wing/apogee.xml */
/* Please DO NOT EDIT */

#ifndef AIRFRAME_H
#define AIRFRAME_H

#define AIRFRAME_NAME "withAPJ"
#define AC_ID 1
#define MD5SUM ((uint8_t*)"\034\360\262\145\103\232\156\151\027\340\354\057\153\307\132\365")

#define SERVOS_DEFAULT_NB 3

#define SERVO_MOTOR 0
#define SERVO_MOTOR_NEUTRAL 1040
#define SERVO_MOTOR_TRAVEL_UP 0.1
#define SERVO_MOTOR_TRAVEL_UP_NUM 1
#define SERVO_MOTOR_TRAVEL_UP_DEN 10
#define SERVO_MOTOR_TRAVEL_DOWN 0
#define SERVO_MOTOR_TRAVEL_DOWN_NUM 0
#define SERVO_MOTOR_TRAVEL_DOWN_DEN 1
#define SERVO_MOTOR_MAX 2000
#define SERVO_MOTOR_MIN 1040

#define SERVO_AILEVON_LEFT 1
#define SERVO_AILEVON_LEFT_NEUTRAL 1543
#define SERVO_AILEVON_LEFT_TRAVEL_UP -0.0461458333333
#define SERVO_AILEVON_LEFT_TRAVEL_UP_NUM 443
#define SERVO_AILEVON_LEFT_TRAVEL_UP_DEN -9600
#define SERVO_AILEVON_LEFT_TRAVEL_DOWN -0.0371875
#define SERVO_AILEVON_LEFT_TRAVEL_DOWN_NUM 119
#define SERVO_AILEVON_LEFT_TRAVEL_DOWN_DEN -3200
#define SERVO_AILEVON_LEFT_MAX 1900
#define SERVO_AILEVON_LEFT_MIN 1100

#define SERVO_AILEVON_RIGHT 2
#define SERVO_AILEVON_RIGHT_NEUTRAL 1561
#define SERVO_AILEVON_RIGHT_TRAVEL_UP 0.0353125
#define SERVO_AILEVON_RIGHT_TRAVEL_UP_NUM 113
#define SERVO_AILEVON_RIGHT_TRAVEL_UP_DEN 3200
#define SERVO_AILEVON_RIGHT_TRAVEL_DOWN 0.0480208333333
#define SERVO_AILEVON_RIGHT_TRAVEL_DOWN_NUM 461
#define SERVO_AILEVON_RIGHT_TRAVEL_DOWN_DEN 9600
#define SERVO_AILEVON_RIGHT_MAX 1900
#define SERVO_AILEVON_RIGHT_MIN 1100


#define COMMAND_THROTTLE 0
#define COMMAND_ROLL 1
#define COMMAND_PITCH 2
#define COMMANDS_NB 3
#define COMMANDS_FAILSAFE {0,0,0}


#define SetCommandsFromRC(_commands_array, _rc_array) { \
  _commands_array[COMMAND_THROTTLE] = _rc_array[RADIO_THROTTLE];\
  _commands_array[COMMAND_ROLL] = _rc_array[RADIO_ROLL];\
  _commands_array[COMMAND_PITCH] = _rc_array[RADIO_PITCH];\
}

#define SECTION_MIXER 1
#define AILEVON_AILERON_RATE 0.75
#define AILEVON_ELEVATOR_RATE 0.75

#define SERVO_AILEVON_RIGHT_IDX 2
#define Set_AILEVON_RIGHT_Servo(_v) { \
  actuators[SERVO_AILEVON_RIGHT_IDX] = Chop(_v, SERVO_AILEVON_RIGHT_MIN, SERVO_AILEVON_RIGHT_MAX); \
  ActuatorDefaultSet(SERVO_AILEVON_RIGHT, actuators[SERVO_AILEVON_RIGHT_IDX]); \
}

#define SERVO_AILEVON_LEFT_IDX 1
#define Set_AILEVON_LEFT_Servo(_v) { \
  actuators[SERVO_AILEVON_LEFT_IDX] = Chop(_v, SERVO_AILEVON_LEFT_MIN, SERVO_AILEVON_LEFT_MAX); \
  ActuatorDefaultSet(SERVO_AILEVON_LEFT, actuators[SERVO_AILEVON_LEFT_IDX]); \
}

#define SERVO_MOTOR_IDX 0
#define Set_MOTOR_Servo(_v) { \
  actuators[SERVO_MOTOR_IDX] = Chop(_v, SERVO_MOTOR_MIN, SERVO_MOTOR_MAX); \
  ActuatorDefaultSet(SERVO_MOTOR, actuators[SERVO_MOTOR_IDX]); \
}

#define ACTUATORS_NB 3

#define AllActuatorsInit() { \
  ActuatorsDefaultInit();\
}

#define AllActuatorsCommit() { \
  ActuatorsDefaultCommit();\
}

#define SetActuatorsFromCommands(values, AP_MODE) { \
  int32_t servo_value;\
  int32_t command_value;\
\
  int16_t _var_aileron = values[COMMAND_ROLL]  * AILEVON_AILERON_RATE; \
  int16_t _var_elevator = values[COMMAND_PITCH] * AILEVON_ELEVATOR_RATE; \
  command_value = values[COMMAND_THROTTLE]; \
  command_value *= command_value>0 ? SERVO_MOTOR_TRAVEL_UP_NUM : SERVO_MOTOR_TRAVEL_DOWN_NUM; \
  command_value /= command_value>0 ? SERVO_MOTOR_TRAVEL_UP_DEN : SERVO_MOTOR_TRAVEL_DOWN_DEN; \
  servo_value = SERVO_MOTOR_NEUTRAL + command_value; \
  Set_MOTOR_Servo(servo_value); \
\
  command_value = _var_elevator - _var_aileron; \
  command_value *= command_value>0 ? SERVO_AILEVON_LEFT_TRAVEL_UP_NUM : SERVO_AILEVON_LEFT_TRAVEL_DOWN_NUM; \
  command_value /= command_value>0 ? SERVO_AILEVON_LEFT_TRAVEL_UP_DEN : SERVO_AILEVON_LEFT_TRAVEL_DOWN_DEN; \
  servo_value = SERVO_AILEVON_LEFT_NEUTRAL + command_value; \
  Set_AILEVON_LEFT_Servo(servo_value); \
\
  command_value = _var_elevator + _var_aileron; \
  command_value *= command_value>0 ? SERVO_AILEVON_RIGHT_TRAVEL_UP_NUM : SERVO_AILEVON_RIGHT_TRAVEL_DOWN_NUM; \
  command_value /= command_value>0 ? SERVO_AILEVON_RIGHT_TRAVEL_UP_DEN : SERVO_AILEVON_RIGHT_TRAVEL_DOWN_DEN; \
  servo_value = SERVO_AILEVON_RIGHT_NEUTRAL + command_value; \
  Set_AILEVON_RIGHT_Servo(servo_value); \
\
  AllActuatorsCommit(); \
}

#define SECTION_AUTO1 1
#define AUTO1_MAX_ROLL 0.85
#define AUTO1_MAX_PITCH 0.6

#define SECTION_IMU 1
#define IMU_GYRO_P_NEUTRAL 0
#define IMU_GYRO_Q_NEUTRAL 0
#define IMU_GYRO_R_NEUTRAL 0 
#define IMU_ACCEL_X_NEUTRAL 0
#define IMU_ACCEL_Y_NEUTRAL 0
#define IMU_ACCEL_Z_NEUTRAL 0
#define IMU_BODY_TO_IMU_PHI 0
#define IMU_BODY_TO_IMU_THETA 0
#define IMU_BODY_TO_IMU_PSI 0

#define SECTION_INS 1
#define INS_ROLL_NEUTRAL_DEFAULT 0.
#define INS_PITCH_NEUTRAL_DEFAULT 0.144

#define SECTION_BAT 1
#define CATASTROPHIC_BAT_LEVEL 9.3
#define MilliAmpereOfAdc(_adc) (_adc-158)*16.5698

#define SECTION_MISC 1
#define NOMINAL_AIRSPEED 15.
#define CARROT 5.
#define KILL_MODE_DISTANCE (1.5*MAX_DIST_FROM_HOME)
#define TRIGGER_DELAY 1.
#define DEFAULT_CIRCLE_RADIUS 80.
#define UNLOCKED_HOME_MODE TRUE

#define SECTION_VERTICAL CONTROL 1
#define V_CTL_ALTITUDE_PGAIN 0.18
#define V_CTL_ALTITUDE_MAX_CLIMB 5.
#define V_CTL_AUTO_THROTTLE_NOMINAL_CRUISE_THROTTLE 0.45
#define V_CTL_AUTO_THROTTLE_MIN_CRUISE_THROTTLE 0.25
#define V_CTL_AUTO_THROTTLE_MAX_CRUISE_THROTTLE 0.85
#define V_CTL_AUTO_PITCH_MAX_PITCH RadOfDeg(20.)
#define V_CTL_AUTO_PITCH_MIN_PITCH -RadOfDeg(20.)
#define V_CTL_AUTO_THROTTLE_CLIMB_THROTTLE_INCREMENT 0.1
#define V_CTL_AUTO_THROTTLE_PGAIN 0.002
#define V_CTL_AUTO_THROTTLE_DGAIN 0.0
#define V_CTL_AUTO_THROTTLE_IGAIN 0.004
#define V_CTL_AUTO_THROTTLE_PITCH_OF_VZ_PGAIN 0.03
#define V_CTL_AUTO_PITCH_PGAIN 0.04
#define V_CTL_AUTO_PITCH_DGAIN 0.04
#define V_CTL_AUTO_PITCH_IGAIN 0.0
#define V_CTL_AUTO_AIRSPEED_SETPOINT 16.
#define V_CTL_AUTO_AIRSPEED_THROTTLE_PGAIN 0.1
#define V_CTL_AUTO_AIRSPEED_THROTTLE_DGAIN 0.12
#define V_CTL_AUTO_AIRSPEED_THROTTLE_IGAIN 0.0
#define V_CTL_AUTO_AIRSPEED_PITCH_PGAIN 0.06
#define V_CTL_AUTO_AIRSPEED_PITCH_DGAIN 0.0
#define V_CTL_AUTO_AIRSPEED_PITCH_IGAIN 0.042
#define V_CTL_AIRSPEED_MAX 30
#define V_CTL_AIRSPEED_MIN 10
#define V_CTL_AUTO_GROUNDSPEED_SETPOINT 15
#define V_CTL_AUTO_GROUNDSPEED_PGAIN 1.
#define V_CTL_AUTO_GROUNDSPEED_IGAIN 0.
#define V_CTL_PITCH_LOITER_TRIM RadOfDeg(0.)
#define V_CTL_PITCH_DASH_TRIM RadOfDeg(-0.)
#define V_CTL_THROTTLE_SLEW 0.1

#define SECTION_HORIZONTAL CONTROL 1
#define H_CTL_COURSE_PGAIN 0.7
#define H_CTL_ROLL_MAX_SETPOINT 0.8
#define H_CTL_PITCH_MAX_SETPOINT 0.5
#define H_CTL_PITCH_MIN_SETPOINT -0.5
#define H_CTL_ROLL_ATTITUDE_GAIN 8800.
#define H_CTL_ROLL_RATE_GAIN 500.
#define H_CTL_ROLL_IGAIN 150.
#define H_CTL_ROLL_KFFA 0
#define H_CTL_ROLL_KFFD 0
#define H_CTL_PITCH_PGAIN 14000.
#define H_CTL_PITCH_DGAIN 0.
#define H_CTL_PITCH_IGAIN 250.
#define H_CTL_PITCH_KFFA 0.
#define H_CTL_PITCH_KFFD 0.
#define H_CTL_PITCH_OF_ROLL RadOfDeg(0.0)
#define H_CTL_AILERON_OF_THROTTLE 0.0
#define H_CTL_ELEVATOR_OF_ROLL 1400

#define SECTION_NAV 1
#define NAV_GLIDE_PITCH_TRIM 0

#define SECTION_FAILSAFE 1
#define FAILSAFE_DELAY_WITHOUT_GPS 2
#define FAILSAFE_DEFAULT_THROTTLE 0.3
#define FAILSAFE_DEFAULT_ROLL 0.3
#define FAILSAFE_DEFAULT_PITCH 0.5
#define FAILSAFE_HOME_RADIUS 100

#define SECTION_SIMU 1


#endif // AIRFRAME_H
