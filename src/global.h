#pragma once

#include <map>
#include <vector>
#include <atomic>
#include <memory>



inline std::string replace_first_occurence(std::string& s, const std::string& toReplace, const std::string& replaceWith) {
    std::size_t pos = s.find(toReplace);
    if (pos == std::string::npos) return s;
    return s.replace(pos, toReplace.length(), replaceWith);
}


inline float filter_weight_gaussian(AtVector2 p, float width) {
  const float r = std::pow(2.0 / width, 2.0) * (std::pow(p.x, 2) + std::pow(p.y, 2));
  if (r > 1.0f) return 0.0;
  return AiFastExp(2 * -r);
}


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


static inline void common_sincosf(double phi, double *sin, double *cos) {
  *sin = std::sin(phi);
  *cos = std::cos(phi);
}




static inline void lens_sample_triangular_aperture(double &x, double &y, double r1, double r2, const double radius, const int blades)
{
  const int tri = (int)(r1*blades);

  // rescale:
  r1 = r1*blades - tri;

  // sample triangle:
  double a = std::sqrt(r1);
  double b = (1.0f-r2)*a;
  double c = r2*a;

  double p1[2], p2[2];

  common_sincosf(2.0f*AI_PI/blades * (tri+1), p1, p1+1);
  common_sincosf(2.0f*AI_PI/blades * tri, p2, p2+1);

  x = radius * (b * p1[1] + c * p2[1]);
  y = radius * (b * p1[0] + c * p2[0]);
}



inline std::vector<std::string> split_str(std::string str, std::string token)
{
    std::vector<std::string>result;
    while(str.size())
    {
        size_t index = static_cast<size_t>(str.find(token));
        
        if(index != std::string::npos)
        {
            result.push_back(str.substr(0, index));
            str = str.substr(index+token.size());
            
            if(str.size() == 0)
                result.push_back(str);
        }
        else
        {
            result.push_back(str);
            str = "";
        }
    }
    return result;
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


inline void reset_iterator_to_id(AtAOVSampleIterator* iterator, int id){
  AiAOVSampleIteratorReset(iterator);
  
  for (int i = 0; AiAOVSampleIteratorGetNext(iterator) == true; i++){
    if (i == id) return;
  }

  return;
}




inline unsigned int string_to_arnold_type(std::string str){
  if (str == "float" || str == "FLOAT" || str == "flt" || str == "FLT") return AI_TYPE_FLOAT;
  else if (str == "rgba" || str == "RGBA") return AI_TYPE_RGBA;
  else if (str == "rgb" || str == "RGB") return AI_TYPE_RGB;
  else if (str == "vector" || str == "vec" || str == "VECTOR" || str == "VEC") return AI_TYPE_VECTOR;

  return 0;
}


// using c++17 functionality here to return multiple values
inline auto find_filter_index_in_aov_string (std::string output_string, AtUniverse *uni) {
  
  struct returnValues {
    int filter_index;
    std::vector<std::string> output_string_split;
  };

  // first find which element is the filter (if *filter* in type_name)
  // then assuming that aov type comes before the filter, and the aov name comes before the type
  // should avoid cases where the camera name is placed in front of the output string
  int filter_index = 0;
  std::vector<std::string> output_string_split = split_str(output_string, std::string(" "));
  for (int s=0; s<output_string_split.size(); s++) {

    AtString substring_as = AtString(output_string_split[s].c_str());
    AtNode *substring_node = AiNodeLookUpByName(uni, substring_as);
    
    if (substring_node == nullptr) continue;

    const AtNodeEntry *substring_ne = AiNodeGetNodeEntry(substring_node);
    std::string substring_ne_name = AiNodeEntryGetNameAtString(substring_ne).c_str();

    if (substring_ne_name.find("filter") != std::string::npos) {
        filter_index = s;
    }
  }

  // filter index can never be 0
  if (filter_index == 0) AiMsgError("[LENTIL] Can't find a filter to replace in AOV string: %s", output_string.c_str());

  return returnValues {filter_index, output_string_split};
}
