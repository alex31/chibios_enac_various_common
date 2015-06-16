/*
 * Copyright (C) 2008-2014 The Paparazzi Team
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
 */

/**
 * @file pprz_algebra_double.h
 * @brief Paparazzi floating point algebra.
 *
 */

#ifndef PPRZ_ALGEBRA_DOUBLE_H
#define PPRZ_ALGEBRA_DOUBLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "algebra.h"

#include <math.h>
#include <float.h> // for FLT_MIN

/* this seems to be missing for some arch */
#ifndef M_SQRT2
#define M_SQRT2         1.41421356237309504880
#endif

struct DoubleVect2 {
  double x;
  double y;
};

struct DoubleVect3 {
  double x;
  double y;
  double z;
};

/**
 * @brief Roation quaternion
 */
struct DoubleQuat {
  double qi;
  double qx;
  double qy;
  double qz;
};

struct DoubleMat33 {
  double m[3 * 3];
};

/**
 * @brief rotation matrix
 */
struct DoubleRMat {
  double m[3 * 3];
};

/**
 * @brief euler angles
 * @details Units: radians */
struct DoubleEulers {
  double phi; ///< in radians
  double theta; ///< in radians
  double psi; ///< in radians
};

/**
 * @brief angular rates
 * @details Units: rad/s */
struct DoubleRates {
  double p; ///< in rad/s
  double q; ///< in rad/s
  double r; ///< in rad/s
};

#define DOUBLE_ANGLE_NORMALIZE(_a) {             \
    while (_a >  M_PI) _a -= (2.f*M_PI);             \
    while (_a < -M_PI) _a += (2.f*M_PI);             \
  }

//
//
// Vector algebra
//
//


/*
 * Dimension 2 Vectors
 */

#define DOUBLE_VECT2_ZERO(_v) VECT2_ASSIGN(_v, 0.0f, 0.0f)

/* macros also usable if _v is not a DoubleVect2, but a different struct with x,y members */
#define DOUBLE_VECT2_NORM(_v) sqrt(VECT2_NORM2(_v))

static inline double double_vect2_norm2(struct DoubleVect2* v)
{
  return v->x * v->x + v->y * v->y;
}

static inline double double_vect2_norm(struct DoubleVect2* v)
{
  return sqrt(double_vect2_norm2(v));
}

/** normalize 2D vector in place */
static inline void double_vect2_normalize(struct DoubleVect2* v)
{
  const double n = double_vect2_norm(v);
  if (n > 0) {
    v->x /= n;
    v->y /= n;
  }
}

#define DOUBLE_VECT2_NORMALIZE(_v) double_vect2_normalize(&(_v))


/*
 * Dimension 3 Vectors
 */

#define DOUBLE_VECT3_ZERO(_v) VECT3_ASSIGN(_v, 0.0f, 0.0f, 0.0f)

/* macros also usable if _v is not a DoubleVect3, but a different struct with x,y,z members */
#define DOUBLE_VECT3_NORM(_v) sqrt(VECT3_NORM2(_v))

static inline double double_vect3_norm2(struct DoubleVect3* v)
{
  return v->x * v->x + v->y * v->y + v->z * v->z;
}

static inline double double_vect3_norm(struct DoubleVect3* v)
{
  return sqrt(double_vect3_norm2(v));
}

/** normalize 3D vector in place */
static inline void double_vect3_normalize(struct DoubleVect3* v)
{
  const double n = double_vect3_norm(v);
  if (n > 0) {
    v->x /= n;
    v->y /= n;
    v->z /= n;
  }
}

#define DOUBLE_VECT3_NORMALIZE(_v) double_vect3_norm(&(_v))



#define DOUBLE_RATES_ZERO(_r) {          \
    RATES_ASSIGN(_r, 0.0f, 0.0f, 0.0f);       \
  }

#define DOUBLE_RATES_NORM(_v) (sqrt((_v).p*(_v).p + (_v).q*(_v).q + (_v).r*(_v).r))

#define DOUBLE_RATES_LIN_CMB(_ro, _r1, _s1, _r2, _s2) {          \
    _ro.p = _s1 * _r1.p + _s2 * _r2.p;                  \
    _ro.q = _s1 * _r1.q + _s2 * _r2.q;                  \
    _ro.r = _s1 * _r1.r + _s2 * _r2.r;                  \
  }


extern void double_vect3_integrate_fi(struct DoubleVect3* vec, struct DoubleVect3* dv,
                                     double dt);

