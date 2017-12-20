#pragma once

#include <ai.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>


#ifdef __APPLE__
    const char emoticon[5] = {0xF0, 0x9F, 0xA5, 0x91, '\0'};
#else
    const char emoticon[0] = {};
#endif



// enum to switch between lens models in interface dropdown
enum LensModel{
    fisheye,
    fisheye_aspherical,
    double_gauss,
    double_gauss_angenieux,
    petzval,
    NONE
};

struct drawData{
    std::ofstream myfile;
    bool draw;
    int counter;

    drawData()
        : draw(false), counter(0){
    }
};


struct MyCameraData
{
	LensModel lensModel;
    drawData draw;


	float sensor_width;
	float sensor_height;
	float fstop;
    float max_fstop;
	float focus_distance;
	float aperture_radius;
	float sensor_shift;
	float aperture_colorshift;
	float lambda;
    int aperture_blades;
	bool dof;

	int rays_succes;
	int rays_fail;
};

extern struct MyCameraData camera_data;




// xorshift fast random number generator
inline uint32_t xor128(void){
    static uint32_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
    uint32_t t = x ^ (x << 11);
    x = y; y = z; z = w;
    return w = (w ^ (w >> 19) ^ t ^ (t >> 8));
}


inline float Lerp(float t, float v1, float v2)
{
    return (1 - t) * v1 + t * v2;
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



// maps points on the unit square onto the unit disk uniformly
inline void concentric_disk_sample(const float ox, const float oy, AtVector2 &lens, bool fast_trigo)
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

    if (!fast_trigo){
        lens.x = r * std::cos(phi);
        lens.y = r * std::sin(phi);
    } else {
        lens.x = r * fastCos(phi);
        lens.y = r * fastSin(phi);
    }

    
}