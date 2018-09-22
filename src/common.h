// these are duplicates, lens.h is double in lentil repo
#pragma once

#include <math.h>


static inline float dotproduct(float *u, float *v)
{
  return ((u)[0]*(v)[0] + (u)[1]*(v)[1] + (u)[2]*(v)[2]);
}


static inline void crossproduct(const float *r, const float *u, float *v)
{
  v[0] = r[1]*u[2]-r[2]*u[1];
  v[1] = r[2]*u[0]-r[0]*u[2];
  v[2] = r[0]*u[1]-r[1]*u[0];
}


static inline void normalise(float *v)
{
  const float ilen = 1.0f/sqrtf(dotproduct(v,v));
  for(int k=0;k<3;k++) v[k] *= ilen;
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
  float normal[3] =
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
  float normal[3] =
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


// untested and probably wrong
static inline void lens_csToCylinder(const float *inpos, const float *indir, float *outpos, float *outdir, const float center, const float R, bool cyl_y)
{
  float normal[3] = {0.0f};
  if (cyl_y){
    normal[0] = inpos[0]/R;
    normal[1] = 0.0f;
    normal[2] = fabsf((inpos[2] - center)/R);
  } else {
    normal[0] = 0.0f;
    normal[1] = inpos[1]/R;
    normal[2] = fabsf((inpos[2] - center)/R);
  }
  float tempDir[3] = {indir[0], indir[1], indir[2]};
  normalise(tempDir);

  // tangent
  float ex[3] = {normal[2], 0, -normal[0]};
  normalise(ex);
  
  // bitangent
  float ey[3];
  crossproduct(ey, normal, ex);
  
  // store ray direction as projected position on unit disk perpendicular to the normal
  outdir[0] = dotproduct(tempDir, ex);
  outdir[1] = dotproduct(tempDir, ey);
  outpos[0] = inpos[0];
  outpos[1] = inpos[1];
}

// untested and probably wrong
static inline void lens_cylinderToCs(const float *inpos, const float *indir, float *outpos, float *outdir, const float center, const float R, bool cyl_y)
{
  float normal[3] = {0.0f};
  if (cyl_y){
    normal[0] = inpos[0]/R;
    normal[1] = 0.0f;
    normal[2] = sqrtf(MAX(0, R*R-inpos[0]*inpos[0]-inpos[1]*inpos[1]))/fabsf(R);
  } else {
    normal[0] = 0.0f;
    normal[1] = inpos[1]/R;
    normal[2] = sqrtf(MAX(0, R*R-inpos[0]*inpos[0]-inpos[1]*inpos[1]))/fabsf(R);
  }

  const float tempDir[3] = {indir[0], indir[1], sqrtf(MAX(0.0, 1.0f-indir[0]*indir[0]-indir[1]*indir[1]))};

  float ex[3] = {normal[2], 0, -normal[0]};
  normalise(ex);
  float ey[3];
  crossproduct(ey, normal, ex);

  outdir[0] = tempDir[0] * ex[0] + tempDir[1] * ey[0] + tempDir[2] * normal[0];
  outdir[1] = tempDir[0] * ex[1] + tempDir[1] * ey[1] + tempDir[2] * normal[1];
  outdir[2] = tempDir[0] * ex[2] + tempDir[1] * ey[2] + tempDir[2] * normal[2];
  outpos[0] = inpos[0];
  outpos[1] = inpos[1];
  outpos[2] = normal[2] * R + center;
}