extern void double_rates_integrate_fi(struct DoubleRates* r, struct DoubleRates* dr,
                                     double dt);

extern void double_rates_of_euler_dot(struct DoubleRates* r, struct DoubleEulers* e,
                                     struct DoubleEulers* edot);

/* defines for backwards compatibility */
#define DOUBLE_VECT3_INTEGRATE_FI(_vo, _dv, _dt) double_vect3_integrate_fi(&(_vo), &(_dv), _dt)
#define DOUBLE_RATES_INTEGRATE_FI(_ra, _racc, _dt) double_rates_integrate_fi(&(_ra), &(_racc), _dt)
#define DOUBLE_RATES_OF_EULER_DOT(_ra, _e, _ed) double_rates_of_euler_dot(&(_ra), &(_e), &(_ed))


/*
 * 3x3 matrices
 */
#define DOUBLE_MAT33_ZERO(_m) {                      \
    MAT33_ELMT(_m, 0, 0) = 0.0f;                      \
    MAT33_ELMT(_m, 0, 1) = 0.0f;                      \
    MAT33_ELMT(_m, 0, 2) = 0.0f;                      \
    MAT33_ELMT(_m, 1, 0) = 0.0f;                      \
    MAT33_ELMT(_m, 1, 1) = 0.0f;                      \
    MAT33_ELMT(_m, 1, 2) = 0.0f;                      \
    MAT33_ELMT(_m, 2, 0) = 0.0f;                      \
    MAT33_ELMT(_m, 2, 1) = 0.0f;                      \
    MAT33_ELMT(_m, 2, 2) = 0.0f;                      \
  }

#define DOUBLE_MAT33_DIAG(_m, _d00, _d11, _d22) {    \
    MAT33_ELMT(_m, 0, 0) = _d00;                    \
    MAT33_ELMT(_m, 0, 1) = 0.0f;                      \
    MAT33_ELMT(_m, 0, 2) = 0.0f;                      \
    MAT33_ELMT(_m, 1, 0) = 0.0f;                      \
    MAT33_ELMT(_m, 1, 1) = _d11;                    \
    MAT33_ELMT(_m, 1, 2) = 0.0f;                      \
    MAT33_ELMT(_m, 2, 0) = 0.0f;                      \
    MAT33_ELMT(_m, 2, 1) = 0.0f;                      \
    MAT33_ELMT(_m, 2, 2) = _d22;                    \
  }


//
//
// Rotation Matrices
//
//


/** initialises a rotation matrix to identity */
static inline void double_rmat_identity(struct DoubleRMat* rm)
{
  DOUBLE_MAT33_DIAG(*rm, 1.0f, 1.0f, 1.0f);
}

/** Inverse/transpose of a rotation matrix.
 * m_b2a = inv(_m_a2b) = transp(_m_a2b)
 */
extern void double_rmat_inv(struct DoubleRMat* m_b2a, struct DoubleRMat* m_a2b);

/** Composition (multiplication) of two rotation matrices.
 * m_a2c = m_a2b comp m_b2c , aka  m_a2c = m_b2c * m_a2b
 */
extern void double_rmat_comp(struct DoubleRMat* m_a2c, struct DoubleRMat* m_a2b,
                            struct DoubleRMat* m_b2c);

/** Composition (multiplication) of two rotation matrices.
 * m_a2b = m_a2c comp_inv m_b2c , aka  m_a2b = inv(_m_b2c) * m_a2c
 */
extern void double_rmat_comp_inv(struct DoubleRMat* m_a2b, struct DoubleRMat* m_a2c,
                                struct DoubleRMat* m_b2c);

/// Norm of a rotation matrix.
extern double double_rmat_norm(struct DoubleRMat* rm);

/** rotate 3D vector by rotation matrix.
 * vb = m_a2b * va
 */
extern void double_rmat_vmult(struct DoubleVect3* vb, struct DoubleRMat* m_a2b,
                             struct DoubleVect3* va);

/** rotate 3D vector by transposed rotation matrix.
 * vb = m_b2a^T * va
 */
extern void double_rmat_transp_vmult(struct DoubleVect3* vb, struct DoubleRMat* m_b2a,
                                    struct DoubleVect3* va);

/** rotate anglular rates by rotation matrix.
 * rb = m_a2b * ra
 */
extern void double_rmat_ratemult(struct DoubleRates* rb, struct DoubleRMat* m_a2b,
                                struct DoubleRates* ra);

