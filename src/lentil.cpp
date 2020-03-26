#include <ai.h>
#include "lentil.h"
#include "lens.h"
#include <cmath>
#include <vector>

AI_CAMERA_NODE_EXPORT_METHODS(lentilMethods)


enum {
  p_lens_modelPO,

  p_sensor_widthPO,
  p_wavelengthPO,
  p_dofPO,
  p_fstopPO,
  p_focus_distancePO,
  p_extra_sensor_shiftPO,

  p_bokeh_aperture_bladesPO,
  p_bokeh_enable_imagePO,
  p_bokeh_image_pathPO,

  p_bidir_min_luminancePO,
  p_bidir_output_pathPO,
  p_bidir_sample_multPO,
  p_bidir_add_luminancePO,
  p_bidir_add_luminance_transitionPO,

  // p_empirical_ca_dist,

  p_vignetting_retriesPO,
  p_proper_ray_derivativesPO
};


// to switch between lens models in interface dropdown
static const char* LensModelNames[] = {
  #include "../include/auto_generated_lens_includes/pota_cpp_lenses.h"
  NULL
};


node_parameters {
  AiParameterEnum("lens_modelPO", angenieux__double_gauss__1953__49mm, LensModelNames);

  AiParameterFlt("sensor_widthPO", 36.0); // 35mm film
  AiParameterFlt("wavelengthPO", 550.0); // wavelength in nm
  AiParameterBool("dofPO", true);
  AiParameterFlt("fstopPO", 0.0);
  AiParameterFlt("focus_distancePO", 150.0); // in cm to be consistent with arnold core
  AiParameterFlt("extra_sensor_shiftPO", 0.0);


  AiParameterInt("bokeh_aperture_bladesPO", 0);
  AiParameterBool("bokeh_enable_imagePO", false);
  AiParameterStr("bokeh_image_pathPO", "");


  AiParameterFlt("bidir_min_luminancePO", 1.0);
  AiParameterStr("bidir_output_pathPO", "");
  AiParameterInt("bidir_sample_multPO", 20);
  AiParameterFlt("bidir_add_luminancePO", 0.0);
  AiParameterFlt("bidir_add_luminance_transitionPO", 1.0);

  // AiParameterFlt("empirical_ca_dist", 0.0);

  AiParameterInt("vignetting_retriesPO", 15);
  AiParameterBool("proper_ray_derivativesPO", true);
}


node_initialize {
  AiCameraInitialize(node);
  AiNodeSetLocalData(node, new Camera());
}


node_update { 
  AiCameraUpdate(node, false);
  Camera* po = (Camera*)AiNodeGetLocalData(node);

  po->sensor_width = AiNodeGetFlt(node, "sensor_widthPO");
  po->input_fstop = AiNodeGetFlt(node, "fstopPO");
  po->focus_distance = AiNodeGetFlt(node, "focus_distancePO") * 10.0; //converting to mm
  po->lensModel = (LensModel) AiNodeGetInt(node, "lens_modelPO");
  po->bokeh_aperture_blades = AiNodeGetInt(node, "bokeh_aperture_bladesPO");
  po->dof = AiNodeGetBool(node, "dofPO");
  po->vignetting_retries = AiNodeGetInt(node, "vignetting_retriesPO");
  po->bidir_min_luminance = AiNodeGetFlt(node, "bidir_min_luminancePO");
  po->bidir_output_path = AiNodeGetStr(node, "bidir_output_pathPO");
  po->proper_ray_derivatives = AiNodeGetBool(node, "proper_ray_derivativesPO");
  po->bokeh_enable_image = AiNodeGetBool(node, "bokeh_enable_imagePO");
  po->bokeh_image_path = AiNodeGetStr(node, "bokeh_image_pathPO");
  // po->empirical_ca_dist = AiNodeGetFlt(node, "empirical_ca_dist");
  
  po->bidir_sample_mult = AiNodeGetInt(node, "bidir_sample_multPO");
  po->bidir_add_luminance = AiNodeGetFlt(node, "bidir_add_luminancePO");
  po->bidir_add_luminance_transition = AiNodeGetFlt(node, "bidir_add_luminance_transitionPO");

  po->lambda = AiNodeGetFlt(node, "wavelengthPO") * 0.001;
  po->extra_sensor_shift = AiNodeGetFlt(node, "extra_sensor_shiftPO");

  #include "node_update_po.h"
}


node_finish {
  Camera* po = (Camera*)AiNodeGetLocalData(node);
  delete po;
}


camera_create_ray {
  Camera* po = (Camera*)AiNodeGetLocalData(node);

  int tries = 0;
  double random1 = 0.0, random2 = 0.0; 
  Eigen::Vector3d origin(0, 0, 0);
  Eigen::Vector3d direction(0, 0, 0);
  Eigen::Vector3d weight(1, 1, 1);

  trace_ray(true, tries, input.sx, input.sy, input.lensx, input.lensy, random1, random2, weight, origin, direction, po);
  
  // calculate new ray derivatives
  // sucks a bit to have to trace 3 rays.. Bit slow
  // is there an analytical solution to this?..
  if (tries > 0){
    if (!po->proper_ray_derivatives){
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
      trace_ray(false, tries, input_dx.sx, input_dx.sy, random1, random2, random1, random2, out_dx_weight, out_dx_origin, out_dx_dir, po);

      Eigen::Vector3d out_dy_weight(output_dy.weight[0], output_dy.weight[1], output_dy.weight[2]);
      Eigen::Vector3d out_dy_origin(output_dy.origin[0], output_dy.origin[1], output_dy.origin[2]);
      Eigen::Vector3d out_dy_dir(output_dy.dir[0], output_dy.dir[1], output_dy.dir[2]);
      trace_ray(false, tries, input_dy.sx, input_dy.sy, random1, random2, random1, random2, out_dy_weight, out_dy_origin, out_dy_dir, po);

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
  const Camera* po = (Camera*)AiNodeGetLocalData(node);

  double coeff = 1.0 / std::max(std::abs(Po.z * po->tan_fov), 1e-3);
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
