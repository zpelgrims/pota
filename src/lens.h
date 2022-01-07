#pragma once
#include <cmath>
#include <vector>
#include <algorithm>
#include "../../Eigen/Eigen/Core"
#include "../../Eigen/Eigen/LU"

#include "global.h"

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif



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



static inline double raytrace_dot(Eigen::Vector3d u, Eigen::Vector3d v) {
  return u(0)*v(0) + u(1)*v(1) + u(2)*v(2);
}

static inline void raytrace_cross(Eigen::Vector3d &r, const Eigen::Vector3d u, const Eigen::Vector3d v) {
  r(0) = u(1)*v(2)-u(2)*v(1);
  r(1) = u(2)*v(0)-u(0)*v(2);
  r(2) = u(0)*v(1)-u(1)*v(0);
}

static inline void raytrace_normalise(Eigen::Vector3d &v) {
  const double ilen = 1.0f/std::sqrt(raytrace_dot(v,v));
  for(int k=0;k<3;k++) v(k) *= ilen;
}

static inline void raytrace_substract(Eigen::Vector3d &u, const Eigen::Vector3d v) {
  for(int k = 0; k < 3; k++) u(k) -= v(k);
}

static inline void raytrace_multiply(Eigen::Vector3d &v, const double s) {
  for(int k = 0; k < 3; k++) v(k) *= s;
}

static inline void propagate(Eigen::Vector3d &pos, const Eigen::Vector3d dir, const double dist) {
  for(int i=0;i<3;i++) pos(i) += dir(i) * dist;
}


static inline void planeToCs(const Eigen::Vector2d inpos, const Eigen::Vector2d indir, Eigen::Vector3d &outpos, Eigen::Vector3d &outdir, const double planepos) {
  outpos(0) = inpos(0);
  outpos(1) = inpos(1);
  outpos(2) = planepos;

  outdir(0) = indir(0);
  outdir(1) = indir(1);
  outdir(2) = 1;

  raytrace_normalise(outdir);
}

static inline void csToPlane(const Eigen::Vector3d inpos, const Eigen::Vector3d indir, Eigen::Vector2d &outpos, Eigen::Vector2d &outdir, const double planepos)
{
  //intersection with plane at z = planepos
  const double t = (planepos - inpos(2)) / indir(2);

  outpos(0) = inpos(0) + t * indir(0);
  outpos(1) = inpos(1) + t * indir(1);

  outdir(0) = indir(0) / std::abs(indir(2));
  outdir(1) = indir(1) / std::abs(indir(2));
}

static inline void sphereToCs(const Eigen::Vector2d inpos, const Eigen::Vector2d indir, Eigen::Vector3d &outpos, Eigen::Vector3d &outdir, const double center, const double sphereRad)
{
  const Eigen::Vector3d normal(
    inpos(0)/sphereRad,
    inpos(1)/sphereRad,
    std::sqrt(std::max(0.0, sphereRad*sphereRad-inpos(0)*inpos(0)-inpos(1)*inpos(1)))/std::abs(sphereRad)
  );

  const Eigen::Vector3d tempDir(
    indir(0),
    indir(1),
    std::sqrt(std::max(0.0, 1.0-indir(0)*indir(0)-indir(1)*indir(1)))
  );

  Eigen::Vector3d ex(normal(2), 0, -normal(0));
  raytrace_normalise(ex);
  Eigen::Vector3d ey(0,0,0);
  raytrace_cross(ey, normal, ex);

  outdir(0) = tempDir(0) * ex(0) + tempDir(1) * ey(0) + tempDir(2) * normal(0);
  outdir(1) = tempDir(0) * ex(1) + tempDir(1) * ey(1) + tempDir(2) * normal(1);
  outdir(2) = tempDir(0) * ex(2) + tempDir(1) * ey(2) + tempDir(2) * normal(2);

  outpos(0) = inpos(0);
  outpos(1) = inpos(1);
  outpos(2) = normal(2) * sphereRad + center;
}

static inline void csToSphere(const Eigen::Vector3d inpos, const Eigen::Vector3d indir, Eigen::Vector2d &outpos, Eigen::Vector2d &outdir, const double sphereCenter, const double sphereRad)
{
  const Eigen::Vector3d normal(
    inpos(0)/sphereRad,
    inpos(1)/sphereRad,
    std::abs((inpos(2)-sphereCenter)/sphereRad)
  );

  Eigen::Vector3d tempDir(indir(0), indir(1), indir(2));
  raytrace_normalise(tempDir);

  // tangent
  Eigen::Vector3d ex(normal(2), 0, -normal(0));
  raytrace_normalise(ex);
  
  // bitangent
  Eigen::Vector3d ey(0,0,0);
  raytrace_cross(ey, normal, ex);
  
  // encode ray direction as projected position on unit disk perpendicular to the normal
  outdir(0) = raytrace_dot(tempDir, ex);
  outdir(1) = raytrace_dot(tempDir, ey);

  // outpos is unchanged, z term omitted
  outpos(0) = inpos(0);
  outpos(1) = inpos(1);
}


