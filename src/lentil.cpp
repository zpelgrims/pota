#include <ai.h>
#include "lentil.h"
#include "lens.h"
#include <cmath>
#include <vector>

AI_CAMERA_NODE_EXPORT_METHODS(lentilMethods)


enum {
  p_units,
  p_lens_model,
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
  p_use_image,
  p_bokeh_input_path,
  p_empirical_ca_dist
};


// to switch between lens models in interface dropdown
// this will need to be automatically filled somehow
static const char* LensModelNames[] = {
  #include "../include/auto_generated_lens_includes/pota_cpp_lenses.h"
  NULL
};

// to switch between units in interface dropdown
static const char* Units[] = {"mm", "cm", "dm", "m", NULL};


node_parameters {
  AiParameterEnum("units", cm, Units);
  AiParameterEnum("lens_model", angenieux__double_gauss__1953__49mm, LensModelNames); // what to do here..? Can i not specify one?
  AiParameterFlt("sensor_width", 36.0); // 35mm film
  AiParameterFlt("wavelength", 550.0); // wavelength in nm
  AiParameterBool("dof", true);
  AiParameterFlt("fstop", 0.0);
  AiParameterFlt("focus_distance", 150.0); // in cm to be consistent with arnold core
  AiParameterFlt("extra_sensor_shift", 0.0);
  AiParameterInt("vignetting_retries", 15);
  AiParameterInt("aperture_blades", 0);
  AiParameterInt("backward_samples", 3);
  AiParameterFlt("minimum_rgb", 2.0);
  AiParameterStr("bokeh_exr_path", "");
  AiParameterBool("proper_ray_derivatives", true);
  AiParameterBool("use_image", false);
  AiParameterStr("bokeh_input_path", "");
  AiParameterFlt("empirical_ca_dist", 0.0);
}


node_initialize {
  AiCameraInitialize(node);
  AiNodeSetLocalData(node, new Camera());
}


