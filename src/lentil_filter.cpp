#include <ai.h>
#include "lentil.h"
#include "lens.h"


AI_FILTER_NODE_EXPORT_METHODS(LentilFilterDataMtd);
 

struct InternalFilterData {
  AtNode *imager_node;
};



AtNode* get_lentil_imager(AtUniverse *uni) {
  AtNode* options = AiUniverseGetOptions(uni);
  AtArray* outputs = AiNodeGetArray(options, "outputs");
  std::string output_string = AiArrayGetStr(outputs, 0).c_str(); // only considering first output string, should be the same for all of them
  std::string driver = split_str(output_string, std::string(" ")).begin()[3];
  AtString driver_as = AtString(driver.c_str());
  AtNode *driver_node = AiNodeLookUpByName(uni, driver_as);
  AtNode *imager_node = (AtNode*)AiNodeGetPtr(driver_node, "input");
  
  
  if (imager_node == nullptr){
    AiMsgError("[LENTIL FILTER] Couldn't find imager input. Is your imager connected?");
    return nullptr;
  }
  
  for (int i=0; i<16; i++){ // test, only considering depth of 16 for now, ideally should be arbitrary
    const AtNodeEntry* imager_ne = AiNodeGetNodeEntry(imager_node);
    if ( AiNodeEntryGetNameAtString(imager_ne) == AtString("imager_lentil")) {
      return imager_node;
    } else {
      imager_node = (AtNode*)AiNodeGetPtr(imager_node, "input");
    }
  }

  AiMsgError("[LENTIL FILTER] Couldn't find lentil_imager in the imager chain. Is your imager connected?");
  AiRenderAbort();
  return nullptr;
}


 
// world to camera space transform, motion blurred
inline Eigen::Vector3d world_to_camera_space_motionblur(const AtVector sample_pos_ws, const float time_start, const float time_end, AtNode *camera){
  AtMatrix world_to_camera_matrix_motionblurred;
  float currenttime = linear_interpolate(xor128() / 4294967296.0, time_start, time_end); // should I create new random sample, or can I re-use another one?
  AiWorldToCameraMatrix(camera, currenttime, world_to_camera_matrix_motionblurred);
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
  AiWorldToCameraMatrix(bokeh->camera, time_middle, world_to_camera_matrix_static);
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
  static const char *required_aovs[] = {"RGBA RGBA", "VECTOR P", "FLOAT Z", "RGB opacity", "RGBA transmission", "FLOAT lentil_bidir_ignore", NULL};
  AiFilterInitialize(node, true, required_aovs);
  AiNodeSetLocalData(node, new InternalFilterData());
}