static inline void csToCylinder(const Eigen::Vector3d inpos, const Eigen::Vector3d indir, Eigen::Vector2d &outpos, Eigen::Vector2d &outdir, const double center, const double R, const bool cyl_y) {

  Eigen::Vector3d normal(0,0,0);
  if (cyl_y){
    normal(0) = inpos(0)/R;
    normal(2) = std::abs((inpos(2) - center)/R);
  } else {
    normal(1) = inpos(1)/R;
    normal(2) = std::abs((inpos(2) - center)/R);
  }

  Eigen::Vector3d tempDir(indir(0), indir(1), indir(2));
  raytrace_normalise(tempDir); //untested

  // tangent
  Eigen::Vector3d ex(normal(2), 0, -normal(0));
  
  // bitangent
  Eigen::Vector3d ey(0,0,0);
  raytrace_cross(ey, normal, ex);
  raytrace_normalise(ey); // not sure if this is necessary
  
  // encode ray direction as projected position on unit disk perpendicular to the normal
  outdir(0) = raytrace_dot(tempDir, ex);
  outdir(1) = raytrace_dot(tempDir, ey);

  // outpos is unchanged, z term omitted
  outpos(0) = inpos(0);
  outpos(1) = inpos(1);
}


static inline void cylinderToCs(const Eigen::Vector2d inpos, const Eigen::Vector2d indir, Eigen::Vector3d &outpos, Eigen::Vector3d &outdir, const double center, const double R, const bool cyl_y) {

  Eigen::Vector3d normal(0,0,0);
  if (cyl_y){
    normal(0) = inpos(0)/R;
    normal(2) = std::sqrt(std::max(0.0, R*R-inpos(0)*inpos(0)))/std::abs(R);
  } else {
    normal(1) = inpos(1)/R;
    normal(2) = std::sqrt(std::max(0.0, R*R-inpos(1)*inpos(1)))/std::abs(R);
  }

  const Eigen::Vector3d tempDir(
    indir(0), 
    indir(1), 
    std::sqrt(std::max(0.0, 1.0-indir(0)*indir(0)-indir(1)*indir(1)))
  );

  // tangent
  Eigen::Vector3d ex(normal(2), 0, -normal(0));
  raytrace_normalise(ex); // not sure if this is necessary
  
  // bitangent
  Eigen::Vector3d ey(0,0,0);
  raytrace_cross(ey, normal, ex);
  raytrace_normalise(ey); // not sure if this is necessary
  
  outdir(0) = tempDir(0) * ex(0) + tempDir(1) * ey(0) + tempDir(2) * normal(0);
  outdir(1) = tempDir(0) * ex(1) + tempDir(1) * ey(1) + tempDir(2) * normal(1);
  outdir(2) = tempDir(0) * ex(2) + tempDir(1) * ey(2) + tempDir(2) * normal(2);

  outpos(0) = inpos(0);
  outpos(1) = inpos(1);
  outpos(2) = normal(2) * R + center;
}



// helper function for dumped polynomials to compute integer powers of x:
static inline double lens_ipow(const double x, const int exp) {
  if(exp == 0) return 1.0f;
  if(exp == 1) return x;
  if(exp == 2) return x*x;
  const double p2 = lens_ipow(x, exp/2);
  if(exp &  1) return x * p2 * p2;
  return p2 * p2;
}





/*
// evaluates from the sensor (in) to the aperture (out) only
// returns the transmittance.
static inline float lens_evaluate_aperture(const float *in, float *out)
{
  const float x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];
#include "pt_evaluate_aperture.h"
  out[0] = out_x; out[1] = out_y; out[2] = out_dx; out[3] = out_dy;
  return std::max(0.0f, out_transmittance);
}
*/





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
  const float deto = std::sqrt(R*R-out[0]*out[0]-out[1]*out[1])/R;
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


