#include <ai.h>
#include <vector>
#include <iostream>
#include <map>
#include "lentil_thinlens.h"


#define AI_DRIVER_SCHEDULE_FULL 0x02


AI_DRIVER_NODE_EXPORT_METHODS(ThinLensBokehImagerMtd);


node_parameters 
{
  AiMetaDataSetStr(nentry, nullptr, AtString("subtype"), AtString("imager"));
}
 
node_initialize
{
  AiDriverInitialize(node, false);
}
 
node_update 
{
  AiRenderSetHintInt(AtString("imager_schedule"), AI_DRIVER_SCHEDULE_FULL);
  
  const AtNodeEntry *bokeh_filter = AiNodeEntryLookUp("lentil_thin_lens_bokeh_filter");
  if (AiNodeEntryGetCount(bokeh_filter) == 0) AiMsgInfo("[LENTIL BIDIRECTIONAL TL] no lentil_filter found.");
  const AtNode *bokeh_filter_node = AiNodeLookUpByName("lentil_filter");
  ThinLensBokehDriver *bokeh = (ThinLensBokehDriver*)AiNodeGetLocalData(bokeh_filter_node);
  CameraThinLens *tl = (CameraThinLens*)AiNodeGetLocalData(AiUniverseGetCamera());

  if (bokeh->enabled) AiMsgInfo("[LENTIL BIDIRECTIONAL TL] Starting Imager.");
}
 
driver_supports_pixel_type { return true; } // not needed for raw drivers
 
driver_open {}
 
driver_extension
{
   static const char *extensions[] = {NULL};
   return extensions;
}
 
driver_needs_bucket
{
   return true; // API: true if the bucket needs to be rendered, false if it can be skipped
}
 
driver_prepare_bucket {} // called before a bucket is rendered


 
driver_process_bucket {
  
  const AtNode *bokeh_filter_node = AiNodeLookUpByName("lentil_filter");
  ThinLensBokehDriver *bokeh = (ThinLensBokehDriver*)AiNodeGetLocalData(bokeh_filter_node);
  CameraThinLens *tl = (CameraThinLens*)AiNodeGetLocalData(AiUniverseGetCamera());

  if (!bokeh->enabled) return;
  
  const double xres = (double)bokeh->xres;
  const double yres = (double)bokeh->yres;

  const char *aov_name_cstr = 0;
  int aov_type = 0;
  const void *bucket_data;
  
  // Iterate over all the AOVs hooked up to this driver
  while (AiOutputIteratorGetNext(iterator, &aov_name_cstr, &aov_type, &bucket_data)){

    const AtString aov_name = AtString(aov_name_cstr);
    if (aov_name == AtString("transmission")) continue;
    AiMsgInfo("imager looping over: %s", aov_name_cstr);

    for (int j = 0; j < bucket_size_y; ++j) {
      for (int i = 0; i < bucket_size_x; ++i) {
        int y = j + bucket_yo;
        int x = i + bucket_xo;
        int in_idx = j * bucket_size_x + i;
        int linear_pixel = x + (y * xres);

        // only rgba/rgb aovs have been guassian filtered, so need to normalize only them
        if (aov_type == AI_TYPE_RGBA || aov_type == AI_TYPE_RGB){

          AtRGBA redist = bokeh->image_redist[aov_name][linear_pixel] / ((bokeh->redist_weight_per_pixel[aov_name][linear_pixel] == 0.0) ? 1.0 : bokeh->redist_weight_per_pixel[aov_name][linear_pixel]);
          AtRGBA unredist = bokeh->image_unredist[aov_name][linear_pixel] / ((bokeh->unredist_weight_per_pixel[aov_name][linear_pixel] == 0.0) ? 1.0 : bokeh->unredist_weight_per_pixel[aov_name][linear_pixel]);
          AtRGBA combined_redist_unredist = (unredist * (1.0-bokeh->redist_weight_per_pixel[aov_name][linear_pixel])) + (redist * (bokeh->redist_weight_per_pixel[aov_name][linear_pixel]));
          
          // this currently doesn't work for the rgb layers because alpha is wrong for rgb layers
          if (combined_redist_unredist.a > 0.95) combined_redist_unredist /= combined_redist_unredist.a;


          ((AtRGBA*)bucket_data)[in_idx] = combined_redist_unredist;
          
        } 
        else {
          ((AtRGBA*)bucket_data)[in_idx] = bokeh->image[aov_name][linear_pixel];
        }
      }
    }
  }
}


driver_write_bucket {}
 
driver_close {}
 
node_finish {}

node_loader
{
  if (i>0) return false;
  node->methods = (AtNodeMethods*) ThinLensBokehImagerMtd;
  node->output_type = AI_TYPE_NODE;
  node->name = "lentil_thin_lens_bokeh_imager";
  node->node_type = AI_NODE_DRIVER;
  strcpy(node->version, AI_VERSION);
  return true;
}
 