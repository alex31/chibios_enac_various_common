/* This file is part of paparazzi.
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
 *@author Jean-Philippe Condomines  <jp.condomines@gmail.com>
 */
#include "subsystems/ahrs/ahrs_int_utils.h"
#include "subsystems/ahrs/ahrs_aligner.h"
#include "subsystems/ahrs.h"
#include "state.h"

#include "math/pprz_algebra_float.h"
#include "math/pprz_algebra_int.h"
#include "math/pprz_algebra.h"

#include "subsystems/imu.h"
#include "generated/airframe.h"

#include "led.h"

#include "mcu_periph/uart.h"
#include "messages.h"
#include "subsystems/datalink/downlink.h"


#ifndef DOWNLINK_DEVICE
#define DOWNLINK_DEVICE DOWNLINK_AP_DEVICE
#endif


/*---------Invariant Observers-----------
 *
 *            State vector :
 *  x = [q0 q1 q2 q3 wb1 wb2 wb3 as cs]
 *
 *
 *            Dynamic model (dim = 9) :
 *  x_qdot     = 0.5 * x_quat * ( x_rates - x_bias );
 *  x_bias_dot = 0;
 *  x_asdot    = 0;
 *  x_csdot    = 0;
 *
 *            Observation model (dim = 6) :
 *  ya = as * (q)-1 * A * q; (A : accelerometers)
 *  yc = cs * (q)-1 * C * q; (C = A x B (cross product), B : magnetometers)
 *
 *------------------------------------------*/

#ifdef INS_ROLL_NEUTRAL_DEFAULT
float ins_roll_neutral = INS_ROLL_NEUTRAL_DEFAULT;
#endif
#ifdef INS_PITCH_NEUTRAL_DEFAULT
float ins_pitch_neutral = INS_PITCH_NEUTRAL_DEFAULT;
#endif


struct AhrsFloatInv ahrs_impl;

/*  Specific structure */

struct FloatRates x_rates;

struct invariant_state x_state;
struct state_dot x_dot;
struct measures z_m;
struct gain_matrix mat;

bool_t new_mesures_accel = FALSE;
bool_t new_mesures_mag = FALSE;

static inline void ahrs_error_output(struct invariant_state *_x_state, struct measures *_z_m, struct gain_matrix *_mat);
static inline void ahrs_runge_kutta4(struct invariant_state *_x_state, struct FloatRates *_x_rates, struct gain_matrix *_mat);
static inline void ahrs_invariant_model(struct state_dot *_x_dot, struct invariant_state *_x_state, struct FloatRates *_x_rates, struct gain_matrix *_mat);

static inline void copy_dot_to_state(struct invariant_state *p_st1,struct state_dot *p_st2);
static inline void copy_struct_state(struct state_dot *p_st1,struct state_dot *p_st2);
static inline void mul_struct_state_scale(struct  state_dot *st1, float s);
static inline void add_struct_state(struct state_dot *st1, struct invariant_state *st2);
static inline void add_struct_dot(struct state_dot *p_st1,struct state_dot *p_st2);
static inline void div_struct_state_scale(struct state_dot *st1, float s);


void ahrs_init(void) {

  ahrs.status = AHRS_UNINIT;

  FLOAT_RATES_ZERO(ahrs_impl.imu_rate);
  FLOAT_RATES_ZERO(ahrs_impl.gyro_bias);
  FLOAT_RATES_ZERO(ahrs_impl.rate_correction);

 // FLOAT_RATES_ZERO(x_rates);
  FLOAT_RATES_ZERO(x_state.x_bias);
  x_state.x_as = 1.;
  x_state.x_cs = 1.;

}
void ahrs_align(void)
{
  /* Compute an initial orientation from accel and mag directly as quaternion */
  ahrs_float_get_quat_from_accel_mag(&ahrs_impl.ltp_to_imu_quat, &ahrs_aligner.lp_accel, &ahrs_aligner.lp_mag);

  //FLOAT_QUAT_ASSIGN(x_state.x_quat,1., 0., 0., 0.);

  FLOAT_QUAT_COPY(x_state.x_quat,ahrs_impl.ltp_to_imu_quat);

  /*set_body_state_from_quat();*/

  /* use average gyro as initial value for bias */
  struct FloatRates bias0;
  FLOAT_RATES_COPY(bias0, ahrs_aligner.lp_gyro);
  RATES_FLOAT_OF_BFP(ahrs_impl.gyro_bias, bias0);

  RATES_FLOAT_OF_BFP(x_state.x_bias, bias0);

  ahrs.status = AHRS_RUNNING;
}

