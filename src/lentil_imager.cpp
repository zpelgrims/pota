#include <ai.h>
#include <vector>
#include <iostream>
#include <map>
#include "lentil_thinlens.h"
#include "lentil.h"

// currently this works by searching for a node with specific name "lentil_replaced_filter", not ideal.

#define AI_DRIVER_SCHEDULE_FULL 0x02

AI_DRIVER_NODE_EXPORT_METHODS(LentilImagerMtd);

// struct LentilImagerData {
//     AtString camera_node_type;
//     AtString lentil_thinlens_string;
//     AtString lentil_po_string;
//     AtNode *camera_node;
// };

node_parameters 
{
  AiMetaDataSetStr(nentry, nullptr, AtString("subtype"), AtString("imager"));
  AiParameterStr(AtString("layer_selection"), AtString("*"));
  AiParameterBool(AtString("enable"), true);
  AiMetaDataSetBool(nentry, nullptr, "force_update", true);
}
 
node_initialize
{
  // LentilImagerData* imager_data = (LentilImagerData*)AiMalloc(sizeof(LentilImagerData));
  // AiNodeSetLocalData(node, imager_data);  
  AiDriverInitialize(node, false);
}
 
node_update 
{
  AiRenderSetHintInt(AtString("imager_schedule"), AI_DRIVER_SCHEDULE_FULL);
  AiRenderSetHintInt(AtString("imager_padding"), 0);

  // LentilImagerData* imager_data = (LentilImagerData*)AiNodeGetLocalData(node);
  // const AtNode *bokeh_filter_node = AiNodeLookUpByName("lentil_replaced_filter");
  // LentilFilterData *filter_data = (LentilFilterData*)AiNodeGetLocalData(bokeh_filter_node);
  // if (filter_data->enabled) AiMsgInfo("[LENTIL IMAGER] Starting Imager.");
}
 
driver_supports_pixel_type 
{
  return pixel_type == AI_TYPE_RGBA || pixel_type == AI_TYPE_RGBA || pixel_type == AI_TYPE_FLOAT || pixel_type == AI_TYPE_INT || pixel_type == AI_TYPE_VECTOR;
}
 
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
  AiOutputIteratorReset(iterator);
  // LentilImagerData* imager_data = (LentilImagerData*)AiNodeGetLocalData(node);

  const AtNode *bokeh_filter_node = AiNodeLookUpByName("lentil_replaced_filter");
  // don't run if lentil_replaced_filter node is not present
  if (bokeh_filter_node == nullptr) {
    AiMsgInfo("[LENTIL IMAGER] Skipping imager, could not find lentil_filter");
    return;
  }

  LentilFilterData *filter_data = (LentilFilterData*)AiNodeGetLocalData(bokeh_filter_node);

  if (!filter_data->enabled) {
    AiMsgInfo("[LENTIL IMAGER] Skipping imager");
    return;
  }

  // BUG: this could potentially fail when using adaptive sampling?
  if (filter_data->current_inv_density > 0.2) {
    AiMsgInfo("[LENTIL IMAGER] Skipping imager, AA samples < 3");
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
              if (((AtRGBA*)bucket_data)[in_idx].a >= 1.0) image_redist /= (image_redist.a == 0.0) ? 1.0 : image_redist.a;
              
              ((AtRGBA*)bucket_data)[in_idx] = image_redist;
              break;
            }

            case AI_TYPE_RGB: {
              AtRGBA image_redist = filter_data->image_redist[aov_name][linear_pixel];
              image_redist /= (image_redist.a == 0.0) ? 1.0 : image_redist.a;

              AtRGB final_value = AI_RGB_ZERO;
              final_value.r = image_redist.r;
              final_value.g = image_redist.g;
              final_value.b = image_redist.b;
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

            // need to add POINTER type

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
  const AtNode *bokeh_filter_node = AiNodeLookUpByName("lentil_replaced_filter");
  LentilFilterData *bokeh = (LentilFilterData*)AiNodeGetLocalData(bokeh_filter_node);
  delete bokeh;
}

node_loader
{
  if (i>0) return false;
  node->methods = (AtNodeMethods*) LentilImagerMtd;
  node->output_type = AI_TYPE_NONE;
  node->name = "lentil_imager";
  node->node_type = AI_NODE_DRIVER;
  strcpy(node->version, AI_VERSION);
  return true;
}
 