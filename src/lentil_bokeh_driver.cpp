#include <ai.h>
#include <vector>
#include "lentil.h"
#include "lens.h"
#include "global.h"

 
AI_DRIVER_NODE_EXPORT_METHODS(LentilBokehDriverMtd);
 
struct LentilBokehDriver {
  unsigned xres;
  unsigned yres;
  int framenumber;
  int samples;
  int aa_samples;
  int min_aa_samples;
  bool enabled;
  float filter_width;
  float time_start;
  float time_end;
  std::map<AtString, std::vector<AtRGBA> > image;
  std::map<AtString, std::vector<AtRGBA> > image_redist;
  std::map<AtString, std::vector<AtRGBA> > image_unredist;
  std::map<AtString, std::vector<float> > redist_weight_per_pixel;
  std::map<AtString, std::vector<float> > unredist_weight_per_pixel;
  std::vector<float> zbuffer;
  std::vector<AtString> aov_list_name;
  std::vector<unsigned int> aov_list_type;
  std::vector<int> aov_types;

  AtString rgba_string;
};

void redistribute_add_to_buffer(AtRGBA sample, int px, int aov_type, AtString aov_name, 
                                int samples, float inv_density, float fitted_bidir_add_luminance, float depth, 
                                struct AtAOVSampleIterator* sample_iterator, LentilBokehDriver *bokeh) {
    switch(aov_type){

        case AI_TYPE_RGBA: {
            
          // RGBA is the only aov with transmission component in
          AtRGBA rgba_energy;
          if (aov_name == bokeh->rgba_string){
            rgba_energy = ((sample)+fitted_bidir_add_luminance) / (double)(samples);
          } else {
            rgba_energy = ((AiAOVSampleIteratorGetAOVRGBA(sample_iterator, aov_name))+fitted_bidir_add_luminance) / (double)(samples);
          }

          bokeh->image_redist[aov_name][px] += rgba_energy * inv_density;
          bokeh->redist_weight_per_pixel[aov_name][px] += inv_density / double(samples);
        
          break;
        }

        case AI_TYPE_RGB: {
          AtRGB rgb_energy = AiAOVSampleIteratorGetAOVRGB(sample_iterator, aov_name) + fitted_bidir_add_luminance;
          AtRGBA rgba_energy = AtRGBA(rgb_energy.r, rgb_energy.g, rgb_energy.b, 1.0) / (double)(samples);

          bokeh->image_redist[aov_name][px] += rgba_energy * inv_density;
          bokeh->redist_weight_per_pixel[aov_name][px] += inv_density / double(samples);
          
          break;
        }

        case AI_TYPE_VECTOR: {
          if ((std::abs(depth) <= bokeh->zbuffer[px]) || bokeh->zbuffer[px] == 0.0){
            AtVector vec_energy = AiAOVSampleIteratorGetAOVVec(sample_iterator, aov_name);
            AtRGBA rgba_energy = AtRGBA(vec_energy.x, vec_energy.y, vec_energy.z, 1.0);
            bokeh->image[aov_name][px] = rgba_energy;
            bokeh->zbuffer[px] = std::abs(depth);
          }

          break;
        }

        case AI_TYPE_FLOAT: {
          if ((std::abs(depth) <= bokeh->zbuffer[px]) || bokeh->zbuffer[px] == 0.0){
            float flt_energy = AiAOVSampleIteratorGetAOVFlt(sample_iterator, aov_name);
            AtRGBA rgba_energy = AtRGBA(flt_energy, flt_energy, flt_energy, 1.0);
            bokeh->image[aov_name][px] = rgba_energy;
            bokeh->zbuffer[px] = std::abs(depth);
          }

          break;
        }
    }
}


