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


inline void load_lens_constants (MyCameraData *camera_data)
{
  switch (camera_data->lensModel){
    case NONE:
    {
      camera_data->lens_name = "Petzval (1900), 66mm"; // descriptive name of the lens
      camera_data->lens_outer_pupil_radius = 11.250000; // scene facing radius in mm
      camera_data->lens_inner_pupil_radius = 6.750000; // sensor facing radius in mm
      camera_data->lens_length = 82.800003; // overall lens length in mm
      camera_data->lens_focal_length = 37.500000; // approximate lens focal length in mm (BFL)
      camera_data->lens_aperture_pos = 22.949999; // distance aperture -> outer pupil in mm
      camera_data->lens_aperture_housing_radius = 7.500000; // lens housing radius at the aperture
      camera_data->lens_outer_pupil_curvature_radius = 39.675003; // radius of curvature of the outer pupil
      camera_data->lens_field_of_view = 0.944800; // cosine of the approximate field of view assuming a 35mm image

    } break;

    case petzval_1900_66mm:
    {
      camera_data->lens_name = "Petzval (1900), 66mm"; // descriptive name of the lens
      camera_data->lens_outer_pupil_radius = 11.250000; // scene facing radius in mm
      camera_data->lens_inner_pupil_radius = 6.750000; // sensor facing radius in mm
      camera_data->lens_length = 82.800003; // overall lens length in mm
      camera_data->lens_focal_length = 37.500000; // approximate lens focal length in mm (BFL)
      camera_data->lens_aperture_pos = 22.949999; // distance aperture -> outer pupil in mm
      camera_data->lens_aperture_housing_radius = 7.500000; // lens housing radius at the aperture
      camera_data->lens_outer_pupil_curvature_radius = 39.675003; // radius of curvature of the outer pupil
      camera_data->lens_field_of_view = 0.944800; // cosine of the approximate field of view assuming a 35mm image

    } break;


    case doublegauss_100mm:
    {
      camera_data->lens_name = "double gauss"; // descriptive name of the lens
      camera_data->lens_outer_pupil_radius = 20.000000; // scene facing radius in mm
      camera_data->lens_inner_pupil_radius = 20.000000; // sensor facing radius in mm
      camera_data->lens_length = 125.580002; // overall lens length in mm
      camera_data->lens_focal_length = 71.000000; // approximate lens focal length in mm (BFL)
      camera_data->lens_aperture_pos = 25.889999; // distance aperture -> outer pupil in mm
      camera_data->lens_aperture_housing_radius = 12.000000; // lens housing radius at the aperture
      camera_data->lens_outer_pupil_curvature_radius = 33.139999; // radius of curvature of the outer pupil
      camera_data->lens_field_of_view = 0.976478; // cosine of the approximate field of view assuming a 35mm image
    } break;


    case angenieux_doublegauss_1953_49mm:
    {
      camera_data->lens_name = "double gauss angenieux"; // descriptive name of the lens
      camera_data->lens_outer_pupil_radius = 27.000000; // scene facing radius in mm
      camera_data->lens_inner_pupil_radius = 17.500000; // sensor facing radius in mm
      camera_data->lens_length = 91.206001; // overall lens length in mm
      camera_data->lens_focal_length = 27.871000; // approximate lens focal length in mm (BFL)
      camera_data->lens_aperture_pos = 32.044998; // distance aperture -> outer pupil in mm
      camera_data->lens_aperture_housing_radius = 12.750000; // lens housing radius at the aperture
      camera_data->lens_outer_pupil_curvature_radius = 82.059998; // radius of curvature of the outer pupil
      camera_data->lens_field_of_view = 0.905122; // cosine of the approximate field of view assuming a 35mm image
    } break;


    case fisheye_aspherical:
    {
      camera_data->lens_name = "fisheye aspherical"; // descriptive name of the lens
      camera_data->lens_outer_pupil_radius = 22.000000; // scene facing radius in mm
      camera_data->lens_inner_pupil_radius = 12.900000; // sensor facing radius in mm
      camera_data->lens_length = 80.000000; // overall lens length in mm
      camera_data->lens_focal_length = 14.000000; // approximate lens focal length in mm (BFL)
      camera_data->lens_aperture_pos = 44.909996; // distance aperture -> outer pupil in mm
      camera_data->lens_aperture_housing_radius = 6.000000; // lens housing radius at the aperture
      camera_data->lens_outer_pupil_curvature_radius = 40.820000; // radius of curvature of the outer pupil
      camera_data->lens_field_of_view = -0.884568; // cosine of the approximate field of view assuming a 35mm image
    } break;


    case fisheye:
    {
      camera_data->lens_name = "fisheye"; // descriptive name of the lens
      camera_data->lens_outer_pupil_radius = 106.500000; // scene facing radius in mm
      camera_data->lens_inner_pupil_radius = 10.000000; // sensor facing radius in mm
      camera_data->lens_length = 208.000031; // overall lens length in mm
      camera_data->lens_focal_length = 39.799999; // approximate lens focal length in mm (BFL)
      camera_data->lens_aperture_pos = 145.000015; // distance aperture -> outer pupil in mm
      camera_data->lens_aperture_housing_radius = 16.500000; // lens housing radius at the aperture
      camera_data->lens_outer_pupil_curvature_radius = 143.470001; // radius of curvature of the outer pupil
      camera_data->lens_field_of_view = -0.852320; // cosine of the approximate field of view assuming a 35mm image
    } break;


    case wideangle:
    {
      camera_data->lens_name = "wideangle"; // descriptive name of the lens
      camera_data->lens_outer_pupil_radius = 20.000000; // scene facing radius in mm
      camera_data->lens_inner_pupil_radius = 10.000000; // sensor facing radius in mm
      camera_data->lens_length = 118.080002; // overall lens length in mm
      camera_data->lens_focal_length = 35.000000; // approximate lens focal length in mm (BFL)
      camera_data->lens_aperture_pos = 64.449997; // distance aperture -> outer pupil in mm
      camera_data->lens_aperture_housing_radius = 8.000000; // lens housing radius at the aperture
      camera_data->lens_outer_pupil_curvature_radius = 38.793999; // radius of curvature of the outer pupil
      camera_data->lens_field_of_view = 0.373947; // cosine of the approximate field of view assuming a 35mm image
    } break;


    case takumar_1969_50mm:
    {
      camera_data->lens_name = "1969 pentax takumar 50mm"; // descriptive name of the lens
      camera_data->lens_outer_pupil_radius = 21.000000; // scene facing radius in mm
      camera_data->lens_inner_pupil_radius = 16.000000; // sensor facing radius in mm
      camera_data->lens_length = 83.804993; // overall lens length in mm
      camera_data->lens_focal_length = 31.750000; // approximate lens focal length in mm (BFL)
      camera_data->lens_aperture_pos = 23.730000; // distance aperture -> outer pupil in mm
      camera_data->lens_aperture_housing_radius = 13.200000; // lens housing radius at the aperture
      camera_data->lens_outer_pupil_curvature_radius = 55.000000; // radius of curvature of the outer pupil
      camera_data->lens_field_of_view = 0.913921; // cosine of the approximate field of view assuming a 35mm image

    } break;


    case zeiss_biotar_1927_58mm:
    {
      camera_data->lens_name = "1927 zeiss biotar 58mm"; // descriptive name of the lens
      camera_data->lens_outer_pupil_radius = 20.299999; // scene facing radius in mm
      camera_data->lens_inner_pupil_radius = 15.660000; // sensor facing radius in mm
      camera_data->lens_length = 91.663193; // overall lens length in mm
      camera_data->lens_focal_length = 37.700001; // approximate lens focal length in mm (BFL)
      camera_data->lens_aperture_pos = 24.939999; // distance aperture -> outer pupil in mm
      camera_data->lens_aperture_housing_radius = 13.049999; // lens housing radius at the aperture
      camera_data->lens_outer_pupil_curvature_radius = 48.487999; // radius of curvature of the outer pupil
      camera_data->lens_field_of_view = 0.926006; // cosine of the approximate field of view assuming a 35mm image
    } break;

  }

  AiMsgInfo("[POTA] ----------  LENS CONSTANTS  -----------");
  AiMsgInfo("[POTA] lens_name: %s", camera_data->lens_name);
  AiMsgInfo("[POTA] lens_outer_pupil_radius: %f", camera_data->lens_outer_pupil_radius);
  AiMsgInfo("[POTA] lens_inner_pupil_radius: %f", camera_data->lens_inner_pupil_radius);
  AiMsgInfo("[POTA] lens_length: %f", camera_data->lens_length);
  AiMsgInfo("[POTA] lens_focal_length: %f", camera_data->lens_focal_length);
  AiMsgInfo("[POTA] lens_aperture_pos: %f", camera_data->lens_aperture_pos);
  AiMsgInfo("[POTA] lens_aperture_housing_radius: %f", camera_data->lens_aperture_housing_radius);
  AiMsgInfo("[POTA] lens_outer_pupil_curvature_radius: %f", camera_data->lens_outer_pupil_curvature_radius);
  AiMsgInfo("[POTA] lens_field_of_view: %f", camera_data->lens_field_of_view);
  AiMsgInfo("[POTA] --------------------------------------");

}

// evaluates from sensor (in) to outer pupil (out).
// input arrays are 5d [x,y,dx,dy,lambda] where dx and dy are the direction in
// two-plane parametrization (that is the third component of the direction would be 1.0).
// units are millimeters for lengths and micrometers for the wavelength (so visible light is about 0.4--0.7)
// returns the transmittance computed from the polynomial.
static inline float lens_evaluate(const float *in, float *out, MyCameraData *camera_data)
{
  const float x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];
