#pragma once

#include <math.h>


static inline float raytrace_dot(const float *u, const float *v)
{
  return ((u)[0]*(v)[0] + (u)[1]*(v)[1] + (u)[2]*(v)[2]);
}

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
#ifdef __APPLE__
  *sin = sinf(phi);
  *cos = cosf(phi);
#else
  sincosf(phi, sin, cos);
#endif
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


inline void load_lens_constants (MyCameraData *camera_data)
{
  switch (camera_data->lensModel){
    case petzval:
    {
      camera_data->lens_name = "petzval"; // descriptive name of the lens
      camera_data->lens_outer_pupil_radius = 11.250000; // scene facing radius in mm
      camera_data->lens_inner_pupil_radius = 6.750000; // sensor facing radius in mm
      camera_data->lens_length = 82.800003; // overall lens length in mm
      camera_data->lens_focal_length = 37.500000; // approximate lens focal length in mm (BFL)
      camera_data->lens_aperture_pos = 22.949999; // distance aperture -> outer pupil in mm
      camera_data->lens_aperture_housing_radius = 7.500000; // lens housing radius at the aperture
      camera_data->lens_outer_pupil_curvature_radius = 39.675003; // radius of curvature of the outer pupil
      camera_data->lens_field_of_view = 0.944800; // cosine of the approximate field of view assuming a 35mm image

    } break;

    // need to add a default case + other cases
  }

  AiMsgInfo("%s  [POTA] ----------  LENS CONSTANTS  -----------", emoticon);
  AiMsgInfo("%s  [POTA] lens_length: %s", emoticon, camera_data->lens_name);
  AiMsgInfo("%s  [POTA] lens_outer_pupil_radius: %f", emoticon, camera_data->lens_outer_pupil_radius);
  AiMsgInfo("%s  [POTA] lens_inner_pupil_radius: %f", emoticon, camera_data->lens_inner_pupil_radius);
  AiMsgInfo("%s  [POTA] lens_length: %f", emoticon, camera_data->lens_length);
  AiMsgInfo("%s  [POTA] lens_focal_length: %f", emoticon, camera_data->lens_focal_length);
  AiMsgInfo("%s  [POTA] lens_aperture_pos: %f", emoticon, camera_data->lens_aperture_pos);
  AiMsgInfo("%s  [POTA] lens_aperture_housing_radius: %f", emoticon, camera_data->lens_aperture_housing_radius);
  AiMsgInfo("%s  [POTA] lens_outer_pupil_curvature_radius: %f", emoticon, camera_data->lens_outer_pupil_curvature_radius);
  AiMsgInfo("%s  [POTA] lens_focal_length: %f", emoticon, camera_data->lens_focal_length);
  AiMsgInfo("%s  [POTA] lens_field_of_view: %f", emoticon, camera_data->lens_field_of_view);
  AiMsgInfo("%s  [POTA] --------------------------------------", emoticon);

}

// evaluates from sensor (in) to outer pupil (out).
// input arrays are 5d [x,y,dx,dy,lambda] where dx and dy are the direction in
// two-plane parametrization (that is the third component of the direction would be 1.0).
// units are millimeters for lengths and micrometers for the wavelength (so visible light is about 0.4--0.7)
// returns the transmittance computed from the polynomial.
static inline float lens_evaluate(const float *in, float *out, MyCameraData *camera_data)
{
  __attribute__ ((unused)) const float x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];
