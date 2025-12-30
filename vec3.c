#include "vec3.h"

void vec3_coordinate_system(Vec3 n, Vec3* s, Vec3* t) {
    if (fabsf(n.x) > fabsf(n.y)) {
        float inv_len = 1.0f / sqrtf(n.x * n.x + n.z * n.z);
        *s = (Vec3){-n.z * inv_len, 0.0f, n.x * inv_len};
    } else {
        float inv_len = 1.0f / sqrtf(n.y * n.y + n.z * n.z);
        *s = (Vec3){0.0f, n.z * inv_len, -n.y * inv_len};
    }
    *t = vec3_cross(n, *s);
}
