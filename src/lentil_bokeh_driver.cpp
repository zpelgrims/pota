// initially try to only do it for rgba, support multiple aovs at later point
// will need to do sample filtering, currently no filtering happens


#include <ai.h>
#include <vector>
#include "lentil.h"
#include "lens.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

 
AI_DRIVER_NODE_EXPORT_METHODS(DriverRAWMtd);
 
struct LentilBokehDriver {
  int xres;
  int yres;
  int samples;
  std::vector<AtRGBA> image;
};
 

node_parameters
{
  AiParameterSTR("filename", "deep.raw");
}
 
node_initialize
{
  //DriverRAWStruct *raw = new DriverRAWStruct();
  AiNodeSetLocalData(node, new DriverRAWStruct());

  static const char *required_aovs[] = {"RGBA RGBA", NULL}; // unsure about this, do i need extra?
  AiRawDriverInitialize(node, required_aovs, false);
}
 
node_update {}
 
driver_supports_pixel_type { return true; } // not needed for raw drivers
 
driver_open
{
  LentilBokehDriver *bokeh = (LentilBokehDriver*)AiNodeGetLocalData(node);
  
  // get general options
  bokeh->xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
  bokeh->yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");

  // this should be determined by more heuristics, such as the size of the bokeh on screen
  // could do 64 initial samples, determine the radius of the bokeh & add more samples accordingly
  bokeh->samples = 512;

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
  
  const double xres = (double)bokeh->xres;
  const double yres = (double)bokeh->yres;
  const double frame_aspect_ratio = xres/yres;

  for (int py = bucket_yo; py < bucket_yo + bucket_size_y; py++) {
		for (int px = bucket_xo; px < bucket_xo + bucket_size_x; px++) {

      AiAOVSampleIteratorInitPixel(sample_iterator, px, py);
      AtRGBA pixel = AI_RGBA_ZERO;
			float total_weight = 0;

			while (AiAOVSampleIteratorGetNext(sample_iterator)) {
				const AtPoint2 position = AiAOVSampleIteratorGetOffset(sample_iterator); // used for pixel filtering, will need to use this to compute sample weight, Raw-drivers only have a radius of 0.5 (one pixel wide).
				const AtRGBA sample = AiAOVSampleIteratorGetRGBA(sample_iterator);
        float sample_luminance = sample.r*0.21 + sample.g*0.71 + sample.b*0.072;

      // TODO: think I will have to filter the final samples. E.g gauss filter

      // ENERGY REDISTRIBUTION
        if (sample_luminance > camera->minimum_rgb) {
          sample.a = 1.0; // TODO: will prob want to use actual sample energy instead, but test with this for now
          sample /= static_cast<double>(bokeh->samples);

          // convert sample world space position to camera space
          AtMatrix world_to_camera_matrix;
          Eigen::Vector2d sensor_position;
          AiWorldToCameraMatrix(AiUniverseGetCamera(), sg->time, world_to_camera_matrix);
          
          // improve this, too much copying
          AtVector camera_space_sample_position_tmp = AiM4PointByMatrixMult(world_to_camera_matrix, sg->P);
          Eigen::Vector3d camera_space_sample_position(camera_space_sample_position_tmp.x, camera_space_sample_position_tmp.y, camera_space_sample_position_tmp.z);
          
          for(int count=0; count<bokeh->samples; count++) {
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
 
driver_write_bucket {}
 
driver_close
{
  // probably want to do the image writing here? guess i could do tiled writing in driver_write_bucket

  LentilBokehDriver *bokeh = (LentilBokehDriver*)AiNodeGetLocalData(node);

  // fill exr
  std::vector<float> image(bokeh->yres * bokeh->xres * 4);
  int offset = -1;
  int pixelnumber = 0;
  int aa_square = bokeh->aa_samples * bokeh->aa_samples;

  for(auto i = 0; i < bokeh->xres * bokeh->yres; i++){
    image[++offset] = bokeh->image[pixelnumber].r / (float)aa_square;
    image[++offset] = bokeh->image[pixelnumber].g / (float)aa_square;
    image[++offset] = bokeh->image[pixelnumber].b / (float)aa_square;
    image[++offset] = bokeh->image[pixelnumber].a / (float)aa_square;
    ++pixelnumber;
  }

  SaveEXR(image.data(), bokeh->xres, bokeh->yres, 4, 0, bokeh->filename);
  AiMsgWarning("[LENTIL] Bokeh AOV written to %s", bokeh->filename);
}
 
node_finish
{
   LentilBokehDriver *bokeh = (LentilBokehDriver*)AiNodeGetLocalData(node);
   delete bokeh;
   AiDriverDestroy(node);
}

node_loader
{
   if (i>0) return FALSE;
   node->methods = (AtNodeMethods*) DriverRAWMtd;
   node->output_type = AI_TYPE_NONE;
   node->name = "driver_raw";
   node->node_type = AI_NODE_DRIVER;
   strcpy(node->version, AI_VERSION);
   return TRUE;
}
 