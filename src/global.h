#pragma once

#include <map>
#include <vector>
#include <atomic>
#include <memory>


struct LentilFilterData {
  unsigned xres;
  unsigned yres;
  unsigned xres_without_region;
  unsigned yres_without_region;
  int region_min_x;
  int region_min_y;
  int region_max_x;
  int region_max_y;
  int shift_x;
  int shift_y;
  int samples;
  bool enabled;
  float current_inv_density;
  float filter_width;
  float time_start;
  float time_end;
  std::map<AtString, std::vector<AtRGBA> > image_data_types;
  std::map<AtString, std::vector<AtRGBA> > image_col_types;
  // std::map<AtString, std::vector<const void *> > image_ptr_types;
  std::map<AtString, unsigned int> aov_duplicates;
  std::vector<float> zbuffer;
  std::vector<AtString> aov_list_name;
  std::vector<unsigned int> aov_list_type;
  std::vector<bool> aov_crypto;

  std::map<AtString, std::vector<std::map<float, float>>> crypto_hash_map;
  std::map<AtString, std::vector<float>> crypto_total_weight;
  std::vector<AtString> cryptomatte_aov_names;

  const AtString atstring_rgba = AtString("RGBA");
  const AtString atstring_p = AtString("P");
  const AtString atstring_z = AtString("Z");
  const AtString atstring_transmission = AtString("transmission");
  const AtString atstring_opacity = AtString("opacity");
  const AtString atstring_lentil_ignore = AtString("lentil_ignore");
  const AtString atstring_motionvector = AtString("lentil_object_motion_vector");
  const AtString atstring_time = AtString("lentil_time");

  AtUniverse *arnold_universe;
  AtNode *camera;

  bool imager_print_once_only;

}; extern struct LentilFilterData bokeh;



extern AtCritSec g_critsec;
extern bool g_critsec_active;



///////////////////////////////////////////////
//
//      Crit sec utilities
//
///////////////////////////////////////////////

inline bool crypto_crit_sec_init() {
    // Called in node_plugin_initialize. Returns true as a convenience.
    g_critsec_active = true;
    AiCritSecInit(&g_critsec);
    return true;
}

inline void crypto_crit_sec_close() {
    // Called in node_plugin_cleanup
    g_critsec_active = false;
    AiCritSecClose(&g_critsec);
}

inline void crypto_crit_sec_enter() {
    // If the crit sec has not been inited since last close, we simply do not enter.
    // (Used by Cryptomatte filter.)
    if (g_critsec_active)
        AiCritSecEnter(&g_critsec);
}

inline void crypto_crit_sec_leave() {
    // If the crit sec has not been inited since last close, we simply do not enter.
    // (Used by Cryptomatte filter.)
    if (g_critsec_active)
        AiCritSecLeave(&g_critsec);
}



inline int coords_to_linear_pixel(const int x, const int y, const int xres) {
  return x + (y * xres);
}

inline int coords_to_linear_pixel_region(const int x, const int y, const int xres, const int region_min_x, const int region_min_y) {
  return (x-region_min_x) + ((y-region_min_y) * xres);
}

inline void linear_pixel_to_coords(const int linear_pixel, int &x, int &y, const int xres) {
  x = linear_pixel % xres;
  y = (int)(linear_pixel / xres);
}

inline void linear_pixel_region_to_coords(const int linear_pixel, int &x, int &y, const int xres, const int region_min_x, const int region_min_y) {
  x = (linear_pixel % xres) + region_min_x;
  y = (int)(linear_pixel / xres) + region_min_y;
}



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

