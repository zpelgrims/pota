#include <ai.h>
#include "lentil.h"
#include "lens.h"


AI_FILTER_NODE_EXPORT_METHODS(LentilFilterDataMtd);
 

// world to camera space transform, motion blurred
inline Eigen::Vector3d world_to_camera_space_motionblur(const AtVector sample_pos_ws, const float time_start, const float time_end){
  AtMatrix world_to_camera_matrix_motionblurred;
  float currenttime = linear_interpolate(xor128() / 4294967296.0, time_start, time_end); // should I create new random sample, or can I re-use another one?
  AiWorldToCameraMatrix(AiUniverseGetCamera(), currenttime, world_to_camera_matrix_motionblurred);
  const AtVector camera_space_sample_position_mb = AiM4PointByMatrixMult(world_to_camera_matrix_motionblurred, sample_pos_ws);
  Eigen::Vector3d camera_space_sample_position_mb_eigen (camera_space_sample_position_mb.x, camera_space_sample_position_mb.y, camera_space_sample_position_mb.z);
  return camera_space_sample_position_mb_eigen;
}

inline Eigen::Vector2d sensor_to_pixel_position(const Eigen::Vector2d sensor_position, const float sensor_width, const float frame_aspect_ratio, const double xres, const double yres){
  // convert sensor position to pixel position
  const Eigen::Vector2d s(sensor_position(0) / (sensor_width * 0.5), sensor_position(1) / (sensor_width * 0.5) * frame_aspect_ratio);
  const Eigen::Vector2d pixel((( s(0) + 1.0) / 2.0) * xres, 
                              ((-s(1) + 1.0) / 2.0) * yres);
  return pixel;
}

inline float thinlens_get_image_dist_focusdist(Camera *po){
    return (-po->focal_length * -po->focus_distance) / (-po->focal_length + -po->focus_distance);
}


inline float thinlens_get_coc(AtVector sample_pos_ws, LentilFilterData *bokeh, Camera *po){
  // world to camera space transform, static just for CoC
  AtMatrix world_to_camera_matrix_static;
  float time_middle = linear_interpolate(0.5, bokeh->time_start, bokeh->time_end);
  AiWorldToCameraMatrix(AiUniverseGetCamera(), time_middle, world_to_camera_matrix_static);
  AtVector camera_space_sample_position_static = AiM4PointByMatrixMult(world_to_camera_matrix_static, sample_pos_ws); // just for CoC size calculation
  
  switch (po->unitModel){
    case mm:
    {
      camera_space_sample_position_static *= 0.1;
    } break;
    case cm:
    { 
      camera_space_sample_position_static *= 1.0;
    } break;
    case dm:
    {
      camera_space_sample_position_static *= 10.0;
    } break;
    case m:
    {
      camera_space_sample_position_static *= 100.0;
    }
  }
  
  const float image_dist_samplepos = (-po->focal_length * camera_space_sample_position_static.z) / (-po->focal_length + camera_space_sample_position_static.z);
  const float image_dist_focusdist = thinlens_get_image_dist_focusdist(po);
  return std::abs((po->aperture_radius * (image_dist_samplepos - image_dist_focusdist))/image_dist_samplepos); // coc diameter
}

node_parameters 
{
  AiMetaDataSetBool(nentry, nullptr, "force_update", true);
}
 
node_initialize
{
  static const char *required_aovs[] = {"RGBA RGBA", "VECTOR P", "FLOAT Z", "RGBA transmission", "RGBA lentil_bidir_ignore", NULL};
  AiFilterInitialize(node, false, required_aovs);
  AiNodeSetLocalData(node, new LentilFilterData());
}
 