void ahrs_propagate(void) {

  struct FloatQuat ned_to_body_quat;
  struct FloatRates body_rate;
  struct FloatEulers angles;

 //  RATES_FLOAT_OF_BFP(x_rates, imu.gyro);

 // if (new_mesures_accel && new_mesures_mag){

    //LED_TOGGLE(4);

    RATES_FLOAT_OF_BFP(x_rates, imu.gyro);

    ahrs_error_output(&x_state, &z_m, &mat);

//DOWNLINK_SEND_INV_FILTER(DefaultChannel, DefaultDevice, &x_state.x_quat.qi, &angles.phi, &angles.theta, &angles.psi, &x_state.x_bias.p, &x_state.x_bias.q, &x_state.x_bias.r, &x_state.x_as, &x_state.x_cs);

    ahrs_runge_kutta4(&x_state, &x_rates, &mat);

    FLOAT_QUAT_NORMALIZE(x_state.x_quat);

    new_mesures_accel = FALSE;
    new_mesures_mag = FALSE;


    ned_to_body_quat = x_state.x_quat;
    stateSetNedToBodyQuat_f(&ned_to_body_quat);

    body_rate = x_state.x_bias;
    stateSetBodyRates_f(&body_rate);

    FLOAT_EULERS_OF_QUAT(angles,x_state.x_quat);

    RunOnceEvery(10,{
   DOWNLINK_SEND_INV_FILTER(DefaultChannel, DefaultDevice, &x_state.x_quat.qi, &angles.phi, &angles.theta, &angles.psi, &x_state.x_bias.p, &x_state.x_bias.q, &x_state.x_bias.r, &x_state.x_as, &x_state.x_cs);
   });
  //}
}

void ahrs_update_gps(void) {
}

void ahrs_update_accel(void){
  ACCELS_FLOAT_OF_BFP(z_m.accelerometers, imu.accel);

  z_m.accelerometers.x = -z_m.accelerometers.x;
  z_m.accelerometers.y = -z_m.accelerometers.y;
  z_m.accelerometers.z = -z_m.accelerometers.z;

  //LED_TOGGLE(1);
  new_mesures_accel = TRUE;
}

void ahrs_update_mag(void){

  //LED_TOGGLE(2);
  MAGS_FLOAT_OF_BFP(z_m.magnetometers, imu.mag);
  new_mesures_mag = TRUE;
}


/* brief :                                       */
/* x_dot = Dynamic model + (gain_matrix * error) */

