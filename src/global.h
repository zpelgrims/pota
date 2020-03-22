#pragma once

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

  common_sincosf(2.0f*M_PI/blades * (tri+1), p1, p1+1);
  common_sincosf(2.0f*M_PI/blades * tri, p2, p2+1);

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