void copy_add_to_buffer(AtRGBA sample, int px, int aov_type, AtString aov_name, float inv_density, float depth, 
                        struct AtAOVSampleIterator* sample_iterator, LentilBokehDriver *bokeh) {
  switch(aov_type){
    case AI_TYPE_RGBA: {
      // RGBA is the only aov with transmission component in
      AtRGBA rgba_energy;
      
      if (aov_name == bokeh->rgba_string){
        rgba_energy = sample;
      } else {
        rgba_energy = AiAOVSampleIteratorGetAOVRGBA(sample_iterator, aov_name);
      }
      
      bokeh->image_unredist[aov_name][px] += rgba_energy * inv_density;
      bokeh->unredist_weight_per_pixel[aov_name][px] += inv_density;


      break;
    }

    case AI_TYPE_RGB: {
        AtRGB rgb_energy = AiAOVSampleIteratorGetAOVRGB(sample_iterator, aov_name);
        AtRGBA rgba_energy = AtRGBA(rgb_energy.r, rgb_energy.g, rgb_energy.b, 1.0);
        bokeh->image_unredist[aov_name][px] += rgba_energy * inv_density;
        bokeh->unredist_weight_per_pixel[aov_name][px] += inv_density;

        break;
      }

    case AI_TYPE_VECTOR: {
      if ((std::abs(depth) <= bokeh->zbuffer[px]) || bokeh->zbuffer[px] == 0.0){
        AtVector vec_energy = AiAOVSampleIteratorGetAOVVec(sample_iterator, aov_name);
        AtRGBA rgba_energy = AtRGBA(vec_energy.x, vec_energy.y, vec_energy.z, 1.0);
        bokeh->image[aov_name][px] = rgba_energy;
        bokeh->zbuffer[px] = std::abs(depth);
      }

      break;
    }

    case AI_TYPE_FLOAT: {
      if ((std::abs(depth) <= bokeh->zbuffer[px]) || bokeh->zbuffer[px] == 0.0){
        float flt_energy = AiAOVSampleIteratorGetAOVFlt(sample_iterator, aov_name);
        AtRGBA rgba_energy = AtRGBA(flt_energy, flt_energy, flt_energy, 1.0);
        bokeh->image[aov_name][px] = rgba_energy;
        bokeh->zbuffer[px] = std::abs(depth);
      }

      break;
    }
  }
}


node_parameters {}
 
node_initialize
{
  AiNodeSetLocalData(node, new LentilBokehDriver());

  static const char *required_aovs[] = {"RGBA RGBA", "VECTOR P", "FLOAT Z", "RGBA transmission", "RGBA lentil_bidir_ignore", NULL};
  AiRawDriverInitialize(node, required_aovs, false);
}
 
node_update 
{
  LentilBokehDriver *bokeh = (LentilBokehDriver*)AiNodeGetLocalData(node);
  Camera *po = (Camera*)AiNodeGetLocalData(AiUniverseGetCamera());


  const AtNodeEntry *nentry = AiNodeGetNodeEntry(node);
  if (AiNodeEntryGetCount(nentry) > 1){
    AiMsgError("[LENTIL BIDIRECTIONAL ERROR]: Multiple nodes of type lentil_bokeh_driver exist. "
               "Use the lentil_operator to avoid this.");
  }



  // get camera params & recompute the node_update section to avoid race condition when sharing datastruct
  // note this currently is DOUBLE code!! find a fix!!
  AtNode *cameranode = AiUniverseGetCamera();
  po->sensor_width = AiNodeGetFlt(cameranode, "sensor_widthPO");
  po->input_fstop = AiNodeGetFlt(cameranode, "fstopPO");
  po->focus_distance = AiNodeGetFlt(cameranode, "focus_distancePO") * 10.0; //converting to mm
  po->lensModel = (LensModel) AiNodeGetInt(cameranode, "lens_modelPO");
  po->bokeh_aperture_blades = AiNodeGetInt(cameranode, "bokeh_aperture_bladesPO");
  po->dof = AiNodeGetBool(cameranode, "dofPO");
  po->vignetting_retries = AiNodeGetInt(cameranode, "vignetting_retriesPO");
  po->bidir_min_luminance = AiNodeGetFlt(cameranode, "bidir_min_luminancePO");
  po->bidir_output_path = AiNodeGetStr(cameranode, "bidir_output_pathPO");
  po->proper_ray_derivatives = AiNodeGetBool(cameranode, "proper_ray_derivativesPO");
  po->bokeh_enable_image = AiNodeGetBool(cameranode, "bokeh_enable_imagePO");
  po->bokeh_image_path = AiNodeGetStr(cameranode, "bokeh_image_pathPO");
  
  po->bidir_sample_mult = AiNodeGetInt(cameranode, "bidir_sample_multPO");
  po->bidir_add_luminance = AiNodeGetFlt(cameranode, "bidir_add_luminancePO");
  po->bidir_add_luminance_transition = AiNodeGetFlt(cameranode, "bidir_add_luminance_transitionPO");

  po->lambda = AiNodeGetFlt(cameranode, "wavelengthPO") * 0.001;
  po->extra_sensor_shift = AiNodeGetFlt(cameranode, "extra_sensor_shiftPO");

  #include "node_update_po.h"



  bokeh->enabled = true;

  // don't compute for interactive previews
  bokeh->aa_samples = AiNodeGetInt(AiUniverseGetOptions(), "AA_samples");
  bokeh->min_aa_samples = 3;
  if (bokeh->aa_samples < bokeh->min_aa_samples) {
    bokeh->enabled = false;
    return;
  }

  // disable for non-lentil cameras
  if (!AiNodeIs(cameranode, AtString("lentil"))) {
    AiMsgWarning("[LENTIL BIDIRECTIONAL PO] Camera is not of type lentil");
    bokeh->enabled = false;
    return;
  }
  
  
  bokeh->xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
  bokeh->yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");
  bokeh->filter_width = 2.0;
  bokeh->rgba_string = AtString("RGBA");
  bokeh->framenumber = static_cast<int>(AiNodeGetFlt(AiUniverseGetOptions(), "frame"));

  bokeh->zbuffer.clear();
  bokeh->zbuffer.resize(bokeh->xres * bokeh->yres);

  bokeh->time_start = AiCameraGetShutterStart();
  bokeh->time_end = AiCameraGetShutterEnd();


  // this is really sketchy, need to watch out for a race condition here :/ Currently avoided by double-computing these params
  if (po->bidir_output_path.empty()) {
    AiMsgWarning("[LENTIL BIDIRECTIONAL PO] No path specified for bidirectional sampling.");
    AiMsgWarning("[LENTIL BIDIRECTIONAL PO] Path: %s", po->bidir_output_path.c_str());
    bokeh->enabled = false;
    return;
  }


  if (po->bidir_sample_mult == 0) bokeh->enabled = false;
  if (bokeh->enabled) AiMsgInfo("[LENTIL BIDIRECTIONAL PO] Starting bidirectional sampling.");
}
 