node_update 
{
  LentilFilterData *bokeh = (LentilFilterData*)AiNodeGetLocalData(node);
  
  bokeh->enabled = true;

  AtNode *cameranode = AiUniverseGetCamera();
  // disable for non-lentil cameras
  if (!AiNodeIs(cameranode, AtString("lentil_camera"))) {
    bokeh->enabled = false;
    AiMsgError("[LENTIL FILTER] Camera is not of type lentil. A full scene update is required.");
    AiRenderAbort();
    return;
  }

  // will only work for the node called lentil_replaced_filter
  if (AtString(AiNodeGetName(node)) != AtString("lentil_replaced_filter")){
    bokeh->enabled = false;
    AiMsgError("[LENTIL FILTER] node is not named correctly: %s (should be: lentil_replaced_filter).", AiNodeGetName(node));
    AiRenderAbort();
    return;
  }

  // if progressive rendering is on, don't redistribute
  if (AiNodeGetBool(AiUniverseGetOptions(), "enable_progressive_render")) {
    bokeh->enabled = false;
    AiMsgError("[LENTIL FILTER] Progressive rendering is not supported.");
    AiRenderAbort();
    return;
  }

  if (!AiNodeGetBool(cameranode, "enable_dof")) {
    AiMsgWarning("[LENTIL FILTER] Depth of field is disabled, therefore disabling bidirectional sampling.");
    bokeh->enabled = false;
  }
  
  bokeh->xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
  bokeh->yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");
  bokeh->filter_width = 2.0;
  bokeh->framenumber = static_cast<int>(AiNodeGetFlt(AiUniverseGetOptions(), "frame"));

  bokeh->zbuffer.clear();
  bokeh->zbuffer.resize(bokeh->xres * bokeh->yres);

  bokeh->time_start = AiCameraGetShutterStart();
  bokeh->time_end = AiCameraGetShutterEnd();


  if (AiNodeGetInt(cameranode, "bidir_sample_mult") == 0){
    bokeh->enabled = false;
    AiMsgWarning("[LENTIL FILTER] Bidirectional samples are set to 0, filter will not execute.");
  }

  if (AiNodeGetBool(cameranode, "bidir_debug")) {
    bokeh->enabled = false;
    AiMsgWarning("[LENTIL FILTER] Bidirectional Debug mode is on, no redistribution.");
  }

  // prepare framebuffers for all AOVS
  bokeh->aov_list_name.clear();
  bokeh->aov_list_type.clear();
  bokeh->aov_duplicates.clear();

  AtNode* options = AiUniverseGetOptions();
  AtArray* outputs = AiNodeGetArray(options, "outputs");
  for (size_t i=0; i<AiArrayGetNumElements(outputs); ++i) {
    std::string output_string = AiArrayGetStr(outputs, i).c_str();
    std::string lentil_str = "lentil_replaced_filter";

    if (output_string.find(lentil_str) != std::string::npos){
     
      std::string name = split_str(output_string, std::string(" ")).begin()[0];
      std::string type = split_str(output_string, std::string(" ")).begin()[1];

      AiMsgInfo("[LENTIL FILTER] Adding aov %s of type %s", name.c_str(), type.c_str());

      bokeh->aov_list_name.push_back(AtString(name.c_str()));
      bokeh->aov_list_type.push_back(string_to_arnold_type(type));

      ++bokeh->aov_duplicates[AtString(name.c_str())];

      bokeh->image_data_types[AtString(name.c_str())].clear();
      bokeh->image_data_types[AtString(name.c_str())].resize(bokeh->xres * bokeh->yres);
      bokeh->image_col_types[AtString(name.c_str())].clear();
      bokeh->image_col_types[AtString(name.c_str())].resize(bokeh->xres * bokeh->yres);
      bokeh->image_ptr_types[AtString(name.c_str())].clear();
      bokeh->image_ptr_types[AtString(name.c_str())].resize(bokeh->xres * bokeh->yres);
    }
  }

  bokeh->pixel_already_visited.clear();
  bokeh->pixel_already_visited.resize(bokeh->xres*bokeh->yres);
  for (size_t i=0;i<bokeh->pixel_already_visited.size(); ++i) bokeh->pixel_already_visited[i] = false; // not sure if i have to
  
  bokeh->current_inv_density = 0.0;

  if (bokeh->enabled) AiMsgInfo("[LENTIL FILTER] Setup completed, starting bidirectional sampling.");

  AiFilterUpdate(node, bokeh->filter_width);
}
 
filter_output_type
{
   switch (input_type)
   {
      case AI_TYPE_RGBA:
         return AI_TYPE_RGBA;
      case AI_TYPE_RGB:
         return AI_TYPE_RGB;
      case AI_TYPE_VECTOR:
        return AI_TYPE_VECTOR;
      case AI_TYPE_FLOAT:
        return AI_TYPE_FLOAT;
      // case AI_TYPE_INT:
      //   return AI_TYPE_INT;
      // case AI_TYPE_UINT:
      //   return AI_TYPE_UINT;
      // case AI_TYPE_POINTER:
      //   return AI_TYPE_POINTER;
      default:
         return AI_TYPE_NONE;
   }
}


