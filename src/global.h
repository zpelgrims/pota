#pragma once

#include <map>

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

std::string replace_first_occurence(std::string& s, const std::string& toReplace, const std::string& replaceWith) {
    std::size_t pos = s.find(toReplace);
    if (pos == std::string::npos) return s;
    return s.replace(pos, toReplace.length(), replaceWith);
}

inline float filter_gaussian(AtVector2 p, float width) {
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


float additional_luminance_soft_trans(float sample_luminance, float additional_luminance, float transition_width, float minimum_luminance){
  // additional luminance with soft transition
  if (sample_luminance > minimum_luminance && sample_luminance < minimum_luminance+transition_width){
    float perc = (sample_luminance - minimum_luminance) / transition_width;
    return additional_luminance * perc;          
  } else if (sample_luminance > minimum_luminance+transition_width) {
    return additional_luminance;
  } 
}


void save_to_exr_rgba(std::vector<float> img, std::string filename, unsigned xres, unsigned yres) {
  EXRHeader header;
  InitEXRHeader(&header);

  EXRImage image;
  InitEXRImage(&image);
  image.num_channels = 4;
  image.width = xres;
  image.height = yres;

  std::vector<float> images[4];
  images[0].resize(xres * yres);
  images[1].resize(xres * yres);
  images[2].resize(xres * yres);
  images[3].resize(xres * yres);

  for (unsigned int i = 0; i < xres * yres; i++) {
    images[0][i] = img[4*i+0];
    images[1][i] = img[4*i+1];
    images[2][i] = img[4*i+2];
    images[3][i] = img[4*i+3];
  }

  float* image_ptr[4];
  image_ptr[0] = &(images[3].at(0)); // A
  image_ptr[1] = &(images[2].at(0)); // B
  image_ptr[2] = &(images[1].at(0)); // G
  image_ptr[3] = &(images[0].at(0)); // R

  image.images = (unsigned char**)image_ptr;
  header.num_channels = 4;
  header.channels = (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);
  strncpy(header.channels[0].name, "A", 255); header.channels[0].name[strlen("A")] = '\0';
  strncpy(header.channels[1].name, "B", 255); header.channels[1].name[strlen("B")] = '\0';
  strncpy(header.channels[2].name, "G", 255); header.channels[2].name[strlen("G")] = '\0';
  strncpy(header.channels[3].name, "R", 255); header.channels[3].name[strlen("R")] = '\0';

  header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels); 
  header.requested_pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
  for (int i = 0; i < header.num_channels; i++) {
    header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
    header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in .EXR
  }
  
  header.compression_type = TINYEXR_COMPRESSIONTYPE_ZIP;

  const char* err;
  int ret = SaveEXRImageToFile(&image, &header, filename.c_str(), &err);
  if (ret != TINYEXR_SUCCESS) {
    AiMsgWarning("[LENTIL BIDIRECTIONAL TL] Error when saving exr: %s", err);
  }
}


std::vector<std::string> split_str(std::string str, std::string token)
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


inline void add_to_buffer(AtRGBA sample, int px, int aov_type, AtString aov_name, 
                          int samples, float inv_density, float fitted_bidir_add_luminance, float depth, 
                          struct AtAOVSampleIterator* sample_iterator, 
                          std::map<AtString, std::vector<AtRGBA> > &image_color_types,
                          std::map<AtString, std::vector<float> > &weight_per_pixel,
                          std::map<AtString, std::vector<AtRGBA> > &image_data_types,
                          std::vector<float> &zbuffer,
                          AtString rgba_string) {
    switch(aov_type){

        case AI_TYPE_RGBA: {
            
          // RGBA is the only aov with transmission component in
          AtRGBA rgba_energy;
          if (aov_name == rgba_string){
            rgba_energy = ((sample)+fitted_bidir_add_luminance) / (double)(samples);
          } else {
            rgba_energy = ((AiAOVSampleIteratorGetAOVRGBA(sample_iterator, aov_name))+fitted_bidir_add_luminance) / (double)(samples);
          }

          image_color_types[aov_name][px] += rgba_energy * inv_density;
          weight_per_pixel[aov_name][px] += inv_density / double(samples);
        
          break;
        }

        case AI_TYPE_RGB: {
          AtRGB rgb_energy = AiAOVSampleIteratorGetAOVRGB(sample_iterator, aov_name) + fitted_bidir_add_luminance;
          AtRGBA rgba_energy = AtRGBA(rgb_energy.r, rgb_energy.g, rgb_energy.b, 1.0) / (double)(samples);

          image_color_types[aov_name][px] += rgba_energy * inv_density;
          weight_per_pixel[aov_name][px] += inv_density / double(samples);
          
          break;
        }

        case AI_TYPE_VECTOR: {
          if ((std::abs(depth) <= zbuffer[px]) || zbuffer[px] == 0.0){
            AtVector vec_energy = AiAOVSampleIteratorGetAOVVec(sample_iterator, aov_name);
            AtRGBA rgba_energy = AtRGBA(vec_energy.x, vec_energy.y, vec_energy.z, 1.0);
            image_data_types[aov_name][px] = rgba_energy;
            zbuffer[px] = std::abs(depth);
          }

          break;
        }

        case AI_TYPE_FLOAT: {
          if ((std::abs(depth) <= zbuffer[px]) || zbuffer[px] == 0.0){
            float flt_energy = AiAOVSampleIteratorGetAOVFlt(sample_iterator, aov_name);
            AtRGBA rgba_energy = AtRGBA(flt_energy, flt_energy, flt_energy, 1.0);
            image_data_types[aov_name][px] = rgba_energy;
            zbuffer[px] = std::abs(depth);
          }

          break;
        }
    }
}