driver_supports_pixel_type { return true; } // not needed for raw drivers
 
driver_open {
  LentilBokehDriver *bokeh = (LentilBokehDriver*)AiNodeGetLocalData(node);

  // get name/type of connected aovs
  const char *name = 0;
  int pixelType = 0;
  bokeh->aov_list_name.clear();
  bokeh->aov_list_type.clear();
  while(AiOutputIteratorGetNext(iterator, &name, &pixelType, 0)){
    // if aov already considered (can happen due to multiple drivers considering the same aov, such as kick_display and driver_exr)
    if (std::find(bokeh->aov_list_name.begin(), bokeh->aov_list_name.end(), AtString(name)) != bokeh->aov_list_name.end()) continue;
    
    bokeh->aov_list_name.push_back(AtString(name));
    bokeh->aov_list_type.push_back(pixelType); 
    bokeh->image[AtString(name)].clear();
    bokeh->image[AtString(name)].resize(bokeh->xres * bokeh->yres);
    bokeh->image_redist[AtString(name)].clear();
    bokeh->image_redist[AtString(name)].resize(bokeh->xres * bokeh->yres);
    bokeh->image_unredist[AtString(name)].clear();
    bokeh->image_unredist[AtString(name)].resize(bokeh->xres * bokeh->yres);
    bokeh->redist_weight_per_pixel[AtString(name)].clear();
    bokeh->redist_weight_per_pixel[AtString(name)].resize(bokeh->xres * bokeh->yres);
    bokeh->unredist_weight_per_pixel[AtString(name)].clear();
    bokeh->unredist_weight_per_pixel[AtString(name)].resize(bokeh->xres * bokeh->yres);
  }
  AiOutputIteratorReset(iterator);

}
 
driver_extension
{
   static const char *extensions[] = {"raw", NULL};
   return extensions;
}
 
driver_needs_bucket
{
   return true; // API: true if the bucket needs to be rendered, false if it can be skipped
}
 
driver_prepare_bucket {} // called before a bucket is rendered
 

