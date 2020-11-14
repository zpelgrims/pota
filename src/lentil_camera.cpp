#include <ai.h>
#include "lentil.h"
#include "lens.h"
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
  AiParameterFlt("focus_distance", 150.0); // in cm to be consistent with arnold core


  // po specifics
  AiParameterEnum("lens_model", angenieux__double_gauss__1953__49mm, LensModelNames);
  AiParameterFlt("wavelength", 550.0); // wavelength in nm
  AiParameterFlt("extra_sensor_shift", 0.0);

  // tl specifics
  AiParameterFlt("focal_length", 35.0); // in mm
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
  AiParameterInt("bidir_sample_mult", 20);
  AiParameterFlt("bidir_add_luminance", 0.0);
  AiParameterFlt("bidir_add_luminance_transition", 1.0);
  AiParameterBool("bidir_debug", false);

  // advanced
  AiParameterInt("vignetting_retries", 15);

  // experimental
  AiParameterFlt("abb_coma", 0.0);

  AiMetaDataSetBool(nentry, nullptr, "force_update", true);
}


node_initialize {
  AiCameraInitialize(node);
  AiNodeSetLocalData(node, new Camera());
}


node_update { 
  AiCameraUpdate(node, false);
  Camera* po = (Camera*)AiNodeGetLocalData(node);

  // try to force recomputation of the operator, if i don't write some data to it, it only runs on scene init
  AtNode *operator_node = (AtNode*)AiNodeGetPtr(AiUniverseGetOptions(), "operator");
  if (operator_node != nullptr){
      if (AiNodeIs(operator_node, AtString("lentil_operator"))){
          AiNodeSetInt(operator_node, "call_me_dirty", rand());
      }
  }


  // global params
  po->cameraType = (CameraType) AiNodeGetInt(node, "cameratype");
  po->unitModel = (UnitModel) AiNodeGetInt(node, "units");
  po->sensor_width = AiNodeGetFlt(node, "sensor_width");
  po->enable_dof = AiNodeGetBool(node, "enable_dof");
  po->input_fstop = clamp_min(AiNodeGetFlt(node, "fstop"), 0.01);
  po->focus_distance = AiNodeGetFlt(node, "focus_distance"); //converting to mm
  po->bokeh_aperture_blades = AiNodeGetInt(node, "bokeh_aperture_blades");

  // po-specific params
  po->lensModel = (LensModel) AiNodeGetInt(node, "lens_model");
  po->lambda = AiNodeGetFlt(node, "wavelength") * 0.001;
  po->extra_sensor_shift = AiNodeGetFlt(node, "extra_sensor_shift");

  // tl specific params
  po->focal_length = clamp_min(AiNodeGetFlt(node, "focal_length"), 0.01);
  po->optical_vignetting_distance = AiNodeGetFlt(node, "optical_vignetting_distance");
  po->optical_vignetting_radius = AiNodeGetFlt(node, "optical_vignetting_radius");
  po->abb_spherical = AiNodeGetFlt(node, "abb_spherical");
  po->abb_spherical = clamp(po->abb_spherical, 0.001, 0.999);
  po->abb_distortion = AiNodeGetFlt(node, "abb_distortion");
  po->abb_coma = AiNodeGetFlt(node, "abb_coma");
  po->circle_to_square = AiNodeGetFlt(node, "bokeh_circle_to_square");
  po->circle_to_square = clamp(po->circle_to_square, 0.01, 0.99);
  po->bokeh_anamorphic = AiNodeGetFlt(node, "bokeh_anamorphic");
  po->bokeh_anamorphic = clamp(po->bokeh_anamorphic, 0.01, 99999.0);

  // bidir params
  po->bidir_min_luminance = AiNodeGetFlt(node, "bidir_min_luminance");
  po->bokeh_enable_image = AiNodeGetBool(node, "bokeh_enable_image");
  po->bokeh_image_path = AiNodeGetStr(node, "bokeh_image_path");
  po->bidir_sample_mult = AiNodeGetInt(node, "bidir_sample_mult");
  po->bidir_add_luminance = AiNodeGetFlt(node, "bidir_add_luminance");
  po->bidir_add_luminance_transition = AiNodeGetFlt(node, "bidir_add_luminance_transition");
  po->bidir_debug = AiNodeGetBool(node, "bidir_debug");
  po->vignetting_retries = AiNodeGetInt(node, "vignetting_retries");


  switch (po->cameraType){
    case PolynomialOptics:
    {
      po->focus_distance *= 10.0;
      #include "node_update_po.h"

      break;
    }
    case ThinLens:
    {
      po->fov = 2.0 * std::atan(po->sensor_width / (2.0*po->focal_length));
      po->tan_fov = std::tan(po->fov/2.0);
      po->aperture_radius = (po->focal_length / (2.0 * po->input_fstop)) / 10.0;
      
      break;
    }
  }


  // make probability functions of the bokeh image
  // if (!(po->stored_useImage == AiNodeGetBool(node, "bokeh_enable_imagePO") && po->stored_path == AiNodeGetStr(node, "bokeh_image_pathPO")) {
  po->image.invalidate();
  if (po->bokeh_enable_image && !po->image.read(po->bokeh_image_path.c_str())){
    AiMsgError("[LENTIL CAMERA PO] Couldn't open bokeh image!");
    AiRenderAbort();
  }

}


