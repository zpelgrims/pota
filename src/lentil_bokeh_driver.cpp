// initially try to only do it for rgba, support multiple aovs at later point
// will need to do sample filtering, currently no filtering happens: https://docs.arnoldrenderer.com/display/A5ARP/Filter+Nodes
// figure out NaNs
// use initial 64 samples so they don't get wasted
// losing energy at the image edges, is this due to vignetting retries?
// doesn't seem to like AA_samples at 1 .. takes long?

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
  std::vector<AtRGBA> image;
};
 

node_parameters {}
 
node_initialize
{
  AiNodeSetLocalData(node, new LentilBokehDriver());

  static const char *required_aovs[] = {"RGBA RGBA", "VECTOR P", NULL};
  AiRawDriverInitialize(node, required_aovs, false);
}
 
node_update {}
 
driver_supports_pixel_type { return true; } // not needed for raw drivers
 
driver_open
{
  LentilBokehDriver *bokeh = (LentilBokehDriver*)AiNodeGetLocalData(node);
  Camera *camera = (Camera*)AiNodeGetLocalData(AiUniverseGetCamera());

  // disable for non-lentil cameras
  AtNode *node_camera = AiUniverseGetCamera();
  AiNodeIs(node_camera, AtString("lentil")) ? bokeh->enabled = true : bokeh->enabled = false;
  
  // get general options
  bokeh->xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
  bokeh->yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");
  bokeh->aa_samples = AiNodeGetInt(AiUniverseGetOptions(), "AA_samples");

  // construct buffer
  bokeh->image.clear();
  bokeh->image.reserve(bokeh->xres * bokeh->yres);
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
  Camera *camera = (Camera*)AiNodeGetLocalData(AiUniverseGetCamera());

  if (!bokeh->enabled) return;
  
  const double xres = (double)bokeh->xres;
  const double yres = (double)bokeh->yres;
  const double frame_aspect_ratio = xres/yres;

  for (int py = bucket_yo; py < bucket_yo + bucket_size_y; py++) {
		for (int px = bucket_xo; px < bucket_xo + bucket_size_x; px++) {

      AiAOVSampleIteratorInitPixel(sample_iterator, px, py);
      AtRGBA pixel = AI_RGBA_ZERO;

			while (AiAOVSampleIteratorGetNext(sample_iterator)) {
				// const AtPoint2 position = AiAOVSampleIteratorGetOffset(sample_iterator); // used for pixel filtering, will need to use this to compute sample weight, Raw-drivers only have a radius of 0.5 (one pixel wide).
				AtRGBA sample = AiAOVSampleIteratorGetRGBA(sample_iterator);
        AtVector sample_pos_ws = AiAOVSampleIteratorGetAOVVec(sample_iterator, AtString("P"));
        float inv_density = AiAOVSampleIteratorGetInvDensity(sample_iterator);
        sample.r *= inv_density;
        sample.g *= inv_density;
        sample.b *= inv_density;
        sample.a *= inv_density;
        float sample_luminance = sample.r*0.21 + sample.g*0.71 + sample.b*0.072;
        
      // TODO: think I will have to filter the final samples. E.g gauss filter

      // ENERGY REDISTRIBUTION
        if (sample_luminance > camera->minimum_rgb) {
          
          // convert sample world space position to camera space
          AtMatrix world_to_camera_matrix;
          Eigen::Vector2d sensor_position;
          AiWorldToCameraMatrix(AiUniverseGetCamera(), AiCameraGetShutterStart(), world_to_camera_matrix); // can i use sg->time? do i have access to shaderglobals? setting this to a constant might disable motion blur..
          
          AtVector camera_space_sample_position_tmp = AiM4PointByMatrixMult(world_to_camera_matrix, sample_pos_ws);
          Eigen::Vector3d camera_space_sample_position(camera_space_sample_position_tmp.x, camera_space_sample_position_tmp.y, camera_space_sample_position_tmp.z);
          


        // PROBE RAYS samples to determine size of bokeh, currently just throwing these away, fix that!
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
          // int samples = std::floor((64.0/(20.0*20.0)) * bbox_area); // 5px*5px=25 is the base area for 64 samples, the chances this metric is finetuned are literally 0, so this needs heavy testing which sample counts are okay
          int samples = std::floor(bbox_area / 10.0); // what if area is lower than 10?
          samples = std::ceil(static_cast<float>(samples) / static_cast<float>(bokeh->aa_samples*bokeh->aa_samples));
        
          sample.a = 1.0; // TODO: will prob want to use actual sample energy instead, but test with this for now
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
            bokeh->image[pixelnumber] += sample;
            
          }
        }

      // COPY ENERGY IF NO REDISTRIBUTION IS REQUIRED
        else {
          int pixelnumber = static_cast<int>(bokeh->xres * py + px);
          bokeh->image[pixelnumber] += sample;
        }
      }
    }
  }
}
 
driver_write_bucket {/* guess i could write the image in tiles? don't really see the point though */}
 
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
 