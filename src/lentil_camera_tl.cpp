#include <ai.h>
#include "lentil.h"
#include <vector>

AI_CAMERA_NODE_EXPORT_METHODS(lentilThinLensMethods)


static const char* Units[] = {"mm", "cm", "dm", "m", NULL};
// static const char* CameraTypes[] = {"ThinLens", "PolynomialOptics", NULL};

// // to switch between lens models in interface dropdown
// static const char* LensModelNames[] = {
//   #include "../include/auto_generated_lens_includes/pota_cpp_lenses.h"
//   NULL
// };


node_parameters {
  // global
  // AiParameterEnum("cameratype", ThinLens, CameraTypes);
  AiParameterEnum("units", cm, Units);
  AiParameterFlt("sensor_width", 36.0); // 35mm film
  AiParameterBool("enable_dof", true);
  AiParameterFlt("fstop", 0.0);
  AiParameterFlt("focus_dist", 150.0); // in cm to be consistent with arnold core


  // po specifics
  // AiParameterEnum("lens_model", cooke__speed_panchro__1920__40mm, LensModelNames);
  // AiParameterFlt("wavelength", 550.0); // wavelength in nm
  // AiParameterFlt("extra_sensor_shift", 0.0);

  // tl specifics
  AiParameterFlt("focal_length_lentil", 35.0); // in mm
  AiParameterFlt("optical_vignetting_distance", 0.0);
  AiParameterFlt("optical_vignetting_radius", 2.0);
  AiParameterFlt("abb_spherical", 0.5);
  AiParameterFlt("abb_distortion", 0.0);
  AiParameterFlt("bokeh_circle_to_square", 0.0);
  AiParameterFlt("bokeh_anamorphic", 1.0);

  // bokeh
  AiParameterInt("bokeh_aperture_blades", 0);
  AiParameterBool("bokeh_enable_image", false);
  AiParameterStr("bokeh_image_path", "");

  // bidir
  AiParameterFlt("bidir_min_luminance", 1.0);
  AiParameterInt("bidir_sample_mult", 10);
  AiParameterFlt("bidir_add_luminance", 0.0);
  AiParameterFlt("bidir_add_luminance_transition", 1.0);

  // advanced
  AiParameterInt("vignetting_retries", 15);
  AiParameterFlt("exp", 1.0);

  // experimental
  AiParameterFlt("abb_coma", 0.0);

  AiMetaDataSetBool(nentry, nullptr, "force_update", true);
}


node_plugin_initialize {return lentil_crit_sec_init();}
node_plugin_cleanup {lentil_crit_sec_close();}
node_initialize {
  AiCameraInitialize(node);
  AiNodeSetLocalData(node, new Camera());
}

node_update { 
  Camera* camera_data = (Camera*)AiNodeGetLocalData(node);
  AtUniverse *universe = AiNodeGetUniverse(node);
  camera_data->setup_all(universe);
  AiCameraUpdate(node, false);
}

node_finish {
  Camera* camera_data = (Camera*)AiNodeGetLocalData(node);
  delete camera_data;
}


camera_create_ray {
  Camera* camera_data = (Camera*)AiNodeGetLocalData(node);
  
  int tries = 0;
  double r1 = input.lensx;
  double r2 = input.lensy; 
  const float step = 0.001;

  AtVector origin(0,0,0);
  AtVector direction(0,0,0);
  AtRGB weight(1,1,1);

  // if (camera_data->cameraType == ThinLens){
    camera_data->trace_ray_fw_thinlens(tries, input.sx, input.sy, origin, direction, weight, r1, r2, false);
  // } 
  // else if (camera_data->cameraType == PolynomialOptics){
  //   camera_data->trace_ray_fw_po(tries, input.sx, input.sy, origin, direction, weight, r1, r2, false);
  // }

  if (tries > 0){
    float input_dx_sx = input.sx + (input.dsx * step);
    float input_dx_sy = input.sy + (input.dsy * step);

    AtVector output_dx_origin(0,0,0);
    AtVector output_dy_origin(0,0,0);
    AtVector output_dx_dir(0,0,0);
    AtVector output_dy_dir(0,0,0);
    AtRGB output_dx_weight = AI_RGB_WHITE;
    AtRGB output_dy_weight = AI_RGB_WHITE;
    
    // if (camera_data->cameraType == ThinLens){
        camera_data->trace_ray_fw_thinlens(tries, input_dx_sx, input.sy, output_dx_origin, output_dx_dir, output_dx_weight, r1, r2, true);
        camera_data->trace_ray_fw_thinlens(tries, input.sx, input_dx_sy, output_dy_origin, output_dy_dir, output_dy_weight, r1, r2, true);
    // } 
    // else if (camera_data->cameraType == PolynomialOptics){
    //     camera_data->trace_ray_fw_po(tries, input_dx_sx, input.sy, output_dx_origin, output_dx_dir, output_dx_weight, r1, r2, true);
    //     camera_data->trace_ray_fw_po(tries, input.sx, input_dx_sy, output_dy_origin, output_dy_dir, output_dy_weight, r1, r2, true);
    // }

    output.dOdx = (output_dx_origin - origin) / step;
    output.dOdy = (output_dy_origin - origin) / step;
    output.dDdx = (output_dx_dir - direction) / step;
    output.dDdy = (output_dy_dir - direction) / step;
  }
  
  
  output.origin = origin;
  output.dir = direction;
  output.weight = weight * camera_data->exposure;
} 



// approximation using pinhole camera FOV
camera_reverse_ray {
  Camera* camera_data = (Camera*)AiNodeGetLocalData(node);

  double coeff = 1.0 / std::max(std::abs(Po.z * camera_data->tan_fov), 1e-3);
  Ps.x = Po.x * coeff;
  Ps.y = Po.y * coeff;

  return true;
}



void registerLentilCameraThinLens(AtNodeLib* node) {
    node->methods = (AtNodeMethods*) lentilThinLensMethods;
    node->output_type = AI_TYPE_UNDEFINED;
    node->name = "lentil_camera_tl";
    node->node_type = AI_NODE_CAMERA;
    strcpy(node->version, AI_VERSION);
}