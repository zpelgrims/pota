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
  // global
  AiParameterEnum("cameratype", ThinLens, CameraTypes);
  AiParameterEnum("units", cm, Units);
  AiParameterFlt("sensor_width", 36.0); // 35mm film
  AiParameterBool("enable_dof", true);
  AiParameterFlt("fstop", 0.0);
  AiParameterFlt("focus_dist", 150.0); // in cm to be consistent with arnold core


  // po specifics
  AiParameterEnum("lens_model", cooke__speed_panchro__1920__40mm, LensModelNames);
  AiParameterFlt("wavelength", 550.0); // wavelength in nm
  AiParameterFlt("extra_sensor_shift", 0.0);

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
  AiParameterInt("bidir_sample_mult", 5);
  AiParameterFlt("bidir_add_energy_minimum_luminance", 1.0);
  AiParameterFlt("bidir_add_energy", 0.0);
  AiParameterFlt("bidir_add_energy_transition", 1.0);

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

  /* 
  NOT NEEDED FOR ARNOLD (convert rays from camera space to world space), GOOD INFO THOUGH FOR OTHER RENDER ENGINES
  // initialise an ONB/a frame around the first vertex at the camera position along n=camera lookat direction:

  view_cam_init_frame(p, &p->v[0].hit);
      
  for(int k=0;k<3;k++)
  {
  //this is the world space position of the outgoing ray:
    p->v[0].hit.x[k] +=   camera_space_pos[0] * p->v[0].hit.a[k] 
              + camera_space_pos[1] * p->v[0].hit.b[k] 
              + camera_space_pos[2] * p->v[0].hit.n[k];

  //this is the world space direction of the outgoing ray:
    p->e[1].omega[k] =   camera_space_omega[0] * p->v[0].hit.a[k] 
               + camera_space_omega[1] * p->v[0].hit.b[k]
               + camera_space_omega[2] * p->v[0].hit.n[k];
  }

  // now need to rotate the normal of the frame, in case you need any cosines later in light transport. if not, leave out:
  const float R = po->lens_outer_pupil_curvature_radius;
  // recompute full frame:
  float n[3] = {0.0f};
  for(int k=0;k<3;k++)
    n[k] +=   p->v[0].hit.a[k] * out[0]/R 
        + p->v[0].hit.b[k] * out[1]/R 
        + p->v[0].hit.n[k] * (out[2] + R)/fabsf(R);

  for(int k=0;k<3;k++) p->v[0].hit.n[k] = n[k];

  // also clip to valid pixel range.
  if(p->sensor.pixel_i < 0.0f || p->sensor.pixel_i >= view_width() ||
  p->sensor.pixel_j < 0.0f || p->sensor.pixel_j >= view_height())
  return 0.0f;
  */
} 


/*
// given camera space scene point, return point on sensor
inline bool trace_backwards(const AtVector sample_position, AtVector2 &sensor_position, Camera *camera)
{
   const float target[3] = {sample_position.x, sample_position.y, sample_position.z};

   // initialize 5d light fields
   float sensor[5] =  {0.0f, 0.0f, 0.0f, 0.0f, po->lambda};
   float out[5] =    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
   float aperture[2] =  {0.0f, 0.0f};

   //randoms are always 0?
   aperture[0] = po->random1 * po->aperture_radius;
   aperture[1] = po->random2 * po->aperture_radius;

   if(lens_lt_sample_aperture(target, aperture, sensor, out, po->lambda, camera) <= 0.0f) return false;

   // crop at inward facing pupil, not needed to crop by outgoing because already done in lens_lt_sample_aperture()
   const float px = sensor[0] + sensor[2] * po->lens_back_focal_length;
   const float py = sensor[1] + sensor[3] * po->lens_back_focal_length; //(note that lens_focal_length is the back focal length, i.e. the distance unshifted sensor -> pupil)
   if (px*px + py*py > po->lens_inner_pupil_radius*po->lens_inner_pupil_radius) return false;

   // shift sensor
   sensor[0] += sensor[2] * -po->sensor_shift;
   sensor[1] += sensor[3] * -po->sensor_shift;

   sensor_position.x = sensor[0];
   sensor_position.y = sensor[1];

   return true;
}



// solve Ps.xy with Po.xyz
// fully backtraced
camera_reverse_ray
{
  Camera* camera = (Camera*)AiNodeGetLocalData(node);

  //AiMsgInfo("Po = %f, %f, %f", Po.x, Po.y, Po.z);

  int xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
  int yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");
  const float frame_aspect_ratio = (float)xres/(float)yres;

  // convert sample world space position to camera space
  AtMatrix world_to_camera_matrix;
  AtVector2 sensor_position;
  AiWorldToCameraMatrix(AiUniverseGetCamera(), relative_time, world_to_camera_matrix);
  AtVector camera_space_sample_position = AiM4PointByMatrixMult(world_to_camera_matrix, Po);

  if( trace_backwards( -camera_space_sample_position * 10.0, sensor_position, camera) )
  {
    AtVector2 s(sensor_position.x / (po->sensor_width * 0.5), 
          sensor_position.y / (po->sensor_width * 0.5) * frame_aspect_ratio);

    AiMsgInfo("sensorposition: %f \t %f", s.x, s.y);

    Ps.x = s.x;
    Ps.y = s.y;

    return true;
  } else {
    return false;
  }
}*/


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