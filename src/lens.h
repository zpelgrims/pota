#pragma once
#include <math.h>
#include <vector>
#include "../../Eigen/Eigen/Dense"


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



// line plane intersection with fixed intersection at y = 0
// used for finding the focal length and sensor shift
Eigen::vector3d line_plane_intersection(Eigen::vector3d rayOrigin, Eigen::vector3d rayDirection)
{
  Eigen::vector3d coord(100.0, 0.0, 100.0);
  Eigen::vector3d planeNormal(0.0, 1.0, 0.0);
  rayDirection.normalize();
  coord.normalize();
  return rayOrigin + (rayDirection * (coord.dot(planeNormal) - planenormal.dot(rayOrigin)) / planeNormal.dot(rayDirection));
}


void camera_get_y0_intersection_distance(float sensor_shift, float &intersection_distance, MyCameraData *camera_data)
{
	float sensor[5] = {0.0f};
  float aperture[5] = {0.0f};
  float out[5] = {0.0f};
  sensor[4] = camera_data->lambda;
  aperture[1] = camera_data->lens_aperture_housing_radius * 0.1;

  lens_pt_sample_aperture(sensor, aperture, sensor_shift, camera_data);

  sensor[0] += sensor[2] * sensor_shift;
	sensor[1] += sensor[3] * sensor_shift;

	float transmittance = lens_evaluate(sensor, out, camera_data);

	// convert from sphere/sphere space to camera space
	float camera_space_pos[3];
	float camera_space_omega[3];
	lens_sphereToCs(out, out+2, camera_space_pos, camera_space_omega, -camera_data->lens_outer_pupil_curvature_radius, camera_data->lens_outer_pupil_curvature_radius);

	Eigen::vector3d ray_origin(camera_space_pos[0], camera_space_pos[1], camera_space_pos[2]);
	Eigen::vector3d ray_dir(camera_space_omega[0], camera_space_omega[1], camera_space_omega[2]);

  intersection_distance = line_plane_intersection(ray_origin, ray_dir)(2);

  //ray_origin *= -0.1;
  //ray_dir *= -0.1;

}


// focal_distance is in mm
void logarithmic_focus_search(const float focal_distance, float &best_sensor_shift, float &closest_distance, MyCameraData *camera_data){
  std::vector<float> log = logarithmic_values();

  for (float sensorshift : log){
  	float intersection_distance = 0.0f;
    //AiMsgInfo("sensorshift: %f", sensorshift);

    camera_get_y0_intersection_distance(sensorshift, intersection_distance, camera_data);
    //AiMsgInfo("intersection_distance: %f at sensor_shift: %f", intersection_distance, sensorshift);
    float new_distance = focal_distance - intersection_distance;
    //AiMsgInfo("new_distance: %f", new_distance);


    if (new_distance < closest_distance && new_distance > 0.0f){
      closest_distance = new_distance;
      best_sensor_shift = sensorshift;
      //AiMsgInfo("best_sensor_shift: %f", best_sensor_shift);
    }
  }
}