static inline void ahrs_invariant_model(struct state_dot *_x_dot, struct invariant_state *_x_state, struct FloatRates *_x_rates, struct gain_matrix *_mat){

  struct FloatVect3 tt;
  struct state_dot tt2;

  /* dot_q = 0.5 * q * (x_rates - x_bias) + LE * q */
  FLOAT_RATES_SUB(*_x_rates, _x_state->x_bias);

  tt.x = _x_rates->p;
  tt.y = _x_rates->q;
  tt.z = _x_rates->r;

  FLOAT_QUAT_VMUL_LEFT(_x_dot->x_qdot, _x_state->x_quat, tt);
  FLOAT_QUAT_SMUL(_x_dot->x_qdot, _x_dot->x_qdot, 0.5);

  FLOAT_QUAT_VMUL_RIGHT(tt2.x_qdot, _x_state->x_quat, _mat->LE);
  FLOAT_QUAT_ADD(_x_dot->x_qdot, tt2.x_qdot);

  /* x_bias_dot = q-1 * (ME) * q */

  tt.x = _x_dot->x_bias_dot.p;
  tt.y = _x_dot->x_bias_dot.q;
  tt.z = _x_dot->x_bias_dot.r;

  FLOAT_QUAT_RMAT_N2B(tt, _x_state->x_quat, _mat->ME);

  _x_dot->x_bias_dot.p = tt.x;
  _x_dot->x_bias_dot.q = tt.y;
  _x_dot->x_bias_dot.r = tt.z;


  /* as_dot = x_as * NE */
  _x_dot->x_asdot = (_x_state->x_as) * (_mat->NE);

  /* cs_dot = x_cs * OE */
  _x_dot->x_csdot = (_x_state->x_cs) * (_mat->OE);
}


  /* brief :                                  */
  /* E = ( ŷ - y )                            */
  /* LE, ME, NE, OE : ( gain matrix * error ) */
 static inline void ahrs_error_output(struct invariant_state *_x_state, struct measures *_z_m, struct gain_matrix *_mat) {

  struct FloatVect3 A, B, C, YAt, YCt;

  const float sigma = 0.05;
  const float g = 9.81;
  const float n = 0.25;
  const float o = 0.5;

  const float l1 = 2. * (0.06 + 0.1);
  const float l2 = 2. * (0.06 + 0.06);
  const float l3 = 2. * (0.1 + 0.06);
  const float m1 = 2. * sigma * (0.06 + 0.1);
  const float m2 = 2. * sigma * (0.06 + 0.06);
  const float m3 = 2. * sigma * (0.1 + 0.06);
  const float b1 = 1. / M_SQRT2;

  FLOAT_VECT3_ASSIGN(A, 0., 0., g);
  //FLOAT_VECT3_ASSIGN(B, (1./M_SQRT2), 0., (1./M_SQRT2)); /* following our magnetic earth model */
  FLOAT_VECT3_ASSIGN(B, AHRS_H_X, AHRS_H_Y, AHRS_H_Z); /* following our magnetic earth model */

  /* transformations of mesures : Yc = A x B */
  FLOAT_VECT3_CROSS_PRODUCT(C, A, B);
  FLOAT_VECT3_CROSS_PRODUCT(YCt, _z_m->accelerometers, _z_m->magnetometers);

  FLOAT_QUAT_RMAT_B2N(YAt, _x_state->x_quat, _z_m->accelerometers);
  FLOAT_QUAT_RMAT_B2N(YCt, _x_state->x_quat, YCt);

  FLOAT_VECT3_SMUL(YAt, YAt, 1. / (_x_state->x_as));
  FLOAT_VECT3_SMUL(YCt, YCt, 1. / (_x_state->x_cs));

  /* E = ( ŷ - y ) */
  FLOAT_VECT3_SUB(A, YAt);
  FLOAT_VECT3_SUB(C, YCt);

  FLOAT_VECT3_ASSIGN(_mat->LE, (-l1 * A.y) / (2. * g), (l2 * A.x) / (2. * g), (-l3 * C.x) / (b1 * 2. * g));
  FLOAT_VECT3_ASSIGN(_mat->ME, (m1 * A.y) / (2. * g), (-m2 * A.x)/(2. * g), (m3 * C.x) / (b1 * 2. * g));
  _mat->NE = (-n * A.z) / g;
  _mat->OE = (-o * C.y) / (b1 * g);
}

/* Fourth-Order Runge-Kutta :
  *
  * k1 = f(x, omega, LE, ME, NE, OE)
  * k2 = f(x + dt * (k1 / 2), omega, LE, ME, NE, OE)
  * k3 = f(x + dt * (k2 / 2), omega, LE, ME, NE, OE)
  * k4 = f(x + dt * k3, omega, LE, ME, NE, OE)
  * xo = x + (dt / 6) * (k1 + 2 * (k2 + k3) + k4)
 */