driver_process_bucket
{
  LentilBokehDriver *bokeh = (LentilBokehDriver*)AiNodeGetLocalData(node);
  Camera *po = (Camera*)AiNodeGetLocalData(AiUniverseGetCamera());

  if (!bokeh->enabled) return;
  
  const double xres = (double)bokeh->xres;
  const double yres = (double)bokeh->yres;
  const double frame_aspect_ratio = xres/yres;

  for (int py = bucket_yo; py < bucket_yo + bucket_size_y; py++) {
		for (int px = bucket_xo; px < bucket_xo + bucket_size_x; px++) {

      AiAOVSampleIteratorInitPixel(sample_iterator, px, py);
			while (AiAOVSampleIteratorGetNext(sample_iterator)) {

        bool redistribute = true;
        bool partly_redistributed = false;

				AtRGBA sample = AiAOVSampleIteratorGetRGBA(sample_iterator);
        const AtVector sample_pos_ws = AiAOVSampleIteratorGetAOVVec(sample_iterator, AtString("P"));
        const float depth = AiAOVSampleIteratorGetAOVFlt(sample_iterator, AtString("Z")); // what to do when values are INF?
        const float inv_density = AiAOVSampleIteratorGetInvDensity(sample_iterator);
        if (inv_density <= 0.f) continue; // does this every happen? test
        const float filter_width_half = std::ceil(bokeh->filter_width * 0.5);


        const AtRGBA sample_transmission = AiAOVSampleIteratorGetAOVRGBA(sample_iterator, AtString("transmission"));
        bool transmitted_energy_in_sample = (AiColorMaxRGB(sample_transmission) > 0.0);
        if (transmitted_energy_in_sample){
          sample.r -= sample_transmission.r;
          sample.g -= sample_transmission.g;
          sample.b -= sample_transmission.b;
        }

        const float sample_luminance = sample.r*0.21 + sample.g*0.71 + sample.b*0.072;
        if (sample_luminance < po->bidir_min_luminance) redistribute = false;
        if (!std::isfinite(depth)) redistribute = false; // not sure if this works.. Z AOV has inf values at skydome hits
        if (AiV3IsSmall(sample_pos_ws)) redistribute = false; // not sure if this works .. position is 0,0,0 at skydome hits
        if (AiAOVSampleIteratorHasAOVValue(sample_iterator, AtString("lentil_bidir_ignore"), AI_TYPE_RGBA)) redistribute = false;
        
        
        AtMatrix world_to_camera_matrix_static;
        float time_middle = linear_interpolate(0.5, bokeh->time_start, bokeh->time_end);
        AiWorldToCameraMatrix(AiUniverseGetCamera(), time_middle, world_to_camera_matrix_static);
        const AtVector camera_space_sample_position_static = AiM4PointByMatrixMult(world_to_camera_matrix_static, sample_pos_ws); // just for CoC size calculation
        if (std::abs(camera_space_sample_position_static.z) < (po->lens_length*0.1)) redistribute = false; // sample can't be inside of lens

       

      // ENERGY REDISTRIBUTION
        if (redistribute) {
          
          // additional luminance with soft transition
          float fitted_bidir_add_luminance = 0.0;
          if (po->bidir_add_luminance > 0.0) fitted_bidir_add_luminance = additional_luminance_soft_trans(sample_luminance, po->bidir_add_luminance, po->bidir_add_luminance_transition, po->bidir_min_luminance);

          Eigen::Vector2d sensor_position(0, 0);
          Eigen::Vector3d camera_space_sample_position(camera_space_sample_position_static.x, camera_space_sample_position_static.y, camera_space_sample_position_static.z);
          

        // PROBE RAYS samples to determine size of bokeh & subsequent sample count
          AtVector2 bbox_min (0, 0);
          AtVector2 bbox_max (0, 0);
          int proberays_total_samples = 0;
          int proberays_base_rays = 32;
          int proberays_max_rays = proberays_base_rays * 2;
          for(int count=0; count<proberays_base_rays; count++) {
            ++proberays_total_samples;
            if (proberays_total_samples >= proberays_max_rays) continue;

            if(!trace_backwards(-camera_space_sample_position * 10.0, po->aperture_radius, po->lambda, sensor_position, po->sensor_shift, po)) {
              --count;
              continue;
            }

            // convert sensor position to pixel position
            Eigen::Vector2d s(sensor_position(0) / (po->sensor_width * 0.5), sensor_position(1) / (po->sensor_width * 0.5) * frame_aspect_ratio);

            const float pixel_x = (( s(0) + 1.0) / 2.0) * xres;
            const float pixel_y = ((-s(1) + 1.0) / 2.0) * yres;

            //figure out why sometimes pixel is nan, can't just skip it
            if ((pixel_x > xres) || (pixel_x < 0) || (pixel_y > yres) || (pixel_y < 0) || 
                (pixel_x != pixel_x) || (pixel_y != pixel_y)) // nan checking
            {
              --count;
              continue;
            }

            // expand bbox
            if (count == 0) {
              bbox_min = {pixel_x, pixel_y};
              bbox_max = {pixel_x, pixel_y};
            } else {
              bbox_min = {std::min(bbox_min.x, pixel_x), std::min(bbox_min.y, pixel_y)};
              bbox_max = {std::max(bbox_max.x, pixel_x), std::max(bbox_max.y, pixel_y)};
            }
          }

          double bbox_area = (bbox_max.x - bbox_min.x) * (bbox_max.y - bbox_min.y);
          if (bbox_area < 20.0) goto no_redist; //might have to increase this?
          int samples = std::floor(bbox_area * po->bidir_sample_mult * 0.01);
          samples = std::ceil((double)(samples) / (double)(bokeh->aa_samples*bokeh->aa_samples));
          samples = std::clamp(samples, 75, 1000000); // not sure if a million is actually ever hit.. 75 seems high but is needed to remove stochastic noise


          unsigned int total_samples_taken = 0;
          unsigned int max_total_samples = samples*5;
          

          for(int count=0; count<samples && total_samples_taken < max_total_samples; count++) {
            ++total_samples_taken;

            // world to camera space transform, motion blurred
            AtMatrix world_to_camera_matrix_motionblurred;
            float currenttime = linear_interpolate(xor128() / 4294967296.0, bokeh->time_start, bokeh->time_end); // should I create new random sample, or can I re-use another one?
            AiWorldToCameraMatrix(AiUniverseGetCamera(), currenttime, world_to_camera_matrix_motionblurred);
            const AtVector camera_space_sample_position_mb = AiM4PointByMatrixMult(world_to_camera_matrix_motionblurred, sample_pos_ws);
            Eigen::Vector3d camera_space_sample_position_mb_eigen (camera_space_sample_position_mb.x, camera_space_sample_position_mb.y, camera_space_sample_position_mb.z);
            
            
            if(!trace_backwards(-camera_space_sample_position_mb_eigen*10.0, po->aperture_radius, po->lambda, sensor_position, po->sensor_shift, po)) {
              --count;
              continue;
            }


            // convert sensor position to pixel position
            Eigen::Vector2d s(sensor_position(0) / (po->sensor_width * 0.5), 
                              sensor_position(1) / (po->sensor_width * 0.5) * frame_aspect_ratio);

            const float pixel_x = (( s(0) + 1.0) / 2.0) * xres;
            const float pixel_y = ((-s(1) + 1.0) / 2.0) * yres;


            // if outside of image
            if ((pixel_x >= xres) || (pixel_x < 0) || (pixel_y >= yres) || (pixel_y < 0) ||
                (pixel_x != pixel_x) || (pixel_y != pixel_y)) // nan checking
            {
              --count; // much room for improvement here, potentially many samples are wasted outside of frame
              continue;
            }
            
            // >>>> currently i've decided not to filter the redistributed energy. If needed, there's an old prototype in github issue #230

            // write sample to image
            unsigned pixelnumber = static_cast<int>(bokeh->xres * floor(pixel_y) + floor(pixel_x));

            for (unsigned i=0; i<bokeh->aov_list_name.size(); i++){
              redistribute_add_to_buffer(sample, pixelnumber, bokeh->aov_list_type[i], bokeh->aov_list_name[i], 
                                         samples, inv_density, fitted_bidir_add_luminance, depth, sample_iterator, bokeh);
            }
          }

          if (transmitted_energy_in_sample) {
            partly_redistributed = true;
            goto no_redist;
          }
        }



        else { // COPY ENERGY IF NO REDISTRIBUTION IS REQUIRED
        no_redist:

          if (transmitted_energy_in_sample && partly_redistributed) {
            sample.r = sample_transmission.r;
            sample.g = sample_transmission.g;
            sample.b = sample_transmission.b;
          } else if (transmitted_energy_in_sample && !partly_redistributed) {
            sample.r += sample_transmission.r;
            sample.g += sample_transmission.g;
            sample.b += sample_transmission.b;
          }


          // loop over all pixels in filter radius, then compute the filter weight based on the offset not to the original pixel (px, py), but the filter pixel (x, y)
          for (unsigned y = py - filter_width_half; y <= py + filter_width_half; y++) {
            for (unsigned x = px - filter_width_half; x <= px + filter_width_half; x++) {
              
              if (y < 0 || y >= bokeh->yres) continue; // edge fix
              if (x < 0 || x >= bokeh->xres) continue; // edge fix

              const unsigned pixelnumber = static_cast<int>(bokeh->xres * y + x);
              
              const AtVector2 &subpixel_position = AiAOVSampleIteratorGetOffset(sample_iterator); // offset within original pixel
              AtVector2 subpixel_pos_dist = AtVector2((px+subpixel_position.x) - x, (py+subpixel_position.y) - y);
              float filter_weight = filter_gaussian(subpixel_pos_dist, bokeh->filter_width);
              if (filter_weight == 0) continue;

              for (size_t i=0; i<bokeh->aov_list_name.size(); i++){
                copy_add_to_buffer(sample, pixelnumber, bokeh->aov_list_type[i], bokeh->aov_list_name[i], inv_density, depth, sample_iterator, bokeh);
              }
            }
          }
        }
      }
    }
  }
}
 
