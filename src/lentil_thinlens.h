#pragma once
#include <string.h>
#include <cmath>
#include "../../Eigen/Eigen/Dense"

#include "imagebokeh.h"

struct CameraThinLens
{
    imageData image;

	float fov;
    float tan_fov;
    float sensor_width;
    float focal_length;
    float fstop;
    float focus_distance;
    float aperture_radius;

    float minimum_rgb;
    AtString bokeh_exr_path;

    float emperical_ca_dist;
    float optical_vignetting_distance;
    float optical_vignetting_radius;

    float abb_spherical;
    float abb_coma;
    float gain;
    bool invert;

    float square;
    float squeeze;

    bool use_image;
    AtString bokeh_input_path;

    unsigned int bokeh_samples_mult;

    float additional_luminance;
    float luminance_remap_transition_width;

    bool proper_ray_derivatives;

    float sensor_distance;
};

extern struct CameraThinLens tl;


inline float clamp(float in, const float min, const float max) {
    if (in < min) in = min;
    if (in > max) in = max;
    return in;
}

inline float clamp_min(float in, const float min) {
    if (in < min) in = min;
    return in;
}

// xorshift fast random number generator
inline uint32_t xor128(void){
  static uint32_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
  uint32_t t = x ^ (x << 11);
  x = y; y = z; z = w;
  return w = (w ^ (w >> 19) ^ t ^ (t >> 8));
}

inline float linear_interpolate(float perc, float a, float b){
    return a + perc * (b - a);
}


// sin approximation, not completely accurate but faster than std::sin
inline float fast_sin(float x){
    x = fmod(x + AI_PI, AI_PI * 2) - AI_PI; // restrict x so that -AI_PI < x < AI_PI
    const float B = 4.0f / AI_PI;
    const float C = -4.0f / (AI_PI*AI_PI);
    float y = B * x + C * x * std::abs(x);
    const float P = 0.225f;
    return P * (y * std::abs(y) - y) + y;
}


inline float fast_cos(float x){
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
inline void concentricDiskSample(float ox, float oy, Eigen::Vector2d &lens, float bias, float squarelerp, float squeeze_x)
{
    if (ox == 0.0 && oy == 0.0){
        lens(0) = 0.0;
        lens(1) = 0.0;
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

    const float cos_phi = fast_trigo ? fast_cos(phi) : std::cos(phi);
    const float sin_phi = fast_trigo ? fast_sin(phi) : std::sin(phi);
    lens(0) = r * cos_phi;
    lens(1) = r * sin_phi;

    if (squarelerp > 0.0){
        lens(0) = linear_interpolate(squarelerp, lens(0), a);
        lens(1) = linear_interpolate(squarelerp, lens(1), b);
    }
}


// creates a secondary, virtual aperture resembling the exit pupil on a real lens
inline bool empericalOpticalVignetting(AtVector origin, AtVector direction, float apertureRadius, float opticalVignettingRadius, float opticalVignettingDistance){
    // because the first intersection point of the aperture is already known, I can just linearly scale it by the distance to the second aperture
    float intersection = std::abs(opticalVignettingDistance / direction.z);
    AtVector opticalVignetPoint = (direction * intersection) - origin;
    float pointHypotenuse = std::sqrt((opticalVignetPoint.x * opticalVignetPoint.x) + (opticalVignetPoint.y * opticalVignetPoint.y));
    float virtualApertureTrueRadius = apertureRadius * opticalVignettingRadius;

    return std::abs(pointHypotenuse) < virtualApertureTrueRadius;
}

inline bool empericalOpticalVignettingSquare(AtVector origin, AtVector direction, float apertureRadius, float opticalVignettingRadius, float opticalVignettingDistance, float squarebias){
    float intersection = std::abs(opticalVignettingDistance / direction.z);
    AtVector opticalVignetPoint = (direction * intersection) - origin;

    float power = 1.0 + squarebias;
    float radius = apertureRadius * opticalVignettingRadius;
    float dist = std::pow(std::abs(opticalVignetPoint.x), power) + std::pow(std::abs(opticalVignetPoint.y), power);
   
	return !(dist > std::pow(radius, power));
}

// emperical mapping
inline float lerp_squircle_mapping(float amount) {
    return 1.0 + std::log(1.0+amount)*std::exp(amount*3.0);
}
