// initially try to only do it for rgba, support multiple aovs at later point
// figure out NaNs, probably related to image edge darkening
  // losing energy at the image edges, is this due to vignetting retries?
// doesn't seem to like AA_samples at 1 .. takes long?
// add image based bokeh & chromatic aberrations to backtracing
// strange behaviour when rendering multiple images after each other.. buffer doesn't seem to be cleared
// filter is losing 40% of the energy, look into how these actually work

#include <ai.h>
#include <vector>
#include "lentil.h"
#include "lens.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

 
AI_DRIVER_NODE_EXPORT_METHODS(LentilBokehDriverMtd);
 
struct LentilBokehDriver {
  int xres;
  int yres;
  int samples;
  int aa_samples;
  bool enabled;
  float filter_width;
  std::vector<AtRGBA> image;
};

float gaussian(AtVector2 p, float width) {
    /* matches Arnold's exactly. */
    /* Sharpness=2 is good for width 2, sigma=1/sqrt(8) for the width=4,sharpness=4 case */
    // const float sigma = 0.5f;
    // const float sharpness = 1.0f / (2.0f * sigma * sigma);

    p /= (width * 0.5f);
    float dist_squared = (p.x * p.x + p.y * p.y);
    if (dist_squared > (1.0f)) {
        return 0.0f;
    }

    // const float normalization_factor = 1;
    // Float weight = normalization_factor * expf(-dist_squared * sharpness);

    float weight = AiFastExp(-dist_squared * 2.0f); // was:

    if (weight > 0.0f) {
        return weight;
    } else {
        return 0.0f;
    }
}
 

node_parameters {}
 
node_initialize
{
  AiNodeSetLocalData(node, new LentilBokehDriver());

  static const char *required_aovs[] = {"RGBA RGBA", "VECTOR P", NULL};
  AiRawDriverInitialize(node, required_aovs, false);
}
 
node_update 
{
  LentilBokehDriver *bokeh = (LentilBokehDriver*)AiNodeGetLocalData(node);

  // disable for non-lentil cameras
  AtNode *node_camera = AiUniverseGetCamera();
  AiNodeIs(node_camera, AtString("lentil")) ? bokeh->enabled = true : bokeh->enabled = false;
  
  // get general options
  bokeh->xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
  bokeh->yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");
  bokeh->filter_width = AiNodeGetFlt(AiUniverseGetOptions(), "filter_width");
  bokeh->aa_samples = AiNodeGetInt(AiUniverseGetOptions(), "AA_samples");

  // construct buffer
  bokeh->image.clear();
  bokeh->image.reserve(bokeh->xres * bokeh->yres);
}
 
driver_supports_pixel_type { return true; } // not needed for raw drivers
 
