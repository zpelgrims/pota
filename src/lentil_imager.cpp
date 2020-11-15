#include <ai.h>
#include <algorithm>
#include "global.h"
#include "lentil.h"
#include "lens.h"





void trace_backwards_thinlens(int s, LentilFilterData *filter_data, Camera *po){

  const float coc_squared_pixels = std::pow(filter_data->circle_of_confusion[s] * filter_data->yres, 2) * std::pow(po->bidir_sample_mult,2) * 0.001; // pixel area as baseline for sample count


  // additional luminance with soft transition
  float fitted_bidir_add_luminance = 0.0;
  if (po->bidir_add_luminance > 0.0) fitted_bidir_add_luminance = additional_luminance_soft_trans(filter_data->sample_luminance[s], po->bidir_add_luminance, po->bidir_add_luminance_transition, po->bidir_min_luminance);
            

  int samples = std::ceil(coc_squared_pixels * filter_data->inv_density[s]); // aa_sample independence
  samples = std::clamp(samples, 5, 10000); // not sure if a million is actually ever hit..
  float inv_samples = 1.0/static_cast<double>(samples);

  unsigned int total_samples_taken = 0;
  unsigned int max_total_samples = samples*5;

  for(int count=0; count<samples && total_samples_taken<max_total_samples; ++count) {
    ++total_samples_taken;

    unsigned int seed = tea<8>((filter_data->x[s]*filter_data->y[s]+filter_data->x[s]), total_samples_taken);

    // world to camera space transform, motion blurred
    AtMatrix world_to_camera_matrix_motionblurred;
    float currenttime = linear_interpolate(rng(seed), filter_data->time_start, filter_data->time_end); // should I create new random sample, or can I re-use another one?
    AiWorldToCameraMatrix(AiUniverseGetCamera(), currenttime, world_to_camera_matrix_motionblurred);
    AtVector camera_space_sample_position_mb = AiM4PointByMatrixMult(world_to_camera_matrix_motionblurred, filter_data->sample_pos_ws[s]);
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
    Eigen::Vector2d sensor(sensor_position.x, sensor_position.y * filter_data->frame_aspect_ratio);
    const float pixel_x = (( sensor(0) + 1.0) / 2.0) * filter_data->xres;
    const float pixel_y = ((-sensor(1) + 1.0) / 2.0) * filter_data->yres;

    // if outside of image
    if ((pixel_x >= filter_data->xres) || (pixel_x < 0) || (pixel_y >= filter_data->yres) || (pixel_y < 0)) {
      --count; // much room for improvement here, potentially many samples are wasted outside of frame, could keep track of a bbox
      continue;
    }

    // write sample to image
    unsigned pixelnumber = static_cast<int>(filter_data->xres * floor(pixel_y) + floor(pixel_x));
    // if (!redistribute) pixelnumber = static_cast<int>(filter_data->xres * filter_data->y[s] + filter_data->x[s]);

    // >>>> currently i've decided not to filter the redistributed energy. If needed, there's an old prototype in github issue #230

    for (unsigned i=0; i<filter_data->aov_list_name.size(); i++){
      add_to_buffer(pixelnumber, filter_data->aov_list_type[i], filter_data->aov_list_name[i], 
                    inv_samples, filter_data->inv_density[s] / std::pow(filter_data->filter_width,2), fitted_bidir_add_luminance, filter_data->depth[s],
                    filter_data->transmitted_energy_in_sample[s], 1, filter_data, filter_data->aov[i][s], filter_data->sample_transmission[s]);
    
    }
  }
}


