#include <ai.h>
#include <algorithm>
#include "global.h"

// currently this works by searching for a node with specific name "lentil_replaced_filter", not ideal.

#define AI_DRIVER_SCHEDULE_FULL 0x02

AI_DRIVER_NODE_EXPORT_METHODS(LentilImagerMtd);



class compareTail {
public:
    bool operator()(const std::pair<float, float> x, const std::pair<float, float> y) {
        return x.second > y.second;
    }
};



// struct LentilImagerData {
//     AtString camera_node_type;
//     AtString lentil_thinlens_string;
//     AtString lentil_po_string;
//     AtNode *camera_node;
// };

node_parameters 
{
  AiMetaDataSetStr(nentry, nullptr, AtString("subtype"), AtString("imager"));
  // AiParameterStr(AtString("layer_selection"), AtString("*")); // if enabled, mtoa/c4dtoa will only run over rgba (hardcoded for now)
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
  return  pixel_type == AI_TYPE_RGBA || 
          pixel_type == AI_TYPE_RGB || 
          pixel_type == AI_TYPE_FLOAT || 
          pixel_type == AI_TYPE_VECTOR;
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
    AiMsgInfo("[LENTIL IMAGER] Imager found AOV %s of type %d", aov_name_cstr, aov_type);
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


              std::string aov_name_string = aov_name_cstr;
              if (aov_name_string.find("crypto") != std::string::npos) {
          
                int rank = 0;
                if (aov_name == AtString("crypto_material00")) rank = 0;
                if (aov_name == AtString("crypto_material01")) rank = 2;
                if (aov_name == AtString("crypto_material02")) rank = 4;

                if (aov_name == AtString("crypto_asset00")) rank = 0;
                if (aov_name == AtString("crypto_asset01")) rank = 2;
                if (aov_name == AtString("crypto_asset02")) rank = 4;

                if (aov_name == AtString("crypto_object00")) rank = 0;
                if (aov_name == AtString("crypto_object01")) rank = 2;
                if (aov_name == AtString("crypto_object02")) rank = 4;
                
                
                // crypto ranking
                AtRGBA out = AI_RGBA_ZERO;
                // rank 0 means if vals.size() does not contain 0, we can stop
                // rank 2 means if vals.size() does not contain 2, we can stop
                if (filter_data->crypto_hash_map[aov_name][linear_pixel].size() <= rank) {
                  break;
                }

                std::map<float, float>::iterator vals_iter;

                std::vector<std::pair<float, float>> all_vals;
                std::vector<std::pair<float, float>>::iterator all_vals_iter;

                all_vals.reserve(filter_data->crypto_hash_map[aov_name][linear_pixel].size());
                for (vals_iter = filter_data->crypto_hash_map[aov_name][linear_pixel].begin(); vals_iter != filter_data->crypto_hash_map[aov_name][linear_pixel].end(); ++vals_iter){
                   all_vals.push_back(*vals_iter);
                }

                std::sort(all_vals.begin(), all_vals.end(), compareTail());

                int iter = 0;
                
                for (all_vals_iter = all_vals.begin(); all_vals_iter != all_vals.end(); ++all_vals_iter) {
                    if (iter == rank) {
                        out.r = all_vals_iter->first;
                        out.g = (all_vals_iter->second / filter_data->crypto_total_weight[aov_name][linear_pixel]);
                    } else if (iter == rank + 1) {
                        out.b = all_vals_iter->first;
                        out.a = (all_vals_iter->second / filter_data->crypto_total_weight[aov_name][linear_pixel]);
                    }
                    iter++;
                }
                
                ((AtRGBA*)bucket_data)[in_idx] = out;
              }

              else {

                AtRGBA image = filter_data->image_col_types[aov_name][linear_pixel];
                if (((AtRGBA*)bucket_data)[in_idx].a >= 1.0) image /= (image.a == 0.0) ? 1.0 : image.a;
                
                ((AtRGBA*)bucket_data)[in_idx] = image;
              }
              break;
            }

            case AI_TYPE_RGB: {
              AtRGBA image = filter_data->image_col_types[aov_name][linear_pixel];
              image /= (image.a == 0.0) ? 1.0 : image.a;

              AtRGB final_value = AtRGB(image.r, image.g, image.b);
              ((AtRGB*)bucket_data)[in_idx] = final_value;
              break;
            }

            case AI_TYPE_FLOAT: {
              
                ((float*)bucket_data)[in_idx] = filter_data->image_data_types[aov_name][linear_pixel].r;
              
              
              break;
            }

            case AI_TYPE_VECTOR: {
              AtVector final_value (filter_data->image_data_types[aov_name][linear_pixel].r, 
                                    filter_data->image_data_types[aov_name][linear_pixel].g,
                                    filter_data->image_data_types[aov_name][linear_pixel].b);
              ((AtVector*)bucket_data)[in_idx] = final_value;
              break;
            }

            // case AI_TYPE_INT: {
            //   ((int*)bucket_data)[in_idx] = filter_data->image_data_types[aov_name][linear_pixel].r;
            //   break;
            // }

            // case AI_TYPE_UINT: {
            //   ((unsigned int*)bucket_data)[in_idx] = std::abs(filter_data->image_data_types[aov_name][linear_pixel].r);
            //   break;
            // }

            // case AI_TYPE_POINTER: {
            //   ((const void**)bucket_data)[in_idx] = filter_data->image_ptr_types[aov_name][linear_pixel];
            //   break;
            // }
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
  crypto_crit_sec_enter();
  const AtNode *bokeh_filter_node = AiNodeLookUpByName("lentil_replaced_filter");
  LentilFilterData *bokeh = (LentilFilterData*)AiNodeGetLocalData(bokeh_filter_node);
  delete bokeh;
  crypto_crit_sec_leave();
}


 void registerLentilImager(AtNodeLib* node) {
    node->methods = (AtNodeMethods*) LentilImagerMtd;
    node->output_type = AI_TYPE_NONE;
    node->name = "lentil_imager";
    node->node_type = AI_NODE_DRIVER;
    strcpy(node->version, AI_VERSION);
}