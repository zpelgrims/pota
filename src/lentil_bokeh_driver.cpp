// initially try to only do it for rgba, support multiple aovs at later point
// figure out NaNs, probably related to image edge darkening
  // losing energy at the image edges, is this due to vignetting retries?
// doesn't seem to like AA_samples at 1 .. takes long?
// add image based bokeh & chromatic aberrations to backtracing
// strange behaviour when rendering multiple images after each other.. buffer doesn't seem to be cleared
// filter is losing 40% of the energy, look into how these actually work

// when debugging newtonian iterations, if the squared error increases, the error code is 1.. might be ok to take the last iteration in that case?
// what could the circle i'm getting possibly be?

#include <ai.h>
#include <vector>
#include "lentil.h"
#include "lens.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"


// need to sleep the updating of the initialization of this node for a bit
#include <chrono>
#include <thread>

 
AI_DRIVER_NODE_EXPORT_METHODS(LentilBokehDriverMtd);
 
struct LentilBokehDriver {
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

inline float filter_gaussian(AtVector2 p, float width) {
  const float r = AiSqr(2.0 / width) * (AiSqr(p.x) + AiSqr(p.y));
  if (r > 1.0f) return 0.0;
  return AiFastExp(2 * -r);
}
 

node_parameters {}
 
node_initialize
{
  AiNodeSetLocalData(node, new LentilBokehDriver());

  static const char *required_aovs[] = {"RGBA RGBA", "VECTOR P", "FLOAT Z", NULL};
  AiRawDriverInitialize(node, required_aovs, false);
}
 
node_update 
{
  LentilBokehDriver *bokeh = (LentilBokehDriver*)AiNodeGetLocalData(node);
  Camera *po = (Camera*)AiNodeGetLocalData(AiUniverseGetCamera());

  bokeh->enabled = true;

  // don't compute for interactive previews
  bokeh->aa_samples = AiNodeGetInt(AiUniverseGetOptions(), "AA_samples");
  bokeh->min_aa_samples = 3;
  if (bokeh->aa_samples < bokeh->min_aa_samples) {
    bokeh->enabled = false;
    return;
  }

  // this is an UGLY solution, sleep this node initialization for x amount of time
  // this is required to try and make sure the data in shared CameraThinLens is filled in first.
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));


  // disable for non-lentil cameras
  AtNode *node_camera = AiUniverseGetCamera();
  if (!AiNodeIs(node_camera, AtString("lentil"))) {
    AiMsgWarning("[LENTIL BIDIRECTIONAL PO] Camera is not of type lentil");
    bokeh->enabled = false;
    return;
  }
  
  
  bokeh->xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
  bokeh->yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");
  bokeh->filter_width = 2.0;

  AiWorldToCameraMatrix(AiUniverseGetCamera(), AiCameraGetShutterStart(), bokeh->world_to_camera_matrix); // can i use sg->time? do i have access to shaderglobals? setting this to a constant might disable motion blur..

  bokeh->zbuffer.clear();
  bokeh->zbuffer.resize(bokeh->xres * bokeh->yres);
  bokeh->filter_weight_buffer.clear();
  bokeh->filter_weight_buffer.resize(bokeh->xres * bokeh->yres);
  bokeh->sample_per_pixel_counter.clear();
  bokeh->sample_per_pixel_counter.resize(bokeh->xres*bokeh->yres);

  // this is really sketchy, need to watch out for a race condition here :/ Currently avoided by 1000ms sleep
  if (po->bokeh_exr_path.empty()) {
    AiMsgWarning("[LENTIL BIDIRECTIONAL PO] No path specified for bidirectional sampling.");
    AiMsgWarning("[LENTIL BIDIRECTIONAL PO] Path: %s", po->bokeh_exr_path.c_str());
    bokeh->enabled = false;
    return;
  }

  if (po->bokeh_samples_mult == 0) bokeh->enabled = false;

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
				AtRGBA sample = AiAOVSampleIteratorGetRGBA(sample_iterator);
        const float sample_luminance = sample.r*0.21 + sample.g*0.71 + sample.b*0.072;

        // convert sample world space position to camera space
        const AtVector sample_pos_ws = AiAOVSampleIteratorGetAOVVec(sample_iterator, AtString("P"));
        const AtVector camera_space_sample_position_tmp = AiM4PointByMatrixMult(bokeh->world_to_camera_matrix, sample_pos_ws);

        const float depth = AiAOVSampleIteratorGetAOVFlt(sample_iterator, AtString("Z")); // what to do when values are INF?
        const float inv_density = AiAOVSampleIteratorGetInvDensity(sample_iterator);
        if (inv_density <= 0.f) continue; // does this every happen? test

        

      // ENERGY REDISTRIBUTION
        if (sample_luminance > po->minimum_rgb) {

          if (!std::isfinite(depth)) continue; // not sure if this works.. Z AOV has inf values at skydome hits
          if (AiV3IsSmall(sample_pos_ws)) continue; // not sure if this works .. position is 0,0,0 at skydome hits
          
          Eigen::Vector2d sensor_position(0, 0);
          Eigen::Vector3d camera_space_sample_position(camera_space_sample_position_tmp.x, camera_space_sample_position_tmp.y, camera_space_sample_position_tmp.z);
          

          // additional luminance with soft transition
          float fitted_additional_luminance = 0.0;
          if (po->additional_luminance > 0.0){
            if (sample_luminance > po->minimum_rgb && sample_luminance < po->minimum_rgb+po->luminance_remap_transition_width){
              float perc = (sample_luminance - po->minimum_rgb) / po->luminance_remap_transition_width;
              fitted_additional_luminance = po->additional_luminance * perc;          
            } else if (sample_luminance > po->minimum_rgb+po->luminance_remap_transition_width) {
              fitted_additional_luminance = po->additional_luminance;
            } 
          }

        // PROBE RAYS samples to determine size of bokeh & subsequent sample count
          // AtVector2 bbox_min (0, 0);
          // AtVector2 bbox_max (0, 0);
          // for(int count=0; count<64; count++) {
          //   if(!trace_backwards(-camera_space_sample_position * 10.0, camera->aperture_radius, camera->lambda, sensor_position, camera->sensor_shift, camera)) {
          //     --count;
          //     continue;
          //   }

          //   // convert sensor position to pixel position
          //   Eigen::Vector2d s(sensor_position(0) / (camera->sensor_width * 0.5), sensor_position(1) / (camera->sensor_width * 0.5) * frame_aspect_ratio);

          //   const float pixel_x = (( s(0) + 1.0) / 2.0) * xres;
          //   const float pixel_y = ((-s(1) + 1.0) / 2.0) * yres;

          //   //figure out why sometimes pixel is nan, can't just skip it
          //   if ((pixel_x > xres) || 
          //       (pixel_x < 0)    || 
          //       (pixel_y > yres) || 
          //       (pixel_y < 0)    || 
          //       (pixel_x != pixel_x) ||  //nan checking
          //       (pixel_y != pixel_y)) // nan checking
          //   {
          //     --count;
          //     continue;
          //   }

          //   // expand bbox
          //   if (count == 0) {
          //     bbox_min[0] = pixel_x;
          //     bbox_min[1] = pixel_y;
          //     bbox_max[0] = pixel_x;
          //     bbox_max[1] = pixel_y;
          //   } else {
          //     if (pixel_x < bbox_min[0]) bbox_min[0] = pixel_x;
          //     if (pixel_y < bbox_min[1]) bbox_min[1] = pixel_y;
          //     if (pixel_x > bbox_max[0]) bbox_max[0] = pixel_x;
          //     if (pixel_y > bbox_max[1]) bbox_max[1] = pixel_y;
          //   }
          // }

          // double bbox_area = (bbox_max[0] - bbox_min[0]) * (bbox_max[1] - bbox_min[1]);
          // int samples = std::floor(bbox_area / 10.0); // what if area is lower than 10?
          int samples = 1000;
          samples = std::ceil(static_cast<float>(samples) / static_cast<float>(bokeh->aa_samples*bokeh->aa_samples));

          unsigned int total_samples_taken = 0;
          unsigned int max_total_samples = samples*5;

          for(int count=0; count<samples && total_samples_taken < max_total_samples; count++) {
            ++total_samples_taken;

            if(!trace_backwards(-camera_space_sample_position*10.0, po->aperture_radius, po->lambda, sensor_position, po->sensor_shift, po)) {
              --count;
              continue;
            }

            AtRGB weight = AI_RGB_WHITE;

            // convert sensor position to pixel position
            Eigen::Vector2d s(sensor_position(0) / (po->sensor_width * 0.5), 
                              sensor_position(1) / (po->sensor_width * 0.5) * frame_aspect_ratio);

            const float pixel_x = (( s(0) + 1.0) / 2.0) * xres;
            const float pixel_y = ((-s(1) + 1.0) / 2.0) * yres;

            //figure out why sometimes pixel is nan, can't just skip it
            // if ((pixel_x > xres) || 
            //     (pixel_x < 0)    || 
            //     (pixel_y > yres) || 
            //     (pixel_y < 0)    || 
            //     (pixel_x != pixel_x) ||  //nan checking
            //     (pixel_y != pixel_y)) // nan checking
            // {
            //   continue;
            // }

            // if outside of image
            if ((pixel_x >= xres) || 
                (pixel_x < 0)    || 
                (pixel_y >= yres) || 
                (pixel_y < 0))
            {
              --count;
              continue;
            }
            

            // write sample to image
            unsigned pixelnumber = static_cast<int>(bokeh->xres * floor(pixel_y) + floor(pixel_x));
            AtVector2 subpixel_position(pixel_x-std::floor(pixel_x), pixel_y-std::floor(pixel_y));
            float filter_weight = filter_gaussian(subpixel_position - 0.5, bokeh->filter_width);

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

      // // COPY ENERGY IF NO REDISTRIBUTION IS REQUIRED
      //   else { // COPY ENERGY IF NO REDISTRIBUTION IS REQUIRED
      //     int pixelnumber = static_cast<int>(bokeh->xres * py + px);
      //     const AtVector2 &subpixel_position = AiAOVSampleIteratorGetOffset(sample_iterator);
      //     float filter_weight = filter_gaussian(subpixel_position, bokeh->filter_width);

      //     for (int i=0; i<bokeh->aov_list_name.size(); i++){
      //       switch(bokeh->aov_list_type[i]){
      //         case AI_TYPE_RGBA: {
      //           AtRGBA rgba_energy = AiAOVSampleIteratorGetAOVRGBA(sample_iterator, bokeh->aov_list_name[i]) * inv_density;
      //           bokeh->image[bokeh->aov_list_name[i]][pixelnumber] += rgba_energy * filter_weight;
      //           bokeh->filter_weight_buffer[pixelnumber] += filter_weight;
      //           ++bokeh->sample_per_pixel_counter[pixelnumber];

      //           break;
      //         }

      //         case AI_TYPE_RGB: {
      //             AtRGB rgb_energy = AiAOVSampleIteratorGetAOVRGB(sample_iterator, bokeh->aov_list_name[i]) * inv_density;
      //             AtRGBA rgba_energy = AtRGBA(rgb_energy.r, rgb_energy.g, rgb_energy.b, 1.0);
      //             bokeh->image[bokeh->aov_list_name[i]][pixelnumber] += rgba_energy * filter_weight;
      //             bokeh->filter_weight_buffer[pixelnumber] += filter_weight;
      //             ++bokeh->sample_per_pixel_counter[pixelnumber];
                  
      //             break;
      //           }

      //         case AI_TYPE_VECTOR: {
      //           if ((std::abs(depth) < bokeh->zbuffer[pixelnumber]) || bokeh->zbuffer[pixelnumber] == 0.0){
      //             AtVector vec_energy = AiAOVSampleIteratorGetAOVVec(sample_iterator, bokeh->aov_list_name[i]);
      //             AtRGBA rgba_energy = AtRGBA(vec_energy.x, vec_energy.y, vec_energy.z, 1.0);
      //             bokeh->image[bokeh->aov_list_name[i]][pixelnumber] = rgba_energy;
      //             bokeh->zbuffer[pixelnumber] = std::abs(depth);
      //           }

      //           break;
      //         }

      //         case AI_TYPE_FLOAT: {
      //           if ((std::abs(depth) < bokeh->zbuffer[pixelnumber]) || bokeh->zbuffer[pixelnumber] == 0.0){
      //             float flt_energy = AiAOVSampleIteratorGetAOVFlt(sample_iterator, bokeh->aov_list_name[i]);
      //             AtRGBA rgba_energy = AtRGBA(flt_energy, flt_energy, flt_energy, 1.0);
      //             bokeh->image[bokeh->aov_list_name[i]][pixelnumber] = rgba_energy;
      //             bokeh->zbuffer[pixelnumber] = std::abs(depth);
      //           }

      //           break;
      //         }
      //       }
      //     }
      //   }
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

  // dump framebuffer to exr
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
    std::string path = po->bokeh_exr_path.c_str();
    std::string substr = path.substr(0, path.size() - 4);
    std::string bokeh_aov_name = substr + "." + bokeh->aov_list_name[i].c_str() + ".exr";
    SaveEXR(image.data(), bokeh->xres, bokeh->yres, 4, 0, bokeh_aov_name.c_str());
    AiMsgWarning("[LENTIL BIDIRECTIONAL PO] Bokeh AOV written to %s", bokeh_aov_name.c_str());
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
 