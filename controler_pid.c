/**********************************************************************************************
 * portage of Arduino PID Library - Version 1.1.1 TO C language, chibios 2.6 RTOS
 * by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
 *
 * This Library is licensed under a GPLv3 License
 * portage by alexandre bustico <alexandre.bustico@enac.fr>
 **********************************************************************************************/


#include "controler_pid.h"
#include "ch.h"
#include "hal.h"
#include "stdutil.h"

static void  pidAdjustCoeffs (ControlerPid* cpid);


/*Constructor (...)*********************************************************
 *    The parameters specified here are those for which we can't set up 
 *    reliable defaults, so we need to have the user set them.
 ***************************************************************************/
void pidInit (ControlerPid* cpid, float* Input, float* Output, float* Setpoint,
        float Kp, float Ki, float Kd, PidDirection ControllerDirection)
{
	
    cpid->myOutput = Output;
    cpid->myInput = Input;
    cpid->mySetpoint = Setpoint;
    cpid->inAuto = false;
	
    pidSetOutputLimits(cpid, -100.0f, 100.0f);	//default output limit 
				
    pidSetControllerDirection(cpid, ControllerDirection);
    pidSetTunings(cpid, Kp, Ki, Kd);

    cpid->lastTime = halGetCounterValue();				
}
 
 
/* Compute() **********************************************************************
 *     This, as they say, is where the magic happens.  this function should be called
 *   every time "void loop()" executes.  the function will decide for itself whether a new
 *   pid Output needs to be computed.  returns true when the output is computed,
 *   false when nothing has been done.
 **********************************************************************************/ 
bool pidCompute(ControlerPid* cpid)
{
  if(!cpid->inAuto) return false;
  const halrtcnt_t now = halGetCounterValue();
  const halrtcnt_t timeChange = rtcntDiff (cpid->lastTime, now);
  cpid->SampleTime = timeChange;
  pidAdjustCoeffs (cpid);
  /*Compute all the working error variables*/
  const float input = *(cpid->myInput);
  const float error = *(cpid->mySetpoint) - input;
  cpid->ITerm += (cpid->ki * error);
  if (cpid->ITerm > cpid->outMax) {
    cpid->ITerm= cpid->outMax;
  } else if (cpid->ITerm < cpid->outMin) {
    cpid->ITerm= cpid->outMin;
  }
  const float dInput = (input - cpid->lastInput);
  
  /*Compute PID Output*/
  float output = cpid->kp * error + cpid->ITerm- cpid->kd * dInput;
  
  if(output > cpid->outMax) {
    output = cpid->outMax;
  } else if(output < cpid->outMin) {
    output = cpid->outMin;
  }
  *(cpid->myOutput) = output;
  
  /*Remember some variables for next time*/
  cpid->lastInput = input;
  cpid->lastTime = now;
  return true;
}



/* SetTunings(...)*************************************************************
 * This function allows the controller's dynamic performance to be adjusted. 
 * it's called automatically from the constructor, but tunings can also
 * be adjusted on the fly during normal operation
 ******************************************************************************/ 
void pidSetTunings(ControlerPid* cpid, float Kp, float Ki, float Kd)
{
   if (Kp<0.0f || Ki<0.0f || Kd<0.0f) return;
 
   cpid->dispKp = Kp; cpid->dispKi = Ki; cpid->dispKd = Kd;
   
   cpid->kp = Kp;
 
  if (cpid->controllerDirection ==PID_REVERSE)   {
    cpid->kp = (0.0f - cpid->kp);
  }
}


static void  pidAdjustCoeffs (ControlerPid* cpid)
{
  const float SampleTimeInSec = ((float)cpid->SampleTime) / halGetCounterFrequency() ;  
  cpid->ki = cpid->dispKi * SampleTimeInSec;
  cpid->kd = cpid->dispKd / SampleTimeInSec;
 
  if (cpid->controllerDirection == PID_REVERSE)   {
    cpid->ki = (0.0f - cpid->ki);
    cpid->kd = (0.0f - cpid->kd);
  }
}

  
/* SetSampleTime(...) *********************************************************
 * sets the period, in Milliseconds, at which the calculation is performed	
 ******************************************************************************/
 