void trace_backwards_po(int s, LentilFilterData *filter_data, Camera *po){


  Eigen::Vector2d sensor_position(0, 0);

  // additional luminance with soft transition
  float fitted_bidir_add_luminance = 0.0;
  if (po->bidir_add_luminance > 0.0) fitted_bidir_add_luminance = additional_luminance_soft_trans(filter_data->sample_luminance[s], po->bidir_add_luminance, po->bidir_add_luminance_transition, po->bidir_min_luminance);


  int samples = std::floor(filter_data->bbox_area[s] * std::pow(po->bidir_sample_mult,2) * 0.001);
  samples = std::ceil((double)(samples) * filter_data->inv_density[s]);
  samples = std::clamp(samples, 5, 10000); // not sure if a million is actually ever hit.. 75 seems high but is needed to remove stochastic noise
  float inv_samples = 1.0 / static_cast<double>(samples);

  unsigned int total_samples_taken = 0;
  unsigned int max_total_samples = samples*5;
    

  for(int count=0; count<samples && total_samples_taken < max_total_samples; ++count, ++total_samples_taken) {
    
    Eigen::Vector2d pixel;
    
    // if (count < pixelcache.size()){ // redist already taken samples from probe rays
    //   pixel(0) = pixelcache[count].x; pixel(1) = pixelcache[count].y;
    // } else {


      Eigen::Vector3d camera_space_sample_position_mb_eigen = world_to_camera_space_motionblur(filter_data->sample_pos_ws[s], filter_data->time_start, filter_data->time_end);  //could check if motionblur is enabled
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

      if(!trace_backwards(-camera_space_sample_position_mb_eigen*10.0, po->aperture_radius, po->lambda, sensor_position, po->sensor_shift, po, filter_data->x[s], filter_data->y[s], total_samples_taken)) {
        --count;
        continue;
      }

      pixel = sensor_to_pixel_position(sensor_position, po->sensor_width, filter_data->frame_aspect_ratio, filter_data->xres, filter_data->yres);

      // if outside of image
      if ((pixel(0) >= filter_data->xres) || (pixel(0) < 0) || (pixel(1) >= filter_data->yres) || (pixel(1) < 0) ||
          (pixel(0) != pixel(0)) || (pixel(1) != pixel(1))) // nan checking
      {
        --count; // much room for improvement here, potentially many samples are wasted outside of frame
        continue;
      }
    // }

    // >>>> currently i've decided not to filter the redistributed energy. If needed, there's an old prototype in github issue #230

    // write sample to image
    unsigned pixelnumber = static_cast<int>(filter_data->xres * floor(pixel(1)) + floor(pixel(0)));

    for (unsigned i=0; i<filter_data->aov_list_name.size(); i++){
      add_to_buffer(pixelnumber, filter_data->aov_list_type[i], filter_data->aov_list_name[i], 
                    inv_samples, filter_data->inv_density[s] / std::pow(filter_data->filter_width,2), fitted_bidir_add_luminance, filter_data->depth[s],
                    filter_data->transmitted_energy_in_sample[s], 1, filter_data, filter_data->aov[i][s], filter_data->sample_transmission[s]);
    }
  }
}



// currently this works by searching for a node with specific name "lentil_replaced_filter", not ideal.

#define AI_DRIVER_SCHEDULE_FULL 0x02

AI_DRIVER_NODE_EXPORT_METHODS(LentilImagerMtd);

// struct LentilImagerData {
//     AtString camera_node_type;
//     AtString lentil_thinlens_string;
//     AtString lentil_po_string;
//     AtNode *camera_node;
// };

node_parameters 
{
  AiMetaDataSetStr(nentry, nullptr, AtString("subtype"), AtString("imager"));
  // AiParameterStr(AtString("layer_selection"), AtString("*")); // if enabled, mtoa/c4dtoa will only run over rgba (hardcoded for now)
  AiParameterBool(AtString("enable"), true);
  AiMetaDataSetBool(nentry, nullptr, "force_update", true);
}
 
node_initialize
{
  // LentilImagerData* imager_data = (LentilImagerData*)AiMalloc(sizeof(LentilImagerData));
  // AiNodeSetLocalData(node, imager_data);  
  AiDriverInitialize(node, false);
}
 
node_update 
{
  AiRenderSetHintInt(AtString("imager_schedule"), AI_DRIVER_SCHEDULE_FULL);
  AiRenderSetHintInt(AtString("imager_padding"), 0);

  // LentilImagerData* imager_data = (LentilImagerData*)AiNodeGetLocalData(node);
  // const AtNode *bokeh_filter_node = AiNodeLookUpByName("lentil_replaced_filter");
  // LentilFilterData *filter_data = (LentilFilterData*)AiNodeGetLocalData(bokeh_filter_node);
  // if (filter_data->enabled) AiMsgInfo("[LENTIL IMAGER] Starting Imager.");
}
 
driver_supports_pixel_type 
{
  return  pixel_type == AI_TYPE_RGBA || 
          pixel_type == AI_TYPE_RGBA || 
          pixel_type == AI_TYPE_FLOAT || 
          pixel_type == AI_TYPE_VECTOR;
}
 
driver_open {}
 
driver_extension
{
   static const char *extensions[] = {NULL};
   return extensions;
}
 
driver_needs_bucket
{
   return true; // API: true if the bucket needs to be rendered, false if it can be skipped
}
 