//#include "pt_evaluate.h"

  float out_transmittance = 0.0f;
  switch (camera_data->lensModel){

    case NONE:
    {
      out[0] =  + 61.6861 *dx + 0.516318 *x + 0.239174 *x*lambda + 6.09756 *dx*lambda + 0.0418018 *y*dx*dy + 0.0291763 *x*y*dy + 0.0384633 *lens_ipow(x, 2)*dx + -41.8684 *lens_ipow(dx, 3) + -0.16516 *x*lens_ipow(lambda, 2) + -41.0878 *dx*lens_ipow(dy, 2) + 0.000319801 *x*lens_ipow(y, 2) + 0.000310337 *lens_ipow(x, 3) + 0.431597 *x*lens_ipow(dy, 2) + 0.417681 *x*lens_ipow(dx, 2) + 0.0106198 *lens_ipow(y, 2)*dx + -4.03513 *dx*lens_ipow(lambda, 3) + 1.11768e-05 *x*lens_ipow(y, 2)*lambda + -0.000382566 *lens_ipow(x, 2)*dx*lambda + -8.637e-05 *lens_ipow(x, 2)*y*dx*dy*lambda + 5.14981e-06 *lens_ipow(x, 7)*lens_ipow(dx, 2)*lens_ipow(lambda, 2) + 13819.6 *lens_ipow(dx, 9)*lens_ipow(lambda, 2) + 1.71189e-08 *lens_ipow(x, 5)*lens_ipow(y, 3)*lens_ipow(dx, 2)*dy + 3.21537e-10 *lens_ipow(x, 9)*lens_ipow(lambda, 2) + 0.00130788 *lens_ipow(x, 3)*lens_ipow(y, 2)*lens_ipow(dx, 4)*lens_ipow(dy, 2) + 0.000150672 *lens_ipow(x, 6)*lens_ipow(dx, 3)*lens_ipow(lambda, 2) + 5.82064e-14 *lens_ipow(x, 7)*lens_ipow(y, 4) + -0.0568649 *lens_ipow(x, 4)*lens_ipow(dx, 5)*lens_ipow(lambda, 2) + 6.75549e-08 *lens_ipow(x, 8)*dx*lens_ipow(lambda, 2);
      out[1] =  + 0.453506 *y + 59.1587 *dy + 19.1364 *dy*lambda + 0.592232 *y*lambda + 0.411922 *y*lens_ipow(dx, 2) + 0.0392662 *lens_ipow(y, 2)*dy + 0.451829 *y*lens_ipow(dy, 2) + 0.0283685 *x*y*dx + -42.1243 *lens_ipow(dx, 2)*dy + -0.817315 *y*lens_ipow(lambda, 2) + 0.000312315 *lens_ipow(x, 2)*y + -19.7228 *dy*lens_ipow(lambda, 2) + 0.000313434 *lens_ipow(y, 3) + 0.0101854 *lens_ipow(x, 2)*dy + -41.478 *lens_ipow(dy, 3) + 0.395356 *y*lens_ipow(lambda, 3) + 1.6101e-05 *lens_ipow(y, 3)*lambda + 8.49311e-06 *lens_ipow(x, 2)*y*lambda + 7.11498 *dy*lens_ipow(lambda, 4) + -7.43062e-05 *lens_ipow(y, 3)*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + -9.90812e-06 *lens_ipow(y, 5)*lens_ipow(dx, 2)*lens_ipow(dy, 2) + 1.08674e-14 *lens_ipow(x, 2)*lens_ipow(y, 9) + 2.57069e-11 *lens_ipow(y, 9)*lens_ipow(dx, 2) + 2.33768e-09 *lens_ipow(x, 4)*lens_ipow(y, 4)*lens_ipow(dx, 2)*dy + 1.52162e-14 *lens_ipow(x, 6)*lens_ipow(y, 5) + -0.313837 *lens_ipow(x, 2)*y*lens_ipow(dy, 8) + 4.28086e-05 *x*lens_ipow(y, 5)*dx*lens_ipow(dy, 4) + 0.00203743 *x*lens_ipow(y, 4)*dx*lens_ipow(dy, 5);
      out[2] =  + -1.63943 *dx + -0.0305953 *x + 0.0323359 *dx*lambda + -0.0004223 *lens_ipow(x, 2)*dx + 1.74614e-06 *x*lens_ipow(y, 2) + 4.9289 *lens_ipow(dx, 3)*lambda + -0.0847201 *x*lens_ipow(dy, 2)*lambda + -1.54502e-05 *x*lens_ipow(y, 2)*lambda + -8.9172e-06 *lens_ipow(x, 3)*lambda + -0.00275435 *x*y*dy*lambda + -0.000390955 *lens_ipow(x, 2)*dx*lambda + -0.000707983 *lens_ipow(y, 2)*dx*lambda + -2.05796 *dx*lens_ipow(dy, 2)*lambda + -0.0998601 *y*dx*dy*lambda + 0.0184584 *x*lens_ipow(dx, 4) + 0.0249779 *y*dx*dy*lens_ipow(lambda, 2) + -8.31442 *lens_ipow(dx, 3)*lens_ipow(lambda, 2) + 1.57657 *lens_ipow(dx, 5) + 4.83602e-06 *lens_ipow(x, 3)*lens_ipow(lambda, 2) + 0.0421838 *x*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + 0.00116668 *x*y*dy*lens_ipow(lambda, 2) + -0.020373 *x*lens_ipow(dx, 2)*lens_ipow(lambda, 3) + 4.0944 *lens_ipow(dx, 3)*lens_ipow(lambda, 3) + -1.18558 *x*y*lens_ipow(dx, 4)*lens_ipow(dy, 3) + 20.8038 *dx*lens_ipow(dy, 8) + 2134.98 *lens_ipow(dx, 5)*lens_ipow(dy, 4) + 0.000186958 *lens_ipow(x, 2)*lens_ipow(y, 2)*lens_ipow(dx, 3)*lens_ipow(dy, 2) + 0.000963685 *x*lens_ipow(lambda, 9);
      out[3] =  + -0.030348 *y + -1.6733 *dy + 0.226598 *dy*lambda + -0.000739868 *y*lambda + 0.0043513 *y*lens_ipow(dx, 2) + -0.000633417 *lens_ipow(y, 2)*dy + -0.00400459 *y*lens_ipow(dy, 2) + 0.0669129 *x*dx*dy + -9.59075e-05 *x*y*dx + 2.91954 *lens_ipow(dx, 2)*dy + 0.000576135 *y*lens_ipow(lambda, 2) + -0.334116 *dy*lens_ipow(lambda, 2) + -3.05205e-06 *lens_ipow(y, 3) + 0.000314298 *lens_ipow(x, 2)*dy + 0.885659 *lens_ipow(dy, 3) + 7.50285e-05 *lens_ipow(x, 2)*dy*lambda + -0.113294 *lens_ipow(dy, 3)*lambda + 0.176171 *dy*lens_ipow(lambda, 3) + -6.45139e-07 *lens_ipow(y, 3)*lambda + 4.94668e-05 *x*y*dx*lambda + 0.00640416 *x*dx*dy*lambda + -0.0369238 *x*lens_ipow(dx, 3)*dy + 2.5536 *lens_ipow(dx, 2)*lens_ipow(dy, 3) + 1.94879 *lens_ipow(dy, 5) + -0.000400921 *x*y*dx*lens_ipow(dy, 2) + 0.0245005 *y*lens_ipow(dy, 4) + 5.94929e-06 *lens_ipow(x, 3)*dx*dy + 0.011227 *y*lens_ipow(dx, 4)*lambda;
      out_transmittance =  + 0.59399  + 0.836383 *lambda + -0.000344805 *x*dx + -7.02536e-06 *lens_ipow(x, 2) + -1.73936 *lens_ipow(lambda, 2) + 1.70047 *lens_ipow(lambda, 3) + -0.644121 *lens_ipow(lambda, 4) + -0.150549 *lens_ipow(dx, 4) + -0.449125 *lens_ipow(dx, 2)*lens_ipow(dy, 2) + -1.08274e-05 *lens_ipow(x, 2)*y*dy + -1.09547e-05 *x*lens_ipow(y, 2)*dx + -4.56631e-07 *lens_ipow(x, 2)*lens_ipow(y, 2) + -3.11249e-07 *lens_ipow(y, 4) + -0.00046016 *lens_ipow(y, 2)*lens_ipow(dy, 2) + -2.35642e-05 *lens_ipow(y, 3)*dy + -2.22792e-09 *lens_ipow(x, 6) + -1.06372e-07 *lens_ipow(x, 5)*dx + -1.21435e-10 *lens_ipow(y, 6) + -5.63631e-10 *lens_ipow(x, 7)*dx + -4.88522e-12 *lens_ipow(x, 4)*lens_ipow(y, 4) + 1.25574e-08 *lens_ipow(x, 3)*lens_ipow(y, 3)*dx*dy + -2.84961e-08 *lens_ipow(x, 6)*lens_ipow(dx, 2) + -3.7395e-13 *lens_ipow(x, 2)*lens_ipow(y, 6) + 2.03261e-06 *lens_ipow(y, 4)*lens_ipow(dy, 4)*lambda + 7.20677e-05 *lens_ipow(x, 4)*lens_ipow(dx, 6) + -3.53471e-15 *lens_ipow(x, 8)*lens_ipow(y, 2) + -3.64078e-15 *lens_ipow(x, 10) + 3.93018e-08 *lens_ipow(x, 5)*y*lens_ipow(dx, 3)*dy*lambda;
    } break;


    case petzval_1900_66mm:
    {
      out[0] =  + 61.6861 *dx + 0.516318 *x + 0.239174 *x*lambda + 6.09756 *dx*lambda + 0.0418018 *y*dx*dy + 0.0291763 *x*y*dy + 0.0384633 *lens_ipow(x, 2)*dx + -41.8684 *lens_ipow(dx, 3) + -0.16516 *x*lens_ipow(lambda, 2) + -41.0878 *dx*lens_ipow(dy, 2) + 0.000319801 *x*lens_ipow(y, 2) + 0.000310337 *lens_ipow(x, 3) + 0.431597 *x*lens_ipow(dy, 2) + 0.417681 *x*lens_ipow(dx, 2) + 0.0106198 *lens_ipow(y, 2)*dx + -4.03513 *dx*lens_ipow(lambda, 3) + 1.11768e-05 *x*lens_ipow(y, 2)*lambda + -0.000382566 *lens_ipow(x, 2)*dx*lambda + -8.637e-05 *lens_ipow(x, 2)*y*dx*dy*lambda + 5.14981e-06 *lens_ipow(x, 7)*lens_ipow(dx, 2)*lens_ipow(lambda, 2) + 13819.6 *lens_ipow(dx, 9)*lens_ipow(lambda, 2) + 1.71189e-08 *lens_ipow(x, 5)*lens_ipow(y, 3)*lens_ipow(dx, 2)*dy + 3.21537e-10 *lens_ipow(x, 9)*lens_ipow(lambda, 2) + 0.00130788 *lens_ipow(x, 3)*lens_ipow(y, 2)*lens_ipow(dx, 4)*lens_ipow(dy, 2) + 0.000150672 *lens_ipow(x, 6)*lens_ipow(dx, 3)*lens_ipow(lambda, 2) + 5.82064e-14 *lens_ipow(x, 7)*lens_ipow(y, 4) + -0.0568649 *lens_ipow(x, 4)*lens_ipow(dx, 5)*lens_ipow(lambda, 2) + 6.75549e-08 *lens_ipow(x, 8)*dx*lens_ipow(lambda, 2);
      out[1] =  + 0.453506 *y + 59.1587 *dy + 19.1364 *dy*lambda + 0.592232 *y*lambda + 0.411922 *y*lens_ipow(dx, 2) + 0.0392662 *lens_ipow(y, 2)*dy + 0.451829 *y*lens_ipow(dy, 2) + 0.0283685 *x*y*dx + -42.1243 *lens_ipow(dx, 2)*dy + -0.817315 *y*lens_ipow(lambda, 2) + 0.000312315 *lens_ipow(x, 2)*y + -19.7228 *dy*lens_ipow(lambda, 2) + 0.000313434 *lens_ipow(y, 3) + 0.0101854 *lens_ipow(x, 2)*dy + -41.478 *lens_ipow(dy, 3) + 0.395356 *y*lens_ipow(lambda, 3) + 1.6101e-05 *lens_ipow(y, 3)*lambda + 8.49311e-06 *lens_ipow(x, 2)*y*lambda + 7.11498 *dy*lens_ipow(lambda, 4) + -7.43062e-05 *lens_ipow(y, 3)*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + -9.90812e-06 *lens_ipow(y, 5)*lens_ipow(dx, 2)*lens_ipow(dy, 2) + 1.08674e-14 *lens_ipow(x, 2)*lens_ipow(y, 9) + 2.57069e-11 *lens_ipow(y, 9)*lens_ipow(dx, 2) + 2.33768e-09 *lens_ipow(x, 4)*lens_ipow(y, 4)*lens_ipow(dx, 2)*dy + 1.52162e-14 *lens_ipow(x, 6)*lens_ipow(y, 5) + -0.313837 *lens_ipow(x, 2)*y*lens_ipow(dy, 8) + 4.28086e-05 *x*lens_ipow(y, 5)*dx*lens_ipow(dy, 4) + 0.00203743 *x*lens_ipow(y, 4)*dx*lens_ipow(dy, 5);
      out[2] =  + -1.63943 *dx + -0.0305953 *x + 0.0323359 *dx*lambda + -0.0004223 *lens_ipow(x, 2)*dx + 1.74614e-06 *x*lens_ipow(y, 2) + 4.9289 *lens_ipow(dx, 3)*lambda + -0.0847201 *x*lens_ipow(dy, 2)*lambda + -1.54502e-05 *x*lens_ipow(y, 2)*lambda + -8.9172e-06 *lens_ipow(x, 3)*lambda + -0.00275435 *x*y*dy*lambda + -0.000390955 *lens_ipow(x, 2)*dx*lambda + -0.000707983 *lens_ipow(y, 2)*dx*lambda + -2.05796 *dx*lens_ipow(dy, 2)*lambda + -0.0998601 *y*dx*dy*lambda + 0.0184584 *x*lens_ipow(dx, 4) + 0.0249779 *y*dx*dy*lens_ipow(lambda, 2) + -8.31442 *lens_ipow(dx, 3)*lens_ipow(lambda, 2) + 1.57657 *lens_ipow(dx, 5) + 4.83602e-06 *lens_ipow(x, 3)*lens_ipow(lambda, 2) + 0.0421838 *x*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + 0.00116668 *x*y*dy*lens_ipow(lambda, 2) + -0.020373 *x*lens_ipow(dx, 2)*lens_ipow(lambda, 3) + 4.0944 *lens_ipow(dx, 3)*lens_ipow(lambda, 3) + -1.18558 *x*y*lens_ipow(dx, 4)*lens_ipow(dy, 3) + 20.8038 *dx*lens_ipow(dy, 8) + 2134.98 *lens_ipow(dx, 5)*lens_ipow(dy, 4) + 0.000186958 *lens_ipow(x, 2)*lens_ipow(y, 2)*lens_ipow(dx, 3)*lens_ipow(dy, 2) + 0.000963685 *x*lens_ipow(lambda, 9);
      out[3] =  + -0.030348 *y + -1.6733 *dy + 0.226598 *dy*lambda + -0.000739868 *y*lambda + 0.0043513 *y*lens_ipow(dx, 2) + -0.000633417 *lens_ipow(y, 2)*dy + -0.00400459 *y*lens_ipow(dy, 2) + 0.0669129 *x*dx*dy + -9.59075e-05 *x*y*dx + 2.91954 *lens_ipow(dx, 2)*dy + 0.000576135 *y*lens_ipow(lambda, 2) + -0.334116 *dy*lens_ipow(lambda, 2) + -3.05205e-06 *lens_ipow(y, 3) + 0.000314298 *lens_ipow(x, 2)*dy + 0.885659 *lens_ipow(dy, 3) + 7.50285e-05 *lens_ipow(x, 2)*dy*lambda + -0.113294 *lens_ipow(dy, 3)*lambda + 0.176171 *dy*lens_ipow(lambda, 3) + -6.45139e-07 *lens_ipow(y, 3)*lambda + 4.94668e-05 *x*y*dx*lambda + 0.00640416 *x*dx*dy*lambda + -0.0369238 *x*lens_ipow(dx, 3)*dy + 2.5536 *lens_ipow(dx, 2)*lens_ipow(dy, 3) + 1.94879 *lens_ipow(dy, 5) + -0.000400921 *x*y*dx*lens_ipow(dy, 2) + 0.0245005 *y*lens_ipow(dy, 4) + 5.94929e-06 *lens_ipow(x, 3)*dx*dy + 0.011227 *y*lens_ipow(dx, 4)*lambda;
      out_transmittance =  + 0.59399  + 0.836383 *lambda + -0.000344805 *x*dx + -7.02536e-06 *lens_ipow(x, 2) + -1.73936 *lens_ipow(lambda, 2) + 1.70047 *lens_ipow(lambda, 3) + -0.644121 *lens_ipow(lambda, 4) + -0.150549 *lens_ipow(dx, 4) + -0.449125 *lens_ipow(dx, 2)*lens_ipow(dy, 2) + -1.08274e-05 *lens_ipow(x, 2)*y*dy + -1.09547e-05 *x*lens_ipow(y, 2)*dx + -4.56631e-07 *lens_ipow(x, 2)*lens_ipow(y, 2) + -3.11249e-07 *lens_ipow(y, 4) + -0.00046016 *lens_ipow(y, 2)*lens_ipow(dy, 2) + -2.35642e-05 *lens_ipow(y, 3)*dy + -2.22792e-09 *lens_ipow(x, 6) + -1.06372e-07 *lens_ipow(x, 5)*dx + -1.21435e-10 *lens_ipow(y, 6) + -5.63631e-10 *lens_ipow(x, 7)*dx + -4.88522e-12 *lens_ipow(x, 4)*lens_ipow(y, 4) + 1.25574e-08 *lens_ipow(x, 3)*lens_ipow(y, 3)*dx*dy + -2.84961e-08 *lens_ipow(x, 6)*lens_ipow(dx, 2) + -3.7395e-13 *lens_ipow(x, 2)*lens_ipow(y, 6) + 2.03261e-06 *lens_ipow(y, 4)*lens_ipow(dy, 4)*lambda + 7.20677e-05 *lens_ipow(x, 4)*lens_ipow(dx, 6) + -3.53471e-15 *lens_ipow(x, 8)*lens_ipow(y, 2) + -3.64078e-15 *lens_ipow(x, 10) + 3.93018e-08 *lens_ipow(x, 5)*y*lens_ipow(dx, 3)*dy*lambda;
    } break;


    case doublegauss_100mm:
    {
      out[0] =  + 100.12 *dx + 0.592582 *x + 0.0499465 *x*lambda + -0.811097 *dx*lambda + 0.0323359 *x*y*dy + 0.0379048 *lens_ipow(x, 2)*dx + -42.7325 *lens_ipow(dx, 3) + -0.0389817 *x*lens_ipow(lambda, 2) + 0.000142213 *x*lens_ipow(y, 2) + 0.000142435 *lens_ipow(x, 3) + 1.67656 *x*lens_ipow(dy, 2) + 1.81407 *x*lens_ipow(dx, 2) + 0.00570726 *lens_ipow(y, 2)*dx + -209.372 *dx*lens_ipow(dy, 2)*lambda + 0.677473 *y*dx*dy*lambda + -0.536195 *y*dx*dy*lens_ipow(lambda, 2) + 299.926 *dx*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + 0.0713542 *x*y*lens_ipow(dy, 3)*lambda + 0.00125855 *x*lens_ipow(y, 2)*lens_ipow(dy, 2)*lambda + -3.55319 *x*lens_ipow(dx, 4)*lambda + 222.317 *lens_ipow(dx, 3)*lens_ipow(dy, 2)*lambda + -0.000266069 *lens_ipow(y, 3)*dx*dy*lambda + -5.3226e-06 *lens_ipow(x, 3)*y*dy*lambda + 0.00102282 *lens_ipow(x, 3)*lens_ipow(dx, 2)*lambda + -168.54 *dx*lens_ipow(dy, 2)*lens_ipow(lambda, 4) + -3.93679e-10 *lens_ipow(x, 3)*lens_ipow(y, 4)*lambda + 1.32918e-08 *lens_ipow(x, 6)*dx*lambda + -2.70347e-08 *lens_ipow(x, 2)*lens_ipow(y, 4)*dx*lambda;
      out[1] =  + 0.588556 *y + 99.6889 *dy + 0.0557925 *y*lambda + 1.62675 *y*lens_ipow(dx, 2) + 0.045876 *lens_ipow(y, 2)*dy + 2.49363 *y*lens_ipow(dy, 2) + 0.153878 *x*dx*dy + 0.0314574 *x*y*dx + -42.0967 *lens_ipow(dx, 2)*dy + -0.0358604 *y*lens_ipow(lambda, 2) + 0.000141395 *lens_ipow(x, 2)*y + 0.000178882 *lens_ipow(y, 3) + 0.00572629 *lens_ipow(x, 2)*dy + -1.35881 *y*lens_ipow(dy, 2)*lambda + -135.315 *lens_ipow(dy, 3)*lambda + -0.0168528 *lens_ipow(y, 2)*dy*lambda + -6.88134e-05 *lens_ipow(y, 3)*lambda + 120.172 *lens_ipow(dx, 2)*lens_ipow(dy, 3) + 9.10801e-06 *lens_ipow(y, 4)*dy + 0.0748529 *lens_ipow(y, 2)*lens_ipow(dy, 3) + 106.566 *lens_ipow(dy, 3)*lens_ipow(lambda, 2) + 0.000223543 *lens_ipow(y, 3)*lens_ipow(dx, 2) + 0.00161417 *lens_ipow(y, 3)*lens_ipow(dy, 2) + 0.000235019 *lens_ipow(x, 2)*y*lens_ipow(dy, 2)*lambda + 0.681351 *lens_ipow(y, 2)*lens_ipow(dx, 4)*dy + -0.000143401 *lens_ipow(x, 2)*lens_ipow(y, 2)*lens_ipow(dx, 2)*dy + -9.81214e-11 *lens_ipow(x, 4)*lens_ipow(y, 3) + -56.6549 *lens_ipow(dy, 3)*lens_ipow(lambda, 6);
      out[2] =  + -3.05455 *dx + -0.0282279 *x + -0.000260254 *x*lambda + 0.150251 *dx*lambda + -0.193971 *y*dx*dy + -0.00214261 *x*y*dy + -0.000343775 *lens_ipow(x, 2)*dx + 1.28863 *lens_ipow(dx, 3) + -13.7923 *dx*lens_ipow(dy, 2) + -6.63026e-06 *x*lens_ipow(y, 2) + -0.144619 *x*lens_ipow(dy, 2) + -0.0045328 *x*lens_ipow(dx, 2) + -0.000631766 *lens_ipow(y, 2)*dx + -0.108421 *dx*lens_ipow(lambda, 2) + -7.70575e-06 *lens_ipow(x, 3)*lens_ipow(lambda, 2) + -0.119872 *lens_ipow(y, 2)*lens_ipow(dx, 3)*lens_ipow(dy, 2) + 3.23657e-05 *lens_ipow(x, 3)*lens_ipow(lambda, 4) + 2457.14 *lens_ipow(dx, 3)*lens_ipow(dy, 4) + -0.00163849 *x*lens_ipow(y, 2)*lens_ipow(dx, 2)*lens_ipow(dy, 2) + 0.274907 *lens_ipow(x, 2)*dx*lens_ipow(dy, 4) + 49.9949 *x*lens_ipow(dx, 2)*lens_ipow(dy, 4) + -4.84099e-06 *lens_ipow(x, 3)*y*lens_ipow(dy, 3) + 1.54348e-06 *lens_ipow(y, 4)*lens_ipow(dx, 3)*lambda + -2.83936e-05 *lens_ipow(x, 3)*lens_ipow(lambda, 5) + -543.956 *x*lens_ipow(dx, 2)*lens_ipow(dy, 6) + -3.42461 *lens_ipow(x, 2)*dx*lens_ipow(dy, 6) + -25109 *lens_ipow(dx, 3)*lens_ipow(dy, 6) + -0.0503811 *lens_ipow(x, 3)*lens_ipow(dy, 8);
      out[3] =  + -0.0282638 *y + -3.02557 *dy + 0.0262125 *dy*lambda + -0.000273248 *y*lambda + 0.0476166 *y*lens_ipow(dx, 2) + -0.00028279 *lens_ipow(y, 2)*dy + 0.278569 *x*dx*dy + 0.000913623 *x*y*dx + 16.1714 *lens_ipow(dx, 2)*dy + 5.08174e-06 *lens_ipow(x, 2)*y + 0.00117665 *lens_ipow(x, 2)*dy + 5.85395 *lens_ipow(dy, 3)*lambda + -1.29969e-06 *lens_ipow(y, 3)*lambda + -0.391177 *x*lens_ipow(dx, 3)*dy + 58.129 *lens_ipow(dx, 2)*lens_ipow(dy, 3) + -5.96864e-07 *x*lens_ipow(y, 3)*dx + 1.32679 *x*dx*lens_ipow(dy, 3) + 0.0182734 *x*y*dx*lens_ipow(dy, 2) + -3.19863e-09 *lens_ipow(x, 2)*lens_ipow(y, 3) + -6.23406 *lens_ipow(dy, 3)*lens_ipow(lambda, 2) + -2.7888e-05 *lens_ipow(y, 3)*lens_ipow(dx, 2) + 0.786317 *y*lens_ipow(dx, 2)*lens_ipow(dy, 2) + 0.00738819 *lens_ipow(x, 2)*lens_ipow(dy, 3) + 1.77456e-05 *lens_ipow(x, 3)*dx*dy + -23.2769 *lens_ipow(dx, 4)*dy + 0.000103722 *lens_ipow(x, 2)*y*lens_ipow(dy, 2) + 2.39665e-06 *lens_ipow(y, 3)*lens_ipow(lambda, 5) + 2.99896 *lens_ipow(dy, 3)*lens_ipow(lambda, 6);
      out_transmittance =  + 3.76112 *lambda + 0.000259609 *y*dy + 0.000266756 *x*dx + 0.0168177 *lens_ipow(dy, 2) + 0.0171709 *lens_ipow(dx, 2) + -10.7968 *lens_ipow(lambda, 2) + 16.4369 *lens_ipow(lambda, 3) + -12.9412 *lens_ipow(lambda, 4) + -7.32614 *lens_ipow(dx, 4) + -0.267851 *y*lens_ipow(dy, 3) + -0.00124542 *lens_ipow(y, 2)*lens_ipow(dx, 2) + -0.268464 *x*lens_ipow(dx, 3) + -14.667 *lens_ipow(dx, 2)*lens_ipow(dy, 2) + -2.35604e-05 *lens_ipow(x, 2)*y*dy + -7.31341 *lens_ipow(dy, 4) + -2.35063e-05 *x*lens_ipow(y, 2)*dx + -1.1016e-07 *lens_ipow(x, 2)*lens_ipow(y, 2) + -5.37091e-08 *lens_ipow(y, 4) + -0.003764 *lens_ipow(x, 2)*lens_ipow(dx, 2) + -0.268213 *y*lens_ipow(dx, 2)*dy + -2.36431e-05 *lens_ipow(x, 3)*dx + -0.00375249 *lens_ipow(y, 2)*lens_ipow(dy, 2) + -0.26844 *x*dx*lens_ipow(dy, 2) + -5.39032e-08 *lens_ipow(x, 4) + -0.00500763 *x*y*dx*dy + -0.00124868 *lens_ipow(x, 2)*lens_ipow(dy, 2) + -2.35432e-05 *lens_ipow(y, 3)*dy + 4.15947 *lens_ipow(lambda, 5);
    } break;


    case angenieux_doublegauss_1953_49mm:
    {
      out[0] =  + 49.6109 *dx + -0.621577 *x + 0.674235 *x*lambda + 0.214431 *y*dx*dy + 0.00612017 *x*y*dy + 0.0185352 *lens_ipow(x, 2)*dx + -19.0762 *lens_ipow(dx, 3) + -0.526696 *x*lens_ipow(lambda, 2) + -19.997 *dx*lens_ipow(dy, 2) + -0.00135091 *x*lens_ipow(y, 2) + -0.00120413 *lens_ipow(x, 3) + 0.265561 *x*lens_ipow(dy, 2) + 0.547505 *x*lens_ipow(dx, 2) + 0.000620579 *x*lens_ipow(y, 2)*lambda + 0.000501355 *lens_ipow(x, 3)*lambda + 0.00915562 *lens_ipow(y, 2)*dx*lambda + 0.0777405 *x*y*lens_ipow(dx, 2)*dy + -4.20841e-05 *lens_ipow(x, 4)*dx + -0.00108756 *lens_ipow(y, 3)*dx*dy + 0.0289986 *lens_ipow(y, 2)*dx*lens_ipow(dy, 2) + -0.00208955 *lens_ipow(x, 3)*lens_ipow(dx, 2) + -4.49517e-06 *x*lens_ipow(y, 4)*lens_ipow(dx, 2) + -9.51913e-09 *lens_ipow(x, 3)*lens_ipow(y, 4) + -3.46111e-09 *lens_ipow(x, 7) + -7.05762e-12 *x*lens_ipow(y, 8) + -6.20338e-11 *lens_ipow(x, 7)*lens_ipow(y, 2) + 0.0285076 *lens_ipow(y, 3)*lens_ipow(dx, 3)*dy*lens_ipow(lambda, 4) + 1.33412e-13 *lens_ipow(x, 9)*lens_ipow(y, 2);
      out[1] =  + -0.613564 *y + 49.7175 *dy + 0.669005 *y*lambda + 0.246172 *y*lens_ipow(dx, 2) + 0.0126827 *lens_ipow(y, 2)*dy + 0.232757 *y*lens_ipow(dy, 2) + 0.00701796 *x*y*dx + -19.0477 *lens_ipow(dx, 2)*dy + -0.524051 *y*lens_ipow(lambda, 2) + -0.00133375 *lens_ipow(x, 2)*y + -0.00127186 *lens_ipow(y, 3) + -21.4258 *lens_ipow(dy, 3) + 0.0105956 *lens_ipow(x, 2)*dy*lambda + 0.000544614 *lens_ipow(y, 3)*lambda + 0.333688 *x*dx*dy*lambda + 0.000640797 *lens_ipow(x, 2)*y*lambda + -1.70371e-05 *lens_ipow(y, 4)*dy + 0.156726 *x*y*dx*lens_ipow(dy, 2)*lambda + 2.94679 *y*lens_ipow(dy, 4)*lambda + 0.128866 *lens_ipow(y, 2)*lens_ipow(dy, 3)*lambda + -3.102e-09 *lens_ipow(y, 7) + 0.000269581 *lens_ipow(x, 4)*lens_ipow(dx, 2)*dy + -1.5407e-07 *lens_ipow(x, 3)*lens_ipow(y, 3)*dx + -2.72774e-09 *lens_ipow(x, 6)*y + 5.47118 *y*lens_ipow(dx, 4)*lens_ipow(dy, 2) + -8.15344e-09 *lens_ipow(x, 2)*lens_ipow(y, 5) + -8.67539e-09 *lens_ipow(x, 4)*lens_ipow(y, 3) + 67.1056 *lens_ipow(dy, 7)*lambda;
      out[2] =  + -0.614401 *dx + -0.0124033 *x + -0.00877929 *x*lambda + 0.00908301 *dx*lambda + -6.4694e-05 *x*y*dy + -9.8007e-05 *lens_ipow(x, 2)*dx + 0.305271 *lens_ipow(dx, 3) + 0.00706724 *x*lens_ipow(lambda, 2) + 0.148208 *dx*lens_ipow(dy, 2) + 1.74459e-05 *x*lens_ipow(y, 2) + 1.77203e-05 *lens_ipow(x, 3) + -0.00111275 *x*lens_ipow(dy, 2) + -7.43443e-06 *x*lens_ipow(y, 2)*lambda + -7.39746e-06 *lens_ipow(x, 3)*lambda + -2.36067e-07 *lens_ipow(x, 4)*dx + -1.07743e-07 *lens_ipow(y, 4)*dx + -0.289597 *lens_ipow(dx, 5) + 6.58747e-09 *x*lens_ipow(y, 4) + -3.09809e-07 *x*lens_ipow(y, 3)*dy + -0.00453847 *x*lens_ipow(dy, 4) + 0.000362057 *lens_ipow(y, 2)*lens_ipow(dx, 3) + 3.63636e-05 *lens_ipow(x, 2)*y*dx*dy + 0.000693516 *lens_ipow(x, 2)*dx*lens_ipow(dy, 2) + 1.32547e-10 *lens_ipow(x, 5)*lens_ipow(y, 2) + -0.0485359 *x*lens_ipow(dx, 6) + 3.30777e-11 *lens_ipow(x, 7) + 4.41165e-13 *lens_ipow(x, 3)*lens_ipow(y, 6) + 0.0506785 *x*y*lens_ipow(dx, 4)*lens_ipow(dy, 3)*lambda;
      out[3] =  + -0.0119193 *y + -0.614842 *dy + -0.00958648 *y*lambda + -0.00420097 *y*lens_ipow(dx, 2) + 0.00389116 *x*dx*dy + -0.000108381 *x*y*dx + 0.428204 *lens_ipow(dx, 2)*dy + 0.00700915 *y*lens_ipow(lambda, 2) + 1.32734e-05 *lens_ipow(x, 2)*y + 1.40797e-05 *lens_ipow(y, 3) + 0.322422 *lens_ipow(dy, 3) + -0.469415 *lens_ipow(dx, 2)*lens_ipow(dy, 3) + -0.191137 *lens_ipow(dy, 5) + 2.31037e-08 *lens_ipow(x, 4)*y + -0.308648 *lens_ipow(dx, 4)*dy + -8.0158e-07 *lens_ipow(x, 2)*lens_ipow(y, 2)*dy + -1.92001e-08 *lens_ipow(x, 4)*y*lambda + -5.34204e-09 *lens_ipow(y, 6)*dy + 2.65798e-10 *lens_ipow(x, 2)*lens_ipow(y, 5) + -3.59985e-10 *lens_ipow(x, 6)*dy + -0.000157427 *lens_ipow(x, 2)*y*lens_ipow(dy, 4)*lambda + -2.54677e-10 *lens_ipow(x, 2)*lens_ipow(y, 5)*lambda + 9.48325e-14 *lens_ipow(y, 9) + 3.56726e-13 *lens_ipow(x, 6)*lens_ipow(y, 3) + 1.47454e-06 *x*lens_ipow(y, 4)*dx*lens_ipow(dy, 3) + 1.24034e-08 *lens_ipow(y, 6)*lens_ipow(dx, 2)*dy + 2.22341e-11 *lens_ipow(y, 8)*dy*lambda + -0.00303946 *lens_ipow(x, 3)*lens_ipow(dx, 3)*lens_ipow(dy, 3)*lens_ipow(lambda, 2);
      out_transmittance =  + 0.238949  + 0.85346 *lambda + -1.16866 *lens_ipow(lambda, 2) + -0.000545894 *y*dy*lambda + -0.000532821 *x*dx*lambda + 0.564312 *lens_ipow(lambda, 3) + -0.206872 *lens_ipow(dx, 4) + -0.00790672 *y*lens_ipow(dy, 3) + -0.00013516 *lens_ipow(y, 2)*lens_ipow(dx, 2) + -0.00809514 *x*lens_ipow(dx, 3) + -0.204782 *lens_ipow(dy, 4) + -8.85833e-08 *lens_ipow(x, 2)*lens_ipow(y, 2) + -0.000243089 *lens_ipow(x, 2)*lens_ipow(dx, 2) + -0.00648351 *y*lens_ipow(dx, 2)*dy + -0.000239826 *lens_ipow(y, 2)*lens_ipow(dy, 2) + -0.00654443 *x*dx*lens_ipow(dy, 2) + -0.000460228 *x*y*dx*dy + -0.00013569 *lens_ipow(x, 2)*lens_ipow(dy, 2) + -1.59096 *lens_ipow(dx, 2)*lens_ipow(dy, 2)*lambda + -1.67893e-10 *lens_ipow(x, 6) + -0.000863147 *lens_ipow(x, 2)*lens_ipow(dx, 4) + 1.18371 *lens_ipow(dx, 2)*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + -1.68689e-10 *lens_ipow(y, 6) + -0.000849445 *lens_ipow(y, 2)*lens_ipow(dy, 4) + -2.32122e-12 *lens_ipow(x, 4)*lens_ipow(y, 4) + 8.17818e-09 *lens_ipow(x, 3)*lens_ipow(y, 3)*dx*dy + -0.321221 *lens_ipow(dx, 6)*lens_ipow(lambda, 2) + -0.341708 *lens_ipow(dy, 6)*lens_ipow(lambda, 2);
    } break;


    case fisheye_aspherical:
    {
      out[0] =  + -1.10799 *x + -0.0905798 *x*lambda + 40.196 *dx*lambda + 2.10515 *y*dx*dy + 0.0754832 *x*y*dy + 0.133226 *lens_ipow(x, 2)*dx + 26.791 *lens_ipow(dx, 3) + 27.9088 *dx*lens_ipow(dy, 2) + 0.00105276 *x*lens_ipow(y, 2) + 0.00119667 *lens_ipow(x, 3) + 1.23306 *x*lens_ipow(dy, 2) + 3.36803 *x*lens_ipow(dx, 2) + 0.0534469 *lens_ipow(y, 2)*dx + -61.0312 *dx*lens_ipow(lambda, 2) + 30.4395 *dx*lens_ipow(lambda, 3) + -0.00016579 *lens_ipow(x, 2)*lens_ipow(y, 2)*dx + -8.16049e-07 *lens_ipow(x, 6)*dx + -2.03688e-06 *lens_ipow(x, 3)*lens_ipow(y, 3)*dy + -6.44038e-08 *lens_ipow(x, 3)*lens_ipow(y, 4) + -0.0262952 *lens_ipow(x, 2)*y*lens_ipow(dx, 3)*dy + -3.92523e-08 *lens_ipow(x, 5)*lens_ipow(y, 2) + -0.00581142 *lens_ipow(x, 3)*lens_ipow(dx, 4) + -1.95829e-08 *lens_ipow(x, 7) + -0.128373 *x*lens_ipow(y, 2)*lens_ipow(dx, 2)*lens_ipow(dy, 4) + -6.99074e-11 *x*lens_ipow(y, 8) + -2.8667e-09 *x*lens_ipow(y, 7)*dy + 1.94447e-07 *lens_ipow(x, 6)*y*dx*dy + 1.37312e-09 *lens_ipow(x, 5)*lens_ipow(y, 4)*lens_ipow(dx, 2);
      out[1] =  + -1.00151 *y + 8.2466 *dy + 0.97099 *dy*lambda + -0.455304 *y*lambda + 0.995845 *y*lens_ipow(dx, 2) + 0.130079 *lens_ipow(y, 2)*dy + 3.34769 *y*lens_ipow(dy, 2) + 1.9089 *x*dx*dy + 0.0699908 *x*y*dx + 18.5642 *lens_ipow(dx, 2)*dy + 0.324668 *y*lens_ipow(lambda, 2) + 0.000856088 *lens_ipow(x, 2)*y + 0.00110582 *lens_ipow(y, 3) + 0.05103 *lens_ipow(x, 2)*dy + 26.4204 *lens_ipow(dy, 3) + -1.25836 *x*lens_ipow(dx, 3)*dy + -0.000134239 *lens_ipow(x, 3)*y*dx + 0.00190002 *lens_ipow(y, 3)*lens_ipow(dx, 2) + -3.15418 *y*lens_ipow(dx, 2)*lens_ipow(dy, 2) + -0.000167132 *lens_ipow(x, 2)*lens_ipow(y, 2)*dy + -1.79689e-08 *lens_ipow(y, 7) + -7.19221e-07 *lens_ipow(y, 6)*dy + -1.1258e-08 *lens_ipow(x, 6)*y + -3.3697e-08 *lens_ipow(x, 2)*lens_ipow(y, 5) + -3.903e-08 *lens_ipow(x, 4)*lens_ipow(y, 3) + 0.143156 *lens_ipow(y, 2)*lens_ipow(dy, 5) + 2.30271e-07 *x*lens_ipow(y, 6)*dx*dy*lambda + 1.74713e-09 *lens_ipow(x, 4)*lens_ipow(y, 5)*lens_ipow(dx, 2);
      out[2] =  + -0.24973 *dx + -0.0790022 *x + 0.00582542 *x*lambda + 0.00236612 *lens_ipow(x, 2)*dx + 3.81983e-05 *x*lens_ipow(y, 2) + 8.9246e-05 *lens_ipow(x, 3) + 0.172699 *x*lens_ipow(dx, 2)*lambda + 0.00165633 *lens_ipow(y, 2)*dx*lambda + 5.81234e-06 *lens_ipow(y, 4)*dx + 6.72492 *lens_ipow(dx, 3)*lens_ipow(lambda, 2) + -6.06714 *lens_ipow(dx, 3)*lens_ipow(lambda, 3) + 0.00621329 *lens_ipow(x, 2)*dx*lens_ipow(lambda, 3) + 0.000999368 *lens_ipow(y, 3)*dx*dy*lens_ipow(lambda, 2) + -5.56523e-12 *lens_ipow(x, 7)*lens_ipow(y, 2) + 3.86556e-09 *x*lens_ipow(y, 6)*lens_ipow(lambda, 2) + 5.03358e-05 *x*lens_ipow(y, 3)*dy*lens_ipow(lambda, 4) + 1.35499e-07 *x*lens_ipow(y, 5)*dy*lens_ipow(lambda, 2) + -3.11126e-05 *lens_ipow(x, 4)*dx*lens_ipow(lambda, 4) + 6.72353e-09 *lens_ipow(x, 3)*lens_ipow(y, 4)*lens_ipow(lambda, 2) + 171.022 *lens_ipow(dx, 5)*lens_ipow(dy, 4)*lambda + -0.00458756 *lens_ipow(y, 3)*dx*lens_ipow(dy, 3)*lens_ipow(lambda, 3) + -2.83521e-11 *lens_ipow(x, 3)*lens_ipow(y, 6)*lambda + -2.96283e-11 *lens_ipow(x, 5)*lens_ipow(y, 4)*lambda + -3.03637e-12 *lens_ipow(x, 9)*lambda + 3.03521e-08 *lens_ipow(y, 6)*dx*lens_ipow(lambda, 4) + 8.32911e-05 *lens_ipow(x, 4)*lens_ipow(dx, 3)*lens_ipow(lambda, 4) + -1.12336e-11 *x*lens_ipow(y, 8)*lens_ipow(lambda, 2) + 1.24652e-05 *x*lens_ipow(y, 4)*lens_ipow(dy, 2)*lens_ipow(lambda, 4);
      out[3] =  + -0.0812351 *y + -0.280867 *dy + 0.0429656 *dy*lambda + 0.0140781 *y*lambda + 0.00330655 *lens_ipow(y, 2)*dy + 0.106182 *y*lens_ipow(dy, 2) + 0.0529577 *x*dx*dy + -0.00852811 *y*lens_ipow(lambda, 2) + 0.000112606 *lens_ipow(x, 2)*y + 8.92269e-05 *lens_ipow(y, 3) + 0.00131885 *lens_ipow(x, 2)*dy + 1.11666 *lens_ipow(dy, 3) + 1.72313 *lens_ipow(dx, 2)*dy*lambda + 0.00206368 *x*y*dx*lambda + 0.0544768 *x*dx*dy*lens_ipow(lambda, 2) + -0.0347859 *x*lens_ipow(dx, 3)*dy + 0.114422 *y*lens_ipow(dx, 2)*lens_ipow(lambda, 2) + 0.00361735 *lens_ipow(y, 2)*lens_ipow(dx, 2)*dy + -0.0311424 *y*lens_ipow(dy, 4) + 9.33907e-05 *lens_ipow(y, 3)*lens_ipow(dx, 2) + -0.0805241 *y*lens_ipow(dx, 2)*lens_ipow(lambda, 3) + 1.877e-05 *lens_ipow(x, 4)*lens_ipow(dy, 3)*lambda + 2.03187e-06 *lens_ipow(x, 3)*lens_ipow(y, 2)*dx*dy*lens_ipow(lambda, 2) + -1.02479e-12 *lens_ipow(y, 9)*lambda + -5.92894e-12 *lens_ipow(x, 2)*lens_ipow(y, 7)*lambda + -2.83505e-15 *lens_ipow(y, 11) + -3.67053e-14 *lens_ipow(x, 8)*lens_ipow(y, 3) + -4.77863e-14 *lens_ipow(x, 4)*lens_ipow(y, 7);
      out_transmittance =  + 0.829835 *lambda + -1.08215 *lens_ipow(lambda, 2) + 0.503594 *lens_ipow(lambda, 3) + 0.0145961 *y*lens_ipow(dy, 3) + -0.568749 *lens_ipow(dx, 2)*lens_ipow(dy, 2) + -2.16067e-06 *lens_ipow(x, 2)*lens_ipow(y, 2) + -3.82145e-05 *lens_ipow(y, 3)*dy + -1.76453e-06 *lens_ipow(x, 4)*lambda + -5.34716e-07 *lens_ipow(y, 4)*lambda + 0.00280576 *x*y*dx*dy*lambda + -0.793914 *lens_ipow(dx, 6) + -2.16535e-08 *lens_ipow(y, 6) + -4.93127e-07 *lens_ipow(x, 5)*dx*lambda + -4.54478e-09 *lens_ipow(x, 3)*lens_ipow(y, 4)*dx + 1.46013e-10 *lens_ipow(y, 8) + -2.45372e-11 *lens_ipow(x, 8) + -0.0114921 *x*y*lens_ipow(dx, 3)*dy*lens_ipow(lambda, 2) + -7.97349e-09 *lens_ipow(x, 4)*lens_ipow(y, 3)*dy*lambda + -0.0138635 *x*y*dx*lens_ipow(dy, 3)*lens_ipow(lambda, 3) + -3.07018e-07 *lens_ipow(x, 3)*lens_ipow(y, 3)*dx*dy*lambda + 1.30754e-10 *lens_ipow(x, 2)*lens_ipow(y, 6)*lambda + -3.74594e-13 *lens_ipow(y, 10) + -5.66313e-05 *lens_ipow(x, 3)*dx*lens_ipow(lambda, 6) + -1.20621e-12 *lens_ipow(x, 4)*lens_ipow(y, 6) + -6.28574e-08 *lens_ipow(x, 6)*lens_ipow(dx, 2)*lens_ipow(lambda, 2) + -2.57329e-13 *lens_ipow(x, 8)*lens_ipow(y, 2) + -1.16038e-12 *lens_ipow(x, 2)*lens_ipow(y, 8)*lambda + -2.06616e-12 *lens_ipow(x, 6)*lens_ipow(y, 4)*lambda;
    } break;


    case fisheye:
    {
      out[0] =  + -8.23478 *x + -23.6448 *dx*lambda + 0.209775 *lens_ipow(x, 2)*dx + -0.00567908 *x*lens_ipow(y, 2) + 7.19223 *x*lens_ipow(dy, 2) + -0.0100983 *lens_ipow(x, 3)*lambda + 0.302739 *x*y*dy*lambda + 1207.94 *x*lens_ipow(dx, 4) + 31.3281 *lens_ipow(x, 2)*lens_ipow(dx, 3) + 10503.8 *lens_ipow(dx, 5) + 0.350912 *lens_ipow(x, 3)*lens_ipow(dx, 2) + 8722.78 *lens_ipow(dx, 3)*lens_ipow(dy, 2) + 1038.42 *y*lens_ipow(dx, 3)*dy*lambda + 3375.71 *dx*lens_ipow(dy, 4)*lambda + -0.000607626 *lens_ipow(x, 4)*y*dx*dy + -1941.37 *x*lens_ipow(dx, 6) + 18.8515 *lens_ipow(y, 2)*lens_ipow(dx, 3)*lens_ipow(lambda, 2) + -8484.66 *y*lens_ipow(dx, 5)*dy*lambda + 14101.2 *y*lens_ipow(dx, 7)*dy + -157.75 *lens_ipow(y, 2)*lens_ipow(dx, 5)*lens_ipow(lambda, 3) + 987.563 *x*lens_ipow(dx, 8)*dy + 1.10767e-08 *lens_ipow(x, 3)*lens_ipow(y, 6)*lambda + 56171.5 *x*lens_ipow(dx, 8)*lens_ipow(dy, 2) + 1.62373e-08 *lens_ipow(x, 7)*lens_ipow(y, 2)*lens_ipow(lambda, 2) + 12.4767 *lens_ipow(y, 3)*dx*lens_ipow(dy, 7) + -3.06799 *lens_ipow(x, 4)*lens_ipow(dx, 3)*lens_ipow(dy, 4) + -958.675 *lens_ipow(x, 2)*lens_ipow(dx, 9) + -6.1304 *x*lens_ipow(y, 3)*lens_ipow(dx, 4)*lens_ipow(dy, 3);
      out[1] =  + -8.37596 *y + -14.0792 *dy + 8.14436 *y*lens_ipow(dx, 2) + 0.642083 *lens_ipow(y, 2)*dy + 30.1401 *y*lens_ipow(dy, 2) + 4.61396 *x*dx*dy + 0.213958 *x*y*dx + -0.00395195 *lens_ipow(x, 2)*y + 294.206 *lens_ipow(dy, 3) + -0.00520918 *lens_ipow(y, 3)*lambda + 9648.97 *lens_ipow(dx, 2)*lens_ipow(dy, 3) + 374.136 *x*dx*lens_ipow(dy, 3) + 6.27376 *lens_ipow(x, 2)*lens_ipow(dy, 3) + 2263.15 *lens_ipow(dx, 4)*dy + -1.22252e-05 *lens_ipow(y, 6)*dy + 41765.7 *lens_ipow(dy, 7) + 1.13934 *lens_ipow(x, 2)*y*lens_ipow(dy, 4) + 1751.59 *y*lens_ipow(dy, 6) + -1.31667 *x*lens_ipow(y, 2)*dx*lens_ipow(dy, 3) + 0.555329 *lens_ipow(y, 4)*lens_ipow(dy, 5) + -0.0200994 *lens_ipow(x, 2)*lens_ipow(y, 3)*lens_ipow(dx, 2)*lens_ipow(dy, 2) + 10.3339 *lens_ipow(x, 3)*lens_ipow(dx, 7)*dy + -3.3773 *lens_ipow(x, 3)*y*lens_ipow(dx, 5)*lens_ipow(dy, 2) + -3.27086 *lens_ipow(y, 4)*lens_ipow(dx, 2)*lens_ipow(dy, 5) + -2.40173 *lens_ipow(y, 4)*lens_ipow(dy, 7) + 1.06144e-10 *lens_ipow(x, 4)*lens_ipow(y, 7) + -0.000123388 *x*lens_ipow(y, 6)*dx*lens_ipow(dy, 3) + 0.379743 *x*lens_ipow(y, 4)*lens_ipow(dx, 3)*lens_ipow(dy, 3);
      out[2] =  + -0.103083 *x + -0.601438 *dx*lambda + 0.234038 *y*dx*dy + 0.00352225 *x*y*dy + 5.38731 *dx*lens_ipow(dy, 2) + 4.57178e-05 *x*lens_ipow(y, 2) + 0.000216091 *lens_ipow(x, 3) + 0.107377 *x*lens_ipow(dy, 2) + 0.852366 *x*lens_ipow(dx, 2)*lambda + 0.0184197 *lens_ipow(x, 2)*dx*lambda + 0.0040717 *lens_ipow(y, 2)*dx*lambda + 17.8917 *lens_ipow(dx, 3)*lens_ipow(lambda, 2) + -0.000423037 *lens_ipow(x, 2)*y*dx*dy + -0.0197977 *lens_ipow(x, 2)*dx*lens_ipow(lambda, 3) + 71.8647 *lens_ipow(dx, 5)*lambda + 9.52493e-05 *lens_ipow(x, 4)*lens_ipow(dx, 3) + -0.00668964 *lens_ipow(y, 3)*dx*lens_ipow(dy, 3) + 0.000421438 *lens_ipow(y, 4)*lens_ipow(dx, 3) + -4.09222e-09 *x*lens_ipow(y, 6) + -1.25617 *x*lens_ipow(dx, 2)*lens_ipow(lambda, 4) + 455.971 *lens_ipow(dx, 5)*lens_ipow(dy, 2) + -0.000581608 *lens_ipow(y, 4)*lens_ipow(dx, 3)*lambda + -492.467 *lens_ipow(dx, 5)*lens_ipow(lambda, 4) + 132.875 *x*lens_ipow(dx, 6)*lens_ipow(dy, 2) + 61.7322 *x*lens_ipow(dx, 8)*lambda + 1828.09 *lens_ipow(dx, 7)*lens_ipow(lambda, 4) + -1.09217e-05 *lens_ipow(y, 6)*dx*lens_ipow(dy, 4) + 0.00256169 *x*lens_ipow(y, 4)*lens_ipow(dx, 4)*lens_ipow(dy, 2);
      out[3] =  + -0.103601 *y + -0.713373 *dy*lambda + 0.000378384 *lens_ipow(x, 2)*y + 0.000247393 *lens_ipow(y, 3) + 0.129964 *y*lens_ipow(dx, 2)*lambda + 0.00402109 *lens_ipow(x, 2)*dy*lambda + 0.0165033 *lens_ipow(y, 2)*dy*lambda + 0.00729411 *x*y*dx*lambda + 0.693063 *x*dx*dy*lens_ipow(lambda, 2) + 0.409198 *dy*lens_ipow(lambda, 4) + 32.9633 *lens_ipow(dy, 3)*lens_ipow(lambda, 2) + -1.96139e-07 *lens_ipow(y, 5) + 18.4048 *lens_ipow(dx, 2)*dy*lens_ipow(lambda, 2) + 2.80892 *y*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + 0.563506 *y*lens_ipow(dx, 4)*lambda + -0.0130565 *lens_ipow(y, 2)*dy*lens_ipow(lambda, 3) + 7.74914e-07 *lens_ipow(x, 2)*lens_ipow(y, 3)*lambda + -3.11538 *y*lens_ipow(dy, 2)*lens_ipow(lambda, 3) + -44.8493 *lens_ipow(dy, 3)*lens_ipow(lambda, 4) + 0.00454339 *lens_ipow(x, 2)*dy*lens_ipow(lambda, 4) + 0.00015627 *lens_ipow(x, 2)*y*lens_ipow(lambda, 5) + 10.8648 *x*dx*lens_ipow(dy, 5)*lens_ipow(lambda, 2) + -16.3559 *y*lens_ipow(dy, 8) + 0.509567 *x*y*dx*lens_ipow(dy, 2)*lens_ipow(lambda, 6) + -0.000484984 *lens_ipow(y, 5)*lens_ipow(dy, 6) + -0.0186708 *lens_ipow(y, 4)*lens_ipow(dy, 7) + -7.7512e-10 *lens_ipow(x, 6)*lens_ipow(y, 3)*lens_ipow(dx, 2) + 12.1926 *y*lens_ipow(dx, 2)*lens_ipow(dy, 2)*lens_ipow(lambda, 6);
      out_transmittance =  + 0.196001  + 0.378559 *lambda + 0.128762 *lens_ipow(dy, 2) + -0.267516 *lens_ipow(lambda, 2) + -3.43289 *lens_ipow(dy, 4) + -5.30271e-06 *lens_ipow(x, 2)*lens_ipow(y, 2) + -0.00124765 *x*lens_ipow(y, 2)*dx*lens_ipow(dy, 2) + -1.33218e-08 *lens_ipow(x, 6) + -1.27157e-08 *lens_ipow(y, 6) + -331.504 *lens_ipow(dx, 4)*lens_ipow(dy, 2) + -22.8135 *y*lens_ipow(dy, 7) + -0.435257 *lens_ipow(y, 2)*lens_ipow(dy, 6) + 4657.45 *lens_ipow(dx, 4)*lens_ipow(dy, 4) + -0.644826 *x*lens_ipow(dx, 3)*lens_ipow(lambda, 4) + 4717.32 *lens_ipow(dx, 6)*lens_ipow(dy, 2) + 7.77492 *x*lens_ipow(dx, 5)*lens_ipow(lambda, 2) + 129.97 *lens_ipow(dy, 8)*lambda + -44.8792 *x*lens_ipow(dx, 7)*lambda + -2092.11 *lens_ipow(dy, 10) + -3144.49 *lens_ipow(dx, 2)*lens_ipow(dy, 8) + -20701.6 *lens_ipow(dx, 4)*lens_ipow(dy, 6) + -42660.6 *lens_ipow(dx, 6)*lens_ipow(dy, 4) + -19041.8 *lens_ipow(dx, 8)*lens_ipow(dy, 2) + -1799.27 *lens_ipow(dx, 10) + 31.9804 *x*y*lens_ipow(dx, 5)*lens_ipow(dy, 3)*lambda + 0.0626751 *lens_ipow(x, 3)*lens_ipow(dx, 7)*lambda + 5.56252e-07 *lens_ipow(x, 3)*lens_ipow(y, 4)*dx*lens_ipow(dy, 2)*lambda + -0.0101147 *x*lens_ipow(y, 3)*lens_ipow(dx, 5)*dy*lambda;
    } break;


    case wideangle:
    {
      out[0] =  + 16.7222 *dx + -1.7126 *x + -0.407092 *x*lambda + -1.50065 *dx*lambda + 2.13945 *y*dx*dy + 0.122543 *lens_ipow(x, 2)*dx + 25.686 *lens_ipow(dx, 3) + 0.265989 *x*lens_ipow(lambda, 2) + 29.8535 *dx*lens_ipow(dy, 2) + 0.00320318 *x*lens_ipow(y, 2) + 0.00349548 *lens_ipow(x, 3) + 1.05804 *x*lens_ipow(dy, 2) + 3.24433 *x*lens_ipow(dx, 2) + 0.0495496 *lens_ipow(y, 2)*dx + -1.2933 *x*lens_ipow(dx, 2)*lambda + 0.30668 *x*y*dy*lambda + -0.83918 *y*dx*dy*lambda + -3.57407e-06 *lens_ipow(x, 5) + 0.298459 *lens_ipow(y, 2)*lens_ipow(dx, 3) + -0.388491 *x*y*dy*lens_ipow(lambda, 2) + 0.0189128 *x*lens_ipow(y, 2)*lens_ipow(dx, 2)*lambda + -0.000272989 *lens_ipow(x, 4)*dx*lambda + 0.000109252 *lens_ipow(x, 3)*lens_ipow(y, 2)*lens_ipow(dy, 2) + -7.08814e-08 *lens_ipow(x, 3)*lens_ipow(y, 4) + 0.00565248 *lens_ipow(x, 2)*lens_ipow(y, 2)*dx*lens_ipow(dy, 2) + 0.225298 *x*y*dy*lens_ipow(lambda, 5) + -0.112286 *x*lens_ipow(y, 3)*lens_ipow(dx, 4)*dy + -7.93361e-10 *lens_ipow(x, 7)*lens_ipow(y, 2)*lens_ipow(lambda, 2);
      out[1] =  + -1.72441 *y + 15.9053 *dy + -0.377417 *y*lambda + 0.96701 *y*lens_ipow(dx, 2) + 0.174206 *lens_ipow(y, 2)*dy + 5.43202 *y*lens_ipow(dy, 2) + 1.53423 *x*dx*dy + 26.636 *lens_ipow(dx, 2)*dy + 0.252523 *y*lens_ipow(lambda, 2) + 0.00320864 *lens_ipow(x, 2)*y + 0.0034628 *lens_ipow(y, 3) + 0.0508916 *lens_ipow(x, 2)*dy + 61.5968 *lens_ipow(dy, 3) + -5.08006 *y*lens_ipow(dy, 2)*lambda + -60.4737 *lens_ipow(dy, 3)*lambda + -0.0976647 *lens_ipow(y, 2)*dy*lambda + 0.234352 *x*y*dx*lambda + 0.00415282 *lens_ipow(y, 3)*lens_ipow(dx, 2) + -0.000238714 *lens_ipow(y, 4)*dy*lambda + 0.36372 *lens_ipow(y, 2)*lens_ipow(dx, 2)*dy*lambda + -6.27167e-06 *lens_ipow(y, 5)*lambda + -0.807132 *x*y*dx*lens_ipow(lambda, 3) + -0.00640517 *lens_ipow(x, 3)*y*dx*lens_ipow(dy, 2) + 0.722321 *x*y*dx*lens_ipow(lambda, 4) + -5.45787e-08 *lens_ipow(x, 2)*lens_ipow(y, 5)*lambda + -6.20086e-10 *lens_ipow(x, 6)*lens_ipow(y, 3) + 0.0654266 *lens_ipow(x, 4)*lens_ipow(dy, 5) + 0.0037559 *lens_ipow(x, 4)*y*lens_ipow(dy, 4)*lambda;
      out[2] =  + -0.334003 *dx + -0.026849 *x + 0.0120662 *x*lambda + -0.0196292 *dx*lambda + 0.49947 *lens_ipow(dx, 3) + -0.00827016 *x*lens_ipow(lambda, 2) + 0.435908 *dx*lens_ipow(dy, 2) + -1.03771e-05 *x*lens_ipow(y, 2) + 0.00186831 *x*lens_ipow(dy, 2) + 0.015388 *x*lens_ipow(dx, 2) + 0.00014192 *lens_ipow(y, 2)*dx + 9.35478e-05 *lens_ipow(x, 3)*lambda + -0.000392059 *lens_ipow(x, 2)*dx*lambda + 0.0477328 *y*dx*dy*lambda + -0.0481053 *y*dx*dy*lens_ipow(lambda, 2) + -0.000148114 *lens_ipow(x, 3)*lens_ipow(lambda, 2) + -0.0221637 *x*lens_ipow(dx, 2)*lens_ipow(lambda, 2) + -4.44502e-08 *lens_ipow(x, 5) + -0.00671039 *x*y*dy*lens_ipow(lambda, 2) + 0.0174403 *x*y*dy*lens_ipow(lambda, 3) + -1.96719e-06 *lens_ipow(x, 3)*y*dy*lambda + -2.82448e-06 *lens_ipow(x, 4)*dx*lambda + 1.35905e-06 *x*lens_ipow(y, 4)*lens_ipow(dx, 2) + 8.77094e-05 *lens_ipow(x, 3)*lens_ipow(lambda, 4) + 4.46548e-05 *lens_ipow(y, 4)*lens_ipow(dx, 3) + -0.0124094 *x*y*dy*lens_ipow(lambda, 4) + 0.00219052 *lens_ipow(y, 3)*lens_ipow(dx, 3)*dy*lambda + -2.70974 *lens_ipow(dx, 5)*lens_ipow(lambda, 3);
      out[3] =  + -0.0266125 *y + -0.334066 *dy + -0.0170558 *dy*lambda + 0.0114614 *y*lambda + -0.00511334 *y*lens_ipow(dx, 2) + 0.0129436 *y*lens_ipow(dy, 2) + 0.00896579 *x*dx*dy + 0.477093 *lens_ipow(dx, 2)*dy + -0.00789395 *y*lens_ipow(lambda, 2) + 3.90402e-05 *lens_ipow(x, 2)*y + -0.000338784 *lens_ipow(x, 2)*dy + 0.457993 *lens_ipow(dy, 3) + -0.000567731 *lens_ipow(y, 2)*dy*lambda + 5.55763e-05 *lens_ipow(y, 3)*lambda + -2.51e-06 *x*lens_ipow(y, 3)*dx + -1.24306e-07 *lens_ipow(x, 2)*lens_ipow(y, 3) + -1.92408e-06 *lens_ipow(x, 3)*y*dx + -9.87323e-08 *lens_ipow(x, 4)*y + -2.58648e-06 *lens_ipow(x, 2)*lens_ipow(y, 2)*dy + -0.0150597 *y*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + 0.00600884 *lens_ipow(y, 2)*lens_ipow(dx, 2)*dy*lambda + -0.000216745 *lens_ipow(y, 3)*lens_ipow(lambda, 3) + -2.56526e-10 *lens_ipow(y, 7) + 0.000199 *lens_ipow(y, 3)*lens_ipow(lambda, 4) + -0.00148407 *lens_ipow(x, 3)*y*dx*lens_ipow(dy, 4) + -7.61229e-11 *lens_ipow(y, 8)*dy + 0.00038575 *lens_ipow(x, 4)*lens_ipow(dy, 5) + 0.0287351 *lens_ipow(y, 3)*lens_ipow(dx, 4)*lens_ipow(dy, 2);
      out_transmittance =  + 1.07795 *lambda + -0.000434464 *y*dy + -1.13994e-05 *lens_ipow(y, 2) + -1.80117 *lens_ipow(lambda, 2) + 1.13999 *lens_ipow(lambda, 3) + -0.000660056 *lens_ipow(y, 2)*lens_ipow(dx, 2) + -0.0815446 *x*lens_ipow(dx, 3) + -8.44238 *lens_ipow(dx, 2)*lens_ipow(dy, 2) + -1.0738e-05 *lens_ipow(x, 2)*y*dy + -0.00279317 *lens_ipow(x, 2)*lens_ipow(dx, 2) + -0.150544 *y*lens_ipow(dx, 2)*dy + -3.27969e-05 *lens_ipow(x, 3)*dx + -0.160615 *x*dx*lens_ipow(dy, 2) + -2.25778e-07 *lens_ipow(x, 4) + -0.0025113 *x*y*dx*dy + -0.000994204 *lens_ipow(x, 2)*lens_ipow(dy, 2) + -2.06406e-05 *lens_ipow(y, 4)*lens_ipow(dy, 2) + -98.5938 *lens_ipow(dx, 6) + -0.195942 *lens_ipow(lambda, 6) + -2.20023 *x*lens_ipow(dx, 5) + -0.00302466 *lens_ipow(y, 3)*lens_ipow(dy, 3) + -87.6123 *lens_ipow(dy, 6) + -0.179227 *lens_ipow(y, 2)*lens_ipow(dy, 4) + -5.59084 *y*lens_ipow(dy, 5) + -0.318447 *lens_ipow(x, 2)*lens_ipow(dx, 6) + -1415.1 *lens_ipow(dx, 4)*lens_ipow(dy, 4) + 394.675 *lens_ipow(dx, 8) + 0.671785 *x*y*lens_ipow(dx, 3)*lens_ipow(dy, 3);
    } break;


    case takumar_1969_50mm:
    {
      out[0] =  + 49.8032 *dx + 0.0143226 *x*y*dy + 0.0149205 *lens_ipow(x, 2)*dx + -20.8277 *lens_ipow(dx, 3) + -21.2379 *dx*lens_ipow(dy, 2) + -0.000546669 *x*lens_ipow(y, 2) + -0.00103006 *lens_ipow(x, 3) + 0.300221 *x*lens_ipow(dx, 2) + 0.00687478 *lens_ipow(y, 2)*dx + 0.556502 *x*lens_ipow(dx, 2)*lambda + 1.14168 *x*lens_ipow(dy, 2)*lambda + 0.000347782 *x*lens_ipow(y, 2)*lambda + 0.00191234 *lens_ipow(x, 3)*lambda + 0.0160337 *lens_ipow(x, 2)*dx*lambda + -0.00119575 *lens_ipow(x, 3)*lens_ipow(lambda, 2) + 0.00142374 *lens_ipow(x, 3)*lens_ipow(dy, 2) + -0.632997 *x*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + 0.000748107 *x*lens_ipow(y, 2)*lens_ipow(dy, 2) + -0.00165018 *lens_ipow(x, 3)*lens_ipow(dy, 2)*lambda + -3.27465e-05 *lens_ipow(x, 4)*dx*lens_ipow(lambda, 2) + -1.81354e-09 *lens_ipow(x, 5)*lens_ipow(y, 2) + -7.07079e-10 *x*lens_ipow(y, 6) + -1.95349 *x*lens_ipow(dx, 4)*lens_ipow(lambda, 2) + -2.73806 *x*lens_ipow(dx, 2)*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + -3.13846e-09 *lens_ipow(x, 3)*lens_ipow(y, 4)*lambda + 0.0657627 *x*y*lens_ipow(dy, 3)*lens_ipow(lambda, 3) + -2.56873e-09 *lens_ipow(x, 7)*lens_ipow(lambda, 2) + -0.000182029 *lens_ipow(y, 5)*lens_ipow(dx, 3)*lens_ipow(dy, 3);
      out[1] =  + -0.141503 *y + 49.4365 *dy + 0.548589 *dy*lambda + 0.41489 *y*lambda + 0.0244066 *lens_ipow(y, 2)*dy + 0.0146213 *x*y*dx + -20.9484 *lens_ipow(dx, 2)*dy + -0.297076 *y*lens_ipow(lambda, 2) + -0.000312083 *lens_ipow(x, 2)*y + -0.000250017 *lens_ipow(y, 3) + 0.00729085 *lens_ipow(x, 2)*dy + -20.3828 *lens_ipow(dy, 3) + 3.33439 *y*lens_ipow(dy, 2)*lambda + 2.51912 *y*lens_ipow(dx, 2)*lambda + -3.78891 *y*lens_ipow(dx, 2)*lens_ipow(lambda, 2) + -1.13034e-06 *lens_ipow(x, 2)*lens_ipow(y, 3) + 0.00048002 *lens_ipow(y, 3)*lens_ipow(dx, 2) + -0.807166 *y*lens_ipow(dx, 2)*lens_ipow(dy, 2) + -5.2979e-07 *lens_ipow(y, 5) + -5.07164 *y*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + -2.56011e-05 *lens_ipow(y, 4)*dy*lambda + 1.06779e-06 *lens_ipow(x, 2)*lens_ipow(y, 3)*lambda + -1.08743e-09 *lens_ipow(x, 6)*y + 2.24155 *y*lens_ipow(dx, 2)*lens_ipow(lambda, 4) + 3.04667 *y*lens_ipow(dy, 2)*lens_ipow(lambda, 4) + 0.0920095 *lens_ipow(y, 2)*lens_ipow(dy, 5) + 3.60861e-06 *lens_ipow(x, 4)*y*lens_ipow(dx, 2)*lambda + -6.55673e-12 *lens_ipow(x, 4)*lens_ipow(y, 5);
      out[2] =  + -0.910038 *dx + -0.0193261 *x + -0.00125658 *x*lambda + -0.000251218 *x*y*dy + -0.000174838 *lens_ipow(x, 2)*dx + 0.4129 *lens_ipow(dx, 3) + 0.0963517 *dx*lens_ipow(dy, 2) + 8.84115e-06 *x*lens_ipow(y, 2) + 2.17952e-05 *lens_ipow(x, 3) + -0.0273656 *x*lens_ipow(dy, 2)*lambda + -4.84779e-05 *lens_ipow(x, 3)*lambda + 6.32952e-05 *lens_ipow(y, 2)*dx*lambda + 2.10771e-08 *lens_ipow(x, 3)*lens_ipow(y, 2) + -2.16392e-07 *lens_ipow(x, 4)*dx + 4.52445e-05 *lens_ipow(x, 3)*lens_ipow(lambda, 2) + -3.78941e-06 *lens_ipow(x, 3)*lens_ipow(dy, 2) + 2.33559e-05 *lens_ipow(x, 2)*y*dx*dy + 0.0814277 *x*lens_ipow(dy, 2)*lens_ipow(lambda, 3) + -2.28441e-08 *lens_ipow(x, 3)*lens_ipow(y, 2)*lambda + -3.87632e-06 *x*lens_ipow(y, 3)*lens_ipow(dx, 2)*dy + -0.0687767 *x*lens_ipow(dy, 2)*lens_ipow(lambda, 4) + 0.00341197 *lens_ipow(y, 2)*lens_ipow(dx, 3)*lens_ipow(lambda, 2) + 2.89162e-11 *lens_ipow(x, 7)*lambda + -0.00400648 *lens_ipow(y, 2)*lens_ipow(dx, 3)*lens_ipow(lambda, 3) + 1.58178e-13 *lens_ipow(x, 5)*lens_ipow(y, 4) + 3.50375e-14 *x*lens_ipow(y, 8) + -6.49335e-09 *lens_ipow(y, 6)*dx*lens_ipow(dy, 2) + -5.95238e-08 *lens_ipow(x, 5)*lens_ipow(lambda, 6);
      out[3] =  + -0.0178342 *y + -0.908344 *dy + -0.00663163 *y*lambda + -0.007604 *y*lens_ipow(dx, 2) + -0.000187248 *lens_ipow(y, 2)*dy + 0.0159022 *x*dx*dy + -0.000243681 *x*y*dx + 0.761103 *lens_ipow(dx, 2)*dy + 0.00492946 *y*lens_ipow(lambda, 2) + 7.69502e-06 *lens_ipow(x, 2)*y + 7.76167e-06 *lens_ipow(y, 3) + 0.40198 *lens_ipow(dy, 3) + 4.60293e-05 *lens_ipow(x, 2)*dy*lambda + 0.000907222 *lens_ipow(x, 2)*lens_ipow(dx, 2)*dy + -5.04469e-07 *lens_ipow(y, 4)*dy + 3.23617e-08 *lens_ipow(x, 2)*lens_ipow(y, 3) + -6.79163e-06 *lens_ipow(y, 3)*lens_ipow(dx, 2) + 7.16848e-09 *lens_ipow(x, 4)*y + 6.51238e-09 *lens_ipow(y, 5) + 0.000381824 *lens_ipow(x, 2)*lens_ipow(dy, 3) + 1.13086e-05 *lens_ipow(x, 3)*dx*dy + -0.383626 *lens_ipow(dx, 4)*dy + 6.49928e-07 *lens_ipow(y, 4)*dy*lambda + -3.25634e-08 *lens_ipow(x, 2)*lens_ipow(y, 3)*lambda + 2.0029e-06 *lens_ipow(y, 4)*lens_ipow(dx, 2)*dy + -3.86222e-06 *x*lens_ipow(y, 3)*dx*lens_ipow(dy, 2) + -4.87562e-12 *lens_ipow(x, 6)*lens_ipow(y, 2)*dy + -1.16396e-11 *lens_ipow(x, 2)*lens_ipow(y, 6)*dy*lens_ipow(lambda, 2);
      out_transmittance =  + 2.13553 *lambda + -4.72826 *lens_ipow(lambda, 2) + -0.00103031 *y*dy*lambda + -0.00102465 *x*dx*lambda + -1.11271e-05 *lens_ipow(x, 2)*lambda + -1.11545e-05 *lens_ipow(y, 2)*lambda + 4.91467 *lens_ipow(lambda, 3) + -1.97319 *lens_ipow(lambda, 4) + -0.361162 *lens_ipow(dx, 4) + -0.0189262 *y*lens_ipow(dy, 3) + -0.00017862 *lens_ipow(y, 2)*lens_ipow(dx, 2) + -0.0190907 *x*lens_ipow(dx, 3) + -0.918305 *lens_ipow(dx, 2)*lens_ipow(dy, 2) + -0.35869 *lens_ipow(dy, 4) + -9.73633e-08 *lens_ipow(x, 2)*lens_ipow(y, 2) + -0.000498028 *lens_ipow(x, 2)*lens_ipow(dx, 2) + -0.0192091 *y*lens_ipow(dx, 2)*dy + -0.000495702 *lens_ipow(y, 2)*lens_ipow(dy, 2) + -0.0193094 *x*dx*lens_ipow(dy, 2) + -0.000651255 *x*y*dx*dy + -0.000179837 *lens_ipow(x, 2)*lens_ipow(dy, 2) + -0.169861 *lens_ipow(dx, 4)*lambda + -0.17091 *lens_ipow(dy, 4)*lambda + 1.62928e-07 *lens_ipow(x, 2)*lens_ipow(y, 2)*lambda + -4.02531e-16 *lens_ipow(y, 10) + -2.37748e-15 *lens_ipow(x, 2)*lens_ipow(y, 8) + -7.81748e-15 *lens_ipow(x, 6)*lens_ipow(y, 4) + -5.03842e-16 *lens_ipow(x, 10);
    } break;


    case zeiss_biotar_1927_58mm:
    {
      out[0] =  + 57.1689 *dx + 0.193146 *x + 0.0484321 *x*lambda + 0.389705 *y*dx*dy + 0.0233441 *x*y*dy + 0.0310786 *lens_ipow(x, 2)*dx + -23.7113 *lens_ipow(dx, 3) + -23.7666 *dx*lens_ipow(dy, 2) + -8.51031e-05 *lens_ipow(x, 3) + 1.06525 *x*lens_ipow(dx, 2) + 0.00892956 *lens_ipow(y, 2)*dx + 1.01113 *dx*lens_ipow(lambda, 2) + 2.53406 *x*lens_ipow(dy, 2)*lambda + 0.000108935 *lens_ipow(x, 3)*lambda + -1.85433e-06 *lens_ipow(x, 3)*lens_ipow(y, 2) + 0.10017 *x*y*lens_ipow(dx, 2)*dy + -6.25497e-05 *lens_ipow(x, 4)*dx + 0.06383 *lens_ipow(x, 2)*lens_ipow(dx, 3) + -9.03315e-07 *lens_ipow(x, 5) + -9.64335e-07 *x*lens_ipow(y, 4) + -7.09054e-05 *lens_ipow(x, 2)*lens_ipow(y, 2)*dx + -5.51919e-05 *lens_ipow(x, 3)*y*dy + -5.40607e-05 *x*lens_ipow(y, 3)*dy + 0.0395167 *lens_ipow(y, 2)*dx*lens_ipow(dy, 2) + -2.25422 *x*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + 0.0239359 *x*y*lens_ipow(dy, 3) + 0.052306 *lens_ipow(x, 2)*dx*lens_ipow(dy, 2)*lambda + -1.71832e-05 *lens_ipow(y, 4)*dx*lambda;
      out[1] =  + 0.223232 *y + 57.5672 *dy + 0.691113 *y*lens_ipow(dx, 2) + 0.017258 *lens_ipow(y, 2)*dy + 0.843734 *y*lens_ipow(dy, 2) + 0.39208 *x*dx*dy + 0.0232544 *x*y*dx + -23.9144 *lens_ipow(dx, 2)*dy + -0.000545388 *lens_ipow(y, 3) + 0.00915536 *lens_ipow(x, 2)*dy + -25.0805 *lens_ipow(dy, 3) + 0.000785302 *lens_ipow(y, 3)*lambda + 0.0413811 *lens_ipow(x, 2)*lens_ipow(dx, 2)*dy + -1.07207e-05 *lens_ipow(x, 4)*dy + -5.43478e-05 *x*lens_ipow(y, 3)*dx + 0.0238682 *x*y*lens_ipow(dx, 3) + 0.0188301 *lens_ipow(y, 2)*dy*lens_ipow(lambda, 2) + 0.100844 *x*y*dx*lens_ipow(dy, 2) + 0.0275789 *lens_ipow(y, 2)*lens_ipow(dx, 2)*dy + -1.83776e-06 *lens_ipow(x, 2)*lens_ipow(y, 3) + -5.44981e-05 *lens_ipow(x, 3)*y*dx + -1.19686e-06 *lens_ipow(x, 4)*y + -7.01775e-05 *lens_ipow(x, 2)*lens_ipow(y, 2)*dy + 0.00737572 *lens_ipow(y, 3)*lens_ipow(dy, 2)*lambda + 4.09626e-07 *lens_ipow(x, 4)*y*lambda + 0.348138 *lens_ipow(y, 2)*lens_ipow(dy, 3)*lambda + -1.64278e-06 *lens_ipow(y, 5)*lens_ipow(lambda, 2) + 5.19554 *y*lens_ipow(dy, 4)*lens_ipow(lambda, 2);
      out[2] =  + -1.19429 *dx + -0.0217102 *x + -0.000720481 *x*lambda + -0.00940156 *y*dx*dy + -0.000254619 *x*y*dy + -0.000118977 *lens_ipow(x, 2)*dx + 0.587852 *lens_ipow(dx, 3) + -0.204803 *dx*lens_ipow(dy, 2) + 7.50277e-06 *x*lens_ipow(y, 2) + 6.29155e-06 *lens_ipow(x, 3) + -0.00442155 *x*lens_ipow(dx, 2) + 7.82376e-05 *lens_ipow(y, 2)*dx + -0.0537613 *x*lens_ipow(dy, 2)*lambda + 7.1261e-09 *lens_ipow(x, 5) + 0.0496316 *x*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + 1.09726e-06 *lens_ipow(x, 4)*dx*lambda + 3.78813e-05 *lens_ipow(x, 3)*lens_ipow(dx, 2)*lambda + -1.66454e-05 *lens_ipow(x, 3)*lens_ipow(dy, 2)*lambda + -0.0135048 *x*y*lens_ipow(dx, 2)*lens_ipow(dy, 3) + 17.0903 *lens_ipow(dx, 3)*lens_ipow(dy, 4) + -1.76629e-06 *lens_ipow(y, 4)*dx*lens_ipow(dy, 2) + 0.00015555 *lens_ipow(x, 2)*y*lens_ipow(dx, 3)*dy + -0.00160529 *lens_ipow(x, 2)*dx*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + 4.71609e-07 *x*lens_ipow(y, 3)*dy*lens_ipow(lambda, 3) + 2.08345e-11 *lens_ipow(x, 4)*lens_ipow(y, 4)*dx + 3.06727e-13 *lens_ipow(x, 5)*lens_ipow(y, 4) + 2.98578e-14 *lens_ipow(x, 5)*lens_ipow(y, 5)*dy + -3.77699e-14 *lens_ipow(x, 4)*lens_ipow(y, 6)*dx;
      out[3] =  + -0.0221765 *y + -1.19483 *dy + 0.0266626 *x*dx*dy + -0.000118529 *x*y*dx + 1.31992 *lens_ipow(dx, 2)*dy + 6.74637e-06 *lens_ipow(x, 2)*y + 1.16569e-05 *lens_ipow(y, 3) + 0.00019251 *lens_ipow(x, 2)*dy + 0.605434 *lens_ipow(dy, 3) + -0.0134873 *y*lens_ipow(dx, 2)*lambda + -7.33135e-06 *lens_ipow(y, 3)*lambda + 1.41568 *lens_ipow(dx, 2)*lens_ipow(dy, 3) + 0.0329062 *x*dx*lens_ipow(dy, 3) + -1.03815e-05 *lens_ipow(x, 2)*y*lens_ipow(dx, 2) + 1.06567e-08 *lens_ipow(x, 2)*lens_ipow(y, 3) + -3.06289e-05 *lens_ipow(y, 3)*lens_ipow(dx, 2) + 4.87314e-09 *lens_ipow(x, 4)*y + 1.3886e-09 *lens_ipow(x, 2)*lens_ipow(y, 4)*dy + 0.00190446 *lens_ipow(x, 2)*lens_ipow(dy, 5) + 0.0885763 *y*lens_ipow(dx, 4)*lens_ipow(lambda, 3) + 2.64595e-09 *lens_ipow(x, 3)*lens_ipow(y, 3)*dx*lambda + 1.23741e-07 *lens_ipow(y, 5)*lens_ipow(dx, 2)*lambda + -0.0933092 *lens_ipow(y, 2)*lens_ipow(dx, 2)*lens_ipow(dy, 3)*lens_ipow(lambda, 2) + 9.98314e-14 *lens_ipow(x, 4)*lens_ipow(y, 5) + -2.22296 *y*lens_ipow(dx, 2)*lens_ipow(dy, 4)*lens_ipow(lambda, 3) + 2.72843e-08 *lens_ipow(y, 5)*lens_ipow(lambda, 5) + -3.78021e-06 *lens_ipow(y, 5)*lens_ipow(dx, 2)*lens_ipow(dy, 2)*lens_ipow(lambda, 2) + 0.000607305 *lens_ipow(y, 3)*lens_ipow(dy, 8);
      out_transmittance =  + 0.445415  + 0.703675 *lambda + -0.00105712 *x*dx + -1.95963e-05 *lens_ipow(x, 2) + -0.0406648 *lens_ipow(dx, 2) + -0.968706 *lens_ipow(lambda, 2) + 0.4698 *lens_ipow(lambda, 3) + -0.0454226 *y*lens_ipow(dy, 3) + -1.65054 *lens_ipow(dx, 2)*lens_ipow(dy, 2) + -1.23797e-05 *lens_ipow(x, 2)*y*dy + -0.885696 *lens_ipow(dy, 4) + -1.25888e-07 *lens_ipow(x, 2)*lens_ipow(y, 2) + -1.09325e-07 *lens_ipow(y, 4) + -0.0373244 *y*lens_ipow(dx, 2)*dy + -4.93975e-06 *lens_ipow(x, 3)*dx + -0.00131154 *lens_ipow(y, 2)*lens_ipow(dy, 2) + -0.0374354 *x*dx*lens_ipow(dy, 2) + -0.00129105 *x*y*dx*dy + -0.000344897 *lens_ipow(x, 2)*lens_ipow(dy, 2) + -1.8055e-05 *lens_ipow(y, 3)*dy + -2.18263e-05 *x*lens_ipow(y, 2)*dx*lambda + -0.000617129 *lens_ipow(y, 2)*lens_ipow(dx, 2)*lambda + -5.14691 *lens_ipow(dx, 6) + -0.419065 *x*lens_ipow(dx, 5) + -0.0184877 *lens_ipow(x, 2)*lens_ipow(dx, 4) + -3.62468e-06 *lens_ipow(x, 4)*lens_ipow(dx, 2) + -0.000379881 *lens_ipow(x, 3)*lens_ipow(dx, 3) + -2.20816e-12 *lens_ipow(x, 4)*lens_ipow(y, 4)*lens_ipow(lambda, 2);

    } break;

  }

  return MAX(0.0f, out_transmittance);
}