// maps points on the unit square onto the unit disk uniformly
inline void concentric_disk_sample(const double ox, const double oy, Eigen::Vector2d &unit_disk, bool fast_trigo)
{
  double phi, r;

  // switch coordinate space from [0, 1] to [-1, 1]
  double a = 2.0*ox - 1.0;
  double b = 2.0*oy - 1.0;

  if ((a*a) > (b*b)){
    r = a;
    phi = (0.78539816339) * (b/a);
  }
  else {
    r = b;
    phi = (M_PI/2.0) - (0.78539816339) * (a/b);
  }

  if (!fast_trigo){
    unit_disk(0) = r * std::cos(phi);
    unit_disk(1) = r * std::sin(phi);
  } else {
    unit_disk(0) = r * fast_cos(phi);
    unit_disk(1) = r * fast_sin(phi);
  }
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





inline std::vector<double> logarithmic_values()
{
  double min = 0.0;
  double max = 45.0;
  double exponent = 2.0; // sharpness
  std::vector<double> log;

  for(double i = -1.0; i <= 1.0; i += 0.0001) {
    log.push_back((i < 0 ? -1 : 1) * std::pow(i, exponent) * (max - min) + min);
  }

  return log;
}


// line plane intersection with fixed intersection at y = 0
// used for finding the focal length and sensor shift
inline Eigen::Vector3d line_plane_intersection(Eigen::Vector3d rayOrigin, Eigen::Vector3d rayDirection)
{
  Eigen::Vector3d coord(100.0, 0.0, 100.0);
  Eigen::Vector3d planeNormal(0.0, 1.0, 0.0);
  rayDirection.normalize();
  coord.normalize();
  return rayOrigin + (rayDirection * (coord.dot(planeNormal) - planeNormal.dot(rayOrigin)) / planeNormal.dot(rayDirection));
}


inline float calculate_distance_vec2(Eigen::Vector2d a, Eigen::Vector2d b) { 
    return std::sqrt(std::pow(b[0] - a[0], 2) +  std::pow(b[1] - a[1], 2));
}

inline Eigen::Vector3d chromatic_abberration_empirical(Eigen::Vector2d pos, float distance_mult, Eigen::Vector2d &lens, float apertureradius) {
  float distance_to_center = calculate_distance_vec2(Eigen::Vector2d(0.0, 0.0), pos);
  int random_aperture = static_cast<int>(std::floor((xor128() / 4294967296.0) * 3.0));

  Eigen::Vector2d aperture_0_center(0.0, 0.0);
  Eigen::Vector2d aperture_1_center(- pos * distance_to_center * distance_mult);
  Eigen::Vector2d aperture_2_center(pos * distance_to_center * distance_mult);

  Eigen::Vector3d weight(1.0, 1.0, 1.0);

  if (random_aperture == 0) {
      if (std::pow(lens(0)-aperture_1_center(0), 2) + std::pow(lens(1) - aperture_1_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(0) = 0.0;
      }
      if (std::pow(lens(0)-aperture_0_center(0), 2) + std::pow(lens(1) - aperture_0_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(2) = 0.0;
      }
      if (std::pow(lens(0)-aperture_2_center(0), 2) + std::pow(lens(1) - aperture_2_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(1) = 0.0;
      }
  } else if (random_aperture == 1) {
      lens += aperture_1_center;
      if (std::pow(lens(0)-aperture_1_center(0), 2) + std::pow(lens(1) - aperture_1_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(0) = 0.0;
      }
      if (std::pow(lens(0)-aperture_0_center(0), 2) + std::pow(lens(1) - aperture_0_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(2) = 0.0;
      }
      if (std::pow(lens(0)-aperture_2_center(0), 2) + std::pow(lens(1) - aperture_2_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(1) = 0.0;
      }
  } else if (random_aperture == 2) {
      lens += aperture_2_center;
      if (std::pow(lens(0)-aperture_1_center(0), 2) + std::pow(lens(1) - aperture_1_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(0) = 0.0;
      }
      if (std::pow(lens(0)-aperture_0_center(0), 2) + std::pow(lens(1) - aperture_0_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(2) = 0.0;
      }
      if (std::pow(lens(0)-aperture_2_center(0), 2) + std::pow(lens(1) - aperture_2_center(1), 2) > std::pow(apertureradius, 2)) {
          weight(1) = 0.0;
      }
  }

  return weight;
}
   


// Improved concentric mapping code by Dave Cline [peter shirley´s blog]
// maps points on the unit square onto the unit disk uniformly
inline void concentricDiskSample(float ox, float oy, Eigen::Vector2d &lens, float bias, float squarelerp, float squeeze_x)
{
    if (ox == 0.0 && oy == 0.0){
        lens(0) = 0.0;
        lens(1) = 0.0;
        return;
    }

    float phi, r;

    // switch coordinate space from [0, 1] to [-1, 1]
    const float a = 2.0 * ox - 1.0;
    const float b = 2.0 * oy - 1.0;

    if ((a * a) > (b * b)){
        r = a;
        phi = 0.78539816339 * (b / a);
    }
    else {
        r = b;
        phi = (AI_PIOVER2) - ((0.78539816339) * (a / b));
    }

    if (bias != 0.5) r = AiBias(std::abs(r), bias) * (r < 0 ? -1 : 1);


    bool fast_trigo = true;

    const float cos_phi = fast_trigo ? fast_cos(phi) : std::cos(phi);
    const float sin_phi = fast_trigo ? fast_sin(phi) : std::sin(phi);
    lens(0) = r * cos_phi;
    lens(1) = r * sin_phi;

    if (squarelerp > 0.0){
        lens(0) = linear_interpolate(squarelerp, lens(0), a);
        lens(1) = linear_interpolate(squarelerp, lens(1), b);
    }
}



// creates a secondary, virtual aperture resembling the exit pupil on a real lens
inline bool empericalOpticalVignetting(AtVector origin, AtVector direction, float apertureRadius, float opticalVignettingRadius, float opticalVignettingDistance){
    // because the first intersection point of the aperture is already known, I can just linearly scale it by the distance to the second aperture
    float intersection = std::abs(opticalVignettingDistance / direction.z);
    AtVector opticalVignetPoint = (direction * intersection) - origin;
    float pointHypotenuse = std::sqrt((opticalVignetPoint.x * opticalVignetPoint.x) + (opticalVignetPoint.y * opticalVignetPoint.y));
    float virtualApertureTrueRadius = apertureRadius * opticalVignettingRadius;

    return std::abs(pointHypotenuse) < virtualApertureTrueRadius;
}

inline bool empericalOpticalVignettingSquare(AtVector origin, AtVector direction, float apertureRadius, float opticalVignettingRadius, float opticalVignettingDistance, float squarebias){
    float intersection = std::abs(opticalVignettingDistance / direction.z);
    AtVector opticalVignetPoint = (direction * intersection) - origin;

    float power = 1.0 + squarebias;
    float radius = apertureRadius * opticalVignettingRadius;
    float dist = std::pow(std::abs(opticalVignetPoint.x), power) + std::pow(std::abs(opticalVignetPoint.y), power);
   
	return !(dist > std::pow(radius, power));
}

// emperical mapping
inline float lerp_squircle_mapping(float amount) {
    return 1.0 + std::log(1.0+amount)*std::exp(amount*3.0);
}

inline AtVector2 barrelDistortion(AtVector2 uv, float distortion) {    
    uv *= 1. + AiV2Dot(uv, uv) * distortion;
    return uv;
}

inline AtVector2 inverseBarrelDistortion(AtVector2 uv, float distortion) {    
    
    float b = distortion;
    float l = AiV2Length(uv);
    
    float x0 = std::pow(9.*b*b*l + std::sqrt(3.) * std::sqrt(27.*b*b*b*b*l*l + 4.*b*b*b), 1./3.);
    float x = x0 / (std::pow(2., 1./3.) * std::pow(3., 2./3.) * b) - std::pow(2./3., 1./3.) / x0;
       
    return uv * (x / l);
}


// idea is to use the middle ray (does not get perturbed) as a measurement of how much coma there needs to be
inline float abb_coma_multipliers(const float sensor_width, const float focal_length, const AtVector dir_from_center, const Eigen::Vector2d unit_disk){
    const AtVector maximal_perturbed_ray(1.0 * (sensor_width*0.5), 1.0 * (sensor_width*0.5), -focal_length);
    float maximal_projection = AiV3Dot(AiV3Normalize(maximal_perturbed_ray), AtVector(0.0, 0.0, -1.0));
    float current_projection = AiV3Dot(dir_from_center, AtVector(0.0, 0.0, -1.0));
    float projection_perc = ((current_projection - maximal_projection)/(1.0-maximal_projection) - 0.5) * 2.0;
    float dist_from_sensor_center = 1.0 - projection_perc;
    float dist_from_aperture = unit_disk.norm();
    return dist_from_sensor_center * dist_from_aperture;
}


// rotate vector on axis orthogonal to ray dir
inline AtVector abb_coma_perturb(AtVector dir_from_lens, AtVector ray_to_perturb, float abb_coma, bool reverse){
    AtVector axis_tmp = AiV3Normalize(AiV3Cross(dir_from_lens, AtVector(0.0, 0.0, -1.0)));
    Eigen::Vector3d axis(axis_tmp.x, axis_tmp.y, axis_tmp.z);
    Eigen::Matrix3d rot(Eigen::AngleAxisd((abb_coma*2.3456*AI_PI)/180.0, axis)); // first arg is angle in degrees, constant is arbitrary
    Eigen::Vector3d raydir(ray_to_perturb.x, ray_to_perturb.y, ray_to_perturb.z);
    Eigen::Vector3d rotated_vector = (reverse ? rot.inverse() : rot) * raydir;
    return AtVector(rotated_vector(0), rotated_vector(1), rotated_vector(2));
}
