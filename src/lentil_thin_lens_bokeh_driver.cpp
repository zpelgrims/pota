// chromatic aberrations, split up in Longitudinal/lateral
// strange behaviour when rendering multiple images after each other.. buffer doesn't seem to be cleared?
// compute analytical size of circle of confusion
// if bokeh ends up in the middle, i need to do: ~lens3d~ + (dir*intersection)!
// samples outside of frame are wasted, i can probably abuse the bounding box to guide these samples
// blue noise redistribution

#include <ai.h>
#include <vector>
#include <iostream>
#include <map>
#include "lentil_thinlens.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

 
AI_DRIVER_NODE_EXPORT_METHODS(ThinLensBokehDriverMtd);
 

struct ThinLensBokehDriver {
  int xres;
  int yres;
  int samples;
  int aa_samples;
  int min_aa_samples;
  bool enabled;
  float filter_width;
  AtMatrix world_to_camera_matrix;
  std::map<AtString, std::vector<AtRGBA> > image;
  std::vector<float> zbuffer;
  std::vector<float> filter_weight_buffer;
  std::vector<int> sample_per_pixel_counter;
  std::vector<AtString> aov_list_name;
  std::vector<unsigned int> aov_list_type;
  std::vector<int> aov_types;
};


inline float gaussian(AtVector2 p, float width) {
  const float r = AiSqr(2.0 / width) * (AiSqr(p.x) + AiSqr(p.y));
  if (r > 1.0f) return 0.0;
  return AiFastExp(2 * -r);
}


node_parameters {}
 
node_initialize
{
  AiNodeSetLocalData(node, new ThinLensBokehDriver());

  static const char *required_aovs[] = {"RGBA RGBA", "VECTOR P", "FLOAT Z", NULL};
  AiRawDriverInitialize(node, required_aovs, false);
}
 
node_update 
{
  ThinLensBokehDriver *bokeh = (ThinLensBokehDriver*)AiNodeGetLocalData(node);
  CameraThinLens *tl = (CameraThinLens*)AiNodeGetLocalData(AiUniverseGetCamera());

  bokeh->enabled = true;
  
  // disable for non-lentil cameras
  AtNode *node_camera = AiUniverseGetCamera();
  if (!AiNodeIs(node_camera, AtString("lentil_thinlens"))) {
    AiMsgWarning("[LENTIL BIDIRECTIONAL TL] Camera is not of type lentil_thinlens");
    bokeh->enabled = false;
    return;
  }

  if (tl->bokeh_exr_path.empty()) {
    AiMsgWarning("[LENTIL BIDIRECTIONAL TL] No path specified for bidirectional sampling.");
    AiMsgWarning("[LENTIL BIDIRECTIONAL TL] Path: %s", tl->bokeh_exr_path.c_str());
    bokeh->enabled = false;
    return;
  }

  if (tl->bokeh_samples_mult == 0) bokeh->enabled = false;
  

  bokeh->xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
  bokeh->yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");
  bokeh->filter_width = 2.0;//AiNodeGetFlt(AiUniverseGetOptions(), "filter_width");
  
  // don't compute for interactive previews
  bokeh->aa_samples = AiNodeGetInt(AiUniverseGetOptions(), "AA_samples");
  bokeh->min_aa_samples = 3;
  if (bokeh->aa_samples < bokeh->min_aa_samples) bokeh->enabled = false;

  AiWorldToCameraMatrix(AiUniverseGetCamera(), AiCameraGetShutterStart(), bokeh->world_to_camera_matrix); // can i use sg->time? do i have access to shaderglobals? setting this to a constant might disable motion blur..

  bokeh->zbuffer.clear();
  bokeh->zbuffer.resize(bokeh->xres * bokeh->yres);
  bokeh->filter_weight_buffer.clear();
  bokeh->filter_weight_buffer.resize(bokeh->xres * bokeh->yres);
  bokeh->sample_per_pixel_counter.clear();
  bokeh->sample_per_pixel_counter.resize(bokeh->xres*bokeh->yres);
}
 