node_update 
{
  InternalFilterData *ifd = (InternalFilterData*)AiNodeGetLocalData(node);
  AtUniverse *uni = AiNodeGetUniverse(node);
  ifd->imager_node = get_lentil_imager(uni);
  AiFilterUpdate(node, 2.0);
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
      // case AI_TYPE_FLOAT:
      //   return AI_TYPE_FLOAT; // ORIG
      case AI_TYPE_FLOAT:
        return AI_TYPE_RGBA; // CRYPTO TEST
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
  InternalFilterData *ifd = (InternalFilterData*)AiNodeGetLocalData(node);
  LentilFilterData *bokeh = (LentilFilterData*)AiNodeGetLocalData(ifd->imager_node);

  if (!AiNodeIs(bokeh->camera, AtString("lentil_camera"))) {
    bokeh->enabled = false;
    AiMsgError("[LENTIL FILTER] Couldn't get correct camera. Please refresh the render.");
    AiRenderAbort();
    return;
  }

  Camera *po = (Camera*)AiNodeGetLocalData(bokeh->camera);


  // count samples because I cannot rely on AiAOVSampleIteratorGetInvDensity() any longer since 7.0.0.0
  int samples_counter = 0;
  while (AiAOVSampleIteratorGetNext(iterator)){
    ++samples_counter;
  }
  float AA_samples = std::sqrt(samples_counter) / 2.0;
  bokeh->current_inv_density = 1.0/(AA_samples*AA_samples);
  if (AA_samples != AiNodeGetInt(AiUniverseGetOptions(bokeh->arnold_universe), "AA_samples")){
    bokeh->enabled = false; // skip when aa samples are below final AA samples
  }

  AiAOVSampleIteratorReset(iterator);

  AtString primary_aov_atstring = AiAOVSampleIteratorGetAOVName(iterator);

  if (bokeh->enabled){
    const double xres = (double)bokeh->xres;
    const double yres = (double)bokeh->yres;
    const double frame_aspect_ratio = (double)bokeh->xres_without_region/(double)bokeh->yres_without_region;

    int px, py;
    AiAOVSampleIteratorGetPixel(iterator, px, py);
    px -= bokeh->region_min_x;
    py -= bokeh->region_min_y;

    for (int sampleid=0; AiAOVSampleIteratorGetNext(iterator)==true; sampleid++) {
      bool redistribute = true;

      if (primary_aov_atstring != bokeh->atstring_rgba) redistribute = false;

      AtRGBA sample = AiAOVSampleIteratorGetRGBA(iterator);
      AtVector sample_pos_ws = AiAOVSampleIteratorGetAOVVec(iterator, bokeh->atstring_p);
      float depth = AiAOVSampleIteratorGetAOVFlt(iterator, bokeh->atstring_z); // what to do when values are INF?
      
      const float filter_width_half = std::ceil(bokeh->filter_width * 0.5);


      const AtRGBA sample_transmission = AiAOVSampleIteratorGetAOVRGBA(iterator, bokeh->atstring_transmission);
      bool transmitted_energy_in_sample = (AiColorMaxRGB(sample_transmission) > 0.0);
      if (transmitted_energy_in_sample){
        sample.r -= sample_transmission.r;
        sample.g -= sample_transmission.g;
        sample.b -= sample_transmission.b;
      }

      const float sample_luminance = (sample.r + sample.g + sample.b)/3.0;
      if (depth == AI_INFINITE ||  AiV3IsSmall(sample_pos_ws) || 
          sample_luminance < po->bidir_min_luminance || AiAOVSampleIteratorGetAOVFlt(iterator, bokeh->atstring_lentil_ignore) > 0.0) {
        redistribute = false;
      }

      // cryptomatte cache
      std::map<AtString, std::map<float, float>> crypto_cache;
      if (po->cryptomatte) cryptomatte_construct_cache(crypto_cache, bokeh->cryptomatte_aov_names, iterator, sampleid, bokeh);


      // additional luminance with soft transition
      float fitted_bidir_add_luminance = 0.0;
      if (po->bidir_add_luminance > 0.0) fitted_bidir_add_luminance = additional_luminance_soft_trans(sample_luminance, po->bidir_add_luminance, po->bidir_add_luminance_transition, po->bidir_min_luminance);
      
      float circle_of_confusion = thinlens_get_coc(sample_pos_ws, bokeh, po);
      const float coc_squared_pixels = std::pow(circle_of_confusion * bokeh->yres, 2) * std::pow(po->bidir_sample_mult,2) * 0.001; // pixel area as baseline for sample count
      if (std::pow(circle_of_confusion * bokeh->yres, 2) < std::pow(20, 2)) redistribute = false; // 20^2 px minimum coc
      int samples = std::ceil(coc_squared_pixels * bokeh->current_inv_density); // aa_sample independence
      samples = clamp(samples, 5, 10000); // not sure if a million is actually ever hit..
      float inv_samples = 1.0/static_cast<double>(samples);


      unsigned int total_samples_taken = 0;
      unsigned int max_total_samples = samples*5;

      switch (po->cameraType){
        case PolynomialOptics:
        { 
          AtMatrix world_to_camera_matrix_static;
          float time_middle = linear_interpolate(0.5, bokeh->time_start, bokeh->time_end);
          AiWorldToCameraMatrix(bokeh->camera, time_middle, world_to_camera_matrix_static);
          AtVector camera_space_sample_position_static = AiM4PointByMatrixMult(world_to_camera_matrix_static, sample_pos_ws); // just for CoC size calculation
          switch (po->unitModel){
            case mm: { camera_space_sample_position_static *= 0.1; } break;
            case cm: { camera_space_sample_position_static *= 1.0; } break;
            case dm: { camera_space_sample_position_static *= 10.0;} break;
            case m:  { camera_space_sample_position_static *= 100.0;}
          }

          if (std::abs(camera_space_sample_position_static.z) < (po->lens_length*0.1)) redistribute = false; // sample can't be inside of lens

          // early out
          if (redistribute == false){
            filter_and_add_to_buffer(px, py, filter_width_half, 
                                    1.0, bokeh->current_inv_density, depth, transmitted_energy_in_sample, 0, sampleid,
                                    iterator, bokeh, crypto_cache);
            if (!transmitted_energy_in_sample) continue;
          }

          for(int count=0; count<samples && total_samples_taken < max_total_samples; ++count, ++total_samples_taken) {
            
            Eigen::Vector2d pixel;
            Eigen::Vector2d sensor_position(0, 0);
            
            Eigen::Vector3d camera_space_sample_position_mb_eigen = world_to_camera_space_motionblur(sample_pos_ws, bokeh->time_start, bokeh->time_end, bokeh->camera);  //could check if motionblur is enabled
            switch (po->unitModel){
              case mm: { camera_space_sample_position_mb_eigen *= 0.1; } break;
              case cm: { camera_space_sample_position_mb_eigen *= 1.0; } break;
              case dm: { camera_space_sample_position_mb_eigen *= 10.0;} break;
              case m:  { camera_space_sample_position_mb_eigen *= 100.0;}
            }

            if(!trace_backwards(-camera_space_sample_position_mb_eigen*10.0, po->aperture_radius, po->lambda, sensor_position, po->sensor_shift, po, px, py, total_samples_taken)) {
              --count;
              continue;
            }

            pixel = sensor_to_pixel_position(sensor_position, po->sensor_width, frame_aspect_ratio, bokeh->xres_without_region, bokeh->yres_without_region);

            // if outside of image
            if ((pixel(0) >= xres) || (pixel(0) < bokeh->region_min_x) || (pixel(1) >= yres) || (pixel(1) < bokeh->region_min_y) ||
                (pixel(0) != pixel(0)) || (pixel(1) != pixel(1))) // nan checking
            {
              --count; // much room for improvement here, potentially many samples are wasted outside of frame
              continue;
            }

            // >>>> currently i've decided not to filter the redistributed energy. If needed, there's an old prototype in github issue #230

            // write sample to image
            unsigned pixelnumber = coords_to_linear_pixel_region(floor(pixel(0)), floor(pixel(1)), bokeh->xres, bokeh->region_min_x, bokeh->region_min_y);

            for (unsigned i=0; i<bokeh->aov_list_name.size(); i++){
              std::string aov_name_str = bokeh->aov_list_name[i].c_str();
              if (aov_name_str.find("crypto_") != std::string::npos) {
                add_to_buffer_cryptomatte(pixelnumber, bokeh, crypto_cache[bokeh->aov_list_name[i]], bokeh->aov_list_name[i], (bokeh->current_inv_density/std::pow(bokeh->filter_width,2)) * inv_samples);
              } else {
                add_to_buffer(pixelnumber, bokeh->aov_list_type[i], bokeh->aov_list_name[i], 
                            inv_samples, bokeh->current_inv_density / std::pow(bokeh->filter_width,2), fitted_bidir_add_luminance, depth,
                            transmitted_energy_in_sample, 1, iterator, bokeh);
              }
            }
          }
        
          break;
        }
        case ThinLens:
        {
          // early out, before coc
          if (redistribute == false){
            filter_and_add_to_buffer(px, py, filter_width_half, 
                                    1.0, bokeh->current_inv_density, depth, transmitted_energy_in_sample, 0, sampleid,
                                    iterator, bokeh, crypto_cache);
            if (!transmitted_energy_in_sample) continue;
          }
          

          for(int count=0; count<samples && total_samples_taken<max_total_samples; ++count, ++total_samples_taken) {
            unsigned int seed = tea<8>((px*py+px), total_samples_taken);

            // world to camera space transform, motion blurred
            AtMatrix world_to_camera_matrix_motionblurred;
            float currenttime = linear_interpolate(rng(seed), bokeh->time_start, bokeh->time_end); // should I create new random sample, or can I re-use another one?
            AiWorldToCameraMatrix(bokeh->camera, currenttime, world_to_camera_matrix_motionblurred);
            AtVector camera_space_sample_position_mb = AiM4PointByMatrixMult(world_to_camera_matrix_motionblurred, sample_pos_ws);
            switch (po->unitModel){
              case mm: { camera_space_sample_position_mb *= 0.1; } break;
              case cm: { camera_space_sample_position_mb *= 1.0; } break;
              case dm: { camera_space_sample_position_mb *= 10.0;} break;
              case m:  { camera_space_sample_position_mb *= 100.0;}
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
            const float pixel_x = (( s(0) + 1.0) / 2.0) * bokeh->xres_without_region;
            const float pixel_y = ((-s(1) + 1.0) / 2.0) * bokeh->yres_without_region;

            // if outside of image
            if ((pixel_x >= xres) || (pixel_x < bokeh->region_min_x) || (pixel_y >= yres) || (pixel_y < bokeh->region_min_y)) {
              --count; // much room for improvement here, potentially many samples are wasted outside of frame, could keep track of a bbox
              continue;
            }

            // write sample to image
            unsigned pixelnumber = coords_to_linear_pixel_region(floor(pixel_x), floor(pixel_y), bokeh->xres, bokeh->region_min_x, bokeh->region_min_y);
            if (!redistribute) pixelnumber = coords_to_linear_pixel_region(px, py, bokeh->xres, bokeh->region_min_x, bokeh->region_min_y);

            // >>>> currently i've decided not to filter the redistributed energy. If needed, there's an old prototype in github issue #230

            for (unsigned i=0; i<bokeh->aov_list_name.size(); i++){
              std::string aov_name_str = bokeh->aov_list_name[i].c_str();
              if (aov_name_str.find("crypto_") != std::string::npos) {
                add_to_buffer_cryptomatte(pixelnumber, bokeh, crypto_cache[bokeh->aov_list_name[i]], bokeh->aov_list_name[i], (bokeh->current_inv_density/std::pow(bokeh->filter_width,2)) * inv_samples);
              } else {
                add_to_buffer(pixelnumber, bokeh->aov_list_type[i], bokeh->aov_list_name[i], 
                            inv_samples, bokeh->current_inv_density / std::pow(bokeh->filter_width,2), fitted_bidir_add_luminance, depth,
                            transmitted_energy_in_sample, 1, iterator, bokeh);
              }
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
          AtVector sample_pos_ws = AiAOVSampleIteratorGetAOVVec(iterator, bokeh->atstring_p);
          float depth = AiAOVSampleIteratorGetAOVFlt(iterator, bokeh->atstring_z);
          const float sample_luminance = (sample_energy.r + sample_energy.g + sample_energy.b)/3.0;
          if (sample_luminance > po->bidir_min_luminance || depth != AI_INFINITE ||  !AiV3IsSmall(sample_pos_ws)) {
              // const AtRGB red = AtRGB(1.0, 0.0, 0.0);
              // const AtRGB green = AtRGB(0.0, 1.0, 0.0);
              // const AtRGB heatmap_colors[2] = {red, green};
              // const float heatmap_positions[2] = {0.0f, 1.0f};

              // AtRGB heatmap = AiColorHeatMap(heatmap_colors, heatmap_positions, 2, sample_luminance);
              // value_out = AtRGBA(heatmap.r, heatmap.g, heatmap.b, 1.0);
              value_out = AtRGBA(1.0, 1.0, 1.0, 1.0);
              
          }
        }
      } else {
        value_out = filter_gaussian_complete(iterator, bokeh->filter_width, data_type, bokeh->current_inv_density);
      }

      *((AtRGBA*)data_out) = value_out;
      break;
    }
    case AI_TYPE_RGB: {
      AtRGBA filtered_value = filter_gaussian_complete(iterator, bokeh->filter_width, data_type, bokeh->current_inv_density);
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

 
node_finish {
   InternalFilterData *ifd = (InternalFilterData*)AiNodeGetLocalData(node);
   delete ifd;
}


void registerLentilFilterPO(AtNodeLib* node) {
    node->methods = (AtNodeMethods*) LentilFilterDataMtd;
    node->output_type = AI_TYPE_NONE;
    node->name = "lentil_filter";
    node->node_type = AI_NODE_FILTER;
    strcpy(node->version, AI_VERSION);
}