#pragma once

#include <math.h>
#include "../../polynomial-optics/src/raytrace.h"


static inline void common_sincosf(float phi, float* sin, float* cos) {
  *sin = std::sin(phi);
  *cos = std::cos(phi);
}


// helper function for dumped polynomials to compute integer powers of x:
static inline float lens_ipow(const float x, const int exp) {
  if(exp == 0) return 1.0f;
  if(exp == 1) return x;
  if(exp == 2) return x*x;
  const float p2 = lens_ipow(x, exp/2);
  if(exp &  1) return x * p2 * p2;
  return p2 * p2;
}
