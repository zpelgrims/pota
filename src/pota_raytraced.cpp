#include <ai.h>
#include "pota.h"
#include "lens.h"
#include <cmath>
#include <string>
#include <chrono>
#include "../../Eigen/Eigen/Dense"

//#define TIMING


AI_CAMERA_NODE_EXPORT_METHODS(pota_raytracedMethods)


enum {
  p_unitModel,
  p_lensModel,
  p_sensor_width,
  p_wavelength,
  p_dof,
  p_fstop,
  p_focus_distance,
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
static const char* LensModelNames[] = {
  #include "../include/auto_generated_lens_includes/pota_cpp_lenses.h"
  NULL
};

// to switch between units in interface dropdown
static const char* UnitModelNames[] = {"mm", "cm", "dm", "m", NULL};


double get_lens_length(Camera *camera) {
  double total_length = 0.0;
  for (int i = 0; i < camera->camera_rt.lenses_cnt; i++) {
    total_length += camera->camera_rt.lenses[i].thickness_short;
  }

  return total_length;
}


// focus_distance is in mm
void rt_logarithmic_focus_search(
  const double focus_distance, 
  double &best_sensor_shift, 
  double &closest_distance,
  const double lambda,
  Camera *camera)
{

  std::vector<double> log = logarithmic_values();

  for (double sensorshift : log){
  	double intersection_distance = 0.0;

    add_to_thickness_last_element(camera->camera_rt.lenses, sensorshift, camera->camera_rt.lenses_cnt, camera->camera_rt.thickness_original);
    const double p_dist = lens_get_thickness(camera->camera_rt.lenses[camera->camera_rt.lenses_cnt-1], camera->camera_rt.zoom);

    Eigen::Vector3d pos(0,0,0);
    Eigen::Vector3d dir(0,0,0);
    Eigen::VectorXd ray_in(5); ray_in.setZero();
    ray_in(2) = (camera->camera_rt.p_rad*0.25 / p_dist) - (ray_in(0) / p_dist);
    ray_in(3) = (camera->camera_rt.p_rad*0.25 / p_dist) - (ray_in(1) / p_dist);
    ray_in(4) = lambda;
    
    int error = evaluate_for_pos_dir(camera->camera_rt.lenses, camera->camera_rt.lenses_cnt, camera->camera_rt.zoom, ray_in, 1, pos, dir, camera->camera_rt.total_lens_length, false);
    if (error) continue;

    intersection_distance = line_plane_intersection(pos, dir)(2);
    double new_distance = focus_distance - intersection_distance;


    if (new_distance < closest_distance && new_distance > 0.0){
      closest_distance = new_distance;
      best_sensor_shift = sensorshift;
    }
  }
}


node_parameters {
  AiParameterEnum("unitModel", cm, UnitModelNames);
  AiParameterEnum("lensModel", angenieux_double_gauss_1953_49mm, LensModelNames); // what to do here..? Can i not specify one?
  AiParameterFlt("sensor_width", 36.0); // 35mm film
  AiParameterFlt("wavelength", 550.0); // wavelength in nm
  AiParameterBool("dof", true);
  AiParameterFlt("fstop", 0.0);
  AiParameterFlt("focus_distance", 150.0); // in cm to be consistent with arnold core
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


node_initialize {
  AiCameraInitialize(node);
  AiNodeSetLocalData(node, new Camera());
}


node_update {
  Camera* camera = (Camera*)AiNodeGetLocalData(node);

  camera->camera_rt.lens_focal_length = AiNodeGetInt(node, "rt_lens_focal_length");
  camera->camera_rt.id = AiNodeGetStr(node, "rt_lens_id");
  camera->camera_rt.zoom = AiNodeGetFlt(node, "rt_lens_zoom");
  camera->camera_rt.lenses_cnt = lens_configuration(camera->camera_rt.lenses, camera->camera_rt.id.c_str(), camera->camera_rt.lens_focal_length);
  camera->camera_rt.p_rad = camera->camera_rt.lenses[camera->camera_rt.lenses_cnt-1].housing_radius;


  // shift lens backwards by lens length so exit pupil vertex is at origin
  camera->camera_rt.total_lens_length = get_lens_length(camera);
  camera->camera_rt.lenses[camera->camera_rt.lenses_cnt-1].thickness_short -= camera->camera_rt.total_lens_length;
  camera->camera_rt.thickness_original = camera->camera_rt.lenses[camera->camera_rt.lenses_cnt-1].thickness_short;

  camera->sensor_width = AiNodeGetFlt(node, "sensor_width");
  camera->input_fstop = AiNodeGetFlt(node, "fstop");
  camera->focus_distance = AiNodeGetFlt(node, "focus_distance") * 10.0; //convert to mm
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
      camera->focus_distance *= 0.1;
    } break;
    case dm:
    {
      camera->focus_distance *= 10.0;
    } break;
    case m:
    {
      camera->focus_distance *= 100.0;
    }
  }

