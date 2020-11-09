#pragma once

#include <map>
#include <vector>

// enum to switch between units in interface dropdown
enum UnitModel{
    mm,
    cm,
    dm,
    m
};

struct LentilFilterData {
  unsigned xres;
  unsigned yres;
  int framenumber;
  int samples;
  bool enabled;
  float current_inv_density;
  float filter_width;
  float time_start;
  float time_end;
  std::map<AtString, std::vector<AtRGBA> > image;
  std::map<AtString, std::vector<AtRGBA> > image_redist;
  std::vector<float> zbuffer;
  std::vector<AtString> aov_list_name;
  std::vector<unsigned int> aov_list_type;
  std::vector<int> aov_types;
  std::vector<bool> pixel_already_visited;
  AtString rgba_string;

  const AtString atstring_rgba = AtString("RGBA");
  const AtString atstring_p = AtString("P");
  const AtString atstring_z = AtString("Z");
  const AtString atstring_transmission = AtString("transmission");
  const AtString atstring_lentil_bidir_ignore = AtString("lentil_bidir_ignore");

  int global_run;
}; extern struct LentilFilterData bokeh;




std::string replace_first_occurence(std::string& s, const std::string& toReplace, const std::string& replaceWith) {
    std::size_t pos = s.find(toReplace);
    if (pos == std::string::npos) return s;
    return s.replace(pos, toReplace.length(), replaceWith);
}

inline float filter_weight_gaussian(AtVector2 p, float width) {
  const float r = std::pow(2.0 / width, 2.0) * (std::pow(p.x, 2) + std::pow(p.y, 2));
  if (r > 1.0f) return 0.0;
  return AiFastExp(2 * -r);
}

inline AtRGBA filter_gaussian_complete(AtAOVSampleIterator *iterator, const float width, const uint8_t aov_type){
  float aweight = 0.0f;
  AtRGBA avalue = AI_RGBA_ZERO;

  while (AiAOVSampleIteratorGetNext(iterator))
  {
      // take into account adaptive sampling
      float inv_density = AiAOVSampleIteratorGetInvDensity(iterator);
      if (inv_density <= 0.f) continue;

      // determine distance to filter center
      const AtVector2& offset = AiAOVSampleIteratorGetOffset(iterator);
      const float r = AiSqr(2 / width) * (AiSqr(offset.x) + AiSqr(offset.y));
      if (r > 1.0f)
          continue;

      // gaussian filter weight
      const float weight = AiFastExp(2 * -r) * inv_density;

      // accumulate weights and colors
      AtRGBA sample_energy = AI_RGBA_ZERO;
      switch (aov_type){
        case AI_TYPE_RGBA: {
          sample_energy = AiAOVSampleIteratorGetRGBA(iterator);
          break;
        }
        case AI_TYPE_RGB: {
          AtRGB sample_energy_rgb = AiAOVSampleIteratorGetRGB(iterator);
          sample_energy = AtRGB(sample_energy_rgb.r, sample_energy_rgb.g, sample_energy_rgb.b);
          break;
        }
      }
      
      avalue += weight * sample_energy;
      aweight += weight;
  }
 
   // compute final filtered color
   if (aweight != 0.0f) avalue /= aweight;

   return avalue;
}


