/*
 * Copyright (C) 2008-2012 The Paparazzi Team
 *
 * This file is part of paparazzi.
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Bibliography : "Design and implementation of low-cost observer-based attitude and heading reference system"
 *                 Philippe Martin, Erwan Salaun.
 *                 Control Engineering Pactrice 2010.
 *
 * For more information, please send an email to "jp.condomines@gmail.com"
*/
#ifndef AHRS_FLOAT_INVARIANT_FILTER_H
#define AHRS_FLOAT_INVARIANT_FILTER_H

#include "subsystems/ahrs.h"


struct invariant_state  {
  struct FloatQuat x_quat;/* Estimated attitude */ 
  struct FloatRates x_bias;/* Estimated gyro biases */ 
  float x_as;             /* Estimated accelerometer biases */ 
  float x_cs;             /* Estimated magnetic biases */  
};

struct state_dot  {
  struct FloatQuat x_qdot;    /* Time derivative of quaternion */
  struct FloatRates x_bias_dot;/* Time derivative of rates */  
  float x_asdot;              /* Time derivative of accelerometer biases */
  float x_csdot;              /* Time derivative of magnetometer biases */ 
};

struct measures {
  struct FloatVect3 accelerometers;
  struct FloatVect3 magnetometers;
};

struct gain_matrix {
  struct FloatVect3 LE;
  struct FloatVect3 ME;
  float NE;
  float OE;
};

extern struct FloatRates x_rates;
extern struct invariant_state x_state; 
extern struct state_dot x_dot; 
extern struct measures z;
extern struct gain_matrix mat; 

struct AhrsFloatInv {
  struct FloatRates gyro_bias;
  struct FloatRates rate_correction;

  struct FloatQuat ltp_to_imu_quat;
  struct FloatEulers ltp_to_imu_euler;
  struct FloatRMat body_to_imu_rmat;
  struct FloatRates imu_rate;
};

extern struct AhrsFloatInv ahrs_impl;

#endif /* AHRS_FLOAT_INVARIANT_FILTER_H */ 
