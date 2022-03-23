#pragma once

#include "ch.h"
#include "hal.h"


typedef enum {PID_AUTOMATIC=0b1, PID_MANUAL=0b10} PidMode;
typedef enum {PID_DIRECT=0b100, PID_REVERSE=0b1000} PidDirection;

typedef struct 
{
  float dispKp;				// * we'll hold on to the tuning parameters in user-entered 
  float dispKi;				//   format for display purposes
  float dispKd;				//
  
  float kp;                  // * (P)roportional Tuning Parameter
  float ki;                  // * (I)ntegral Tuning Parameter
  float kd;                  // * (D)erivative Tuning Parameter
  
  PidDirection controllerDirection;

  volatile float *myInput;              // * Pointers to the Input, Output, and Setpoint variables
  volatile float *myOutput;             //   This creates a hard link between the variables and the 
  volatile float *mySetpoint;           //   PID, freeing the user from having to constantly tell us
  //   what these values are.  with pointers we'll just know.
  
  volatile rtcnt_t  lastTime;
  volatile float ITerm, lastInput;
  
  volatile rtcnt_t SampleTime;
  volatile float outMin, outMax;
  bool inAuto;
} ControlerPid;

#ifdef __cplusplus
extern "C" {
#endif




  //commonly used functions **************************************************************************
void pidInit (ControlerPid* cpid,
	      volatile float* Input, volatile float* Output,
	      volatile float* Setpoint,
	      float Kp, float Ki, float Kd,
	      PidDirection ControllerDirection);

	
void pidSetMode (ControlerPid* cpid, PidMode Mode);               // * sets PID to either Manual (0) or Auto (non-0)

bool pidCompute(ControlerPid* cpid);                       // * performs the PID calculation.  it should be
//   called every time loop() cycles. ON/OFF and
//   calculation frequency can be set using SetMode
//   SetSampleTime respectively

void  pidSetOutputLimits(ControlerPid* cpid, float Min, float Max);
//clamps the output to a specific range. -100 .. 100  by default, but
//it's likely the user will want to change this depending on
//the application
void  pidSetOutputLimitsRatio(ControlerPid* cpid, float ratio);


//available but not commonly used functions ********************************************************
void  pidSetTunings(ControlerPid* cpid, float Kp, float Ki,       // * While most users will set the tunings once in the 
		    float Kd);         	  //   constructor, this function gives the user the option
                                          //   of changing tunings during runtime for Adaptive control
void  pidSetControllerDirection(ControlerPid* cpid, PidDirection Direction);	  // * Sets the Direction, or "Action" of the controller. DIRECT
//   means the output will increase when error is positive. REVERSE
//   means the opposite.  it's very unlikely that this will be needed
//   once it is set in the constructor.




//Display functions ****************************************************************
float  pidGetKp(ControlerPid*);				  // These functions query the pid for interal values.
float  pidGetKi(ControlerPid*);				  //  they were created mainly for the pid front-end,
float  pidGetKd(ControlerPid*);				  // where it's important to know what is actually 
PidMode  pidGetMode(ControlerPid*);			  //  inside the PID.
PidDirection  pidGetDirection(ControlerPid*);		  //

void  pidInitialize(ControlerPid*);
rtcnt_t pidGetSampleTime (ControlerPid* cpid);

#ifdef __cplusplus
}
#endif


