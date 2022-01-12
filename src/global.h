#pragma once

#include <map>
#include <vector>
#include <atomic>
#include <memory>



inline float linear_interpolate(float perc, float a, float b){
    return a + perc * (b - a);
}

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

// https://github.com/nvpro-samples/optix_advanced_samples/blob/master/src/optixIntroduction/optixIntro_06/shaders/random_number_generators.h
// Tiny Encryption Algorithm (TEA) to calculate a the seed per launch index and iteration.
template<unsigned int N>
inline unsigned int tea(const unsigned int val0, const unsigned int val1)
{
  unsigned int v0 = val0;
  unsigned int v1 = val1;
  unsigned int s0 = 0;

  for (unsigned int n = 0; n < N; ++n)
  {
    s0 += 0x9e3779b9;
    v0 += ((v1 << 4) + 0xA341316C) ^ (v1 + s0) ^ ((v1 >> 5) + 0xC8013EA4);
    v1 += ((v0 << 4) + 0xAD90777D) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7E95761E);
  }
  return v0;
}


// https://github.com/nvpro-samples/optix_advanced_samples/blob/master/src/optixIntroduction/optixIntro_06/shaders/random_number_generators.h
// Return a random sample in the range [0, 1) with a simple Linear Congruential Generator.
inline float rng(unsigned int& previous)
{
  previous = previous * 1664525u + 1013904223u;
  
  return float(previous & 0X00FFFFFF) / float(0x01000000u); // Use the lower 24 bits.
  // return float(previous >> 8) / float(0x01000000u);      // Use the upper 24 bits
}



inline unsigned int string_to_arnold_type(std::string str){
  if (str == "float" || str == "FLOAT" || str == "flt" || str == "FLT") return AI_TYPE_FLOAT;
  else if (str == "rgba" || str == "RGBA") return AI_TYPE_RGBA;
  else if (str == "rgb" || str == "RGB") return AI_TYPE_RGB;
  else if (str == "vector" || str == "vec" || str == "VECTOR" || str == "VEC") return AI_TYPE_VECTOR;

  return 0;
}



// inline float crypto_gaussian(AtVector2 p, float width) {
//     /* matches Arnold's exactly. */
//     /* Sharpness=2 is good for width 2, sigma=1/sqrt(8) for the width=4,sharpness=4 case */
//     // const float sigma = 0.5f;
//     // const float sharpness = 1.0f / (2.0f * sigma * sigma);

//     p /= (width * 0.5f);
//     float dist_squared = (p.x * p.x + p.y * p.y);
//     if (dist_squared > (1.0f)) {
//         return 0.0f;
//     }

//     // const float normalization_factor = 1;
//     // Float weight = normalization_factor * expf(-dist_squared * sharpness);

//     float weight = expf(-dist_squared * 2.0f); // was:

//     if (weight > 0.0f) {
//         return weight;
//     } else {
//         return 0.0f;
//     }
// }