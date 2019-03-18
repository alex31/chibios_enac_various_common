#include "simpleMagnetoCalib.h"

void magCalibInit(MagCalibCoeffs *cal)
{
  for (size_t i=0; i<3; i++) {
    cal->v[i].maxVal = -1e6;
    cal->v[i].minVal = 1e6;
    cal->v[i].offset = 0.0f;
    cal->v[i].scale = 1.0f;
  }
}

void magCalibAddMeasure(MagCalibCoeffs *cal, const Vec3f *vec)
{
  for (size_t i=0; i<3; i++) {
    if (vec->v[i] < cal->v[i].minVal)
      cal->v[i].minVal = vec->v[i] ;
    if (vec->v[i] > cal->v[i].maxVal)
      cal->v[i].maxVal = vec->v[i] ;
    cal->v[i].offset = (cal->v[i].maxVal + cal->v[i].minVal) / 2.0f;
    cal->v[i].scale = (cal->v[i].maxVal - cal->v[i].minVal) / 2.0f;
  }

  const float averageScale = (cal->v[0].scale + cal->v[1].scale + cal->v[2].scale) / 3.0f;
  for (size_t i=0; i<3; i++)
    cal->v[i].scale = averageScale /  cal->v[i].scale;
}

void magCalibApplyCalib(const MagCalibCoeffs *cal, const Vec3f *vin, Vec3f *vout)
{
  for (size_t i=0; i<3; i++) {
    vout->v[i] = (vin->v[i] - cal->v[i].offset) * cal->v[i].scale;
  }
}