/** rotate anglular rates by transposed rotation matrix.
 * rb = m_b2a^T * ra
 */
extern void double_rmat_transp_ratemult(struct DoubleRates* rb, struct DoubleRMat* m_b2a,
                                       struct DoubleRates* ra);

/** initialises a rotation matrix from unit vector axis and angle */
extern void double_rmat_of_axis_angle(struct DoubleRMat* rm, struct DoubleVect3* uv, double angle);

extern void double_rmat_of_eulers_321(struct DoubleRMat* rm, struct DoubleEulers* e);
extern void double_rmat_of_eulers_312(struct DoubleRMat* rm, struct DoubleEulers* e);
#define double_rmat_of_eulers double_rmat_of_eulers_321

extern void double_rmat_of_quat(struct DoubleRMat* rm, struct DoubleQuat* q);
/** in place first order integration of a rotation matrix */
extern void double_rmat_integrate_fi(struct DoubleRMat* rm, struct DoubleRates* omega, double dt);
extern double double_rmat_reorthogonalize(struct DoubleRMat* rm);

/* defines for backwards compatibility */
#define DOUBLE_RMAT_INV(_m_b2a, _m_a2b) double_rmat_inv(&(_m_b2a), &(_m_a2b))
#define DOUBLE_RMAT_NORM(_m) double_rmat_norm(&(_m))
#define DOUBLE_RMAT_COMP(_m_a2c, _m_a2b, _m_b2c) double_rmat_comp(&(_m_a2c), &(_m_a2b), &(_m_b2c))
#define DOUBLE_RMAT_COMP_INV(_m_a2b, _m_a2c, _m_b2c) double_rmat_comp_inv(&(_m_a2b), &(_m_a2c), &(_m_b2c))
#define DOUBLE_RMAT_VMULT(_vb, _m_a2b, _va) double_rmat_vmult(&(_vb), &(_m_a2b), &(_va))
#define DOUBLE_RMAT_TRANSP_VMULT(_vb, _m_b2a, _va) double_rmat_transp_vmult(&(_vb), &(_m_b2a), &(_va))
#define DOUBLE_RMAT_RATEMULT(_rb, _m_a2b, _ra) double_rmat_ratemult(&(_rb), &(_m_a2b), &(_ra))
#define DOUBLE_RMAT_TRANSP_RATEMULT(_rb, _m_b2a, _ra) double_rmat_ratemult(&(_rb), &(_m_b2a), &(_ra))
#define DOUBLE_RMAT_OF_AXIS_ANGLE(_rm, _uv, _an) double_rmat_of_axis_angle(&(_rm), &(_uv), _an)
#define DOUBLE_RMAT_OF_EULERS(_rm, _e)     double_rmat_of_eulers_321(&(_rm), &(_e))
#define DOUBLE_RMAT_OF_EULERS_321(_rm, _e) double_rmat_of_eulers_321(&(_rm), &(_e))
#define DOUBLE_RMAT_OF_EULERS_312(_rm, _e) double_rmat_of_eulers_312(&(_rm), &(_e))
#define DOUBLE_RMAT_OF_QUAT(_rm, _q)       double_rmat_of_quat(&(_rm), &(_q))
#define DOUBLE_RMAT_INTEGRATE_FI(_rm, _omega, _dt) double_rmat_integrate_fi(&(_rm), &(_omega), &(_dt))



//
//
// Quaternion algebras
//
//

/** initialises a quaternion to identity */
static inline void double_quat_identity(struct DoubleQuat* q)
{
  q->qi = 1.0f;
  q->qx = 0;
  q->qy = 0;
  q->qz = 0;
}

#define DOUBLE_QUAT_NORM2(_q) (SQUARE((_q).qi) + SQUARE((_q).qx) + SQUARE((_q).qy) + SQUARE((_q).qz))

static inline double double_quat_norm(struct DoubleQuat* q)
{
  return sqrt(SQUARE(q->qi) + SQUARE(q->qx) +  SQUARE(q->qy) + SQUARE(q->qz));
}

static inline void double_quat_normalize(struct DoubleQuat* q)
{
  double qnorm = double_quat_norm(q);
  if (qnorm > FLT_MIN) {
    q->qi = q->qi / qnorm;
    q->qx = q->qx / qnorm;
    q->qy = q->qy / qnorm;
    q->qz = q->qz / qnorm;
  }
}