inline AtRGBA filter_closest_complete(AtAOVSampleIterator *iterator, const uint8_t aov_type, LentilFilterData *bokeh){
  AtRGBA pixel_energy = AI_RGBA_ZERO;
  float z = 0.0;

  while (AiAOVSampleIteratorGetNext(iterator))
  {
      float depth = AiAOVSampleIteratorGetAOVFlt(iterator, bokeh->atstring_z);
      if ((std::abs(depth) <= z) || z == 0.0){
        
        z = std::abs(depth);
        AtRGBA sample_energy = AI_RGBA_ZERO;

        switch (aov_type){
          case AI_TYPE_VECTOR: {
            AtVector sample_energy = AiAOVSampleIteratorGetVec(iterator);
            pixel_energy = AtRGBA(sample_energy.x, sample_energy.y, sample_energy.z, 1.0);
            break;
          }
          case AI_TYPE_FLOAT: {
            float sample_energy = AiAOVSampleIteratorGetFlt(iterator);
            pixel_energy = AtRGBA(sample_energy, sample_energy, sample_energy, 1.0);
            break;
          }
          case AI_TYPE_INT: {
            int sample_energy = AiAOVSampleIteratorGetInt(iterator);
            pixel_energy = AtRGBA(sample_energy, sample_energy, sample_energy, 1.0);
            break;
          }
        }
      }
  }

   return pixel_energy;
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

  return 0.0;
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


inline void add_to_buffer(int px, int aov_type, AtString aov_name, 
                          float inv_samples, float inv_density, float fitted_bidir_add_luminance, float depth,
                          bool transmitted_energy_in_sample,
                          int transmission_layer,
                          struct AtAOVSampleIterator* sample_iterator, 
                          std::map<AtString, std::vector<AtRGBA> > &image_color_types,
                          std::map<AtString, std::vector<AtRGBA> > &image_data_types,
                          std::vector<float> &zbuffer) {    
    switch(aov_type){

        case AI_TYPE_RGBA: {
          // RGBA is the only aov with transmission component in, account for that (prob skip something)
          AtRGBA rgba_energy;
          rgba_energy = AiAOVSampleIteratorGetAOVRGBA(sample_iterator, aov_name);
          if (transmitted_energy_in_sample && transmission_layer == 0) rgba_energy = AiAOVSampleIteratorGetAOVRGBA(sample_iterator, AtString("transmission"));
          else if (transmitted_energy_in_sample && transmission_layer == 1) rgba_energy -= AiAOVSampleIteratorGetAOVRGBA(sample_iterator, AtString("transmission"));

          image_color_types[aov_name][px] += (rgba_energy+fitted_bidir_add_luminance) * inv_density * inv_samples;

          break;
        }

        case AI_TYPE_RGB: {
          AtRGB rgb_energy = AiAOVSampleIteratorGetAOVRGB(sample_iterator, aov_name);
          AtRGBA rgba_energy = AtRGBA(rgb_energy.r, rgb_energy.g, rgb_energy.b, 1.0);

          image_color_types[aov_name][px] += (rgba_energy+fitted_bidir_add_luminance) * inv_density * inv_samples;
          
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

        case AI_TYPE_INT: {
          if ((std::abs(depth) <= zbuffer[px]) || zbuffer[px] == 0.0){
            int int_energy = AiAOVSampleIteratorGetAOVInt(sample_iterator, aov_name);
            AtRGBA rgba_energy = AtRGBA(int_energy, int_energy, int_energy, 1.0);
            image_data_types[aov_name][px] = rgba_energy;
            zbuffer[px] = std::abs(depth);
          }

          break;
        }
    }
}

inline void filter_and_add_to_buffer(int px, int py, float filter_width_half, 
                                     float inv_samples, float inv_density, float depth, 
                                     bool transmitted_energy_in_sample, int transmission_layer,
                                     struct AtAOVSampleIterator* iterator, LentilFilterData *filter_data){

    // loop over all pixels in filter radius, then compute the filter weight based on the offset not to the original pixel (px, py), but the filter pixel (x, y)
    for (unsigned y = py - filter_width_half; y <= py + filter_width_half; y++) {
      for (unsigned x = px - filter_width_half; x <= px + filter_width_half; x++) {

        if (y < 0 || y >= filter_data->yres) continue; // edge fix
        if (x < 0 || x >= filter_data->xres) continue; // edge fix

        const unsigned pixelnumber = static_cast<int>(filter_data->xres * y + x);
        
        const AtVector2 &subpixel_position = AiAOVSampleIteratorGetOffset(iterator); // offset within original pixel
        AtVector2 subpixel_pos_dist = AtVector2((px+subpixel_position.x) - x, (py+subpixel_position.y) - y);
        float filter_weight = filter_weight_gaussian(subpixel_pos_dist, filter_data->filter_width);
        if (filter_weight == 0) continue;

        float inv_filter_samples = (1.0 / filter_width_half) / 12.5555; // figure this out so it doesn't break when filter width is not 2
        for (unsigned i=0; i<filter_data->aov_list_name.size(); i++){
          add_to_buffer(pixelnumber, filter_data->aov_list_type[i], filter_data->aov_list_name[i], 
                        inv_samples * inv_filter_samples, inv_density, 0.0, depth, transmitted_energy_in_sample, transmission_layer, iterator,
                        filter_data->image_redist, filter_data->image, filter_data->zbuffer);
        }
      }
    }
  }

unsigned int string_to_arnold_type(std::string str){
  if (str == "float" || str == "FLOAT" || str == "flt" || str == "FLT") return AI_TYPE_FLOAT;
  else if (str == "rgba" || str == "RGBA") return AI_TYPE_RGBA;
  else if (str == "rgb" || str == "RGB") return AI_TYPE_RGB;
  else if (str == "vector" || str == "vec" || str == "VECTOR" || str == "VEC") return AI_TYPE_VECTOR;

  return 0;
}


std::string erase_string(std::string str, std::string pattern) {
  std::string::size_type i = str.find(pattern);
  while (i != std::string::npos) {
    str.erase(i, pattern.length());
    i = str.find(pattern, i);
  }

  return str;
}
