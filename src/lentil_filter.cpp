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
  static const char *required_aovs[] = {"RGBA RGBA", "VECTOR P", "FLOAT Z", "FLOAT lentil_time", "FLOAT lentil_debug", "RGB opacity", "RGBA transmission", "FLOAT lentil_bidir_ignore", NULL};
  AiFilterInitialize(node, false, required_aovs);
}


node_update 
{
  AiFilterUpdate(node, 1.5);
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

  int aa_samples_set_by_user = AiNodeGetInt(AiUniverseGetOptions(universe), AtString("AA_samples"));
  bool rgba_aov = (AiAOVSampleIteratorGetAOVName(iterator) == camera_data->atstring_rgba); // early out for non-primary AOV samples
  bool adaptive_sampling = AiNodeGetBool(AiUniverseGetOptions(universe), AtString("enable_adaptive_sampling")); 
  float inverse_sample_density = 0.0;

  
  // count samples because I cannot rely on AiAOVSampleIteratorGetInvDensity() any longer since 7.0.0.0. It only works for adaptive sampling.
  if (!adaptive_sampling && rgba_aov) {
    int samples_counter = 0;
    while (AiAOVSampleIteratorGetNext(iterator)) ++samples_counter;
    AiAOVSampleIteratorReset(iterator);
    float AA_samples = std::sqrt(samples_counter) / camera_data->filter_width;
    inverse_sample_density = 1.0/(AA_samples*AA_samples);
    if (static_cast<int>(std::round(AA_samples)) != aa_samples_set_by_user || (aa_samples_set_by_user < 3)){
      camera_data->redistribution = false; // skip when aa samples are below final AA samples
    }
  }


  if (camera_data->redistribution && rgba_aov){
    const double xres = (double)camera_data->xres;
    const double yres = (double)camera_data->yres;
    const double frame_aspect_ratio = (xres)/yres;
    const double frame_aspect_ratio_without_region = (double)camera_data->xres_without_region/(double)camera_data->yres_without_region;

    int px, py;
    AiAOVSampleIteratorGetPixel(iterator, px, py);
    
    px -= camera_data->region_min_x;
    py -= camera_data->region_min_y;
    AtShaderGlobals *shaderglobals = AiShaderGlobals();


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
      
      const AtRGBA sample_transmission = AiAOVSampleIteratorGetAOVRGBA(iterator, camera_data->atstring_transmission);
      bool transmitted_energy_in_sample = camera_data->enable_bidir_transmission ? false : (AiColorMaxRGB(sample_transmission) > 0.0);
      if (transmitted_energy_in_sample){
        sample.r -= sample_transmission.r;
        sample.g -= sample_transmission.g;
        sample.b -= sample_transmission.b;
      }
      if (transmitted_energy_in_sample) redistribute = false;

      const float sample_luminance = (sample.r + sample.g + sample.b)/3.0;
      if (depth == AI_INFINITE || AiV3IsSmall(sample_pos_ws) || AiAOVSampleIteratorGetAOVFlt(iterator, camera_data->atstring_lentil_ignore) > 0.0) {
        redistribute = false;
      }

      // cryptomatte cache
      std::vector<std::map<float, float>> crypto_cache(camera_data->aovcount);
      if (camera_data->cryptomatte_lentil) camera_data->cryptomatte_construct_cache(crypto_cache, iterator, sampleid);


      // additional luminance with soft transition
      float fitted_bidir_add_energy = 0.0;
      if (camera_data->bidir_add_energy > 0.0) fitted_bidir_add_energy = camera_data->additional_luminance_soft_trans(sample_luminance);


      float luminance_mult = std::max(0.0, std::pow(std::min(sample_luminance, 20.0f), 0.5) * camera_data->bidir_sample_mult); // ^0.5 to slightly tweak the sample_luminance curve, clamping at luminance 50
      float circle_of_confusion = camera_data->get_coc_thinlens(camera_space_sample_position);
      const float coc_squared_pixels = std::pow(circle_of_confusion * camera_data->yres, 2) * std::pow(luminance_mult, 2) * 0.00001; // pixel area as baseline for sample count
      if (circle_of_confusion < 0.4) redistribute = false; // don't redistribute under certain CoC size, emperically tested
      int samples = std::ceil(coc_squared_pixels * inverse_sample_density); // aa_sample independence
      samples = clamp(samples, 4, 5000);
      float inv_samples = 1.0/static_cast<float>(samples);


      unsigned int total_samples_taken = 0;
      unsigned int max_total_samples = samples*5;


      // store all aov values
      std::vector<AtRGBA> aov_values(camera_data->aovcount, AI_RGBA_ZERO);
      for (auto &aov : camera_data->aovs){
        if (aov.is_crypto) continue;
        if (aov.name == camera_data->atstring_lentil_debug){
          aov_values[aov.index] = samples * redistribute;
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
            camera_data->filter_and_add_to_buffer_new(px, py, 
                                    depth, iterator, crypto_cache, aov_values, inverse_sample_density);
            if (!transmitted_energy_in_sample) continue;
          }

          for(int count=0; count<samples && total_samples_taken < max_total_samples; ++count, ++total_samples_taken) {
            
            Eigen::Vector2d sensor_position(0, 0);            
            Eigen::Vector3d camera_space_sample_position_eigen(camera_space_sample_position.x, camera_space_sample_position.y, camera_space_sample_position.z);

            // ca, works quite well
            AtRGB rgb_weight = AI_RGB_WHITE;
            float lambda_per_sample = 0.55;
            // const int rand_ch = static_cast<int>(std::floor((xor128() / 4294967296.0) * 3.0));
            // const float weight = 0.1;

            // if (rand_ch == 0){
            //   lambda_per_sample = linear_interpolate(1.0-weight, 0.35, 0.55);
            //   rgb_weight *= AtRGB(3,0,0);
            // } else if (rand_ch == 1) {
            //   lambda_per_sample = 0.55;
            //   rgb_weight *= AtRGB(0,3,0);
            // } else if (rand_ch == 2) {
            //   lambda_per_sample = linear_interpolate(weight, 0.55, 0.85);
            //   rgb_weight *= AtRGB(0,0,3);
            // }

            if(!camera_data->trace_ray_bw_po(-camera_space_sample_position_eigen*10.0, sensor_position, px, py, total_samples_taken, cam_to_world, sample_pos_ws, shaderglobals, lambda_per_sample)) {
              --count;
              continue;
            }

            const Eigen::Vector2d s(sensor_position(0) / (camera_data->sensor_width * 0.5), sensor_position(1) / (camera_data->sensor_width * 0.5) * frame_aspect_ratio_without_region);
            const Eigen::Vector2d pixel(((( s(0) + 1.0) / 2.0) * camera_data->xres_without_region) - camera_data->region_min_x, 
                                        (((-s(1) + 1.0) / 2.0) * camera_data->yres_without_region) - camera_data->region_min_y);
        

            // if outside of image
            if ((pixel(0) >= xres) || (pixel(0) < 0) || (pixel(1) >= yres) || (pixel(1) < 0) ||
                (pixel(0) != pixel(0)) || (pixel(1) != pixel(1))) // nan checking
            {
              --count; // much room for improvement here, potentially many samples are wasted outside of frame
              continue;
            }

            // unsigned pixelnumber = camera_data->coords_to_linear_pixel(floor(pixel(0)), floor(pixel(1)));
            unsigned pixelnumber = redistribute ? camera_data->coords_to_linear_pixel(floor(pixel(0)), floor(pixel(1))) : camera_data->coords_to_linear_pixel(px, py);

            // box filtering, see thin-lens
            float filter_weight = 1.0;

            for (auto &aov : camera_data->aovs){
                if (aov.is_crypto) camera_data->add_to_buffer_cryptomatte(aov, pixelnumber, crypto_cache[aov.index], inverse_sample_density * inv_samples);
                else camera_data->add_to_buffer(aov, pixelnumber, aov_values[aov.index], fitted_bidir_add_energy, depth, iterator, filter_weight * inverse_sample_density * inv_samples, rgb_weight); 
            }
          }
        } break;

        case ThinLens:
        {
          // early out
          if (redistribute == false){
            camera_data->filter_and_add_to_buffer_new(px, py, 
                                    depth, iterator, crypto_cache, aov_values, inverse_sample_density);
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


            // ray through center of lens
            AtVector dir_from_center = AiV3Normalize(camera_space_sample_position);
            AtVector dir_lens_to_P = AiV3Normalize(camera_space_sample_position - lens);

            // perturb ray direction to simulate coma aberration
            // todo: the bidirectional case isn't entirely the same as the forward case.. fix!
            // current strategy is to perturb the initial sample position by doing the same ray perturbation i'm doing in the forward case
            float abb_coma_multiplied = camera_data->abb_coma * abb_coma_multipliers(camera_data->sensor_width, camera_data->focal_length, dir_from_center, unit_disk);
            dir_lens_to_P = abb_coma_perturb(dir_lens_to_P, dir_from_center, abb_coma_multiplied, true);

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
            AtVector ws_direction = AiV3Normalize(cam_pos_ws - sample_pos_ws);
            AtRay ray = AiMakeRay(AI_RAY_SHADOW, sample_pos_ws, &ws_direction, AiV3Dist(cam_pos_ws, sample_pos_ws), shaderglobals);
            AtScrSample hit = AtScrSample();
            if (AiTrace(ray, AI_RGB_WHITE, hit)){
              // if (hit.point.x != 0.0) AiMsgInfo("hit.point: %f %f %f", hit.point.x, hit.point.y, hit.point.z);
              // if (hit.opacity != AI_RGB_WHITE) AiMsgInfo("hit.opacity: %f %f %f", hit.opacity.r, hit.opacity.g, hit.opacity.b);
              //   AiMsgInfo("uhoh");
              // }
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

            // doesn't work yet, rethink the whole thing.. shouldn't i be 
            AtRGB rgb_weight = AI_RGB_WHITE;
            // rgb_weight *= 3.0;
            // AtVector2 lens_ca(lens.x, lens.y);
            // float emperical_ca_dist = 0.01;
            // if (emperical_ca_dist > 0.0){
            //     const AtVector2 p2(lens_ca.x, lens_ca.y);
            //     const float distance_to_center = AiV2Dist(AtVector2(0.0, 0.0), sensor_position);
            //     const int random_aperture = static_cast<int>(std::floor((xor128() / 4294967296.0) * 3.0));
            //     AtVector2 aperture_0_center(0.0, 0.0);
            //     AtVector2 aperture_1_center(- lens_ca * circle_of_confusion * distance_to_center * emperical_ca_dist);
            //     AtVector2 aperture_2_center(lens_ca * circle_of_confusion * distance_to_center * emperical_ca_dist);
                

            //     if (random_aperture == 1)      lens_ca += aperture_1_center;
            //     else if (random_aperture == 2) lens_ca += aperture_2_center;

            //     if (std::pow(lens_ca.x-aperture_1_center.x, 2) + std::pow(lens_ca.y - aperture_1_center.y, 2) > std::pow((camera_data->aperture_radius*10.0)/camera_data->focus_distance, 2)) {
            //         rgb_weight.r = 0.0;
            //     }
            //     if (std::pow(lens_ca.x-aperture_0_center.x, 2) + std::pow(lens_ca.y - aperture_0_center.y, 2) > std::pow((camera_data->aperture_radius*10.0)/camera_data->focus_distance, 2)) {
            //         rgb_weight.b = 0.0;
            //     }
            //     if (std::pow(lens_ca.x-aperture_2_center.x, 2) + std::pow(lens_ca.y - aperture_2_center.y, 2) > std::pow((camera_data->aperture_radius*10.0)/camera_data->focus_distance, 2)) {
            //         rgb_weight.g = 0.0;
            //     }

            //     if (rgb_weight == AI_RGB_ZERO){
            //         --count;
            //         continue;
            //     }
            // }


            // barrel distortion (inverse)
            if (camera_data->abb_distortion > 0.0) sensor_position = inverseBarrelDistortion(AtVector2(sensor_position.x, sensor_position.y), camera_data->abb_distortion);
            

            // convert sensor position to pixel position
            Eigen::Vector2d s(sensor_position.x, sensor_position.y * frame_aspect_ratio_without_region);
            const float pixel_x = ((( s(0) + 1.0) / 2.0) * camera_data->xres_without_region) - camera_data->region_min_x;
            const float pixel_y = (((-s(1) + 1.0) / 2.0) * camera_data->yres_without_region) - camera_data->region_min_y;

            // if outside of image
            if ((pixel_x >= xres) || (pixel_x < 0) || (pixel_y >= yres) || (pixel_y < 0)) {
              --count; // much room for improvement here, potentially many samples are wasted outside of frame, could keep track of a bbox
              continue;
            }

            unsigned pixelnumber = redistribute ? camera_data->coords_to_linear_pixel(floor(pixel_x), floor(pixel_y)) : camera_data->coords_to_linear_pixel(px, py);

            // couldn't get gaussian filtering to work yet... so box filtering for now.
            // AtVector2 offset_from_pixel_center(std::abs(0.5 - fmod(pixel_x, 1)), std::abs(0.5 - fmod(pixel_y, 1)));
            // float filter_weight = camera_data->filter_weight_gaussian(offset_from_pixel_center, 2.0);
            // if (filter_weight == 0) continue;
            float filter_weight = 1.0;

            for (auto &aov : camera_data->aovs){
                if (aov.is_crypto) camera_data->add_to_buffer_cryptomatte(aov, pixelnumber, crypto_cache[aov.index], inverse_sample_density * inv_samples);
                else camera_data->add_to_buffer(aov, pixelnumber, aov_values[aov.index], fitted_bidir_add_energy, depth, iterator, filter_weight * inverse_sample_density * inv_samples, rgb_weight); 
            }
          }
        } break;
      }
    }
    AiShaderGlobalsDestroy(shaderglobals);
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