driver_open {}
 
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
  Camera *camera = (Camera*)AiNodeGetLocalData(AiUniverseGetCamera());

  if (!bokeh->enabled) return;
  
  const double xres = (double)bokeh->xres;
  const double yres = (double)bokeh->yres;
  const double frame_aspect_ratio = xres/yres;

  for (int py = bucket_yo; py < bucket_yo + bucket_size_y; py++) {
		for (int px = bucket_xo; px < bucket_xo + bucket_size_x; px++) {

      AiAOVSampleIteratorInitPixel(sample_iterator, px, py);
			while (AiAOVSampleIteratorGetNext(sample_iterator)) {
				AtRGBA sample = AiAOVSampleIteratorGetRGBA(sample_iterator);
        AtVector sample_pos_ws = AiAOVSampleIteratorGetAOVVec(sample_iterator, AtString("P"));
        float inv_density = AiAOVSampleIteratorGetInvDensity(sample_iterator);
        if (inv_density <= 0.f) continue; // does this every happen? test
        
        
      // GAUSSIAN FILTER
        float filter_width = 1.8;
        const AtVector2& offset = AiAOVSampleIteratorGetOffset(sample_iterator);
        sample *= gaussian(offset, filter_width) * inv_density;


        float sample_luminance = sample.r*0.21 + sample.g*0.71 + sample.b*0.072;


      // ENERGY REDISTRIBUTION
        if (sample_luminance > camera->minimum_rgb) {
          
          // convert sample world space position to camera space
          AtMatrix world_to_camera_matrix;
          Eigen::Vector2d sensor_position;
          AiWorldToCameraMatrix(AiUniverseGetCamera(), AiCameraGetShutterStart(), world_to_camera_matrix); // can i use sg->time? do i have access to shaderglobals? setting this to a constant might disable motion blur..
          
          AtVector camera_space_sample_position_tmp = AiM4PointByMatrixMult(world_to_camera_matrix, sample_pos_ws);
          Eigen::Vector3d camera_space_sample_position(camera_space_sample_position_tmp.x, camera_space_sample_position_tmp.y, camera_space_sample_position_tmp.z);
          


        // PROBE RAYS samples to determine size of bokeh & subsequent sample count
          AtVector2 bbox_min (0, 0);
          AtVector2 bbox_max (0, 0);
          for(int count=0; count<64; count++) {
            if(!trace_backwards(-camera_space_sample_position * 10.0, camera->aperture_radius, camera->lambda, sensor_position, camera->sensor_shift, camera)) {
              continue;
              --count;
            }

            // convert sensor position to pixel position
            Eigen::Vector2d s(sensor_position(0) / (camera->sensor_width * 0.5), sensor_position(1) / (camera->sensor_width * 0.5) * frame_aspect_ratio);

            const float pixel_x = (( s(0) + 1.0) / 2.0) * xres;
            const float pixel_y = ((-s(1) + 1.0) / 2.0) * yres;

            //figure out why sometimes pixel is nan, can't just skip it
            if ((pixel_x > xres) || 
                (pixel_x < 0)    || 
                (pixel_y > yres) || 
                (pixel_y < 0)    || 
                (pixel_x != pixel_x) ||  //nan checking
                (pixel_y != pixel_y)) // nan checking
            {
              continue;
              --count;
            }

            // expand bbox
            if (count == 0) {
              bbox_min[0] = pixel_x;
              bbox_min[1] = pixel_y;
              bbox_max[0] = pixel_x;
              bbox_max[1] = pixel_y;
            } else {
              if (pixel_x < bbox_min[0]) bbox_min[0] = pixel_x;
              if (pixel_y < bbox_min[1]) bbox_min[1] = pixel_y;
              if (pixel_x > bbox_max[0]) bbox_max[0] = pixel_x;
              if (pixel_y > bbox_max[1]) bbox_max[1] = pixel_y;
            }
          }

          double bbox_area = (bbox_max[0] - bbox_min[0]) * (bbox_max[1] - bbox_min[1]);
          int samples = std::floor(bbox_area / 10.0); // what if area is lower than 10?
          samples = std::ceil(static_cast<float>(samples) / static_cast<float>(bokeh->aa_samples*bokeh->aa_samples));
          
          sample /= static_cast<double>(samples);


          for(int count=0; count<samples; count++) {
            if(!trace_backwards(-camera_space_sample_position * 10.0, camera->aperture_radius, camera->lambda, sensor_position, camera->sensor_shift, camera)) {
              continue;
              --count;
            }

            // convert sensor position to pixel position
            Eigen::Vector2d s(sensor_position(0) / (camera->sensor_width * 0.5), sensor_position(1) / (camera->sensor_width * 0.5) * frame_aspect_ratio);

            const float pixel_x = (( s(0) + 1.0) / 2.0) * xres;
            const float pixel_y = ((-s(1) + 1.0) / 2.0) * yres;

            //figure out why sometimes pixel is nan, can't just skip it
            if ((pixel_x > xres) || 
                (pixel_x < 0)    || 
                (pixel_y > yres) || 
                (pixel_y < 0)    || 
                (pixel_x != pixel_x) ||  //nan checking
                (pixel_y != pixel_y)) // nan checking
            {
              continue;
              --count;
            }

            // write sample to image
            int pixelnumber = static_cast<int>(bokeh->xres * floor(pixel_y) + floor(pixel_x));
            bokeh->image[pixelnumber] += sample * 10; // remove 10 after debugging;
            
          }
        }

      // COPY ENERGY IF NO REDISTRIBUTION IS REQUIRED
        else {
          /* tmp removed for debugging, renenable!*/
          int pixelnumber = static_cast<int>(bokeh->xres * py + px);
          //bokeh->image[pixelnumber] += sample;
        }
      }
    }
  }
}
 
driver_write_bucket {}
 
driver_close
{
  LentilBokehDriver *bokeh = (LentilBokehDriver*)AiNodeGetLocalData(node);
  Camera *camera = (Camera*)AiNodeGetLocalData(AiUniverseGetCamera());  

  if (!bokeh->enabled) return;

  // dump framebuffer to exr
  std::vector<float> image(bokeh->yres * bokeh->xres * 4);
  int offset = -1;
  int pixelnumber = 0;

  for(auto i = 0; i < bokeh->xres * bokeh->yres; i++){
    image[++offset] = bokeh->image[pixelnumber].r;
    image[++offset] = bokeh->image[pixelnumber].g;
    image[++offset] = bokeh->image[pixelnumber].b;
    image[++offset] = bokeh->image[pixelnumber].a;
    ++pixelnumber;
  }

  SaveEXR(image.data(), bokeh->xres, bokeh->yres, 4, 0, camera->bokeh_exr_path.c_str());
  AiMsgWarning("[LENTIL] Bokeh AOV written to %s", camera->bokeh_exr_path.c_str());
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
 