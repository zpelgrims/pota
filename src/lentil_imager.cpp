#include <ai.h>
#include <vector>
#include <iostream>
#include <map>
#include "lentil_thinlens.h"
#include "lentil.h"

// currently this works by searching for a node with specific name "lentil_replaced_filter", not ideal.

#define AI_DRIVER_SCHEDULE_FULL 0x02

AI_DRIVER_NODE_EXPORT_METHODS(ThinLensBokehImagerMtd);

// struct LentilImagerData {
//     AtString camera_node_type;
//     AtString lentil_thinlens_string;
//     AtString lentil_po_string;
//     AtNode *camera_node;
// };

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

  // const AtNode *bokeh_filter_node = AiNodeLookUpByName("lentil_replaced_filter");
  // LentilFilterData *filter_data = (LentilFilterData*)AiNodeGetLocalData(bokeh_filter_node);
  // if (filter_data->enabled) AiMsgInfo("[LENTIL BIDIRECTIONAL TL] Starting Imager.");
}
 
driver_supports_pixel_type { return true; } // not needed for raw drivers
 
driver_open {}
 
driver_extension
{
   static const char *extensions[] = {"exr", NULL};
   return extensions;
}
 
driver_needs_bucket
{
   return true; // API: true if the bucket needs to be rendered, false if it can be skipped
}
 
driver_prepare_bucket {} // called before a bucket is rendered


 
driver_process_bucket {
  // LentilImagerData* imager_data = (LentilImagerData*)AiNodeGetLocalData(node);

  const AtNode *bokeh_filter_node = AiNodeLookUpByName("lentil_replaced_filter");
  LentilFilterData *filter_data = (LentilFilterData*)AiNodeGetLocalData(bokeh_filter_node);

  
  if (!filter_data->enabled) {
    AiMsgInfo("[LENTIL IMAGER] Skipping imager");
    return;
  }

  const char *aov_name_cstr = 0;
  int aov_type = 0;
  const void *bucket_data;
  while (AiOutputIteratorGetNext(iterator, &aov_name_cstr, &aov_type, &bucket_data)){
    if (std::find(filter_data->aov_list_name.begin(), filter_data->aov_list_name.end(), AtString(aov_name_cstr)) != filter_data->aov_list_name.end()){
      if (AtString(aov_name_cstr) == AtString("transmission")) continue;
      AiMsgInfo("[LENTIL IMAGER] %s writing to: %s", AiNodeGetName(node), aov_name_cstr);
      AtString aov_name = AtString(aov_name_cstr);

      for (int j = 0; j < bucket_size_y; ++j) {
        for (int i = 0; i < bucket_size_x; ++i) {
          int y = j + bucket_yo;
          int x = i + bucket_xo;
          int in_idx = j * bucket_size_x + i;
          int linear_pixel = x + (y * (double)filter_data->xres);
          
          switch (aov_type){
            case AI_TYPE_RGBA: {
              AtRGBA image_redist = filter_data->image_redist[aov_name][linear_pixel];
              AtRGBA image_unredist = filter_data->image_unredist[aov_name][linear_pixel];
              float redist_weight = filter_data->redist_weight_per_pixel[aov_name][linear_pixel];
              float unredist_weight = filter_data->unredist_weight_per_pixel[aov_name][linear_pixel];

              AtRGBA redist = AI_RGBA_ZERO;
              if ((redist_weight) != 0.0) {
                // BUG!!! *2.0 is because it's ran two times i think..
                redist = image_redist / (4.0); //magic number related to pixel filter width of 2. (4 times as many pixels considered)
              }

              AtRGBA unredist = AI_RGBA_ZERO;
              if ((unredist_weight) != 0.0) {
                unredist = image_unredist / unredist_weight;
              }

              ((AtRGBA*)bucket_data)[in_idx] = redist+unredist;
              break;
            }

            case AI_TYPE_RGB: {
              AtRGBA image_redist = filter_data->image_redist[aov_name][linear_pixel];
              AtRGBA image_unredist = filter_data->image_unredist[aov_name][linear_pixel];
              float redist_weight = filter_data->redist_weight_per_pixel[aov_name][linear_pixel];
              float unredist_weight = filter_data->unredist_weight_per_pixel[aov_name][linear_pixel];
              
              
              AtRGBA redist = AI_RGBA_ZERO;
              if ((redist_weight) != 0.0) {
                // BUG!!! *2.0 is because it's ran two times i think..
                redist = image_redist / (4.0); //magic number related to pixel filter width of 2. (4 times as many pixels considered)
              }

              AtRGBA unredist = AI_RGBA_ZERO;
              if ((unredist_weight) != 0.0) {
                unredist = image_unredist / unredist_weight;
              }

              AtRGB final_value = AI_RGB_ZERO;
              final_value.r = redist.r + unredist.r;
              final_value.g = redist.g + unredist.g;
              final_value.b = redist.b + unredist.b;
              ((AtRGB*)bucket_data)[in_idx] = final_value;
              break;
            }

            case AI_TYPE_FLOAT: {
              ((float*)bucket_data)[in_idx] = filter_data->image[aov_name][linear_pixel].r;
              break;
            }

            case AI_TYPE_INT: {
              ((int*)bucket_data)[in_idx] = filter_data->image[aov_name][linear_pixel].r;
              break;
            }

            case AI_TYPE_VECTOR: {
              AtVector final_value (0, 0, 0);
              final_value[0] = filter_data->image[aov_name][linear_pixel].r;
              final_value[1] = filter_data->image[aov_name][linear_pixel].g;
              final_value[2] = filter_data->image[aov_name][linear_pixel].b;
              ((AtVector*)bucket_data)[in_idx] = final_value;
              break;
            }

          }
        }
      }
    }
  }
}


driver_write_bucket {}
 
driver_close {}
 
node_finish {
  // LentilImagerData* imager_data = (LentilImagerData*)AiNodeGetLocalData(node);
  // delete imager_data;
}

node_loader
{
  if (i>0) return false;
  node->methods = (AtNodeMethods*) ThinLensBokehImagerMtd;
  node->output_type = AI_TYPE_NONE;
  node->name = "lentil_imager";
  node->node_type = AI_NODE_DRIVER;
  strcpy(node->version, AI_VERSION);
  return true;
}
 