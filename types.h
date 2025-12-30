#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <float.h>
#include <math.h>

#define PI 3.14159265358979323846f
#define INV_PI 0.31830988618379067154f
#define EPSILON 1e-4f
#define MAX_FLOAT FLT_MAX

typedef struct { float x, y, z; } Vec3;
typedef struct { float m[3][3]; } Mat3;
typedef struct { Vec3 origin, direction; } Ray;

#define SPECTRUM_SAMPLES 4
typedef struct {
    float wavelengths[SPECTRUM_SAMPLES];
    float values[SPECTRUM_SAMPLES];
} Spectrum;

#endif
