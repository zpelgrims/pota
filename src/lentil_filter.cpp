#include <ai.h>
#include "lentil.h"
#include "lens.h"


AI_FILTER_NODE_EXPORT_METHODS(LentilFilterDataMtd);


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


  bool rgba_aov = (AiAOVSampleIteratorGetAOVName(iterator) == camera_data->atstring_rgba); // early out for non-primary AOV samples
  bool adaptive_sampling = AiNodeGetBool(AiUniverseGetOptions(universe), "enable_adaptive_sampling"); 
  float inverse_sample_density = 0.0;
  
  // count samples because I cannot rely on AiAOVSampleIteratorGetInvDensity() any longer since 7.0.0.0. It only works for adaptive sampling.
  if (!adaptive_sampling && rgba_aov) {
    int samples_counter = 0;
    while (AiAOVSampleIteratorGetNext(iterator)) ++samples_counter;
    AiAOVSampleIteratorReset(iterator);
    float AA_samples = std::sqrt(samples_counter) / 2.0;
    inverse_sample_density = 1.0/(AA_samples*AA_samples);
    if (static_cast<int>(AA_samples) != AiNodeGetInt(AiUniverseGetOptions(universe), "AA_samples")){
      camera_data->redistribution = false; // skip when aa samples are below final AA samples
    }
  }


  if (camera_data->redistribution && rgba_aov){
    const double xres = (double)camera_data->xres;
    const double yres = (double)camera_data->yres;
    const double frame_aspect_ratio = xres/yres;

    int px, py;
    AiAOVSampleIteratorGetPixel(iterator, px, py);
    AtShaderGlobals *sg = AiShaderGlobals();


    for (int sampleid=0; AiAOVSampleIteratorGetNext(iterator)==true; sampleid++) {
      bool redistribute = true;

      if (adaptive_sampling) {
        inverse_sample_density = AiAOVSampleIteratorGetInvDensity(iterator);
        
        // skip AA < 3 (ipr passes, for example)
        if (inverse_sample_density > 0.2) redistribute = false;
      }

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
      std::map<AtString, std::map<float, float>> crypto_cache;
      if (camera_data->cryptomatte_lentil) camera_data->cryptomatte_construct_cache(crypto_cache, iterator, sampleid);


      // additional luminance with soft transition
      float fitted_bidir_add_luminance = 0.0;
      if (camera_data->bidir_add_luminance > 0.0) fitted_bidir_add_luminance = camera_data->additional_luminance_soft_trans(sample_luminance);


      float circle_of_confusion = camera_data->get_coc_thinlens(camera_space_sample_position);
      const float coc_squared_pixels = std::pow(circle_of_confusion * camera_data->yres, 2) * std::pow(camera_data->bidir_sample_mult,2) * 0.00001; // pixel area as baseline for sample count
      if (circle_of_confusion < 0.5) redistribute = false; // don't redistribute under certain CoC size
      int samples = std::ceil(coc_squared_pixels * inverse_sample_density); // aa_sample independence
      samples = clamp(samples, 5, 10000);
      float inv_samples = 1.0/static_cast<double>(samples);


      unsigned int total_samples_taken = 0;
      unsigned int max_total_samples = samples*5;


      // store all aov values
      std::vector<AtRGBA> aov_values(camera_data->aovs_upper_limit, AI_RGBA_ZERO);
      for (auto &aov : camera_data->aovs){
        if (aov.is_crypto) continue;
        if (aov.name == camera_data->atstring_lentil_debug){
          aov_values[aov.index] = static_cast<float>(samples) * redistribute;
          continue;
        }

        switch(aov.type){
          case AI_TYPE_RGBA: {
            aov_values[aov.index] = AiAOVSampleIteratorGetAOVRGBA(iterator, aov.name);
          } break;

          case AI_TYPE_RGB: {
            AtRGB value_rgb = AiAOVSampleIteratorGetAOVRGB(iterator, aov.name);
            aov_values[aov.index] = AtRGBA(value_rgb.r, value_rgb.g, value_rgb.b, 1.0);
          } break;

          case AI_TYPE_FLOAT: {
            float value_flt = AiAOVSampleIteratorGetAOVFlt(iterator, aov.name);
            aov_values[aov.index] = AtRGBA(value_flt, value_flt, value_flt, 1.0);
          } break;

          case AI_TYPE_VECTOR: {
            AtVector value_vec = AiAOVSampleIteratorGetAOVVec(iterator, aov.name);
            aov_values[aov.index] = AtRGBA(value_vec.x, value_vec.y, value_vec.z, 1.0);
          } break;
        }
      }


      switch (camera_data->cameraType){
        case PolynomialOptics:
        { 
          if (std::abs(camera_space_sample_position.z) < (camera_data->lens_length*0.1)) redistribute = false; // sample can't be inside of lens

          // early out
          if (redistribute == false){
            camera_data->filter_and_add_to_buffer(px, py, filter_width_half, 
                                    1.0, inverse_sample_density, depth, transmitted_energy_in_sample, 0,
                                    iterator, crypto_cache, aov_values);
            if (!transmitted_energy_in_sample) continue;
          }

          for(int count=0; count<samples && total_samples_taken < max_total_samples; ++count, ++total_samples_taken) {
            
            Eigen::Vector2d pixel;
            Eigen::Vector2d sensor_position(0, 0);            
            Eigen::Vector3d camera_space_sample_position_eigen(camera_space_sample_position.x, camera_space_sample_position.y, camera_space_sample_position.z);

            if(!camera_data->trace_ray_bw_po(-camera_space_sample_position_eigen*10.0, sensor_position, px, py, total_samples_taken, cam_to_world, sample_pos_ws, sg)) {
              --count;
              continue;
            }

            pixel = camera_data->sensor_to_pixel_position(sensor_position, frame_aspect_ratio);

            // if outside of image
            if ((pixel(0) >= xres) || (pixel(0) < 0) || (pixel(1) >= yres) || (pixel(1) < 0) ||
                (pixel(0) != pixel(0)) || (pixel(1) != pixel(1))) // nan checking
            {
              --count; // much room for improvement here, potentially many samples are wasted outside of frame
              continue;
            }

            // >>>> currently i've decided not to filter the redistributed energy. If needed, there's an old prototype in github issue #230

            // write sample to image
            unsigned pixelnumber = camera_data->coords_to_linear_pixel(floor(pixel(0)), floor(pixel(1)));

            for (auto &aov : camera_data->aovs){
              if (aov.is_crypto) camera_data->add_to_buffer_cryptomatte(aov, pixelnumber, crypto_cache[aov.name], (inverse_sample_density/std::pow(camera_data->filter_width,2)) * inv_samples);
              else camera_data->add_to_buffer(aov, pixelnumber, aov_values[aov.index],
                                 inv_samples, inverse_sample_density / std::pow(camera_data->filter_width,2), fitted_bidir_add_luminance, depth,
                                 transmitted_energy_in_sample, 1, iterator);
            }
          }
        } break;

        case ThinLens:
        {
          // early out
          if (redistribute == false){
            camera_data->filter_and_add_to_buffer(px, py, filter_width_half, 
                                    1.0, inverse_sample_density, depth, transmitted_energy_in_sample, 0,
                                    iterator, crypto_cache, aov_values);
            if (!transmitted_energy_in_sample) continue;
          }

          for(int count=0; count<samples && total_samples_taken<max_total_samples; ++count, ++total_samples_taken) {
            unsigned int seed = tea<8>((px*py+px), total_samples_taken);
            
            float image_dist_samplepos = (-camera_data->focal_length * camera_space_sample_position.z) / (-camera_data->focal_length + camera_space_sample_position.z);

            // either get uniformly distributed points on the unit disk or bokeh image
            Eigen::Vector2d unit_disk(0, 0);
            if (camera_data->bokeh_enable_image) camera_data->image.bokehSample(rng(seed),rng(seed), unit_disk, rng(seed), rng(seed));
            else if (camera_data->bokeh_aperture_blades < 2) concentricDiskSample(rng(seed),rng(seed), unit_disk, camera_data->abb_spherical, camera_data->circle_to_square, camera_data->bokeh_anamorphic);
            else camera_data->lens_sample_triangular_aperture(unit_disk(0), unit_disk(1), rng(seed),rng(seed), 1.0, camera_data->bokeh_aperture_blades);

            unit_disk(0) *= camera_data->bokeh_anamorphic;
            AtVector lens(unit_disk(0) * camera_data->aperture_radius, unit_disk(1) * camera_data->aperture_radius, 0.0);


            // aberration inputs
            // float abb_field_curvature = 0.0;


            // ray through center of lens
            AtVector dir_from_center = AiV3Normalize(camera_space_sample_position);
            AtVector dir_lens_to_P = AiV3Normalize(camera_space_sample_position - lens);
            // perturb ray direction to simulate coma aberration
            // todo: the bidirectional case isn't entirely the same as the forward case.. fix!
            // current strategy is to perturb the initial sample position by doing the same ray perturbation i'm doing in the forward case
            // float abb_coma = camera_data->abb_coma * abb_coma_multipliers(camera_data->sensor_width, camera_data->focal_length, dir_from_center, unit_disk);
            // dir_lens_to_P = abb_coma_perturb(dir_lens_to_P, dir_from_center, abb_coma, true);
            dir_lens_to_P = dir_from_center; // no coma
            AtVector camera_space_sample_position_perturbed = AiV3Length(camera_space_sample_position) * dir_lens_to_P;
            dir_from_center = AiV3Normalize(camera_space_sample_position_perturbed);

            float samplepos_image_intersection = std::abs(image_dist_samplepos/dir_from_center.z);
            AtVector samplepos_image_point = dir_from_center * samplepos_image_intersection;


            // depth of field
            AtVector dir_from_lens_to_image_sample = AiV3Normalize(samplepos_image_point - lens);

            float focusdist_intersection = std::abs(camera_data->get_image_dist_focusdist_thinlens()/dir_from_lens_to_image_sample.z);
            AtVector focusdist_image_point = lens + dir_from_lens_to_image_sample*focusdist_intersection;


            // raytrace for scene/geometrical occlusions along the ray
            AtVector lens_correct_scaled = lens;
            switch (camera_data->unitModel){
              case mm: { lens_correct_scaled /= 0.1; } break;
              case cm: { lens_correct_scaled /= 1.0; } break;
              case dm: { lens_correct_scaled /= 10.0;} break;
              case m:  { lens_correct_scaled /= 100.0;}
            }
            AtVector cam_pos_ws = AiM4PointByMatrixMult(cam_to_world, lens_correct_scaled);
            AtVector ws_direction = cam_pos_ws - sample_pos_ws;
            AtRay ray = AiMakeRay(AI_RAY_SHADOW, sample_pos_ws, &ws_direction, AI_BIG, sg);
            if (AiTraceProbe(ray, sg)){
              --count;
              continue;
            }

            // bring back to (x, y, 1)
            AtVector2 sensor_position(focusdist_image_point.x / focusdist_image_point.z,
                                      focusdist_image_point.y / focusdist_image_point.z);
            // transform to screenspace coordinate mapping
            sensor_position /= (camera_data->sensor_width*0.5)/-camera_data->focal_length;


            // optical vignetting
            dir_lens_to_P = AiV3Normalize(camera_space_sample_position_perturbed - lens);
            if (camera_data->optical_vignetting_distance > 0.0){
              // if (image_dist_samplepos<image_dist_focusdist) lens *= -1.0; // this really shouldn't be the case.... also no way i can do that in forward tracing?
              if (!empericalOpticalVignettingSquare(lens, dir_lens_to_P, camera_data->aperture_radius, camera_data->optical_vignetting_radius, camera_data->optical_vignetting_distance, lerp_squircle_mapping(camera_data->circle_to_square))){
                  --count;
                  continue;
              }
            }


            // barrel distortion (inverse)
            if (camera_data->abb_distortion > 0.0) sensor_position = inverseBarrelDistortion(AtVector2(sensor_position.x, sensor_position.y), camera_data->abb_distortion);
            

            // convert sensor position to pixel position
            Eigen::Vector2d s(sensor_position.x, sensor_position.y * frame_aspect_ratio);
            // const float pixel_x = (( s(0) + 1.0) / 2.0) * camera_data->xres_without_region;
            // const float pixel_y = ((-s(1) + 1.0) / 2.0) * camera_data->yres_without_region;
            const float pixel_x = (( s(0) + 1.0) / 2.0) * camera_data->xres;
            const float pixel_y = ((-s(1) + 1.0) / 2.0) * camera_data->yres;

            // if outside of image
            // if ((pixel_x >= xres) || (pixel_x < camera_data->region_min_x) || (pixel_y >= yres) || (pixel_y < camera_data->region_min_y)) {
            if ((pixel_x >= xres) || (pixel_x < 0) || (pixel_y >= yres) || (pixel_y < 0)) {
              --count; // much room for improvement here, potentially many samples are wasted outside of frame, could keep track of a bbox
              continue;
            }

            // write sample to image
            // unsigned pixelnumber = coords_to_linear_pixel_region(floor(pixel_x), floor(pixel_y), camera_data->xres, camera_data->region_min_x, camera_data->region_min_y);
            // if (!redistribute) pixelnumber = coords_to_linear_pixel_region(px, py, camera_data->xres, camera_data->region_min_x, camera_data->region_min_y);
            unsigned pixelnumber = camera_data->coords_to_linear_pixel(floor(pixel_x), floor(pixel_y));
            if (!redistribute) pixelnumber = camera_data->coords_to_linear_pixel(px, py);

            // >>>> currently i've decided not to filter the redistributed energy. If needed, there's an old prototype in github issue #230

            for (auto &aov : camera_data->aovs){
              if (aov.is_crypto) camera_data->add_to_buffer_cryptomatte(aov, pixelnumber, crypto_cache[aov.name], (inverse_sample_density/std::pow(camera_data->filter_width,2)) * inv_samples);
              else camera_data->add_to_buffer(aov, pixelnumber, aov_values[aov.index],
                                inv_samples, inverse_sample_density / std::pow(camera_data->filter_width,2), fitted_bidir_add_luminance, depth,
                                transmitted_energy_in_sample, 1, iterator);
            }
            
            
          }
        } break;
      }
    }

    AiShaderGlobalsDestroy(sg);
  } 
  

  // do regular filtering (passthrough) for display purposes
  AiAOVSampleIteratorReset(iterator);
  switch(data_type){
    case AI_TYPE_RGBA: {
      AtRGBA value_out = camera_data->filter_gaussian_complete(iterator, data_type, inverse_sample_density, adaptive_sampling);
      *((AtRGBA*)data_out) = value_out;
    } break;

    case AI_TYPE_RGB: {
      AtRGBA filtered_value = camera_data->filter_gaussian_complete(iterator, data_type, inverse_sample_density, adaptive_sampling);
      AtRGB rgb_energy {filtered_value.r, filtered_value.g, filtered_value.b};
      *((AtRGB*)data_out) = rgb_energy;
    } break;

    case AI_TYPE_VECTOR: {
      AtRGBA filtered_value = camera_data->filter_closest_complete(iterator, data_type);
      AtVector rgb_energy {filtered_value.r, filtered_value.g, filtered_value.b};
      *((AtVector*)data_out) = rgb_energy;
    } break;

    case AI_TYPE_FLOAT: {
      AtRGBA filtered_value = camera_data->filter_closest_complete(iterator, data_type);
      float rgb_energy = filtered_value.r;
      *((float*)data_out) = rgb_energy;
    } break;
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