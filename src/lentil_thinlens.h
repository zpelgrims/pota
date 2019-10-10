#pragma once
#include <string.h>
#include <cmath>
#include "../../Eigen/Eigen/Dense"


struct CameraThinLens
{
	float fov;
    float tan_fov;
    float sensor_width;
    float sensor_height;
    float focal_length;
    float fstop;
    float focus_distance;
    float aperture_radius;

    float minimum_rgb;
    AtString bokeh_exr_path;

    float chr_abb_mult;
    float optical_vignetting_distance;
    float optical_vignetting_radius;

    float bias;
    float gain;
    bool invert;
};

extern struct CameraThinLens tl;


// xorshift fast random number generator
inline uint32_t xor128(void){
  static uint32_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
  uint32_t t = x ^ (x << 11);
  x = y; y = z; z = w;
  return w = (w ^ (w >> 19) ^ t ^ (t >> 8));
}

inline float bias_symmetrical(float x, float b) {
    b = -std::log2(1.0f - b);
    return 1.0f - std::pow(1.0f - std::pow(x, 1.0f/b), b);
}

// Improved concentric mapping code by Dave Cline [peter shirley´s blog]
// maps points on the unit square onto the unit disk uniformly
inline void concentricDiskSample(float ox, float oy, AtVector2 *lens, float bias)
{
    float phi, r;

    // switch coordinate space from [0, 1] to [-1, 1]
    float a = 2.0 * ox - 1.0;
    float b = 2.0 * oy - 1.0;

    if ((a * a) > (b * b)){
        r = a;
        phi = (0.78539816339f) * (b / a);
    }
    else {
        r = b;
        phi = (AI_PIOVER2)-(0.78539816339f) * (a / b);
    }

    if (bias != 0.5) r = AiBias(std::abs(r), bias) * (r < 0 ? -1 : 1);

    lens->x = r * std::cos(phi);
    lens->y = r * std::sin(phi);
}

// creates a secondary, virtual aperture resembling the exit pupil on a real lens
bool empericalOpticalVignetting(AtVector origin, AtVector direction, float apertureRadius, float opticalVignettingRadius, float opticalVignettingDistance){
    // because the first intersection point of the aperture is already known, I can just linearly scale it by the distance to the second aperture
    AtVector opticalVignetPoint = (direction * opticalVignettingDistance) - origin;
    float pointHypotenuse = std::sqrt((opticalVignetPoint.x * opticalVignetPoint.x) + (opticalVignetPoint.y * opticalVignetPoint.y));
    float virtualApertureTrueRadius = apertureRadius * opticalVignettingRadius;

    return std::abs(pointHypotenuse) < virtualApertureTrueRadius;
}