//#include "pt_evaluate.h"

  float out_transmittance = 0.0f;
  switch (camera_data->lensModel){
    case petzval:
    {
      out[0] =  + 61.6861 *dx + 0.516318 *x + 0.239174 *x*lambda + 6.09756 *dx*lambda + 0.0418018 *y*dx*dy + 0.0291763 *x*y*dy + 0.0384633 *lens_ipow(x, 2)*dx + -41.8684 *lens_ipow(dx, 3) + -0.16516 *x*lens_ipow(lambda, 2) + -41.0878 *dx*lens_ipow(dy, 2) + 0.000319801 *x*lens_ipow(y, 2) + 0.000310337 *lens_ipow(x, 3) + 0.431597 *x*lens_ipow(dy, 2) + 0.417681 *x*lens_ipow(dx, 2) + 0.0106198 *lens_ipow(y, 2)*dx + -4.03513 *dx*lens_ipow(lambda, 3) + 1.11768e-05 *x*lens_ipow(y, 2)*lambda + -0.000382566 *lens_ipow(x, 2)*dx*lambda + -8.637e-05 *lens_ipow(x, 2)*y*dx*dy*lambda + 5.14981e-06 *lens_ipow(x, 7)*lens_ipow(dx, 2)*lens_ipow(lambda, 2) + 13819.6 *lens_ipow(dx, 9)*lens_ipow(lambda, 2) + 1.71189e-08 *lens_ipow(x, 5)*lens_ipow(y, 3)*lens_ipow(dx, 2)*dy + 3.21537e-10 *lens_ipow(x, 9)*lens_ipow(lambda, 2) + 0.00130788 *lens_ipow(x, 3)*lens_ipow(y, 2)*lens_ipow(dx, 4)*lens_ipow(dy, 2) + 0.000150672 *lens_ipow(x, 6)*lens_ipow(dx, 3)*lens_ipow(lambda, 2) + 5.82064e-14 *lens_ipow(x, 7)*lens_ipow(y, 4) + -0.0568649 *lens_ipow(x, 4)*lens_ipow(dx, 5)*lens_ipow(lambda, 2) + 6.75549e-08 *lens_ipow(x, 8)*dx*lens_ipow(lambda, 2);
      out[1] =  + 0.453506 *y + 59.1587 *dy + 19.1364 *dy*lambda + 0.592232 *y*lambda + 0.411922 *y*lens_ipow(dx, 2) + 0.0392662 *lens_ipow(y, 2)*dy + 0.451829 *y*lens_ipow(dy, 2) + 0.0283685 *x*y*dx + -42.1243 *lens_ipow(dx, 2)*dy + -0.817315 *y*lens_ipow(lambda, 2) + 0.000312315 *lens_ipow(x, 2)*y + -19.7228 *dy*lens_ipow(lambda, 2) + 0.000313434 *lens_ipow(y, 3) + 0.0101854 *lens_ipow(x, 2)*dy + -41.478 *lens_ipow(dy, 3) + 0.395356 *y*lens_ipow(lambda, 3) + 1.6101e-05 *lens_ipow(y, 3)*lambda + 8.49311e-06 *lens_ipow(x, 2)*y*lambda + 7.11498 *dy*lens_ipow(lambda, 4) + -7.43062e-05 *lens_ipow(y, 3)*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + -9.90812e-06 *lens_ipow(y, 5)*lens_ipow(dx, 2)*lens_ipow(dy, 2) + 1.08674e-14 *lens_ipow(x, 2)*lens_ipow(y, 9) + 2.57069e-11 *lens_ipow(y, 9)*lens_ipow(dx, 2) + 2.33768e-09 *lens_ipow(x, 4)*lens_ipow(y, 4)*lens_ipow(dx, 2)*dy + 1.52162e-14 *lens_ipow(x, 6)*lens_ipow(y, 5) + -0.313837 *lens_ipow(x, 2)*y*lens_ipow(dy, 8) + 4.28086e-05 *x*lens_ipow(y, 5)*dx*lens_ipow(dy, 4) + 0.00203743 *x*lens_ipow(y, 4)*dx*lens_ipow(dy, 5);
      out[2] =  + -1.63943 *dx + -0.0305953 *x + 0.0323359 *dx*lambda + -0.0004223 *lens_ipow(x, 2)*dx + 1.74614e-06 *x*lens_ipow(y, 2) + 4.9289 *lens_ipow(dx, 3)*lambda + -0.0847201 *x*lens_ipow(dy, 2)*lambda + -1.54502e-05 *x*lens_ipow(y, 2)*lambda + -8.9172e-06 *lens_ipow(x, 3)*lambda + -0.00275435 *x*y*dy*lambda + -0.000390955 *lens_ipow(x, 2)*dx*lambda + -0.000707983 *lens_ipow(y, 2)*dx*lambda + -2.05796 *dx*lens_ipow(dy, 2)*lambda + -0.0998601 *y*dx*dy*lambda + 0.0184584 *x*lens_ipow(dx, 4) + 0.0249779 *y*dx*dy*lens_ipow(lambda, 2) + -8.31442 *lens_ipow(dx, 3)*lens_ipow(lambda, 2) + 1.57657 *lens_ipow(dx, 5) + 4.83602e-06 *lens_ipow(x, 3)*lens_ipow(lambda, 2) + 0.0421838 *x*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + 0.00116668 *x*y*dy*lens_ipow(lambda, 2) + -0.020373 *x*lens_ipow(dx, 2)*lens_ipow(lambda, 3) + 4.0944 *lens_ipow(dx, 3)*lens_ipow(lambda, 3) + -1.18558 *x*y*lens_ipow(dx, 4)*lens_ipow(dy, 3) + 20.8038 *dx*lens_ipow(dy, 8) + 2134.98 *lens_ipow(dx, 5)*lens_ipow(dy, 4) + 0.000186958 *lens_ipow(x, 2)*lens_ipow(y, 2)*lens_ipow(dx, 3)*lens_ipow(dy, 2) + 0.000963685 *x*lens_ipow(lambda, 9);
      out[3] =  + -0.030348 *y + -1.6733 *dy + 0.226598 *dy*lambda + -0.000739868 *y*lambda + 0.0043513 *y*lens_ipow(dx, 2) + -0.000633417 *lens_ipow(y, 2)*dy + -0.00400459 *y*lens_ipow(dy, 2) + 0.0669129 *x*dx*dy + -9.59075e-05 *x*y*dx + 2.91954 *lens_ipow(dx, 2)*dy + 0.000576135 *y*lens_ipow(lambda, 2) + -0.334116 *dy*lens_ipow(lambda, 2) + -3.05205e-06 *lens_ipow(y, 3) + 0.000314298 *lens_ipow(x, 2)*dy + 0.885659 *lens_ipow(dy, 3) + 7.50285e-05 *lens_ipow(x, 2)*dy*lambda + -0.113294 *lens_ipow(dy, 3)*lambda + 0.176171 *dy*lens_ipow(lambda, 3) + -6.45139e-07 *lens_ipow(y, 3)*lambda + 4.94668e-05 *x*y*dx*lambda + 0.00640416 *x*dx*dy*lambda + -0.0369238 *x*lens_ipow(dx, 3)*dy + 2.5536 *lens_ipow(dx, 2)*lens_ipow(dy, 3) + 1.94879 *lens_ipow(dy, 5) + -0.000400921 *x*y*dx*lens_ipow(dy, 2) + 0.0245005 *y*lens_ipow(dy, 4) + 5.94929e-06 *lens_ipow(x, 3)*dx*dy + 0.011227 *y*lens_ipow(dx, 4)*lambda;
      out_transmittance =  + 0.59399  + 0.836383 *lambda + -0.000344805 *x*dx + -7.02536e-06 *lens_ipow(x, 2) + -1.73936 *lens_ipow(lambda, 2) + 1.70047 *lens_ipow(lambda, 3) + -0.644121 *lens_ipow(lambda, 4) + -0.150549 *lens_ipow(dx, 4) + -0.449125 *lens_ipow(dx, 2)*lens_ipow(dy, 2) + -1.08274e-05 *lens_ipow(x, 2)*y*dy + -1.09547e-05 *x*lens_ipow(y, 2)*dx + -4.56631e-07 *lens_ipow(x, 2)*lens_ipow(y, 2) + -3.11249e-07 *lens_ipow(y, 4) + -0.00046016 *lens_ipow(y, 2)*lens_ipow(dy, 2) + -2.35642e-05 *lens_ipow(y, 3)*dy + -2.22792e-09 *lens_ipow(x, 6) + -1.06372e-07 *lens_ipow(x, 5)*dx + -1.21435e-10 *lens_ipow(y, 6) + -5.63631e-10 *lens_ipow(x, 7)*dx + -4.88522e-12 *lens_ipow(x, 4)*lens_ipow(y, 4) + 1.25574e-08 *lens_ipow(x, 3)*lens_ipow(y, 3)*dx*dy + -2.84961e-08 *lens_ipow(x, 6)*lens_ipow(dx, 2) + -3.7395e-13 *lens_ipow(x, 2)*lens_ipow(y, 6) + 2.03261e-06 *lens_ipow(y, 4)*lens_ipow(dy, 4)*lambda + 7.20677e-05 *lens_ipow(x, 4)*lens_ipow(dx, 6) + -3.53471e-15 *lens_ipow(x, 8)*lens_ipow(y, 2) + -3.64078e-15 *lens_ipow(x, 10) + 3.93018e-08 *lens_ipow(x, 5)*y*lens_ipow(dx, 3)*dy*lambda;
    } break;
  }

  return MAX(0.0f, out_transmittance);
}

