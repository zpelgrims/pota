#include <ai.h>
#include <vector>
#include "pota.h"
#include "lens.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

//json parsing
// #include "../../polynomial-optics/ext/json.hpp"
// #include <fstream>
// using json = nlohmann::json;


AI_SHADER_NODE_EXPORT_METHODS(PotaBokehAOVMtd);


inline void replace_frame_numbering(std::string &original_string){
  std::string substring = "";
  size_t numberofhashes = std::count(original_string.begin(), original_string.end(), '#');
  for (int i = 0; i < numberofhashes; i++) substring.insert(0, "#");

  std::string framestring = std::to_string(static_cast<int>(AiNodeGetFlt(AiUniverseGetOptions(), "frame")));
  framestring.insert(0, substring.length() - framestring.length(), '0');
  original_string.replace(original_string.find(substring), substring.length(), framestring);
}


struct PotaBokehAOV
{
  AtString bokeh_aov_name;
  AtString discarded_samples_aov_name;
  int aa_samples;
  int xres;
  int yres;
  int samples;
  std::vector<AtRGBA> image;
};
 

enum SampleBokehParams
{
   p_bokeh_aov_name,
   p_discarded_samples_aov_name,
};


node_parameters
{
   AiParameterStr("bokeh_aov_name", "");
   AiMetaDataSetBool(nentry, "bokeh_aov_name", "linkable", false);
   AiParameterStr("discarded_samples_aov_name", "");
   AiMetaDataSetBool(nentry, "discarded_samples_aov_name", "linkable", false);
}

 
node_initialize
{
   AiNodeSetLocalData(node, new PotaBokehAOV());
}

 
node_update
{
  PotaBokehAOV *bokeh = (PotaBokehAOV*)AiNodeGetLocalData(node);
  // Camera *camera = (Camera*)AiNodeGetLocalData(AiUniverseGetCamera());

  // register AOVs
  bokeh->bokeh_aov_name = AiNodeGetStr(node, "bokeh_aov_name");
  AiAOVRegister(bokeh->bokeh_aov_name, AI_TYPE_RGBA, AI_AOV_BLEND_OPACITY);
  bokeh->discarded_samples_aov_name = AiNodeGetStr(node, "discarded_samples_aov_name");
  AiAOVRegister(bokeh->discarded_samples_aov_name, AI_TYPE_RGBA, AI_AOV_BLEND_OPACITY);

  // get general options
  bokeh->aa_samples = AiNodeGetInt(AiUniverseGetOptions(), "AA_samples");
  bokeh->xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
  bokeh->yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");

  bokeh->image.clear();
  bokeh->image.reserve(bokeh->xres * bokeh->yres);


  // Draw &draw = camera->draw;
  // draw.pxpy.clear();
  // draw.sensor_shifted.clear();
  // draw.aperture.clear();
  // draw.out.clear();
  // draw.sensor.clear();
  // draw.counter = 0;
}


node_finish
{
  PotaBokehAOV *bokeh = (PotaBokehAOV*)AiNodeGetLocalData(node);
  Camera *camera = (Camera*)AiNodeGetLocalData(AiUniverseGetCamera());

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


  // replace frame numbering
  std::string original_string = camera->bokeh_exr_path.c_str();
  replace_frame_numbering(original_string);
  SaveEXR(image.data(), bokeh->xres, bokeh->yres, 4, 0, original_string.c_str());
  AiMsgWarning("[POTA] Bokeh AOV written to %s", original_string.c_str());



  // Draw &draw = camera->draw;
  // json point_data;
  // point_data["pxpy"] = draw.pxpy;
  // point_data["sensor_shifted"] = draw.sensor_shifted;
  // point_data["sensor"] = draw.sensor;
  // point_data["out"] = draw.out;
  // point_data["aperture"] = draw.aperture;
  // std::ofstream out_json("/Users/zeno/lentil/pota/point_data.json");
  // out_json << std::setw(2) << point_data << std::endl;
  // AiMsgInfo("Written point data json");

  delete bokeh;
}


shader_evaluate
{
   PotaBokehAOV *bokeh = (PotaBokehAOV*)AiNodeGetLocalData(node);
   Camera *camera = (Camera*)AiNodeGetLocalData(AiUniverseGetCamera());

  // Draw &draw = camera->draw;
  // draw.enabled = true;

   // why does this need to be in shader_evaluate to work? returns 0 in _update_?
   bokeh->samples = camera->backward_samples * (bokeh->aa_samples * bokeh->aa_samples);

   const float xres = (float)bokeh->xres;
   const float yres = (float)bokeh->yres;
   const float frame_aspect_ratio = xres/yres;
   AtRGBA sample_energy = AI_RGBA_ZERO;

   // write AOV only if in use
   if ((sg->Rt & AI_RAY_CAMERA) && AiAOVEnabled(bokeh->bokeh_aov_name, AI_TYPE_RGBA))
   {
      // figure out better way, based on:
      // distance from focus point
      // intensity of sample
      if ((sg->out.RGBA().r > camera->minimum_rgb) || (sg->out.RGBA().g > camera->minimum_rgb) || (sg->out.RGBA().b > camera->minimum_rgb))
      {
        // write discarded samples into separate AOV to be able to difference
        if (AiAOVEnabled(bokeh->discarded_samples_aov_name, AI_TYPE_RGBA)){
          AiAOVSetRGBA(sg, bokeh->discarded_samples_aov_name, sg->out.RGBA());
        }
        
        sample_energy = sg->out.RGBA();
        sample_energy.a = 1.0f;
        sample_energy /=  static_cast<float>(bokeh->samples);

        // convert sample world space position to camera space
        AtMatrix world_to_camera_matrix;
        Eigen::Vector2d sensor_position;

        AiWorldToCameraMatrix(AiUniverseGetCamera(), sg->time, world_to_camera_matrix);
        // improve this, too much copying
        AtVector camera_space_sample_position_tmp = AiM4PointByMatrixMult(world_to_camera_matrix, sg->P);
        Eigen::Vector3f camera_space_sample_position(camera_space_sample_position_tmp.x, camera_space_sample_position_tmp.y, camera_space_sample_position_tmp.z);
         
        for(int count=0; count<bokeh->samples; count++)
        {
          if(!trace_backwards( -camera_space_sample_position * 10.0, camera->aperture_radius, camera->lambda, sensor_position, camera->sensor_shift, camera))
          {
            continue;
            --count;
          }

          // convert sensor position to pixel position
          Eigen::Vector2d s(sensor_position(0) / (camera->sensor_width * 0.5), sensor_position(1) / (camera->sensor_width * 0.5) * frame_aspect_ratio);

          const float pixel_x = ((s(0) + 1.0) / 2.0) * xres;
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
          bokeh->image[pixelnumber] += sample_energy;
        }
      }

    // ideally would be cool to write to an aov but not sure if I can access the different pixels other than
    // the one related to the current sample
    // AtRGBA aov_value = bokeh->image[bokeh->xres * (sg->y+1, sg->x+1)]; // IS THE +1 CORRECT? 
    // AiAOVSetRGBA(sg, bokeh->bokeh_aov_name, aov_value);
  }
}
 
node_loader
{
  if (i != 0) return false;
  node->methods     = PotaBokehAOVMtd;
  node->output_type = AI_TYPE_RGBA;
  node->name        = "potabokehAOV";
  node->node_type   = AI_NODE_SHADER;
  strcpy(node->version, AI_VERSION);
  return true;
}