inline bool trace_ray_focus_check(float sensor_shift, MyCameraData *camera_data)
{

  float sensor[5] = {0.0f};
  float aperture[5] = {0.0f};
  float out[5] = {0.0f};
  sensor[4] = camera_data->lambda;
  aperture[1] = camera_data->lens_aperture_housing_radius * 0.1;

	lens_pt_sample_aperture(sensor, aperture, sensor_shift, camera_data);

  // move to beginning of polynomial
	sensor[0] += sensor[2] * sensor_shift;
	sensor[1] += sensor[3] * sensor_shift;


	// propagate ray from sensor to outer lens element
  float transmittance = lens_evaluate(sensor, out, camera_data);
  if(transmittance <= 0.0f){
    return false;
  }


  // crop out by outgoing pupil
  if( out[0]*out[0] + out[1]*out[1] > camera_data->lens_outer_pupil_radius*camera_data->lens_outer_pupil_radius){
    return false;
  }


  // crop at inward facing pupil
  const float px = sensor[0] + sensor[2] * camera_data->lens_focal_length;
  const float py = sensor[1] + sensor[3] * camera_data->lens_focal_length;
  if (px*px + py*py > camera_data->lens_inner_pupil_radius*camera_data->lens_inner_pupil_radius){
    return false;
  }


	// convert from sphere/sphere space to camera space
	float camera_space_pos[3];
	float camera_space_omega[3];
	lens_sphereToCs(out, out+2, camera_space_pos, camera_space_omega, -camera_data->lens_outer_pupil_curvature_radius, camera_data->lens_outer_pupil_curvature_radius);

  Eigen::vector3d origin(camera_space_pos[0], camera_space_pos[1], camera_space_pos[2]);
  Eigen::vector3d direction(camera_space_omega[0], camera_space_omega[1], camera_space_omega[2]);

  float y0 = line_plane_intersection(origin, direction)(2);
  //printf("[LENTIL] y=0 ray plane intersection: %f", y0);

	origin *= -0.1; // convert to cm
  direction *= -0.1; //reverse rays and convert to cm

  return true;
}





inline void trace_ray(bool original_ray, int &tries, const float input_sx, const float input_sy, const float input_lensx, const float input_lensy, float &r1, float &r2, Eigen::vector3d &weight, Eigen::vector3d &origin, Eigen::vector3d &direction, MyCameraData *camera_data)
{

  bool ray_succes = false;
  tries = 0;
  float sensor[5] = {0.0f};
  float aperture[5] = {0.0f};
  float out[5] = {0.0f};

  while(ray_succes == false && tries <= camera_data->vignetting_retries){

  	// set sensor position coords
	  sensor[0] = input_sx * (camera_data->sensor_width * 0.5f);
	  sensor[1] = input_sy * (camera_data->sensor_width * 0.5f);
  	sensor[2] = sensor[3] = 0.0f;
	  sensor[4] = camera_data->lambda;

	  aperture[0] = aperture[1] = aperture[2] = aperture[3]  = aperture[4] = 0.0f;
	  out[0] = out[1] = out[2] = out[3] = out[4] = 0.0f;

	  // no dof, all rays through single aperture point
	  if (!camera_data->dof) aperture[0] = aperture[1] = 0.0;
	  else if (camera_data->dof && camera_data->aperture_blades <= 2)
	  {
			// transform unit square to unit disk
		  Eigen::vector2d unit_disk(0.0f, 0.0f);
		  if (tries == 0) concentric_disk_sample(input_lensx, input_lensy, unit_disk, false);
		  else {
		  	if (original_ray){
				  r1 = xor128() / 4294967296.0f;
				  r2 = xor128() / 4294967296.0f;
			  }

		  	concentric_disk_sample(r1, r2, unit_disk, true);
		  }

      aperture[0] = unit_disk(0) * camera_data->aperture_radius;
      aperture[1] = unit_disk(1) * camera_data->aperture_radius;
	  } 
	  else if (camera_data->dof && camera_data->aperture_blades > 2)
	  {
	  	if (tries == 0) lens_sample_aperture(&aperture[0], &aperture[1], input_lensx, input_lensy, camera_data->aperture_radius, camera_data->aperture_blades);
	  	else {
	  		if (original_ray)
	  		{
		  		r1 = xor128() / 4294967296.0f;
		  		r2 = xor128() / 4294967296.0f;
	  		}

	  		lens_sample_aperture(&aperture[0], &aperture[1], r1, r2, camera_data->aperture_radius, camera_data->aperture_blades);
	  	}
	  }

	  if (camera_data->dof)
	  {
	  	// aperture sampling, to make sure ray is able to propagate through whole lens system
	  	lens_pt_sample_aperture(sensor, aperture, camera_data->sensor_shift, camera_data);
	  }
	  

	  // move to beginning of polynomial
		sensor[0] += sensor[2] * camera_data->sensor_shift;
		sensor[1] += sensor[3] * camera_data->sensor_shift;


		// propagate ray from sensor to outer lens element
	  float transmittance = lens_evaluate(sensor, out, camera_data);
		if(transmittance <= 0.0f){
			++tries;
			continue;
		}


		// crop out by outgoing pupil
		if( out[0]*out[0] + out[1]*out[1] > camera_data->lens_outer_pupil_radius*camera_data->lens_outer_pupil_radius){
			++tries;
			continue;
		}


		// crop at inward facing pupil
		const float px = sensor[0] + sensor[2] * camera_data->lens_focal_length;
		const float py = sensor[1] + sensor[3] * camera_data->lens_focal_length; //(note that lens_focal_length is the back focal length, i.e. the distance unshifted sensor -> pupil)
		if (px*px + py*py > camera_data->lens_inner_pupil_radius*camera_data->lens_inner_pupil_radius){
			++tries;
			continue;
		}
		
		ray_succes = true;
	}

	if (ray_succes == false) weight << 0.0, 0.0, 0.0;


	// convert from sphere/sphere space to camera space
	float camera_space_pos[3];
	float camera_space_omega[3];
	lens_sphereToCs(out, out+2, camera_space_pos, camera_space_omega, -camera_data->lens_outer_pupil_curvature_radius, camera_data->lens_outer_pupil_curvature_radius);

  for (int i=0; i<3; i++){
    origin(i) = camera_space_pos[i];
    direction(i) = camera_space_omega[i]
  }


  switch (camera_data->unitModel){
    case mm:
    {
      origin *= -1.0; // reverse rays and convert to cm
      direction *= -1.0; //reverse rays and convert to cm
    } break;
    case cm:
    { 
      origin *= -0.1; // reverse rays and convert to cm
      direction *= -0.1; //reverse rays and convert to cm
    } break;
    case dm:
    {
      origin *= -0.01; // reverse rays and convert to cm
      direction *= -0.01; //reverse rays and convert to cm
    } break;
    case m:
    {
      origin *= -0.001; // reverse rays and convert to cm
      direction *= -0.001; //reverse rays and convert to cm
    }
  }

  direction.normalize();

  // Nan bailout
  if (origin(0) != origin(0) || origin(1) != origin(1) || origin(2) != origin(2) || 
    direction(0) != direction(0) || direction(1) != direction(1) || direction(2) != direction(2))
  {
    weight(0) = 0.0f;
    weight(1) = 0.0f;
    weight(2) = 0.0f;
  }

}