node_finish {
  Camera* po = (Camera*)AiNodeGetLocalData(node);
  delete po;
}


camera_create_ray {
  Camera* po = (Camera*)AiNodeGetLocalData(node);


  switch (po->cameraType){
    case PolynomialOptics:
    { 
      int tries = 0;
      double random1 = 0.0, random2 = 0.0; 
      Eigen::Vector3d origin(0, 0, 0);
      Eigen::Vector3d direction(0, 0, 0);
      Eigen::Vector3d weight(1, 1, 1);

      trace_ray(true, tries, input.sx, input.sy, input.lensx, input.lensy, random1, random2, weight, origin, direction, po);


      // calculate new ray derivatives
      if (tries > 0){
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

      for (int i = 0; i<3; i++){
        output.origin[i] = origin(i);
        output.dir[i] = direction(i);
        output.weight[i] = weight(i);
      }

      break;
    }

    case ThinLens:
    {
      int tries = 0;
      float r1 = 0.0, r2 = 0.0;
      AtVector origin (0, 0, 0);
      AtVector dir (0, 0, 0);
      AtRGB weight (1, 1, 1);
      
      trace_ray_fw_thinlens(true, tries, input.sx, input.sy, input.lensx, input.lensy, origin, dir, weight, r1, r2, po);

      if (tries > 0){
        
          float step = 0.001;
          AtCameraInput input_dx = input;
          AtCameraInput input_dy = input;
          AtCameraOutput output_dx;
          AtCameraOutput output_dy;

          input_dx.sx += input.dsx * step;
          input_dy.sy += input.dsy * step;

          trace_ray_fw_thinlens(false, tries, input_dx.sx, input_dx.sy, r1, r2, output_dx.origin, output_dx.dir, output_dx.weight, r1, r2, po);
          trace_ray_fw_thinlens(false, tries, input_dy.sx, input_dy.sy, r1, r2, output_dy.origin, output_dy.dir, output_dy.weight, r1, r2, po);

          output.dOdx = (output_dx.origin - origin) / step;
          output.dOdy = (output_dy.origin - origin) / step;
          output.dDdx = (output_dx.dir - dir) / step;
          output.dDdy = (output_dy.dir - dir) / step;
      }


      output.origin = origin;
      output.dir = AiV3Normalize(dir);
      output.weight = weight;
      
      break;
    }
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



void registerLentilCameraPO(AtNodeLib* node) {
    node->methods = (AtNodeMethods*) lentilMethods;
    node->output_type = AI_TYPE_UNDEFINED;
    node->name = "lentil_camera";
    node->node_type = AI_NODE_CAMERA;
    strcpy(node->version, AI_VERSION);
}