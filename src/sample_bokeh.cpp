#include <ai.h>
#include "../include/lens.h"
#include "pota.h"


#define cimg_display 0
#include "../include/CImg.h"
using namespace cimg_library;


// need to pass the lens data to this shader, probably have to do this in a global structure (gael honorez)
// check for intersections along P->Lens path
// summing of samples is probably wrong
// write to exr, oiio, tinyexr?
// how to get rid of under-sampling fireflies? need to average with already-existing pixel values for the pixel? maybe they arent fireflies?


AI_SHADER_NODE_EXPORT_METHODS(SampleBokehMtd);
 
struct SampleBokehData
{
   AtString aov_name;
   CImg<float> * img_out;
   int aa_samples;
   int xres;
   int yres;
   int samples;
   float minimum_rgb;
};
 
enum SampleBokehParams
{
   p_aov_name,
};


// LOTS OF NaNs near the edges, why does this happen? Key to everything!

// given camera space scene point, return point on sensor
inline bool trace_backwards(const AtVector sample_position, const float aperture_radius, const float lambda, AtVector2 &sensor_position, const float sensor_shift)
{
   const float target[3] = { -sample_position.x, sample_position.y, -sample_position.z};

   // initialize 5d light fields
   float sensor[5] = {0.0f};
   float out[5] = {0.0f};
   float aperture[2] = {0.0f};
   sensor[4] = lambda;

   AtVector2 lens;
   concentric_disk_sample(xor128() / 4294967296.0f, xor128() / 4294967296.0f, &lens, true);
   aperture[0] = lens.x * aperture_radius;
   aperture[1] = lens.y * aperture_radius;


   if(lens_lt_sample_aperture(target, aperture, sensor, out, lambda) <= 0.0f) return false;

   // shift sensor
   // does this need to be before or after aperture blocking?..
   sensor[0] += sensor[2] * -sensor_shift;
   sensor[1] += sensor[3] * -sensor_shift;

   // crop at inward facing pupil, not needed to crop by outgoing because already done in lens_lt_sample_aperture()
   const float px = sensor[0] + sensor[2] * lens_focal_length;
   const float py = sensor[1] + sensor[3] * lens_focal_length; //(note that lens_focal_length is the back focal length, i.e. the distance unshifted sensor -> pupil)
   if (px*px + py*py > lens_inner_pupil_radius*lens_inner_pupil_radius) return false;


   sensor_position.x = sensor[0];
   sensor_position.y = sensor[1];

   /*
   if ((sensor[0] != sensor[0]) || (sensor[1] != sensor[1])){
      AiMsgInfo("sensor: [%f, %f, %f, %f, %f]", sensor[0], sensor[1], sensor[2], sensor[3], sensor[4]);
      AiMsgInfo("out: [%f, %f, %f, %f, %f]", out[0], out[1], out[2], out[3], out[4]);
      AiMsgInfo("aperture: [%f, %f]", aperture[0], aperture[1]);
   }
   */

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

   // register AOV
   bokeh_data->aov_name = AiNodeGetStr(node, "aov_name");
   AiAOVRegister(bokeh_data->aov_name, AI_TYPE_RGBA, AI_AOV_BLEND_OPACITY);
   
   bokeh_data->aa_samples = AiNodeGetInt(AiUniverseGetOptions(), "AA_samples");

   bokeh_data->xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
   bokeh_data->yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");
   bokeh_data->img_out = new CImg<float>(bokeh_data->xres, bokeh_data->yres, 1, 4, 0);

   
}
 
node_finish
{
   SampleBokehData *bokeh_data = (SampleBokehData*)AiNodeGetLocalData(node);

   /*
   float xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
   float yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");

   const int samples = 1000;
   
   // normalize samples
   for (int j = 0; j < yres; ++j)
   {
     for (int i = 0; i < xres; ++i)
     { 
         bokeh_data->img_out->atXY(i,j,0,0) /= samples;
         bokeh_data->img_out->atXY(i,j,0,1) /= samples;
         bokeh_data->img_out->atXY(i,j,0,2) /= samples;
         bokeh_data->img_out->atXY(i,j,0,3) /= samples;
     } 
   }
   */

   // change to exr...
   bokeh_data->img_out->save("/Users/zeno/pota/tests/image/cimg_bokeh.ppm");

   delete bokeh_data;
}
 