driver_prepare_bucket {} // called before a bucket is rendered


 
driver_process_bucket {
  AiOutputIteratorReset(iterator);
  // LentilImagerData* imager_data = (LentilImagerData*)AiNodeGetLocalData(node);

  const AtNode *bokeh_filter_node = AiNodeLookUpByName("lentil_replaced_filter");
  // don't run if lentil_replaced_filter node is not present
  if (bokeh_filter_node == nullptr) {
    AiMsgInfo("[LENTIL IMAGER] Skipping imager, could not find lentil_filter");
    return;
  }

  LentilFilterData *filter_data = (LentilFilterData*)AiNodeGetLocalData(bokeh_filter_node);

  if (!filter_data->enabled) {
    AiMsgInfo("[LENTIL IMAGER] Skipping imager");
    return;
  }

  // BUG: this could potentially fail when using adaptive sampling?
  if (filter_data->current_inv_density > 0.2) {
    AiMsgInfo("[LENTIL IMAGER] Skipping imager, AA samples < 3");
    return;
  }



  if (!AiNodeIs(AiUniverseGetCamera(), AtString("lentil_camera"))) {
    filter_data->enabled = false;
    AiMsgError("[LENTIL FILTER] Couldn't get correct camera. Please refresh the render.");
    AiRenderAbort();
    return;
  }

  Camera *po = (Camera*)AiNodeGetLocalData(AiUniverseGetCamera());
  

  AiMsgInfo("starting bidirectional sampling");

  switch (po->cameraType){
    case PolynomialOptics:
    {
      #pragma omp parallel for
      for (int s = 0; s<filter_data->global_run_counter; s++){
        trace_backwards_po(s, filter_data, po);
      }

      break;
    }
    case ThinLens:
    {
      #pragma omp parallel for
      for (int s = 0; s<filter_data->global_run_counter; s++){
        trace_backwards_thinlens(s, filter_data, po);
      }
      break;
    }
  }
      
    

  AiMsgInfo("Starting dump to arnold framebuffer");





  const char *aov_name_cstr = 0;
  int aov_type = 0;
  const void *bucket_data;

  while (AiOutputIteratorGetNext(iterator, &aov_name_cstr, &aov_type, &bucket_data)){
    AiMsgInfo("[LENTIL IMAGER] Imager found AOV: %s", aov_name_cstr);
    if (std::find(filter_data->aov_list_name.begin(), filter_data->aov_list_name.end(), AtString(aov_name_cstr)) != filter_data->aov_list_name.end()){
      if (AtString(aov_name_cstr) == AtString("transmission")) continue;
      AiMsgInfo("[LENTIL IMAGER] %s writing to: %s", AiNodeGetName(node), aov_name_cstr);
      AtString aov_name = AtString(aov_name_cstr);

      for (int j = 0; j < bucket_size_y; ++j) {
        for (int i = 0; i < bucket_size_x; ++i) {
          int y = j + bucket_yo;
          int x = i + bucket_xo;
          int in_idx = j * bucket_size_x + i;
          int linear_pixel = x + (y * (double)filter_data->xres);
          
          switch (aov_type){
            case AI_TYPE_RGBA: {
              AtRGBA image = filter_data->image_col_types[aov_name][linear_pixel];
              if (((AtRGBA*)bucket_data)[in_idx].a >= 1.0) image /= (image.a == 0.0) ? 1.0 : image.a;
              
              ((AtRGBA*)bucket_data)[in_idx] = image;
              break;
            }

            case AI_TYPE_RGB: {
              AtRGBA image = filter_data->image_col_types[aov_name][linear_pixel];
              image /= (image.a == 0.0) ? 1.0 : image.a;

              AtRGB final_value = AtRGB(image.r, image.g, image.b);
              ((AtRGB*)bucket_data)[in_idx] = final_value;
              break;
            }

            case AI_TYPE_FLOAT: {
              ((float*)bucket_data)[in_idx] = filter_data->image_data_types[aov_name][linear_pixel].r;
              break;
            }

            case AI_TYPE_VECTOR: {
              AtVector final_value (filter_data->image_data_types[aov_name][linear_pixel].r, 
                                    filter_data->image_data_types[aov_name][linear_pixel].g,
                                    filter_data->image_data_types[aov_name][linear_pixel].b);
              ((AtVector*)bucket_data)[in_idx] = final_value;
              break;
            }

            // case AI_TYPE_INT: {
            //   ((int*)bucket_data)[in_idx] = filter_data->image_data_types[aov_name][linear_pixel].r;
            //   break;
            // }

            // case AI_TYPE_UINT: {
            //   ((unsigned int*)bucket_data)[in_idx] = std::abs(filter_data->image_data_types[aov_name][linear_pixel].r);
            //   break;
            // }

            // case AI_TYPE_POINTER: {
            //   ((const void**)bucket_data)[in_idx] = filter_data->image_ptr_types[aov_name][linear_pixel];
            //   break;
            // }
          }
        }
      }
    }
  }
}


driver_write_bucket {}
 
driver_close {}
 
node_finish {
  // LentilImagerData* imager_data = (LentilImagerData*)AiNodeGetLocalData(node);
  // delete imager_data;
  crypto_crit_sec_enter();
  const AtNode *bokeh_filter_node = AiNodeLookUpByName("lentil_replaced_filter");
  LentilFilterData *bokeh = (LentilFilterData*)AiNodeGetLocalData(bokeh_filter_node);
  delete bokeh;
  crypto_crit_sec_leave();
}


 void registerLentilImager(AtNodeLib* node) {
    node->methods = (AtNodeMethods*) LentilImagerMtd;
    node->output_type = AI_TYPE_NONE;
    node->name = "lentil_imager";
    node->node_type = AI_NODE_DRIVER;
    strcpy(node->version, AI_VERSION);
}