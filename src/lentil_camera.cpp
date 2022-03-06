#include <ai.h>
#include "lentil.h"
#include <vector>

AI_CAMERA_NODE_EXPORT_METHODS(lentilMethods)


static const char* Units[] = {"mm", "cm", "dm", "m", NULL};
static const char* CameraTypes[] = {"ThinLens", "PolynomialOptics", NULL};

// to switch between lens models in interface dropdown
static const char* LensModelNames[] = {
  #include "../include/auto_generated_lens_includes/pota_cpp_lenses.h"
  NULL
};


node_parameters {
  AiParameterEnum("camera_type", ThinLens, CameraTypes);
  AiParameterInt("bidir_sample_mult", 5);
  AiParameterEnum("units", cm, Units);
  AiParameterFlt("sensor_width", 36.0); // 35mm film
  AiParameterBool("enable_dof", true);
  AiParameterFlt("fstop", 0.0);
  AiParameterFlt("focus_dist", 150.0); // in cm to be consistent with arnold core
  AiParameterInt("aperture_blades_lentil", 0);
  AiParameterFlt("exp", 1.0);
  AiParameterEnum("lens_model", cooke__speed_panchro__1920__40mm, LensModelNames);
  AiParameterFlt("wavelength", 550.0); // wavelength in nm
  AiParameterFlt("extra_sensor_shift", 0.0);
  AiParameterFlt("focal_length_lentil", 35.0); // in mm
  AiParameterFlt("optical_vignetting", 0.0);
  // AiParameterFlt("optical_vignetting_radius", 1.0);
  AiParameterFlt("abb_spherical", 0.5);
  AiParameterFlt("abb_distortion", 0.0);
  AiParameterFlt("abb_coma", 0.0);
  AiParameterFlt("bokeh_circle_to_square", 0.0);
  AiParameterFlt("bokeh_anamorphic", 0.0);
  AiParameterBool("bokeh_enable_image", false);
  AiParameterStr("bokeh_image_path", "");
  AiParameterInt("vignetting_retries", 15);
  AiParameterFlt("bidir_add_energy", 0.0);
  AiParameterFlt("bidir_add_energy_minimum_luminance", 2.0);
  AiParameterFlt("bidir_add_energy_transition", 1.0);
  AiParameterBool("enable_bidir_transmission", false)

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

  if (camera_data->cameraType == ThinLens){
    camera_data->trace_ray_fw_thinlens(tries, input.sx, input.sy, origin, direction, weight, r1, r2, false);
  } else if (camera_data->cameraType == PolynomialOptics){
    camera_data->trace_ray_fw_po(tries, input.sx, input.sy, origin, direction, weight, r1, r2, false);
  }

  // if (tries > 0){
    float input_dx_sx = input.sx + (input.dsx * step);
    float input_dx_sy = input.sy + (input.dsy * step);

    AtVector output_dx_origin(0,0,0);
    AtVector output_dy_origin(0,0,0);
    AtVector output_dx_dir(0,0,0);
    AtVector output_dy_dir(0,0,0);
    AtRGB output_dx_weight = AI_RGB_WHITE;
    AtRGB output_dy_weight = AI_RGB_WHITE;
    
    if (camera_data->cameraType == ThinLens){
        camera_data->trace_ray_fw_thinlens(tries, input_dx_sx, input.sy, output_dx_origin, output_dx_dir, output_dx_weight, r1, r2, true);
        camera_data->trace_ray_fw_thinlens(tries, input.sx, input_dx_sy, output_dy_origin, output_dy_dir, output_dy_weight, r1, r2, true);
    } else if (camera_data->cameraType == PolynomialOptics){
        camera_data->trace_ray_fw_po(tries, input_dx_sx, input.sy, output_dx_origin, output_dx_dir, output_dx_weight, r1, r2, true);
        camera_data->trace_ray_fw_po(tries, input.sx, input_dx_sy, output_dy_origin, output_dy_dir, output_dy_weight, r1, r2, true);
    }

    output.dOdx = (output_dx_origin - origin) / step;
    output.dOdy = (output_dy_origin - origin) / step;
    output.dDdx = (output_dx_dir - direction) / step;
    output.dDdy = (output_dy_dir - direction) / step;
  // }
  
  
  output.origin = origin;
  output.dir = direction;
  output.weight = weight * camera_data->exposure;
} 


// solve Ps.xy with Po.xyz
// fully backtraced
// camera_reverse_ray
// {
//   Camera* camera = (Camera*)AiNodeGetLocalData(node);

//   //AiMsgInfo("Po = %f, %f, %f", Po.x, Po.y, Po.z);

//   int xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
//   int yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");
//   const float frame_aspect_ratio = (float)xres/(float)yres;

//   // convert sample world space position to camera space
//   AtMatrix world_to_camera_matrix;
//   AtVector2 sensor_position;
//   AiWorldToCameraMatrix(AiUniverseGetCamera(), relative_time, world_to_camera_matrix);
//   AtVector camera_space_sample_position = AiM4PointByMatrixMult(world_to_camera_matrix, Po);

//   if( trace_backwards( -camera_space_sample_position * 10.0, sensor_position, camera) )
//   {
//     AtVector2 s(sensor_position.x / (po->sensor_width * 0.5), 
//           sensor_position.y / (po->sensor_width * 0.5) * frame_aspect_ratio);

//     AiMsgInfo("sensorposition: %f \t %f", s.x, s.y);

//     Ps.x = s.x;
//     Ps.y = s.y;

//     return true;
//   } else {
//     return false;
//   }
// }


// approximation using pinhole camera FOV
camera_reverse_ray {
  Camera* camera_data = (Camera*)AiNodeGetLocalData(node);

  double coeff = 1.0 / std::max(std::abs(Po.z * camera_data->tan_fov), 1e-3);
  Ps.x = Po.x * coeff;
  Ps.y = Po.y * coeff;

  return true;
}



void registerLentilCamera(AtNodeLib* node) {
    node->methods = (AtNodeMethods*) lentilMethods;
    node->output_type = AI_TYPE_UNDEFINED;
    node->name = "lentil_camera";
    node->node_type = AI_NODE_CAMERA;
    strcpy(node->version, AI_VERSION);
}