  AiMsgInfo("");
  AiMsgInfo("[lentil raytraced] ----------  LENS CONSTANTS  -----------");
  //AiMsgInfo("[lentil raytraced] lens name: %s", camera->camera_rt.id.c_str()); //not sure why this isn't printing out.. weird
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

  AiMsgInfo("[lentil raytraced] focus distance (mm): %f", camera->focus_distance);

  // logartihmic focus search
  double best_sensor_shift = 0.0;
  double closest_distance = AI_BIG;
  rt_logarithmic_focus_search(camera->focus_distance, best_sensor_shift, closest_distance, camera->lambda, camera);
  AiMsgInfo("[lentil raytraced] sensor_shift using logarithmic search: %f", best_sensor_shift);
  camera->sensor_shift = best_sensor_shift + AiNodeGetFlt(node, "extra_sensor_shift");
  add_to_thickness_last_element(camera->camera_rt.lenses, camera->sensor_shift, camera->camera_rt.lenses_cnt, camera->camera_rt.thickness_original); //is this needed or already set by log focus search?

  camera->tan_fov = std::tan(camera->lens_field_of_view / 2.0);

#ifdef TIMING
  camera->camera_rt.total_duration = 0;
  camera->camera_rt.execution_counter = 0;
#endif

  AiMsgInfo("");
  AiCameraUpdate(node, false);
}


node_finish {
  Camera* camera = (Camera*)AiNodeGetLocalData(node);

#ifdef TIMING
  AiMsgInfo("[lentil raytraced] Average execution time: %lld nanoseconds over %lld camera rays", camera->camera_rt.total_duration / camera->camera_rt.execution_counter, camera->camera_rt.execution_counter);
#endif

  delete camera;
}


camera_create_ray {
  Camera* camera = (Camera*)AiNodeGetLocalData(node);
  
#ifdef TIMING
  std::chrono::high_resolution_clock::time_point timer_start = std::chrono::high_resolution_clock::now();
#endif

  int tries = 0;
  bool ray_success = false;

  Eigen::Vector3d pos(0,0,0);
  Eigen::Vector3d dir(0,0,0);

  while(ray_success == false && tries <= camera->vignetting_retries) {
    
    Eigen::Vector3d sensor_pos(input.sx * (camera->sensor_width * 0.5),
                               input.sy * (camera->sensor_width * 0.5),
                               0.0
    );

    // transform unit square to unit disk
    Eigen::Vector2d unit_disk(0.0, 0.0);
    if (tries == 0) concentric_disk_sample(input.lensx, input.lensy, unit_disk, false);
    else concentric_disk_sample(xor128() / 4294967296.0, xor128() / 4294967296.0, unit_disk, true);
    
    // p_rad should cover -1, 1 in this config.. not sure why i have to scale it up further.. debug
    Eigen::Vector3d first_lens_element_pos(camera->camera_rt.p_rad * unit_disk(0),
                                           camera->camera_rt.p_rad * unit_disk(1),
                                           camera->camera_rt.thickness_original
    );

    Eigen::Vector3d direction = first_lens_element_pos - sensor_pos;
    direction.normalize(); 
    
    Eigen::VectorXd ray_in(5); ray_in << sensor_pos(0), sensor_pos(1), direction(0), direction(1), camera->lambda;

    add_to_thickness_last_element(camera->camera_rt.lenses, camera->sensor_shift, camera->camera_rt.lenses_cnt, camera->camera_rt.thickness_original);

    int error = evaluate_for_pos_dir(camera->camera_rt.lenses, camera->camera_rt.lenses_cnt, camera->camera_rt.zoom, ray_in, 1, pos, dir, camera->camera_rt.total_lens_length, false);//camera->camera_rt.position_list, camera->camera_rt.direction_list, false);
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
    output.origin[i] = pos(i);
    output.dir[i] = dir(i);
  }

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
    camera->camera_rt.total_duration += static_cast<long long int>(std::chrono::duration_cast<std::chrono::nanoseconds>( timer_end - timer_start ).count());
    camera->camera_rt.execution_counter += 1;
  #endif
} 



// approximation using pinhole camera FOV
camera_reverse_ray
{
  const Camera* camera = (Camera*)AiNodeGetLocalData(node);

  float coeff = 1.0 / std::max(std::abs(Po.z * camera->tan_fov), 0.001);
  Ps.x = Po.x * coeff;
  Ps.y = Po.y * coeff;

  return true;
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