static inline void double_quat_invert(struct DoubleQuat* qo, struct DoubleQuat* qi)
{
  QUAT_INVERT(*qo, *qi);
}

static inline void double_quat_wrap_shortest(struct DoubleQuat* q)
{
  if (q->qi < 0.0f) {
    QUAT_EXPLEMENTARY(*q, *q);
  }
}

#define DOUBLE_QUAT_EXTRACT(_vo, _qi) QUAT_EXTRACT_Q(_vo, _qi)


/** Composition (multiplication) of two quaternions.
 * a2c = a2b comp b2c , aka  a2c = a2b * b2c
 */
extern void double_quat_comp(struct DoubleQuat* a2c, struct DoubleQuat* a2b, struct DoubleQuat* b2c);

/** Composition (multiplication) of two quaternions.
 * a2b = a2c comp_inv b2c , aka  a2b = a2c * inv(b2c)
 */
extern void double_quat_comp_inv(struct DoubleQuat* a2b, struct DoubleQuat* a2c, struct DoubleQuat* b2c);

/** Composition (multiplication) of two quaternions.
 * b2c = a2b inv_comp a2c , aka  b2c = inv(_a2b) * a2c
 */
extern void double_quat_inv_comp(struct DoubleQuat* b2c, struct DoubleQuat* a2b, struct DoubleQuat* a2c);

/** Composition (multiplication) of two quaternions with normalization.
 * a2c = a2b comp b2c , aka  a2c = a2b * b2c
 */
extern void double_quat_comp_norm_shortest(struct DoubleQuat* a2c, struct DoubleQuat* a2b, struct DoubleQuat* b2c);

/** Composition (multiplication) of two quaternions with normalization.
 * a2b = a2c comp_inv b2c , aka  a2b = a2c * inv(b2c)
 */
extern void double_quat_comp_inv_norm_shortest(struct DoubleQuat* a2b, struct DoubleQuat* a2c, struct DoubleQuat* b2c);

/** Composition (multiplication) of two quaternions with normalization.
 * b2c = a2b inv_comp a2c , aka  b2c = inv(_a2b) * a2c
 */
extern void double_quat_inv_comp_norm_shortest(struct DoubleQuat* b2c, struct DoubleQuat* a2b, struct DoubleQuat* a2c);

/** Quaternion derivative from rotational velocity.
 * qd = -0.5f*omega(r) * q
 * or equally:
 * qd = 0.5f * q * omega(r)
 */
extern void double_quat_derivative(struct DoubleQuat* qd, struct DoubleRates* r, struct DoubleQuat* q);

/** Quaternion derivative from rotational velocity with Lagrange multiplier.
 * qd = -0.5f*omega(r) * q
 * or equally:
 * qd = 0.5f * q * omega(r)
 */
extern void double_quat_derivative_lagrange(struct DoubleQuat* qd, struct DoubleRates* r, struct DoubleQuat* q);

/** Delta rotation quaternion with constant angular rates.
 */
extern void double_quat_differential(struct DoubleQuat* q_out, struct DoubleRates* w, double dt);

/** in place first order quaternion integration with constant rotational velocity */
extern void double_quat_integrate_fi(struct DoubleQuat* q, struct DoubleRates* omega, double dt);

/** in place quaternion integration with constant rotational velocity */
extern void double_quat_integrate(struct DoubleQuat* q, struct DoubleRates* omega, double dt);

/** rotate 3D vector by quaternion.
 * vb = q_a2b * va * q_a2b^-1
 */
extern void double_quat_vmult(struct DoubleVect3* v_out, struct DoubleQuat* q, struct DoubleVect3* v_in);

/// Quaternion from Euler angles.
extern void double_quat_of_eulers(struct DoubleQuat* q, struct DoubleEulers* e);

/// Quaternion from unit vector and angle.
extern void double_quat_of_axis_angle(struct DoubleQuat* q, const struct DoubleVect3* uv, double angle);

/** Quaternion from orientation vector.
 * Length/norm of the vector is the angle.
 */
extern void double_quat_of_orientation_vect(struct DoubleQuat* q, const struct DoubleVect3* ov);

/// Quaternion from rotation matrix.
extern void double_quat_of_rmat(struct DoubleQuat* q, struct DoubleRMat* rm);