// given camera space scene point, return point on sensor
inline bool trace_backwards(Eigen::vector3d sample_position, const float aperture_radius, const float lambda, Eigen::vector2d &sensor_position, const float sensor_shift, MyCameraData *camera_data)
{
   const float target[3] = {sample_position(0), sample_position(1), sample_position(2)};

   // initialize 5d light fields
   float sensor[5] =  {0.0f, 0.0f, 0.0f, 0.0f, lambda};
   float out[5] =    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
   float aperture[2] =  {0.0f, 0.0f};

   Eigen::vector2d lens;
   concentric_disk_sample(xor128() / 4294967296.0f, xor128() / 4294967296.0f, lens, true);
   aperture[0] = lens(0) * aperture_radius;
   aperture[1] = lens(1) * aperture_radius;

   if(lens_lt_sample_aperture(target, aperture, sensor, out, lambda, camera_data) <= 0.0f) return false;

   // crop at inward facing pupil, not needed to crop by outgoing because already done in lens_lt_sample_aperture()
   const float px = sensor[0] + sensor[2] * camera_data->lens_focal_length;
   const float py = sensor[1] + sensor[3] * camera_data->lens_focal_length; //(note that lens_focal_length is the back focal length, i.e. the distance unshifted sensor -> pupil)
   if (px*px + py*py > camera_data->lens_inner_pupil_radius*camera_data->lens_inner_pupil_radius) return false;

   // shift sensor
   sensor[0] += sensor[2] * -sensor_shift;
   sensor[1] += sensor[3] * -sensor_shift;

   sensor_position(0) = sensor[0];
   sensor_position(1) = sensor[1];

   return true;
}