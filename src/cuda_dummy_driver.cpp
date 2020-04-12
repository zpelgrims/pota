#include <ai.h>
#include <vector>
#include <iostream>
#include <map>


#include "cuda_dummy_driver.h"

 
AI_DRIVER_NODE_EXPORT_METHODS(CudaBokehDriverMtd);
 
struct ThinLensBokehDriver {
  unsigned xres;
  unsigned yres;
  int framenumber;
  int samples;
  int aa_samples;
  int min_aa_samples;
  bool enabled;
  float filter_width;
  float time_start;
  float time_end;
  std::map<AtString, std::vector<AtRGBA> > image;
  std::map<AtString, std::vector<AtRGBA> > image_redist;
  std::map<AtString, std::vector<AtRGBA> > image_unredist;
  std::map<AtString, std::vector<float> > redist_weight_per_pixel;
  std::map<AtString, std::vector<float> > unredist_weight_per_pixel;
  std::vector<float> zbuffer;
  std::vector<AtString> aov_list_name;
  std::vector<unsigned int> aov_list_type;
  std::vector<int> aov_types;

  AtString rgba_string;
};



node_parameters {}
 
node_initialize
{
  AiNodeSetLocalData(node, new ThinLensBokehDriver());
  static const char *required_aovs[] = {"RGBA RGBA", "VECTOR P", "FLOAT Z", "RGBA transmission", "RGBA lentil_bidir_ignore", NULL};
  AiRawDriverInitialize(node, required_aovs, false);
}
 
node_update  {}
 
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


 
driver_process_bucket {}
 
driver_write_bucket {}
 
driver_close
{
  allthestuff();
}
 
node_finish
{
  ThinLensBokehDriver *bokeh = (ThinLensBokehDriver*)AiNodeGetLocalData(node);
  delete bokeh;
}

node_loader
{
  if (i>0) return false;
  node->methods = (AtNodeMethods*) CudaBokehDriverMtd;
  node->output_type = AI_TYPE_NONE;
  node->name = "cuda_bokeh_driver";
  node->node_type = AI_NODE_DRIVER;
  strcpy(node->version, AI_VERSION);
  return true;
}
 