driver_write_bucket {}
 
driver_close
{
  LentilBokehDriver *bokeh = (LentilBokehDriver*)AiNodeGetLocalData(node);
  Camera *po = (Camera*)AiNodeGetLocalData(AiUniverseGetCamera());

  if (!bokeh->enabled) return;

   // dump framebuffers to exrs
  for (unsigned i=0; i<bokeh->aov_list_name.size(); i++){

    const AtString aov = bokeh->aov_list_name[i];
    
    if (bokeh->aov_list_name[i] == AtString("transmission")) continue;
    
    std::vector<float> image(bokeh->yres * bokeh->xres * 4);
    int offset = -1;

    for(unsigned px = 0; px < bokeh->xres * bokeh->yres; px++){
      
      // only rgba/rgb aovs have been guassian filtered, so need to normalize only them
      if (bokeh->aov_list_type[i] == AI_TYPE_RGBA || bokeh->aov_list_type[i] == AI_TYPE_RGB){

        AtRGBA redist = bokeh->image_redist[aov][px] / ((bokeh->redist_weight_per_pixel[aov][px] == 0.0) ? 1.0 : bokeh->redist_weight_per_pixel[aov][px]);
        AtRGBA unredist = bokeh->image_unredist[aov][px] / ((bokeh->unredist_weight_per_pixel[aov][px] == 0.0) ? 1.0 : bokeh->unredist_weight_per_pixel[aov][px]);
        AtRGBA combined_redist_unredist = (unredist * (1.0-bokeh->redist_weight_per_pixel[aov][px])) + (redist * (bokeh->redist_weight_per_pixel[aov][px]));
        
        // this currently doesn't work for the rgb layers because alpha is wrong for rgb layers
        if (combined_redist_unredist.a > 0.95) combined_redist_unredist /= combined_redist_unredist.a;

        image[++offset] = combined_redist_unredist.r;
        image[++offset] = combined_redist_unredist.g;
        image[++offset] = combined_redist_unredist.b;
        image[++offset] = combined_redist_unredist.a;
      } else {
        image[++offset] = bokeh->image[aov][px].r;
        image[++offset] = bokeh->image[aov][px].g;
        image[++offset] = bokeh->image[aov][px].b;
        image[++offset] = bokeh->image[aov][px].a;
      }
    }

    // replace <aov> and <frame>
    std::string path = po->bidir_output_path.c_str();
    std::string path_replaced_aov = replace_first_occurence(path, "<aov>", aov.c_str());
    
    std::string frame_str = std::to_string(bokeh->framenumber);
    std::string frame_padded = std::string(4 - frame_str.length(), '0') + frame_str;
    std::string path_replaced_framenumber = replace_first_occurence(path, "<frame>", frame_padded);

    save_to_exr_rgba(image, path_replaced_framenumber, bokeh->xres, bokeh->yres);
    AiMsgWarning("[LENTIL BIDIRECTIONAL PO] %s AOV written to %s", aov.c_str(), path_replaced_framenumber.c_str());
  }
}
 
node_finish
{
   LentilBokehDriver *bokeh = (LentilBokehDriver*)AiNodeGetLocalData(node);
   delete bokeh;
}

node_loader
{
   if (i>0) return false;
   node->methods = (AtNodeMethods*) LentilBokehDriverMtd;
   node->output_type = AI_TYPE_NONE;
   node->name = "lentil_bokeh_driver";
   node->node_type = AI_NODE_DRIVER;
   strcpy(node->version, AI_VERSION);
   return true;
}
 