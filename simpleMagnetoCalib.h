#pragma once

#include "ch.h"
#include "stdutil.h"

typedef struct {
  struct {
    float maxVal;
    float minVal;
    float offset;
    float scale;
  } v[3];
} MagCalibCoeffs;


void magCalibInit(MagCalibCoeffs *cal);
void magCalibAddMeasure(MagCalibCoeffs *cal, const Vec3f *v);
void magCalibApplyCalib(const MagCalibCoeffs *cal, const Vec3f *vin, Vec3f *vout);