node_update { 
  AiCameraUpdate(node, false);
  Camera* camera = (Camera*)AiNodeGetLocalData(node);

  camera->sensor_width = AiNodeGetFlt(node, "sensor_width");
  camera->input_fstop = AiNodeGetFlt(node, "fstop");
  camera->focus_distance = AiNodeGetFlt(node, "focus_distance") * 10.0; //converting to mm
  camera->lensModel = (LensModel) AiNodeGetInt(node, "lens_model");
  camera->unitModel = (UnitModel) AiNodeGetInt(node, "units");
  camera->aperture_blades = AiNodeGetInt(node, "aperture_blades");
  camera->dof = AiNodeGetBool(node, "dof");
  camera->vignetting_retries = AiNodeGetInt(node, "vignetting_retries");
  camera->backward_samples = AiNodeGetInt(node, "backward_samples");
  camera->minimum_rgb = AiNodeGetFlt(node, "minimum_rgb");
  camera->bokeh_exr_path = AiNodeGetStr(node, "bokeh_exr_path");
  camera->proper_ray_derivatives = AiNodeGetBool(node, "proper_ray_derivatives");
  camera->use_image = AiNodeGetBool(node, "use_image");
  camera->bokeh_input_path = AiNodeGetStr(node, "bokeh_input_path");
  camera->empirical_ca_dist = AiNodeGetFlt(node, "empirical_ca_dist") / 2500.0; //2500 is empirically set, not sure if correct for all cases?
  
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

  load_lens_constants(camera);
  AiMsgInfo("[LENTIL] ----------  LENS CONSTANTS  -----------");
  AiMsgInfo("[LENTIL] Lens Name: %s", camera->lens_name);
  AiMsgInfo("[LENTIL] Lens F-Stop: %f", camera->lens_fstop);
#ifdef DEBUG_LOG
  AiMsgInfo("[LENTIL] lens_outer_pupil_radius: %f", camera->lens_outer_pupil_radius);
  AiMsgInfo("[LENTIL] lens_inner_pupil_radius: %f", camera->lens_inner_pupil_radius);
  AiMsgInfo("[LENTIL] lens_length: %f", camera->lens_length);
  AiMsgInfo("[LENTIL] lens_back_focal_length: %f", camera->lens_back_focal_length);
  AiMsgInfo("[LENTIL] lens_effective_focal_length: %f", camera->lens_effective_focal_length);
  AiMsgInfo("[LENTIL] lens_aperture_pos: %f", camera->lens_aperture_pos);
  AiMsgInfo("[LENTIL] lens_aperture_housing_radius: %f", camera->lens_aperture_housing_radius);
  AiMsgInfo("[LENTIL] lens_inner_pupil_curvature_radius: %f", camera->lens_inner_pupil_curvature_radius);
  AiMsgInfo("[LENTIL] lens_outer_pupil_curvature_radius: %f", camera->lens_outer_pupil_curvature_radius);
  AiMsgInfo("[LENTIL] lens_inner_pupil_geometry: %s", camera->lens_inner_pupil_geometry.c_str());
  AiMsgInfo("[LENTIL] lens_outer_pupil_geometry: %s", camera->lens_outer_pupil_geometry.c_str());
  AiMsgInfo("[LENTIL] lens_field_of_view: %f", camera->lens_field_of_view);
  AiMsgInfo("[LENTIL] lens_aperture_radius_at_fstop: %f", camera->lens_aperture_radius_at_fstop);
#endif
  AiMsgInfo("[LENTIL] --------------------------------------");


  camera->lambda = AiNodeGetFlt(node, "wavelength") * 0.001;
  AiMsgInfo("[LENTIL] wavelength: %f nm", camera->lambda);


  if (camera->input_fstop == 0.0) {
    camera->aperture_radius = camera->lens_aperture_radius_at_fstop;
  } else {
    double calculated_fstop = 0.0;
    double calculated_aperture_radius = 0.0;
    trace_backwards_for_fstop(camera, camera->input_fstop, calculated_fstop, calculated_aperture_radius);
    
    AiMsgInfo("[LENTIL] calculated fstop: %f", calculated_fstop);
    AiMsgInfo("[LENTIL] calculated aperture radius: %f mm", calculated_aperture_radius);
    
    camera->aperture_radius = std::min(camera->lens_aperture_radius_at_fstop, calculated_aperture_radius);
  }

  AiMsgInfo("[LENTIL] lens wide open f-stop: %f", camera->lens_fstop);
  AiMsgInfo("[LENTIL] lens wide open aperture radius: %f mm", camera->lens_aperture_radius_at_fstop);
  AiMsgInfo("[LENTIL] fstop-calculated aperture radius: %f mm", camera->aperture_radius);
  AiMsgInfo("[LENTIL] --------------------------------------");


  AiMsgInfo("[LENTIL] user supplied focus distance: %f mm", camera->focus_distance);

  /*
  AiMsgInfo("[LENTIL] calculating sensor shift at focus distance:");
  camera->sensor_shift = camera_set_focus(camera->focus_distance, camera);
  AiMsgInfo("[LENTIL] sensor_shift to focus at %f: %f", camera->focus_distance, camera->sensor_shift);
  */

  // logartihmic focus search
  double best_sensor_shift = logarithmic_focus_search(camera->focus_distance, camera);
  AiMsgInfo("[LENTIL] sensor_shift using logarithmic search: %f mm", best_sensor_shift);
  camera->sensor_shift = best_sensor_shift + AiNodeGetFlt(node, "extra_sensor_shift");

  /*
  // average guesses infinity focus search
  double infinity_focus_sensor_shift = camera_set_focus(AI_BIG, camera);
  AiMsgInfo("[LENTIL] sensor_shift [average guesses backwards light tracing] to focus at infinity: %f", infinity_focus_sensor_shift);
  */

  // logarithmic infinity focus search
  double best_sensor_shift_infinity = logarithmic_focus_search(999999999.0, camera);
  AiMsgInfo("[LENTIL] sensor_shift [logarithmic forward tracing] to focus at infinity: %f mm", best_sensor_shift_infinity);
      
  // bidirectional parallel infinity focus search
  double infinity_focus_parallel_light_tracing = camera_set_focus_infinity(camera);
  AiMsgInfo("[LENTIL] sensor_shift [parallel backwards light tracing] to focus at infinity: %f mm", infinity_focus_parallel_light_tracing);

  // double check where y=0 intersection point is, should be the same as focus distance
  double test_focus_distance = 0.0;
  bool focus_test = trace_ray_focus_check(camera->sensor_shift, test_focus_distance, camera);
  AiMsgInfo("[LENTIL] focus test ray: %f mm", test_focus_distance);
  if(!focus_test){
    AiMsgWarning("[LENTIL] focus check failed. Either the lens system is not correct, or the sensor is placed at a wrong distance.");
  }

  camera->tan_fov = std::tan(camera->lens_field_of_view / 2.0);

  AiMsgInfo("[LENTIL] --------------------------------------");
  

  // make probability functions of the bokeh image
  // if (parms.bokehChanged(camera->params)) {
    camera->image.invalidate();
    if (camera->use_image && !camera->image.read(camera->bokeh_input_path.c_str())){
      AiMsgError("[LENTIL] Couldn't open bokeh image!");
      AiRenderAbort();
    }
  // }
  AiMsgInfo("");
}


