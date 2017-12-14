#include <ai.h>
#include "../include/lens.h"

// need to pass the lens data to this shader

AI_SHADER_NODE_EXPORT_METHODS(SampleBokehMtd);
 
struct SampleBokehData
{
   AtString aov_name;
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




// watch out, scene point needs to be in camera space, not world space!

// given camera space scene point, return point on sensor
void trace_backwards(const AtVector sample_position, const float aperture_radius, const float lambda)
{
   const float target[3] = { sample_position.x, sample_position.y, sample_position.z};

   // initialize 5d light fields
   float sensor[5] = {0.0f};
   float out[5] = {0.0f};
   sensor[4] = lambda;
   int count = 0;

   // just point through center of aperture
   float aperture[2] = {0.0f, 0.0f};
   AtVector2 lens;
      
   concentricDiskSample(xor128() / 4294967296.0f, xor128() / 4294967296.0f, &lens);

   // reset aperture
   aperture[0] = lens.x * aperture_radius;
   aperture[1] = lens.y * aperture_radius;

   lens_lt_sample_aperture(target, aperture, sensor, out, lambda);

   /*
   if(sensor[2+k] > 0){
      offset = sensor[k]/sensor[2+k];
   }
   */
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
}
 
node_finish
{
   SampleBokehData *data = (SampleBokehData*)AiNodeGetLocalData(node);
   delete data;
}
 
shader_evaluate
{
   const SampleBokehData *data = (SampleBokehData*)AiNodeGetLocalData(node);

   // figure out better way, based on:
   // distance from focus point
   // intensity of sample
   if (sg->out.RGBA().r > 4.0f || sg->out.RGBA().g > 4.0f || sg->out.RGBA().b > 4.0f)
   {

      // convert sample world space position to camera space
      AtMatrix camera_space;
      AiWorldToCameraMatrix(&camera_node, AiCameraGetShutterStart(), camera_space); //how do i get the camera node?

      //trace backwards
      for(count=0; count<samples; count++)
      {
         AtRGBA sample_energy = sg->out.RGBA / samples;


         trace_backwards
      }

      // print sample positions as test
      AiMsgInfo("[POTA] flagged sample position: [%f, %f, %f]", sg->P.x, sg->P.y, sg->P.z);
   }
 




   // write AOV only if in use
   if ((sg->Rt & AI_RAY_CAMERA) && AiAOVEnabled(data->aov_name, AI_TYPE_RGBA))
   {
      AiAOVSetRGBA(sg, data->aov_name, sample_r_intensity);
   }
   
   // probably have to write to separate image buffer instead, not sure if i can write to different pixels..
   //sg->out.RGBA() = sample_r_intensity;
}
 
node_loader
{
   if (i != 0)
      return false;
   node->methods     = SampleBokehMtd;
   node->output_type = AI_TYPE_RGBA;
   node->name        = "sample_bokeh";
   node->node_type   = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}