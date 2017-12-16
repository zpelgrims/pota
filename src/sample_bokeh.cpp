#include <ai.h>
#include <math.h>
#include "../include/lens.h"


#define cimg_display 0
#include "../include/CImg.h"
using namespace cimg_library;


// need to pass the lens data to this shader, probably have to do this in a global structure (gael honorez)
// check for intersections along P->Lens path

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


// given camera space scene point, return point on sensor
inline void trace_backwards(const AtVector sample_position, const float aperture_radius, const float lambda, AtVector2 &sensor_position)
{
   const float target[3] = { sample_position.x, sample_position.y, - sample_position.z};

   // initialize 5d light fields
   float sensor[5] = {0.0f};
   float out[5] = {0.0f};
   float aperture[2] = {0.0f};
   sensor[4] = lambda;

   AtVector2 lens;
   concentric_disk_sample(xor128() / 4294967296.0f, xor128() / 4294967296.0f, &lens);
   aperture[0] = lens.x * aperture_radius;
   aperture[1] = lens.y * aperture_radius;

   lens_lt_sample_aperture(target, aperture, sensor, out, lambda);

   sensor_position.x = sensor[0];
   sensor_position.y = sensor[1];
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
   SampleBokehData *data = (SampleBokehData*)AiNodeGetLocalData(node);
   data->aov_name = AiNodeGetStr(node, "aov_name");
   AiAOVRegister(data->aov_name, AI_TYPE_RGBA, AI_AOV_BLEND_OPACITY);

   // camera data should be updated globally, no need to re-assign

   
   float xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
   float yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");
   data->img_out = new CImg<float>(xres, yres, 1, 4, 0);

   
}
 
node_finish
{
   SampleBokehData *data = (SampleBokehData*)AiNodeGetLocalData(node);


   /*
   float xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
   float yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");

   const int samples = 1000;
   
   // normalize samples
   for (int j = 0; j < yres; ++j)
   {
     for (int i = 0; i < xres; ++i)
     { 
         data->img_out->atXY(i,j,0,0) /= samples;
         data->img_out->atXY(i,j,0,1) /= samples;
         data->img_out->atXY(i,j,0,2) /= samples;
         data->img_out->atXY(i,j,0,3) /= samples;
     } 
   }
   */

   // change to exr...
   data->img_out->save("/Users/zeno/pota/tests/image/cimg_bokeh.ppm");
   

   delete data;
}
 
shader_evaluate
{
   const SampleBokehData *data = (SampleBokehData*)AiNodeGetLocalData(node);

   // write AOV only if in use
   // should i be putting everthing inside this loop?
   if ((sg->Rt & AI_RAY_CAMERA) && AiAOVEnabled(data->aov_name, AI_TYPE_RGBA))
   {

      const float aperture_radius = 12.75;
      const float lambda = 0.55f;
      const float sensor_width = 36.0;
      const float xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
      const float yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");
      const float frame_aspect_ratio = xres / yres;

      const int samples = 1000;
      const float minimum_rgb = 5.0f;

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

         sample_energy /= samples;

   
         // convert sample world space position to camera space
         AtMatrix world_to_camera_matrix;
         AtVector2 sensor_position;

         AiWorldToCameraMatrix(AiUniverseGetCamera(), sg->time, world_to_camera_matrix);
         AtVector camera_space_sample_position = AiM4PointByMatrixMult(world_to_camera_matrix, sg->P);

         //AiMsgInfo("[POTA] sample camera space pos: [%f, %f, %f]", camera_space_sample_position.x, camera_space_sample_position.y, camera_space_sample_position.z);
         // do i need to care about the w vector element? As in Marc's shader?

         
         //trace backwards
         for(int count=0; count<samples; count++)
         {
            // is the sensor at z0? I think first lens element is instead. Might have to subtract lens length? not sure
            trace_backwards(camera_space_sample_position, aperture_radius, lambda, sensor_position);

            //AiMsgInfo("sensor_position: [%f, %f]", sensor_position.x, sensor_position.y);

            // do i need to check for pupil intersections or not?

            // what is the pixel position? Convert sensor position to pixel position
            // also not sure about units of sensor_width here
            AtVector2 s(sensor_position.x / (sensor_width * 0.5), sensor_position.y / (sensor_width * 0.5) * frame_aspect_ratio);
            //const int pixel_x = (int)round(((s.x + 1)/2.0) * xres);
            //const int pixel_y = (int)round(((s.y + 1)/2.0) * yres);
            const float pixel_x = ((-s.x + 1.0)/2.0) * xres;
            const float pixel_y = ((s.y + 1.0)/2.0) * yres;

            // something is going wrong with the pixel positions, probably due to sensor_position
            //AiMsgInfo("pixel: [%f, %f]", pixel_x, pixel_y);
            // screen-space coordinates will range between
            // (screen_window_min.x, screen_window_min.y/frame_aspect_ratio) and
            // (screen_window_max.x, screen_window_max.y/frame_aspect_ratio) 

            // write sample to image
            // think cimg rgb values are (0->255) instead of (0->1)
            data->img_out->set_linear_atXY(sample_energy.r * 255, pixel_x, pixel_y, 0, 0, true);
            data->img_out->set_linear_atXY(sample_energy.g * 255, pixel_x, pixel_y, 0, 1, true);
            data->img_out->set_linear_atXY(sample_energy.b * 255, pixel_x, pixel_y, 0, 2, true);
            data->img_out->set_linear_atXY(sample_energy.a * 255, pixel_x, pixel_y, 0, 3, true);

            /*data->img_out->atXY(pixel_x, pixel_y, 0, 0) = sample_energy.r + data->img_out->atXY(pixel_x, pixel_y, 0, 0);
            data->img_out->atXY(pixel_x, pixel_y, 0, 1) = sample_energy.g + data->img_out->atXY(pixel_x, pixel_y, 0, 1);
            data->img_out->atXY(pixel_x, pixel_y, 0, 2) = sample_energy.b + data->img_out->atXY(pixel_x, pixel_y, 0, 2);
            data->img_out->atXY(pixel_x, pixel_y, 0, 3) = sample_energy.a + data->img_out->atXY(pixel_x, pixel_y, 0, 3);
            */
         }

         
      }

      // ideally would be cool to write to an aov but not sure if I can access the different pixels other than
      // the one related to the current sample
      // AiAOVSetRGBA(sg, data->aov_name, sample_energy);
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