inline AtRGBA filter_gaussian_complete(AtAOVSampleIterator *iterator, const float width, const uint8_t aov_type, float inv_density){
  float aweight = 0.0f;
  AtRGBA avalue = AI_RGBA_ZERO;

  while (AiAOVSampleIteratorGetNext(iterator))
  {
      // take into account adaptive sampling
      // float inv_density = AiAOVSampleIteratorGetInvDensity(iterator);
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

        switch (aov_type){
          case AI_TYPE_VECTOR: {
            const AtVector sample_energy = AiAOVSampleIteratorGetVec(iterator);
            pixel_energy = AtRGBA(sample_energy.x, sample_energy.y, sample_energy.z, 1.0);
            break;
          }
          case AI_TYPE_FLOAT: {
            const float sample_energy = AiAOVSampleIteratorGetFlt(iterator);
            pixel_energy = AtRGBA(sample_energy, sample_energy, sample_energy, 1.0);
            break;
          }
          // case AI_TYPE_INT: {
          //   const int sample_energy = AiAOVSampleIteratorGetInt(iterator);
          //   pixel_energy = AtRGBA(sample_energy, sample_energy, sample_energy, 1.0);
          //   break;
          // }
          // case AI_TYPE_UINT: {
          //   const unsigned sample_energy = AiAOVSampleIteratorGetUInt(iterator);
          //   pixel_energy = AtRGBA(sample_energy, sample_energy, sample_energy, 1.0);
          //   break;
          // }
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



// get all depth samples so i can re-use them
inline void cryptomatte_construct_cache(std::map<AtString, std::map<float, float>> &crypto_hashmap_cache,
                                        std::vector<AtString> &cryptomatte_aov_names,
                                        struct AtAOVSampleIterator* sample_iterator, 
                                        const int sampleid, LentilFilterData *filter_data) {

  for (auto &aov : cryptomatte_aov_names) {

    float iterative_transparency_weight = 1.0f;
    float quota = 1.0;
    float sample_value = 0.0f;

    while (AiAOVSampleIteratorGetNextDepth(sample_iterator)) {
        const float sub_sample_opacity = AiColorToGrey(AiAOVSampleIteratorGetAOVRGB(sample_iterator, filter_data->atstring_opacity));
        sample_value = AiAOVSampleIteratorGetAOVFlt(sample_iterator, aov);
        const float sub_sample_weight = sub_sample_opacity * iterative_transparency_weight;

        // so if the current sub sample is 80% opaque, it means 20% of the weight will remain for the next subsample
        iterative_transparency_weight *= (1.0f - sub_sample_opacity);

        quota -= sub_sample_weight;

        crypto_hashmap_cache[aov][sample_value] += sub_sample_weight;
    }

    if (quota > 0.0) { // the remaining values gets allocated to the last sample
        crypto_hashmap_cache[aov][sample_value] += quota;
    }

    // reset is required because AiAOVSampleIteratorGetNextDepth() automatically moves to next sample after final depth sample
    // still need to use the iterator afterwards, so need to do a reset to the current sample id
    AiAOVSampleIteratorReset(sample_iterator);
    for (int i = 0; AiAOVSampleIteratorGetNext(sample_iterator) == true; i++){
      if (i == sampleid) return;
    }
  }
}


inline void add_to_buffer_cryptomatte(int px, LentilFilterData *filter_data, std::map<float, float> &cryptomatte_cache, const AtString aov_name, const float sample_weight) {
  filter_data->crypto_total_weight[aov_name][px] += sample_weight;
  for (auto const& sample : cryptomatte_cache) {
    filter_data->crypto_hash_map[aov_name][px][sample.first] += sample.second * sample_weight;
  }
}


inline void add_to_buffer(int px, int aov_type, AtString aov_name, AtRGBA aov_value,
                          float inv_samples, float inv_density, float fitted_bidir_add_luminance, float depth,
                          bool transmitted_energy_in_sample, int transmission_layer,
                          struct AtAOVSampleIterator* sample_iterator, LentilFilterData *filter_data) {


    const float inv_aov_count = 1.0/(double)filter_data->aov_duplicates[aov_name];
    
    switch(aov_type){

        case AI_TYPE_RGBA: {
          // RGBA is the only aov with transmission component in, account for that (prob skip something)
          AtRGBA rgba_energy = aov_value;
          if (transmitted_energy_in_sample && transmission_layer == 0) rgba_energy = AiAOVSampleIteratorGetAOVRGBA(sample_iterator, filter_data->atstring_transmission);
          else if (transmitted_energy_in_sample && transmission_layer == 1) rgba_energy -= AiAOVSampleIteratorGetAOVRGBA(sample_iterator, filter_data->atstring_transmission);

          filter_data->image_col_types[aov_name][px] += (rgba_energy+fitted_bidir_add_luminance) * inv_density * inv_samples * inv_aov_count;

          break;
        }

        case AI_TYPE_RGB: { // could be buggy due to discrepancy between this and RGBA above??? test!
          const AtRGBA rgba_energy = aov_value;
          filter_data->image_col_types[aov_name][px] += (rgba_energy+fitted_bidir_add_luminance) * inv_density * inv_samples;
          
          break;
        }

        case AI_TYPE_VECTOR: {
          if ((std::abs(depth) <= filter_data->zbuffer[px]) || filter_data->zbuffer[px] == 0.0){
            filter_data->image_data_types[aov_name][px] = aov_value;
            filter_data->zbuffer[px] = std::abs(depth);
          }

          break;
        }

        case AI_TYPE_FLOAT: {
          if ((std::abs(depth) <= filter_data->zbuffer[px]) || filter_data->zbuffer[px] == 0.0){
            filter_data->image_data_types[aov_name][px] = aov_value;
            filter_data->zbuffer[px] = std::abs(depth);
          }
      
          break;
        }

        // case AI_TYPE_INT: {
        //   if ((std::abs(depth) <= filter_data->zbuffer[px]) || filter_data->zbuffer[px] == 0.0){
        //     const int int_energy = AiAOVSampleIteratorGetAOVInt(sample_iterator, aov_name);
        //     const AtRGBA rgba_energy = AtRGBA(int_energy, int_energy, int_energy, 1.0);
        //     filter_data->image_data_types[aov_name][px] = rgba_energy;
        //     filter_data->zbuffer[px] = std::abs(depth);
        //   }

        //   break;
        // }

        // case AI_TYPE_UINT: {
        //   if ((std::abs(depth) <= filter_data->zbuffer[px]) || filter_data->zbuffer[px] == 0.0){
        //     const unsigned uint_energy = AiAOVSampleIteratorGetAOVUInt(sample_iterator, aov_name);
        //     const AtRGBA rgba_energy = AtRGBA(uint_energy, uint_energy, uint_energy, 1.0);
        //     filter_data->image_data_types[aov_name][px] = rgba_energy;
        //     filter_data->zbuffer[px] = std::abs(depth);
        //   }

        //   break;
        // }

        // case AI_TYPE_POINTER: {
        //   if ((std::abs(depth) <= filter_data->zbuffer[px]) || filter_data->zbuffer[px] == 0.0){
        //     const void *ptr_energy = AiAOVSampleIteratorGetAOVPtr(sample_iterator, aov_name);
        //     filter_data->image_ptr_types[aov_name][px] = ptr_energy;
        //     filter_data->zbuffer[px] = std::abs(depth);
        //   }

        //   break;
        // }
    }
}

inline void filter_and_add_to_buffer(int px, int py, float filter_width_half, 
                                     float inv_samples, float inv_density, float depth, 
                                     bool transmitted_energy_in_sample, int transmission_layer, int sampleid,
                                     struct AtAOVSampleIterator* iterator, LentilFilterData *filter_data,
                                     std::map<AtString, std::map<float, float>> &cryptomatte_cache, std::vector<AtRGBA> &aov_values){

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
          if (filter_data->aov_crypto[i]){
            add_to_buffer_cryptomatte(pixelnumber, filter_data, cryptomatte_cache[filter_data->aov_list_name[i]], filter_data->aov_list_name[i], filter_weight * inv_samples * inv_filter_samples * inv_density);
          } else {
            add_to_buffer(pixelnumber, filter_data->aov_list_type[i], filter_data->aov_list_name[i], aov_values[i],
                          inv_samples * inv_filter_samples, inv_density, 0.0, depth, transmitted_energy_in_sample, transmission_layer, iterator,
                          filter_data);
          }
        }
      }
    }
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
