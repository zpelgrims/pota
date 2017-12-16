#include <ai.h>
#include "../include/lens.h"
#include "pota.h"


#define cimg_display 0
#include "../include/CImg.h"
using namespace cimg_library;


// need to pass the lens data to this shader, probably have to do this in a global structure (gael honorez)
// check for intersections along P->Lens path
// are my units correct? everything is in mm but sg->P etc is probably in cm

AI_SHADER_NODE_EXPORT_METHODS(SampleBokehMtd);
 
struct SampleBokehData
{
   AtString aov_name;
   CImg<float> * img_out;

};
 
enum SampleBokehParams
{
   p_aov_name,
};




// xorshift fast random number generator
inline uint32_t xor128(void){
    static uint32_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
    uint32_t t = x ^ (x << 11);
    x = y; y = z; z = w;
    return w = (w ^ (w >> 19) ^ t ^ (t >> 8));
}


// Improved concentric mapping code by Dave Cline [peter shirley´s blog]
// maps points on the unit square onto the unit disk uniformly
inline void concentric_disk_sample(float ox, float oy, AtVector2 *lens)
{
    float phi, r;

    // switch coordinate space from [0, 1] to [-1, 1]
    float a = 2.0 * ox - 1.0;
    float b = 2.0 * oy - 1.0;

    if ((a * a) > (b * b)){
        r = a;
        phi = (0.78539816339f) * (b / a);
    }
    else {
        r = b;
        phi = (AI_PIOVER2)-(0.78539816339f) * (a / b);
    }

    lens->x = r * std::cos(phi);
    lens->y = r * std::sin(phi);
}




// this probably needs to know about the sensor shift!
// given camera space scene point, return point on sensor
inline bool trace_backwards(const AtVector sample_position, const float aperture_radius, const float lambda, AtVector2 &sensor_position)
{
   const float target[3] = { sample_position.x, sample_position.y, - sample_position.z};

   // initialize 5d light fields
   float sensor[5] = {0.0f};
   float out[5] = {0.0f};
   float aperture[2] = {0.0f};
   sensor[4] = lambda;

   // remove temp
   float sensor_shift = 1.398059;

   AtVector2 lens;
   concentric_disk_sample(xor128() / 4294967296.0f, xor128() / 4294967296.0f, &lens);
   aperture[0] = lens.x * aperture_radius;
   aperture[1] = lens.y * aperture_radius;

   lens_lt_sample_aperture(target, aperture, sensor, out, lambda);

   // need to find point on shifted sensor for focusing
   // can probably do some manual ray/plane intersection.. but i only have 2 axis pos/dir information

   sensor[0] += sensor[2] * sensor_shift;
   sensor[1] += sensor[3] * sensor_shift;

   // crop out by outgoing pupil
   if( out[0]*out[0] + out[1]*out[1] > lens_outer_pupil_radius*lens_outer_pupil_radius) return false;
   
   // crop at inward facing pupil
   const float px = sensor[0] + sensor[2] * lens_focal_length;
   const float py = sensor[1] + sensor[3] * lens_focal_length; //(note that lens_focal_length is the back focal length, i.e. the distance unshifted sensor -> pupil)
   if (px*px + py*py > lens_inner_pupil_radius*lens_inner_pupil_radius) return false;

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
   // register AOV
   SampleBokehData *bokeh_data = (SampleBokehData*)AiNodeGetLocalData(node);
   bokeh_data->aov_name = AiNodeGetStr(node, "aov_name");
   AiAOVRegister(bokeh_data->aov_name, AI_TYPE_RGBA, AI_AOV_BLEND_OPACITY);

   // camera data should be updated globally, no need to re-assign

   
   float xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
   float yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");
   bokeh_data->img_out = new CImg<float>(xres, yres, 1, 4, 0);

   
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
   const SampleBokehData *bokeh_data = (SampleBokehData*)AiNodeGetLocalData(node);

   // write AOV only if in use
   if ((sg->Rt & AI_RAY_CAMERA) && AiAOVEnabled(bokeh_data->aov_name, AI_TYPE_RGBA))
   {

      const float aperture_radius = 12.75f;
      const float lambda = 0.55f;
      const float sensor_width = 36.0f;
      const float xres = (float)AiNodeGetInt(AiUniverseGetOptions(), "xres");
      const float yres = (float)AiNodeGetInt(AiUniverseGetOptions(), "yres");
      const float frame_aspect_ratio = xres / yres;

      const int samples = 300;
      const float minimum_rgb = 2.0f;

      AtRGBA sample_energy(0.0, 0.0, 0.0, 0.0);

      // figure out better way, based on:
      // distance from focus point
      // intensity of sample
      if ((sg->out.RGBA().r > minimum_rgb) || (sg->out.RGBA().g > minimum_rgb) || (sg->out.RGBA().b > minimum_rgb))
      {
         
         sample_energy.r = sg->out.RGBA().r;
         sample_energy.g = sg->out.RGBA().g;
         sample_energy.b = sg->out.RGBA().b;
         sample_energy.a = 1.0f;

         sample_energy /= (float)samples;

   
         // convert sample world space position to camera space
         AtMatrix world_to_camera_matrix;
         AtVector2 sensor_position;

         AiWorldToCameraMatrix(AiUniverseGetCamera(), sg->time, world_to_camera_matrix);
         AtVector camera_space_sample_position = AiM4PointByMatrixMult(world_to_camera_matrix, sg->P);
         // do i need to care about the w vector element? As in Marc's shader?


         for(int count=0; count<samples; count++)
         {
            // is the sensor at z0? I think first lens element is instead. Might have to subtract lens length? not sure
            // probably have to *10.0 for cm to mm conversion
            if(!trace_backwards(camera_space_sample_position * 10.0, aperture_radius, lambda, sensor_position)){
               continue;
            }

            // do i need to check for pupil intersections or not?

            // convert sensor position to pixel position
            AtVector2 s(sensor_position.x / (sensor_width * 0.5), 
                        sensor_position.y / (sensor_width * 0.5) * frame_aspect_ratio);

            const float pixel_x = ((-s.x + 1.0) / 2.0) * xres;
            const float pixel_y = ((s.y + 1.0) / 2.0) * yres;

            //figure out why sometimes pixel is nan, can't just skip it
            if ((pixel_x > xres) || (pixel_y > yres) || (pixel_x != pixel_x) || (pixel_y != pixel_y)) continue;

            // write sample to image
            // think cimg rgb values are (0->255) instead of (0->1)
            //AiMsgInfo("pixel: %f, %f", pixel_x, pixel_y);

            bokeh_data->img_out->set_linear_atXY(sample_energy.r * 255, pixel_x, pixel_y, 0, 0, true);
            bokeh_data->img_out->set_linear_atXY(sample_energy.g * 255, pixel_x, pixel_y, 0, 1, true);
            bokeh_data->img_out->set_linear_atXY(sample_energy.b * 255, pixel_x, pixel_y, 0, 2, true);
            bokeh_data->img_out->set_linear_atXY(sample_energy.a * 255, pixel_x, pixel_y, 0, 3, true);
         }

         
      }

      // ideally would be cool to write to an aov but not sure if I can access the different pixels other than
      // the one related to the current sample
      // AiAOVSetRGBA(sg, bokeh_data->aov_name, sample_energy);
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