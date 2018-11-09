#include <ai.h>
#include "pota.h"
#include "lens.h"
#include <cmath>
#include <string>
#include <chrono>
#include "../../polynomial-optics/src/lenssystem.h"
#include "../../polynomial-optics/src/raytrace.h"
#include "../../Eigen/Eigen/Dense"

//#define TIMING

AI_CAMERA_NODE_EXPORT_METHODS(pota_raytracedMethods)

struct CameraRaytraced
{
  std::string id;
	int lenses_cnt;
  lens_element_t lenses[50];
  float p_rad;
  float zoom;
  float lens_focal_length;
  float thickness_original;
  float total_lens_length;

  //test
  int test_cnt;
  int test_maxcnt;
  double test_intersection_distance;

  #ifdef TIMING
    long long int total_duration;
    long long int execution_counter;
  #endif
} camera_rt;


enum
{
  p_unitModel,
  p_lensModel,
  p_sensor_width,
  p_wavelength,
  p_dof,
  p_fstop,
  p_focal_distance,
  p_extra_sensor_shift,
  p_vignetting_retries,
  p_aperture_blades,
  p_backward_samples,
  p_minimum_rgb,
  p_bokeh_exr_path,
  p_proper_ray_derivatives,

  p_rt_lens_focal_length,
  p_rt_lens_id,
  p_rt_lens_zoom,

  p_anamorphic_stretch
};


// to switch between lens models in interface dropdown
// this will need to be automatically filled somehow
static const char* LensModelNames[] =
{
  #include "auto_generated_lens_includes/pota_cpp_lenses.h"
  NULL
};

// to switch between units in interface dropdown
static const char* UnitModelNames[] =
{
  "mm",
  "cm",
  "dm",
  "m",
  NULL
};


float get_lens_length(CameraRaytraced *camera_rt) {
  float total_length = 0.0f;
  for (int i = 0; i < camera_rt->lenses_cnt; i++) {
    total_length += camera_rt->lenses[i].thickness_short;
  }

  return total_length;
}