/*
// evaluates from the sensor (in) to the aperture (out) only
// returns the transmittance.
static inline float lens_evaluate_aperture(const float *in, float *out)
{
  const float x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];
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
  float out_x = out[0], out_y = out[1], out_dx = out[2], out_dy = out[3], out_transmittance = 1.0f;
  float x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];

//#include "pt_sample_aperture.h"
  switch (camera_data->lensModel){
    
    case NONE:
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
        const float begin_lambda = lambda;
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



    case petzval_1900_66mm:
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
        const float begin_lambda = lambda;
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


    case doublegauss_100mm:
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
        const float begin_lambda = lambda;
        pred_x =  + 66.2069 *begin_dx + 0.5962 *begin_x + 0.250805 *begin_x*begin_lambda + 17.9457 *begin_dx*begin_lambda + -0.631637 *begin_y*begin_dx*begin_dy + -0.00169732 *begin_x*begin_y*begin_dy + -0.00274299 *lens_ipow(begin_x, 2)*begin_dx + -48.5219 *lens_ipow(begin_dx, 3) + -0.346547 *begin_x*lens_ipow(begin_lambda, 2) + -46.9272 *begin_dx*lens_ipow(begin_dy, 2) + -2.76025e-06 *lens_ipow(begin_x, 3) + -0.29773 *begin_x*lens_ipow(begin_dy, 2) + -0.945888 *begin_x*lens_ipow(begin_dx, 2) + -0.00117191 *lens_ipow(begin_y, 2)*begin_dx + -24.4136 *begin_dx*lens_ipow(begin_lambda, 2) + 11.7019 *begin_dx*lens_ipow(begin_lambda, 3) + 0.167874 *begin_x*lens_ipow(begin_lambda, 3) + -4.80601e-06 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + -0.00107164 *begin_x*begin_y*begin_dy*begin_lambda + -0.00151481 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + -0.0170197 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 3) + 48.409 *lens_ipow(begin_dx, 5) + -0.00013336 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + -0.0107591 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3) + -0.0237075 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + 121.445 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2)*begin_lambda + -0.000262956 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2)*begin_lambda + -0.000233004 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2);
        pred_y =  + 0.623078 *begin_y + 67.8437 *begin_dy + 8.18959 *begin_dy*begin_lambda + 0.0989906 *begin_y*begin_lambda + -0.303511 *begin_y*lens_ipow(begin_dx, 2) + -0.00213596 *lens_ipow(begin_y, 2)*begin_dy + -0.858359 *begin_y*lens_ipow(begin_dy, 2) + -0.00237215 *begin_x*begin_y*begin_dx + -0.067194 *begin_y*lens_ipow(begin_lambda, 2) + -4.29705e-06 *lens_ipow(begin_x, 2)*begin_y + -5.64735 *begin_dy*lens_ipow(begin_lambda, 2) + -45.5101 *lens_ipow(begin_dy, 3) + -201.662 *lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + -0.00104475 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + -2.35583 *begin_x*begin_dx*begin_dy*begin_lambda + 2.1099 *begin_x*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -0.0133921 *lens_ipow(begin_x, 2)*begin_dy*lens_ipow(begin_lambda, 2) + 259.61 *lens_ipow(begin_dx, 2)*begin_dy*lens_ipow(begin_lambda, 2) + -1.13034e-05 *lens_ipow(begin_y, 4)*begin_dy*begin_lambda + 41.9825 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3)*begin_lambda + -94.6589 *lens_ipow(begin_dx, 2)*begin_dy*lens_ipow(begin_lambda, 3) + -0.00229265 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2)*begin_lambda + -2.62607e-08 *lens_ipow(begin_y, 5)*begin_lambda + 0.015556 *lens_ipow(begin_x, 2)*begin_dy*lens_ipow(begin_lambda, 3) + -8.02377 *begin_y*lens_ipow(begin_dy, 4)*begin_lambda + -0.22871 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 3)*begin_lambda + -4.33424 *begin_y*lens_ipow(begin_dy, 4)*lens_ipow(begin_lambda, 2) + -264.677 *lens_ipow(begin_dy, 5)*lens_ipow(begin_lambda, 2);
        pred_dx =  + 0.750736 *begin_dx + -0.007991 *begin_x + -0.325733 *begin_dx*begin_lambda + 0.023804 *begin_y*begin_dx*begin_dy + 0.000322971 *lens_ipow(begin_x, 2)*begin_dx + -0.00144593 *begin_x*lens_ipow(begin_lambda, 2) + 1.78703 *begin_dx*lens_ipow(begin_dy, 2) + 0.000105249 *lens_ipow(begin_y, 2)*begin_dx + 0.245136 *begin_dx*lens_ipow(begin_lambda, 2) + 0.142763 *begin_x*lens_ipow(begin_dx, 2)*begin_lambda + 6.35475 *lens_ipow(begin_dx, 3)*begin_lambda + 0.0458962 *begin_x*lens_ipow(begin_dy, 2)*begin_lambda + 0.00174266 *begin_x*lens_ipow(begin_lambda, 3) + 6.53855e-07 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + 2.5233e-06 *lens_ipow(begin_x, 3)*begin_lambda + 0.000798757 *begin_x*begin_y*begin_dy*begin_lambda + 0.205025 *begin_x*lens_ipow(begin_dx, 4) + -6.30575 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + 10.3936 *lens_ipow(begin_dx, 5) + -3.92589e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_lambda, 2) + -0.155157 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -0.0439539 *begin_x*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + -0.000792449 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 2) + 0.00801955 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 3)*begin_lambda + 0.286882 *begin_x*lens_ipow(begin_dx, 4)*begin_lambda + -0.000538618 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_lambda, 4) + 5.99453e-05 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -2.67236e-06 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 6);
        pred_dy =  + -0.00812867 *begin_y + 0.782426 *begin_dy + -0.552262 *begin_dy*begin_lambda + 0.0117314 *begin_y*lens_ipow(begin_dx, 2) + 0.000456385 *lens_ipow(begin_y, 2)*begin_dy + 0.043139 *begin_y*lens_ipow(begin_dy, 2) + 0.0230275 *begin_x*begin_dx*begin_dy + 0.000199112 *begin_x*begin_y*begin_dx + 1.74763 *lens_ipow(begin_dx, 2)*begin_dy + 2.44318e-07 *lens_ipow(begin_x, 2)*begin_y + 0.758713 *begin_dy*lens_ipow(begin_lambda, 2) + 1.10781e-06 *lens_ipow(begin_y, 3) + 9.80018e-05 *lens_ipow(begin_x, 2)*begin_dy + 1.82849 *lens_ipow(begin_dy, 3) + -0.030375 *begin_y*lens_ipow(begin_dy, 2)*begin_lambda + -0.820966 *lens_ipow(begin_dy, 3)*begin_lambda + -0.000408932 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + -0.363228 *begin_dy*lens_ipow(begin_lambda, 3) + -1.70969e-06 *lens_ipow(begin_y, 3)*begin_lambda + 1.61518e-07 *lens_ipow(begin_y, 4)*begin_dy + 15.3789 *lens_ipow(begin_dy, 5) + 0.00882723 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 3) + -2.19196e-05 *begin_x*begin_y*begin_dx*lens_ipow(begin_lambda, 2) + 0.000255095 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_dy + 0.596808 *begin_y*lens_ipow(begin_dy, 4) + 5.86072e-05 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2) + -0.00717465 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 4) + 0.0044526 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 5);
        float dx1_domega0[2][2];
        dx1_domega0[0][0] =  + 66.2069  + 17.9457 *begin_lambda + -0.631637 *begin_y*begin_dy + -0.00274299 *lens_ipow(begin_x, 2) + -145.566 *lens_ipow(begin_dx, 2) + -46.9272 *lens_ipow(begin_dy, 2) + -1.89178 *begin_x*begin_dx + -0.00117191 *lens_ipow(begin_y, 2) + -24.4136 *lens_ipow(begin_lambda, 2) + 11.7019 *lens_ipow(begin_lambda, 3) + -0.00151481 *lens_ipow(begin_x, 2)*begin_lambda + -0.0510591 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + 242.045 *lens_ipow(begin_dx, 4) + -0.000266721 *begin_x*lens_ipow(begin_y, 2)*begin_dx + -0.0322772 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + -0.0237075 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2)*begin_lambda + 364.336 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*begin_lambda + -0.000525913 *lens_ipow(begin_x, 3)*begin_dx*begin_lambda+0.0f;
        dx1_domega0[0][1] =  + -0.631637 *begin_y*begin_dx + -0.00169732 *begin_x*begin_y + -93.8544 *begin_dx*begin_dy + -0.59546 *begin_x*begin_dy + -0.00107164 *begin_x*begin_y*begin_lambda + -0.047415 *lens_ipow(begin_x, 2)*begin_dx*begin_dy*begin_lambda + 242.891 *lens_ipow(begin_dx, 3)*begin_dy*begin_lambda + -0.000466008 *lens_ipow(begin_x, 3)*begin_dy*lens_ipow(begin_lambda, 2)+0.0f;
        dx1_domega0[1][0] =  + -0.607023 *begin_y*begin_dx + -0.00237215 *begin_x*begin_y + -403.324 *begin_dx*begin_dy*begin_lambda + -2.35583 *begin_x*begin_dy*begin_lambda + 2.1099 *begin_x*begin_dy*lens_ipow(begin_lambda, 2) + 519.221 *begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 83.965 *begin_dx*lens_ipow(begin_dy, 3)*begin_lambda + -189.318 *begin_dx*begin_dy*lens_ipow(begin_lambda, 3)+0.0f;
        dx1_domega0[1][1] =  + 67.8437  + 8.18959 *begin_lambda + -0.00213596 *lens_ipow(begin_y, 2) + -1.71672 *begin_y*begin_dy + -5.64735 *lens_ipow(begin_lambda, 2) + -136.53 *lens_ipow(begin_dy, 2) + -201.662 *lens_ipow(begin_dx, 2)*begin_lambda + -0.00104475 *lens_ipow(begin_y, 2)*begin_lambda + -2.35583 *begin_x*begin_dx*begin_lambda + 2.1099 *begin_x*begin_dx*lens_ipow(begin_lambda, 2) + -0.0133921 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 2) + 259.61 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -1.13034e-05 *lens_ipow(begin_y, 4)*begin_lambda + 125.947 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*begin_lambda + -94.6589 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 3) + -0.0045853 *lens_ipow(begin_y, 3)*begin_dy*begin_lambda + 0.015556 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 3) + -32.0951 *begin_y*lens_ipow(begin_dy, 3)*begin_lambda + -0.686131 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2)*begin_lambda + -17.3369 *begin_y*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + -1323.39 *lens_ipow(begin_dy, 4)*lens_ipow(begin_lambda, 2)+0.0f;
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


    case angenieux_doublegauss_1953_49mm:
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
        const float begin_lambda = lambda;
        pred_x =  + 26.7119 *begin_dx + 1.09538 *begin_x*begin_lambda + 6.60947 *begin_dx*begin_lambda + -0.0547589 *begin_y*begin_dx*begin_dy + 0.00189755 *lens_ipow(begin_x, 2)*begin_dx + -16.5359 *lens_ipow(begin_dx, 3) + -1.63151 *begin_x*lens_ipow(begin_lambda, 2) + -14.2808 *begin_dx*lens_ipow(begin_dy, 2) + -0.000478074 *begin_x*lens_ipow(begin_y, 2) + -0.000412757 *lens_ipow(begin_x, 3) + -0.184661 *begin_x*lens_ipow(begin_dy, 2) + -4.54335 *begin_dx*lens_ipow(begin_lambda, 2) + -0.356237 *begin_x*lens_ipow(begin_dx, 2)*begin_lambda + 0.838616 *begin_x*lens_ipow(begin_lambda, 3) + 4.95014e-06 *lens_ipow(begin_y, 4)*begin_dx + 7.55289 *lens_ipow(begin_dx, 5) + 0.310712 *begin_x*lens_ipow(begin_dy, 4) + -0.0272238 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 2) + -0.000405637 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2) + -0.0016729 *lens_ipow(begin_x, 2)*begin_y*begin_dx*begin_dy*begin_lambda + 1.38156e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2) + -5.59676e-09 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 2) + -2.08299e-09 *lens_ipow(begin_x, 7) + 1.51037e-09 *lens_ipow(begin_x, 7)*begin_lambda + 1.62764 *begin_x*lens_ipow(begin_dx, 4)*lens_ipow(begin_lambda, 3) + -2.43877e-11 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6) + -0.000166531 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 5) + 3.91242e-09 *begin_x*lens_ipow(begin_y, 7)*lens_ipow(begin_dx, 2)*begin_dy;
        pred_y =  + 26.6478 *begin_dy + 7.07798 *begin_dy*begin_lambda + 1.14323 *begin_y*begin_lambda + -0.180053 *begin_y*lens_ipow(begin_dx, 2) + -0.142826 *begin_y*lens_ipow(begin_dy, 2) + -0.0529828 *begin_x*begin_dx*begin_dy + -15.8269 *lens_ipow(begin_dx, 2)*begin_dy + -1.77677 *begin_y*lens_ipow(begin_lambda, 2) + -0.000519123 *lens_ipow(begin_x, 2)*begin_y + -4.90498 *begin_dy*lens_ipow(begin_lambda, 2) + -0.000503188 *lens_ipow(begin_y, 3) + 0.00136072 *lens_ipow(begin_x, 2)*begin_dy + -16.844 *lens_ipow(begin_dy, 3) + 0.931493 *begin_y*lens_ipow(begin_lambda, 3) + 0.000190732 *lens_ipow(begin_y, 3)*begin_lambda + 0.0001998 *lens_ipow(begin_x, 2)*begin_y*begin_lambda + -0.000822313 *begin_x*lens_ipow(begin_y, 2)*begin_dx*begin_dy + 0.318617 *begin_y*lens_ipow(begin_dx, 4) + 6.93717 *lens_ipow(begin_dy, 5) + -3.41864e-07 *lens_ipow(begin_x, 4)*begin_y + -0.00699567 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 3) + -0.000951 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2) + 0.000114581 *lens_ipow(begin_x, 3)*begin_y*begin_dx*lens_ipow(begin_dy, 2) + -1.3737e-09 *lens_ipow(begin_y, 7) + 88.5367 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 3) + -4.94822e-09 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 5) + -1.54899e-11 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3) + -0.00168031 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3);
        pred_dx =  + 0.399786 *begin_dx + -0.0374335 *begin_x + 0.0213387 *begin_x*begin_lambda + -0.0222137 *begin_y*begin_dx*begin_dy + 0.00011936 *begin_x*begin_y*begin_dy + -0.491997 *lens_ipow(begin_dx, 3) + -0.0165778 *begin_x*lens_ipow(begin_lambda, 2) + -0.483482 *begin_dx*lens_ipow(begin_dy, 2) + -2.52176e-05 *begin_x*lens_ipow(begin_y, 2) + -2.76551e-05 *lens_ipow(begin_x, 3) + -0.0329376 *begin_x*lens_ipow(begin_dx, 2) + -0.0367118 *begin_x*lens_ipow(begin_dy, 2)*begin_lambda + 2.08498e-06 *lens_ipow(begin_x, 4)*begin_dx + -4.32665e-08 *begin_x*lens_ipow(begin_y, 4) + 2.38937e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dx + 1.53062e-06 *lens_ipow(begin_x, 3)*begin_y*begin_dy + 1.87765e-06 *begin_x*lens_ipow(begin_y, 3)*begin_dy + 0.0326943 *begin_x*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 5.17241e-08 *begin_x*lens_ipow(begin_y, 4)*begin_lambda + 8.80235e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*begin_lambda + -5.13369e-10 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 2) + -9.63804e-08 *lens_ipow(begin_x, 5)*lens_ipow(begin_dy, 2) + 4.36787e-08 *lens_ipow(begin_x, 5)*lens_ipow(begin_lambda, 2) + 4.61028e-12 *lens_ipow(begin_y, 8)*begin_dx + -2.84439e-13 *lens_ipow(begin_x, 9) + -1.33303e-12 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6) + -3.14982e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2) + 2.94026e-08 *lens_ipow(begin_x, 6)*lens_ipow(begin_dx, 3);
        pred_dy =  + -0.0358994 *begin_y + 0.396945 *begin_dy + 0.0169134 *begin_y*begin_lambda + -0.0119194 *begin_y*lens_ipow(begin_dx, 2) + 0.000274491 *lens_ipow(begin_y, 2)*begin_dy + -0.030044 *begin_y*lens_ipow(begin_dy, 2) + -0.0217624 *begin_x*begin_dx*begin_dy + 0.000303076 *begin_x*begin_y*begin_dx + -0.491456 *lens_ipow(begin_dx, 2)*begin_dy + -0.0139602 *begin_y*lens_ipow(begin_lambda, 2) + -3.47907e-05 *lens_ipow(begin_x, 2)*begin_y + -3.48187e-05 *lens_ipow(begin_y, 3) + -0.4821 *lens_ipow(begin_dy, 3) + 1.8503e-05 *lens_ipow(begin_y, 3)*begin_lambda + 1.95796e-05 *lens_ipow(begin_x, 2)*begin_y*begin_lambda + 0.0103834 *begin_y*lens_ipow(begin_dx, 4) + -4.48971e-05 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dy, 2) + 2.71146e-09 *lens_ipow(begin_y, 6)*begin_dy + -9.43117e-08 *lens_ipow(begin_y, 5)*lens_ipow(begin_dx, 2) + 4.17668e-06 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 2)*begin_dy + -2.58285e-07 *lens_ipow(begin_y, 5)*lens_ipow(begin_dy, 2) + -1.19747e-10 *lens_ipow(begin_x, 6)*begin_y + 2.96507e-09 *begin_x*lens_ipow(begin_y, 5)*begin_dx + -3.63437e-10 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 5) + -2.40231e-13 *lens_ipow(begin_y, 9) + 1.48883e-10 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4)*begin_dy + 1.09134e-10 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 3)*begin_dx + 3.74579e-12 *lens_ipow(begin_x, 8)*begin_dy;
        float dx1_domega0[2][2];
        dx1_domega0[0][0] =  + 26.7119  + 6.60947 *begin_lambda + -0.0547589 *begin_y*begin_dy + 0.00189755 *lens_ipow(begin_x, 2) + -49.6076 *lens_ipow(begin_dx, 2) + -14.2808 *lens_ipow(begin_dy, 2) + -4.54335 *lens_ipow(begin_lambda, 2) + -0.712474 *begin_x*begin_dx*begin_lambda + 4.95014e-06 *lens_ipow(begin_y, 4) + 37.7644 *lens_ipow(begin_dx, 4) + -0.0272238 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + -0.000811273 *lens_ipow(begin_x, 3)*begin_dx + -0.0016729 *lens_ipow(begin_x, 2)*begin_y*begin_dy*begin_lambda + 6.51054 *begin_x*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 3) + -0.000832657 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 4) + 7.82484e-09 *begin_x*lens_ipow(begin_y, 7)*begin_dx*begin_dy+0.0f;
        dx1_domega0[0][1] =  + -0.0547589 *begin_y*begin_dx + -28.5616 *begin_dx*begin_dy + -0.369321 *begin_x*begin_dy + 1.24285 *begin_x*lens_ipow(begin_dy, 3) + -0.0544476 *lens_ipow(begin_x, 2)*begin_dx*begin_dy + -0.0016729 *lens_ipow(begin_x, 2)*begin_y*begin_dx*begin_lambda + 3.91242e-09 *begin_x*lens_ipow(begin_y, 7)*lens_ipow(begin_dx, 2)+0.0f;
        dx1_domega0[1][0] =  + -0.360107 *begin_y*begin_dx + -0.0529828 *begin_x*begin_dy + -31.6538 *begin_dx*begin_dy + -0.000822313 *begin_x*lens_ipow(begin_y, 2)*begin_dy + 1.27447 *begin_y*lens_ipow(begin_dx, 3) + 0.000114581 *lens_ipow(begin_x, 3)*begin_y*lens_ipow(begin_dy, 2) + 354.147 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3) + -0.00336062 *lens_ipow(begin_y, 4)*begin_dx*lens_ipow(begin_dy, 3)+0.0f;
        dx1_domega0[1][1] =  + 26.6478  + 7.07798 *begin_lambda + -0.285652 *begin_y*begin_dy + -0.0529828 *begin_x*begin_dx + -15.8269 *lens_ipow(begin_dx, 2) + -4.90498 *lens_ipow(begin_lambda, 2) + 0.00136072 *lens_ipow(begin_x, 2) + -50.532 *lens_ipow(begin_dy, 2) + -0.000822313 *begin_x*lens_ipow(begin_y, 2)*begin_dx + 34.6859 *lens_ipow(begin_dy, 4) + -0.020987 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + -0.001902 *lens_ipow(begin_y, 3)*begin_dy + 0.000229162 *lens_ipow(begin_x, 3)*begin_y*begin_dx*begin_dy + 265.61 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2) + -0.00504093 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)+0.0f;
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


    case fisheye_aspherical:
    {
      float pred_x;
      float pred_y;
      float pred_dx;
      float pred_dy;
      float sqr_err = FLT_MAX;
      for(int k=0; k<5 && sqr_err > 1e-4f; k++)
      {
        const float begin_x = x + dist * dx;
        const float begin_y = y + dist * dy;
        const float begin_dx = dx;
        const float begin_dy = dy;
        const float begin_lambda = lambda;
        pred_x =  + 24.0758 *begin_dx + 0.714404 *begin_x + 1.13639 *begin_y*begin_dx*begin_dy + 0.032411 *begin_x*begin_y*begin_dy + 0.0496792 *lens_ipow(begin_x, 2)*begin_dx + 9.19327 *lens_ipow(begin_dx, 3) + 10.149 *begin_dx*lens_ipow(begin_dy, 2) + 0.464028 *begin_x*lens_ipow(begin_dy, 2) + 1.51791 *begin_x*lens_ipow(begin_dx, 2) + 0.0192878 *lens_ipow(begin_y, 2)*begin_dx + -1.14405e-06 *lens_ipow(begin_x, 5) + -0.000677498 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 3) + -5.64672e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4) + -2.12508e-07 *lens_ipow(begin_x, 5)*begin_y*begin_dy + -6.02066e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4)*begin_dx + -0.0098213 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 4) + -1.90383e-08 *lens_ipow(begin_y, 7)*begin_dx*begin_dy + -6.69381e-10 *lens_ipow(begin_y, 8)*begin_dx + -7.78368e-08 *lens_ipow(begin_x, 7)*lens_ipow(begin_dx, 2) + -7.13255e-11 *begin_x*lens_ipow(begin_y, 8) + -2.5276e-10 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 2) + 1.63176e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 2) + -4.59009e-09 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4)*begin_dx*begin_lambda + 2.41334e-08 *lens_ipow(begin_x, 7)*lens_ipow(begin_dy, 2)*begin_lambda + -2.13711e-13 *lens_ipow(begin_x, 11) + -2.81141e-11 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 2)*begin_dx + -1.07931e-11 *lens_ipow(begin_x, 10)*begin_dx + -7.39003e-12 *begin_x*lens_ipow(begin_y, 9)*begin_dy;
        pred_y =  + 0.720082 *begin_y + 24.0428 *begin_dy + -0.014603 *begin_y*begin_lambda + 0.488201 *begin_y*lens_ipow(begin_dx, 2) + 0.0532851 *lens_ipow(begin_y, 2)*begin_dy + 1.6659 *begin_y*lens_ipow(begin_dy, 2) + 1.1315 *begin_x*begin_dx*begin_dy + 0.033149 *begin_x*begin_y*begin_dx + 10.3268 *lens_ipow(begin_dx, 2)*begin_dy + 0.0189151 *lens_ipow(begin_x, 2)*begin_dy + 10.648 *lens_ipow(begin_dy, 3) + 6.86709e-07 *lens_ipow(begin_y, 5)*begin_lambda + -1.18904e-08 *lens_ipow(begin_y, 7) + -1.59389e-07 *begin_x*lens_ipow(begin_y, 5)*begin_dx + -6.6993e-08 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3) + -7.33692e-07 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 2)*begin_dy + 2.01469e-08 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3)*begin_lambda + -5.45356e-11 *lens_ipow(begin_x, 8)*begin_y + -1.45339e-09 *lens_ipow(begin_y, 8)*begin_dy + 1.39319e-07 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + -2.51749e-10 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 7) + 1.70727e-10 *lens_ipow(begin_x, 8)*begin_y*lens_ipow(begin_dx, 2) + -2.91946e-11 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 8)*begin_dy + -7.61611e-14 *lens_ipow(begin_y, 11) + 5.90072e-11 *lens_ipow(begin_y, 9)*lens_ipow(begin_dx, 2) + -1.60411e-12 *lens_ipow(begin_x, 10)*begin_dy + 8.55147e-10 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_dx*begin_dy + -0.000201456 *lens_ipow(begin_x, 5)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3);
        pred_dx =  + 1.07844 *begin_dx + -0.00976452 *begin_x + -0.00679202 *begin_x*begin_lambda + -0.243874 *begin_dx*begin_lambda + 0.00443152 *begin_x*begin_y*begin_dy + 0.00637913 *lens_ipow(begin_x, 2)*begin_dx + 6.34639e-05 *begin_x*lens_ipow(begin_y, 2) + 6.36474e-05 *lens_ipow(begin_x, 3) + 0.0312271 *begin_x*lens_ipow(begin_dy, 2) + 0.0963793 *begin_x*lens_ipow(begin_dx, 2) + 0.00192144 *lens_ipow(begin_y, 2)*begin_dx + 0.433169 *lens_ipow(begin_dx, 3)*begin_lambda + 0.197062 *begin_y*begin_dx*begin_dy*begin_lambda + -0.0065923 *begin_x*begin_y*lens_ipow(begin_dx, 2)*begin_dy + -0.168223 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -0.000346556 *lens_ipow(begin_x, 2)*begin_y*begin_dx*begin_dy + 0.000109518 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2)*begin_lambda + 6.31452e-08 *lens_ipow(begin_x, 6)*begin_dx + -2.03789e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4) + 6.2172e-07 *lens_ipow(begin_x, 5)*lens_ipow(begin_dy, 2) + -3.67141 *lens_ipow(begin_dx, 7)*begin_lambda + -0.000108841 *lens_ipow(begin_x, 3)*begin_y*lens_ipow(begin_dy, 5) + -4.27484e-12 *begin_x*lens_ipow(begin_y, 8) + -1.9923e-11 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 2) + -2.43505e-10 *lens_ipow(begin_x, 8)*begin_dx + -4.27357e-10 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 2)*begin_dx*begin_lambda + -1.66259e-14 *lens_ipow(begin_x, 11) + -1.34222e-13 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 6);
        pred_dy =  + -0.00698832 *begin_y + 1.25591 *begin_dy + -0.955525 *begin_dy*begin_lambda + -0.0153323 *begin_y*begin_lambda + 0.0418371 *begin_y*lens_ipow(begin_dx, 2) + 0.00762595 *lens_ipow(begin_y, 2)*begin_dy + 0.128823 *begin_y*lens_ipow(begin_dy, 2) + 0.0743635 *begin_x*begin_dx*begin_dy + 0.00493618 *begin_x*begin_y*begin_dx + 0.346896 *lens_ipow(begin_dx, 2)*begin_dy + 6.95118e-05 *lens_ipow(begin_x, 2)*begin_y + 0.653258 *begin_dy*lens_ipow(begin_lambda, 2) + 7.30205e-05 *lens_ipow(begin_y, 3) + 0.00218526 *lens_ipow(begin_x, 2)*begin_dy + 0.452226 *lens_ipow(begin_dy, 3) + 0.00918914 *begin_y*lens_ipow(begin_lambda, 3) + -7.18563e-10 *lens_ipow(begin_x, 6)*begin_y + 1.53205e-08 *begin_x*lens_ipow(begin_y, 5)*begin_dx + 2.85692e-09 *lens_ipow(begin_y, 7)*lens_ipow(begin_dx, 2) + -4.07508e-12 *lens_ipow(begin_y, 9) + -3.29749e-11 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 5) + -1.84743e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*begin_dx*begin_dy + -1.29876e-11 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 7) + -9.14928e-10 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4)*begin_dy*begin_lambda + -6.40826e-11 *lens_ipow(begin_y, 8)*begin_dy*lens_ipow(begin_lambda, 2) + -7.04503e-14 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 3) + -3.13147e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 5)*lens_ipow(begin_dx, 3) + -205.259 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 9);
        float dx1_domega0[2][2];
        dx1_domega0[0][0] =  + 24.0758  + 1.13639 *begin_y*begin_dy + 0.0496792 *lens_ipow(begin_x, 2) + 27.5798 *lens_ipow(begin_dx, 2) + 10.149 *lens_ipow(begin_dy, 2) + 3.03582 *begin_x*begin_dx + 0.0192878 *lens_ipow(begin_y, 2) + -0.00203249 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 2) + -6.02066e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4) + -0.0392852 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 3) + -1.90383e-08 *lens_ipow(begin_y, 7)*begin_dy + -6.69381e-10 *lens_ipow(begin_y, 8) + -1.55674e-07 *lens_ipow(begin_x, 7)*begin_dx + -4.59009e-09 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4)*begin_lambda + -2.81141e-11 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 2) + -1.07931e-11 *lens_ipow(begin_x, 10)+0.0f;
        dx1_domega0[0][1] =  + 1.13639 *begin_y*begin_dx + 0.032411 *begin_x*begin_y + 20.2981 *begin_dx*begin_dy + 0.928057 *begin_x*begin_dy + -2.12508e-07 *lens_ipow(begin_x, 5)*begin_y + -1.90383e-08 *lens_ipow(begin_y, 7)*begin_dx + 3.26352e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*begin_dy + 4.82668e-08 *lens_ipow(begin_x, 7)*begin_dy*begin_lambda + -7.39003e-12 *begin_x*lens_ipow(begin_y, 9)+0.0f;
        dx1_domega0[1][0] =  + 0.976401 *begin_y*begin_dx + 1.1315 *begin_x*begin_dy + 0.033149 *begin_x*begin_y + 20.6537 *begin_dx*begin_dy + -1.59389e-07 *begin_x*lens_ipow(begin_y, 5) + 2.78637e-07 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3)*begin_dx + 3.41454e-10 *lens_ipow(begin_x, 8)*begin_y*begin_dx + 1.18014e-10 *lens_ipow(begin_y, 9)*begin_dx + 8.55147e-10 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_dy + -0.000604368 *lens_ipow(begin_x, 5)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3)+0.0f;
        dx1_domega0[1][1] =  + 24.0428  + 0.0532851 *lens_ipow(begin_y, 2) + 3.3318 *begin_y*begin_dy + 1.1315 *begin_x*begin_dx + 10.3268 *lens_ipow(begin_dx, 2) + 0.0189151 *lens_ipow(begin_x, 2) + 31.944 *lens_ipow(begin_dy, 2) + -7.33692e-07 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 2) + -1.45339e-09 *lens_ipow(begin_y, 8) + -2.91946e-11 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 8) + -1.60411e-12 *lens_ipow(begin_x, 10) + 8.55147e-10 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_dx + -0.000604368 *lens_ipow(begin_x, 5)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2)+0.0f;
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


    case fisheye:
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
        const float begin_lambda = lambda;
        pred_x =  + 46.3231 *begin_dx + 0.713762 *begin_x + -0.203224 *begin_x*begin_lambda + -8.49003 *begin_dx*begin_lambda + 1.48432 *begin_y*begin_dx*begin_dy + 0.136162 *begin_x*lens_ipow(begin_lambda, 2) + 19.351 *begin_dx*lens_ipow(begin_dy, 2) + 0.000222426 *lens_ipow(begin_x, 3) + 0.0178375 *lens_ipow(begin_y, 2)*begin_dx + 6.06598 *begin_dx*lens_ipow(begin_lambda, 2) + 6.93669 *begin_x*lens_ipow(begin_dx, 2)*begin_lambda + 78.5921 *lens_ipow(begin_dx, 3)*begin_lambda + 0.657095 *begin_x*lens_ipow(begin_dy, 2)*begin_lambda + 0.00080037 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + 0.0599381 *begin_x*begin_y*begin_dy*begin_lambda + 0.152641 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + -78.7821 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + -0.000697152 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2) + -6.68444 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -0.14429 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_lambda, 2) + -0.0622761 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 3) + -1.17139 *begin_x*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 5) + 241.074 *begin_y*lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 3) + 438829 *lens_ipow(begin_dx, 9)*lens_ipow(begin_dy, 2) + 41684.5 *begin_x*lens_ipow(begin_dx, 8)*lens_ipow(begin_dy, 2) + 14.6511 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 6)*lens_ipow(begin_dy, 2) + 57564.4 *lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 6) + 1344.32 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 7)*lens_ipow(begin_dy, 2);
        pred_y =  + 0.711668 *begin_y + 46.3848 *begin_dy + -8.46928 *begin_dy*begin_lambda + -0.193319 *begin_y*begin_lambda + 0.286513 *begin_y*lens_ipow(begin_dx, 2) + 1.47114 *begin_x*begin_dx*begin_dy + 0.0219261 *begin_x*begin_y*begin_dx + 18.7219 *lens_ipow(begin_dx, 2)*begin_dy + 0.128495 *begin_y*lens_ipow(begin_lambda, 2) + 0.000225772 *lens_ipow(begin_x, 2)*begin_y + 6.05942 *begin_dy*lens_ipow(begin_lambda, 2) + 0.000205517 *lens_ipow(begin_y, 3) + 0.0176433 *lens_ipow(begin_x, 2)*begin_dy + 6.17418 *begin_y*lens_ipow(begin_dy, 2)*begin_lambda + 63.0907 *lens_ipow(begin_dy, 3)*begin_lambda + 0.140338 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + -0.132736 *lens_ipow(begin_y, 2)*begin_dy*lens_ipow(begin_lambda, 2) + -64.8887 *lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + -5.97842 *begin_y*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 11183.1 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 5) + 396.191 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 7) + 42860.5 *lens_ipow(begin_dy, 9) + 0.0130309 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + 6467.85 *begin_y*lens_ipow(begin_dy, 8) + 0.189225 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 5) + 0.00117308 *lens_ipow(begin_y, 5)*lens_ipow(begin_dy, 4) + 12.2293 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 6) + -24.246 *begin_x*begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 4);
        pred_dx =  + 0.193426 *begin_dx + -0.0194105 *begin_x + -0.00757095 *begin_x*begin_lambda + -0.519864 *begin_dx*begin_lambda + 0.00295916 *lens_ipow(begin_x, 2)*begin_dx + 1.75002 *lens_ipow(begin_dx, 3) + 1.82003 *begin_dx*lens_ipow(begin_dy, 2) + 1.55421e-05 *lens_ipow(begin_x, 3) + 0.0471877 *begin_x*lens_ipow(begin_dy, 2) + 0.139162 *begin_x*lens_ipow(begin_dx, 2) + 0.000387383 *lens_ipow(begin_y, 2)*begin_dx + 0.349752 *begin_dx*lens_ipow(begin_lambda, 2) + 0.00486373 *begin_x*lens_ipow(begin_lambda, 3) + 3.33722e-05 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + 0.00794281 *begin_x*begin_y*begin_dy*begin_lambda + 0.352251 *begin_y*begin_dx*begin_dy*begin_lambda + -0.321666 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 0.00251438 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_lambda, 2) + -0.00721483 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 2) + -0.00548668 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_lambda, 5) + 51.0763 *begin_y*lens_ipow(begin_dx, 7)*begin_dy*begin_lambda + 0.868666 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 7)*begin_lambda + -0.000137796 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 7) + 0.0285012 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 6)*begin_lambda + -0.0541286 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dx, 5)*begin_dy*begin_lambda + 24290.7 *lens_ipow(begin_dx, 9)*lens_ipow(begin_dy, 2) + 1444.58 *begin_x*lens_ipow(begin_dx, 8)*lens_ipow(begin_dy, 2) + 21.2078 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 7)*lens_ipow(begin_dy, 2);
        pred_dy =  + -0.0168014 *begin_y + 0.265126 *begin_dy + -0.864899 *begin_dy*begin_lambda + -0.0204047 *begin_y*begin_lambda + 0.024334 *begin_y*lens_ipow(begin_dx, 2) + 0.016939 *begin_y*lens_ipow(begin_lambda, 2) + 1.80474e-05 *lens_ipow(begin_x, 2)*begin_y + 0.717785 *begin_dy*lens_ipow(begin_lambda, 2) + 0.000955023 *lens_ipow(begin_x, 2)*begin_dy + 0.677667 *begin_y*lens_ipow(begin_dy, 2)*begin_lambda + 0.0883553 *begin_y*lens_ipow(begin_dx, 2)*begin_lambda + 8.99256 *lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + 8.75028 *lens_ipow(begin_dy, 3)*begin_lambda + 0.0138233 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + 5.51942e-05 *lens_ipow(begin_y, 3)*begin_lambda + 0.0100797 *begin_x*begin_y*begin_dx*begin_lambda + 0.450628 *begin_x*begin_dx*begin_dy*begin_lambda + -0.486312 *begin_x*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -0.0146537 *lens_ipow(begin_y, 2)*begin_dy*lens_ipow(begin_lambda, 2) + -0.0108135 *begin_x*begin_y*begin_dx*lens_ipow(begin_lambda, 2) + -9.43872 *lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + -9.85376 *lens_ipow(begin_dx, 2)*begin_dy*lens_ipow(begin_lambda, 2) + -0.759634 *begin_y*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + -0.137746 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 3) + -7.90994e-05 *lens_ipow(begin_y, 3)*lens_ipow(begin_lambda, 3) + 0.0770499 *begin_y*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 4) + -0.000133563 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_lambda, 8) + -0.00617896 *lens_ipow(begin_x, 2)*begin_dy*lens_ipow(begin_lambda, 8);
        float dx1_domega0[2][2];
        dx1_domega0[0][0] =  + 46.3231  + -8.49003 *begin_lambda + 1.48432 *begin_y*begin_dy + 19.351 *lens_ipow(begin_dy, 2) + 0.0178375 *lens_ipow(begin_y, 2) + 6.06598 *lens_ipow(begin_lambda, 2) + 13.8734 *begin_x*begin_dx*begin_lambda + 235.776 *lens_ipow(begin_dx, 2)*begin_lambda + 0.152641 *lens_ipow(begin_x, 2)*begin_lambda + -236.346 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -13.3689 *begin_x*begin_dx*lens_ipow(begin_lambda, 2) + -0.14429 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 2) + 1205.37 *begin_y*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 3) + 3.94946e+06 *lens_ipow(begin_dx, 8)*lens_ipow(begin_dy, 2) + 333476 *begin_x*lens_ipow(begin_dx, 7)*lens_ipow(begin_dy, 2) + 87.9067 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 2) + 287822 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 6) + 9410.21 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 6)*lens_ipow(begin_dy, 2)+0.0f;
        dx1_domega0[0][1] =  + 1.48432 *begin_y*begin_dx + 38.7021 *begin_dx*begin_dy + 1.31419 *begin_x*begin_dy*begin_lambda + 0.0599381 *begin_x*begin_y*begin_lambda + -0.0622761 *begin_x*begin_y*lens_ipow(begin_lambda, 3) + -2.34278 *begin_x*begin_dy*lens_ipow(begin_lambda, 5) + 723.221 *begin_y*lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 2) + 877658 *lens_ipow(begin_dx, 9)*begin_dy + 83369 *begin_x*lens_ipow(begin_dx, 8)*begin_dy + 29.3022 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 6)*begin_dy + 345386 *lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 5) + 2688.63 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 7)*begin_dy+0.0f;
        dx1_domega0[1][0] =  + 0.573025 *begin_y*begin_dx + 1.47114 *begin_x*begin_dy + 0.0219261 *begin_x*begin_y + 37.4438 *begin_dx*begin_dy + 44732.5 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 5) + 0.0260617 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 3) + -72.738 *begin_x*begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 4)+0.0f;
        dx1_domega0[1][1] =  + 46.3848  + -8.46928 *begin_lambda + 1.47114 *begin_x*begin_dx + 18.7219 *lens_ipow(begin_dx, 2) + 6.05942 *lens_ipow(begin_lambda, 2) + 0.0176433 *lens_ipow(begin_x, 2) + 12.3484 *begin_y*begin_dy*begin_lambda + 189.272 *lens_ipow(begin_dy, 2)*begin_lambda + 0.140338 *lens_ipow(begin_y, 2)*begin_lambda + -0.132736 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2) + -194.666 *lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + -11.9568 *begin_y*begin_dy*lens_ipow(begin_lambda, 2) + 55915.7 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 4) + 2773.33 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 6) + 385744 *lens_ipow(begin_dy, 8) + 0.0390926 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 51742.8 *begin_y*lens_ipow(begin_dy, 7) + 0.946123 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 4) + 0.00469234 *lens_ipow(begin_y, 5)*lens_ipow(begin_dy, 3) + 73.3758 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 5) + -96.9839 *begin_x*begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3)+0.0f;
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


    case wideangle:
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
        const float begin_lambda = lambda;
        pred_x =  + 33.436 *begin_dx + 0.620268 *begin_x + -0.0997683 *begin_x*begin_lambda + -1.02423 *begin_dx*begin_lambda + 0.0779104 *begin_x*begin_y*begin_dy + 0.11032 *lens_ipow(begin_x, 2)*begin_dx + 22.3258 *lens_ipow(begin_dx, 3) + 0.066893 *begin_x*lens_ipow(begin_lambda, 2) + 28.7753 *begin_dx*lens_ipow(begin_dy, 2) + 0.00114721 *begin_x*lens_ipow(begin_y, 2) + 0.00104281 *lens_ipow(begin_x, 3) + 1.05782 *begin_x*lens_ipow(begin_dy, 2) + 3.30806 *begin_x*lens_ipow(begin_dx, 2) + 0.622831 *begin_dx*lens_ipow(begin_lambda, 3) + 0.252479 *lens_ipow(begin_y, 2)*begin_dx*begin_lambda + 15.3869 *begin_y*begin_dx*begin_dy*begin_lambda + 0.000265991 *lens_ipow(begin_x, 4)*begin_dx + 17.5869 *begin_x*lens_ipow(begin_dx, 4) + 0.849033 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 3) + -28.4185 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 147.745 *lens_ipow(begin_dx, 5) + 1.40136e-06 *lens_ipow(begin_x, 5) + -0.469428 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_lambda, 2) + 0.020917 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2) + 17.1989 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 3) + 0.285723 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_lambda, 3) + 4.60929e-13 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 4) + -3.19292e-09 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dx*begin_dy;
        pred_y =  + 0.620382 *begin_y + 33.2697 *begin_dy + -0.0988428 *begin_y*begin_lambda + 1.03903 *begin_y*lens_ipow(begin_dx, 2) + 0.108851 *lens_ipow(begin_y, 2)*begin_dy + 3.26034 *begin_y*lens_ipow(begin_dy, 2) + 2.71107 *begin_x*begin_dx*begin_dy + 0.0773918 *begin_x*begin_y*begin_dx + 28.2928 *lens_ipow(begin_dx, 2)*begin_dy + 0.0661776 *begin_y*lens_ipow(begin_lambda, 2) + 0.00113968 *lens_ipow(begin_x, 2)*begin_y + -1.94549 *begin_dy*lens_ipow(begin_lambda, 2) + 0.00102739 *lens_ipow(begin_y, 3) + 0.0442643 *lens_ipow(begin_x, 2)*begin_dy + 21.7286 *lens_ipow(begin_dy, 3) + 1.83525 *begin_dy*lens_ipow(begin_lambda, 3) + 0.000304092 *lens_ipow(begin_y, 4)*begin_dy + 156.877 *lens_ipow(begin_dy, 5) + 0.920891 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 3) + 18.733 *begin_y*lens_ipow(begin_dy, 4) + 1.5905e-06 *lens_ipow(begin_y, 5) + 0.0233247 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2) + 9.47641e-06 *lens_ipow(begin_y, 5)*lens_ipow(begin_dx, 2) + 0.000414172 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*begin_dy + 7.77217e-09 *begin_x*lens_ipow(begin_y, 7)*begin_dx*begin_lambda + 3.2448e-13 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 9) + 2.5744e-13 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 3) + 6.2086e-07 *begin_x*lens_ipow(begin_y, 6)*begin_dx*begin_dy*lens_ipow(begin_lambda, 2);
        pred_dx =  + -0.028575 *begin_x + -0.00932014 *begin_x*begin_lambda + -0.0549223 *begin_y*begin_dx*begin_dy + 0.00203034 *lens_ipow(begin_x, 2)*begin_dx + -3.2338 *lens_ipow(begin_dx, 3) + 0.00683675 *begin_x*lens_ipow(begin_lambda, 2) + 3.22939e-05 *begin_x*lens_ipow(begin_y, 2) + 3.53345e-05 *lens_ipow(begin_x, 3) + -0.0206119 *begin_x*lens_ipow(begin_dy, 2) + -0.0612014 *begin_x*lens_ipow(begin_dx, 2) + 0.000172077 *lens_ipow(begin_y, 2)*begin_dx + -0.51415 *begin_dx*lens_ipow(begin_lambda, 2) + -0.0284093 *begin_x*lens_ipow(begin_dx, 2)*begin_lambda + 0.50677 *begin_dx*lens_ipow(begin_lambda, 3) + -7.7071e-06 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + -1.47665e-05 *lens_ipow(begin_x, 3)*begin_lambda + 0.00842516 *begin_x*begin_y*begin_dy*begin_lambda + -0.00120913 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + -17.3322 *begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + 25.3256 *begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + -0.0171832 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 2) + 0.0109504 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 3) + -14.6692 *begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 4) + 1.3924e-14 *lens_ipow(begin_x, 11) + 8.00155e-12 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 3)*begin_dy + 2.8413e-13 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 4) + 1.22356e-12 *lens_ipow(begin_x, 10)*begin_dx + 1.62527e-11 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 4)*begin_dx;
        pred_dy =  + -0.028835 *begin_y + 0.0581261 *begin_dy + -0.382108 *begin_dy*begin_lambda + -0.00834163 *begin_y*begin_lambda + -0.0206117 *begin_y*lens_ipow(begin_dx, 2) + 0.00200626 *lens_ipow(begin_y, 2)*begin_dy + -0.0624634 *begin_y*lens_ipow(begin_dy, 2) + -0.0548905 *begin_x*begin_dx*begin_dy + 0.00124677 *begin_x*begin_y*begin_dx + -3.2151 *lens_ipow(begin_dx, 2)*begin_dy + 0.00594847 *begin_y*lens_ipow(begin_lambda, 2) + 2.77024e-05 *lens_ipow(begin_x, 2)*begin_y + 0.266371 *begin_dy*lens_ipow(begin_lambda, 2) + 3.50767e-05 *lens_ipow(begin_y, 3) + 0.000195321 *lens_ipow(begin_x, 2)*begin_dy + -3.20615 *lens_ipow(begin_dy, 3) + -0.0275655 *begin_y*lens_ipow(begin_dy, 2)*begin_lambda + -0.0012689 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + -1.4775e-05 *lens_ipow(begin_y, 3)*begin_lambda + -0.00384855 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + 4.9258e-10 *lens_ipow(begin_y, 8)*begin_dy + 3.13678e-12 *lens_ipow(begin_y, 9) + 1.88125e-09 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4)*begin_dy*begin_lambda + 1.45253e-10 *lens_ipow(begin_y, 9)*lens_ipow(begin_dy, 2) + 1.06796e-13 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 3) + 6.69022e-12 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 7)*begin_dx + 6.62722e-12 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 3)*begin_dx + 2.23917e-13 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 7);
        float dx1_domega0[2][2];
        dx1_domega0[0][0] =  + 33.436  + -1.02423 *begin_lambda + 0.11032 *lens_ipow(begin_x, 2) + 66.9774 *lens_ipow(begin_dx, 2) + 28.7753 *lens_ipow(begin_dy, 2) + 6.61613 *begin_x*begin_dx + 0.622831 *lens_ipow(begin_lambda, 3) + 0.252479 *lens_ipow(begin_y, 2)*begin_lambda + 15.3869 *begin_y*begin_dy*begin_lambda + 0.000265991 *lens_ipow(begin_x, 4) + 70.3475 *begin_x*lens_ipow(begin_dx, 3) + 2.5471 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + -28.4185 *begin_y*begin_dy*lens_ipow(begin_lambda, 2) + 738.725 *lens_ipow(begin_dx, 4) + -0.469428 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2) + 0.041834 *lens_ipow(begin_x, 3)*begin_dx + 17.1989 *begin_y*begin_dy*lens_ipow(begin_lambda, 3) + 0.285723 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 3) + -3.19292e-09 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dy+0.0f;
        dx1_domega0[0][1] =  + 0.0779104 *begin_x*begin_y + 57.5505 *begin_dx*begin_dy + 2.11564 *begin_x*begin_dy + 15.3869 *begin_y*begin_dx*begin_lambda + -28.4185 *begin_y*begin_dx*lens_ipow(begin_lambda, 2) + 17.1989 *begin_y*begin_dx*lens_ipow(begin_lambda, 3) + -3.19292e-09 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dx+0.0f;
        dx1_domega0[1][0] =  + 2.07807 *begin_y*begin_dx + 2.71107 *begin_x*begin_dy + 0.0773918 *begin_x*begin_y + 56.5856 *begin_dx*begin_dy + 1.89528e-05 *lens_ipow(begin_y, 5)*begin_dx + 0.000828343 *lens_ipow(begin_y, 4)*begin_dx*begin_dy + 7.77217e-09 *begin_x*lens_ipow(begin_y, 7)*begin_lambda + 6.2086e-07 *begin_x*lens_ipow(begin_y, 6)*begin_dy*lens_ipow(begin_lambda, 2)+0.0f;
        dx1_domega0[1][1] =  + 33.2697  + 0.108851 *lens_ipow(begin_y, 2) + 6.52068 *begin_y*begin_dy + 2.71107 *begin_x*begin_dx + 28.2928 *lens_ipow(begin_dx, 2) + -1.94549 *lens_ipow(begin_lambda, 2) + 0.0442643 *lens_ipow(begin_x, 2) + 65.1857 *lens_ipow(begin_dy, 2) + 1.83525 *lens_ipow(begin_lambda, 3) + 0.000304092 *lens_ipow(begin_y, 4) + 784.387 *lens_ipow(begin_dy, 4) + 2.76267 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + 74.9321 *begin_y*lens_ipow(begin_dy, 3) + 0.0466494 *lens_ipow(begin_y, 3)*begin_dy + 0.000414172 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2) + 6.2086e-07 *begin_x*lens_ipow(begin_y, 6)*begin_dx*lens_ipow(begin_lambda, 2)+0.0f;
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


    case zeiss_biotar_1927_58mm:
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
        const float begin_lambda = lambda;
        pred_x =  + 36.5996 *begin_dx + 0.454717 *begin_x + 0.041798 *begin_x*begin_lambda + 1.8289 *begin_dx*begin_lambda + -0.005327 *lens_ipow(begin_x, 2)*begin_dx + -22.4015 *lens_ipow(begin_dx, 3) + -23.0927 *begin_dx*lens_ipow(begin_dy, 2) + -0.000160418 *begin_x*lens_ipow(begin_y, 2) + -0.000163105 *lens_ipow(begin_x, 3) + -0.216495 *begin_x*lens_ipow(begin_dy, 2) + -0.462226 *begin_x*lens_ipow(begin_dx, 2) + -0.00111586 *lens_ipow(begin_y, 2)*begin_dx + -0.0170951 *begin_x*begin_y*begin_dy*begin_lambda + -0.603514 *begin_y*begin_dx*begin_dy*begin_lambda + -3.98252e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2) + 0.0316894 *begin_x*begin_y*lens_ipow(begin_dx, 2)*begin_dy + -1.29849e-07 *begin_x*lens_ipow(begin_y, 4) + -1.6084e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dx + -0.426172 *begin_y*begin_dx*lens_ipow(begin_dy, 3) + 0.0202469 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 3) + -7.04916e-08 *lens_ipow(begin_x, 6)*begin_dx + -1.97483e-06 *lens_ipow(begin_x, 5)*lens_ipow(begin_dx, 2) + 0.830881 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 4) + -7.23502e-10 *lens_ipow(begin_x, 7) + -2.65605e-08 *lens_ipow(begin_x, 5)*begin_y*begin_dy*begin_lambda + -3.26686e-05 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 3)*begin_lambda + -6.91074e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dy*begin_lambda + -2.27346e-12 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 4);
        pred_y =  + 0.456094 *begin_y + 36.6078 *begin_dy + 1.85026 *begin_dy*begin_lambda + 0.0401649 *begin_y*begin_lambda + -0.218319 *begin_y*lens_ipow(begin_dx, 2) + -0.00569834 *lens_ipow(begin_y, 2)*begin_dy + -0.479457 *begin_y*lens_ipow(begin_dy, 2) + -23.1138 *lens_ipow(begin_dx, 2)*begin_dy + -0.000170969 *lens_ipow(begin_x, 2)*begin_y + -0.000164487 *lens_ipow(begin_y, 3) + -0.00134042 *lens_ipow(begin_x, 2)*begin_dy + -22.5954 *lens_ipow(begin_dy, 3) + -0.0143399 *begin_x*begin_y*begin_dx*begin_lambda + -0.610815 *begin_x*begin_dx*begin_dy*begin_lambda + -0.441549 *begin_x*lens_ipow(begin_dx, 3)*begin_dy + -3.59301e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + -1.05117e-07 *lens_ipow(begin_x, 4)*begin_y + 0.000261591 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dy, 2) + -1.86206e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dy*begin_lambda + 0.0593064 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + -7.67347e-10 *lens_ipow(begin_y, 7) + -7.31072e-08 *lens_ipow(begin_y, 6)*begin_dy + -2.04157e-06 *lens_ipow(begin_y, 5)*lens_ipow(begin_dy, 2) + -3.34223e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dx + 0.0196232 *begin_x*begin_y*begin_dx*lens_ipow(begin_lambda, 4) + 0.903883 *begin_x*begin_dx*begin_dy*lens_ipow(begin_lambda, 4) + -3.19105e-08 *begin_x*lens_ipow(begin_y, 5)*begin_dx*begin_lambda + -2.10282e-12 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 5);
        pred_dx =  + 0.467568 *begin_dx + -0.0229943 *begin_x + 0.00742905 *begin_x*begin_lambda + -0.0171701 *begin_y*begin_dx*begin_dy + -0.000165131 *begin_x*begin_y*begin_dy + -0.249908 *lens_ipow(begin_dx, 3) + -0.00536854 *begin_x*lens_ipow(begin_lambda, 2) + -0.262829 *begin_dx*lens_ipow(begin_dy, 2) + -1.04805e-05 *begin_x*lens_ipow(begin_y, 2) + -1.08788e-05 *lens_ipow(begin_x, 3) + -0.00954645 *begin_x*lens_ipow(begin_dy, 2) + -0.0268255 *begin_x*lens_ipow(begin_dx, 2) + -0.000894062 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + -0.000291871 *lens_ipow(begin_y, 2)*begin_dx*begin_lambda + -1.74848e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2) + 0.000575377 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_lambda, 2) + 7.50303e-08 *lens_ipow(begin_x, 4)*begin_y*begin_dx*begin_dy + -0.000109488 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 4) + -2.49935e-11 *begin_x*lens_ipow(begin_y, 6) + -0.41979 *begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3) + -3.44027e-08 *lens_ipow(begin_x, 5)*lens_ipow(begin_dy, 2) + -2.02028e-11 *lens_ipow(begin_x, 7) + -0.529128 *begin_x*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2)*begin_lambda + -1.99617e-13 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 4) + 9.41261e-07 *lens_ipow(begin_y, 4)*begin_dx*lens_ipow(begin_lambda, 4) + -1.24327 *begin_y*begin_dx*lens_ipow(begin_dy, 7)*begin_lambda + -0.88097 *begin_x*lens_ipow(begin_dx, 8)*begin_lambda + -6.49398e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dy*lens_ipow(begin_lambda, 3);
        pred_dy =  + -0.0229102 *begin_y + 0.467941 *begin_dy + 0.00700857 *begin_y*begin_lambda + -0.00960727 *begin_y*lens_ipow(begin_dx, 2) + -0.0273515 *begin_y*lens_ipow(begin_dy, 2) + -0.0175974 *begin_x*begin_dx*begin_dy + -0.00017146 *begin_x*begin_y*begin_dx + -0.249872 *lens_ipow(begin_dx, 2)*begin_dy + -0.00498688 *begin_y*lens_ipow(begin_lambda, 2) + -1.05302e-05 *lens_ipow(begin_x, 2)*begin_y + -9.64725e-06 *lens_ipow(begin_y, 3) + -0.26292 *lens_ipow(begin_dy, 3) + -0.000281665 *lens_ipow(begin_x, 2)*begin_dy*begin_lambda + -0.000789011 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + -2.11368e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + -9.8521e-09 *lens_ipow(begin_y, 5) + -4.56476e-08 *lens_ipow(begin_y, 5)*lens_ipow(begin_dx, 2) + -5.02616e-11 *lens_ipow(begin_x, 6)*begin_y + -0.114917 *begin_y*lens_ipow(begin_dy, 6) + -2.57554e-09 *begin_x*lens_ipow(begin_y, 5)*begin_dx*begin_lambda + 0.00314807 *lens_ipow(begin_y, 2)*begin_dy*lens_ipow(begin_lambda, 5) + 5.12646e-11 *lens_ipow(begin_x, 6)*begin_y*begin_lambda + -4.41944e-07 *lens_ipow(begin_x, 4)*begin_y*lens_ipow(begin_dy, 4) + -0.00412254 *lens_ipow(begin_y, 2)*begin_dy*lens_ipow(begin_lambda, 7) + 1.48608e-06 *lens_ipow(begin_x, 4)*begin_dy*lens_ipow(begin_lambda, 5) + -28.2788 *begin_x*lens_ipow(begin_dx, 7)*lens_ipow(begin_dy, 3) + 1533.74 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 7) + -4.79903e-16 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 7);
        float dx1_domega0[2][2];
        dx1_domega0[0][0] =  + 36.5996  + 1.8289 *begin_lambda + -0.005327 *lens_ipow(begin_x, 2) + -67.2044 *lens_ipow(begin_dx, 2) + -23.0927 *lens_ipow(begin_dy, 2) + -0.924453 *begin_x*begin_dx + -0.00111586 *lens_ipow(begin_y, 2) + -0.603514 *begin_y*begin_dy*begin_lambda + 0.0633788 *begin_x*begin_y*begin_dx*begin_dy + -1.6084e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -0.426172 *begin_y*lens_ipow(begin_dy, 3) + -7.04916e-08 *lens_ipow(begin_x, 6) + -3.94966e-06 *lens_ipow(begin_x, 5)*begin_dx + 0.830881 *begin_y*begin_dy*lens_ipow(begin_lambda, 4) + -9.80058e-05 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*begin_lambda+0.0f;
        dx1_domega0[0][1] =  + -46.1854 *begin_dx*begin_dy + -0.432989 *begin_x*begin_dy + -0.0170951 *begin_x*begin_y*begin_lambda + -0.603514 *begin_y*begin_dx*begin_lambda + 0.0316894 *begin_x*begin_y*lens_ipow(begin_dx, 2) + -1.27852 *begin_y*begin_dx*lens_ipow(begin_dy, 2) + 0.0202469 *begin_x*begin_y*lens_ipow(begin_lambda, 3) + 0.830881 *begin_y*begin_dx*lens_ipow(begin_lambda, 4) + -2.65605e-08 *lens_ipow(begin_x, 5)*begin_y*begin_lambda + -6.91074e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_lambda+0.0f;
        dx1_domega0[1][0] =  + -0.436637 *begin_y*begin_dx + -46.2276 *begin_dx*begin_dy + -0.0143399 *begin_x*begin_y*begin_lambda + -0.610815 *begin_x*begin_dy*begin_lambda + -1.32465 *begin_x*lens_ipow(begin_dx, 2)*begin_dy + 0.0593064 *begin_x*begin_y*lens_ipow(begin_dy, 2)*begin_lambda + -3.34223e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3) + 0.0196232 *begin_x*begin_y*lens_ipow(begin_lambda, 4) + 0.903883 *begin_x*begin_dy*lens_ipow(begin_lambda, 4) + -3.19105e-08 *begin_x*lens_ipow(begin_y, 5)*begin_lambda+0.0f;
        dx1_domega0[1][1] =  + 36.6078  + 1.85026 *begin_lambda + -0.00569834 *lens_ipow(begin_y, 2) + -0.958913 *begin_y*begin_dy + -23.1138 *lens_ipow(begin_dx, 2) + -0.00134042 *lens_ipow(begin_x, 2) + -67.7861 *lens_ipow(begin_dy, 2) + -0.610815 *begin_x*begin_dx*begin_lambda + -0.441549 *begin_x*lens_ipow(begin_dx, 3) + 0.000523182 *lens_ipow(begin_x, 2)*begin_y*begin_dy + -1.86206e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_lambda + 0.118613 *begin_x*begin_y*begin_dx*begin_dy*begin_lambda + -7.31072e-08 *lens_ipow(begin_y, 6) + -4.08314e-06 *lens_ipow(begin_y, 5)*begin_dy + 0.903883 *begin_x*begin_dx*lens_ipow(begin_lambda, 4)+0.0f;
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

    case takumar_1969_50mm:
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
        const float begin_lambda = lambda;
        pred_x =  + 30.4517 *begin_dx + 0.335491 *begin_x + 0.189788 *begin_x*begin_lambda + 5.85562 *begin_dx*begin_lambda + -0.141897 *begin_y*begin_dx*begin_dy + 0.00157184 *begin_x*begin_y*begin_dy + 0.0036529 *lens_ipow(begin_x, 2)*begin_dx + -18.0466 *lens_ipow(begin_dx, 3) + -0.139713 *begin_x*lens_ipow(begin_lambda, 2) + -17.5218 *begin_dx*lens_ipow(begin_dy, 2) + -0.000157628 *begin_x*lens_ipow(begin_y, 2) + -0.000149439 *lens_ipow(begin_x, 3) + -0.164804 *begin_x*lens_ipow(begin_dy, 2) + -0.277242 *begin_x*lens_ipow(begin_dx, 2) + -4.06094 *begin_dx*lens_ipow(begin_lambda, 2) + 8.19514e-05 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + 6.17978e-05 *lens_ipow(begin_x, 3)*begin_lambda + 0.00342147 *lens_ipow(begin_y, 2)*begin_dx*begin_lambda + -4.00211e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2) + -1.93383e-07 *lens_ipow(begin_x, 5) + -2.14205e-07 *begin_x*lens_ipow(begin_y, 4) + 0.243286 *begin_x*lens_ipow(begin_dy, 4) + -0.0124942 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 2) + 5.92223 *lens_ipow(begin_dx, 5)*begin_lambda + 0.148547 *begin_x*begin_y*lens_ipow(begin_dx, 4)*begin_dy + -0.0244494 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + -43.3246 *begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 5) + -3.99556e-11 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dx*begin_dy;
        pred_y =  + 0.333768 *begin_y + 30.3142 *begin_dy + 6.19823 *begin_dy*begin_lambda + 0.194459 *begin_y*begin_lambda + -0.16036 *begin_y*lens_ipow(begin_dx, 2) + 0.00366671 *lens_ipow(begin_y, 2)*begin_dy + -0.277416 *begin_y*lens_ipow(begin_dy, 2) + -0.132609 *begin_x*begin_dx*begin_dy + 0.00165636 *begin_x*begin_y*begin_dx + -17.3693 *lens_ipow(begin_dx, 2)*begin_dy + -0.142883 *begin_y*lens_ipow(begin_lambda, 2) + -0.000147929 *lens_ipow(begin_x, 2)*begin_y + -4.24615 *begin_dy*lens_ipow(begin_lambda, 2) + -0.000147913 *lens_ipow(begin_y, 3) + 0.00203755 *lens_ipow(begin_x, 2)*begin_dy + -18.0699 *lens_ipow(begin_dy, 3) + 6.06488e-05 *lens_ipow(begin_y, 3)*begin_lambda + 6.63746e-05 *lens_ipow(begin_x, 2)*begin_y*begin_lambda + 0.224708 *begin_y*lens_ipow(begin_dx, 4) + 3.54072 *lens_ipow(begin_dy, 5) + -0.0124248 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_dy + -4.00944e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + -2.13743e-07 *lens_ipow(begin_x, 4)*begin_y + -1.95879e-07 *lens_ipow(begin_y, 5) + -0.0087466 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 3) + 0.141742 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 4) + -4.65243 *begin_x*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3) + -4.04516e-11 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_dx*begin_dy;
        pred_dx =  + 0.354976 *begin_dx + -0.0297645 *begin_x + 0.0101801 *begin_x*begin_lambda + -0.0254185 *begin_y*begin_dx*begin_dy + -0.000384867 *lens_ipow(begin_x, 2)*begin_dx + -0.745377 *lens_ipow(begin_dx, 3) + -0.00743799 *begin_x*lens_ipow(begin_lambda, 2) + -0.703223 *begin_dx*lens_ipow(begin_dy, 2) + -1.16151e-05 *begin_x*lens_ipow(begin_y, 2) + -1.25599e-05 *lens_ipow(begin_x, 3) + -0.0134835 *begin_x*lens_ipow(begin_dy, 2) + -0.0397407 *begin_x*lens_ipow(begin_dx, 2) + -0.000137945 *lens_ipow(begin_y, 2)*begin_dx + -0.000160391 *begin_x*begin_y*begin_dy*begin_lambda + -3.25033e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2) + 1.20509e-06 *lens_ipow(begin_x, 4)*begin_dx + -9.33333e-09 *lens_ipow(begin_x, 5) + -1.51762e-08 *begin_x*lens_ipow(begin_y, 4) + 7.91498e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dx + -0.000743166 *begin_x*begin_y*lens_ipow(begin_dy, 3) + 3.03966e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*begin_lambda + -9.2384e-07 *lens_ipow(begin_x, 4)*begin_dx*begin_lambda + -2.14671e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + 3.21907 *lens_ipow(begin_dx, 7)*begin_lambda + 5.58099e-09 *lens_ipow(begin_y, 6)*lens_ipow(begin_dx, 3) + -4.92172 *begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 5)*begin_lambda + 519.145 *lens_ipow(begin_dx, 7)*lens_ipow(begin_dy, 4) + 5.01736e-14 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 4)*begin_dx;
        pred_dy =  + -0.0298109 *begin_y + 0.352584 *begin_dy + 0.0105834 *begin_y*begin_lambda + -0.0130088 *begin_y*lens_ipow(begin_dx, 2) + -0.000316348 *lens_ipow(begin_y, 2)*begin_dy + -0.0376198 *begin_y*lens_ipow(begin_dy, 2) + -0.024998 *begin_x*begin_dx*begin_dy + -0.000150875 *begin_x*begin_y*begin_dx + -0.662402 *lens_ipow(begin_dx, 2)*begin_dy + -0.00765622 *begin_y*lens_ipow(begin_lambda, 2) + -1.39981e-05 *lens_ipow(begin_x, 2)*begin_y + -1.48982e-05 *lens_ipow(begin_y, 3) + -0.000178082 *lens_ipow(begin_x, 2)*begin_dy + -0.699257 *lens_ipow(begin_dy, 3) + 1.99975e-07 *lens_ipow(begin_x, 4)*begin_dy + -3.1307e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + 1.03186e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dy + -0.0564625 *begin_x*begin_dx*lens_ipow(begin_dy, 3)*begin_lambda + -0.0656574 *begin_y*lens_ipow(begin_dy, 4)*begin_lambda + 4.13981e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_lambda + -0.00184793 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 3)*begin_lambda + 4.07312e-09 *lens_ipow(begin_y, 6)*begin_dy + 4.45956e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dx + -2.81257e-11 *lens_ipow(begin_x, 6)*begin_y + 0.000625798 *begin_x*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 3)*begin_lambda + 5.8015e-12 *begin_x*lens_ipow(begin_y, 7)*begin_dx + -6.65151e-12 *lens_ipow(begin_y, 8)*begin_dy*begin_lambda + -2.9162e-12 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 5)*lens_ipow(begin_dy, 2);
        float dx1_domega0[2][2];
        dx1_domega0[0][0] =  + 30.4517  + 5.85562 *begin_lambda + -0.141897 *begin_y*begin_dy + 0.0036529 *lens_ipow(begin_x, 2) + -54.1398 *lens_ipow(begin_dx, 2) + -17.5218 *lens_ipow(begin_dy, 2) + -0.554483 *begin_x*begin_dx + -4.06094 *lens_ipow(begin_lambda, 2) + 0.00342147 *lens_ipow(begin_y, 2)*begin_lambda + -0.0124942 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + 29.6111 *lens_ipow(begin_dx, 4)*begin_lambda + 0.594187 *begin_x*begin_y*lens_ipow(begin_dx, 3)*begin_dy + -0.0733483 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -129.974 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 5) + -3.99556e-11 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dy+0.0f;
        dx1_domega0[0][1] =  + -0.141897 *begin_y*begin_dx + 0.00157184 *begin_x*begin_y + -35.0437 *begin_dx*begin_dy + -0.329608 *begin_x*begin_dy + 0.973146 *begin_x*lens_ipow(begin_dy, 3) + -0.0249884 *lens_ipow(begin_x, 2)*begin_dx*begin_dy + 0.148547 *begin_x*begin_y*lens_ipow(begin_dx, 4) + -216.623 *begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 4) + -3.99556e-11 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dx+0.0f;
        dx1_domega0[1][0] =  + -0.32072 *begin_y*begin_dx + -0.132609 *begin_x*begin_dy + 0.00165636 *begin_x*begin_y + -34.7386 *begin_dx*begin_dy + 0.898834 *begin_y*lens_ipow(begin_dx, 3) + -0.0248497 *lens_ipow(begin_y, 2)*begin_dx*begin_dy + 0.141742 *begin_x*begin_y*lens_ipow(begin_dy, 4) + -13.9573 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + -4.04516e-11 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_dy+0.0f;
        dx1_domega0[1][1] =  + 30.3142  + 6.19823 *begin_lambda + 0.00366671 *lens_ipow(begin_y, 2) + -0.554831 *begin_y*begin_dy + -0.132609 *begin_x*begin_dx + -17.3693 *lens_ipow(begin_dx, 2) + -4.24615 *lens_ipow(begin_lambda, 2) + 0.00203755 *lens_ipow(begin_x, 2) + -54.2097 *lens_ipow(begin_dy, 2) + 17.7036 *lens_ipow(begin_dy, 4) + -0.0124248 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + -0.0262398 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + 0.566968 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 3) + -13.9573 *begin_x*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2) + -4.04516e-11 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_dx+0.0f;
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
    
    case NONE:
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
            x += 0.72 * invJ[0][i]*delta_out[i];
            y += 0.72 * invJ[1][i]*delta_out[i];
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


    case petzval_1900_66mm:
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
            x += 0.72 * invJ[0][i]*delta_out[i];
            y += 0.72 * invJ[1][i]*delta_out[i];
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


    case doublegauss_100mm:
    {
      float view[3] = { scene_x, scene_y, scene_z + camera_data->lens_outer_pupil_curvature_radius};
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
             + 66.2069 *begin_dx + 0.5962 *begin_x + 0.250805 *begin_x*begin_lambda + 17.9457 *begin_dx*begin_lambda + -0.631637 *begin_y*begin_dx*begin_dy + -0.00169732 *begin_x*begin_y*begin_dy + -0.00274299 *lens_ipow(begin_x, 2)*begin_dx + -48.5219 *lens_ipow(begin_dx, 3) + -0.346547 *begin_x*lens_ipow(begin_lambda, 2) + -46.9272 *begin_dx*lens_ipow(begin_dy, 2) + -2.76025e-06 *lens_ipow(begin_x, 3) + -0.29773 *begin_x*lens_ipow(begin_dy, 2) + -0.945888 *begin_x*lens_ipow(begin_dx, 2) + -0.00117191 *lens_ipow(begin_y, 2)*begin_dx + -24.4136 *begin_dx*lens_ipow(begin_lambda, 2) + 11.7019 *begin_dx*lens_ipow(begin_lambda, 3) + 0.167874 *begin_x*lens_ipow(begin_lambda, 3) + -4.80601e-06 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + -0.00107164 *begin_x*begin_y*begin_dy*begin_lambda + -0.00151481 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + -0.0170197 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 3) + 48.409 *lens_ipow(begin_dx, 5) + -0.00013336 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + -0.0107591 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3) + -0.0237075 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + 121.445 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2)*begin_lambda + -0.000262956 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2)*begin_lambda + -0.000233004 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2),
             + 0.623078 *begin_y + 67.8437 *begin_dy + 8.18959 *begin_dy*begin_lambda + 0.0989906 *begin_y*begin_lambda + -0.303511 *begin_y*lens_ipow(begin_dx, 2) + -0.00213596 *lens_ipow(begin_y, 2)*begin_dy + -0.858359 *begin_y*lens_ipow(begin_dy, 2) + -0.00237215 *begin_x*begin_y*begin_dx + -0.067194 *begin_y*lens_ipow(begin_lambda, 2) + -4.29705e-06 *lens_ipow(begin_x, 2)*begin_y + -5.64735 *begin_dy*lens_ipow(begin_lambda, 2) + -45.5101 *lens_ipow(begin_dy, 3) + -201.662 *lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + -0.00104475 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + -2.35583 *begin_x*begin_dx*begin_dy*begin_lambda + 2.1099 *begin_x*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -0.0133921 *lens_ipow(begin_x, 2)*begin_dy*lens_ipow(begin_lambda, 2) + 259.61 *lens_ipow(begin_dx, 2)*begin_dy*lens_ipow(begin_lambda, 2) + -1.13034e-05 *lens_ipow(begin_y, 4)*begin_dy*begin_lambda + 41.9825 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3)*begin_lambda + -94.6589 *lens_ipow(begin_dx, 2)*begin_dy*lens_ipow(begin_lambda, 3) + -0.00229265 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2)*begin_lambda + -2.62607e-08 *lens_ipow(begin_y, 5)*begin_lambda + 0.015556 *lens_ipow(begin_x, 2)*begin_dy*lens_ipow(begin_lambda, 3) + -8.02377 *begin_y*lens_ipow(begin_dy, 4)*begin_lambda + -0.22871 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 3)*begin_lambda + -4.33424 *begin_y*lens_ipow(begin_dy, 4)*lens_ipow(begin_lambda, 2) + -264.677 *lens_ipow(begin_dy, 5)*lens_ipow(begin_lambda, 2)
          };
          const float delta_ap[] = {ap_x - pred_ap[0], ap_y - pred_ap[1]};
          sqr_ap_err = delta_ap[0]*delta_ap[0]+delta_ap[1]*delta_ap[1];
          float dx1_domega0[2][2];
          dx1_domega0[0][0] =  + 66.2069  + 17.9457 *begin_lambda + -0.631637 *begin_y*begin_dy + -0.00274299 *lens_ipow(begin_x, 2) + -145.566 *lens_ipow(begin_dx, 2) + -46.9272 *lens_ipow(begin_dy, 2) + -1.89178 *begin_x*begin_dx + -0.00117191 *lens_ipow(begin_y, 2) + -24.4136 *lens_ipow(begin_lambda, 2) + 11.7019 *lens_ipow(begin_lambda, 3) + -0.00151481 *lens_ipow(begin_x, 2)*begin_lambda + -0.0510591 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + 242.045 *lens_ipow(begin_dx, 4) + -0.000266721 *begin_x*lens_ipow(begin_y, 2)*begin_dx + -0.0322772 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + -0.0237075 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2)*begin_lambda + 364.336 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*begin_lambda + -0.000525913 *lens_ipow(begin_x, 3)*begin_dx*begin_lambda+0.0f;
          dx1_domega0[0][1] =  + -0.631637 *begin_y*begin_dx + -0.00169732 *begin_x*begin_y + -93.8544 *begin_dx*begin_dy + -0.59546 *begin_x*begin_dy + -0.00107164 *begin_x*begin_y*begin_lambda + -0.047415 *lens_ipow(begin_x, 2)*begin_dx*begin_dy*begin_lambda + 242.891 *lens_ipow(begin_dx, 3)*begin_dy*begin_lambda + -0.000466008 *lens_ipow(begin_x, 3)*begin_dy*lens_ipow(begin_lambda, 2)+0.0f;
          dx1_domega0[1][0] =  + -0.607023 *begin_y*begin_dx + -0.00237215 *begin_x*begin_y + -403.324 *begin_dx*begin_dy*begin_lambda + -2.35583 *begin_x*begin_dy*begin_lambda + 2.1099 *begin_x*begin_dy*lens_ipow(begin_lambda, 2) + 519.221 *begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 83.965 *begin_dx*lens_ipow(begin_dy, 3)*begin_lambda + -189.318 *begin_dx*begin_dy*lens_ipow(begin_lambda, 3)+0.0f;
          dx1_domega0[1][1] =  + 67.8437  + 8.18959 *begin_lambda + -0.00213596 *lens_ipow(begin_y, 2) + -1.71672 *begin_y*begin_dy + -5.64735 *lens_ipow(begin_lambda, 2) + -136.53 *lens_ipow(begin_dy, 2) + -201.662 *lens_ipow(begin_dx, 2)*begin_lambda + -0.00104475 *lens_ipow(begin_y, 2)*begin_lambda + -2.35583 *begin_x*begin_dx*begin_lambda + 2.1099 *begin_x*begin_dx*lens_ipow(begin_lambda, 2) + -0.0133921 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 2) + 259.61 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -1.13034e-05 *lens_ipow(begin_y, 4)*begin_lambda + 125.947 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*begin_lambda + -94.6589 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 3) + -0.0045853 *lens_ipow(begin_y, 3)*begin_dy*begin_lambda + 0.015556 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 3) + -32.0951 *begin_y*lens_ipow(begin_dy, 3)*begin_lambda + -0.686131 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2)*begin_lambda + -17.3369 *begin_y*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + -1323.39 *lens_ipow(begin_dy, 4)*lens_ipow(begin_lambda, 2)+0.0f;
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
          out[0] =  + 100.12 *begin_dx + 0.592582 *begin_x + 0.0499465 *begin_x*begin_lambda + -0.811097 *begin_dx*begin_lambda + 0.0323359 *begin_x*begin_y*begin_dy + 0.0379048 *lens_ipow(begin_x, 2)*begin_dx + -42.7325 *lens_ipow(begin_dx, 3) + -0.0389817 *begin_x*lens_ipow(begin_lambda, 2) + 0.000142213 *begin_x*lens_ipow(begin_y, 2) + 0.000142435 *lens_ipow(begin_x, 3) + 1.67656 *begin_x*lens_ipow(begin_dy, 2) + 1.81407 *begin_x*lens_ipow(begin_dx, 2) + 0.00570726 *lens_ipow(begin_y, 2)*begin_dx + -209.372 *begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + 0.677473 *begin_y*begin_dx*begin_dy*begin_lambda + -0.536195 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 299.926 *begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.0713542 *begin_x*begin_y*lens_ipow(begin_dy, 3)*begin_lambda + 0.00125855 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2)*begin_lambda + -3.55319 *begin_x*lens_ipow(begin_dx, 4)*begin_lambda + 222.317 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2)*begin_lambda + -0.000266069 *lens_ipow(begin_y, 3)*begin_dx*begin_dy*begin_lambda + -5.3226e-06 *lens_ipow(begin_x, 3)*begin_y*begin_dy*begin_lambda + 0.00102282 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2)*begin_lambda + -168.54 *begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 4) + -3.93679e-10 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*begin_lambda + 1.32918e-08 *lens_ipow(begin_x, 6)*begin_dx*begin_lambda + -2.70347e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4)*begin_dx*begin_lambda;
          out[1] =  + 0.588556 *begin_y + 99.6889 *begin_dy + 0.0557925 *begin_y*begin_lambda + 1.62675 *begin_y*lens_ipow(begin_dx, 2) + 0.045876 *lens_ipow(begin_y, 2)*begin_dy + 2.49363 *begin_y*lens_ipow(begin_dy, 2) + 0.153878 *begin_x*begin_dx*begin_dy + 0.0314574 *begin_x*begin_y*begin_dx + -42.0967 *lens_ipow(begin_dx, 2)*begin_dy + -0.0358604 *begin_y*lens_ipow(begin_lambda, 2) + 0.000141395 *lens_ipow(begin_x, 2)*begin_y + 0.000178882 *lens_ipow(begin_y, 3) + 0.00572629 *lens_ipow(begin_x, 2)*begin_dy + -1.35881 *begin_y*lens_ipow(begin_dy, 2)*begin_lambda + -135.315 *lens_ipow(begin_dy, 3)*begin_lambda + -0.0168528 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + -6.88134e-05 *lens_ipow(begin_y, 3)*begin_lambda + 120.172 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + 9.10801e-06 *lens_ipow(begin_y, 4)*begin_dy + 0.0748529 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 3) + 106.566 *lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + 0.000223543 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + 0.00161417 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2) + 0.000235019 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dy, 2)*begin_lambda + 0.681351 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 4)*begin_dy + -0.000143401 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_dy + -9.81214e-11 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3) + -56.6549 *lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 6);
          out[2] =  + -3.05455 *begin_dx + -0.0282279 *begin_x + -0.000260254 *begin_x*begin_lambda + 0.150251 *begin_dx*begin_lambda + -0.193971 *begin_y*begin_dx*begin_dy + -0.00214261 *begin_x*begin_y*begin_dy + -0.000343775 *lens_ipow(begin_x, 2)*begin_dx + 1.28863 *lens_ipow(begin_dx, 3) + -13.7923 *begin_dx*lens_ipow(begin_dy, 2) + -6.63026e-06 *begin_x*lens_ipow(begin_y, 2) + -0.144619 *begin_x*lens_ipow(begin_dy, 2) + -0.0045328 *begin_x*lens_ipow(begin_dx, 2) + -0.000631766 *lens_ipow(begin_y, 2)*begin_dx + -0.108421 *begin_dx*lens_ipow(begin_lambda, 2) + -7.70575e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_lambda, 2) + -0.119872 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2) + 3.23657e-05 *lens_ipow(begin_x, 3)*lens_ipow(begin_lambda, 4) + 2457.14 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 4) + -0.00163849 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 0.274907 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 4) + 49.9949 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 4) + -4.84099e-06 *lens_ipow(begin_x, 3)*begin_y*lens_ipow(begin_dy, 3) + 1.54348e-06 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 3)*begin_lambda + -2.83936e-05 *lens_ipow(begin_x, 3)*lens_ipow(begin_lambda, 5) + -543.956 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 6) + -3.42461 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 6) + -25109 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 6) + -0.0503811 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 8);
          out[3] =  + -0.0282638 *begin_y + -3.02557 *begin_dy + 0.0262125 *begin_dy*begin_lambda + -0.000273248 *begin_y*begin_lambda + 0.0476166 *begin_y*lens_ipow(begin_dx, 2) + -0.00028279 *lens_ipow(begin_y, 2)*begin_dy + 0.278569 *begin_x*begin_dx*begin_dy + 0.000913623 *begin_x*begin_y*begin_dx + 16.1714 *lens_ipow(begin_dx, 2)*begin_dy + 5.08174e-06 *lens_ipow(begin_x, 2)*begin_y + 0.00117665 *lens_ipow(begin_x, 2)*begin_dy + 5.85395 *lens_ipow(begin_dy, 3)*begin_lambda + -1.29969e-06 *lens_ipow(begin_y, 3)*begin_lambda + -0.391177 *begin_x*lens_ipow(begin_dx, 3)*begin_dy + 58.129 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + -5.96864e-07 *begin_x*lens_ipow(begin_y, 3)*begin_dx + 1.32679 *begin_x*begin_dx*lens_ipow(begin_dy, 3) + 0.0182734 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 2) + -3.19863e-09 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + -6.23406 *lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + -2.7888e-05 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + 0.786317 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 0.00738819 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 3) + 1.77456e-05 *lens_ipow(begin_x, 3)*begin_dx*begin_dy + -23.2769 *lens_ipow(begin_dx, 4)*begin_dy + 0.000103722 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dy, 2) + 2.39665e-06 *lens_ipow(begin_y, 3)*lens_ipow(begin_lambda, 5) + 2.99896 *lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 6);
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
          domega2_dx0[0][0] =  + -0.0282279  + -0.000260254 *begin_lambda + -0.00214261 *begin_y*begin_dy + -0.00068755 *begin_x*begin_dx + -6.63026e-06 *lens_ipow(begin_y, 2) + -0.144619 *lens_ipow(begin_dy, 2) + -0.0045328 *lens_ipow(begin_dx, 2) + -2.31173e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 2) + 9.70971e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 4) + -0.00163849 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 0.549814 *begin_x*begin_dx*lens_ipow(begin_dy, 4) + 49.9949 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 4) + -1.4523e-05 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dy, 3) + -8.51808e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 5) + -543.956 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 6) + -6.84923 *begin_x*begin_dx*lens_ipow(begin_dy, 6) + -0.151143 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 8)+0.0f;
          domega2_dx0[0][1] =  + -0.193971 *begin_dx*begin_dy + -0.00214261 *begin_x*begin_dy + -1.32605e-05 *begin_x*begin_y + -0.00126353 *begin_y*begin_dx + -0.239743 *begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2) + -0.00327698 *begin_x*begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + -4.84099e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 3) + 6.17393e-06 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 3)*begin_lambda+0.0f;
          domega2_dx0[1][0] =  + 0.278569 *begin_dx*begin_dy + 0.000913623 *begin_y*begin_dx + 1.01635e-05 *begin_x*begin_y + 0.0023533 *begin_x*begin_dy + -0.391177 *lens_ipow(begin_dx, 3)*begin_dy + -5.96864e-07 *lens_ipow(begin_y, 3)*begin_dx + 1.32679 *begin_dx*lens_ipow(begin_dy, 3) + 0.0182734 *begin_y*begin_dx*lens_ipow(begin_dy, 2) + -6.39725e-09 *begin_x*lens_ipow(begin_y, 3) + 0.0147764 *begin_x*lens_ipow(begin_dy, 3) + 5.32368e-05 *lens_ipow(begin_x, 2)*begin_dx*begin_dy + 0.000207444 *begin_x*begin_y*lens_ipow(begin_dy, 2)+0.0f;
          domega2_dx0[1][1] =  + -0.0282638  + -0.000273248 *begin_lambda + 0.0476166 *lens_ipow(begin_dx, 2) + -0.00056558 *begin_y*begin_dy + 0.000913623 *begin_x*begin_dx + 5.08174e-06 *lens_ipow(begin_x, 2) + -3.89906e-06 *lens_ipow(begin_y, 2)*begin_lambda + -1.79059e-06 *begin_x*lens_ipow(begin_y, 2)*begin_dx + 0.0182734 *begin_x*begin_dx*lens_ipow(begin_dy, 2) + -9.59588e-09 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -8.36641e-05 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + 0.786317 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 0.000103722 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + 7.18995e-06 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 5)+0.0f;
          float invJ[2][2];
          const float invdet = 1.0f/(domega2_dx0[0][0]*domega2_dx0[1][1] - domega2_dx0[0][1]*domega2_dx0[1][0]);
          invJ[0][0] =  domega2_dx0[1][1]*invdet;
          invJ[1][1] =  domega2_dx0[0][0]*invdet;
          invJ[0][1] = -domega2_dx0[0][1]*invdet;
          invJ[1][0] = -domega2_dx0[1][0]*invdet;
          for(int i=0;i<2;i++)
          {
            x += 0.72 * invJ[0][i]*delta_out[i];
            y += 0.72 * invJ[1][i]*delta_out[i];
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
        out[4] =  + 3.76112 *begin_lambda + 0.000259609 *begin_y*begin_dy + 0.000266756 *begin_x*begin_dx + 0.0168177 *lens_ipow(begin_dy, 2) + 0.0171709 *lens_ipow(begin_dx, 2) + -10.7968 *lens_ipow(begin_lambda, 2) + 16.4369 *lens_ipow(begin_lambda, 3) + -12.9412 *lens_ipow(begin_lambda, 4) + -7.32614 *lens_ipow(begin_dx, 4) + -0.267851 *begin_y*lens_ipow(begin_dy, 3) + -0.00124542 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + -0.268464 *begin_x*lens_ipow(begin_dx, 3) + -14.667 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + -2.35604e-05 *lens_ipow(begin_x, 2)*begin_y*begin_dy + -7.31341 *lens_ipow(begin_dy, 4) + -2.35063e-05 *begin_x*lens_ipow(begin_y, 2)*begin_dx + -1.1016e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -5.37091e-08 *lens_ipow(begin_y, 4) + -0.003764 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + -0.268213 *begin_y*lens_ipow(begin_dx, 2)*begin_dy + -2.36431e-05 *lens_ipow(begin_x, 3)*begin_dx + -0.00375249 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + -0.26844 *begin_x*begin_dx*lens_ipow(begin_dy, 2) + -5.39032e-08 *lens_ipow(begin_x, 4) + -0.00500763 *begin_x*begin_y*begin_dx*begin_dy + -0.00124868 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + -2.35432e-05 *lens_ipow(begin_y, 3)*begin_dy + 4.15947 *lens_ipow(begin_lambda, 5);
      else
        out[4] = 0.0f;
    } break;


    case angenieux_doublegauss_1953_49mm:
    {
      float view[3] = { scene_x, scene_y, scene_z + camera_data->lens_outer_pupil_curvature_radius};
      normalise(view);
      int error = 0;
      if(1 || view[2] >= camera_data->lens_field_of_view)
      {
        const float eps = 1e-8;
        float sqr_err = 1e30, sqr_ap_err = 1e30;
        float prev_sqr_err = 1e32, prev_sqr_ap_err = 1e32;
        for(int k=0;k<100&&(sqr_err>eps||sqr_ap_err>eps)&&error==0;k++)
        {
          prev_sqr_err = sqr_err;
          prev_sqr_ap_err = sqr_ap_err;
          const float begin_x = x;
          const float begin_y = y;
          const float begin_dx = dx;
          const float begin_dy = dy;
          const float begin_lambda = lambda;
          const float pred_ap[2] = {
             + 26.7119 *begin_dx + 1.09538 *begin_x*begin_lambda + 6.60947 *begin_dx*begin_lambda + -0.0547589 *begin_y*begin_dx*begin_dy + 0.00189755 *lens_ipow(begin_x, 2)*begin_dx + -16.5359 *lens_ipow(begin_dx, 3) + -1.63151 *begin_x*lens_ipow(begin_lambda, 2) + -14.2808 *begin_dx*lens_ipow(begin_dy, 2) + -0.000478074 *begin_x*lens_ipow(begin_y, 2) + -0.000412757 *lens_ipow(begin_x, 3) + -0.184661 *begin_x*lens_ipow(begin_dy, 2) + -4.54335 *begin_dx*lens_ipow(begin_lambda, 2) + -0.356237 *begin_x*lens_ipow(begin_dx, 2)*begin_lambda + 0.838616 *begin_x*lens_ipow(begin_lambda, 3) + 4.95014e-06 *lens_ipow(begin_y, 4)*begin_dx + 7.55289 *lens_ipow(begin_dx, 5) + 0.310712 *begin_x*lens_ipow(begin_dy, 4) + -0.0272238 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 2) + -0.000405637 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2) + -0.0016729 *lens_ipow(begin_x, 2)*begin_y*begin_dx*begin_dy*begin_lambda + 1.38156e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2) + -5.59676e-09 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 2) + -2.08299e-09 *lens_ipow(begin_x, 7) + 1.51037e-09 *lens_ipow(begin_x, 7)*begin_lambda + 1.62764 *begin_x*lens_ipow(begin_dx, 4)*lens_ipow(begin_lambda, 3) + -2.43877e-11 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6) + -0.000166531 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 5) + 3.91242e-09 *begin_x*lens_ipow(begin_y, 7)*lens_ipow(begin_dx, 2)*begin_dy,
             + 26.6478 *begin_dy + 7.07798 *begin_dy*begin_lambda + 1.14323 *begin_y*begin_lambda + -0.180053 *begin_y*lens_ipow(begin_dx, 2) + -0.142826 *begin_y*lens_ipow(begin_dy, 2) + -0.0529828 *begin_x*begin_dx*begin_dy + -15.8269 *lens_ipow(begin_dx, 2)*begin_dy + -1.77677 *begin_y*lens_ipow(begin_lambda, 2) + -0.000519123 *lens_ipow(begin_x, 2)*begin_y + -4.90498 *begin_dy*lens_ipow(begin_lambda, 2) + -0.000503188 *lens_ipow(begin_y, 3) + 0.00136072 *lens_ipow(begin_x, 2)*begin_dy + -16.844 *lens_ipow(begin_dy, 3) + 0.931493 *begin_y*lens_ipow(begin_lambda, 3) + 0.000190732 *lens_ipow(begin_y, 3)*begin_lambda + 0.0001998 *lens_ipow(begin_x, 2)*begin_y*begin_lambda + -0.000822313 *begin_x*lens_ipow(begin_y, 2)*begin_dx*begin_dy + 0.318617 *begin_y*lens_ipow(begin_dx, 4) + 6.93717 *lens_ipow(begin_dy, 5) + -3.41864e-07 *lens_ipow(begin_x, 4)*begin_y + -0.00699567 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 3) + -0.000951 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2) + 0.000114581 *lens_ipow(begin_x, 3)*begin_y*begin_dx*lens_ipow(begin_dy, 2) + -1.3737e-09 *lens_ipow(begin_y, 7) + 88.5367 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 3) + -4.94822e-09 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 5) + -1.54899e-11 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3) + -0.00168031 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3)
          };
          const float delta_ap[] = {ap_x - pred_ap[0], ap_y - pred_ap[1]};
          sqr_ap_err = delta_ap[0]*delta_ap[0]+delta_ap[1]*delta_ap[1];
          float dx1_domega0[2][2];
          dx1_domega0[0][0] =  + 26.7119  + 6.60947 *begin_lambda + -0.0547589 *begin_y*begin_dy + 0.00189755 *lens_ipow(begin_x, 2) + -49.6076 *lens_ipow(begin_dx, 2) + -14.2808 *lens_ipow(begin_dy, 2) + -4.54335 *lens_ipow(begin_lambda, 2) + -0.712474 *begin_x*begin_dx*begin_lambda + 4.95014e-06 *lens_ipow(begin_y, 4) + 37.7644 *lens_ipow(begin_dx, 4) + -0.0272238 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + -0.000811273 *lens_ipow(begin_x, 3)*begin_dx + -0.0016729 *lens_ipow(begin_x, 2)*begin_y*begin_dy*begin_lambda + 6.51054 *begin_x*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 3) + -0.000832657 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 4) + 7.82484e-09 *begin_x*lens_ipow(begin_y, 7)*begin_dx*begin_dy+0.0f;
          dx1_domega0[0][1] =  + -0.0547589 *begin_y*begin_dx + -28.5616 *begin_dx*begin_dy + -0.369321 *begin_x*begin_dy + 1.24285 *begin_x*lens_ipow(begin_dy, 3) + -0.0544476 *lens_ipow(begin_x, 2)*begin_dx*begin_dy + -0.0016729 *lens_ipow(begin_x, 2)*begin_y*begin_dx*begin_lambda + 3.91242e-09 *begin_x*lens_ipow(begin_y, 7)*lens_ipow(begin_dx, 2)+0.0f;
          dx1_domega0[1][0] =  + -0.360107 *begin_y*begin_dx + -0.0529828 *begin_x*begin_dy + -31.6538 *begin_dx*begin_dy + -0.000822313 *begin_x*lens_ipow(begin_y, 2)*begin_dy + 1.27447 *begin_y*lens_ipow(begin_dx, 3) + 0.000114581 *lens_ipow(begin_x, 3)*begin_y*lens_ipow(begin_dy, 2) + 354.147 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3) + -0.00336062 *lens_ipow(begin_y, 4)*begin_dx*lens_ipow(begin_dy, 3)+0.0f;
          dx1_domega0[1][1] =  + 26.6478  + 7.07798 *begin_lambda + -0.285652 *begin_y*begin_dy + -0.0529828 *begin_x*begin_dx + -15.8269 *lens_ipow(begin_dx, 2) + -4.90498 *lens_ipow(begin_lambda, 2) + 0.00136072 *lens_ipow(begin_x, 2) + -50.532 *lens_ipow(begin_dy, 2) + -0.000822313 *begin_x*lens_ipow(begin_y, 2)*begin_dx + 34.6859 *lens_ipow(begin_dy, 4) + -0.020987 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + -0.001902 *lens_ipow(begin_y, 3)*begin_dy + 0.000229162 *lens_ipow(begin_x, 3)*begin_y*begin_dx*begin_dy + 265.61 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2) + -0.00504093 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)+0.0f;
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
          out[0] =  + 49.6109 *begin_dx + -0.621577 *begin_x + 0.674235 *begin_x*begin_lambda + 0.214431 *begin_y*begin_dx*begin_dy + 0.00612017 *begin_x*begin_y*begin_dy + 0.0185352 *lens_ipow(begin_x, 2)*begin_dx + -19.0762 *lens_ipow(begin_dx, 3) + -0.526696 *begin_x*lens_ipow(begin_lambda, 2) + -19.997 *begin_dx*lens_ipow(begin_dy, 2) + -0.00135091 *begin_x*lens_ipow(begin_y, 2) + -0.00120413 *lens_ipow(begin_x, 3) + 0.265561 *begin_x*lens_ipow(begin_dy, 2) + 0.547505 *begin_x*lens_ipow(begin_dx, 2) + 0.000620579 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + 0.000501355 *lens_ipow(begin_x, 3)*begin_lambda + 0.00915562 *lens_ipow(begin_y, 2)*begin_dx*begin_lambda + 0.0777405 *begin_x*begin_y*lens_ipow(begin_dx, 2)*begin_dy + -4.20841e-05 *lens_ipow(begin_x, 4)*begin_dx + -0.00108756 *lens_ipow(begin_y, 3)*begin_dx*begin_dy + 0.0289986 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 2) + -0.00208955 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2) + -4.49517e-06 *begin_x*lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2) + -9.51913e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4) + -3.46111e-09 *lens_ipow(begin_x, 7) + -7.05762e-12 *begin_x*lens_ipow(begin_y, 8) + -6.20338e-11 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 2) + 0.0285076 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 3)*begin_dy*lens_ipow(begin_lambda, 4) + 1.33412e-13 *lens_ipow(begin_x, 9)*lens_ipow(begin_y, 2);
          out[1] =  + -0.613564 *begin_y + 49.7175 *begin_dy + 0.669005 *begin_y*begin_lambda + 0.246172 *begin_y*lens_ipow(begin_dx, 2) + 0.0126827 *lens_ipow(begin_y, 2)*begin_dy + 0.232757 *begin_y*lens_ipow(begin_dy, 2) + 0.00701796 *begin_x*begin_y*begin_dx + -19.0477 *lens_ipow(begin_dx, 2)*begin_dy + -0.524051 *begin_y*lens_ipow(begin_lambda, 2) + -0.00133375 *lens_ipow(begin_x, 2)*begin_y + -0.00127186 *lens_ipow(begin_y, 3) + -21.4258 *lens_ipow(begin_dy, 3) + 0.0105956 *lens_ipow(begin_x, 2)*begin_dy*begin_lambda + 0.000544614 *lens_ipow(begin_y, 3)*begin_lambda + 0.333688 *begin_x*begin_dx*begin_dy*begin_lambda + 0.000640797 *lens_ipow(begin_x, 2)*begin_y*begin_lambda + -1.70371e-05 *lens_ipow(begin_y, 4)*begin_dy + 0.156726 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + 2.94679 *begin_y*lens_ipow(begin_dy, 4)*begin_lambda + 0.128866 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 3)*begin_lambda + -3.102e-09 *lens_ipow(begin_y, 7) + 0.000269581 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 2)*begin_dy + -1.5407e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dx + -2.72774e-09 *lens_ipow(begin_x, 6)*begin_y + 5.47118 *begin_y*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2) + -8.15344e-09 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 5) + -8.67539e-09 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3) + 67.1056 *lens_ipow(begin_dy, 7)*begin_lambda;
          out[2] =  + -0.614401 *begin_dx + -0.0124033 *begin_x + -0.00877929 *begin_x*begin_lambda + 0.00908301 *begin_dx*begin_lambda + -6.4694e-05 *begin_x*begin_y*begin_dy + -9.8007e-05 *lens_ipow(begin_x, 2)*begin_dx + 0.305271 *lens_ipow(begin_dx, 3) + 0.00706724 *begin_x*lens_ipow(begin_lambda, 2) + 0.148208 *begin_dx*lens_ipow(begin_dy, 2) + 1.74459e-05 *begin_x*lens_ipow(begin_y, 2) + 1.77203e-05 *lens_ipow(begin_x, 3) + -0.00111275 *begin_x*lens_ipow(begin_dy, 2) + -7.43443e-06 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + -7.39746e-06 *lens_ipow(begin_x, 3)*begin_lambda + -2.36067e-07 *lens_ipow(begin_x, 4)*begin_dx + -1.07743e-07 *lens_ipow(begin_y, 4)*begin_dx + -0.289597 *lens_ipow(begin_dx, 5) + 6.58747e-09 *begin_x*lens_ipow(begin_y, 4) + -3.09809e-07 *begin_x*lens_ipow(begin_y, 3)*begin_dy + -0.00453847 *begin_x*lens_ipow(begin_dy, 4) + 0.000362057 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3) + 3.63636e-05 *lens_ipow(begin_x, 2)*begin_y*begin_dx*begin_dy + 0.000693516 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 2) + 1.32547e-10 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 2) + -0.0485359 *begin_x*lens_ipow(begin_dx, 6) + 3.30777e-11 *lens_ipow(begin_x, 7) + 4.41165e-13 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6) + 0.0506785 *begin_x*begin_y*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 3)*begin_lambda;
          out[3] =  + -0.0119193 *begin_y + -0.614842 *begin_dy + -0.00958648 *begin_y*begin_lambda + -0.00420097 *begin_y*lens_ipow(begin_dx, 2) + 0.00389116 *begin_x*begin_dx*begin_dy + -0.000108381 *begin_x*begin_y*begin_dx + 0.428204 *lens_ipow(begin_dx, 2)*begin_dy + 0.00700915 *begin_y*lens_ipow(begin_lambda, 2) + 1.32734e-05 *lens_ipow(begin_x, 2)*begin_y + 1.40797e-05 *lens_ipow(begin_y, 3) + 0.322422 *lens_ipow(begin_dy, 3) + -0.469415 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + -0.191137 *lens_ipow(begin_dy, 5) + 2.31037e-08 *lens_ipow(begin_x, 4)*begin_y + -0.308648 *lens_ipow(begin_dx, 4)*begin_dy + -8.0158e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dy + -1.92001e-08 *lens_ipow(begin_x, 4)*begin_y*begin_lambda + -5.34204e-09 *lens_ipow(begin_y, 6)*begin_dy + 2.65798e-10 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 5) + -3.59985e-10 *lens_ipow(begin_x, 6)*begin_dy + -0.000157427 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dy, 4)*begin_lambda + -2.54677e-10 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 5)*begin_lambda + 9.48325e-14 *lens_ipow(begin_y, 9) + 3.56726e-13 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3) + 1.47454e-06 *begin_x*lens_ipow(begin_y, 4)*begin_dx*lens_ipow(begin_dy, 3) + 1.24034e-08 *lens_ipow(begin_y, 6)*lens_ipow(begin_dx, 2)*begin_dy + 2.22341e-11 *lens_ipow(begin_y, 8)*begin_dy*begin_lambda + -0.00303946 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2);
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
          domega2_dx0[0][0] =  + -0.0124033  + -0.00877929 *begin_lambda + -6.4694e-05 *begin_y*begin_dy + -0.000196014 *begin_x*begin_dx + 0.00706724 *lens_ipow(begin_lambda, 2) + 1.74459e-05 *lens_ipow(begin_y, 2) + 5.31609e-05 *lens_ipow(begin_x, 2) + -0.00111275 *lens_ipow(begin_dy, 2) + -7.43443e-06 *lens_ipow(begin_y, 2)*begin_lambda + -2.21924e-05 *lens_ipow(begin_x, 2)*begin_lambda + -9.44267e-07 *lens_ipow(begin_x, 3)*begin_dx + 6.58747e-09 *lens_ipow(begin_y, 4) + -3.09809e-07 *lens_ipow(begin_y, 3)*begin_dy + -0.00453847 *lens_ipow(begin_dy, 4) + 7.27272e-05 *begin_x*begin_y*begin_dx*begin_dy + 0.00138703 *begin_x*begin_dx*lens_ipow(begin_dy, 2) + 6.62735e-10 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 2) + -0.0485359 *lens_ipow(begin_dx, 6) + 2.31544e-10 *lens_ipow(begin_x, 6) + 1.3235e-12 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 6) + 0.0506785 *begin_y*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 3)*begin_lambda+0.0f;
          domega2_dx0[0][1] =  + -6.4694e-05 *begin_x*begin_dy + 3.48918e-05 *begin_x*begin_y + -1.48689e-05 *begin_x*begin_y*begin_lambda + -4.3097e-07 *lens_ipow(begin_y, 3)*begin_dx + 2.63499e-08 *begin_x*lens_ipow(begin_y, 3) + -9.29426e-07 *begin_x*lens_ipow(begin_y, 2)*begin_dy + 0.000724114 *begin_y*lens_ipow(begin_dx, 3) + 3.63636e-05 *lens_ipow(begin_x, 2)*begin_dx*begin_dy + 2.65094e-10 *lens_ipow(begin_x, 5)*begin_y + 2.64699e-12 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 5) + 0.0506785 *begin_x*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 3)*begin_lambda+0.0f;
          domega2_dx0[1][0] =  + 0.00389116 *begin_dx*begin_dy + -0.000108381 *begin_y*begin_dx + 2.65468e-05 *begin_x*begin_y + 9.24149e-08 *lens_ipow(begin_x, 3)*begin_y + -1.60316e-06 *begin_x*lens_ipow(begin_y, 2)*begin_dy + -7.68005e-08 *lens_ipow(begin_x, 3)*begin_y*begin_lambda + 5.31597e-10 *begin_x*lens_ipow(begin_y, 5) + -2.15991e-09 *lens_ipow(begin_x, 5)*begin_dy + -0.000314854 *begin_x*begin_y*lens_ipow(begin_dy, 4)*begin_lambda + -5.09353e-10 *begin_x*lens_ipow(begin_y, 5)*begin_lambda + 2.14036e-12 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 3) + 1.47454e-06 *lens_ipow(begin_y, 4)*begin_dx*lens_ipow(begin_dy, 3) + -0.00911838 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2)+0.0f;
          domega2_dx0[1][1] =  + -0.0119193  + -0.00958648 *begin_lambda + -0.00420097 *lens_ipow(begin_dx, 2) + -0.000108381 *begin_x*begin_dx + 0.00700915 *lens_ipow(begin_lambda, 2) + 1.32734e-05 *lens_ipow(begin_x, 2) + 4.22392e-05 *lens_ipow(begin_y, 2) + 2.31037e-08 *lens_ipow(begin_x, 4) + -1.60316e-06 *lens_ipow(begin_x, 2)*begin_y*begin_dy + -1.92001e-08 *lens_ipow(begin_x, 4)*begin_lambda + -3.20523e-08 *lens_ipow(begin_y, 5)*begin_dy + 1.32899e-09 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4) + -0.000157427 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 4)*begin_lambda + -1.27338e-09 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4)*begin_lambda + 8.53492e-13 *lens_ipow(begin_y, 8) + 1.07018e-12 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 2) + 5.89815e-06 *begin_x*lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_dy, 3) + 7.44204e-08 *lens_ipow(begin_y, 5)*lens_ipow(begin_dx, 2)*begin_dy + 1.77873e-10 *lens_ipow(begin_y, 7)*begin_dy*begin_lambda+0.0f;
          float invJ[2][2];
          const float invdet = 1.0f/(domega2_dx0[0][0]*domega2_dx0[1][1] - domega2_dx0[0][1]*domega2_dx0[1][0]);
          invJ[0][0] =  domega2_dx0[1][1]*invdet;
          invJ[1][1] =  domega2_dx0[0][0]*invdet;
          invJ[0][1] = -domega2_dx0[0][1]*invdet;
          invJ[1][0] = -domega2_dx0[1][0]*invdet;
          for(int i=0;i<2;i++)
          {
            x += 0.72 * invJ[0][i]*delta_out[i];
            y += 0.72 * invJ[1][i]*delta_out[i];
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
        out[4] =  + 0.238949  + 0.85346 *begin_lambda + -1.16866 *lens_ipow(begin_lambda, 2) + -0.000545894 *begin_y*begin_dy*begin_lambda + -0.000532821 *begin_x*begin_dx*begin_lambda + 0.564312 *lens_ipow(begin_lambda, 3) + -0.206872 *lens_ipow(begin_dx, 4) + -0.00790672 *begin_y*lens_ipow(begin_dy, 3) + -0.00013516 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + -0.00809514 *begin_x*lens_ipow(begin_dx, 3) + -0.204782 *lens_ipow(begin_dy, 4) + -8.85833e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -0.000243089 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + -0.00648351 *begin_y*lens_ipow(begin_dx, 2)*begin_dy + -0.000239826 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + -0.00654443 *begin_x*begin_dx*lens_ipow(begin_dy, 2) + -0.000460228 *begin_x*begin_y*begin_dx*begin_dy + -0.00013569 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + -1.59096 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*begin_lambda + -1.67893e-10 *lens_ipow(begin_x, 6) + -0.000863147 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 4) + 1.18371 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + -1.68689e-10 *lens_ipow(begin_y, 6) + -0.000849445 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 4) + -2.32122e-12 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4) + 8.17818e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dx*begin_dy + -0.321221 *lens_ipow(begin_dx, 6)*lens_ipow(begin_lambda, 2) + -0.341708 *lens_ipow(begin_dy, 6)*lens_ipow(begin_lambda, 2);
      else
        out[4] = 0.0f;
    } break;


    case fisheye_aspherical:
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
             + 24.0758 *begin_dx + 0.714404 *begin_x + 1.13639 *begin_y*begin_dx*begin_dy + 0.032411 *begin_x*begin_y*begin_dy + 0.0496792 *lens_ipow(begin_x, 2)*begin_dx + 9.19327 *lens_ipow(begin_dx, 3) + 10.149 *begin_dx*lens_ipow(begin_dy, 2) + 0.464028 *begin_x*lens_ipow(begin_dy, 2) + 1.51791 *begin_x*lens_ipow(begin_dx, 2) + 0.0192878 *lens_ipow(begin_y, 2)*begin_dx + -1.14405e-06 *lens_ipow(begin_x, 5) + -0.000677498 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 3) + -5.64672e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4) + -2.12508e-07 *lens_ipow(begin_x, 5)*begin_y*begin_dy + -6.02066e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4)*begin_dx + -0.0098213 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 4) + -1.90383e-08 *lens_ipow(begin_y, 7)*begin_dx*begin_dy + -6.69381e-10 *lens_ipow(begin_y, 8)*begin_dx + -7.78368e-08 *lens_ipow(begin_x, 7)*lens_ipow(begin_dx, 2) + -7.13255e-11 *begin_x*lens_ipow(begin_y, 8) + -2.5276e-10 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 2) + 1.63176e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 2) + -4.59009e-09 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4)*begin_dx*begin_lambda + 2.41334e-08 *lens_ipow(begin_x, 7)*lens_ipow(begin_dy, 2)*begin_lambda + -2.13711e-13 *lens_ipow(begin_x, 11) + -2.81141e-11 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 2)*begin_dx + -1.07931e-11 *lens_ipow(begin_x, 10)*begin_dx + -7.39003e-12 *begin_x*lens_ipow(begin_y, 9)*begin_dy,
             + 0.720082 *begin_y + 24.0428 *begin_dy + -0.014603 *begin_y*begin_lambda + 0.488201 *begin_y*lens_ipow(begin_dx, 2) + 0.0532851 *lens_ipow(begin_y, 2)*begin_dy + 1.6659 *begin_y*lens_ipow(begin_dy, 2) + 1.1315 *begin_x*begin_dx*begin_dy + 0.033149 *begin_x*begin_y*begin_dx + 10.3268 *lens_ipow(begin_dx, 2)*begin_dy + 0.0189151 *lens_ipow(begin_x, 2)*begin_dy + 10.648 *lens_ipow(begin_dy, 3) + 6.86709e-07 *lens_ipow(begin_y, 5)*begin_lambda + -1.18904e-08 *lens_ipow(begin_y, 7) + -1.59389e-07 *begin_x*lens_ipow(begin_y, 5)*begin_dx + -6.6993e-08 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3) + -7.33692e-07 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 2)*begin_dy + 2.01469e-08 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3)*begin_lambda + -5.45356e-11 *lens_ipow(begin_x, 8)*begin_y + -1.45339e-09 *lens_ipow(begin_y, 8)*begin_dy + 1.39319e-07 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + -2.51749e-10 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 7) + 1.70727e-10 *lens_ipow(begin_x, 8)*begin_y*lens_ipow(begin_dx, 2) + -2.91946e-11 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 8)*begin_dy + -7.61611e-14 *lens_ipow(begin_y, 11) + 5.90072e-11 *lens_ipow(begin_y, 9)*lens_ipow(begin_dx, 2) + -1.60411e-12 *lens_ipow(begin_x, 10)*begin_dy + 8.55147e-10 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_dx*begin_dy + -0.000201456 *lens_ipow(begin_x, 5)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3)
          };
          const float delta_ap[] = {ap_x - pred_ap[0], ap_y - pred_ap[1]};
          sqr_ap_err = delta_ap[0]*delta_ap[0]+delta_ap[1]*delta_ap[1];
          float dx1_domega0[2][2];
          dx1_domega0[0][0] =  + 24.0758  + 1.13639 *begin_y*begin_dy + 0.0496792 *lens_ipow(begin_x, 2) + 27.5798 *lens_ipow(begin_dx, 2) + 10.149 *lens_ipow(begin_dy, 2) + 3.03582 *begin_x*begin_dx + 0.0192878 *lens_ipow(begin_y, 2) + -0.00203249 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 2) + -6.02066e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4) + -0.0392852 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 3) + -1.90383e-08 *lens_ipow(begin_y, 7)*begin_dy + -6.69381e-10 *lens_ipow(begin_y, 8) + -1.55674e-07 *lens_ipow(begin_x, 7)*begin_dx + -4.59009e-09 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4)*begin_lambda + -2.81141e-11 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 2) + -1.07931e-11 *lens_ipow(begin_x, 10)+0.0f;
          dx1_domega0[0][1] =  + 1.13639 *begin_y*begin_dx + 0.032411 *begin_x*begin_y + 20.2981 *begin_dx*begin_dy + 0.928057 *begin_x*begin_dy + -2.12508e-07 *lens_ipow(begin_x, 5)*begin_y + -1.90383e-08 *lens_ipow(begin_y, 7)*begin_dx + 3.26352e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*begin_dy + 4.82668e-08 *lens_ipow(begin_x, 7)*begin_dy*begin_lambda + -7.39003e-12 *begin_x*lens_ipow(begin_y, 9)+0.0f;
          dx1_domega0[1][0] =  + 0.976401 *begin_y*begin_dx + 1.1315 *begin_x*begin_dy + 0.033149 *begin_x*begin_y + 20.6537 *begin_dx*begin_dy + -1.59389e-07 *begin_x*lens_ipow(begin_y, 5) + 2.78637e-07 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3)*begin_dx + 3.41454e-10 *lens_ipow(begin_x, 8)*begin_y*begin_dx + 1.18014e-10 *lens_ipow(begin_y, 9)*begin_dx + 8.55147e-10 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_dy + -0.000604368 *lens_ipow(begin_x, 5)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3)+0.0f;
          dx1_domega0[1][1] =  + 24.0428  + 0.0532851 *lens_ipow(begin_y, 2) + 3.3318 *begin_y*begin_dy + 1.1315 *begin_x*begin_dx + 10.3268 *lens_ipow(begin_dx, 2) + 0.0189151 *lens_ipow(begin_x, 2) + 31.944 *lens_ipow(begin_dy, 2) + -7.33692e-07 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 2) + -1.45339e-09 *lens_ipow(begin_y, 8) + -2.91946e-11 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 8) + -1.60411e-12 *lens_ipow(begin_x, 10) + 8.55147e-10 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_dx + -0.000604368 *lens_ipow(begin_x, 5)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2)+0.0f;
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
          out[0] =  + -1.10799 *begin_x + -0.0905798 *begin_x*begin_lambda + 40.196 *begin_dx*begin_lambda + 2.10515 *begin_y*begin_dx*begin_dy + 0.0754832 *begin_x*begin_y*begin_dy + 0.133226 *lens_ipow(begin_x, 2)*begin_dx + 26.791 *lens_ipow(begin_dx, 3) + 27.9088 *begin_dx*lens_ipow(begin_dy, 2) + 0.00105276 *begin_x*lens_ipow(begin_y, 2) + 0.00119667 *lens_ipow(begin_x, 3) + 1.23306 *begin_x*lens_ipow(begin_dy, 2) + 3.36803 *begin_x*lens_ipow(begin_dx, 2) + 0.0534469 *lens_ipow(begin_y, 2)*begin_dx + -61.0312 *begin_dx*lens_ipow(begin_lambda, 2) + 30.4395 *begin_dx*lens_ipow(begin_lambda, 3) + -0.00016579 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dx + -8.16049e-07 *lens_ipow(begin_x, 6)*begin_dx + -2.03688e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dy + -6.44038e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4) + -0.0262952 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dx, 3)*begin_dy + -3.92523e-08 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 2) + -0.00581142 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 4) + -1.95829e-08 *lens_ipow(begin_x, 7) + -0.128373 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 4) + -6.99074e-11 *begin_x*lens_ipow(begin_y, 8) + -2.8667e-09 *begin_x*lens_ipow(begin_y, 7)*begin_dy + 1.94447e-07 *lens_ipow(begin_x, 6)*begin_y*begin_dx*begin_dy + 1.37312e-09 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2);
          out[1] =  + -1.00151 *begin_y + 8.2466 *begin_dy + 0.97099 *begin_dy*begin_lambda + -0.455304 *begin_y*begin_lambda + 0.995845 *begin_y*lens_ipow(begin_dx, 2) + 0.130079 *lens_ipow(begin_y, 2)*begin_dy + 3.34769 *begin_y*lens_ipow(begin_dy, 2) + 1.9089 *begin_x*begin_dx*begin_dy + 0.0699908 *begin_x*begin_y*begin_dx + 18.5642 *lens_ipow(begin_dx, 2)*begin_dy + 0.324668 *begin_y*lens_ipow(begin_lambda, 2) + 0.000856088 *lens_ipow(begin_x, 2)*begin_y + 0.00110582 *lens_ipow(begin_y, 3) + 0.05103 *lens_ipow(begin_x, 2)*begin_dy + 26.4204 *lens_ipow(begin_dy, 3) + -1.25836 *begin_x*lens_ipow(begin_dx, 3)*begin_dy + -0.000134239 *lens_ipow(begin_x, 3)*begin_y*begin_dx + 0.00190002 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + -3.15418 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + -0.000167132 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dy + -1.79689e-08 *lens_ipow(begin_y, 7) + -7.19221e-07 *lens_ipow(begin_y, 6)*begin_dy + -1.1258e-08 *lens_ipow(begin_x, 6)*begin_y + -3.3697e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 5) + -3.903e-08 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3) + 0.143156 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 5) + 2.30271e-07 *begin_x*lens_ipow(begin_y, 6)*begin_dx*begin_dy*begin_lambda + 1.74713e-09 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 5)*lens_ipow(begin_dx, 2);
          out[2] =  + -0.24973 *begin_dx + -0.0790022 *begin_x + 0.00582542 *begin_x*begin_lambda + 0.00236612 *lens_ipow(begin_x, 2)*begin_dx + 3.81983e-05 *begin_x*lens_ipow(begin_y, 2) + 8.9246e-05 *lens_ipow(begin_x, 3) + 0.172699 *begin_x*lens_ipow(begin_dx, 2)*begin_lambda + 0.00165633 *lens_ipow(begin_y, 2)*begin_dx*begin_lambda + 5.81234e-06 *lens_ipow(begin_y, 4)*begin_dx + 6.72492 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + -6.06714 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 3) + 0.00621329 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_lambda, 3) + 0.000999368 *lens_ipow(begin_y, 3)*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -5.56523e-12 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 2) + 3.86556e-09 *begin_x*lens_ipow(begin_y, 6)*lens_ipow(begin_lambda, 2) + 5.03358e-05 *begin_x*lens_ipow(begin_y, 3)*begin_dy*lens_ipow(begin_lambda, 4) + 1.35499e-07 *begin_x*lens_ipow(begin_y, 5)*begin_dy*lens_ipow(begin_lambda, 2) + -3.11126e-05 *lens_ipow(begin_x, 4)*begin_dx*lens_ipow(begin_lambda, 4) + 6.72353e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*lens_ipow(begin_lambda, 2) + 171.022 *lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 4)*begin_lambda + -0.00458756 *lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 3) + -2.83521e-11 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_lambda + -2.96283e-11 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 4)*begin_lambda + -3.03637e-12 *lens_ipow(begin_x, 9)*begin_lambda + 3.03521e-08 *lens_ipow(begin_y, 6)*begin_dx*lens_ipow(begin_lambda, 4) + 8.32911e-05 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 4) + -1.12336e-11 *begin_x*lens_ipow(begin_y, 8)*lens_ipow(begin_lambda, 2) + 1.24652e-05 *begin_x*lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 4);
          out[3] =  + -0.0812351 *begin_y + -0.280867 *begin_dy + 0.0429656 *begin_dy*begin_lambda + 0.0140781 *begin_y*begin_lambda + 0.00330655 *lens_ipow(begin_y, 2)*begin_dy + 0.106182 *begin_y*lens_ipow(begin_dy, 2) + 0.0529577 *begin_x*begin_dx*begin_dy + -0.00852811 *begin_y*lens_ipow(begin_lambda, 2) + 0.000112606 *lens_ipow(begin_x, 2)*begin_y + 8.92269e-05 *lens_ipow(begin_y, 3) + 0.00131885 *lens_ipow(begin_x, 2)*begin_dy + 1.11666 *lens_ipow(begin_dy, 3) + 1.72313 *lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + 0.00206368 *begin_x*begin_y*begin_dx*begin_lambda + 0.0544768 *begin_x*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -0.0347859 *begin_x*lens_ipow(begin_dx, 3)*begin_dy + 0.114422 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + 0.00361735 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_dy + -0.0311424 *begin_y*lens_ipow(begin_dy, 4) + 9.33907e-05 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + -0.0805241 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 3) + 1.877e-05 *lens_ipow(begin_x, 4)*lens_ipow(begin_dy, 3)*begin_lambda + 2.03187e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -1.02479e-12 *lens_ipow(begin_y, 9)*begin_lambda + -5.92894e-12 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 7)*begin_lambda + -2.83505e-15 *lens_ipow(begin_y, 11) + -3.67053e-14 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 3) + -4.77863e-14 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 7);
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
          domega2_dx0[0][0] =  + -0.0790022  + 0.00582542 *begin_lambda + 0.00473224 *begin_x*begin_dx + 3.81983e-05 *lens_ipow(begin_y, 2) + 0.000267738 *lens_ipow(begin_x, 2) + 0.172699 *lens_ipow(begin_dx, 2)*begin_lambda + 0.0124266 *begin_x*begin_dx*lens_ipow(begin_lambda, 3) + -3.89566e-11 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 2) + 3.86556e-09 *lens_ipow(begin_y, 6)*lens_ipow(begin_lambda, 2) + 5.03358e-05 *lens_ipow(begin_y, 3)*begin_dy*lens_ipow(begin_lambda, 4) + 1.35499e-07 *lens_ipow(begin_y, 5)*begin_dy*lens_ipow(begin_lambda, 2) + -0.000124451 *lens_ipow(begin_x, 3)*begin_dx*lens_ipow(begin_lambda, 4) + 2.01706e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4)*lens_ipow(begin_lambda, 2) + -8.50564e-11 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 6)*begin_lambda + -1.48141e-10 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4)*begin_lambda + -2.73274e-11 *lens_ipow(begin_x, 8)*begin_lambda + 0.000333164 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 4) + -1.12336e-11 *lens_ipow(begin_y, 8)*lens_ipow(begin_lambda, 2) + 1.24652e-05 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 4)+0.0f;
          domega2_dx0[0][1] =  + 7.63966e-05 *begin_x*begin_y + 0.00331265 *begin_y*begin_dx*begin_lambda + 2.32494e-05 *lens_ipow(begin_y, 3)*begin_dx + 0.0029981 *lens_ipow(begin_y, 2)*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -1.11305e-11 *lens_ipow(begin_x, 7)*begin_y + 2.31933e-08 *begin_x*lens_ipow(begin_y, 5)*lens_ipow(begin_lambda, 2) + 0.000151007 *begin_x*lens_ipow(begin_y, 2)*begin_dy*lens_ipow(begin_lambda, 4) + 6.77497e-07 *begin_x*lens_ipow(begin_y, 4)*begin_dy*lens_ipow(begin_lambda, 2) + 2.68941e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*lens_ipow(begin_lambda, 2) + -0.0137627 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 3) + -1.70113e-10 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 5)*begin_lambda + -1.18513e-10 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 3)*begin_lambda + 1.82113e-07 *lens_ipow(begin_y, 5)*begin_dx*lens_ipow(begin_lambda, 4) + -8.98691e-11 *begin_x*lens_ipow(begin_y, 7)*lens_ipow(begin_lambda, 2) + 4.9861e-05 *begin_x*lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 4)+0.0f;
          domega2_dx0[1][0] =  + 0.0529577 *begin_dx*begin_dy + 0.000225211 *begin_x*begin_y + 0.0026377 *begin_x*begin_dy + 0.00206368 *begin_y*begin_dx*begin_lambda + 0.0544768 *begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -0.0347859 *lens_ipow(begin_dx, 3)*begin_dy + 7.50802e-05 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 3)*begin_lambda + 6.09562e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -1.18579e-11 *begin_x*lens_ipow(begin_y, 7)*begin_lambda + -2.93642e-13 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 3) + -1.91145e-13 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 7)+0.0f;
          domega2_dx0[1][1] =  + -0.0812351  + 0.0140781 *begin_lambda + 0.0066131 *begin_y*begin_dy + 0.106182 *lens_ipow(begin_dy, 2) + -0.00852811 *lens_ipow(begin_lambda, 2) + 0.000112606 *lens_ipow(begin_x, 2) + 0.000267681 *lens_ipow(begin_y, 2) + 0.00206368 *begin_x*begin_dx*begin_lambda + 0.114422 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + 0.00723471 *begin_y*lens_ipow(begin_dx, 2)*begin_dy + -0.0311424 *lens_ipow(begin_dy, 4) + 0.000280172 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + -0.0805241 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 3) + 4.06375e-06 *lens_ipow(begin_x, 3)*begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -9.22309e-12 *lens_ipow(begin_y, 8)*begin_lambda + -4.15026e-11 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 6)*begin_lambda + -3.11856e-14 *lens_ipow(begin_y, 10) + -1.10116e-13 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 2) + -3.34504e-13 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 6)+0.0f;
          float invJ[2][2];
          const float invdet = 1.0f/(domega2_dx0[0][0]*domega2_dx0[1][1] - domega2_dx0[0][1]*domega2_dx0[1][0]);
          invJ[0][0] =  domega2_dx0[1][1]*invdet;
          invJ[1][1] =  domega2_dx0[0][0]*invdet;
          invJ[0][1] = -domega2_dx0[0][1]*invdet;
          invJ[1][0] = -domega2_dx0[1][0]*invdet;
          for(int i=0;i<2;i++)
          {
            x += 0.72 * invJ[0][i]*delta_out[i];
            y += 0.72 * invJ[1][i]*delta_out[i];
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
        out[4] =  + 0.829835 *begin_lambda + -1.08215 *lens_ipow(begin_lambda, 2) + 0.503594 *lens_ipow(begin_lambda, 3) + 0.0145961 *begin_y*lens_ipow(begin_dy, 3) + -0.568749 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + -2.16067e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -3.82145e-05 *lens_ipow(begin_y, 3)*begin_dy + -1.76453e-06 *lens_ipow(begin_x, 4)*begin_lambda + -5.34716e-07 *lens_ipow(begin_y, 4)*begin_lambda + 0.00280576 *begin_x*begin_y*begin_dx*begin_dy*begin_lambda + -0.793914 *lens_ipow(begin_dx, 6) + -2.16535e-08 *lens_ipow(begin_y, 6) + -4.93127e-07 *lens_ipow(begin_x, 5)*begin_dx*begin_lambda + -4.54478e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*begin_dx + 1.46013e-10 *lens_ipow(begin_y, 8) + -2.45372e-11 *lens_ipow(begin_x, 8) + -0.0114921 *begin_x*begin_y*lens_ipow(begin_dx, 3)*begin_dy*lens_ipow(begin_lambda, 2) + -7.97349e-09 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3)*begin_dy*begin_lambda + -0.0138635 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 3) + -3.07018e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dx*begin_dy*begin_lambda + 1.30754e-10 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 6)*begin_lambda + -3.74594e-13 *lens_ipow(begin_y, 10) + -5.66313e-05 *lens_ipow(begin_x, 3)*begin_dx*lens_ipow(begin_lambda, 6) + -1.20621e-12 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 6) + -6.28574e-08 *lens_ipow(begin_x, 6)*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -2.57329e-13 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 2) + -1.16038e-12 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 8)*begin_lambda + -2.06616e-12 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 4)*begin_lambda;
      else
        out[4] = 0.0f;
    } break;


    case fisheye:
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
             + 46.3231 *begin_dx + 0.713762 *begin_x + -0.203224 *begin_x*begin_lambda + -8.49003 *begin_dx*begin_lambda + 1.48432 *begin_y*begin_dx*begin_dy + 0.136162 *begin_x*lens_ipow(begin_lambda, 2) + 19.351 *begin_dx*lens_ipow(begin_dy, 2) + 0.000222426 *lens_ipow(begin_x, 3) + 0.0178375 *lens_ipow(begin_y, 2)*begin_dx + 6.06598 *begin_dx*lens_ipow(begin_lambda, 2) + 6.93669 *begin_x*lens_ipow(begin_dx, 2)*begin_lambda + 78.5921 *lens_ipow(begin_dx, 3)*begin_lambda + 0.657095 *begin_x*lens_ipow(begin_dy, 2)*begin_lambda + 0.00080037 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + 0.0599381 *begin_x*begin_y*begin_dy*begin_lambda + 0.152641 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + -78.7821 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + -0.000697152 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2) + -6.68444 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -0.14429 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_lambda, 2) + -0.0622761 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 3) + -1.17139 *begin_x*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 5) + 241.074 *begin_y*lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 3) + 438829 *lens_ipow(begin_dx, 9)*lens_ipow(begin_dy, 2) + 41684.5 *begin_x*lens_ipow(begin_dx, 8)*lens_ipow(begin_dy, 2) + 14.6511 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 6)*lens_ipow(begin_dy, 2) + 57564.4 *lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 6) + 1344.32 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 7)*lens_ipow(begin_dy, 2),
             + 0.711668 *begin_y + 46.3848 *begin_dy + -8.46928 *begin_dy*begin_lambda + -0.193319 *begin_y*begin_lambda + 0.286513 *begin_y*lens_ipow(begin_dx, 2) + 1.47114 *begin_x*begin_dx*begin_dy + 0.0219261 *begin_x*begin_y*begin_dx + 18.7219 *lens_ipow(begin_dx, 2)*begin_dy + 0.128495 *begin_y*lens_ipow(begin_lambda, 2) + 0.000225772 *lens_ipow(begin_x, 2)*begin_y + 6.05942 *begin_dy*lens_ipow(begin_lambda, 2) + 0.000205517 *lens_ipow(begin_y, 3) + 0.0176433 *lens_ipow(begin_x, 2)*begin_dy + 6.17418 *begin_y*lens_ipow(begin_dy, 2)*begin_lambda + 63.0907 *lens_ipow(begin_dy, 3)*begin_lambda + 0.140338 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + -0.132736 *lens_ipow(begin_y, 2)*begin_dy*lens_ipow(begin_lambda, 2) + -64.8887 *lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + -5.97842 *begin_y*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 11183.1 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 5) + 396.191 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 7) + 42860.5 *lens_ipow(begin_dy, 9) + 0.0130309 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + 6467.85 *begin_y*lens_ipow(begin_dy, 8) + 0.189225 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 5) + 0.00117308 *lens_ipow(begin_y, 5)*lens_ipow(begin_dy, 4) + 12.2293 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 6) + -24.246 *begin_x*begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 4)
          };
          const float delta_ap[] = {ap_x - pred_ap[0], ap_y - pred_ap[1]};
          sqr_ap_err = delta_ap[0]*delta_ap[0]+delta_ap[1]*delta_ap[1];
          float dx1_domega0[2][2];
          dx1_domega0[0][0] =  + 46.3231  + -8.49003 *begin_lambda + 1.48432 *begin_y*begin_dy + 19.351 *lens_ipow(begin_dy, 2) + 0.0178375 *lens_ipow(begin_y, 2) + 6.06598 *lens_ipow(begin_lambda, 2) + 13.8734 *begin_x*begin_dx*begin_lambda + 235.776 *lens_ipow(begin_dx, 2)*begin_lambda + 0.152641 *lens_ipow(begin_x, 2)*begin_lambda + -236.346 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -13.3689 *begin_x*begin_dx*lens_ipow(begin_lambda, 2) + -0.14429 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 2) + 1205.37 *begin_y*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 3) + 3.94946e+06 *lens_ipow(begin_dx, 8)*lens_ipow(begin_dy, 2) + 333476 *begin_x*lens_ipow(begin_dx, 7)*lens_ipow(begin_dy, 2) + 87.9067 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 2) + 287822 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 6) + 9410.21 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 6)*lens_ipow(begin_dy, 2)+0.0f;
          dx1_domega0[0][1] =  + 1.48432 *begin_y*begin_dx + 38.7021 *begin_dx*begin_dy + 1.31419 *begin_x*begin_dy*begin_lambda + 0.0599381 *begin_x*begin_y*begin_lambda + -0.0622761 *begin_x*begin_y*lens_ipow(begin_lambda, 3) + -2.34278 *begin_x*begin_dy*lens_ipow(begin_lambda, 5) + 723.221 *begin_y*lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 2) + 877658 *lens_ipow(begin_dx, 9)*begin_dy + 83369 *begin_x*lens_ipow(begin_dx, 8)*begin_dy + 29.3022 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 6)*begin_dy + 345386 *lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 5) + 2688.63 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 7)*begin_dy+0.0f;
          dx1_domega0[1][0] =  + 0.573025 *begin_y*begin_dx + 1.47114 *begin_x*begin_dy + 0.0219261 *begin_x*begin_y + 37.4438 *begin_dx*begin_dy + 44732.5 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 5) + 0.0260617 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 3) + -72.738 *begin_x*begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 4)+0.0f;
          dx1_domega0[1][1] =  + 46.3848  + -8.46928 *begin_lambda + 1.47114 *begin_x*begin_dx + 18.7219 *lens_ipow(begin_dx, 2) + 6.05942 *lens_ipow(begin_lambda, 2) + 0.0176433 *lens_ipow(begin_x, 2) + 12.3484 *begin_y*begin_dy*begin_lambda + 189.272 *lens_ipow(begin_dy, 2)*begin_lambda + 0.140338 *lens_ipow(begin_y, 2)*begin_lambda + -0.132736 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2) + -194.666 *lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + -11.9568 *begin_y*begin_dy*lens_ipow(begin_lambda, 2) + 55915.7 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 4) + 2773.33 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 6) + 385744 *lens_ipow(begin_dy, 8) + 0.0390926 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 51742.8 *begin_y*lens_ipow(begin_dy, 7) + 0.946123 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 4) + 0.00469234 *lens_ipow(begin_y, 5)*lens_ipow(begin_dy, 3) + 73.3758 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 5) + -96.9839 *begin_x*begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3)+0.0f;
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
          out[0] =  + -8.23478 *begin_x + -23.6448 *begin_dx*begin_lambda + 0.209775 *lens_ipow(begin_x, 2)*begin_dx + -0.00567908 *begin_x*lens_ipow(begin_y, 2) + 7.19223 *begin_x*lens_ipow(begin_dy, 2) + -0.0100983 *lens_ipow(begin_x, 3)*begin_lambda + 0.302739 *begin_x*begin_y*begin_dy*begin_lambda + 1207.94 *begin_x*lens_ipow(begin_dx, 4) + 31.3281 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 3) + 10503.8 *lens_ipow(begin_dx, 5) + 0.350912 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2) + 8722.78 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2) + 1038.42 *begin_y*lens_ipow(begin_dx, 3)*begin_dy*begin_lambda + 3375.71 *begin_dx*lens_ipow(begin_dy, 4)*begin_lambda + -0.000607626 *lens_ipow(begin_x, 4)*begin_y*begin_dx*begin_dy + -1941.37 *begin_x*lens_ipow(begin_dx, 6) + 18.8515 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + -8484.66 *begin_y*lens_ipow(begin_dx, 5)*begin_dy*begin_lambda + 14101.2 *begin_y*lens_ipow(begin_dx, 7)*begin_dy + -157.75 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 5)*lens_ipow(begin_lambda, 3) + 987.563 *begin_x*lens_ipow(begin_dx, 8)*begin_dy + 1.10767e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_lambda + 56171.5 *begin_x*lens_ipow(begin_dx, 8)*lens_ipow(begin_dy, 2) + 1.62373e-08 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2) + 12.4767 *lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_dy, 7) + -3.06799 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 4) + -958.675 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 9) + -6.1304 *begin_x*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 3);
          out[1] =  + -8.37596 *begin_y + -14.0792 *begin_dy + 8.14436 *begin_y*lens_ipow(begin_dx, 2) + 0.642083 *lens_ipow(begin_y, 2)*begin_dy + 30.1401 *begin_y*lens_ipow(begin_dy, 2) + 4.61396 *begin_x*begin_dx*begin_dy + 0.213958 *begin_x*begin_y*begin_dx + -0.00395195 *lens_ipow(begin_x, 2)*begin_y + 294.206 *lens_ipow(begin_dy, 3) + -0.00520918 *lens_ipow(begin_y, 3)*begin_lambda + 9648.97 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + 374.136 *begin_x*begin_dx*lens_ipow(begin_dy, 3) + 6.27376 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 3) + 2263.15 *lens_ipow(begin_dx, 4)*begin_dy + -1.22252e-05 *lens_ipow(begin_y, 6)*begin_dy + 41765.7 *lens_ipow(begin_dy, 7) + 1.13934 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dy, 4) + 1751.59 *begin_y*lens_ipow(begin_dy, 6) + -1.31667 *begin_x*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 3) + 0.555329 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 5) + -0.0200994 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + 10.3339 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 7)*begin_dy + -3.3773 *lens_ipow(begin_x, 3)*begin_y*lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 2) + -3.27086 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 5) + -2.40173 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 7) + 1.06144e-10 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 7) + -0.000123388 *begin_x*lens_ipow(begin_y, 6)*begin_dx*lens_ipow(begin_dy, 3) + 0.379743 *begin_x*lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3);
          out[2] =  + -0.103083 *begin_x + -0.601438 *begin_dx*begin_lambda + 0.234038 *begin_y*begin_dx*begin_dy + 0.00352225 *begin_x*begin_y*begin_dy + 5.38731 *begin_dx*lens_ipow(begin_dy, 2) + 4.57178e-05 *begin_x*lens_ipow(begin_y, 2) + 0.000216091 *lens_ipow(begin_x, 3) + 0.107377 *begin_x*lens_ipow(begin_dy, 2) + 0.852366 *begin_x*lens_ipow(begin_dx, 2)*begin_lambda + 0.0184197 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + 0.0040717 *lens_ipow(begin_y, 2)*begin_dx*begin_lambda + 17.8917 *lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + -0.000423037 *lens_ipow(begin_x, 2)*begin_y*begin_dx*begin_dy + -0.0197977 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_lambda, 3) + 71.8647 *lens_ipow(begin_dx, 5)*begin_lambda + 9.52493e-05 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 3) + -0.00668964 *lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_dy, 3) + 0.000421438 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 3) + -4.09222e-09 *begin_x*lens_ipow(begin_y, 6) + -1.25617 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 4) + 455.971 *lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 2) + -0.000581608 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 3)*begin_lambda + -492.467 *lens_ipow(begin_dx, 5)*lens_ipow(begin_lambda, 4) + 132.875 *begin_x*lens_ipow(begin_dx, 6)*lens_ipow(begin_dy, 2) + 61.7322 *begin_x*lens_ipow(begin_dx, 8)*begin_lambda + 1828.09 *lens_ipow(begin_dx, 7)*lens_ipow(begin_lambda, 4) + -1.09217e-05 *lens_ipow(begin_y, 6)*begin_dx*lens_ipow(begin_dy, 4) + 0.00256169 *begin_x*lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2);
          out[3] =  + -0.103601 *begin_y + -0.713373 *begin_dy*begin_lambda + 0.000378384 *lens_ipow(begin_x, 2)*begin_y + 0.000247393 *lens_ipow(begin_y, 3) + 0.129964 *begin_y*lens_ipow(begin_dx, 2)*begin_lambda + 0.00402109 *lens_ipow(begin_x, 2)*begin_dy*begin_lambda + 0.0165033 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + 0.00729411 *begin_x*begin_y*begin_dx*begin_lambda + 0.693063 *begin_x*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 0.409198 *begin_dy*lens_ipow(begin_lambda, 4) + 32.9633 *lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + -1.96139e-07 *lens_ipow(begin_y, 5) + 18.4048 *lens_ipow(begin_dx, 2)*begin_dy*lens_ipow(begin_lambda, 2) + 2.80892 *begin_y*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.563506 *begin_y*lens_ipow(begin_dx, 4)*begin_lambda + -0.0130565 *lens_ipow(begin_y, 2)*begin_dy*lens_ipow(begin_lambda, 3) + 7.74914e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_lambda + -3.11538 *begin_y*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 3) + -44.8493 *lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 4) + 0.00454339 *lens_ipow(begin_x, 2)*begin_dy*lens_ipow(begin_lambda, 4) + 0.00015627 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_lambda, 5) + 10.8648 *begin_x*begin_dx*lens_ipow(begin_dy, 5)*lens_ipow(begin_lambda, 2) + -16.3559 *begin_y*lens_ipow(begin_dy, 8) + 0.509567 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 6) + -0.000484984 *lens_ipow(begin_y, 5)*lens_ipow(begin_dy, 6) + -0.0186708 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 7) + -7.7512e-10 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + 12.1926 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 6);
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
          domega2_dx0[0][0] =  + -0.103083  + 0.00352225 *begin_y*begin_dy + 4.57178e-05 *lens_ipow(begin_y, 2) + 0.000648272 *lens_ipow(begin_x, 2) + 0.107377 *lens_ipow(begin_dy, 2) + 0.852366 *lens_ipow(begin_dx, 2)*begin_lambda + 0.0368395 *begin_x*begin_dx*begin_lambda + -0.000846074 *begin_x*begin_y*begin_dx*begin_dy + -0.0395955 *begin_x*begin_dx*lens_ipow(begin_lambda, 3) + 0.000380997 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 3) + -4.09222e-09 *lens_ipow(begin_y, 6) + -1.25617 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 4) + 132.875 *lens_ipow(begin_dx, 6)*lens_ipow(begin_dy, 2) + 61.7322 *lens_ipow(begin_dx, 8)*begin_lambda + 0.00256169 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2)+0.0f;
          domega2_dx0[0][1] =  + 0.234038 *begin_dx*begin_dy + 0.00352225 *begin_x*begin_dy + 9.14357e-05 *begin_x*begin_y + 0.00814339 *begin_y*begin_dx*begin_lambda + -0.000423037 *lens_ipow(begin_x, 2)*begin_dx*begin_dy + -0.0200689 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 3) + 0.00168575 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 3) + -2.45533e-08 *begin_x*lens_ipow(begin_y, 5) + -0.00232643 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 3)*begin_lambda + -6.55305e-05 *lens_ipow(begin_y, 5)*begin_dx*lens_ipow(begin_dy, 4) + 0.0102467 *begin_x*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2)+0.0f;
          domega2_dx0[1][0] =  + 0.000756768 *begin_x*begin_y + 0.00804217 *begin_x*begin_dy*begin_lambda + 0.00729411 *begin_y*begin_dx*begin_lambda + 0.693063 *begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 1.54983e-06 *begin_x*lens_ipow(begin_y, 3)*begin_lambda + 0.00908677 *begin_x*begin_dy*lens_ipow(begin_lambda, 4) + 0.00031254 *begin_x*begin_y*lens_ipow(begin_lambda, 5) + 10.8648 *begin_dx*lens_ipow(begin_dy, 5)*lens_ipow(begin_lambda, 2) + 0.509567 *begin_y*begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 6) + -4.65072e-09 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2)+0.0f;
          domega2_dx0[1][1] =  + -0.103601  + 0.000378384 *lens_ipow(begin_x, 2) + 0.000742179 *lens_ipow(begin_y, 2) + 0.129964 *lens_ipow(begin_dx, 2)*begin_lambda + 0.0330067 *begin_y*begin_dy*begin_lambda + 0.00729411 *begin_x*begin_dx*begin_lambda + -9.80694e-07 *lens_ipow(begin_y, 4) + 2.80892 *lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.563506 *lens_ipow(begin_dx, 4)*begin_lambda + -0.026113 *begin_y*begin_dy*lens_ipow(begin_lambda, 3) + 2.32474e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_lambda + -3.11538 *lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 3) + 0.00015627 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 5) + -16.3559 *lens_ipow(begin_dy, 8) + 0.509567 *begin_x*begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 6) + -0.00242492 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 6) + -0.0746833 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 7) + -2.32536e-09 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + 12.1926 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 6)+0.0f;
          float invJ[2][2];
          const float invdet = 1.0f/(domega2_dx0[0][0]*domega2_dx0[1][1] - domega2_dx0[0][1]*domega2_dx0[1][0]);
          invJ[0][0] =  domega2_dx0[1][1]*invdet;
          invJ[1][1] =  domega2_dx0[0][0]*invdet;
          invJ[0][1] = -domega2_dx0[0][1]*invdet;
          invJ[1][0] = -domega2_dx0[1][0]*invdet;
          for(int i=0;i<2;i++)
          {
            x += 0.72 * invJ[0][i]*delta_out[i];
            y += 0.72 * invJ[1][i]*delta_out[i];
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
        out[4] =  + 0.196001  + 0.378559 *begin_lambda + 0.128762 *lens_ipow(begin_dy, 2) + -0.267516 *lens_ipow(begin_lambda, 2) + -3.43289 *lens_ipow(begin_dy, 4) + -5.30271e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -0.00124765 *begin_x*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 2) + -1.33218e-08 *lens_ipow(begin_x, 6) + -1.27157e-08 *lens_ipow(begin_y, 6) + -331.504 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2) + -22.8135 *begin_y*lens_ipow(begin_dy, 7) + -0.435257 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 6) + 4657.45 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 4) + -0.644826 *begin_x*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 4) + 4717.32 *lens_ipow(begin_dx, 6)*lens_ipow(begin_dy, 2) + 7.77492 *begin_x*lens_ipow(begin_dx, 5)*lens_ipow(begin_lambda, 2) + 129.97 *lens_ipow(begin_dy, 8)*begin_lambda + -44.8792 *begin_x*lens_ipow(begin_dx, 7)*begin_lambda + -2092.11 *lens_ipow(begin_dy, 10) + -3144.49 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 8) + -20701.6 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 6) + -42660.6 *lens_ipow(begin_dx, 6)*lens_ipow(begin_dy, 4) + -19041.8 *lens_ipow(begin_dx, 8)*lens_ipow(begin_dy, 2) + -1799.27 *lens_ipow(begin_dx, 10) + 31.9804 *begin_x*begin_y*lens_ipow(begin_dx, 5)*lens_ipow(begin_dy, 3)*begin_lambda + 0.0626751 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 7)*begin_lambda + 5.56252e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + -0.0101147 *begin_x*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 5)*begin_dy*begin_lambda;
      else
        out[4] = 0.0f;
    } break;


    case wideangle:
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
             + 33.436 *begin_dx + 0.620268 *begin_x + -0.0997683 *begin_x*begin_lambda + -1.02423 *begin_dx*begin_lambda + 0.0779104 *begin_x*begin_y*begin_dy + 0.11032 *lens_ipow(begin_x, 2)*begin_dx + 22.3258 *lens_ipow(begin_dx, 3) + 0.066893 *begin_x*lens_ipow(begin_lambda, 2) + 28.7753 *begin_dx*lens_ipow(begin_dy, 2) + 0.00114721 *begin_x*lens_ipow(begin_y, 2) + 0.00104281 *lens_ipow(begin_x, 3) + 1.05782 *begin_x*lens_ipow(begin_dy, 2) + 3.30806 *begin_x*lens_ipow(begin_dx, 2) + 0.622831 *begin_dx*lens_ipow(begin_lambda, 3) + 0.252479 *lens_ipow(begin_y, 2)*begin_dx*begin_lambda + 15.3869 *begin_y*begin_dx*begin_dy*begin_lambda + 0.000265991 *lens_ipow(begin_x, 4)*begin_dx + 17.5869 *begin_x*lens_ipow(begin_dx, 4) + 0.849033 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 3) + -28.4185 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + 147.745 *lens_ipow(begin_dx, 5) + 1.40136e-06 *lens_ipow(begin_x, 5) + -0.469428 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_lambda, 2) + 0.020917 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2) + 17.1989 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 3) + 0.285723 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_lambda, 3) + 4.60929e-13 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 4) + -3.19292e-09 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dx*begin_dy,
             + 0.620382 *begin_y + 33.2697 *begin_dy + -0.0988428 *begin_y*begin_lambda + 1.03903 *begin_y*lens_ipow(begin_dx, 2) + 0.108851 *lens_ipow(begin_y, 2)*begin_dy + 3.26034 *begin_y*lens_ipow(begin_dy, 2) + 2.71107 *begin_x*begin_dx*begin_dy + 0.0773918 *begin_x*begin_y*begin_dx + 28.2928 *lens_ipow(begin_dx, 2)*begin_dy + 0.0661776 *begin_y*lens_ipow(begin_lambda, 2) + 0.00113968 *lens_ipow(begin_x, 2)*begin_y + -1.94549 *begin_dy*lens_ipow(begin_lambda, 2) + 0.00102739 *lens_ipow(begin_y, 3) + 0.0442643 *lens_ipow(begin_x, 2)*begin_dy + 21.7286 *lens_ipow(begin_dy, 3) + 1.83525 *begin_dy*lens_ipow(begin_lambda, 3) + 0.000304092 *lens_ipow(begin_y, 4)*begin_dy + 156.877 *lens_ipow(begin_dy, 5) + 0.920891 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 3) + 18.733 *begin_y*lens_ipow(begin_dy, 4) + 1.5905e-06 *lens_ipow(begin_y, 5) + 0.0233247 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2) + 9.47641e-06 *lens_ipow(begin_y, 5)*lens_ipow(begin_dx, 2) + 0.000414172 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*begin_dy + 7.77217e-09 *begin_x*lens_ipow(begin_y, 7)*begin_dx*begin_lambda + 3.2448e-13 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 9) + 2.5744e-13 *lens_ipow(begin_x, 8)*lens_ipow(begin_y, 3) + 6.2086e-07 *begin_x*lens_ipow(begin_y, 6)*begin_dx*begin_dy*lens_ipow(begin_lambda, 2)
          };
          const float delta_ap[] = {ap_x - pred_ap[0], ap_y - pred_ap[1]};
          sqr_ap_err = delta_ap[0]*delta_ap[0]+delta_ap[1]*delta_ap[1];
          float dx1_domega0[2][2];
          dx1_domega0[0][0] =  + 33.436  + -1.02423 *begin_lambda + 0.11032 *lens_ipow(begin_x, 2) + 66.9774 *lens_ipow(begin_dx, 2) + 28.7753 *lens_ipow(begin_dy, 2) + 6.61613 *begin_x*begin_dx + 0.622831 *lens_ipow(begin_lambda, 3) + 0.252479 *lens_ipow(begin_y, 2)*begin_lambda + 15.3869 *begin_y*begin_dy*begin_lambda + 0.000265991 *lens_ipow(begin_x, 4) + 70.3475 *begin_x*lens_ipow(begin_dx, 3) + 2.5471 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + -28.4185 *begin_y*begin_dy*lens_ipow(begin_lambda, 2) + 738.725 *lens_ipow(begin_dx, 4) + -0.469428 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2) + 0.041834 *lens_ipow(begin_x, 3)*begin_dx + 17.1989 *begin_y*begin_dy*lens_ipow(begin_lambda, 3) + 0.285723 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 3) + -3.19292e-09 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dy+0.0f;
          dx1_domega0[0][1] =  + 0.0779104 *begin_x*begin_y + 57.5505 *begin_dx*begin_dy + 2.11564 *begin_x*begin_dy + 15.3869 *begin_y*begin_dx*begin_lambda + -28.4185 *begin_y*begin_dx*lens_ipow(begin_lambda, 2) + 17.1989 *begin_y*begin_dx*lens_ipow(begin_lambda, 3) + -3.19292e-09 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dx+0.0f;
          dx1_domega0[1][0] =  + 2.07807 *begin_y*begin_dx + 2.71107 *begin_x*begin_dy + 0.0773918 *begin_x*begin_y + 56.5856 *begin_dx*begin_dy + 1.89528e-05 *lens_ipow(begin_y, 5)*begin_dx + 0.000828343 *lens_ipow(begin_y, 4)*begin_dx*begin_dy + 7.77217e-09 *begin_x*lens_ipow(begin_y, 7)*begin_lambda + 6.2086e-07 *begin_x*lens_ipow(begin_y, 6)*begin_dy*lens_ipow(begin_lambda, 2)+0.0f;
          dx1_domega0[1][1] =  + 33.2697  + 0.108851 *lens_ipow(begin_y, 2) + 6.52068 *begin_y*begin_dy + 2.71107 *begin_x*begin_dx + 28.2928 *lens_ipow(begin_dx, 2) + -1.94549 *lens_ipow(begin_lambda, 2) + 0.0442643 *lens_ipow(begin_x, 2) + 65.1857 *lens_ipow(begin_dy, 2) + 1.83525 *lens_ipow(begin_lambda, 3) + 0.000304092 *lens_ipow(begin_y, 4) + 784.387 *lens_ipow(begin_dy, 4) + 2.76267 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + 74.9321 *begin_y*lens_ipow(begin_dy, 3) + 0.0466494 *lens_ipow(begin_y, 3)*begin_dy + 0.000414172 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2) + 6.2086e-07 *begin_x*lens_ipow(begin_y, 6)*begin_dx*lens_ipow(begin_lambda, 2)+0.0f;
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
          out[0] =  + 16.7222 *begin_dx + -1.7126 *begin_x + -0.407092 *begin_x*begin_lambda + -1.50065 *begin_dx*begin_lambda + 2.13945 *begin_y*begin_dx*begin_dy + 0.122543 *lens_ipow(begin_x, 2)*begin_dx + 25.686 *lens_ipow(begin_dx, 3) + 0.265989 *begin_x*lens_ipow(begin_lambda, 2) + 29.8535 *begin_dx*lens_ipow(begin_dy, 2) + 0.00320318 *begin_x*lens_ipow(begin_y, 2) + 0.00349548 *lens_ipow(begin_x, 3) + 1.05804 *begin_x*lens_ipow(begin_dy, 2) + 3.24433 *begin_x*lens_ipow(begin_dx, 2) + 0.0495496 *lens_ipow(begin_y, 2)*begin_dx + -1.2933 *begin_x*lens_ipow(begin_dx, 2)*begin_lambda + 0.30668 *begin_x*begin_y*begin_dy*begin_lambda + -0.83918 *begin_y*begin_dx*begin_dy*begin_lambda + -3.57407e-06 *lens_ipow(begin_x, 5) + 0.298459 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3) + -0.388491 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 2) + 0.0189128 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_lambda + -0.000272989 *lens_ipow(begin_x, 4)*begin_dx*begin_lambda + 0.000109252 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + -7.08814e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4) + 0.00565248 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 2) + 0.225298 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 5) + -0.112286 *begin_x*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 4)*begin_dy + -7.93361e-10 *lens_ipow(begin_x, 7)*lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 2);
          out[1] =  + -1.72441 *begin_y + 15.9053 *begin_dy + -0.377417 *begin_y*begin_lambda + 0.96701 *begin_y*lens_ipow(begin_dx, 2) + 0.174206 *lens_ipow(begin_y, 2)*begin_dy + 5.43202 *begin_y*lens_ipow(begin_dy, 2) + 1.53423 *begin_x*begin_dx*begin_dy + 26.636 *lens_ipow(begin_dx, 2)*begin_dy + 0.252523 *begin_y*lens_ipow(begin_lambda, 2) + 0.00320864 *lens_ipow(begin_x, 2)*begin_y + 0.0034628 *lens_ipow(begin_y, 3) + 0.0508916 *lens_ipow(begin_x, 2)*begin_dy + 61.5968 *lens_ipow(begin_dy, 3) + -5.08006 *begin_y*lens_ipow(begin_dy, 2)*begin_lambda + -60.4737 *lens_ipow(begin_dy, 3)*begin_lambda + -0.0976647 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + 0.234352 *begin_x*begin_y*begin_dx*begin_lambda + 0.00415282 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + -0.000238714 *lens_ipow(begin_y, 4)*begin_dy*begin_lambda + 0.36372 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + -6.27167e-06 *lens_ipow(begin_y, 5)*begin_lambda + -0.807132 *begin_x*begin_y*begin_dx*lens_ipow(begin_lambda, 3) + -0.00640517 *lens_ipow(begin_x, 3)*begin_y*begin_dx*lens_ipow(begin_dy, 2) + 0.722321 *begin_x*begin_y*begin_dx*lens_ipow(begin_lambda, 4) + -5.45787e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 5)*begin_lambda + -6.20086e-10 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3) + 0.0654266 *lens_ipow(begin_x, 4)*lens_ipow(begin_dy, 5) + 0.0037559 *lens_ipow(begin_x, 4)*begin_y*lens_ipow(begin_dy, 4)*begin_lambda;
          out[2] =  + -0.334003 *begin_dx + -0.026849 *begin_x + 0.0120662 *begin_x*begin_lambda + -0.0196292 *begin_dx*begin_lambda + 0.49947 *lens_ipow(begin_dx, 3) + -0.00827016 *begin_x*lens_ipow(begin_lambda, 2) + 0.435908 *begin_dx*lens_ipow(begin_dy, 2) + -1.03771e-05 *begin_x*lens_ipow(begin_y, 2) + 0.00186831 *begin_x*lens_ipow(begin_dy, 2) + 0.015388 *begin_x*lens_ipow(begin_dx, 2) + 0.00014192 *lens_ipow(begin_y, 2)*begin_dx + 9.35478e-05 *lens_ipow(begin_x, 3)*begin_lambda + -0.000392059 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + 0.0477328 *begin_y*begin_dx*begin_dy*begin_lambda + -0.0481053 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -0.000148114 *lens_ipow(begin_x, 3)*lens_ipow(begin_lambda, 2) + -0.0221637 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -4.44502e-08 *lens_ipow(begin_x, 5) + -0.00671039 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 2) + 0.0174403 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 3) + -1.96719e-06 *lens_ipow(begin_x, 3)*begin_y*begin_dy*begin_lambda + -2.82448e-06 *lens_ipow(begin_x, 4)*begin_dx*begin_lambda + 1.35905e-06 *begin_x*lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2) + 8.77094e-05 *lens_ipow(begin_x, 3)*lens_ipow(begin_lambda, 4) + 4.46548e-05 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 3) + -0.0124094 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 4) + 0.00219052 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 3)*begin_dy*begin_lambda + -2.70974 *lens_ipow(begin_dx, 5)*lens_ipow(begin_lambda, 3);
          out[3] =  + -0.0266125 *begin_y + -0.334066 *begin_dy + -0.0170558 *begin_dy*begin_lambda + 0.0114614 *begin_y*begin_lambda + -0.00511334 *begin_y*lens_ipow(begin_dx, 2) + 0.0129436 *begin_y*lens_ipow(begin_dy, 2) + 0.00896579 *begin_x*begin_dx*begin_dy + 0.477093 *lens_ipow(begin_dx, 2)*begin_dy + -0.00789395 *begin_y*lens_ipow(begin_lambda, 2) + 3.90402e-05 *lens_ipow(begin_x, 2)*begin_y + -0.000338784 *lens_ipow(begin_x, 2)*begin_dy + 0.457993 *lens_ipow(begin_dy, 3) + -0.000567731 *lens_ipow(begin_y, 2)*begin_dy*begin_lambda + 5.55763e-05 *lens_ipow(begin_y, 3)*begin_lambda + -2.51e-06 *begin_x*lens_ipow(begin_y, 3)*begin_dx + -1.24306e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + -1.92408e-06 *lens_ipow(begin_x, 3)*begin_y*begin_dx + -9.87323e-08 *lens_ipow(begin_x, 4)*begin_y + -2.58648e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dy + -0.0150597 *begin_y*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.00600884 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + -0.000216745 *lens_ipow(begin_y, 3)*lens_ipow(begin_lambda, 3) + -2.56526e-10 *lens_ipow(begin_y, 7) + 0.000199 *lens_ipow(begin_y, 3)*lens_ipow(begin_lambda, 4) + -0.00148407 *lens_ipow(begin_x, 3)*begin_y*begin_dx*lens_ipow(begin_dy, 4) + -7.61229e-11 *lens_ipow(begin_y, 8)*begin_dy + 0.00038575 *lens_ipow(begin_x, 4)*lens_ipow(begin_dy, 5) + 0.0287351 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2);
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
          domega2_dx0[0][0] =  + -0.026849  + 0.0120662 *begin_lambda + -0.00827016 *lens_ipow(begin_lambda, 2) + -1.03771e-05 *lens_ipow(begin_y, 2) + 0.00186831 *lens_ipow(begin_dy, 2) + 0.015388 *lens_ipow(begin_dx, 2) + 0.000280643 *lens_ipow(begin_x, 2)*begin_lambda + -0.000784118 *begin_x*begin_dx*begin_lambda + -0.000444342 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 2) + -0.0221637 *lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -2.22251e-07 *lens_ipow(begin_x, 4) + -0.00671039 *begin_y*begin_dy*lens_ipow(begin_lambda, 2) + 0.0174403 *begin_y*begin_dy*lens_ipow(begin_lambda, 3) + -5.90157e-06 *lens_ipow(begin_x, 2)*begin_y*begin_dy*begin_lambda + -1.12979e-05 *lens_ipow(begin_x, 3)*begin_dx*begin_lambda + 1.35905e-06 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2) + 0.000263128 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 4) + -0.0124094 *begin_y*begin_dy*lens_ipow(begin_lambda, 4)+0.0f;
          domega2_dx0[0][1] =  + -2.07541e-05 *begin_x*begin_y + 0.00028384 *begin_y*begin_dx + 0.0477328 *begin_dx*begin_dy*begin_lambda + -0.0481053 *begin_dx*begin_dy*lens_ipow(begin_lambda, 2) + -0.00671039 *begin_x*begin_dy*lens_ipow(begin_lambda, 2) + 0.0174403 *begin_x*begin_dy*lens_ipow(begin_lambda, 3) + -1.96719e-06 *lens_ipow(begin_x, 3)*begin_dy*begin_lambda + 5.43619e-06 *begin_x*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + 0.000178619 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 3) + -0.0124094 *begin_x*begin_dy*lens_ipow(begin_lambda, 4) + 0.00657155 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*begin_dy*begin_lambda+0.0f;
          domega2_dx0[1][0] =  + 0.00896579 *begin_dx*begin_dy + 7.80803e-05 *begin_x*begin_y + -0.000677568 *begin_x*begin_dy + -2.51e-06 *lens_ipow(begin_y, 3)*begin_dx + -2.48612e-07 *begin_x*lens_ipow(begin_y, 3) + -5.77225e-06 *lens_ipow(begin_x, 2)*begin_y*begin_dx + -3.94929e-07 *lens_ipow(begin_x, 3)*begin_y + -5.17297e-06 *begin_x*lens_ipow(begin_y, 2)*begin_dy + -0.00445222 *lens_ipow(begin_x, 2)*begin_y*begin_dx*lens_ipow(begin_dy, 4) + 0.001543 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 5)+0.0f;
          domega2_dx0[1][1] =  + -0.0266125  + 0.0114614 *begin_lambda + -0.00511334 *lens_ipow(begin_dx, 2) + 0.0129436 *lens_ipow(begin_dy, 2) + -0.00789395 *lens_ipow(begin_lambda, 2) + 3.90402e-05 *lens_ipow(begin_x, 2) + -0.00113546 *begin_y*begin_dy*begin_lambda + 0.000166729 *lens_ipow(begin_y, 2)*begin_lambda + -7.52999e-06 *begin_x*lens_ipow(begin_y, 2)*begin_dx + -3.72919e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -1.92408e-06 *lens_ipow(begin_x, 3)*begin_dx + -9.87323e-08 *lens_ipow(begin_x, 4) + -5.17297e-06 *lens_ipow(begin_x, 2)*begin_y*begin_dy + -0.0150597 *lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.0120177 *begin_y*lens_ipow(begin_dx, 2)*begin_dy*begin_lambda + -0.000650235 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 3) + -1.79568e-09 *lens_ipow(begin_y, 6) + 0.000596999 *lens_ipow(begin_y, 2)*lens_ipow(begin_lambda, 4) + -0.00148407 *lens_ipow(begin_x, 3)*begin_dx*lens_ipow(begin_dy, 4) + -6.08983e-10 *lens_ipow(begin_y, 7)*begin_dy + 0.0862054 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 2)+0.0f;
          float invJ[2][2];
          const float invdet = 1.0f/(domega2_dx0[0][0]*domega2_dx0[1][1] - domega2_dx0[0][1]*domega2_dx0[1][0]);
          invJ[0][0] =  domega2_dx0[1][1]*invdet;
          invJ[1][1] =  domega2_dx0[0][0]*invdet;
          invJ[0][1] = -domega2_dx0[0][1]*invdet;
          invJ[1][0] = -domega2_dx0[1][0]*invdet;
          for(int i=0;i<2;i++)
          {
            x += 0.72 * invJ[0][i]*delta_out[i];
            y += 0.72 * invJ[1][i]*delta_out[i];
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
        out[4] =  + 1.07795 *begin_lambda + -0.000434464 *begin_y*begin_dy + -1.13994e-05 *lens_ipow(begin_y, 2) + -1.80117 *lens_ipow(begin_lambda, 2) + 1.13999 *lens_ipow(begin_lambda, 3) + -0.000660056 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + -0.0815446 *begin_x*lens_ipow(begin_dx, 3) + -8.44238 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + -1.0738e-05 *lens_ipow(begin_x, 2)*begin_y*begin_dy + -0.00279317 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + -0.150544 *begin_y*lens_ipow(begin_dx, 2)*begin_dy + -3.27969e-05 *lens_ipow(begin_x, 3)*begin_dx + -0.160615 *begin_x*begin_dx*lens_ipow(begin_dy, 2) + -2.25778e-07 *lens_ipow(begin_x, 4) + -0.0025113 *begin_x*begin_y*begin_dx*begin_dy + -0.000994204 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + -2.06406e-05 *lens_ipow(begin_y, 4)*lens_ipow(begin_dy, 2) + -98.5938 *lens_ipow(begin_dx, 6) + -0.195942 *lens_ipow(begin_lambda, 6) + -2.20023 *begin_x*lens_ipow(begin_dx, 5) + -0.00302466 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 3) + -87.6123 *lens_ipow(begin_dy, 6) + -0.179227 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 4) + -5.59084 *begin_y*lens_ipow(begin_dy, 5) + -0.318447 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 6) + -1415.1 *lens_ipow(begin_dx, 4)*lens_ipow(begin_dy, 4) + 394.675 *lens_ipow(begin_dx, 8) + 0.671785 *begin_x*begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3);
      else
        out[4] = 0.0f;
    } break;


    case takumar_1969_50mm:
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
             + 30.4517 *begin_dx + 0.335491 *begin_x + 0.189788 *begin_x*begin_lambda + 5.85562 *begin_dx*begin_lambda + -0.141897 *begin_y*begin_dx*begin_dy + 0.00157184 *begin_x*begin_y*begin_dy + 0.0036529 *lens_ipow(begin_x, 2)*begin_dx + -18.0466 *lens_ipow(begin_dx, 3) + -0.139713 *begin_x*lens_ipow(begin_lambda, 2) + -17.5218 *begin_dx*lens_ipow(begin_dy, 2) + -0.000157628 *begin_x*lens_ipow(begin_y, 2) + -0.000149439 *lens_ipow(begin_x, 3) + -0.164804 *begin_x*lens_ipow(begin_dy, 2) + -0.277242 *begin_x*lens_ipow(begin_dx, 2) + -4.06094 *begin_dx*lens_ipow(begin_lambda, 2) + 8.19514e-05 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + 6.17978e-05 *lens_ipow(begin_x, 3)*begin_lambda + 0.00342147 *lens_ipow(begin_y, 2)*begin_dx*begin_lambda + -4.00211e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2) + -1.93383e-07 *lens_ipow(begin_x, 5) + -2.14205e-07 *begin_x*lens_ipow(begin_y, 4) + 0.243286 *begin_x*lens_ipow(begin_dy, 4) + -0.0124942 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 2) + 5.92223 *lens_ipow(begin_dx, 5)*begin_lambda + 0.148547 *begin_x*begin_y*lens_ipow(begin_dx, 4)*begin_dy + -0.0244494 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + -43.3246 *begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 5) + -3.99556e-11 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dx*begin_dy,
             + 0.333768 *begin_y + 30.3142 *begin_dy + 6.19823 *begin_dy*begin_lambda + 0.194459 *begin_y*begin_lambda + -0.16036 *begin_y*lens_ipow(begin_dx, 2) + 0.00366671 *lens_ipow(begin_y, 2)*begin_dy + -0.277416 *begin_y*lens_ipow(begin_dy, 2) + -0.132609 *begin_x*begin_dx*begin_dy + 0.00165636 *begin_x*begin_y*begin_dx + -17.3693 *lens_ipow(begin_dx, 2)*begin_dy + -0.142883 *begin_y*lens_ipow(begin_lambda, 2) + -0.000147929 *lens_ipow(begin_x, 2)*begin_y + -4.24615 *begin_dy*lens_ipow(begin_lambda, 2) + -0.000147913 *lens_ipow(begin_y, 3) + 0.00203755 *lens_ipow(begin_x, 2)*begin_dy + -18.0699 *lens_ipow(begin_dy, 3) + 6.06488e-05 *lens_ipow(begin_y, 3)*begin_lambda + 6.63746e-05 *lens_ipow(begin_x, 2)*begin_y*begin_lambda + 0.224708 *begin_y*lens_ipow(begin_dx, 4) + 3.54072 *lens_ipow(begin_dy, 5) + -0.0124248 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_dy + -4.00944e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + -2.13743e-07 *lens_ipow(begin_x, 4)*begin_y + -1.95879e-07 *lens_ipow(begin_y, 5) + -0.0087466 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 3) + 0.141742 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 4) + -4.65243 *begin_x*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3) + -4.04516e-11 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_dx*begin_dy
          };
          const float delta_ap[] = {ap_x - pred_ap[0], ap_y - pred_ap[1]};
          sqr_ap_err = delta_ap[0]*delta_ap[0]+delta_ap[1]*delta_ap[1];
          float dx1_domega0[2][2];
          dx1_domega0[0][0] =  + 30.4517  + 5.85562 *begin_lambda + -0.141897 *begin_y*begin_dy + 0.0036529 *lens_ipow(begin_x, 2) + -54.1398 *lens_ipow(begin_dx, 2) + -17.5218 *lens_ipow(begin_dy, 2) + -0.554483 *begin_x*begin_dx + -4.06094 *lens_ipow(begin_lambda, 2) + 0.00342147 *lens_ipow(begin_y, 2)*begin_lambda + -0.0124942 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + 29.6111 *lens_ipow(begin_dx, 4)*begin_lambda + 0.594187 *begin_x*begin_y*lens_ipow(begin_dx, 3)*begin_dy + -0.0733483 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -129.974 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 5) + -3.99556e-11 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dy+0.0f;
          dx1_domega0[0][1] =  + -0.141897 *begin_y*begin_dx + 0.00157184 *begin_x*begin_y + -35.0437 *begin_dx*begin_dy + -0.329608 *begin_x*begin_dy + 0.973146 *begin_x*lens_ipow(begin_dy, 3) + -0.0249884 *lens_ipow(begin_x, 2)*begin_dx*begin_dy + 0.148547 *begin_x*begin_y*lens_ipow(begin_dx, 4) + -216.623 *begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 4) + -3.99556e-11 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 3)*begin_dx+0.0f;
          dx1_domega0[1][0] =  + -0.32072 *begin_y*begin_dx + -0.132609 *begin_x*begin_dy + 0.00165636 *begin_x*begin_y + -34.7386 *begin_dx*begin_dy + 0.898834 *begin_y*lens_ipow(begin_dx, 3) + -0.0248497 *lens_ipow(begin_y, 2)*begin_dx*begin_dy + 0.141742 *begin_x*begin_y*lens_ipow(begin_dy, 4) + -13.9573 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + -4.04516e-11 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_dy+0.0f;
          dx1_domega0[1][1] =  + 30.3142  + 6.19823 *begin_lambda + 0.00366671 *lens_ipow(begin_y, 2) + -0.554831 *begin_y*begin_dy + -0.132609 *begin_x*begin_dx + -17.3693 *lens_ipow(begin_dx, 2) + -4.24615 *lens_ipow(begin_lambda, 2) + 0.00203755 *lens_ipow(begin_x, 2) + -54.2097 *lens_ipow(begin_dy, 2) + 17.7036 *lens_ipow(begin_dy, 4) + -0.0124248 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + -0.0262398 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + 0.566968 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 3) + -13.9573 *begin_x*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 2) + -4.04516e-11 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_dx+0.0f;
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
          out[0] =  + 49.8032 *begin_dx + 0.0143226 *begin_x*begin_y*begin_dy + 0.0149205 *lens_ipow(begin_x, 2)*begin_dx + -20.8277 *lens_ipow(begin_dx, 3) + -21.2379 *begin_dx*lens_ipow(begin_dy, 2) + -0.000546669 *begin_x*lens_ipow(begin_y, 2) + -0.00103006 *lens_ipow(begin_x, 3) + 0.300221 *begin_x*lens_ipow(begin_dx, 2) + 0.00687478 *lens_ipow(begin_y, 2)*begin_dx + 0.556502 *begin_x*lens_ipow(begin_dx, 2)*begin_lambda + 1.14168 *begin_x*lens_ipow(begin_dy, 2)*begin_lambda + 0.000347782 *begin_x*lens_ipow(begin_y, 2)*begin_lambda + 0.00191234 *lens_ipow(begin_x, 3)*begin_lambda + 0.0160337 *lens_ipow(begin_x, 2)*begin_dx*begin_lambda + -0.00119575 *lens_ipow(begin_x, 3)*lens_ipow(begin_lambda, 2) + 0.00142374 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 2) + -0.632997 *begin_x*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.000748107 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + -0.00165018 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 2)*begin_lambda + -3.27465e-05 *lens_ipow(begin_x, 4)*begin_dx*lens_ipow(begin_lambda, 2) + -1.81354e-09 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 2) + -7.07079e-10 *begin_x*lens_ipow(begin_y, 6) + -1.95349 *begin_x*lens_ipow(begin_dx, 4)*lens_ipow(begin_lambda, 2) + -2.73806 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + -3.13846e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*begin_lambda + 0.0657627 *begin_x*begin_y*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 3) + -2.56873e-09 *lens_ipow(begin_x, 7)*lens_ipow(begin_lambda, 2) + -0.000182029 *lens_ipow(begin_y, 5)*lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 3);
          out[1] =  + -0.141503 *begin_y + 49.4365 *begin_dy + 0.548589 *begin_dy*begin_lambda + 0.41489 *begin_y*begin_lambda + 0.0244066 *lens_ipow(begin_y, 2)*begin_dy + 0.0146213 *begin_x*begin_y*begin_dx + -20.9484 *lens_ipow(begin_dx, 2)*begin_dy + -0.297076 *begin_y*lens_ipow(begin_lambda, 2) + -0.000312083 *lens_ipow(begin_x, 2)*begin_y + -0.000250017 *lens_ipow(begin_y, 3) + 0.00729085 *lens_ipow(begin_x, 2)*begin_dy + -20.3828 *lens_ipow(begin_dy, 3) + 3.33439 *begin_y*lens_ipow(begin_dy, 2)*begin_lambda + 2.51912 *begin_y*lens_ipow(begin_dx, 2)*begin_lambda + -3.78891 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 2) + -1.13034e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + 0.00048002 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + -0.807166 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + -5.2979e-07 *lens_ipow(begin_y, 5) + -5.07164 *begin_y*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + -2.56011e-05 *lens_ipow(begin_y, 4)*begin_dy*begin_lambda + 1.06779e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_lambda + -1.08743e-09 *lens_ipow(begin_x, 6)*begin_y + 2.24155 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_lambda, 4) + 3.04667 *begin_y*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 4) + 0.0920095 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 5) + 3.60861e-06 *lens_ipow(begin_x, 4)*begin_y*lens_ipow(begin_dx, 2)*begin_lambda + -6.55673e-12 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 5);
          out[2] =  + -0.910038 *begin_dx + -0.0193261 *begin_x + -0.00125658 *begin_x*begin_lambda + -0.000251218 *begin_x*begin_y*begin_dy + -0.000174838 *lens_ipow(begin_x, 2)*begin_dx + 0.4129 *lens_ipow(begin_dx, 3) + 0.0963517 *begin_dx*lens_ipow(begin_dy, 2) + 8.84115e-06 *begin_x*lens_ipow(begin_y, 2) + 2.17952e-05 *lens_ipow(begin_x, 3) + -0.0273656 *begin_x*lens_ipow(begin_dy, 2)*begin_lambda + -4.84779e-05 *lens_ipow(begin_x, 3)*begin_lambda + 6.32952e-05 *lens_ipow(begin_y, 2)*begin_dx*begin_lambda + 2.10771e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2) + -2.16392e-07 *lens_ipow(begin_x, 4)*begin_dx + 4.52445e-05 *lens_ipow(begin_x, 3)*lens_ipow(begin_lambda, 2) + -3.78941e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 2) + 2.33559e-05 *lens_ipow(begin_x, 2)*begin_y*begin_dx*begin_dy + 0.0814277 *begin_x*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 3) + -2.28441e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*begin_lambda + -3.87632e-06 *begin_x*lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2)*begin_dy + -0.0687767 *begin_x*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 4) + 0.00341197 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + 2.89162e-11 *lens_ipow(begin_x, 7)*begin_lambda + -0.00400648 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 3) + 1.58178e-13 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 4) + 3.50375e-14 *begin_x*lens_ipow(begin_y, 8) + -6.49335e-09 *lens_ipow(begin_y, 6)*begin_dx*lens_ipow(begin_dy, 2) + -5.95238e-08 *lens_ipow(begin_x, 5)*lens_ipow(begin_lambda, 6);
          out[3] =  + -0.0178342 *begin_y + -0.908344 *begin_dy + -0.00663163 *begin_y*begin_lambda + -0.007604 *begin_y*lens_ipow(begin_dx, 2) + -0.000187248 *lens_ipow(begin_y, 2)*begin_dy + 0.0159022 *begin_x*begin_dx*begin_dy + -0.000243681 *begin_x*begin_y*begin_dx + 0.761103 *lens_ipow(begin_dx, 2)*begin_dy + 0.00492946 *begin_y*lens_ipow(begin_lambda, 2) + 7.69502e-06 *lens_ipow(begin_x, 2)*begin_y + 7.76167e-06 *lens_ipow(begin_y, 3) + 0.40198 *lens_ipow(begin_dy, 3) + 4.60293e-05 *lens_ipow(begin_x, 2)*begin_dy*begin_lambda + 0.000907222 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2)*begin_dy + -5.04469e-07 *lens_ipow(begin_y, 4)*begin_dy + 3.23617e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + -6.79163e-06 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + 7.16848e-09 *lens_ipow(begin_x, 4)*begin_y + 6.51238e-09 *lens_ipow(begin_y, 5) + 0.000381824 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 3) + 1.13086e-05 *lens_ipow(begin_x, 3)*begin_dx*begin_dy + -0.383626 *lens_ipow(begin_dx, 4)*begin_dy + 6.49928e-07 *lens_ipow(begin_y, 4)*begin_dy*begin_lambda + -3.25634e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_lambda + 2.0029e-06 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*begin_dy + -3.86222e-06 *begin_x*lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_dy, 2) + -4.87562e-12 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 2)*begin_dy + -1.16396e-11 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 6)*begin_dy*lens_ipow(begin_lambda, 2);
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
          domega2_dx0[0][0] =  + -0.0193261  + -0.00125658 *begin_lambda + -0.000251218 *begin_y*begin_dy + -0.000349677 *begin_x*begin_dx + 8.84115e-06 *lens_ipow(begin_y, 2) + 6.53857e-05 *lens_ipow(begin_x, 2) + -0.0273656 *lens_ipow(begin_dy, 2)*begin_lambda + -0.000145434 *lens_ipow(begin_x, 2)*begin_lambda + 6.32312e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -8.65567e-07 *lens_ipow(begin_x, 3)*begin_dx + 0.000135734 *lens_ipow(begin_x, 2)*lens_ipow(begin_lambda, 2) + -1.13682e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + 4.67118e-05 *begin_x*begin_y*begin_dx*begin_dy + 0.0814277 *lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 3) + -6.85322e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_lambda + -3.87632e-06 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2)*begin_dy + -0.0687767 *lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 4) + 2.02414e-10 *lens_ipow(begin_x, 6)*begin_lambda + 7.90889e-13 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4) + 3.50375e-14 *lens_ipow(begin_y, 8) + -2.97619e-07 *lens_ipow(begin_x, 4)*lens_ipow(begin_lambda, 6)+0.0f;
          domega2_dx0[0][1] =  + -0.000251218 *begin_x*begin_dy + 1.76823e-05 *begin_x*begin_y + 0.00012659 *begin_y*begin_dx*begin_lambda + 4.21541e-08 *lens_ipow(begin_x, 3)*begin_y + 2.33559e-05 *lens_ipow(begin_x, 2)*begin_dx*begin_dy + -4.56882e-08 *lens_ipow(begin_x, 3)*begin_y*begin_lambda + -1.1629e-05 *begin_x*lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_dy + 0.00682395 *begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 2) + -0.00801296 *begin_y*lens_ipow(begin_dx, 3)*lens_ipow(begin_lambda, 3) + 6.32711e-13 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 3) + 2.803e-13 *begin_x*lens_ipow(begin_y, 7) + -3.89601e-08 *lens_ipow(begin_y, 5)*begin_dx*lens_ipow(begin_dy, 2)+0.0f;
          domega2_dx0[1][0] =  + 0.0159022 *begin_dx*begin_dy + -0.000243681 *begin_y*begin_dx + 1.539e-05 *begin_x*begin_y + 9.20586e-05 *begin_x*begin_dy*begin_lambda + 0.00181444 *begin_x*lens_ipow(begin_dx, 2)*begin_dy + 6.47233e-08 *begin_x*lens_ipow(begin_y, 3) + 2.86739e-08 *lens_ipow(begin_x, 3)*begin_y + 0.000763648 *begin_x*lens_ipow(begin_dy, 3) + 3.39258e-05 *lens_ipow(begin_x, 2)*begin_dx*begin_dy + -6.51267e-08 *begin_x*lens_ipow(begin_y, 3)*begin_lambda + -3.86222e-06 *lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_dy, 2) + -2.92537e-11 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 2)*begin_dy + -2.32792e-11 *begin_x*lens_ipow(begin_y, 6)*begin_dy*lens_ipow(begin_lambda, 2)+0.0f;
          domega2_dx0[1][1] =  + -0.0178342  + -0.00663163 *begin_lambda + -0.007604 *lens_ipow(begin_dx, 2) + -0.000374496 *begin_y*begin_dy + -0.000243681 *begin_x*begin_dx + 0.00492946 *lens_ipow(begin_lambda, 2) + 7.69502e-06 *lens_ipow(begin_x, 2) + 2.3285e-05 *lens_ipow(begin_y, 2) + -2.01788e-06 *lens_ipow(begin_y, 3)*begin_dy + 9.70849e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -2.03749e-05 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + 7.16848e-09 *lens_ipow(begin_x, 4) + 3.25619e-08 *lens_ipow(begin_y, 4) + 2.59971e-06 *lens_ipow(begin_y, 3)*begin_dy*begin_lambda + -9.76901e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_lambda + 8.01162e-06 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2)*begin_dy + -1.15867e-05 *begin_x*lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 2) + -9.75123e-12 *lens_ipow(begin_x, 6)*begin_y*begin_dy + -6.98377e-11 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 5)*begin_dy*lens_ipow(begin_lambda, 2)+0.0f;
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
        out[4] =  + 2.13553 *begin_lambda + -4.72826 *lens_ipow(begin_lambda, 2) + -0.00103031 *begin_y*begin_dy*begin_lambda + -0.00102465 *begin_x*begin_dx*begin_lambda + -1.11271e-05 *lens_ipow(begin_x, 2)*begin_lambda + -1.11545e-05 *lens_ipow(begin_y, 2)*begin_lambda + 4.91467 *lens_ipow(begin_lambda, 3) + -1.97319 *lens_ipow(begin_lambda, 4) + -0.361162 *lens_ipow(begin_dx, 4) + -0.0189262 *begin_y*lens_ipow(begin_dy, 3) + -0.00017862 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + -0.0190907 *begin_x*lens_ipow(begin_dx, 3) + -0.918305 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + -0.35869 *lens_ipow(begin_dy, 4) + -9.73633e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -0.000498028 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + -0.0192091 *begin_y*lens_ipow(begin_dx, 2)*begin_dy + -0.000495702 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + -0.0193094 *begin_x*begin_dx*lens_ipow(begin_dy, 2) + -0.000651255 *begin_x*begin_y*begin_dx*begin_dy + -0.000179837 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + -0.169861 *lens_ipow(begin_dx, 4)*begin_lambda + -0.17091 *lens_ipow(begin_dy, 4)*begin_lambda + 1.62928e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_lambda + -4.02531e-16 *lens_ipow(begin_y, 10) + -2.37748e-15 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 8) + -7.81748e-15 *lens_ipow(begin_x, 6)*lens_ipow(begin_y, 4) + -5.03842e-16 *lens_ipow(begin_x, 10);
      else
        out[4] = 0.0f;

    } break;


    case zeiss_biotar_1927_58mm:
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
             + 36.5996 *begin_dx + 0.454717 *begin_x + 0.041798 *begin_x*begin_lambda + 1.8289 *begin_dx*begin_lambda + -0.005327 *lens_ipow(begin_x, 2)*begin_dx + -22.4015 *lens_ipow(begin_dx, 3) + -23.0927 *begin_dx*lens_ipow(begin_dy, 2) + -0.000160418 *begin_x*lens_ipow(begin_y, 2) + -0.000163105 *lens_ipow(begin_x, 3) + -0.216495 *begin_x*lens_ipow(begin_dy, 2) + -0.462226 *begin_x*lens_ipow(begin_dx, 2) + -0.00111586 *lens_ipow(begin_y, 2)*begin_dx + -0.0170951 *begin_x*begin_y*begin_dy*begin_lambda + -0.603514 *begin_y*begin_dx*begin_dy*begin_lambda + -3.98252e-07 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2) + 0.0316894 *begin_x*begin_y*lens_ipow(begin_dx, 2)*begin_dy + -1.29849e-07 *begin_x*lens_ipow(begin_y, 4) + -1.6084e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dx + -0.426172 *begin_y*begin_dx*lens_ipow(begin_dy, 3) + 0.0202469 *begin_x*begin_y*begin_dy*lens_ipow(begin_lambda, 3) + -7.04916e-08 *lens_ipow(begin_x, 6)*begin_dx + -1.97483e-06 *lens_ipow(begin_x, 5)*lens_ipow(begin_dx, 2) + 0.830881 *begin_y*begin_dx*begin_dy*lens_ipow(begin_lambda, 4) + -7.23502e-10 *lens_ipow(begin_x, 7) + -2.65605e-08 *lens_ipow(begin_x, 5)*begin_y*begin_dy*begin_lambda + -3.26686e-05 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 3)*begin_lambda + -6.91074e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dy*begin_lambda + -2.27346e-12 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 4),
             + 0.456094 *begin_y + 36.6078 *begin_dy + 1.85026 *begin_dy*begin_lambda + 0.0401649 *begin_y*begin_lambda + -0.218319 *begin_y*lens_ipow(begin_dx, 2) + -0.00569834 *lens_ipow(begin_y, 2)*begin_dy + -0.479457 *begin_y*lens_ipow(begin_dy, 2) + -23.1138 *lens_ipow(begin_dx, 2)*begin_dy + -0.000170969 *lens_ipow(begin_x, 2)*begin_y + -0.000164487 *lens_ipow(begin_y, 3) + -0.00134042 *lens_ipow(begin_x, 2)*begin_dy + -22.5954 *lens_ipow(begin_dy, 3) + -0.0143399 *begin_x*begin_y*begin_dx*begin_lambda + -0.610815 *begin_x*begin_dx*begin_dy*begin_lambda + -0.441549 *begin_x*lens_ipow(begin_dx, 3)*begin_dy + -3.59301e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + -1.05117e-07 *lens_ipow(begin_x, 4)*begin_y + 0.000261591 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dy, 2) + -1.86206e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dy*begin_lambda + 0.0593064 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + -7.67347e-10 *lens_ipow(begin_y, 7) + -7.31072e-08 *lens_ipow(begin_y, 6)*begin_dy + -2.04157e-06 *lens_ipow(begin_y, 5)*lens_ipow(begin_dy, 2) + -3.34223e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dx + 0.0196232 *begin_x*begin_y*begin_dx*lens_ipow(begin_lambda, 4) + 0.903883 *begin_x*begin_dx*begin_dy*lens_ipow(begin_lambda, 4) + -3.19105e-08 *begin_x*lens_ipow(begin_y, 5)*begin_dx*begin_lambda + -2.10282e-12 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 5)
          };
          const float delta_ap[] = {ap_x - pred_ap[0], ap_y - pred_ap[1]};
          sqr_ap_err = delta_ap[0]*delta_ap[0]+delta_ap[1]*delta_ap[1];
          float dx1_domega0[2][2];
          dx1_domega0[0][0] =  + 36.5996  + 1.8289 *begin_lambda + -0.005327 *lens_ipow(begin_x, 2) + -67.2044 *lens_ipow(begin_dx, 2) + -23.0927 *lens_ipow(begin_dy, 2) + -0.924453 *begin_x*begin_dx + -0.00111586 *lens_ipow(begin_y, 2) + -0.603514 *begin_y*begin_dy*begin_lambda + 0.0633788 *begin_x*begin_y*begin_dx*begin_dy + -1.6084e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -0.426172 *begin_y*lens_ipow(begin_dy, 3) + -7.04916e-08 *lens_ipow(begin_x, 6) + -3.94966e-06 *lens_ipow(begin_x, 5)*begin_dx + 0.830881 *begin_y*begin_dy*lens_ipow(begin_lambda, 4) + -9.80058e-05 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*begin_lambda+0.0f;
          dx1_domega0[0][1] =  + -46.1854 *begin_dx*begin_dy + -0.432989 *begin_x*begin_dy + -0.0170951 *begin_x*begin_y*begin_lambda + -0.603514 *begin_y*begin_dx*begin_lambda + 0.0316894 *begin_x*begin_y*lens_ipow(begin_dx, 2) + -1.27852 *begin_y*begin_dx*lens_ipow(begin_dy, 2) + 0.0202469 *begin_x*begin_y*lens_ipow(begin_lambda, 3) + 0.830881 *begin_y*begin_dx*lens_ipow(begin_lambda, 4) + -2.65605e-08 *lens_ipow(begin_x, 5)*begin_y*begin_lambda + -6.91074e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_lambda+0.0f;
          dx1_domega0[1][0] =  + -0.436637 *begin_y*begin_dx + -46.2276 *begin_dx*begin_dy + -0.0143399 *begin_x*begin_y*begin_lambda + -0.610815 *begin_x*begin_dy*begin_lambda + -1.32465 *begin_x*lens_ipow(begin_dx, 2)*begin_dy + 0.0593064 *begin_x*begin_y*lens_ipow(begin_dy, 2)*begin_lambda + -3.34223e-08 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3) + 0.0196232 *begin_x*begin_y*lens_ipow(begin_lambda, 4) + 0.903883 *begin_x*begin_dy*lens_ipow(begin_lambda, 4) + -3.19105e-08 *begin_x*lens_ipow(begin_y, 5)*begin_lambda+0.0f;
          dx1_domega0[1][1] =  + 36.6078  + 1.85026 *begin_lambda + -0.00569834 *lens_ipow(begin_y, 2) + -0.958913 *begin_y*begin_dy + -23.1138 *lens_ipow(begin_dx, 2) + -0.00134042 *lens_ipow(begin_x, 2) + -67.7861 *lens_ipow(begin_dy, 2) + -0.610815 *begin_x*begin_dx*begin_lambda + -0.441549 *begin_x*lens_ipow(begin_dx, 3) + 0.000523182 *lens_ipow(begin_x, 2)*begin_y*begin_dy + -1.86206e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_lambda + 0.118613 *begin_x*begin_y*begin_dx*begin_dy*begin_lambda + -7.31072e-08 *lens_ipow(begin_y, 6) + -4.08314e-06 *lens_ipow(begin_y, 5)*begin_dy + 0.903883 *begin_x*begin_dx*lens_ipow(begin_lambda, 4)+0.0f;
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
          out[0] =  + 57.1689 *begin_dx + 0.193146 *begin_x + 0.0484321 *begin_x*begin_lambda + 0.389705 *begin_y*begin_dx*begin_dy + 0.0233441 *begin_x*begin_y*begin_dy + 0.0310786 *lens_ipow(begin_x, 2)*begin_dx + -23.7113 *lens_ipow(begin_dx, 3) + -23.7666 *begin_dx*lens_ipow(begin_dy, 2) + -8.51031e-05 *lens_ipow(begin_x, 3) + 1.06525 *begin_x*lens_ipow(begin_dx, 2) + 0.00892956 *lens_ipow(begin_y, 2)*begin_dx + 1.01113 *begin_dx*lens_ipow(begin_lambda, 2) + 2.53406 *begin_x*lens_ipow(begin_dy, 2)*begin_lambda + 0.000108935 *lens_ipow(begin_x, 3)*begin_lambda + -1.85433e-06 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2) + 0.10017 *begin_x*begin_y*lens_ipow(begin_dx, 2)*begin_dy + -6.25497e-05 *lens_ipow(begin_x, 4)*begin_dx + 0.06383 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 3) + -9.03315e-07 *lens_ipow(begin_x, 5) + -9.64335e-07 *begin_x*lens_ipow(begin_y, 4) + -7.09054e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dx + -5.51919e-05 *lens_ipow(begin_x, 3)*begin_y*begin_dy + -5.40607e-05 *begin_x*lens_ipow(begin_y, 3)*begin_dy + 0.0395167 *lens_ipow(begin_y, 2)*begin_dx*lens_ipow(begin_dy, 2) + -2.25422 *begin_x*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.0239359 *begin_x*begin_y*lens_ipow(begin_dy, 3) + 0.052306 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 2)*begin_lambda + -1.71832e-05 *lens_ipow(begin_y, 4)*begin_dx*begin_lambda;
          out[1] =  + 0.223232 *begin_y + 57.5672 *begin_dy + 0.691113 *begin_y*lens_ipow(begin_dx, 2) + 0.017258 *lens_ipow(begin_y, 2)*begin_dy + 0.843734 *begin_y*lens_ipow(begin_dy, 2) + 0.39208 *begin_x*begin_dx*begin_dy + 0.0232544 *begin_x*begin_y*begin_dx + -23.9144 *lens_ipow(begin_dx, 2)*begin_dy + -0.000545388 *lens_ipow(begin_y, 3) + 0.00915536 *lens_ipow(begin_x, 2)*begin_dy + -25.0805 *lens_ipow(begin_dy, 3) + 0.000785302 *lens_ipow(begin_y, 3)*begin_lambda + 0.0413811 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2)*begin_dy + -1.07207e-05 *lens_ipow(begin_x, 4)*begin_dy + -5.43478e-05 *begin_x*lens_ipow(begin_y, 3)*begin_dx + 0.0238682 *begin_x*begin_y*lens_ipow(begin_dx, 3) + 0.0188301 *lens_ipow(begin_y, 2)*begin_dy*lens_ipow(begin_lambda, 2) + 0.100844 *begin_x*begin_y*begin_dx*lens_ipow(begin_dy, 2) + 0.0275789 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_dy + -1.83776e-06 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + -5.44981e-05 *lens_ipow(begin_x, 3)*begin_y*begin_dx + -1.19686e-06 *lens_ipow(begin_x, 4)*begin_y + -7.01775e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2)*begin_dy + 0.00737572 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 2)*begin_lambda + 4.09626e-07 *lens_ipow(begin_x, 4)*begin_y*begin_lambda + 0.348138 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 3)*begin_lambda + -1.64278e-06 *lens_ipow(begin_y, 5)*lens_ipow(begin_lambda, 2) + 5.19554 *begin_y*lens_ipow(begin_dy, 4)*lens_ipow(begin_lambda, 2);
          out[2] =  + -1.19429 *begin_dx + -0.0217102 *begin_x + -0.000720481 *begin_x*begin_lambda + -0.00940156 *begin_y*begin_dx*begin_dy + -0.000254619 *begin_x*begin_y*begin_dy + -0.000118977 *lens_ipow(begin_x, 2)*begin_dx + 0.587852 *lens_ipow(begin_dx, 3) + -0.204803 *begin_dx*lens_ipow(begin_dy, 2) + 7.50277e-06 *begin_x*lens_ipow(begin_y, 2) + 6.29155e-06 *lens_ipow(begin_x, 3) + -0.00442155 *begin_x*lens_ipow(begin_dx, 2) + 7.82376e-05 *lens_ipow(begin_y, 2)*begin_dx + -0.0537613 *begin_x*lens_ipow(begin_dy, 2)*begin_lambda + 7.1261e-09 *lens_ipow(begin_x, 5) + 0.0496316 *begin_x*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 1.09726e-06 *lens_ipow(begin_x, 4)*begin_dx*begin_lambda + 3.78813e-05 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 2)*begin_lambda + -1.66454e-05 *lens_ipow(begin_x, 3)*lens_ipow(begin_dy, 2)*begin_lambda + -0.0135048 *begin_x*begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + 17.0903 *lens_ipow(begin_dx, 3)*lens_ipow(begin_dy, 4) + -1.76629e-06 *lens_ipow(begin_y, 4)*begin_dx*lens_ipow(begin_dy, 2) + 0.00015555 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dx, 3)*begin_dy + -0.00160529 *lens_ipow(begin_x, 2)*begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 4.71609e-07 *begin_x*lens_ipow(begin_y, 3)*begin_dy*lens_ipow(begin_lambda, 3) + 2.08345e-11 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4)*begin_dx + 3.06727e-13 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 4) + 2.98578e-14 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 5)*begin_dy + -3.77699e-14 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 6)*begin_dx;
          out[3] =  + -0.0221765 *begin_y + -1.19483 *begin_dy + 0.0266626 *begin_x*begin_dx*begin_dy + -0.000118529 *begin_x*begin_y*begin_dx + 1.31992 *lens_ipow(begin_dx, 2)*begin_dy + 6.74637e-06 *lens_ipow(begin_x, 2)*begin_y + 1.16569e-05 *lens_ipow(begin_y, 3) + 0.00019251 *lens_ipow(begin_x, 2)*begin_dy + 0.605434 *lens_ipow(begin_dy, 3) + -0.0134873 *begin_y*lens_ipow(begin_dx, 2)*begin_lambda + -7.33135e-06 *lens_ipow(begin_y, 3)*begin_lambda + 1.41568 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + 0.0329062 *begin_x*begin_dx*lens_ipow(begin_dy, 3) + -1.03815e-05 *lens_ipow(begin_x, 2)*begin_y*lens_ipow(begin_dx, 2) + 1.06567e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3) + -3.06289e-05 *lens_ipow(begin_y, 3)*lens_ipow(begin_dx, 2) + 4.87314e-09 *lens_ipow(begin_x, 4)*begin_y + 1.3886e-09 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 4)*begin_dy + 0.00190446 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 5) + 0.0885763 *begin_y*lens_ipow(begin_dx, 4)*lens_ipow(begin_lambda, 3) + 2.64595e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 3)*begin_dx*begin_lambda + 1.23741e-07 *lens_ipow(begin_y, 5)*lens_ipow(begin_dx, 2)*begin_lambda + -0.0933092 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + 9.98314e-14 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 5) + -2.22296 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 4)*lens_ipow(begin_lambda, 3) + 2.72843e-08 *lens_ipow(begin_y, 5)*lens_ipow(begin_lambda, 5) + -3.78021e-06 *lens_ipow(begin_y, 5)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.000607305 *lens_ipow(begin_y, 3)*lens_ipow(begin_dy, 8);
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
          domega2_dx0[0][0] =  + -0.0217102  + -0.000720481 *begin_lambda + -0.000254619 *begin_y*begin_dy + -0.000237953 *begin_x*begin_dx + 7.50277e-06 *lens_ipow(begin_y, 2) + 1.88747e-05 *lens_ipow(begin_x, 2) + -0.00442155 *lens_ipow(begin_dx, 2) + -0.0537613 *lens_ipow(begin_dy, 2)*begin_lambda + 3.56305e-08 *lens_ipow(begin_x, 4) + 0.0496316 *lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 4.38904e-06 *lens_ipow(begin_x, 3)*begin_dx*begin_lambda + 0.000113644 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2)*begin_lambda + -4.99363e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2)*begin_lambda + -0.0135048 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + 0.000311099 *begin_x*begin_y*lens_ipow(begin_dx, 3)*begin_dy + -0.00321059 *begin_x*begin_dx*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 4.71609e-07 *lens_ipow(begin_y, 3)*begin_dy*lens_ipow(begin_lambda, 3) + 8.3338e-11 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 4)*begin_dx + 1.53363e-12 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4) + 1.49289e-13 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 5)*begin_dy + -1.5108e-13 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 6)*begin_dx+0.0f;
          domega2_dx0[0][1] =  + -0.00940156 *begin_dx*begin_dy + -0.000254619 *begin_x*begin_dy + 1.50055e-05 *begin_x*begin_y + 0.000156475 *begin_y*begin_dx + -0.0135048 *begin_x*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3) + -7.06517e-06 *lens_ipow(begin_y, 3)*begin_dx*lens_ipow(begin_dy, 2) + 0.00015555 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 3)*begin_dy + 1.41483e-06 *begin_x*lens_ipow(begin_y, 2)*begin_dy*lens_ipow(begin_lambda, 3) + 8.3338e-11 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 3)*begin_dx + 1.22691e-12 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 3) + 1.49289e-13 *lens_ipow(begin_x, 5)*lens_ipow(begin_y, 4)*begin_dy + -2.2662e-13 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 5)*begin_dx+0.0f;
          domega2_dx0[1][0] =  + 0.0266626 *begin_dx*begin_dy + -0.000118529 *begin_y*begin_dx + 1.34927e-05 *begin_x*begin_y + 0.000385019 *begin_x*begin_dy + 0.0329062 *begin_dx*lens_ipow(begin_dy, 3) + -2.07631e-05 *begin_x*begin_y*lens_ipow(begin_dx, 2) + 2.13134e-08 *begin_x*lens_ipow(begin_y, 3) + 1.94926e-08 *lens_ipow(begin_x, 3)*begin_y + 2.7772e-09 *begin_x*lens_ipow(begin_y, 4)*begin_dy + 0.00380891 *begin_x*lens_ipow(begin_dy, 5) + 7.93784e-09 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_dx*begin_lambda + 3.99326e-13 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 5)+0.0f;
          domega2_dx0[1][1] =  + -0.0221765  + -0.000118529 *begin_x*begin_dx + 6.74637e-06 *lens_ipow(begin_x, 2) + 3.49706e-05 *lens_ipow(begin_y, 2) + -0.0134873 *lens_ipow(begin_dx, 2)*begin_lambda + -2.19941e-05 *lens_ipow(begin_y, 2)*begin_lambda + -1.03815e-05 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 2) + 3.19701e-08 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -9.18866e-05 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2) + 4.87314e-09 *lens_ipow(begin_x, 4) + 5.5544e-09 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 3)*begin_dy + 0.0885763 *lens_ipow(begin_dx, 4)*lens_ipow(begin_lambda, 3) + 7.93784e-09 *lens_ipow(begin_x, 3)*lens_ipow(begin_y, 2)*begin_dx*begin_lambda + 6.18705e-07 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*begin_lambda + -0.186618 *begin_y*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 3)*lens_ipow(begin_lambda, 2) + 4.99157e-13 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4) + -2.22296 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 4)*lens_ipow(begin_lambda, 3) + 1.36422e-07 *lens_ipow(begin_y, 4)*lens_ipow(begin_lambda, 5) + -1.89011e-05 *lens_ipow(begin_y, 4)*lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2)*lens_ipow(begin_lambda, 2) + 0.00182192 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 8)+0.0f;
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
        out[4] =  + 0.445415  + 0.703675 *begin_lambda + -0.00105712 *begin_x*begin_dx + -1.95963e-05 *lens_ipow(begin_x, 2) + -0.0406648 *lens_ipow(begin_dx, 2) + -0.968706 *lens_ipow(begin_lambda, 2) + 0.4698 *lens_ipow(begin_lambda, 3) + -0.0454226 *begin_y*lens_ipow(begin_dy, 3) + -1.65054 *lens_ipow(begin_dx, 2)*lens_ipow(begin_dy, 2) + -1.23797e-05 *lens_ipow(begin_x, 2)*begin_y*begin_dy + -0.885696 *lens_ipow(begin_dy, 4) + -1.25888e-07 *lens_ipow(begin_x, 2)*lens_ipow(begin_y, 2) + -1.09325e-07 *lens_ipow(begin_y, 4) + -0.0373244 *begin_y*lens_ipow(begin_dx, 2)*begin_dy + -4.93975e-06 *lens_ipow(begin_x, 3)*begin_dx + -0.00131154 *lens_ipow(begin_y, 2)*lens_ipow(begin_dy, 2) + -0.0374354 *begin_x*begin_dx*lens_ipow(begin_dy, 2) + -0.00129105 *begin_x*begin_y*begin_dx*begin_dy + -0.000344897 *lens_ipow(begin_x, 2)*lens_ipow(begin_dy, 2) + -1.8055e-05 *lens_ipow(begin_y, 3)*begin_dy + -2.18263e-05 *begin_x*lens_ipow(begin_y, 2)*begin_dx*begin_lambda + -0.000617129 *lens_ipow(begin_y, 2)*lens_ipow(begin_dx, 2)*begin_lambda + -5.14691 *lens_ipow(begin_dx, 6) + -0.419065 *begin_x*lens_ipow(begin_dx, 5) + -0.0184877 *lens_ipow(begin_x, 2)*lens_ipow(begin_dx, 4) + -3.62468e-06 *lens_ipow(begin_x, 4)*lens_ipow(begin_dx, 2) + -0.000379881 *lens_ipow(begin_x, 3)*lens_ipow(begin_dx, 3) + -2.20816e-12 *lens_ipow(begin_x, 4)*lens_ipow(begin_y, 4)*lens_ipow(begin_lambda, 2);
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
  const float x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];
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
  const float x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];
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
  const float tri = .5f*radius * radius * sinf(2.0f*AI_PI/(float)blades);
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

  common_sincosf(2.0f*AI_PI/blades * (tri+1), p1, p1+1);
  common_sincosf(2.0f*AI_PI/blades * tri, p2, p2+1);

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
    common_sincosf(2.0f*(float)AI_PI/blades * b, &tmpy, &tmpx);
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
