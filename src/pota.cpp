#include <ai.h>
#include "pota.h"
#include "lens.h"
#include <cmath>
#include <vector>



AI_CAMERA_NODE_EXPORT_METHODS(potaMethods)


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
  p_proper_ray_derivatives
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
  AiParameterBool("proper_ray_derivatives", true);
}


node_initialize
{
  AiCameraInitialize(node);
  AiNodeSetLocalData(node, new Camera());
}


node_update
{ 
  AiCameraUpdate(node, false);
  Camera* camera = (Camera*)AiNodeGetLocalData(node);

  camera->sensor_width = AiNodeGetFlt(node, "sensor_width");
  camera->input_fstop = AiNodeGetFlt(node, "fstop");
  camera->focal_distance = AiNodeGetFlt(node, "focal_distance") * 10.0f; //converting to mm
  camera->lensModel = (LensModel) AiNodeGetInt(node, "lensModel");
  camera->unitModel = (UnitModel) AiNodeGetInt(node, "unitModel");
  camera->aperture_blades = AiNodeGetInt(node, "aperture_blades");
  camera->dof = AiNodeGetBool(node, "dof");
  camera->vignetting_retries = AiNodeGetInt(node, "vignetting_retries");
  camera->backward_samples = AiNodeGetInt(node, "backward_samples");
  camera->minimum_rgb = AiNodeGetFlt(node, "minimum_rgb");
  camera->bokeh_exr_path = AiNodeGetStr(node, "bokeh_exr_path");
  camera->proper_ray_derivatives = AiNodeGetBool(node, "proper_ray_derivatives");

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

  load_lens_constants(camera);
  AiMsgInfo("[POTA] ----------  LENS CONSTANTS  -----------");
  AiMsgInfo("[POTA] Lens Name: %s", camera->lens_name);
  AiMsgInfo("[POTA] Lens F-Stop: %f", camera->lens_fstop);
#ifdef DEBUG_LOG
  AiMsgInfo("[POTA] lens_outer_pupil_radius: %f", camera->lens_outer_pupil_radius);
  AiMsgInfo("[POTA] lens_inner_pupil_radius: %f", camera->lens_inner_pupil_radius);
  AiMsgInfo("[POTA] lens_length: %f", camera->lens_length);
  AiMsgInfo("[POTA] lens_back_focal_length: %f", camera->lens_back_focal_length);
  AiMsgInfo("[POTA] lens_effective_focal_length: %f", camera->lens_effective_focal_length);
  AiMsgInfo("[POTA] lens_aperture_pos: %f", camera->lens_aperture_pos);
  AiMsgInfo("[POTA] lens_aperture_housing_radius: %f", camera->lens_aperture_housing_radius);
  AiMsgInfo("[POTA] lens_inner_pupil_curvature_radius: %f", camera->lens_inner_pupil_curvature_radius);
  AiMsgInfo("[POTA] lens_outer_pupil_curvature_radius: %f", camera->lens_outer_pupil_curvature_radius);
  AiMsgInfo("[POTA] lens_inner_pupil_geometry: %s", camera->lens_inner_pupil_geometry.c_str());
  AiMsgInfo("[POTA] lens_outer_pupil_geometry: %s", camera->lens_outer_pupil_geometry.c_str());
  AiMsgInfo("[POTA] lens_field_of_view: %f", camera->lens_field_of_view);
  AiMsgInfo("[POTA] lens_aperture_radius_at_fstop: %f", camera->lens_aperture_radius_at_fstop);
#endif
  AiMsgInfo("[POTA] --------------------------------------");


  camera->lambda = AiNodeGetFlt(node, "wavelength") * 0.001;
  AiMsgInfo("[POTA] wavelength: %f", camera->lambda);


  if (camera->input_fstop == 0.0f) {
    camera->aperture_radius = camera->lens_aperture_radius_at_fstop;
  } else {
    float calculated_fstop = 0.0f;
    float calculated_aperture_radius = 0.0f;
    trace_backwards_for_fstop(camera, camera->input_fstop, calculated_fstop, calculated_aperture_radius);
    
    AiMsgInfo("[POTA] calculated fstop: %f", calculated_fstop);
    AiMsgInfo("[POTA] calculated aperture radius: %f", calculated_aperture_radius);
    
    camera->aperture_radius = fminf(camera->lens_aperture_radius_at_fstop, calculated_aperture_radius);
  }

  AiMsgInfo("[POTA] lens wide open f-stop: %f", camera->lens_fstop);
  AiMsgInfo("[POTA] lens wide open aperture radius: %f", camera->lens_aperture_radius_at_fstop);
  AiMsgInfo("[POTA] fstop-calculated aperture radius: %f", camera->aperture_radius);
  AiMsgInfo("[POTA] --------------------------------------");


  AiMsgInfo("[POTA] focus distance: %f", camera->focal_distance);

  /*
  AiMsgInfo("[POTA] calculating sensor shift at focus distance:");
  camera->sensor_shift = camera_set_focus(camera->focal_distance, camera);
  AiMsgInfo("[POTA] sensor_shift to focus at %f: %f", camera->focal_distance, camera->sensor_shift);
  */

  // logartihmic focus search
  float best_sensor_shift = 0.0f;
  float closest_distance = AI_BIG;
  logarithmic_focus_search(camera->focal_distance, best_sensor_shift, closest_distance, camera);
  AiMsgInfo("[POTA] sensor_shift using logarithmic search: %f", best_sensor_shift);
  camera->sensor_shift = best_sensor_shift + AiNodeGetFlt(node, "extra_sensor_shift");

  /*
  // average guesses infinity focus search
  float infinity_focus_sensor_shift = camera_set_focus(AI_BIG, camera);
  AiMsgInfo("[POTA] sensor_shift [average guesses backwards light tracing] to focus at infinity: %f", infinity_focus_sensor_shift);
  */

  // logarithmic infinity focus search
  float best_sensor_shift_infinity = 0.0f;
  float closest_distance_infinity = AI_BIG;
  logarithmic_focus_search(AI_BIG, best_sensor_shift_infinity, closest_distance_infinity, camera);
  AiMsgInfo("[POTA] sensor_shift [logarithmic forward tracing] to focus at infinity: %f", best_sensor_shift_infinity);
      
  // bidirectional parallel infinity focus search
  float infinity_focus_parallel_light_tracing = camera_set_focus_infinity(camera);
  AiMsgInfo("[POTA] sensor_shift [parallel backwards light tracing] to focus at infinity: %f", infinity_focus_parallel_light_tracing);


  // double check where y=0 intersection point is, should be the same as focus distance
  if(!trace_ray_focus_check(camera->sensor_shift, camera)){
    AiMsgWarning("[POTA] focus check failed. Either the lens system is not correct, or the sensor is placed at a wrong distance.");
  }


  camera->tan_fov = tanf(camera->lens_field_of_view / 2.0f);

  // tmp for testing comparison to raytraced, remove!
  //camera->sensor_shift = 0.0f;


  AiMsgInfo("");
}


