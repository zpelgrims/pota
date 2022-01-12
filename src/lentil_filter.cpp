#include <ai.h>
#include "lentil.h"
#include "lens.h"


AI_FILTER_NODE_EXPORT_METHODS(LentilFilterDataMtd);





inline float additional_luminance_soft_trans(float sample_luminance, float additional_luminance, float transition_width, float minimum_luminance){
  // additional luminance with soft transition
  if (sample_luminance > minimum_luminance && sample_luminance < minimum_luminance+transition_width){
    float perc = (sample_luminance - minimum_luminance) / transition_width;
    return additional_luminance * perc;          
  } else if (sample_luminance > minimum_luminance+transition_width) {
    return additional_luminance;
  }

  return 0.0;
}







node_parameters 
{
  AiMetaDataSetBool(nentry, nullptr, "force_update", true);
}
 
node_initialize
{
  static const char *required_aovs[] = {"RGBA RGBA", "VECTOR P", "FLOAT Z", "FLOAT lentil_time", "RGB opacity", "RGBA transmission", "FLOAT lentil_bidir_ignore", NULL};
  AiFilterInitialize(node, true, required_aovs);
}


node_update 
{
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
      case AI_TYPE_FLOAT:
        // for some reason float->rgba is required by cryptomatte to output the suffixed aov's (correct). if float->float it only does "display" layers (incorrect)
        // does this affect any of my own work? not sure. I should test e.g depth aov.
        return AI_TYPE_RGBA; 
      default:
         return AI_TYPE_NONE;
   }
}