/* defines for backwards compatibility */
#define DOUBLE_QUAT_ZERO(_q) double_quat_identity(&(_q))
#define DOUBLE_QUAT_INVERT(_qo, _qi) double_quat_invert(&(_qo), &(_qi))
#define DOUBLE_QUAT_WRAP_SHORTEST(_q) double_quat_wrap_shortest(&(_q))
#define DOUBLE_QUAT_NORM(_q) double_quat_norm(&(_q))
#define DOUBLE_QUAT_NORMALIZE(_q) double_quat_normalize(&(_q))
#define DOUBLE_QUAT_COMP(_a2c, _a2b, _b2c) double_quat_comp(&(_a2c), &(_a2b), &(_b2c))
#define DOUBLE_QUAT_MULT(_a2c, _a2b, _b2c) double_quat_comp(&(_a2c), &(_a2b), &(_b2c))
#define DOUBLE_QUAT_INV_COMP(_b2c, _a2b, _a2c) double_quat_inv_comp(&(_b2c), &(_a2b), &(_a2c))
#define DOUBLE_QUAT_COMP_INV(_a2b, _a2c, _b2c) double_quat_comp_inv(&(_a2b), &(_a2c), &(_b2c))
#define DOUBLE_QUAT_COMP_NORM_SHORTEST(_a2c, _a2b, _b2c) double_quat_comp_norm_shortest(&(_a2c), &(_a2b), &(_b2c))
#define DOUBLE_QUAT_COMP_INV_NORM_SHORTEST(_a2b, _a2c, _b2c) double_quat_comp_inv_norm_shortest(&(_a2b), &(_a2c), &(_b2c))
#define DOUBLE_QUAT_INV_COMP_NORM_SHORTEST(_b2c, _a2b, _a2c) double_quat_inv_comp_norm_shortest(&(_b2c), &(_a2b), &(_a2c))
#define DOUBLE_QUAT_DIFFERENTIAL(q_out, w, dt) double_quat_differential(&(q_out), &(w), dt)
#define DOUBLE_QUAT_INTEGRATE(_q, _omega, _dt) double_quat_integrate(&(_q), &(_omega), _dt)
#define DOUBLE_QUAT_VMULT(v_out, q, v_in) double_quat_vmult(&(v_out), &(q), &(v_in))
#define DOUBLE_QUAT_DERIVATIVE(_qd, _r, _q) double_quat_derivative(&(_qd), &(_r), &(_q))
#define DOUBLE_QUAT_DERIVATIVE_LAGRANGE(_qd, _r, _q) double_quat_derivative_lagrange(&(_qd), &(_r), &(_q))
#define DOUBLE_QUAT_OF_EULERS(_q, _e) double_quat_of_eulers(&(_q), &(_e))
#define DOUBLE_QUAT_OF_AXIS_ANGLE(_q, _uv, _an) double_quat_of_axis_angle(&(_q), &(_uv), _an)
#define DOUBLE_QUAT_OF_ORIENTATION_VECT(_q, _ov) double_quat_of_orientation_vect(&(_q), &(_ov))
#define DOUBLE_QUAT_OF_RMAT(_q, _r) double_quat_of_rmat(&(_q), &(_r))



//
//
// Euler angles
//
//

#define DOUBLE_EULERS_ZERO(_e) EULERS_ASSIGN(_e, 0.0f, 0.0f, 0.0f);

static inline double double_eulers_norm(struct DoubleEulers* e)
{
  return sqrt(SQUARE(e->phi) + SQUARE(e->theta) + SQUARE(e->psi));
}
extern void double_eulers_of_rmat(struct DoubleEulers* e, struct DoubleRMat* rm);
extern void double_eulers_of_quat(struct DoubleEulers* e, struct DoubleQuat* q);

/* defines for backwards compatibility */
#define DOUBLE_EULERS_OF_RMAT(_e, _rm) double_eulers_of_rmat(&(_e), &(_rm))
#define DOUBLE_EULERS_OF_QUAT(_e, _q) double_eulers_of_quat(&(_e), &(_q))
#define DOUBLE_EULERS_NORM(_e) double_eulers_norm(&(_e))

//
//
// Generic vector algebra
//
//

/** a = 0 */
static inline void double_vect_zero(double* a, const int n)
{
  int i;
  for (i = 0; i < n; i++) { a[i] = 0.0f; }
}

/** a = b */
static inline void double_vect_copy(double* a, const double* b, const int n)
{
  int i;
  for (i = 0; i < n; i++) { a[i] = b[i]; }
}

