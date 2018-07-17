#pragma once
#include <math.h>
#include <vector>


// xorshift fast random number generator
inline uint32_t xor128(void){
    static uint32_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
    uint32_t t = x ^ (x << 11);
    x = y; y = z; z = w;
    return w = (w ^ (w >> 19) ^ t ^ (t >> 8));
}


inline float Lerp(float t, float v1, float v2)
{
    return (1.0f - t) * v1 + t * v2;
}


// sin approximation, not completely accurate but faster than std::sin
inline float fastSin(float x){
    x = fmod(x + M_PI, M_PI * 2) - M_PI; // restrict x so that -M_PI < x < M_PI
    const float B = 4.0f / M_PI;
    const float C = -4.0f / (M_PI*M_PI);
    float y = B * x + C * x * std::abs(x);
    const float P = 0.225f;
    return P * (y * std::abs(y) - y) + y;
}


inline float fastCos(float x){
    // conversion from sin to cos
    x += M_PI * 0.5;

    x = fmod(x + M_PI, M_PI * 2) - M_PI; // restrict x so that -M_PI < x < M_PI
    const float B = 4.0f / M_PI;
    const float C = -4.0f / (M_PI*M_PI);
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
        phi = (M_PI/2.0f)-(0.78539816339f) * (a / b);
    }

    if (!fast_trigo){
        lens.x = r * std::cos(phi);
        lens.y = r * std::sin(phi);
    } else {
        lens.x = r * fastCos(phi);
        lens.y = r * fastSin(phi);
    }
}

// these are duplicates, lens.h is double in lentil repo
static inline float raytrace_dot(const float *u, const float *v)
{
  return ((u)[0]*(v)[0] + (u)[1]*(v)[1] + (u)[2]*(v)[2]);
}

// these are duplicates, lens.h is double in lentil repo
static inline void raytrace_cross(float *r, const float *u, const float *v)
{
  r[0] = u[1]*v[2]-u[2]*v[1];
  r[1] = u[2]*v[0]-u[0]*v[2];
  r[2] = u[0]*v[1]-u[1]*v[0];
}


static inline void raytrace_normalise(float *v)
{
  const float ilen = 1.0f/sqrtf(raytrace_dot(v,v));
  for(int k=0;k<3;k++) v[k] *= ilen;
}


static inline float dotproduct(float *u, float *v)
{
  return raytrace_dot(u, v);
}


static inline void crossproduct(const float *r, const float *u, float *v)
{
  return raytrace_cross(v, r, u);
}


static inline void normalise(float *v)
{
  return raytrace_normalise(v);
}


static inline float MAX(float a, float b)
{
  return a>b?a:b;
}


static inline void common_sincosf(float phi, float* sin, float* cos)
{
  *sin = std::sin(phi);
  *cos = std::cos(phi);
}


// helper function for dumped polynomials to compute integer powers of x:
static inline float lens_ipow(const float x, const int exp)
{
  if(exp == 0) return 1.0f;
  if(exp == 1) return x;
  if(exp == 2) return x*x;
  const float p2 = lens_ipow(x, exp/2);
  if(exp &  1) return x * p2 * p2;
  return p2 * p2;
}


static inline void lens_sphereToCs(const float *inpos, const float *indir, float *outpos, float *outdir, const float sphereCenter, const float sphereRad)
{
  const float normal[3] =
  {
    inpos[0]/sphereRad,
    inpos[1]/sphereRad,
    sqrtf(MAX(0, sphereRad*sphereRad-inpos[0]*inpos[0]-inpos[1]*inpos[1]))/fabsf(sphereRad)
  };
  const float tempDir[3] = {indir[0], indir[1], sqrtf(MAX(0.0, 1.0f-indir[0]*indir[0]-indir[1]*indir[1]))};

  float ex[3] = {normal[2], 0, -normal[0]};
  normalise(ex);
  float ey[3];
  crossproduct(normal, ex, ey);

  outdir[0] = tempDir[0] * ex[0] + tempDir[1] * ey[0] + tempDir[2] * normal[0];
  outdir[1] = tempDir[0] * ex[1] + tempDir[1] * ey[1] + tempDir[2] * normal[1];
  outdir[2] = tempDir[0] * ex[2] + tempDir[1] * ey[2] + tempDir[2] * normal[2];
  outpos[0] = inpos[0];
  outpos[1] = inpos[1];
  outpos[2] = normal[2] * sphereRad + sphereCenter;
}


static inline void lens_csToSphere(const float *inpos, const float *indir, float *outpos, float *outdir, const float sphereCenter, const float sphereRad)
{
  const float normal[3] =
  {
    inpos[0]/sphereRad,
    inpos[1]/sphereRad,
    fabsf((inpos[2]-sphereCenter)/sphereRad)
  };
  float tempDir[3] = {indir[0], indir[1], indir[2]};
  normalise(tempDir);

  float ex[3] = {normal[2], 0, -normal[0]};
  normalise(ex);
  float ey[3];
  crossproduct(normal, ex, ey);
  outdir[0] = dotproduct(tempDir, ex);
  outdir[1] = dotproduct(tempDir, ey);
  outpos[0] = inpos[0];
  outpos[1] = inpos[1];
}


static inline void lens_sample_aperture(float *x, float *y, float r1, float r2, const float radius, const int blades)
{
  const int tri = (int)(r1*blades);
  // rescale:
  r1 = r1*blades - tri;

  // sample triangle:
  float a = sqrtf(r1);
  float b = (1.0f-r2)*a;
  float c = r2*a;

  float p1[2], p2[2];

  common_sincosf(2.0f*M_PI/blades * (tri+1), p1, p1+1);
  common_sincosf(2.0f*M_PI/blades * tri, p2, p2+1);

  *x = radius * (b * p1[1] + c * p2[1]);
  *y = radius * (b * p1[0] + c * p2[0]);
}