shader_evaluate
{
   SampleBokehData *bokeh_data = (SampleBokehData*)AiNodeGetLocalData(node);

   // write AOV only if in use
   if ((sg->Rt & AI_RAY_CAMERA) && AiAOVEnabled(bokeh_data->aov_name, AI_TYPE_RGBA))
   {

      const float aperture_radius = 12.75f;
      const float lambda = 0.55f;
      const float sensor_width = 36.0f;

      const float xres = (float)bokeh_data->xres;
      const float yres = (float)bokeh_data->yres;
      const float frame_aspect_ratio = xres / yres;

      bokeh_data->samples = 200;
      bokeh_data->minimum_rgb = 2.0f;

      AtRGBA sample_energy(0.0, 0.0, 0.0, 0.0);

      // figure out better way, based on:
      // distance from focus point
      // intensity of sample
      if ((sg->out.RGBA().r > bokeh_data-> minimum_rgb) || (sg->out.RGBA().g > bokeh_data-> minimum_rgb) || (sg->out.RGBA().b > bokeh_data-> minimum_rgb))
      {
         
         sample_energy.r = sg->out.RGBA().r;
         sample_energy.g = sg->out.RGBA().g;
         sample_energy.b = sg->out.RGBA().b;
         sample_energy.a = 1.0f;

         sample_energy /= (float)bokeh_data->samples;

   
         // convert sample world space position to camera space
         AtMatrix world_to_camera_matrix;
         AtVector2 sensor_position;

         AiWorldToCameraMatrix(AiUniverseGetCamera(), sg->time, world_to_camera_matrix);
         AtVector camera_space_sample_position = AiM4PointByMatrixMult(world_to_camera_matrix, sg->P);


         for(int count=0; count<bokeh_data->samples; count++)
         {
            // probably have to *10.0 for cm to mm conversion
            if(!trace_backwards(camera_space_sample_position * 10.0, aperture_radius, lambda, sensor_position, 1.398059))
            {
               continue;
            }


            // convert sensor position to pixel position
            AtVector2 s(sensor_position.x / (sensor_width * 0.5), 
                        sensor_position.y / (sensor_width * 0.5) * frame_aspect_ratio);

            const float pixel_x = ((s.x + 1.0) / 2.0) * xres;
            const float pixel_y = ((s.y + 1.0) / 2.0) * yres;


            //figure out why sometimes pixel is nan, can't just skip it
            if ((pixel_x > xres) || (pixel_y > yres) || (pixel_x != pixel_x) || (pixel_y != pixel_y))
            {
               continue;
            }

            /*
            if ((pixel_x != pixel_x) || (pixel_y != pixel_y)){
               AiMsgInfo("sensor_position: \t\t[%f, %f]", sensor_position.x, sensor_position.y);
               AiMsgInfo("s: \t\t[%f, %f]", s.x, s.y);
               AiMsgInfo("pixel: \t\t[%f, %f]", pixel_x, pixel_y);
            }
            */

            // write sample to image
            // think cimg rgb values are (0->255) instead of (0->1)
            bokeh_data->img_out->set_linear_atXY(sample_energy.r * 255.0f * 1.5, pixel_x, pixel_y, 0, 0, true);
            bokeh_data->img_out->set_linear_atXY(sample_energy.g * 255.0f * 1.5, pixel_x, pixel_y, 0, 1, true);
            bokeh_data->img_out->set_linear_atXY(sample_energy.b * 255.0f * 1.5, pixel_x, pixel_y, 0, 2, true);
            bokeh_data->img_out->set_linear_atXY(sample_energy.a * 255.0f * 1.5, pixel_x, pixel_y, 0, 3, true);

         }

         
      }

      // ideally would be cool to write to an aov but not sure if I can access the different pixels other than
      // the one related to the current sample
      AtRGBA aov_value;
      aov_value.r = bokeh_data->img_out->atXY(sg->x, sg->y, 0);
      aov_value.g = bokeh_data->img_out->atXY(sg->x, sg->y, 1);
      aov_value.b = bokeh_data->img_out->atXY(sg->x, sg->y, 2);
      aov_value.a = bokeh_data->img_out->atXY(sg->x, sg->y, 3);

      AiAOVSetRGBA(sg, bokeh_data->aov_name, aov_value / 255.0f);
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