/** o = a + b */
static inline void double_vect_sum(double* o, const double* a, const double* b, const int n)
{
  int i;
  for (i = 0; i < n; i++) { o[i] = a[i] + b[i]; }
}

/** o = a - b */
static inline void double_vect_diff(double* o, const double* a, const double* b, const int n)
{
  int i;
  for (i = 0; i < n; i++) { o[i] = a[i] - b[i]; }
}

/** o = a * b (element wise) */
static inline void double_vect_mul(double* o, const double* a, const double* b, const int n)
{
  int i;
  for (i = 0; i < n; i++) { o[i] = a[i] * b[i]; }
}

/** a += b */
static inline void double_vect_add(double* a, const double* b, const int n)
{
  int i;
  for (i = 0; i < n; i++) { a[i] += b[i]; }
}

/** a -= b */
static inline void double_vect_sub(double* a, const double* b, const int n)
{
  int i;
  for (i = 0; i < n; i++) { a[i] -= b[i]; }
}

/** o = a * s */
static inline void double_vect_smul(double* o, const double* a, const double s, const int n)
{
  int i;
  for (i = 0; i < n; i++) { o[i] = a[i] * s; }
}

/** o = a / s */
static inline void double_vect_sdiv(double* o, const double* a, const double s, const int n)
{
  int i;
  if (fabs(s) > 1e-5f) {
    for (i = 0; i < n; i++) { o[i] = a[i] / s; }
  }
}

/** ||a|| */
static inline double double_vect_norm(const double* a, const int n)
{
  int i;
  double sum = 0;
  for (i = 0; i < n; i++) { sum += a[i] * a[i]; }
  return sqrt(sum);
}

//
//
// Generic matrix algebra
//
//

/** Make a pointer to a matrix of _rows lines */
#define MAKE_MATRIX_PTR(_ptr, _mat, _rows) \
  double * _ptr[_rows]; \
  { \
    int i; \
    for (i = 0; i < _rows; i++) { _ptr[i] = &_mat[i][0]; } \
  }

/** a = 0 */
static inline void double_mat_zero(double** a, int m, int n)
{
  int i, j;
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) { a[i][j] = 0.0f; }
  }
}

/** a = b */
static inline void double_mat_copy(double** a, double** b, int m, int n)
{
  int i, j;
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) { a[i][j] = b[i][j]; }
  }
}

/** o = a + b */
static inline void double_mat_sum(double** o, double** a, double** b, int m, int n)
{
  int i, j;
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) { o[i][j] = a[i][j] + b[i][j]; }
  }
}

/** o = a - b */
static inline void double_mat_diff(double** o, double** a, double** b, int m, int n)
{
  int i, j;
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) { o[i][j] = a[i][j] - b[i][j]; }
  }
}

/** transpose square matrix */
static inline void double_mat_transpose(double** a, int n)
{
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < i; j++) {
      double t = a[i][j];
      a[i][j] = a[j][i];
      a[j][i] = t;
    }
  }
}

/** o = a * b
 *
 * a: [m x n]
 * b: [n x l]
 * o: [m x l]
 */
static inline void double_mat_mul(double** o, double** a, double** b, int m, int n, int l)
{
  int i, j, k;
  for (i = 0; i < m; i++) {
    for (j = 0; j < l; j++) {
      o[i][j] = 0.0f;
      for (k = 0; k < n; k++) {
        o[i][j] += a[i][k] * b[k][j];
      }
    }
  }
}

/** matrix minor
 *
 * a: [m x n]
 * o: [I(d,d)     0     ]
 *    [  0    a(d,m:d,n)]
 */
static inline void double_mat_minor(double** o, double** a, int m, int n, int d)
{
  int i, j;
  double_mat_zero(o, m, n);
  for (i = 0; i < d; i++) { o[i][i] = 1.0f; }
  for (i = d; i < m; i++) {
    for (j = d; j < n; j++) {
      o[i][j] = a[i][j];
    }
  }
}

/** o = I - v v^T */
static inline void double_mat_vmul(double** o, double* v, int n)
{
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      o[i][j] = -2.f *  v[i] * v[j];
    }
  }
  for (i = 0; i < n; i++) {
    o[i][i] += 1.f;
  }
}

/** o = c-th column of matrix a[m x n] */
static inline void double_mat_col(double* o, double** a, int m, int c)
{
  int i;
  for (i = 0; i < m; i++) {
    o[i] = a[i][c];
  }
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PPRZ_ALGEBRA_DOUBLE_H */