/* SetOutputLimits(...)****************************************************
 *     This function will be used far more often than SetInputLimits.  while
 *  the input to the controller will generally be in the 0-1023 range (which is
 *  the default already,)  the output will be a little different.  maybe they'll
 *  be doing a time window and will need 0-8000 or something.  or maybe they'll
 *  want to clamp it from 0-125.  who knows.  at any rate, that can all be done
 *  here.
 **************************************************************************/
void pidSetOutputLimits(ControlerPid* cpid, float Min, float Max)
{
   if(Min >= Max) return;
   cpid->outMin = Min;
   cpid->outMax = Max;
 
   if(cpid->inAuto)   {
     if(*(cpid->myOutput) > cpid->outMax) {
       *(cpid->myOutput) = cpid->outMax;
     }   else if(*(cpid->myOutput) < cpid->outMin) {
       *(cpid->myOutput) = cpid->outMin;
     }
	 
     if(cpid->ITerm > cpid->outMax) {
       cpid->ITerm= cpid->outMax;
     } else if(cpid->ITerm < cpid->outMin) {
       cpid->ITerm= cpid->outMin;
     }
   }
}

/* SetMode(...)****************************************************************
 * Allows the controller Mode to be set to manual (0) or Automatic (non-zero)
 * when the transition from manual to auto occurs, the controller is
 * automatically initialized
 ******************************************************************************/ 
void pidSetMode(ControlerPid* cpid, PidMode Mode)
{
    bool newAuto = (Mode == PID_AUTOMATIC);
    if(newAuto == !cpid->inAuto) {
      /*we just went from manual to auto*/
      pidInitialize(cpid);
      cpid->inAuto = newAuto;
    }
}
 
/* Initialize()****************************************************************
 *	does all the things that need to happen to ensure a bumpless transfer
 *  from manual to automatic mode.
 ******************************************************************************/ 
void pidInitialize(ControlerPid* cpid)
{
  cpid->ITerm = *(cpid->myOutput);
  cpid->lastInput = *(cpid->myInput);
  if(cpid->ITerm > cpid->outMax) {
    cpid->ITerm = cpid->outMax;
  } else if(cpid->ITerm < cpid->outMin) {
    cpid->ITerm = cpid->outMin;
  }
}

/* SetControllerDirection(...)*************************************************
 * The PID will either be connected to a DIRECT acting process (+Output leads 
 * to +Input) or a REVERSE acting process(+Output leads to -Input.)  we need to
 * know which one, because otherwise we may increase the output when we should
 * be decreasing.  This is called from the constructor.
 ******************************************************************************/
void pidSetControllerDirection(ControlerPid* cpid, PidDirection Direction)
{
   if(cpid->inAuto && Direction !=cpid->controllerDirection)   {
     cpid->kp = (0.0f - cpid->kp);
     cpid->ki = (0.0f - cpid->ki);
     cpid->kd = (0.0f - cpid->kd);
   }   
   cpid->controllerDirection = Direction;
}

/* Status Funcions*************************************************************
 * Just because you set the Kp=-1 doesn't mean it actually happened.  these
 * functions query the internal state of the PID.  they're here for display 
 * purposes.  this are the functions the PID Front-end uses for example
 ******************************************************************************/
float pidGetKp(ControlerPid* cpid){ return  cpid->dispKp; }
float pidGetKi(ControlerPid* cpid){ return  cpid->dispKi;}
float pidGetKd(ControlerPid* cpid){ return  cpid->dispKd;}
PidMode pidGetMode(ControlerPid* cpid){ return  cpid->inAuto ? PID_AUTOMATIC : PID_MANUAL;}
PidDirection pidGetDirection(ControlerPid* cpid){ return cpid->controllerDirection;}
halrtcnt_t pidGetSampleTime (ControlerPid* cpid){ return cpid->SampleTime;}

