#include <ai.h>
#include <vector>
#include "../include/lens.h"
#include "pota.h"

#define TINYEXR_IMPLEMENTATION
#include "../include/tinyexr.h"

// CRASHED at 00:00:00, pixel (1068, -1) -> sort out, why is this -1? should be 0
// check for intersections along P->Lens path
// come up with better triggering of backtracing, based on sample intensity, distance from focal point, fstop, ..?
// fix nans

AI_SHADER_NODE_EXPORT_METHODS(SampleBokehMtd);
 
struct SampleBokehData
{
   AtString aov_name;
   int aa_samples;
   int xres;
   int yres;
   int samples;
   std::vector<AtRGBA> image;
};
 

enum SampleBokehParams
{
   p_aov_name,
};


// LOTS OF NaNs near the edges, why does this happen? Key to everything!
// given camera space scene point, return point on sensor
inline bool trace_backwards(const AtVector sample_position, const float aperture_radius, const float lambda, AtVector2 &sensor_position, const float sensor_shift)
{
   const float target[3] = {sample_position.x, sample_position.y, sample_position.z};

   // initialize 5d light fields
   float sensor[5] =    {0.0f, 0.0f, 0.0f, 0.0f, lambda};
   float out[5] =       {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
   float aperture[2] =  {0.0f, 0.0f};

   AtVector2 lens;
   concentric_disk_sample(xor128() / 4294967296.0f, xor128() / 4294967296.0f, lens, true);
   aperture[0] = lens.x * aperture_radius;
   aperture[1] = lens.y * aperture_radius;

   if(lens_lt_sample_aperture(target, aperture, sensor, out, lambda) <= 0.0f) return false;

   // crop at inward facing pupil, not needed to crop by outgoing because already done in lens_lt_sample_aperture()
   const float px = sensor[0] + sensor[2] * lens_focal_length;
   const float py = sensor[1] + sensor[3] * lens_focal_length; //(note that lens_focal_length is the back focal length, i.e. the distance unshifted sensor -> pupil)
   if (px*px + py*py > lens_inner_pupil_radius*lens_inner_pupil_radius) return false;

   // shift sensor
   sensor[0] += sensor[2] * -sensor_shift;
   sensor[1] += sensor[3] * -sensor_shift;

   sensor_position.x = sensor[0];
   sensor_position.y = sensor[1];

   return true;
}


 
node_parameters
{
   AiParameterStr("aov_name", "");
   AiMetaDataSetBool(nentry, "aov_name", "linkable", false);
}

 
node_initialize
{
   AiNodeSetLocalData(node, new SampleBokehData());
}

 
node_update
{
   SampleBokehData *bokeh_data = (SampleBokehData*)AiNodeGetLocalData(node);
   const MyCameraData *camera_data = (MyCameraData*)AiNodeGetLocalData(AiUniverseGetCamera());

   // register AOV
   bokeh_data->aov_name = AiNodeGetStr(node, "aov_name");
   AiAOVRegister(bokeh_data->aov_name, AI_TYPE_RGBA, AI_AOV_BLEND_OPACITY);
   
   // get general options
   bokeh_data->aa_samples = AiNodeGetInt(AiUniverseGetOptions(), "AA_samples");
   bokeh_data->xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
   bokeh_data->yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");

   bokeh_data->image.clear();
   bokeh_data->image.reserve(bokeh_data->xres * bokeh_data->yres);

}


node_finish
{
   SampleBokehData *bokeh_data = (SampleBokehData*)AiNodeGetLocalData(node);
   //const MyCameraData *camera_data = (MyCameraData*)AiNodeGetLocalData(AiUniverseGetCamera());

   // fill exr
   std::vector<float> image(bokeh_data->yres * bokeh_data->xres * 4);
   int offset = -1;
   int pixelnumber = 0;
   int aa_square = bokeh_data->aa_samples * bokeh_data->aa_samples;

   for(auto i = 0; i < bokeh_data->xres * bokeh_data->yres; i++){
      image[++offset] = bokeh_data->image[pixelnumber].r / (float)aa_square;
      image[++offset] = bokeh_data->image[pixelnumber].g / (float)aa_square;
      image[++offset] = bokeh_data->image[pixelnumber].b / (float)aa_square;
      image[++offset] = bokeh_data->image[pixelnumber].a / (float)aa_square;
      ++pixelnumber;
   }

   SaveEXR(image.data(), bokeh_data->xres, bokeh_data->yres, 4, 0, "/Users/zeno/pota/tests/image/pota_bokeh_tinyexr.exr");

   delete bokeh_data;
}
 
shader_evaluate
{
   SampleBokehData *bokeh_data = (SampleBokehData*)AiNodeGetLocalData(node);
   const MyCameraData *camera_data = (MyCameraData*)AiNodeGetLocalData(AiUniverseGetCamera());

   // why does this need to be in shader_evaluate to work? returns 0 in _update_?
   bokeh_data->samples = camera_data->backward_samples * (bokeh_data->aa_samples * bokeh_data->aa_samples);


   const float xres = (float)bokeh_data->xres;
   const float yres = (float)bokeh_data->yres;
   const float frame_aspect_ratio = xres/yres;
   AtRGBA sample_energy = AI_RGBA_ZERO;

   // write AOV only if in use
   if ((sg->Rt & AI_RAY_CAMERA) && AiAOVEnabled(bokeh_data->aov_name, AI_TYPE_RGBA))
   {
      // figure out better way, based on:
      // distance from focus point
      // intensity of sample
      if ((sg->out.RGBA().r > camera_data->minimum_rgb) || (sg->out.RGBA().g > camera_data->minimum_rgb) || (sg->out.RGBA().b > camera_data->minimum_rgb))
      {
         sample_energy = sg->out.RGBA();
         sample_energy.a = 1.0f;
         sample_energy /=  static_cast<float>(bokeh_data->samples);

         // convert sample world space position to camera space
         AtMatrix world_to_camera_matrix;
         AtVector2 sensor_position;

         AiWorldToCameraMatrix(AiUniverseGetCamera(), sg->time, world_to_camera_matrix);
         AtVector camera_space_sample_position = AiM4PointByMatrixMult(world_to_camera_matrix, sg->P);

         for(int count=0; count<bokeh_data->samples; count++)
         {
            if(!trace_backwards( -camera_space_sample_position * 10.0, camera_data->aperture_radius, camera_data->lambda, sensor_position, camera_data->sensor_shift))
            {
               continue;
            }

            // convert sensor position to pixel position
            AtVector2 s(sensor_position.x / (camera_data->sensor_width * 0.5), 
                        sensor_position.y / (camera_data->sensor_width * 0.5) * frame_aspect_ratio);

            const float pixel_x = ((s.x + 1.0) / 2.0) * xres;
            const float pixel_y = ((-s.y + 1.0) / 2.0) * yres;

            //figure out why sometimes pixel is nan, can't just skip it
            if ((pixel_x > xres) || 
                (pixel_x < 0)    || 
                (pixel_y > yres) || 
                (pixel_y < 0)    || 
                (pixel_x != pixel_x) || 
                (pixel_y != pixel_y))
            {
               continue;
            }

            // write sample to image
            int pixelnumber = static_cast<int>(bokeh_data->xres * floor(pixel_y) + floor(pixel_x));
            bokeh_data->image[pixelnumber] += sample_energy;
         }
      }

      // ideally would be cool to write to an aov but not sure if I can access the different pixels other than
      // the one related to the current sample
      //AtRGBA aov_value = bokeh_data->image[bokeh_data->xres * (sg->y) + sg->x]; (sg->y+1, sg->x+1?)
      //AiAOVSetRGBA(sg, bokeh_data->aov_name, aov_value);
   }
}
 
node_loader
{
   if (i != 0) return false;
   node->methods     = SampleBokehMtd;
   node->output_type = AI_TYPE_RGBA;
   node->name        = "sample_bokeh";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}