// focal_distance is in mm
void rt_logarithmic_focus_search(
  const float focal_distance, 
  float &best_sensor_shift, 
  float &closest_distance,
  const float lambda,
  CameraRaytraced *camera_rt)
{

  std::vector<float> log = logarithmic_values();

  for (float sensorshift : log){
  	float intersection_distance = 0.0f;
    //AiMsgInfo("----------------------");

    add_to_thickness_last_element(camera_rt->lenses, sensorshift, camera_rt->lenses_cnt, camera_rt->thickness_original);
    const float p_dist = lens_get_thickness(camera_rt->lenses + camera_rt->lenses_cnt-1, camera_rt->zoom);

    float pos[3] = {0.0f};
    float dir[3] = {0.0f};
    float ray_in[5] = {0.0};
    ray_in[2] = (camera_rt->p_rad*0.25 / p_dist) - (ray_in[0] / p_dist);
    ray_in[3] = (camera_rt->p_rad*0.25 / p_dist) - (ray_in[1] / p_dist);
    ray_in[4] = lambda;
    
    int error = evaluate_for_pos_dir(camera_rt->lenses, camera_rt->lenses_cnt, camera_rt->zoom, ray_in, 1, pos, dir, camera_rt->total_lens_length);
    if (error) continue;

    Eigen::Vector3d pos_eigen(pos[0], pos[1], pos[2]);
    Eigen::Vector3d dir_eigen(dir[0], dir[1], dir[2]);
    intersection_distance = line_plane_intersection(pos_eigen, dir_eigen)(2);
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


node_parameters
{
  AiParameterEnum("unitModel", cm, UnitModelNames);
  AiParameterEnum("lensModel", angenieux_double_gauss_1953_100mm, LensModelNames); // what to do here..? Can i not specify one?
  AiParameterFlt("sensor_width", 36.0); // 35mm film
  AiParameterFlt("wavelength", 550.0); // wavelength in nm
  AiParameterBool("dof", true);
  AiParameterFlt("fstop", 0.0);
  AiParameterFlt("focal_distance", 150.0); // in cm to be consistent with arnold core
  AiParameterFlt("extra_sensor_shift", 0.0);
  AiParameterInt("vignetting_retries", 15);
  AiParameterInt("aperture_blades", 0);
  AiParameterInt("backward_samples", 3);
  AiParameterFlt("minimum_rgb", 3.0f);
  AiParameterStr("bokeh_exr_path", "");
  AiParameterBool("proper_ray_derivatives", false);

  AiParameterInt("rt_lens_focal_length", 100);
  AiParameterStr("rt_lens_id", "0001");
  AiParameterFlt("rt_lens_zoom", 0.0f);

  AiParameterFlt("anamorphic_stretch", 2.0f);

}


node_initialize
{
  AiCameraInitialize(node);
  AiNodeSetLocalData(node, new Camera());
  AiNodeSetLocalData(node, new CameraRaytraced());
}


node_update
{   
  Camera* camera = (Camera*)AiNodeGetLocalData(node);
  CameraRaytraced* camera_rt = (CameraRaytraced*)AiNodeGetLocalData(node);

  camera_rt->lens_focal_length = AiNodeGetInt(node, "rt_lens_focal_length");
  camera_rt->id = AiNodeGetStr(node, "rt_lens_id");
  camera_rt->zoom = AiNodeGetFlt(node, "rt_lens_zoom");
  memset(camera_rt->lenses, 0, sizeof(camera_rt->lenses)); // not sure if the resetting is necessary?
  camera_rt->lenses_cnt = lens_configuration(camera_rt->lenses, camera_rt->id.c_str(), camera_rt->lens_focal_length);
  camera_rt->p_rad = camera_rt->lenses[camera_rt->lenses_cnt-1].housing_radius;


  // shift lens backwards by lens length so exit pupil vertex is at origin
  camera_rt->total_lens_length = get_lens_length(camera_rt);
  camera_rt->lenses[camera_rt->lenses_cnt-1].thickness_short -= camera_rt->total_lens_length;
  camera_rt->thickness_original = camera_rt->lenses[camera_rt->lenses_cnt-1].thickness_short;

  camera->sensor_width = AiNodeGetFlt(node, "sensor_width");
  camera->input_fstop = AiNodeGetFlt(node, "fstop");
  camera->focal_distance = AiNodeGetFlt(node, "focal_distance") * 10.0f; //convert to mm
  camera->lensModel = (LensModel) AiNodeGetInt(node, "lensModel");
  camera->unitModel = (UnitModel) AiNodeGetInt(node, "unitModel");
  //camera->aperture_blades = AiNodeGetInt(node, "aperture_blades");
  //camera->dof = AiNodeGetBool(node, "dof");
  camera->vignetting_retries = AiNodeGetInt(node, "vignetting_retries");
  //camera->backward_samples = AiNodeGetInt(node, "backward_samples");
  //camera->minimum_rgb = AiNodeGetFlt(node, "minimum_rgb");
  //camera->bokeh_exr_path = AiNodeGetStr(node, "bokeh_exr_path");
  //camera->proper_ray_derivatives = AiNodeGetBool(node, "proper_ray_derivatives");

  camera->anamorphic_stretch = AiNodeGetFlt(node, "anamorphic_stretch");

  // convert to cm
  switch (camera->unitModel){
    case mm:
    {
      camera->focal_distance *= 0.1f;
    } break;
    case dm:
    {
      camera->focal_distance *= 10.0f;
    } break;
    case m:
    {
      camera->focal_distance *= 100.0f;
    }
  }

  AiMsgInfo("");
  AiMsgInfo("[lentil raytraced] ----------  LENS CONSTANTS  -----------");
  AiMsgInfo("[lentil raytraced] lens name: %s", camera_rt->id.c_str()); //not sure why this isn't printing out.. weird
  //AiMsgInfo("[lentil raytraced] lens f-stop: %f", camera->lens_fstop);
  AiMsgInfo("[lentil raytraced] --------------------------------------");

  camera->lambda = AiNodeGetFlt(node, "wavelength") * 0.001;
  AiMsgInfo("[lentil raytraced] wavelength: %f", camera->lambda);
/*
  //camera->max_fstop = camera->lens_effective_focal_length / (camera->lens_aperture_housing_radius * 2.0f);
  AiMsgInfo("[lentil raytraced] lens wide open f-stop: %f", camera->lens_fstop);

  if (camera->input_fstop == 0.0f) camera->aperture_radius = camera->lens_aperture_housing_radius;
  // how do i get to proper aperture radius..?
  else camera->aperture_radius = fminf(camera->lens_aperture_housing_radius, camera->lens_effective_focal_length / (2.0f * camera->input_fstop));

  AiMsgInfo("[lentil raytraced] full aperture radius: %f", camera->lens_aperture_housing_radius);
  AiMsgInfo("[lentil raytraced] fstop-calculated aperture radius: %f", camera->aperture_radius);
  AiMsgInfo("[lentil raytraced] --------------------------------------");
*/

  AiMsgInfo("[lentil raytraced] focus distance (mm): %f", camera->focal_distance);

  // logartihmic focus search
  float best_sensor_shift = 0.0f;
  float closest_distance = AI_BIG;
  rt_logarithmic_focus_search(camera->focal_distance, best_sensor_shift, closest_distance, camera->lambda, camera_rt);
  AiMsgInfo("[lentil raytraced] sensor_shift using logarithmic search: %f", best_sensor_shift);
  camera->sensor_shift = best_sensor_shift + AiNodeGetFlt(node, "extra_sensor_shift");
  add_to_thickness_last_element(camera_rt->lenses, camera->sensor_shift, camera_rt->lenses_cnt, camera_rt->thickness_original); //is this needed or already set by log focus search?

/*
  // logarithmic infinity focus search
  float best_sensor_shift_infinity = 0.0f;
  float closest_distance_infinity = AI_BIG;
  logarithmic_focus_search(AI_BIG, best_sensor_shift_infinity, closest_distance_infinity, camera);
  AiMsgInfo("[lentil raytraced] sensor_shift [logarithmic forward tracing] to focus at infinity: %f", best_sensor_shift_infinity);
      
  // bidirectional parallel infinity focus search
  float infinity_focus_parallel_light_tracing = camera_set_focus_infinity(camera);
  AiMsgInfo("[lentil raytraced] sensor_shift [parallel backwards light tracing] to focus at infinity: %f", infinity_focus_parallel_light_tracing);


  // double check where y=0 intersection point is, should be the same as focus distance
  if(!trace_ray_focus_check(camera->sensor_shift, camera)){
    AiMsgWarning("[lentil raytraced] focus check failed. Either the lens system is not correct, or the sensor is placed at a wrong distance.");
  }


  camera->tan_fov = tanf(camera->lens_field_of_view / 2.0f);
*/


  // remove
  camera_rt->test_cnt = 0;
  camera_rt->test_maxcnt = 1000;
  camera_rt->test_intersection_distance = 0.0f;

#ifdef TIMING
  camera_rt->total_duration = 0;
  camera_rt->execution_counter = 0;
#endif

  AiMsgInfo("");
  AiCameraUpdate(node, false);
}


node_finish
{
  Camera* camera = (Camera*)AiNodeGetLocalData(node);
  CameraRaytraced* camera_rt = (CameraRaytraced*)AiNodeGetLocalData(node);

#ifdef TIMING
  AiMsgInfo("[lentil raytraced] Average execution time: %lld nanoseconds over %lld camera rays", camera_rt->total_duration / camera_rt->execution_counter, camera_rt->execution_counter);
#endif

  delete camera;

  // why can't i delete camera_rt without getting "pointer being freed was not allocated" error?
  // should be deleting my resources to avoid memory leaks...
  //delete camera_rt;
}


camera_create_ray
{
  Camera* camera = (Camera*)AiNodeGetLocalData(node);
  CameraRaytraced* camera_rt = (CameraRaytraced*)AiNodeGetLocalData(node);
  
#ifdef TIMING
  std::chrono::high_resolution_clock::time_point timer_start = std::chrono::high_resolution_clock::now();
#endif

  int tries = 0;
  bool ray_success = false;

  float pos[3] = {0.0f};
  float dir[3] = {0.0f};

  while(ray_success == false && tries <= camera->vignetting_retries)
  {
    add_to_thickness_last_element(camera_rt->lenses, camera->sensor_shift, camera_rt->lenses_cnt, camera_rt->thickness_original);

    // transform unit square to unit disk
    Eigen::Vector2d unit_disk(0.0f, 0.0f);
    if (tries == 0) concentric_disk_sample(input.lensx, input.lensy, unit_disk, false);
    else concentric_disk_sample(drand48(), drand48(), unit_disk, false);
    
    Eigen::Vector3d sensor_pos(input.sx * (camera->sensor_width * 0.5f),
                               input.sy * (camera->sensor_width * 0.5f),
                               0.0
    );
    // Eigen::Vector3d sensor_pos(0.0,//input.sx * (camera->sensor_width * 0.5f),
    //                            0.0,//input.sy * (camera->sensor_width * 0.5f),
    //                            0.0
    // );
    
    Eigen::Vector3d first_lens_element_pos(camera_rt->p_rad * unit_disk(0),
                                 camera_rt->p_rad * unit_disk(1),
                                 camera_rt->thickness_original
    );

    Eigen::Vector3d direction = first_lens_element_pos - sensor_pos;
    direction.normalize(); 
    
    float ray_in[5] = {sensor_pos(0), sensor_pos(1), direction(0), direction(1), camera->lambda};
  
    int error = evaluate_for_pos_dir(camera_rt->lenses, camera_rt->lenses_cnt, camera_rt->zoom, ray_in, 1, pos, dir, camera_rt->total_lens_length);
    if (error){
      ++tries;
      continue;
    }

    ray_success = true;
  }

  if (!ray_success){
    output.weight = {0.0, 0.0, 0.0};
    return;
  }
  
  for (int i = 0; i<3; i++){
    output.origin[i] = pos[i];
    output.dir[i] = dir[i];
  }

  /*
  Eigen::Vector3d pos_eigen(output.origin[0], output.origin[1], output.origin[2]);
  Eigen::Vector3d dir_eigen(output.dir[0], output.dir[1], output.dir[2]);
  camera_rt->test_intersection_distance += line_plane_intersection(pos_eigen, dir_eigen)(2);
  ++camera_rt->test_cnt;
  if (camera_rt->test_cnt == camera_rt->test_maxcnt){
    AiMsgInfo("full intersection distance: %f", camera_rt->test_intersection_distance);
    AiMsgInfo("average intersection distance: %f", camera_rt->test_intersection_distance/static_cast<double>(camera_rt->test_maxcnt));
    camera_rt->test_cnt = 0;
    camera_rt->test_intersection_distance = 0.0f;
  }*/
  
  switch (camera->unitModel) {
    case mm:
    {
      output.origin *= -1.0; // reverse rays and convert to cm
      output.dir *= -1.0; //reverse rays and convert to cm
    } break;
    case cm:
    { 
      output.origin *= -0.1; // reverse rays and convert to cm
      output.dir *= -0.1; //reverse rays and convert to cm
    } break;
    case dm:
    {
      output.origin *= -0.01; // reverse rays and convert to cm
      output.dir *= -0.01; //reverse rays and convert to cm
    } break;
    case m:
    {
      output.origin *= -0.001; // reverse rays and convert to cm
      output.dir *= -0.001; //reverse rays and convert to cm
    }
  }

  // anamorphics
  // output.dir[0] /= camera->anamorphic_stretch;

  AiV3Normalize(output.dir);

  // calculate new ray derivatives
  if (tries > 0){
    if (!camera->proper_ray_derivatives){
      for(int i=0; i<3; i++){
        output.dOdy[i] = output.origin[i];
        output.dDdy[i] = output.dir[i];
      }
    }
  }


  #ifdef TIMING
    std::chrono::high_resolution_clock::time_point timer_end = std::chrono::high_resolution_clock::now();
    camera_rt->total_duration += static_cast<long long int>(std::chrono::duration_cast<std::chrono::nanoseconds>( timer_end - timer_start ).count());
    camera_rt->execution_counter += 1;
  #endif
} 



// approximation using pinhole camera FOV
camera_reverse_ray
{
  /*
  const Camera* camera = (Camera*)AiNodeGetLocalData(node);

  float coeff = 1.0 / AiMax(fabsf(Po.z * camera->tan_fov), 1e-3f);
  Ps.x = Po.x * coeff;
  Ps.y = Po.y * coeff;

  return true;
  */
 return false;
}


node_loader
{
  if (i != 0) return false;
  node->methods = pota_raytracedMethods;
  node->output_type = AI_TYPE_UNDEFINED;
  node->name = "pota_raytraced";
  node->node_type = AI_NODE_CAMERA;
  strcpy(node->version, AI_VERSION);
  return true;
}