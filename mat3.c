#include "mat3.h"

Vec3 mat3_mul_vec3(Mat3 m, Vec3 v) {
    return (Vec3){
        m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z,
        m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z,
        m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z
    };
}

Mat3 mat3_mul(Mat3 a, Mat3 b) {
    Mat3 res = {0};
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            for (int k = 0; k < 3; k++)
                res.m[i][j] += a.m[i][k] * b.m[k][j];
    return res;
}

Mat3 mat3_transpose(Mat3 m) {
    Mat3 res;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            res.m[i][j] = m.m[j][i];
    return res;
}

Mat3 mat3_from_axis_angle(Vec3 axis, float angle) {
    float c = cosf(angle), s = sinf(angle), t = 1.0f - c;
    float x = axis.x, y = axis.y, z = axis.z;
    Mat3 res;
    res.m[0][0] = t*x*x + c;   res.m[0][1] = t*x*y - z*s; res.m[0][2] = t*x*z + y*s;
    res.m[1][0] = t*x*y + z*s; res.m[1][1] = t*y*y + c;   res.m[1][2] = t*y*z - x*s;
    res.m[2][0] = t*x*z - y*s; res.m[2][1] = t*y*z + x*s; res.m[2][2] = t*z*z + c;
    return res;
}