/*
// evaluates from the sensor (in) to the aperture (out) only
// returns the transmittance.
static inline float lens_evaluate_aperture(const float *in, float *out)
{
  __attribute__ ((unused)) const float x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];
#include "pt_evaluate_aperture.h"
  out[0] = out_x; out[1] = out_y; out[2] = out_dx; out[3] = out_dy;
  return MAX(0.0f, out_transmittance);
}
*/


// solves for the two directions [dx,dy], keeps the two positions [x,y] and the
// wavelength, such that the path through the lens system will be valid, i.e.
// lens_evaluate_aperture(in, out) will yield the same out given the solved for in.
// in: point on sensor. out: point on aperture.
static inline void lens_pt_sample_aperture(float *in, float *out, float dist, MyCameraData *camera_data)
{
  __attribute__ ((unused)) float out_x = out[0], out_y = out[1], out_dx = out[2], out_dy = out[3], out_transmittance = 1.0f;
  __attribute__ ((unused)) float x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];

//#include "pt_sample_aperture.h"
  switch (camera_data->lensModel){
    case petzval:
    {
      float pred_x;
      float pred_y;
      float pred_dx;
      float pred_dy;
      float sqr_err = FLT_MAX;
      for(int k=0;k<5&&sqr_err > 1e-4f;k++)
      {
        const float begin_x = x + dist * dx;
        const float begin_y = y + dist * dy;
        const float begin_dx = dx;
        const float begin_dy = dy;
        __attribute__((unused)) const float begin_lambda = lambda;
        pred_x =  + 47.9583 *begin_dx + 0.726604 *begin_x + 0.30773 *begin_x*begin_lambda + 10.0443 *begin_dx*begin_lambda + 0.51757 *begin_y*begin_dx*begin_dy + 0.0130025 *begin_x*begin_y*begin_dy + 0.0209696 *lens_ipow(begin_x, 2)*begin_dx + -0.428152 *begin_x*lens_ipow(begin_lambda, 2) + 0.0001599 *begin_x*lens_ipow(begin_y, 2) + 0.000146126 *lens_ipow(begin_x, 3) + 0.109933 *begin_x*lens_ipow(begin_dy, 2) + 0.60574 *begin_x*lens_ipow(begin_dx, 2) + 0.00896142 *lens_ipow(begin_y, 2)*begin_dx + -10.4332 *begin_dx*lens_ipow(begin_lambda, 2) + 0.208867 *begin_x*lens_ipow(begin_lambda, 3) + 2.55743e-05 *lens_ipow(begin_x, 4)*begin_dx + 3.82423 *begin_dx*lens_ipow(begin_lambda, 4) + 2.4372e-07 *lens_ipow(begin_x, 5) + -0.000238119 *lens_ipow(begin_x, 2)*begin_y*begin_dx*begin_dy + 0.000614863 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2) + -3.80606e-05 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + 0.836959 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*begin_lambda + 2.60785e-08 *lens_ipow(begin_x, 5)*begin_lambda + 2.35096e-10 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 2) + -1.07059e-06 *lens_ipow(begin_x, 4)*begin_y*begin_dx*begin_dy*begin_lambda + -3.9565e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 5.01276e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*lens_ipow(begin_lambda, 3) + -4.29584e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*lens_ipow(begin_lambda, 4);
        pred_y =  + 0.727662 *begin_y + 47.9805 *begin_dy + 9.94011 *begin_dy*begin_lambda + 0.303795 *begin_y*begin_lambda + 0.116675 *begin_y*lens_ipow(begin_dx, 2) + 0.0212751 *lens_ipow(begin_y, 2)*begin_dy + 0.606324 *begin_y*lens_ipow(begin_dy, 2) + 0.541493 *begin_x*begin_dx*begin_dy + 0.0133007 *begin_x*begin_y*begin_dx + 0.525397 *lens_ipow(begin_dx, 2)*begin_dy + -0.422576 *begin_y*lens_ipow(begin_lambda, 2) + 0.000155139 *lens_ipow(begin_x, 2)*begin_y + -10.2959 *begin_dy*lens_ipow(begin_lambda, 2) + 0.000145719 *lens_ipow(begin_y, 3) + 0.00910908 *lens_ipow(begin_x, 2)*begin_dy + 0.205574 *begin_y*lens_ipow(begin_lambda, 3) + -0.000530731 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + 9.11966e-06 *lens_ipow(begin_x, 2)*begin_y*begin_lambda + 0.00188791 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2)*begin_dy + -0.000453148 *begin_x*lens_ipow(begin_y, 2)*begin_dx*begin_dy + 3.75232 *begin_dy*lens_ipow(begin_lambda, 4) + 7.66467e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + 0.54031 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 4.37653e-05 *lens_ipow(begin_y, 4)*begin_dy*begin_lambda + 0.00104185 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2)*begin_lambda + 4.47109e-07 *lens_ipow(begin_y, 5)*begin_lambda + 8.89044e-10 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3)*lens_ipow(begin_lambda, 3) + -3.83891e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*begin_dx*begin_dy*lens_ipow(begin_lambda, 4);
        pred_dx =  + 0.489166 *begin_dx + -0.0138238 *begin_x + 0.0158286 *begin_x*begin_lambda + 0.469504 *begin_dx*begin_lambda + 0.00808112 *begin_y*begin_dx*begin_dy + 0.000528987 *begin_x*begin_y*begin_dy + 0.000694739 *lens_ipow(begin_x, 2)*begin_dx + -0.341088 *lens_ipow(begin_dx, 3) + -0.0220346 *begin_x*lens_ipow(begin_lambda, 2) + -0.33104 *begin_dx*lens_ipow(begin_dy, 2) + 6.53379e-06 *begin_x*lens_ipow(begin_y, 2) + 5.59884e-06 *lens_ipow(begin_x, 3) + 0.00268339 *begin_x*lens_ipow(begin_dy, 2) + 0.00916356 *begin_x*lens_ipow(begin_dx, 2) + 0.00022903 *lens_ipow(begin_y, 2)*begin_dx + -0.4885 *begin_dx*lens_ipow(begin_lambda, 2) + 0.0107461 *begin_x*lens_ipow(begin_lambda, 3) + 3.4652e-07 *lens_ipow(begin_x, 3)*begin_lambda + 9.87083e-07 *lens_ipow(begin_x, 4)*begin_dx + 0.179614 *begin_dx*lens_ipow(begin_lambda, 4) + 1.02144e-08 *lens_ipow(begin_x, 5) + 9.19451e-05 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 2) + 2.29793e-05 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2) + 0.0140785 *begin_x*begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + -27.9691 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 4) + -8.02962e-05 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dx, 3)*begin_dy + 2.2643e-11 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*begin_lambda + 2.26493e-14 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 2);
        pred_dy =  + -0.0137889 *begin_y + 0.490712 *begin_dy + 0.461858 *begin_dy*begin_lambda + 0.0156755 *begin_y*begin_lambda + 0.00261594 *begin_y*lens_ipow(begin_dx, 2) + 0.000728088 *lens_ipow(begin_y, 2)*begin_dy + 0.00969924 *begin_y*lens_ipow(begin_dy, 2) + 0.00798963 *begin_x*begin_dx*begin_dy + 0.000529122 *begin_x*begin_y*begin_dx + -0.341312 *lens_ipow(begin_dx, 2)*begin_dy + -0.0217976 *begin_y*lens_ipow(begin_lambda, 2) + 6.30389e-06 *lens_ipow(begin_x, 2)*begin_y + -0.477862 *begin_dy*lens_ipow(begin_lambda, 2) + 5.9506e-06 *lens_ipow(begin_y, 3) + 0.000227942 *lens_ipow(begin_x, 2)*begin_dy + -0.346063 *lens_ipow(begin_dy, 3) + 0.01061 *begin_y*lens_ipow(begin_lambda, 3) + 4.20858e-07 *lens_ipow(begin_y, 3)*begin_lambda + 4.18636e-07 *lens_ipow(begin_x, 2)*begin_y*begin_lambda + 7.75236e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2)*begin_dy + 0.173747 *begin_dy*lens_ipow(begin_lambda, 4) + 4.92036e-11 *lens_ipow(begin_y, 7) + 4.91915e-09 *lens_ipow(begin_y, 6)*begin_dy + 1.20536e-07 *lens_ipow(begin_y, 5)*lens_ipow(begin_dy, 2) + 0.00352233 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 4) + 2.7495e-13 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 5) + -9.16196e-10 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*begin_dx*begin_dy + -4051.9 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 7);
        float dx1_domega0[2][2];
        dx1_domega0[0][0] =  + 47.9583  + 10.0443 *begin_lambda + 0.51757 *begin_y*begin_dy + 0.0209696 *lens_ipow(begin_x, 2) + 1.21148 *begin_x*begin_dx + 0.00896142 *lens_ipow(begin_y, 2) + -10.4332 *lens_ipow(begin_lambda, 2) + 2.55743e-05 *lens_ipow(begin_x, 4) + 3.82423 *lens_ipow(begin_lambda, 4) + -0.000238119 *lens_ipow(begin_x, 2)*begin_y*begin_dy + 0.00122973 *lens_ipow(begin_x, 3)*begin_dx + 1.67392 *begin_x*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + -1.07059e-06 *lens_ipow(begin_x, 4)*begin_y*begin_dy*begin_lambda + -3.9565e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_dy*lens_ipow(begin_lambda, 2)+0.0f;
        dx1_domega0[0][1] =  + 0.51757 *begin_y*begin_dx + 0.0130025 *begin_x*begin_y + 0.219866 *begin_x*begin_dy + -0.000238119 *lens_ipow(begin_x, 2)*begin_y*begin_dx + -7.61212e-05 *begin_x*lens_ipow(begin_y, 2)*begin_dy + 1.67392 *begin_x*lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + -1.07059e-06 *lens_ipow(begin_x, 4)*begin_y*begin_dx*begin_lambda + -3.9565e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_lambda, 2)+0.0f;
        dx1_domega0[1][0] =  + 0.233349 *begin_y*begin_dx + 0.541493 *begin_x*begin_dy + 0.0133007 *begin_x*begin_y + 1.05079 *begin_dx*begin_dy + 0.00377582 *lens_ipow(begin_x, 2)*begin_dx*begin_dy + -0.000453148 *begin_x*lens_ipow(begin_y, 2)*begin_dy + 1.08062 *begin_y*begin_dx*lens_ipow(begin_dy, 2) + -3.83891e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*begin_dy*lens_ipow(begin_lambda, 4)+0.0f;
        dx1_domega0[1][1] =  + 47.9805  + 9.94011 *begin_lambda + 0.0212751 *lens_ipow(begin_y, 2) + 1.21265 *begin_y*begin_dy + 0.541493 *begin_x*begin_dx + 0.525397 *lens_ipow(begin_dx, 2) + -10.2959 *lens_ipow(begin_lambda, 2) + 0.00910908 *lens_ipow(begin_x, 2) + -0.000530731 *lens_ipow(begin_y, 2)*begin_lambda + 0.00188791 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + -0.000453148 *begin_x*lens_ipow(begin_y, 2)*begin_dx + 3.75232 *lens_ipow(begin_lambda, 4) + 1.08062 *begin_y*lens_ipow(begin_dx, 2)*begin_dy + 4.37653e-05 *lens_ipow(begin_y, 4)*begin_lambda + 0.00208371 *lens_ipow(begin_y, 3)*begin_dy*begin_lambda + -3.83891e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_lambda, 4)+0.0f;
        float invJ[2][2];
        const float invdet = 1.0f/(dx1_domega0[0][0]*dx1_domega0[1][1] - dx1_domega0[0][1]*dx1_domega0[1][0]);
        invJ[0][0] =  dx1_domega0[1][1]*invdet;
        invJ[1][1] =  dx1_domega0[0][0]*invdet;
        invJ[0][1] = -dx1_domega0[0][1]*invdet;
        invJ[1][0] = -dx1_domega0[1][0]*invdet;
        const float dx1[2] = {out_x - pred_x, out_y - pred_y};
        for(int i=0;i<2;i++)
        {
          dx += invJ[0][i]*dx1[i];
          dy += invJ[1][i]*dx1[i];
        }
        sqr_err = dx1[0]*dx1[0] + dx1[1]*dx1[1];
      }
      out_dx = pred_dx;
      out_dy = pred_dy;
    } break;
  }



  // directions may have changed, copy all to be sure.
  out[0] = out_x;
  out[1] = out_y;
  out[2] = out_dx;
  out[3] = out_dy;

  in[0] = x;
  in[1] = y;
  in[2] = dx;
  in[3] = dy;
}



