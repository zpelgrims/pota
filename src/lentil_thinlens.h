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

    float emperical_ca_dist;
    float optical_vignetting_distance;
    float optical_vignetting_radius;

    float bias;
    float gain;
    bool invert;

    float square;
    float squeeze;
};

extern struct CameraThinLens tl;


// xorshift fast random number generator
inline uint32_t xor128(void){
  static uint32_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
  uint32_t t = x ^ (x << 11);
  x = y; y = z; z = w;
  return w = (w ^ (w >> 19) ^ t ^ (t >> 8));
}

inline float linearInterpolate(float perc, float a, float b){
    return a + perc * (b - a);
}


// sin approximation, not completely accurate but faster than std::sin
inline float fastSin(float x){
    x = fmod(x + AI_PI, AI_PI * 2) - AI_PI; // restrict x so that -AI_PI < x < AI_PI
    const float B = 4.0f / AI_PI;
    const float C = -4.0f / (AI_PI*AI_PI);
    float y = B * x + C * x * std::abs(x);
    const float P = 0.225f;
    return P * (y * std::abs(y) - y) + y;
}


inline float fastCos(float x){
    // conversion from sin to cos
    x += AI_PI * 0.5;

    x = fmod(x + AI_PI, AI_PI * 2) - AI_PI; // restrict x so that -AI_PI < x < AI_PI
    const float B = 4.0f / AI_PI;
    const float C = -4.0f / (AI_PI*AI_PI);
    float y = B * x + C * x * std::abs(x);
    const float P = 0.225f;
    return P * (y * std::abs(y) - y) + y;
}

// Improved concentric mapping code by Dave Cline [peter shirley´s blog]
// maps points on the unit square onto the unit disk uniformly
inline void concentricDiskSample(float ox, float oy, AtVector2 *lens, float bias, float squarelerp, float squeeze_x)
{
    if (ox == 0.0 && oy == 0.0){
        lens->x = 0.0;
        lens->y = 0.0;
        return;
    }

    float phi, r;

    // switch coordinate space from [0, 1] to [-1, 1]
    const float a = 2.0 * ox - 1.0;
    const float b = 2.0 * oy - 1.0;

    if ((a * a) > (b * b)){
        r = a;
        phi = 0.78539816339 * (b / a);
    }
    else {
        r = b;
        phi = (AI_PIOVER2) - ((0.78539816339) * (a / b));
    }

    if (bias != 0.5) r = AiBias(std::abs(r), bias) * (r < 0 ? -1 : 1);


    bool fast_trigo = true;

    const float cos_phi = fast_trigo ? fastCos(phi) : std::cos(phi);
    const float sin_phi = fast_trigo ? fastSin(phi) : std::sin(phi);
    lens->x = r * cos_phi;
    lens->y = r * sin_phi;

    if (squarelerp > 0.0){
        lens->x = linearInterpolate(squarelerp, lens->x, a);
        lens->y = linearInterpolate(squarelerp, lens->y, b);
    }

    lens->x *= squeeze_x;
}


// creates a secondary, virtual aperture resembling the exit pupil on a real lens
bool empericalOpticalVignetting(AtVector origin, AtVector direction, float apertureRadius, float opticalVignettingRadius, float opticalVignettingDistance){
    // because the first intersection point of the aperture is already known, I can just linearly scale it by the distance to the second aperture
    AtVector opticalVignetPoint = (direction * opticalVignettingDistance) - origin;
    float pointHypotenuse = std::sqrt((opticalVignetPoint.x * opticalVignetPoint.x) + (opticalVignetPoint.y * opticalVignetPoint.y));
    float virtualApertureTrueRadius = apertureRadius * opticalVignettingRadius;

    return std::abs(pointHypotenuse) < virtualApertureTrueRadius;
}