filter_pixel
{
  LentilFilterData *bokeh = (LentilFilterData*)AiNodeGetLocalData(node);

  if (!AiNodeIs(AiUniverseGetCamera(), AtString("lentil_camera"))) {
    bokeh->enabled = false;
    AiMsgError("[LENTIL FILTER] Couldn't get correct camera. Please refresh the render.");
    AiRenderAbort();
    return;
  }

  Camera *po = (Camera*)AiNodeGetLocalData(AiUniverseGetCamera());

  if (bokeh->enabled){
    const double xres = (double)bokeh->xres;
    const double yres = (double)bokeh->yres;
    const double frame_aspect_ratio = xres/yres;

    int px, py;
    AiAOVSampleIteratorGetPixel(iterator, px, py);

    // hack to try avoid running over same pixel twice
    int linear_pixel = px + (py * (double)bokeh->xres);
    if (bokeh->pixel_already_visited[linear_pixel]) {
        goto just_filter;
    } else bokeh->pixel_already_visited[linear_pixel] = true;


    while (AiAOVSampleIteratorGetNext(iterator)) {
      
      bool redistribute = true;

      AtRGBA sample = AiAOVSampleIteratorGetRGBA(iterator);
      const float inv_density = AiAOVSampleIteratorGetInvDensity(iterator);
      bokeh->current_inv_density = inv_density;
      AtVector sample_pos_ws = AiAOVSampleIteratorGetAOVVec(iterator, bokeh->atstring_p);
      float depth = AiAOVSampleIteratorGetAOVFlt(iterator, bokeh->atstring_z); // what to do when values are INF?
    
      if (inv_density <= 0.f) continue; // does this every happen? test
      if (inv_density > 0.2){
        continue; // skip when aa samples are below 3
      }
      
      const float filter_width_half = std::ceil(bokeh->filter_width * 0.5);


      const AtRGBA sample_transmission = AiAOVSampleIteratorGetAOVRGBA(iterator, bokeh->atstring_transmission);
      bool transmitted_energy_in_sample = (AiColorMaxRGB(sample_transmission) > 0.0);
      if (transmitted_energy_in_sample){
        sample.r -= sample_transmission.r;
        sample.g -= sample_transmission.g;
        sample.b -= sample_transmission.b;
      }

      const float sample_luminance = sample.r*0.21 + sample.g*0.71 + sample.b*0.072;
      if (sample_luminance < po->bidir_min_luminance) redistribute = false;
      if (depth == AI_INFINITE) redistribute = false; // not sure if this works.. Z AOV has inf values at skydome hits
      if (AiV3IsSmall(sample_pos_ws)) redistribute = false; // not sure if this works .. position is 0,0,0 at skydome hits
      if (AiAOVSampleIteratorHasAOVValue(iterator, bokeh->atstring_lentil_bidir_ignore, AI_TYPE_RGBA)) redistribute = false;
      


      switch (po->cameraType){
        case PolynomialOptics:
        { 
          AtMatrix world_to_camera_matrix_static;
          float time_middle = linear_interpolate(0.5, bokeh->time_start, bokeh->time_end);
          AiWorldToCameraMatrix(AiUniverseGetCamera(), time_middle, world_to_camera_matrix_static);
          AtVector camera_space_sample_position_static = AiM4PointByMatrixMult(world_to_camera_matrix_static, sample_pos_ws); // just for CoC size calculation
          switch (po->unitModel){
            case mm:
            {
              camera_space_sample_position_static *= 0.1;
            } break;
            case cm:
            { 
              camera_space_sample_position_static *= 1.0;
            } break;
            case dm:
            {
              camera_space_sample_position_static *= 10.0;
            } break;
            case m:
            {
              camera_space_sample_position_static *= 100.0;
            }
          }

          if (std::abs(camera_space_sample_position_static.z) < (po->lens_length*0.1)) redistribute = false; // sample can't be inside of lens

          // early out, before coc
          bool transmission_dump_already_happened = false;
          if (redistribute == false){
            filter_and_add_to_buffer(px, py, filter_width_half, 
                                    1.0, inv_density, depth, transmitted_energy_in_sample, 0,
                                    iterator, bokeh);
            if (!transmitted_energy_in_sample) continue;
            else transmission_dump_already_happened = true;
          }
          
          
          
          // additional luminance with soft transition
          float fitted_bidir_add_luminance = 0.0;
          if (po->bidir_add_luminance > 0.0) fitted_bidir_add_luminance = additional_luminance_soft_trans(sample_luminance, po->bidir_add_luminance, po->bidir_add_luminance_transition, po->bidir_min_luminance);

          Eigen::Vector2d sensor_position(0, 0);


          // PROBE RAYS samples to determine size of bokeh & subsequent sample count
          AtVector2 bbox_min (0, 0);
          AtVector2 bbox_max (0, 0);
          int proberays_total_samples = 0;
          int proberays_base_rays = 32;
          int proberays_max_rays = proberays_base_rays * 2;
          std::vector<AtVector2> pixelcache;

          for(int count=0; count<proberays_base_rays; count++) {
            ++proberays_total_samples;
            if (proberays_total_samples >= proberays_max_rays) continue;

            Eigen::Vector3d camera_space_sample_position_mb_eigen = world_to_camera_space_motionblur(sample_pos_ws, bokeh->time_start, bokeh->time_end); //could check if motionblur is enabled
            switch (po->unitModel){
              case mm:
              {
                camera_space_sample_position_mb_eigen *= 0.1;
              } break;
              case cm:
              { 
                camera_space_sample_position_mb_eigen *= 1.0;
              } break;
              case dm:
              {
                camera_space_sample_position_mb_eigen *= 10.0;
              } break;
              case m:
              {
                camera_space_sample_position_mb_eigen *= 100.0;
              }
            }
          
            if(!trace_backwards(-camera_space_sample_position_mb_eigen * 10.0, po->aperture_radius, po->lambda, sensor_position, po->sensor_shift, po, px, py, proberays_total_samples)) {
              --count;
              continue;
            }

            const Eigen::Vector2d pixel = sensor_to_pixel_position(sensor_position, po->sensor_width, frame_aspect_ratio, xres, yres);

            //figure out why sometimes pixel is nan, can't just skip it
            if ((pixel(0) > xres) || (pixel(0) < 0) || (pixel(1) > yres) || (pixel(1) < 0) || 
                (pixel(0) != pixel(0)) || (pixel(1) != pixel(1))) // nan checking
            {
              --count;
              continue;
            }

            // expand bbox
            if (count == 0) {
              bbox_min = {(float)pixel(0), (float)pixel(1)};
              bbox_max = {(float)pixel(0), (float)pixel(1)};
            } else {
              bbox_min = {std::min(bbox_min.x, (float)pixel(0)), std::min(bbox_min.y, (float)pixel(1))};
              bbox_max = {std::max(bbox_max.x, (float)pixel(0)), std::max(bbox_max.y, (float)pixel(1))};
            }

            pixelcache.push_back(AtVector2(pixel(0), pixel(1))); // store for re-use later
          }


          double bbox_area = (bbox_max.x - bbox_min.x) * (bbox_max.y - bbox_min.y);
          if (bbox_area < 20.0) redistribute = false; //might have to increase this?
          int samples = std::floor(bbox_area * std::pow(po->bidir_sample_mult,2) * 0.001);
          samples = std::ceil((double)(samples) * inv_density);
          samples = std::clamp(samples, 5, 10000); // not sure if a million is actually ever hit.. 75 seems high but is needed to remove stochastic noise
          float inv_samples = 1.0 / static_cast<double>(samples);


          // early out, after coc
          if (redistribute == false){
            if (!transmission_dump_already_happened){
              filter_and_add_to_buffer(px, py, filter_width_half, 
                                      1.0, inv_density, depth, transmitted_energy_in_sample, 0,
                                      iterator, bokeh);
            }
            if (!transmitted_energy_in_sample) continue;
          }


          unsigned int total_samples_taken = 0;
          unsigned int max_total_samples = samples*5;
            

          for(int count=0; count<samples && total_samples_taken < max_total_samples; ++count, ++total_samples_taken) {
            
            Eigen::Vector2d pixel;
            
            if (count < pixelcache.size()){ // redist already taken samples from probe rays
              pixel(0) = pixelcache[count].x; pixel(1) = pixelcache[count].y;
            } else {

              Eigen::Vector3d camera_space_sample_position_mb_eigen = world_to_camera_space_motionblur(sample_pos_ws, bokeh->time_start, bokeh->time_end);  //could check if motionblur is enabled
              switch (po->unitModel){
                case mm:
                {
                  camera_space_sample_position_mb_eigen *= 0.1;
                } break;
                case cm:
                { 
                  camera_space_sample_position_mb_eigen *= 1.0;
                } break;
                case dm:
                {
                  camera_space_sample_position_mb_eigen *= 10.0;
                } break;
                case m:
                {
                  camera_space_sample_position_mb_eigen *= 100.0;
                }
              }

              if(!trace_backwards(-camera_space_sample_position_mb_eigen*10.0, po->aperture_radius, po->lambda, sensor_position, po->sensor_shift, po, px, py, total_samples_taken)) {
                --count;
                continue;
              }

              pixel = sensor_to_pixel_position(sensor_position, po->sensor_width, frame_aspect_ratio, xres, yres);

              // if outside of image
              if ((pixel(0) >= xres) || (pixel(0) < 0) || (pixel(1) >= yres) || (pixel(1) < 0) ||
                  (pixel(0) != pixel(0)) || (pixel(1) != pixel(1))) // nan checking
              {
                --count; // much room for improvement here, potentially many samples are wasted outside of frame
                continue;
              }
            }

            // >>>> currently i've decided not to filter the redistributed energy. If needed, there's an old prototype in github issue #230

            // write sample to image
            unsigned pixelnumber = static_cast<int>(bokeh->xres * floor(pixel(1)) + floor(pixel(0)));

            for (unsigned i=0; i<bokeh->aov_list_name.size(); i++){
              add_to_buffer(pixelnumber, bokeh->aov_list_type[i], bokeh->aov_list_name[i], 
                            inv_samples, inv_density / std::pow(bokeh->filter_width,2), fitted_bidir_add_luminance, depth,
                            transmitted_energy_in_sample, 1, iterator, bokeh);
            }
          }
        
          break;
        }
        case ThinLens:
        {
          // early out, before coc
          bool transmission_dump_already_happened = false;
          if (redistribute == false){
            filter_and_add_to_buffer(px, py, filter_width_half, 
                                    1.0, inv_density, depth, transmitted_energy_in_sample, 0,
                                    iterator, bokeh);
            if (!transmitted_energy_in_sample) continue;
            else transmission_dump_already_happened = true;
          }
          
          
          // additional luminance with soft transition
          float fitted_bidir_add_luminance = 0.0;
          if (po->bidir_add_luminance > 0.0) fitted_bidir_add_luminance = additional_luminance_soft_trans(sample_luminance, po->bidir_add_luminance, po->bidir_add_luminance_transition, po->bidir_min_luminance);
          

          float circle_of_confusion = thinlens_get_coc(sample_pos_ws, bokeh, po);
          const float coc_squared_pixels = std::pow(circle_of_confusion * bokeh->yres, 2) * std::pow(po->bidir_sample_mult,2) * 0.001; // pixel area as baseline for sample count
          if (std::pow(circle_of_confusion * bokeh->yres, 2) < std::pow(20, 2)) redistribute = false; // 20^2 px minimum coc
          int samples = std::ceil(coc_squared_pixels * inv_density); // aa_sample independence
          samples = std::clamp(samples, 5, 10000); // not sure if a million is actually ever hit..
          float inv_samples = 1.0/static_cast<double>(samples);


          // early out, after coc
          if (redistribute == false){
            if (!transmission_dump_already_happened){
              filter_and_add_to_buffer(px, py, filter_width_half, 
                                      1.0, inv_density, depth, transmitted_energy_in_sample, 0,
                                      iterator, bokeh);
            }
            if (!transmitted_energy_in_sample) continue;
          }
        
        
          unsigned int total_samples_taken = 0;
          unsigned int max_total_samples = samples*5;

          for(int count=0; count<samples && total_samples_taken<max_total_samples; ++count, ++total_samples_taken) {
            
            unsigned int seed = tea<8>((px*py+px), total_samples_taken);

            // world to camera space transform, motion blurred
            AtMatrix world_to_camera_matrix_motionblurred;
            float currenttime = linear_interpolate(rng(seed), bokeh->time_start, bokeh->time_end); // should I create new random sample, or can I re-use another one?
            AiWorldToCameraMatrix(AiUniverseGetCamera(), currenttime, world_to_camera_matrix_motionblurred);
            AtVector camera_space_sample_position_mb = AiM4PointByMatrixMult(world_to_camera_matrix_motionblurred, sample_pos_ws);
            switch (po->unitModel){
              case mm:
              {
                camera_space_sample_position_mb *= 0.1;
              } break;
              case cm:
              { 
                camera_space_sample_position_mb *= 1.0;
              } break;
              case dm:
              {
                camera_space_sample_position_mb *= 10.0;
              } break;
              case m:
              {
                camera_space_sample_position_mb *= 100.0;
              }
            }
            float image_dist_samplepos_mb = (-po->focal_length * camera_space_sample_position_mb.z) / (-po->focal_length + camera_space_sample_position_mb.z);

            // either get uniformly distributed points on the unit disk or bokeh image
            Eigen::Vector2d unit_disk(0, 0);
            if (po->bokeh_enable_image) po->image.bokehSample(rng(seed),rng(seed), unit_disk, rng(seed), rng(seed));
            else if (po->bokeh_aperture_blades < 2) concentricDiskSample(rng(seed),rng(seed), unit_disk, po->abb_spherical, po->circle_to_square, po->bokeh_anamorphic);
            else lens_sample_triangular_aperture(unit_disk(0), unit_disk(1), rng(seed),rng(seed), 1.0, po->bokeh_aperture_blades);



            unit_disk(0) *= po->bokeh_anamorphic;
            AtVector lens(unit_disk(0) * po->aperture_radius, unit_disk(1) * po->aperture_radius, 0.0);


            // aberration inputs
            // float abb_field_curvature = 0.0;


            // ray through center of lens
            AtVector dir_from_center = AiV3Normalize(camera_space_sample_position_mb);
            AtVector dir_lens_to_P = AiV3Normalize(camera_space_sample_position_mb - lens);
            // perturb ray direction to simulate coma aberration
            // todo: the bidirectional case isn't entirely the same as the forward case.. fix!
            // current strategy is to perturb the initial sample position by doing the same ray perturbation i'm doing in the forward case
            float abb_coma = po->abb_coma * abb_coma_multipliers(po->sensor_width, po->focal_length, dir_from_center, unit_disk);
            dir_lens_to_P = abb_coma_perturb(dir_lens_to_P, dir_from_center, abb_coma, true);
            camera_space_sample_position_mb = AiV3Length(camera_space_sample_position_mb) * dir_lens_to_P;
            dir_from_center = AiV3Normalize(camera_space_sample_position_mb);

            float samplepos_image_intersection = std::abs(image_dist_samplepos_mb/dir_from_center.z);
            AtVector samplepos_image_point = dir_from_center * samplepos_image_intersection;


            // depth of field
            AtVector dir_from_lens_to_image_sample = AiV3Normalize(samplepos_image_point - lens);





            float focusdist_intersection = std::abs(thinlens_get_image_dist_focusdist(po)/dir_from_lens_to_image_sample.z);
            AtVector focusdist_image_point = lens + dir_from_lens_to_image_sample*focusdist_intersection;
            
            // bring back to (x, y, 1)
            AtVector2 sensor_position(focusdist_image_point.x / focusdist_image_point.z,
                                      focusdist_image_point.y / focusdist_image_point.z);
            // transform to screenspace coordinate mapping
            sensor_position /= (po->sensor_width*0.5)/-po->focal_length;


            // optical vignetting
            dir_lens_to_P = AiV3Normalize(camera_space_sample_position_mb - lens);
            if (po->optical_vignetting_distance > 0.0){
              // if (image_dist_samplepos<image_dist_focusdist) lens *= -1.0; // this really shouldn't be the case.... also no way i can do that in forward tracing?
              if (!empericalOpticalVignettingSquare(lens, dir_lens_to_P, po->aperture_radius, po->optical_vignetting_radius, po->optical_vignetting_distance, lerp_squircle_mapping(po->circle_to_square))){
                  --count;
                  continue;
              }
            }


            // barrel distortion (inverse)
            if (po->abb_distortion > 0.0) sensor_position = inverseBarrelDistortion(AtVector2(sensor_position.x, sensor_position.y), po->abb_distortion);
            

            // convert sensor position to pixel position
            Eigen::Vector2d s(sensor_position.x, sensor_position.y * frame_aspect_ratio);
            const float pixel_x = (( s(0) + 1.0) / 2.0) * xres;
            const float pixel_y = ((-s(1) + 1.0) / 2.0) * yres;

            // if outside of image
            if ((pixel_x >= xres) || (pixel_x < 0) || (pixel_y >= yres) || (pixel_y < 0)) {
              --count; // much room for improvement here, potentially many samples are wasted outside of frame, could keep track of a bbox
              continue;
            }

            // write sample to image
            unsigned pixelnumber = static_cast<int>(bokeh->xres * floor(pixel_y) + floor(pixel_x));
            if (!redistribute) pixelnumber = static_cast<int>(bokeh->xres * py + px);

            // >>>> currently i've decided not to filter the redistributed energy. If needed, there's an old prototype in github issue #230

            for (unsigned i=0; i<bokeh->aov_list_name.size(); i++){
              add_to_buffer(pixelnumber, bokeh->aov_list_type[i], bokeh->aov_list_name[i], 
                            inv_samples, inv_density / std::pow(bokeh->filter_width,2), fitted_bidir_add_luminance, depth,
                            transmitted_energy_in_sample, 1, iterator, bokeh);
            
            }
          }

          break;
        }
      }
    }
  } 
  
  
  // do regular filtering (passthrough) for display purposes
  just_filter:
  AiAOVSampleIteratorReset(iterator);
  switch(data_type){
    case AI_TYPE_RGBA: {
      AtRGBA value_out = AI_RGBA_ZERO;
      if (po->bidir_debug){
        while (AiAOVSampleIteratorGetNext(iterator))
        {
          AtRGBA sample_energy = AiAOVSampleIteratorGetRGBA(iterator);
          const float sample_luminance = sample_energy.r*0.21 + sample_energy.g*0.71 + sample_energy.b*0.072;
          if (sample_luminance > po->bidir_min_luminance) {
              value_out = AtRGBA(1.0, 1.0, 1.0, 1.0);
          }
        }
      } else {
        value_out = filter_gaussian_complete(iterator, bokeh->filter_width, data_type);
      }

      *((AtRGBA*)data_out) = value_out;
      break;
    }
    case AI_TYPE_RGB: {
      AtRGBA filtered_value = filter_gaussian_complete(iterator, bokeh->filter_width, data_type);
      AtRGB rgb_energy {filtered_value.r, filtered_value.g, filtered_value.b};
      *((AtRGB*)data_out) = rgb_energy;
      break;
    }
    case AI_TYPE_VECTOR: {
      AtRGBA filtered_value = filter_closest_complete(iterator, data_type, bokeh);
      AtVector rgb_energy {filtered_value.r, filtered_value.g, filtered_value.b};
      *((AtVector*)data_out) = rgb_energy;
      break;
    }
    case AI_TYPE_FLOAT: {
      AtRGBA filtered_value = filter_closest_complete(iterator, data_type, bokeh);
      float rgb_energy = filtered_value.r;
      *((float*)data_out) = rgb_energy;
      break;
    }
    // case AI_TYPE_INT: {
    //   AtRGBA filtered_value = filter_closest_complete(iterator, data_type, bokeh);
    //   int rgb_energy = filtered_value.r;
    //   *((int*)data_out) = rgb_energy;
    //   break;
    // }
    // case AI_TYPE_UINT: {
    //   AtRGBA filtered_value = filter_closest_complete(iterator, data_type, bokeh);
    //   unsigned rgb_energy = std::abs(filtered_value.r);
    //   *((unsigned*)data_out) = rgb_energy;
    //   break;
    // }
  }
  
}

 
node_finish {}


void registerLentilFilterPO(AtNodeLib* node) {
    node->methods = (AtNodeMethods*) LentilFilterDataMtd;
    node->output_type = AI_TYPE_NONE;
    node->name = "lentil_filter";
    node->node_type = AI_NODE_FILTER;
    strcpy(node->version, AI_VERSION);
}