// solves for a sensor position given a scene point and an aperture point
// returns transmittance from sensor to outer pupil
static inline float lens_lt_sample_aperture(
    const float *scene,   // 3d point in scene in camera space
    const float *ap,      // 2d point on aperture (in camera space, z is known)
    float *sensor,        // output point and direction on sensor plane/plane
    float *out,           // output point and direction on outer pupil
    const float lambda,   // wavelength
    MyCameraData *camera_data)   
{
  const float scene_x = scene[0], scene_y = scene[1], scene_z = scene[2];
  const float ap_x = ap[0], ap_y = ap[1];
  float x = 0, y = 0, dx = 0, dy = 0;
//#include "lt_sample_aperture.h"


  switch (camera_data->lensModel){
    case petzval:
    {
      float view[3] =
      {
        scene_x,
        scene_y,
        scene_z + camera_data->lens_outer_pupil_curvature_radius
      };
      normalise(view);
      int error = 0;
      if(1 || view[2] >= camera_data->lens_field_of_view)
      {
        const float eps = 1e-8;
        float sqr_err = 1e30, sqr_ap_err = 1e30;
        float prev_sqr_err = 1e32, prev_sqr_ap_err = 1e32;
        for(int k=0;k<100&&(sqr_err>eps||sqr_ap_err>eps)&&error==0;k++)
        {
          prev_sqr_err = sqr_err, prev_sqr_ap_err = sqr_ap_err;
          const float begin_x = x;
          const float begin_y = y;
          const float begin_dx = dx;
          const float begin_dy = dy;
          const float begin_lambda = lambda;
          const float pred_ap[2] = {
             + 47.9583 *begin_dx + 0.726604 *begin_x + 0.30773 *begin_x*begin_lambda + 10.0443 *begin_dx*begin_lambda + 0.51757 *begin_y*begin_dx*begin_dy + 0.0130025 *begin_x*begin_y*begin_dy + 0.0209696 *lens_ipow(begin_x, 2)*begin_dx + -0.428152 *begin_x*lens_ipow(begin_lambda, 2) + 0.0001599 *begin_x*lens_ipow(begin_y, 2) + 0.000146126 *lens_ipow(begin_x, 3) + 0.109933 *begin_x*lens_ipow(begin_dy, 2) + 0.60574 *begin_x*lens_ipow(begin_dx, 2) + 0.00896142 *lens_ipow(begin_y, 2)*begin_dx + -10.4332 *begin_dx*lens_ipow(begin_lambda, 2) + 0.208867 *begin_x*lens_ipow(begin_lambda, 3) + 2.55743e-05 *lens_ipow(begin_x, 4)*begin_dx + 3.82423 *begin_dx*lens_ipow(begin_lambda, 4) + 2.4372e-07 *lens_ipow(begin_x, 5) + -0.000238119 *lens_ipow(begin_x, 2)*begin_y*begin_dx*begin_dy + 0.000614863 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2) + -3.80606e-05 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + 0.836959 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*begin_lambda + 2.60785e-08 *lens_ipow(begin_x, 5)*begin_lambda + 2.35096e-10 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 2) + -1.07059e-06 *lens_ipow(begin_x, 4)*begin_y*begin_dx*begin_dy*begin_lambda + -3.9565e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 5.01276e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*lens_ipow(begin_lambda, 3) + -4.29584e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*lens_ipow(begin_lambda, 4),
             + 0.727662 *begin_y + 47.9805 *begin_dy + 9.94011 *begin_dy*begin_lambda + 0.303795 *begin_y*begin_lambda + 0.116675 *begin_y*lens_ipow(begin_dx, 2) + 0.0212751 *lens_ipow(begin_y, 2)*begin_dy + 0.606324 *begin_y*lens_ipow(begin_dy, 2) + 0.541493 *begin_x*begin_dx*begin_dy + 0.0133007 *begin_x*begin_y*begin_dx + 0.525397 *lens_ipow(begin_dx, 2)*begin_dy + -0.422576 *begin_y*lens_ipow(begin_lambda, 2) + 0.000155139 *lens_ipow(begin_x, 2)*begin_y + -10.2959 *begin_dy*lens_ipow(begin_lambda, 2) + 0.000145719 *lens_ipow(begin_y, 3) + 0.00910908 *lens_ipow(begin_x, 2)*begin_dy + 0.205574 *begin_y*lens_ipow(begin_lambda, 3) + -0.000530731 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + 9.11966e-06 *lens_ipow(begin_x, 2)*begin_y*begin_lambda + 0.00188791 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2)*begin_dy + -0.000453148 *begin_x*lens_ipow(begin_y, 2)*begin_dx*begin_dy + 3.75232 *begin_dy*lens_ipow(begin_lambda, 4) + 7.66467e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + 0.54031 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 4.37653e-05 *lens_ipow(begin_y, 4)*begin_dy*begin_lambda + 0.00104185 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2)*begin_lambda + 4.47109e-07 *lens_ipow(begin_y, 5)*begin_lambda + 8.89044e-10 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3)*lens_ipow(begin_lambda, 3) + -3.83891e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*begin_dx*begin_dy*lens_ipow(begin_lambda, 4)
          };
          const float delta_ap[] = {ap_x - pred_ap[0], ap_y - pred_ap[1]};
          sqr_ap_err = delta_ap[0]*delta_ap[0]+delta_ap[1]*delta_ap[1];
          float dx1_domega0[2][2];
          dx1_domega0[0][0] =  + 47.9583  + 10.0443 *begin_lambda + 0.51757 *begin_y*begin_dy + 0.0209696 *lens_ipow(begin_x, 2) + 1.21148 *begin_x*begin_dx + 0.00896142 *lens_ipow(begin_y, 2) + -10.4332 *lens_ipow(begin_lambda, 2) + 2.55743e-05 *lens_ipow(begin_x, 4) + 3.82423 *lens_ipow(begin_lambda, 4) + -0.000238119 *lens_ipow(begin_x, 2)*begin_y*begin_dy + 0.00122973 *lens_ipow(begin_x, 3)*begin_dx + 1.67392 *begin_x*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + -1.07059e-06 *lens_ipow(begin_x, 4)*begin_y*begin_dy*begin_lambda + -3.9565e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_dy*lens_ipow(begin_lambda, 2)+0.0f;
          dx1_domega0[0][1] =  + 0.51757 *begin_y*begin_dx + 0.0130025 *begin_x*begin_y + 0.219866 *begin_x*begin_dy + -0.000238119 *lens_ipow(begin_x, 2)*begin_y*begin_dx + -7.61212e-05 *begin_x*lens_ipow(begin_y, 2)*begin_dy + 1.67392 *begin_x*lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + -1.07059e-06 *lens_ipow(begin_x, 4)*begin_y*begin_dx*begin_lambda + -3.9565e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_lambda, 2)+0.0f;
          dx1_domega0[1][0] =  + 0.233349 *begin_y*begin_dx + 0.541493 *begin_x*begin_dy + 0.0133007 *begin_x*begin_y + 1.05079 *begin_dx*begin_dy + 0.00377582 *lens_ipow(begin_x, 2)*begin_dx*begin_dy + -0.000453148 *begin_x*lens_ipow(begin_y, 2)*begin_dy + 1.08062 *begin_y*begin_dx*lens_ipow(begin_dy, 2) + -3.83891e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*begin_dy*lens_ipow(begin_lambda, 4)+0.0f;
          dx1_domega0[1][1] =  + 47.9805  + 9.94011 *begin_lambda + 0.0212751 *lens_ipow(begin_y, 2) + 1.21265 *begin_y*begin_dy + 0.541493 *begin_x*begin_dx + 0.525397 *lens_ipow(begin_dx, 2) + -10.2959 *lens_ipow(begin_lambda, 2) + 0.00910908 *lens_ipow(begin_x, 2) + -0.000530731 *lens_ipow(begin_y, 2)*begin_lambda + 0.00188791 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + -0.000453148 *begin_x*lens_ipow(begin_y, 2)*begin_dx + 3.75232 *lens_ipow(begin_lambda, 4) + 1.08062 *begin_y*lens_ipow(begin_dx, 2)*begin_dy + 4.37653e-05 *lens_ipow(begin_y, 4)*begin_lambda + 0.00208371 *lens_ipow(begin_y, 3)*begin_dy*begin_lambda + -3.83891e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_lambda, 4)+0.0f;
          float invApJ[2][2];
          const float invdetap = 1.0f/(dx1_domega0[0][0]*dx1_domega0[1][1] - dx1_domega0[0][1]*dx1_domega0[1][0]);
          invApJ[0][0] =  dx1_domega0[1][1]*invdetap;
          invApJ[1][1] =  dx1_domega0[0][0]*invdetap;
          invApJ[0][1] = -dx1_domega0[0][1]*invdetap;
          invApJ[1][0] = -dx1_domega0[1][0]*invdetap;
          for(int i=0;i<2;i++)
          {
            dx += invApJ[0][i]*delta_ap[i];
            dy += invApJ[1][i]*delta_ap[i];
          }
          out[0] =  + 61.6861 *begin_dx + 0.516318 *begin_x + 0.239174 *begin_x*begin_lambda + 6.09756 *begin_dx*begin_lambda + 0.0418018 *begin_y*begin_dx*begin_dy + 0.0291763 *begin_x*begin_y*begin_dy + 0.0384633 *lens_ipow(begin_x, 2)*begin_dx + -41.8684 *lens_ipow(begin_dx, 3) + -0.16516 *begin_x*lens_ipow(begin_lambda, 2) + -41.0878 *begin_dx*lens_ipow(begin_dy, 2) + 0.000319801 *begin_x*lens_ipow(begin_y, 2) + 0.000310337 *lens_ipow(begin_x, 3) + 0.431597 *begin_x*lens_ipow(begin_dy, 2) + 0.417681 *begin_x*lens_ipow(begin_dx, 2) + 0.0106198 *lens_ipow(begin_y, 2)*begin_dx + -4.03513 *begin_dx*lens_ipow(begin_lambda, 3) + 1.11768e-05 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + -0.000382566 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + -8.637e-05 *lens_ipow(begin_x, 2)*begin_y*begin_dx*begin_dy*begin_lambda + 5.14981e-06 *lens_ipow(begin_x, 7)*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + 13819.6 *lens_ipow(begin_dx, 9)*lens_ipow(begin_lambda, 2) + 1.71189e-08 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2)*begin_dy + 3.21537e-10 *lens_ipow(begin_x, 9)*lens_ipow(begin_lambda, 2) + 0.00130788 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2) + 0.000150672 *lens_ipow(begin_x, 6)*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + 5.82064e-14 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 4) + -0.0568649 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 5)*lens_ipow(begin_lambda, 2) + 6.75549e-08 *lens_ipow(begin_x, 8)*begin_dx*lens_ipow(begin_lambda, 2);
          out[1] =  + 0.453506 *begin_y + 59.1587 *begin_dy + 19.1364 *begin_dy*begin_lambda + 0.592232 *begin_y*begin_lambda + 0.411922 *begin_y*lens_ipow(begin_dx, 2) + 0.0392662 *lens_ipow(begin_y, 2)*begin_dy + 0.451829 *begin_y*lens_ipow(begin_dy, 2) + 0.0283685 *begin_x*begin_y*begin_dx + -42.1243 *lens_ipow(begin_dx, 2)*begin_dy + -0.817315 *begin_y*lens_ipow(begin_lambda, 2) + 0.000312315 *lens_ipow(begin_x, 2)*begin_y + -19.7228 *begin_dy*lens_ipow(begin_lambda, 2) + 0.000313434 *lens_ipow(begin_y, 3) + 0.0101854 *lens_ipow(begin_x, 2)*begin_dy + -41.478 *lens_ipow(begin_dy, 3) + 0.395356 *begin_y*lens_ipow(begin_lambda, 3) + 1.6101e-05 *lens_ipow(begin_y, 3)*begin_lambda + 8.49311e-06 *lens_ipow(begin_x, 2)*begin_y*begin_lambda + 7.11498 *begin_dy*lens_ipow(begin_lambda, 4) + -7.43062e-05 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + -9.90812e-06 *lens_ipow(begin_y, 5)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 1.08674e-14 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 9) + 2.57069e-11 *lens_ipow(begin_y, 9)*lens_ipow(begin_dx, 2) + 2.33768e-09 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*begin_dy + 1.52162e-14 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 5) + -0.313837 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dy, 8) + 4.28086e-05 *begin_x*lens_ipow(begin_y, 5)*begin_dx*lens_ipow(begin_dy, 4) + 0.00203743 *begin_x*lens_ipow(begin_y, 4)*begin_dx*lens_ipow(begin_dy, 5);
          out[2] =  + -1.63943 *begin_dx + -0.0305953 *begin_x + 0.0323359 *begin_dx*begin_lambda + -0.0004223 *lens_ipow(begin_x, 2)*begin_dx + 1.74614e-06 *begin_x*lens_ipow(begin_y, 2) + 4.9289 *lens_ipow(begin_dx, 3)*begin_lambda + -0.0847201 *begin_x*lens_ipow(begin_dy, 2)*begin_lambda + -1.54502e-05 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + -8.9172e-06 *lens_ipow(begin_x, 3)*begin_lambda + -0.00275435 *begin_x*begin_y*begin_dy*begin_lambda + -0.000390955 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + -0.000707983 *lens_ipow(begin_y, 2)*begin_dx*begin_lambda + -2.05796 *begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + -0.0998601 *begin_y*begin_dx*begin_dy*begin_lambda + 0.0184584 *begin_x*lens_ipow(begin_dx, 4) + 0.0249779 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -8.31442 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + 1.57657 *lens_ipow(begin_dx, 5) + 4.83602e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_lambda, 2) + 0.0421838 *begin_x*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.00116668 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 2) + -0.020373 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 3) + 4.0944 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 3) + -1.18558 *begin_x*begin_y*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 3) + 20.8038 *begin_dx*lens_ipow(begin_dy, 8) + 2134.98 *lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 4) + 0.000186958 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2) + 0.000963685 *begin_x*lens_ipow(begin_lambda, 9);
          out[3] =  + -0.030348 *begin_y + -1.6733 *begin_dy + 0.226598 *begin_dy*begin_lambda + -0.000739868 *begin_y*begin_lambda + 0.0043513 *begin_y*lens_ipow(begin_dx, 2) + -0.000633417 *lens_ipow(begin_y, 2)*begin_dy + -0.00400459 *begin_y*lens_ipow(begin_dy, 2) + 0.0669129 *begin_x*begin_dx*begin_dy + -9.59075e-05 *begin_x*begin_y*begin_dx + 2.91954 *lens_ipow(begin_dx, 2)*begin_dy + 0.000576135 *begin_y*lens_ipow(begin_lambda, 2) + -0.334116 *begin_dy*lens_ipow(begin_lambda, 2) + -3.05205e-06 *lens_ipow(begin_y, 3) + 0.000314298 *lens_ipow(begin_x, 2)*begin_dy + 0.885659 *lens_ipow(begin_dy, 3) + 7.50285e-05 *lens_ipow(begin_x, 2)*begin_dy*begin_lambda + -0.113294 *lens_ipow(begin_dy, 3)*begin_lambda + 0.176171 *begin_dy*lens_ipow(begin_lambda, 3) + -6.45139e-07 *lens_ipow(begin_y, 3)*begin_lambda + 4.94668e-05 *begin_x*begin_y*begin_dx*begin_lambda + 0.00640416 *begin_x*begin_dx*begin_dy*begin_lambda + -0.0369238 *begin_x*lens_ipow(begin_dx, 3)*begin_dy + 2.5536 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + 1.94879 *lens_ipow(begin_dy, 5) + -0.000400921 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 2) + 0.0245005 *begin_y*lens_ipow(begin_dy, 4) + 5.94929e-06 *lens_ipow(begin_x, 3)*begin_dx*begin_dy + 0.011227 *begin_y*lens_ipow(begin_dx, 4)*begin_lambda;
          float pred_out_cs[7] = {0.0f};
          lens_sphereToCs(out, out+2, pred_out_cs, pred_out_cs+3, - camera_data->lens_outer_pupil_curvature_radius, camera_data->lens_outer_pupil_curvature_radius);
          float view[3] =
          {
            scene_x - pred_out_cs[0],
            scene_y - pred_out_cs[1],
            scene_z - pred_out_cs[2]
          };
          normalise(view);
          float out_new[5];
          lens_csToSphere(pred_out_cs, view, out_new, out_new+2, - camera_data->lens_outer_pupil_curvature_radius, camera_data->lens_outer_pupil_curvature_radius);
          const float delta_out[] = {out_new[2] - out[2], out_new[3] - out[3]};
          sqr_err = delta_out[0]*delta_out[0]+delta_out[1]*delta_out[1];
          float domega2_dx0[2][2];
          domega2_dx0[0][0] =  + -0.0305953  + -0.000844601 *begin_x*begin_dx + 1.74614e-06 *lens_ipow(begin_y, 2) + -0.0847201 *lens_ipow(begin_dy, 2)*begin_lambda + -1.54502e-05 *lens_ipow(begin_y, 2)*begin_lambda + -2.67516e-05 *lens_ipow(begin_x, 2)*begin_lambda + -0.00275435 *begin_y*begin_dy*begin_lambda + -0.00078191 *begin_x*begin_dx*begin_lambda + 0.0184584 *lens_ipow(begin_dx, 4) + 1.45081e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 2) + 0.0421838 *lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.00116668 *begin_y*begin_dy*lens_ipow(begin_lambda, 2) + -0.020373 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 3) + -1.18558 *begin_y*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 3) + 0.000373915 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2) + 0.000963685 *lens_ipow(begin_lambda, 9)+0.0f;
          domega2_dx0[0][1] =  + 3.49228e-06 *begin_x*begin_y + -3.09004e-05 *begin_x*begin_y*begin_lambda + -0.00275435 *begin_x*begin_dy*begin_lambda + -0.00141597 *begin_y*begin_dx*begin_lambda + -0.0998601 *begin_dx*begin_dy*begin_lambda + 0.0249779 *begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 0.00116668 *begin_x*begin_dy*lens_ipow(begin_lambda, 2) + -1.18558 *begin_x*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 3) + 0.000373915 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2)+0.0f;
          domega2_dx0[1][0] =  + 0.0669129 *begin_dx*begin_dy + -9.59075e-05 *begin_y*begin_dx + 0.000628596 *begin_x*begin_dy + 0.000150057 *begin_x*begin_dy*begin_lambda + 4.94668e-05 *begin_y*begin_dx*begin_lambda + 0.00640416 *begin_dx*begin_dy*begin_lambda + -0.0369238 *lens_ipow(begin_dx, 3)*begin_dy + -0.000400921 *begin_y*begin_dx*lens_ipow(begin_dy, 2) + 1.78479e-05 *lens_ipow(begin_x, 2)*begin_dx*begin_dy+0.0f;
          domega2_dx0[1][1] =  + -0.030348  + -0.000739868 *begin_lambda + 0.0043513 *lens_ipow(begin_dx, 2) + -0.00126683 *begin_y*begin_dy + -0.00400459 *lens_ipow(begin_dy, 2) + -9.59075e-05 *begin_x*begin_dx + 0.000576135 *lens_ipow(begin_lambda, 2) + -9.15615e-06 *lens_ipow(begin_y, 2) + -1.93542e-06 *lens_ipow(begin_y, 2)*begin_lambda + 4.94668e-05 *begin_x*begin_dx*begin_lambda + -0.000400921 *begin_x*begin_dx*lens_ipow(begin_dy, 2) + 0.0245005 *lens_ipow(begin_dy, 4) + 0.011227 *lens_ipow(begin_dx, 4)*begin_lambda+0.0f;
          float invJ[2][2];
          const float invdet = 1.0f/(domega2_dx0[0][0]*domega2_dx0[1][1] - domega2_dx0[0][1]*domega2_dx0[1][0]);
          invJ[0][0] =  domega2_dx0[1][1]*invdet;
          invJ[1][1] =  domega2_dx0[0][0]*invdet;
          invJ[0][1] = -domega2_dx0[0][1]*invdet;
          invJ[1][0] = -domega2_dx0[1][0]*invdet;
          for(int i=0;i<2;i++)
          {
            x += invJ[0][i]*delta_out[i];
            y += invJ[1][i]*delta_out[i];
          }
          if(sqr_err>prev_sqr_err) error |= 1;
          if(sqr_ap_err>prev_sqr_ap_err) error |= 2;
          if(out[0]!=out[0]) error |= 4;
          if(out[1]!=out[1]) error |= 8;
          // reset error code for first few iterations.
          if(k<10) error = 0;
        }
      }
      else
        error = 128;
      if(out[0]*out[0]+out[1]*out[1] > camera_data->lens_outer_pupil_radius*camera_data->lens_outer_pupil_radius) error |= 16;
      const float begin_x = x;
      const float begin_y = y;
      const float begin_dx = dx;
      const float begin_dy = dy;
      const float begin_lambda = lambda;
      if(error==0)
        out[4] =  + 0.59399  + 0.836383 *begin_lambda + -0.000344805 *begin_x*begin_dx + -7.02536e-06 *lens_ipow(begin_x, 2) + -1.73936 *lens_ipow(begin_lambda, 2) + 1.70047 *lens_ipow(begin_lambda, 3) + -0.644121 *lens_ipow(begin_lambda, 4) + -0.150549 *lens_ipow(begin_dx, 4) + -0.449125 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + -1.08274e-05 *lens_ipow(begin_x, 2)*begin_y*begin_dy + -1.09547e-05 *begin_x*lens_ipow(begin_y, 2)*begin_dx + -4.56631e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -3.11249e-07 *lens_ipow(begin_y, 4) + -0.00046016 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + -2.35642e-05 *lens_ipow(begin_y, 3)*begin_dy + -2.22792e-09 *lens_ipow(begin_x, 6) + -1.06372e-07 *lens_ipow(begin_x, 5)*begin_dx + -1.21435e-10 *lens_ipow(begin_y, 6) + -5.63631e-10 *lens_ipow(begin_x, 7)*begin_dx + -4.88522e-12 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4) + 1.25574e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dx*begin_dy + -2.84961e-08 *lens_ipow(begin_x, 6)*lens_ipow(begin_dx, 2) + -3.7395e-13 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 6) + 2.03261e-06 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 4)*begin_lambda + 7.20677e-05 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 6) + -3.53471e-15 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 2) + -3.64078e-15 *lens_ipow(begin_x, 10) + 3.93018e-08 *lens_ipow(begin_x, 5)*begin_y*lens_ipow(begin_dx, 3)*begin_dy*begin_lambda;
      else
        out[4] = 0.0f;

    } break;
  }




  sensor[0] = x; sensor[1] = y; sensor[2] = dx; sensor[3] = dy; sensor[4] = lambda;
  return MAX(0.0f, out[4]);

}