driver_supports_pixel_type { return true; } // not needed for raw drivers
 
driver_open {
  ThinLensBokehDriver *bokeh = (ThinLensBokehDriver*)AiNodeGetLocalData(node);

  // get name/type of connected aovs
  const char *name = 0;
  int pixelType = 0;
  bokeh->aov_list_name.clear();
  bokeh->aov_list_type.clear();
  while(AiOutputIteratorGetNext(iterator, &name, &pixelType, 0)){
    bokeh->aov_list_name.push_back(AtString(name));
    bokeh->aov_list_type.push_back(pixelType); 
    bokeh->image[AtString(name)].clear();
    bokeh->image[AtString(name)].resize(bokeh->xres * bokeh->yres);
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
  ThinLensBokehDriver *bokeh = (ThinLensBokehDriver*)AiNodeGetLocalData(node);
  CameraThinLens *tl = (CameraThinLens*)AiNodeGetLocalData(AiUniverseGetCamera());


  if (!bokeh->enabled) return;
  
  const double xres = (double)bokeh->xres;
  const double yres = (double)bokeh->yres;
  const double frame_aspect_ratio = xres/yres;


  for (int py = bucket_yo; py < bucket_yo + bucket_size_y; py++) {
    for (int px = bucket_xo; px < bucket_xo + bucket_size_x; px++) {

      AiAOVSampleIteratorInitPixel(sample_iterator, px, py);
      while (AiAOVSampleIteratorGetNext(sample_iterator)) {
        AtRGBA sample = AiAOVSampleIteratorGetRGBA(sample_iterator);
        const float sample_luminance = sample.r*0.21 + sample.g*0.71 + sample.b*0.072;

        // convert sample world space position to camera space
        const AtVector sample_pos_ws = AiAOVSampleIteratorGetAOVVec(sample_iterator, AtString("P"));
        const AtVector camera_space_sample_position = AiM4PointByMatrixMult(bokeh->world_to_camera_matrix, sample_pos_ws);

        const float depth = AiAOVSampleIteratorGetAOVFlt(sample_iterator, AtString("Z"));
        const float inv_density = AiAOVSampleIteratorGetInvDensity(sample_iterator);
        if (inv_density <= 0.f) continue; // does this every happen? test


      // ENERGY REDISTRIBUTION
        if (sample_luminance > tl->minimum_rgb) {
          
          // additional luminance with soft transition
          float fitted_additional_luminance = 0.0;
          if (tl->additional_luminance > 0.0){
            if (sample_luminance > tl->minimum_rgb && sample_luminance < tl->minimum_rgb+tl->luminance_remap_transition_width){
              float perc = (sample_luminance - tl->minimum_rgb) / tl->luminance_remap_transition_width;
              fitted_additional_luminance = tl->additional_luminance * perc;          
            } else if (sample_luminance > tl->minimum_rgb+tl->luminance_remap_transition_width) {
              fitted_additional_luminance = tl->additional_luminance;
            } 
          }


          

        // PROBE RAYS samples to determine size of bokeh & subsequent sample count
          AtVector2 bbox_min (0, 0);
          AtVector2 bbox_max (0, 0);
          for(int count=0; count<128; count++) {
            Eigen::Vector2d unit_disk(0, 0);
            const float r1 = xor128() / 4294967296.0;
            const float r2 = xor128() / 4294967296.0;
            concentricDiskSample(r1, r2, unit_disk, 0.8, 0.0, 1.0);
            unit_disk *= -1.0;
            
            // tmp copy
            AtVector2 lens(unit_disk(0), unit_disk(1));
            
            // scale points in [-1, 1] domain to actual aperture radius
            lens *= tl->aperture_radius / tl->focus_distance;
            AtVector lens3d(lens.x, lens.y, 0.0);

            // intersect at z=focusdistance
            AtVector dir_tobase = AiV3Normalize(camera_space_sample_position);
            float focusplane_intersection = std::abs(tl->focus_distance/dir_tobase.z);
            AtVector focusplanepoint = dir_tobase * focusplane_intersection;

            // intersect at z=1
            AtVector dir = AiV3Normalize(focusplanepoint - lens3d);
            float intersection = std::abs(1.0 / dir.z);
            AtVector sensor_position = (lens3d + (dir*intersection)) / tl->tan_fov;
            

            // convert sensor position to pixel position
            Eigen::Vector2d s(sensor_position.x / (tl->sensor_width * 0.5), sensor_position.y / (tl->sensor_width * 0.5) * frame_aspect_ratio);
            const float pixel_x = (( s(0) + 1.0) / 2.0) * xres;
            const float pixel_y = ((-s(1) + 1.0) / 2.0) * yres;

            // expand bbox
            if (count == 0) {
              bbox_min = {pixel_x, pixel_y};
              bbox_max = {pixel_x, pixel_y};
            } else {
              if (pixel_x < bbox_min[0]) bbox_min[0] = pixel_x;
              if (pixel_y < bbox_min[1]) bbox_min[1] = pixel_y;
              if (pixel_x > bbox_max[0]) bbox_max[0] = pixel_x;
              if (pixel_y > bbox_max[1]) bbox_max[1] = pixel_y;
            }
          }

          double bbox_area = (bbox_max[0] - bbox_min[0]) * (bbox_max[1] - bbox_min[1]);
          int samples = std::floor(bbox_area * tl->bokeh_samples_mult);
          samples = std::ceil((double)(samples) / (double)(bokeh->aa_samples*bokeh->aa_samples));

          int total_samples_taken = 0;
          int max_total_samples = samples*1.5;
          for(int count=0; count<samples && total_samples_taken < max_total_samples; count++) {
            ++total_samples_taken;

            // either get uniformly distributed points on the unit disk or bokeh image
            Eigen::Vector2d unit_disk(0, 0);

            float r1 = xor128() / 4294967296.0;
            float r2 = xor128() / 4294967296.0;

            if (tl->use_image) {
                tl->image.bokehSample(r1, r2, unit_disk, xor128() / 4294967296.0, xor128() / 4294967296.0);
            } else {
                concentricDiskSample(r1, r2, unit_disk, tl->bias, tl->square, tl->squeeze);
            }

            unit_disk(0) *= tl->squeeze;
            
            // tmp copy
            AtVector2 lens(unit_disk(0), unit_disk(1));

            // intersect at z=focusdistance
            AtVector dir_tobase = AiV3Normalize(camera_space_sample_position);
            float focusplane_intersection = std::abs(tl->focus_distance/dir_tobase.z);
            AtVector focusplanepoint = dir_tobase * focusplane_intersection;
            
            // scale points in [-1, 1] domain to actual aperture radius
            lens *= tl->aperture_radius / tl->focus_distance;
            AtVector lens3d(lens.x, lens.y, 0.0);

            // intersect at z=1
            AtVector dir = AiV3Normalize(focusplanepoint - lens3d);
            float intersection = std::abs(1.0 / dir.z);
            AtVector sensor_position = (lens3d + (dir*intersection)) / tl->tan_fov;
            

            if (tl->optical_vignetting_distance > 0.0){
              if (!empericalOpticalVignettingSquare(lens3d, dir, tl->aperture_radius/tl->focus_distance, tl->optical_vignetting_radius, tl->optical_vignetting_distance/tl->focus_distance, lerp_squircle_mapping(tl->square))){
                  --count;
                  continue;
              }
            }

                
            // this is most likely wrong!
            float coc_aperture = (tl->aperture_radius*unit_disk(0))/tl->focus_distance;
            AtVector coc_focuspoint = (lens3d + (dir*intersection));
            float coc = std::abs(coc_aperture * (tl->focal_length * (tl->focus_distance - coc_focuspoint.z)) / (tl->focus_distance * (coc_focuspoint.z - tl->focal_length)));
            //float coc = 0.05; // need to implement this!
            // CoC = abs(aperture * (focallength * (objectdistance - planeinfocus)) /
            //   (objectdistance * (planeinfocus - focallength)))
            AtRGB weight = AI_RGB_WHITE;
            if (tl->emperical_ca_dist > 0.0){
                const AtVector2 p2(lens3d.x, lens3d.y);
                const float distance_to_center = AiV2Dist(AtVector2(0.0, 0.0), AtVector2((lens3d + (dir*intersection)).x, (lens3d + (dir*intersection)).y));
                const int random_aperture = static_cast<int>(std::floor((xor128() / 4294967296.0) * 3.0));
                AtVector2 aperture_0_center(0.0, 0.0);
                AtVector2 aperture_1_center(- p2 * coc * distance_to_center * tl->emperical_ca_dist);
                AtVector2 aperture_2_center(p2 * coc * distance_to_center * tl->emperical_ca_dist);
                

                if (random_aperture == 1)      lens += aperture_1_center;
                else if (random_aperture == 2) lens += aperture_2_center;

                if (std::pow(lens.x-aperture_1_center.x, 2) + std::pow(lens.y - aperture_1_center.y, 2) > std::pow(tl->aperture_radius/tl->focus_distance, 2)) {
                    weight.r = 0.0;
                }
                if (std::pow(lens.x-aperture_0_center.x, 2) + std::pow(lens.y - aperture_0_center.y, 2) > std::pow(tl->aperture_radius/tl->focus_distance, 2)) {
                    weight.b = 0.0;
                }
                if (std::pow(lens.x-aperture_2_center.x, 2) + std::pow(lens.y - aperture_2_center.y, 2) > std::pow(tl->aperture_radius/tl->focus_distance, 2)) {
                    weight.g = 0.0;
                }

                if (weight == AI_RGB_ZERO){
                    --count;
                    continue;
                }
            
            //     //ca, not sure if this should be done, evens out the intensity?
            //     // float sum = (output.weight.r + output.weight.g + output.weight.b) / 3.0;
            //     // output.weight.r /= sum;
            //     // output.weight.g /= sum;
            //     // output.weight.b /= sum;
            }

            // convert sensor position to pixel position
            Eigen::Vector2d s(sensor_position[0], 
                              sensor_position[1] * frame_aspect_ratio);

            const float pixel_x = (( s(0) + 1.0) / 2.0) * xres;
            const float pixel_y = ((-s(1) + 1.0) / 2.0) * yres;

            // if outside of image
            if ((pixel_x >= xres) || 
                (pixel_x < 0)     || 
                (pixel_y >= yres) || 
                (pixel_y < 0))
            {
              --count; // really need something better here, many samples are wasted outside of frame
              continue;
            }

            // write sample to image
            unsigned pixelnumber = static_cast<int>(bokeh->xres * floor(pixel_y) + floor(pixel_x));
            AtVector2 subpixel_position(pixel_x-std::floor(pixel_x), pixel_y-std::floor(pixel_y));
            float filter_weight = gaussian(subpixel_position - 0.5, bokeh->filter_width);

            for (unsigned i=0; i<bokeh->aov_list_name.size(); i++){

              switch(bokeh->aov_list_type[i]){

                case AI_TYPE_RGBA: {
                  AtRGBA rgba_energy = ((AiAOVSampleIteratorGetAOVRGBA(sample_iterator, bokeh->aov_list_name[i])*inv_density)+fitted_additional_luminance) / (double)(samples);
                  rgba_energy = rgba_energy * weight * filter_weight;
                  bokeh->image[bokeh->aov_list_name[i]][pixelnumber] += rgba_energy;
                  bokeh->filter_weight_buffer[pixelnumber] += filter_weight;
                  ++bokeh->sample_per_pixel_counter[pixelnumber];

                  break;
                }

                case AI_TYPE_RGB: {
                  AtRGB rgb_energy = ((AiAOVSampleIteratorGetAOVRGB(sample_iterator, bokeh->aov_list_name[i])*inv_density)+fitted_additional_luminance) / (double)(samples);
                  AtRGBA rgba_energy = AtRGBA(rgb_energy.r, rgb_energy.g, rgb_energy.b, 1.0) * weight * filter_weight;
                  bokeh->image[bokeh->aov_list_name[i]][pixelnumber] += rgba_energy;
                  bokeh->filter_weight_buffer[pixelnumber] += filter_weight;
                  ++bokeh->sample_per_pixel_counter[pixelnumber];
                  
                  break;
                }

                case AI_TYPE_VECTOR: {
                  if ((std::abs(depth) < bokeh->zbuffer[pixelnumber]) || bokeh->zbuffer[pixelnumber] == 0.0){
                    AtVector vec_energy = AiAOVSampleIteratorGetAOVVec(sample_iterator, bokeh->aov_list_name[i]);
                    AtRGBA rgba_energy = AtRGBA(vec_energy.x, vec_energy.y, vec_energy.z, 1.0);
                    bokeh->image[bokeh->aov_list_name[i]][pixelnumber] = rgba_energy;
                    bokeh->zbuffer[pixelnumber] = std::abs(depth);
                  }

                  break;
                }

                case AI_TYPE_FLOAT: {
                  if ((std::abs(depth) < bokeh->zbuffer[pixelnumber]) || bokeh->zbuffer[pixelnumber] == 0.0){
                    float flt_energy = AiAOVSampleIteratorGetAOVFlt(sample_iterator, bokeh->aov_list_name[i]);
                    AtRGBA rgba_energy = AtRGBA(flt_energy, flt_energy, flt_energy, 1.0);
                    bokeh->image[bokeh->aov_list_name[i]][pixelnumber] = rgba_energy;
                    bokeh->zbuffer[pixelnumber] = std::abs(depth);
                  }

                  break;
                }
              }
            }
          }
        }

        else { // COPY ENERGY IF NO REDISTRIBUTION IS REQUIRED
          int pixelnumber = static_cast<int>(bokeh->xres * py + px);
          const AtVector2 &subpixel_position = AiAOVSampleIteratorGetOffset(sample_iterator);
          float filter_weight = gaussian(subpixel_position, bokeh->filter_width);

          for (int i=0; i<bokeh->aov_list_name.size(); i++){
            switch(bokeh->aov_list_type[i]){
              case AI_TYPE_RGBA: {
                AtRGBA rgba_energy = AiAOVSampleIteratorGetAOVRGBA(sample_iterator, bokeh->aov_list_name[i]) * inv_density;
                bokeh->image[bokeh->aov_list_name[i]][pixelnumber] += rgba_energy * filter_weight;
                bokeh->filter_weight_buffer[pixelnumber] += filter_weight;
                ++bokeh->sample_per_pixel_counter[pixelnumber];

                break;
              }

              case AI_TYPE_RGB: {
                  AtRGB rgb_energy = AiAOVSampleIteratorGetAOVRGB(sample_iterator, bokeh->aov_list_name[i]) * inv_density;
                  AtRGBA rgba_energy = AtRGBA(rgb_energy.r, rgb_energy.g, rgb_energy.b, 1.0);
                  bokeh->image[bokeh->aov_list_name[i]][pixelnumber] += rgba_energy * filter_weight;
                  bokeh->filter_weight_buffer[pixelnumber] += filter_weight;
                  ++bokeh->sample_per_pixel_counter[pixelnumber];
                  
                  break;
                }

              case AI_TYPE_VECTOR: {
                if ((std::abs(depth) < bokeh->zbuffer[pixelnumber]) || bokeh->zbuffer[pixelnumber] == 0.0){
                  AtVector vec_energy = AiAOVSampleIteratorGetAOVVec(sample_iterator, bokeh->aov_list_name[i]);
                  AtRGBA rgba_energy = AtRGBA(vec_energy.x, vec_energy.y, vec_energy.z, 1.0);
                  bokeh->image[bokeh->aov_list_name[i]][pixelnumber] = rgba_energy;
                  bokeh->zbuffer[pixelnumber] = std::abs(depth);
                }

                break;
              }

              case AI_TYPE_FLOAT: {
                if ((std::abs(depth) < bokeh->zbuffer[pixelnumber]) || bokeh->zbuffer[pixelnumber] == 0.0){
                  float flt_energy = AiAOVSampleIteratorGetAOVFlt(sample_iterator, bokeh->aov_list_name[i]);
                  AtRGBA rgba_energy = AtRGBA(flt_energy, flt_energy, flt_energy, 1.0);
                  bokeh->image[bokeh->aov_list_name[i]][pixelnumber] = rgba_energy;
                  bokeh->zbuffer[pixelnumber] = std::abs(depth);
                }

                break;
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
  ThinLensBokehDriver *bokeh = (ThinLensBokehDriver*)AiNodeGetLocalData(node);
  CameraThinLens *tl = (CameraThinLens*)AiNodeGetLocalData(AiUniverseGetCamera());  

  if (!bokeh->enabled) return;

  // dump framebuffers to exrs
  for (unsigned i=0; i<bokeh->aov_list_name.size(); i++){
    
    std::vector<float> image(bokeh->yres * bokeh->xres * 4);
    int offset = -1;

    for(unsigned pixelnumber = 0; pixelnumber < bokeh->xres * bokeh->yres; pixelnumber++){
      
      // only rgba/rgb aovs have been guassian filtered, so need to normalize only them
      if (bokeh->aov_list_type[i] == AI_TYPE_RGBA || bokeh->aov_list_type[i] == AI_TYPE_RGB){

        float filter_weight_accum = (bokeh->filter_weight_buffer[pixelnumber] != 0.0) ? bokeh->filter_weight_buffer[pixelnumber] : 1.0;
        unsigned int samples_per_pixel = (bokeh->sample_per_pixel_counter[pixelnumber] != 0) ? bokeh->sample_per_pixel_counter[pixelnumber] : 1;
        
        image[++offset] = bokeh->image[bokeh->aov_list_name[i]][pixelnumber].r / (filter_weight_accum/samples_per_pixel);
        image[++offset] = bokeh->image[bokeh->aov_list_name[i]][pixelnumber].g / (filter_weight_accum/samples_per_pixel);
        image[++offset] = bokeh->image[bokeh->aov_list_name[i]][pixelnumber].b / (filter_weight_accum/samples_per_pixel);
        image[++offset] = bokeh->image[bokeh->aov_list_name[i]][pixelnumber].a / (filter_weight_accum/samples_per_pixel);
      } else {
        image[++offset] = bokeh->image[bokeh->aov_list_name[i]][pixelnumber].r;
        image[++offset] = bokeh->image[bokeh->aov_list_name[i]][pixelnumber].g;
        image[++offset] = bokeh->image[bokeh->aov_list_name[i]][pixelnumber].b;
        image[++offset] = bokeh->image[bokeh->aov_list_name[i]][pixelnumber].a;
      }
      
    }


    // need to check if it ends with .exr or not
    std::string path = tl->bokeh_exr_path.c_str();
    std::string substr = path.substr(0, path.size() - 4);
    std::string bokeh_aov_name = substr + "." + bokeh->aov_list_name[i].c_str() + ".exr";
    SaveEXR(image.data(), bokeh->xres, bokeh->yres, 4, 0, bokeh_aov_name.c_str());
    AiMsgWarning("[LENTIL BIDIRECTIONAL TL] Bokeh AOV written to %s", bokeh_aov_name.c_str());
  }
}
 
node_finish
{
  ThinLensBokehDriver *bokeh = (ThinLensBokehDriver*)AiNodeGetLocalData(node);
  delete bokeh;
}

node_loader
{
  if (i>0) return false;
  node->methods = (AtNodeMethods*) ThinLensBokehDriverMtd;
  node->output_type = AI_TYPE_NONE;
  node->name = "lentil_thin_lens_bokeh_driver";
  node->node_type = AI_NODE_DRIVER;
  strcpy(node->version, AI_VERSION);
  return true;
}
 