node_finish {
  Camera* camera = (Camera*)AiNodeGetLocalData(node);
  delete camera;
}


camera_create_ray {
  Camera* camera = (Camera*)AiNodeGetLocalData(node);

  int tries = 0;
  double random1 = 0.0, random2 = 0.0; 
  Eigen::Vector3d origin(0, 0, 0);
  Eigen::Vector3d direction(0, 0, 0);
  Eigen::Vector3d weight(1, 1, 1);

  trace_ray(true, tries, input.sx, input.sy, input.lensx, input.lensy, random1, random2, weight, origin, direction, camera);
  
  // calculate new ray derivatives
  // sucks a bit to have to trace 3 rays.. Bit slow
  // is there an analytical solution to this?..
  if (tries > 0){
    if (!camera->proper_ray_derivatives){
      for(int i=0; i<3; i++){
        output.dOdx[i] = origin(i); // is this necessary?
        output.dOdy[i] = origin(i);
        output.dDdx[i] = direction(i); // is this necessary?
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

      Eigen::Vector3d out_dx_weight(output_dx.weight[0], output_dx.weight[1], output_dx.weight[2]);
      Eigen::Vector3d out_dx_origin(output_dx.origin[0], output_dx.origin[1], output_dx.origin[2]);
      Eigen::Vector3d out_dx_dir(output_dx.dir[0], output_dx.dir[1], output_dx.dir[2]);
      trace_ray(false, tries, input_dx.sx, input_dx.sy, random1, random2, random1, random2, out_dx_weight, out_dx_origin, out_dx_dir, camera);

      Eigen::Vector3d out_dy_weight(output_dy.weight[0], output_dy.weight[1], output_dy.weight[2]);
      Eigen::Vector3d out_dy_origin(output_dy.origin[0], output_dy.origin[1], output_dy.origin[2]);
      Eigen::Vector3d out_dy_dir(output_dy.dir[0], output_dy.dir[1], output_dy.dir[2]);
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

  for (int i = 0; i<3; i++){
    output.origin[i] = origin(i);
    output.dir[i] = direction(i);
    output.weight[i] = weight(i);
  }

  // tmp debug
  //printf("[%f, %f, %f],", origin(0), origin(1), origin(2));
  


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
camera_reverse_ray {
  const Camera* camera = (Camera*)AiNodeGetLocalData(node);

  double coeff = 1.0 / AiMax(fabs(Po.z * camera->tan_fov), 1e-3);
  Ps.x = Po.x * coeff;
  Ps.y = Po.y * coeff;

  return true;
}


node_loader {
  if (i != 0) return false;
  node->methods = lentilMethods;
  node->output_type = AI_TYPE_UNDEFINED;
  node->name = "lentil";
  node->node_type = AI_NODE_CAMERA;
  strcpy(node->version, AI_VERSION);
  return true;
}