/*
// jacobian of polynomial mapping sensor to outer pupil. in[]: sensor point/direction/lambda.
static inline void lens_evaluate_jacobian(const float *in, float *J)
{
  __attribute__ ((unused)) const float x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];
#include "pt_evaluate_jacobian.h"
  J[0]  = dx00; J[1]  = dx01; J[2]  = dx02; J[3]  = dx03; J[4]  = dx04;
  J[5]  = dx10; J[6]  = dx11; J[7]  = dx12; J[8]  = dx13; J[9]  = dx14;
  J[10] = dx20; J[11] = dx21; J[12] = dx22; J[13] = dx23; J[14] = dx24;
  J[15] = dx30; J[16] = dx31; J[17] = dx32; J[18] = dx33; J[19] = dx34;
  J[20] = dx40; J[21] = dx41; J[22] = dx42; J[23] = dx43; J[24] = dx44;
}*/

/*
static inline float lens_det_sensor_to_outer_pupil(const float *sensor, const float *out, const float focus)
{
  float J[25];
  lens_evaluate_jacobian(sensor, J);
  // only interested in how the direction density at the sensor changes wrt the vertex area on the output
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
  const float det = JT[2] * JT[5+3] - JT[3] * JT[5+2];

  // convert from projected disk to point on hemi-sphere
  const float R = lens_outer_pupil_curvature_radius;
  const float deto = sqrtf(R*R-out[0]*out[0]-out[1]*out[1])/R;
  // there are two spatial components which need conversion to dm:
  const float dm2mm = 100.0f;
  return fabsf(det * deto) / (dm2mm*dm2mm);
}*/

/*
static inline void lens_evaluate_aperture_jacobian(const float *in, float *J)
{
  __attribute__ ((unused)) const float x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];
#include "pt_evaluate_aperture_jacobian.h"
  J[0]  = dx00; J[1]  = dx01; J[2]  = dx02; J[3]  = dx03; J[4]  = dx04;
  J[5]  = dx10; J[6]  = dx11; J[7]  = dx12; J[8]  = dx13; J[9]  = dx14;
  J[10] = dx20; J[11] = dx21; J[12] = dx22; J[13] = dx23; J[14] = dx24;
  J[15] = dx30; J[16] = dx31; J[17] = dx32; J[18] = dx33; J[19] = dx34;
  J[20] = dx40; J[21] = dx41; J[22] = dx42; J[23] = dx43; J[24] = dx44;
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
  const float tri = .5f*radius * radius * sinf(2.0f*M_PI/(float)blades);
  return blades * tri;
}*/


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