/*
static inline int lens_clip_aperture(const float x, const float y, const float radius, const int blades)
{ 
  // early out
  if(x*x + y*y > radius*radius) return 0;
  float xx = radius; 
  float yy = 0.0f;
  for(int b=1;b<blades+1;b++)
  {      
    float tmpx, tmpy;
    common_sincosf(2.0f*(float)M_PI/blades * b, &tmpy, &tmpx);
    tmpx *= radius;
    tmpy *= radius;
    const float normalx = xx + tmpx;
    const float normaly = yy + tmpy;
    float dot0 = (normalx)*(x-xx) + (normaly)*(y-yy);
    if(dot0 > 0.0f) return 0;
    xx = tmpx;
    yy = tmpy;
  }
  return 1;
}*/


/*
static inline float lens_det_aperture_to_sensor(const float *sensor, const float focus)
{ 
  float J[25];
  lens_evaluate_aperture_jacobian(sensor, J);
  // only interested in how the directional density at the sensor changes wrt the vertex area (spatial) at the aperture
  float T[25] = {
    1., 0., focus, 0., 0.,
    0., 1., 0., focus, 0.,
    0., 0., 1., 0., 0.,
    0., 0., 0., 1., 0.,
    0., 0., 0., 0., 1.};
  float JT[25] = {0.};
  for(int i=2;i<4;i++) // only interested in 2x2 subblock.
    for(int j=0;j<2;j++)
      for(int k=0;k<4;k++)
        JT[i+5*j] += J[k + 5*j] * T[i + 5*k];
  const float det = fabsf(JT[2] * JT[5+3] - JT[3] * JT[5+2]);
  // there are two spatial components which need conversion to dm:
  const float dm2mm = 100.0f;
  return dm2mm*dm2mm/det;
}*/

/*
static inline float lens_aperture_area(const float radius, const int blades)
{
  const float tri = .5f*radius * radius * sinf(2.0f*AI_PI/(float)blades);
  return blades * tri;
}*/



// returns sensor offset in mm
// traces rays backwards through the lens
float camera_set_focus(float dist, MyCameraData *camera_data)
{
  const float target[3] = { 0.0, 0.0, dist};
  float sensor[5] = {0.0f};
  float out[5] = {0.0f};
  sensor[4] = camera_data->lambda;
  float offset = 0.0f;
  int count = 0;
  float scale_samples = 0.1f;
  float aperture[2] = {0.0f, 0.0f};

  const int S = 4;

  // trace a couple of adjoint rays from there to the sensor and
  // see where we need to put the sensor plane.
  for(int s=1; s<=S; s++){
    for(int k=0; k<2; k++){
      
      // reset aperture
      aperture[0] = aperture[1] = 0.0f;

      aperture[k] = camera_data->lens_aperture_housing_radius * (s/(S+1.0f) * scale_samples); // (1to4)/(4+1) = (.2, .4, .6, .8) * scale_samples

      lens_lt_sample_aperture(target, aperture, sensor, out, camera_data->lambda, camera_data);

      if(sensor[2+k] > 0){
          offset += sensor[k]/sensor[2+k];
          printf("\t[LENTIL] raytraced sensor shift at aperture[%f, %f]: %f", aperture[0], aperture[1], sensor[k]/sensor[2+k]);
          count ++;
      }
    }
  }

  // average guesses
  offset /= count; 
  
  // the focus plane/sensor offset:
  // negative because of reverse direction
  if(offset == offset){ // check NaN cases
    const float limit = 45.0f;
    if(offset > limit){
      printf("[LENTIL] sensor offset bigger than maxlimit: %f > %f", offset, limit);
      return limit;
    } else if(offset < -limit){
      printf("[LENTIL] sensor offset smaller than minlimit: %f < %f", offset, -limit);
      return -limit;
    } else {
      return offset; // in mm
    }
  }
}



// returns sensor offset in mm
float camera_set_focus_infinity(MyCameraData *camera_data)
{
	float parallel_ray_height = camera_data->lens_aperture_housing_radius * 0.1;
  const float target[3] = { 0.0, parallel_ray_height, AI_BIG};
  float sensor[5] = {0.0f};
  float out[5] = {0.0f};
  sensor[4] = camera_data->lambda;
  float offset = 0.0f;
  int count = 0;

  // just point through center of aperture
  float aperture[2] = {0.0f, parallel_ray_height};

  const int S = 4;

  // trace a couple of adjoint rays from there to the sensor and
  // see where we need to put the sensor plane.
  for(int s=1; s<=S; s++){
    for(int k=0; k<2; k++){
      
      // reset aperture
      aperture[0] = 0.0f;
      aperture[1] = parallel_ray_height;

      lens_lt_sample_aperture(target, aperture, sensor, out, camera_data->lambda, camera_data);

      if(sensor[2+k] > 0){
          offset += sensor[k]/sensor[2+k];
          count ++;
      }
    }
  }

  offset /= count; 
  
  // check NaN cases
  if(offset == offset){
    return offset;
  }
}



std::vector<float> logarithmic_values()
{
    float min = 0.0;
    float max = 45.0;
    float exponent = 2.0; // sharpness
    std::vector<float> log;

    for(float i = -1.0; i <= 1.0; i += 0.0001) {
        log.push_back((i < 0 ? -1 : 1) * std::pow(i, exponent) * (max - min) + min);
    }

    return log;
}