node_finish
{

  Camera* camera = (Camera*)AiNodeGetLocalData(node);

  delete camera;
}


camera_create_ray
{
  Camera* camera = (Camera*)AiNodeGetLocalData(node);

  int tries = 0;
  float random1 = 0.0;
  float random2 = 0.0;

  Eigen::Vector3d origin(output.origin[0], output.origin[1], output.origin[2]);
  Eigen::Vector3d direction(output.dir[0], output.dir[1], output.dir[2]);
  Eigen::Vector3d weight(output.weight[0], output.weight[1], output.weight[2]);

  trace_ray(true, tries, input.sx, input.sy, input.lensx, input.lensy, random1, random2, weight, origin, direction, camera);
  
  // calculate new ray derivatives
  // sucks a bit to have to trace 3 rays.. Bit slow
  // is there an analytical solution to this?..
  if (tries > 0){
    if (!camera->proper_ray_derivatives){
      for(int i=0; i<3; i++){
        output.dOdy[i] = origin(i);
        output.dDdy[i] = direction(i);
      }
    } else {
      float step = 0.001;
      AtCameraInput input_dx = input;
      AtCameraInput input_dy = input;
      AtCameraOutput output_dx;
      AtCameraOutput output_dy;

      input_dx.sx += input.dsx * step;
      input_dy.sy += input.dsy * step;

      // copy vectors
      Eigen::Vector3d out_dx_weight(output_dx.weight[0], output_dx.weight[1], output_dx.weight[2]);
      Eigen::Vector3d out_dx_origin(output_dx.origin[0], output_dx.origin[1], output_dx.origin[2]);
      Eigen::Vector3d out_dx_dir(output_dx.dir[0], output_dx.dir[1], output_dx.dir[2]);
      Eigen::Vector3d out_dy_weight(output_dy.weight[0], output_dy.weight[1], output_dy.weight[2]);
      Eigen::Vector3d out_dy_origin(output_dy.origin[0], output_dy.origin[1], output_dy.origin[2]);
      Eigen::Vector3d out_dy_dir(output_dy.dir[0], output_dy.dir[1], output_dy.dir[2]);

      trace_ray(false, tries, input_dx.sx, input_dx.sy, random1, random2, random1, random2, out_dx_weight, out_dx_origin, out_dx_dir, camera);
      trace_ray(false, tries, input_dy.sx, input_dy.sy, random1, random2, random1, random2, out_dy_weight, out_dy_origin, out_dy_dir, camera);

      Eigen::Vector3d out_d0dx = (out_dx_origin - origin) / step;
      Eigen::Vector3d out_dOdy = (out_dy_origin - origin) / step;
      Eigen::Vector3d out_dDdx = (out_dx_dir - direction) / step;
      Eigen::Vector3d out_dDdy = (out_dy_dir - direction) / step;

      for (int i = 0; i<3; i++){
        output.dOdx[i] = out_d0dx(i);
        output.dOdy[i] = out_dOdy(i);
        output.dDdx[i] = out_dDdx(i);
        output.dDdy[i] = out_dDdy(i);
      }
    }
  }

  // eigen->arnold
  for (int i = 0; i<3; i++){
    output.origin[i] = origin(i);
    output.dir[i] = direction(i);
    output.weight[i] = weight(i);
  }
  


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
  const float R = camera->lens_outer_pupil_curvature_radius;
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
   float sensor[5] =  {0.0f, 0.0f, 0.0f, 0.0f, camera->lambda};
   float out[5] =    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
   float aperture[2] =  {0.0f, 0.0f};

   //randoms are always 0?
   aperture[0] = camera->random1 * camera->aperture_radius;
   aperture[1] = camera->random2 * camera->aperture_radius;

   if(lens_lt_sample_aperture(target, aperture, sensor, out, camera->lambda, camera) <= 0.0f) return false;

   // crop at inward facing pupil, not needed to crop by outgoing because already done in lens_lt_sample_aperture()
   const float px = sensor[0] + sensor[2] * camera->lens_back_focal_length;
   const float py = sensor[1] + sensor[3] * camera->lens_back_focal_length; //(note that lens_focal_length is the back focal length, i.e. the distance unshifted sensor -> pupil)
   if (px*px + py*py > camera->lens_inner_pupil_radius*camera->lens_inner_pupil_radius) return false;

   // shift sensor
   sensor[0] += sensor[2] * -camera->sensor_shift;
   sensor[1] += sensor[3] * -camera->sensor_shift;

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
    AtVector2 s(sensor_position.x / (camera->sensor_width * 0.5), 
          sensor_position.y / (camera->sensor_width * 0.5) * frame_aspect_ratio);

    AiMsgInfo("sensorposition: %f \t %f", s.x, s.y);

    Ps.x = s.x;
    Ps.y = s.y;

    return true;
  } else {
    return false;
  }
}*/


// approximation using pinhole camera FOV
camera_reverse_ray
{
  const Camera* camera = (Camera*)AiNodeGetLocalData(node);

  float coeff = 1.0 / AiMax(fabsf(Po.z * camera->tan_fov), 1e-3f);
  Ps.x = Po.x * coeff;
  Ps.y = Po.y * coeff;

  return true;
}


node_loader
{
  if (i != 0) return false;
  node->methods = potaMethods;
  node->output_type = AI_TYPE_UNDEFINED;
  node->name = "pota";
  node->node_type = AI_NODE_CAMERA;
  strcpy(node->version, AI_VERSION);
  return true;
}