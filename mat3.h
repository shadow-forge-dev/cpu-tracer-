#ifndef MAT3_H
#define MAT3_H

#include "types.h"
#include "vec3.h"

static inline Mat3 mat3_identity() {
    return (Mat3){{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
}

Vec3 mat3_mul_vec3(Mat3 m, Vec3 v);
Mat3 mat3_mul(Mat3 a, Mat3 b);
Mat3 mat3_transpose(Mat3 m);
Mat3 mat3_from_axis_angle(Vec3 axis, float angle);

#endif