filter_pixel
{
  AtUniverse *universe = AiNodeGetUniverse(node);
  AtNode *camera_node = AiUniverseGetCamera(universe);
  Camera *camera_data = (Camera*)AiNodeGetLocalData(camera_node);

  // if (!AiNodeIs(camera_data->camera_node, AtString("lentil_camera"))) {
  //   camera_data->redistribution = false;
  //   AiMsgError("[LENTIL FILTER] Couldn't get correct camera. Please refresh the render.");
  //   AiRenderAbort();
  //   return;
  // }


  bool skip_redistribution_local = false; // camera_data->redistribution is a global switch, which also turns of the imager - this is local to the filter.

  // count samples because I cannot rely on AiAOVSampleIteratorGetInvDensity() any longer since 7.0.0.0
  int samples_counter = 0;
  while (AiAOVSampleIteratorGetNext(iterator)) ++samples_counter;
  AiAOVSampleIteratorReset(iterator);
  float AA_samples = std::sqrt(samples_counter) / 2.0;
  camera_data->current_inv_density = 1.0/(AA_samples*AA_samples);
  if (AA_samples != AiNodeGetInt(AiUniverseGetOptions(universe), "AA_samples")){
    camera_data->redistribution = false; // skip when aa samples are below final AA samples
  }
  

  if (AiAOVSampleIteratorGetAOVName(iterator) != camera_data->atstring_rgba) skip_redistribution_local = true; // early out for non-primary AOV samples
  

  if (camera_data->redistribution && !skip_redistribution_local){
    const double xres = (double)camera_data->xres;
    const double yres = (double)camera_data->yres;
    const double frame_aspect_ratio = xres/yres;

    int px, py;
    AiAOVSampleIteratorGetPixel(iterator, px, py);

    AtShaderGlobals *sg = AiShaderGlobals();

    for (int sampleid=0; AiAOVSampleIteratorGetNext(iterator)==true; sampleid++) {
      bool redistribute = true;

      AtRGBA sample = AiAOVSampleIteratorGetRGBA(iterator);
      AtVector sample_pos_ws = AiAOVSampleIteratorGetAOVVec(iterator, camera_data->atstring_p);
      float depth = AiAOVSampleIteratorGetAOVFlt(iterator, camera_data->atstring_z); // what to do when values are INF?

      float time = AiAOVSampleIteratorGetAOVFlt(iterator, camera_data->atstring_time);
      AtMatrix cam_to_world; AiCameraToWorldMatrix(camera_data->camera_node, time, cam_to_world);
      AtMatrix world_to_camera_matrix; AiWorldToCameraMatrix(camera_data->camera_node, time, world_to_camera_matrix);
      AtVector camera_space_sample_position = AiM4PointByMatrixMult(world_to_camera_matrix, sample_pos_ws);
      switch (camera_data->unitModel){
        case mm: { camera_space_sample_position *= 0.1; } break;
        case cm: { camera_space_sample_position *= 1.0; } break;
        case dm: { camera_space_sample_position *= 10.0;} break;
        case m:  { camera_space_sample_position *= 100.0;}
      }

      if (camera_data->cameraType == PolynomialOptics && std::abs(camera_space_sample_position.z) < (camera_data->lens_length*0.1)){
        redistribute = false; // sample can't be inside of lens
      }
      
      const float filter_width_half = std::ceil(camera_data->filter_width * 0.5);


      const AtRGBA sample_transmission = AiAOVSampleIteratorGetAOVRGBA(iterator, camera_data->atstring_transmission);
      bool transmitted_energy_in_sample = (AiColorMaxRGB(sample_transmission) > 0.0);
      if (transmitted_energy_in_sample){
        sample.r -= sample_transmission.r;
        sample.g -= sample_transmission.g;
        sample.b -= sample_transmission.b;
      }

      const float sample_luminance = (sample.r + sample.g + sample.b)/3.0;
      if (depth == AI_INFINITE || AiV3IsSmall(sample_pos_ws) || 
          sample_luminance < camera_data->bidir_min_luminance || 
          AiAOVSampleIteratorGetAOVFlt(iterator, camera_data->atstring_lentil_ignore) > 0.0) {
        redistribute = false;
      }

      // cryptomatte cache
      std::map<std::string, std::map<float, float>> crypto_cache;
      if (camera_data->cryptomatte_lentil) camera_data->cryptomatte_construct_cache(crypto_cache, iterator, sampleid);


      // additional luminance with soft transition
      float fitted_bidir_add_luminance = 0.0;
      if (camera_data->bidir_add_luminance > 0.0) fitted_bidir_add_luminance = additional_luminance_soft_trans(sample_luminance, camera_data->bidir_add_luminance, camera_data->bidir_add_luminance_transition, camera_data->bidir_min_luminance);


      float circle_of_confusion = camera_data->get_coc_thinlens(camera_space_sample_position);
      const float coc_squared_pixels = std::pow(circle_of_confusion * camera_data->yres, 2) * std::pow(camera_data->bidir_sample_mult,2) * 0.00001; // pixel area as baseline for sample count
      if (circle_of_confusion < 0.5) redistribute = false; // don't redistribute under certain CoC size
      int samples = std::ceil(coc_squared_pixels * camera_data->current_inv_density); // aa_sample independence
      samples = std::clamp(samples, 5, 10000);
      float inv_samples = 1.0/static_cast<double>(samples);


      unsigned int total_samples_taken = 0;
      unsigned int max_total_samples = samples*5;


      // store all aov values
      std::map<std::string, AtRGBA> aov_values;
      for (auto &aov : camera_data->aovs){
        if (aov.is_crypto) continue;

        switch(aov.type){
          case AI_TYPE_RGBA: {
            aov_values[aov.to.aov_name_tok] = AiAOVSampleIteratorGetAOVRGBA(iterator, aov.name_as);
          } break;

          case AI_TYPE_RGB: {
            AtRGB value_rgb = AiAOVSampleIteratorGetAOVRGB(iterator, aov.name_as);
            aov_values[aov.to.aov_name_tok] = AtRGBA(value_rgb.r, value_rgb.g, value_rgb.b, 1.0);
          } break;

          case AI_TYPE_FLOAT: {
            if (aov.to.aov_name_tok == "lentil_debug"){
              aov_values[aov.to.aov_name_tok] = static_cast<float>(samples) * redistribute;
              continue;
            }

            float value_flt = AiAOVSampleIteratorGetAOVFlt(iterator, aov.name_as);
            aov_values[aov.to.aov_name_tok] = AtRGBA(value_flt, value_flt, value_flt, 1.0);
          } break;

          case AI_TYPE_VECTOR: {
            AtVector value_vec = AiAOVSampleIteratorGetAOVVec(iterator, aov.name_as);
            aov_values[aov.to.aov_name_tok] = AtRGBA(value_vec.x, value_vec.y, value_vec.z, 1.0);
          } break;
        }
      }

      
      // early out
      if (redistribute == false){
        camera_data->filter_and_add_to_buffer(px, py, filter_width_half, 
                                1.0, camera_data->current_inv_density, depth, transmitted_energy_in_sample, 0,
                                iterator, crypto_cache, aov_values);
        if (!transmitted_energy_in_sample) continue;
      }

      for(int count=0; count<samples && total_samples_taken < max_total_samples; ++count, ++total_samples_taken) {
            Eigen::Vector2d pixel(0,0);
            Eigen::Vector2d sensor_position(0, 0);            
            Eigen::Vector3d camera_space_sample_position_eigen(camera_space_sample_position.x, camera_space_sample_position.y, camera_space_sample_position.z);

            if (camera_data->cameraType == ThinLens) {
              if(!camera_data->trace_ray_bw_tl(-camera_space_sample_position, sensor_position, px, py, total_samples_taken, cam_to_world, sample_pos_ws, sg)) {
                --count;
                continue;
              }
            } else if (camera_data->cameraType == PolynomialOptics) {
              if(!camera_data->trace_ray_bw_po(-camera_space_sample_position_eigen*10.0, sensor_position, px, py, total_samples_taken, cam_to_world, sample_pos_ws, sg)) {
                --count;
                continue;
              }
            }

            pixel = camera_data->sensor_to_pixel_position(sensor_position, frame_aspect_ratio);

            // if outside of image
            // if ((pixel_x >= xres) || (pixel_x < camera_data->region_min_x) || (pixel_y >= yres) || (pixel_y < camera_data->region_min_y)) {
            if ((pixel(0) >= xres) || (pixel(0) < 0) || (pixel(1) >= yres) || (pixel(1) < 0) ||
                (pixel(0) != pixel(0)) || (pixel(1) != pixel(1))) // nan checking
            {
              --count; // much room for improvement here, potentially many samples are wasted outside of frame
              continue;
            }

            // write sample to image
            // unsigned pixelnumber = coords_to_linear_pixel_region(floor(pixel(0)), floor(pixel(1)), camera_data->xres, camera_data->region_min_x, camera_data->region_min_y);
            // if (!redistribute) pixelnumber = coords_to_linear_pixel_region(px, py, camera_data->xres, camera_data->region_min_x, camera_data->region_min_y);
            unsigned pixelnumber = camera_data->coords_to_linear_pixel(std::floor(pixel(0)), std::floor(pixel(1)));
            if (!redistribute) pixelnumber = camera_data->coords_to_linear_pixel(px, py); // QUESTION: should this also exist for PO??

            // >>>> currently i've decided not to filter the redistributed energy. If needed, there's an old prototype in github issue #230

            for (auto &aov : camera_data->aovs){
              if (aov.is_crypto) camera_data->add_to_buffer_cryptomatte(aov, pixelnumber, crypto_cache[aov.to.aov_name_tok], (camera_data->current_inv_density/std::pow(camera_data->filter_width,2)) * inv_samples);
              else camera_data->add_to_buffer(aov, pixelnumber, aov_values[aov.to.aov_name_tok],
                                inv_samples, camera_data->current_inv_density / std::pow(camera_data->filter_width,2), fitted_bidir_add_luminance, depth,
                                transmitted_energy_in_sample, 1, iterator);
            }
      }
    }
  } 
}

 
node_finish {}


void registerLentilFilter(AtNodeLib* node) {
    node->methods = (AtNodeMethods*) LentilFilterDataMtd;
    node->output_type = AI_TYPE_NONE;
    node->name = "lentil_filter";
    node->node_type = AI_NODE_FILTER;
    strcpy(node->version, AI_VERSION);
}