static inline void ahrs_runge_kutta4(struct invariant_state *_x_state, struct FloatRates *_x_rates, struct gain_matrix *_mat) {

  struct state_dot xtemp, k1, k2, k3, k4;
  struct invariant_state xtemp2;

  const float dt = (1./ ((float)AHRS_PROPAGATE_FREQUENCY));

  ahrs_invariant_model(&xtemp, _x_state, _x_rates, _mat);

    //copy_struct_state(&k1, &xtemp);/* backup  k1 */
    k1 = xtemp;

    div_struct_state_scale(&xtemp, 2.);
    mul_struct_state_scale(&xtemp, dt);
    add_struct_state(&xtemp, _x_state);

    copy_dot_to_state(&xtemp2, &xtemp);/* state_dot to invariant_state*/

  ahrs_invariant_model(&xtemp, &xtemp2, _x_rates, _mat);

    //copy_struct_state(&k2, &xtemp);/* backup k2 */
    k2 = xtemp;

    div_struct_state_scale(&xtemp, 2.);
    mul_struct_state_scale(&xtemp, dt);
    add_struct_state(&xtemp, _x_state);

    copy_dot_to_state(&xtemp2, &xtemp);/* state_dot to invariant_state*/

  ahrs_invariant_model(&xtemp, &xtemp2, _x_rates, _mat);

    //copy_struct_state(&k3, &xtemp);/* backup k3 */
    k3 = xtemp;

    mul_struct_state_scale(&xtemp, dt);
    add_struct_state(&xtemp, _x_state);

    copy_dot_to_state(&xtemp2, &xtemp);/* state_dot to invariant_state*/

  ahrs_invariant_model(&xtemp, &xtemp2, _x_rates, _mat);

    //copy_struct_state(&k4, &xtemp);/* backup k4 */
    k4 = xtemp;

    add_struct_dot(&k2, &k3);
    mul_struct_state_scale(&k2, 2.);
    add_struct_dot(&k2, &k4);   /* x + (dt / 6) * (k1 + 2 * (k2 + k3) + k4) */
    add_struct_dot(&k2, &k1);
    mul_struct_state_scale(&k2, (dt / 6.));
    add_struct_state(&k2, _x_state);

    copy_dot_to_state(_x_state, &k2);/* state_dot to invariant_state*/

}

static inline void copy_dot_to_state(struct invariant_state *p_st1,struct state_dot *p_st2){

  FLOAT_QUAT_COPY(p_st1->x_quat, p_st2->x_qdot);
  FLOAT_RATES_COPY(p_st1->x_bias, p_st2->x_bias_dot);
  p_st1->x_as = p_st2->x_asdot;
  p_st1->x_cs = p_st2->x_csdot;
}


static inline void copy_struct_state(struct state_dot *p_st1,struct state_dot *p_st2){

  FLOAT_QUAT_COPY(p_st1->x_qdot, p_st2->x_qdot);
  FLOAT_RATES_COPY(p_st1->x_bias_dot, p_st2->x_bias_dot);
  p_st1->x_asdot = p_st2->x_asdot;
  p_st1->x_csdot = p_st2->x_csdot;
}

static inline void mul_struct_state_scale(struct state_dot *p_st1,float s){

  FLOAT_QUAT_SMUL(p_st1->x_qdot, p_st1->x_qdot, s);
  FLOAT_RATES_SCALE(p_st1->x_bias_dot, s);
  p_st1->x_asdot = (p_st1->x_asdot) * s;
  p_st1->x_csdot = (p_st1->x_csdot) * s;
}

static inline void add_struct_state(struct state_dot *p_st1,struct invariant_state *p_st2) {

  FLOAT_QUAT_ADD(p_st1->x_qdot, p_st2->x_quat);
  FLOAT_RATES_ADD(p_st1->x_bias_dot, p_st2->x_bias);
  p_st1->x_asdot = (p_st1->x_asdot) + (p_st2->x_as);
  p_st1->x_csdot = (p_st1->x_csdot) + (p_st2->x_cs);
}

static inline void add_struct_dot(struct state_dot *p_st1,struct state_dot *p_st2) {

  FLOAT_QUAT_ADD(p_st1->x_qdot, p_st2->x_qdot);
  FLOAT_RATES_ADD(p_st1->x_bias_dot, p_st2->x_bias_dot);
  p_st1->x_asdot = (p_st1->x_asdot) + (p_st2->x_asdot);
  p_st1->x_csdot = (p_st1->x_csdot) + (p_st2->x_csdot);
}

static inline void div_struct_state_scale(struct state_dot *p_st1, float s) {

  FLOAT_QUAT_SDIV(p_st1->x_qdot, p_st1->x_qdot, s);
  FLOAT_RATES_SDIV(p_st1->x_bias_dot, p_st1->x_bias_dot, s);
  p_st1->x_asdot = (p_st1->x_asdot) / s;
  p_st1->x_csdot = (p_st1->x_csdot) / s;

}
