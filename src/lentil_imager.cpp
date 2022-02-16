#include <ai.h>
#include <algorithm>
#include "global.h"
#include "lentil.h"


AI_DRIVER_NODE_EXPORT_METHODS(LentilImagerMtd);



class compareTail {
public:
    bool operator()(const std::pair<float, float> x, const std::pair<float, float> y) {
        return x.second > y.second;
    }
};


node_parameters 
{
  AiMetaDataSetStr(nentry, nullptr, AtString("subtype"), AtString("imager"));
  // AiParameterStr(AtString("layer_selection"), AtString("*")); // if enabled, mtoa/c4dtoa will only run over rgba (hardcoded for now)
  AiParameterBool(AtString("enable"), true);
  // AiMetaDataSetBool(nentry, nullptr, "force_update", true);
}


 
node_initialize
{
  AiDriverInitialize(node, false);
}
 
node_update {
    AtUniverse *universe = AiNodeGetUniverse(node);
    AtRenderSession *render_session = AiUniverseGetRenderSession(universe);
    AiRenderSetHintInt(render_session, AtString("imager_padding"), 0);
    AiRenderSetHintInt(render_session, AtString("imager_schedule"), 0x02); // SEEMS TO CAUSE ISSUES WITH NEGATIVE RENDER REGIONS    
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

  AtUniverse *universe = AiNodeGetUniverse(node);
  AtNode *camera_node = AiUniverseGetCamera(universe);
  Camera *camera_data = (Camera*)AiNodeGetLocalData(camera_node);

  if (!camera_data->redistribution) {
    if (!camera_data->imager_print_once_only){
      AiMsgInfo("[LENTIL IMAGER] Skipping imager");
      camera_data->imager_print_once_only = true;
    }
    
    return;
  }


  const AtString crypto_material00 = AtString("crypto_material00");
  const AtString crypto_material01 = AtString("crypto_material01");
  const AtString crypto_material02 = AtString("crypto_material02");
  const AtString crypto_object00 = AtString("crypto_object00");
  const AtString crypto_object01 = AtString("crypto_object01");
  const AtString crypto_object02 = AtString("crypto_object02");
  const AtString crypto_asset00 = AtString("crypto_asset00");
  const AtString crypto_asset01 = AtString("crypto_asset01");
  const AtString crypto_asset02 = AtString("crypto_asset02");



  AtString aov_name = AtString("");
  int aov_type = 0;
  const void *bucket_data = nullptr;

  while (AiOutputIteratorGetNext(iterator, &aov_name, &aov_type, &bucket_data)){
    if (!camera_data->imager_print_once_only) AiMsgInfo("[LENTIL IMAGER] Imager found AOV %s of type %s", aov_name.c_str(), AiParamGetTypeName(aov_type));

    AOVData *aov_current = nullptr;
    for (auto &aov : camera_data->aovs) {
      if (aov.name == aov_name) aov_current = &aov;
    }
    if (!aov_current) continue;

    if (aov_name == camera_data->atstring_transmission || aov_name == camera_data->atstring_lentil_ignore || aov_name == camera_data->atstring_time) continue;
    if (!camera_data->imager_print_once_only) AiMsgInfo("[LENTIL IMAGER] '%s' writing to: %s", AiNodeGetName(node), aov_name.c_str());

    for (int j = 0; j < bucket_size_y; ++j) {
      for (int i = 0; i < bucket_size_x; ++i) {
        int y = j + bucket_yo;
        int x = i + bucket_xo;
        int in_idx = j * bucket_size_x + i;
        // int linear_pixel = camera_data->coords_to_linear_pixel_region(x, y);
        int linear_pixel = camera_data->coords_to_linear_pixel(x, y);

        switch (aov_type){
          case AI_TYPE_RGBA: {

            std::string aov_name_string = aov_name.c_str();

            // CRYPTOMATTE
            if (aov_name_string.find("crypto") != std::string::npos) {

              int rank = 0;
              // if (aov_name == crypto_material00 || aov_name == crypto_asset00 || aov_name == crypto_object00) rank = 0;
              if (aov_name == crypto_material01 || aov_name == crypto_asset01 || aov_name == crypto_object01) rank = 2;
              else if (aov_name == crypto_material02 || aov_name == crypto_asset02 || aov_name == crypto_object02) rank = 4;
              
              // crypto ranking
              AtRGBA out = AI_RGBA_ZERO;
              // rank 0 means if vals.size() does not contain 0, we can stop
              // rank 2 means if vals.size() does not contain 2, we can stop
              if (aov_current->crypto_hash_map[linear_pixel].size() <= rank) {
                break;
              }

              std::map<float, float>::iterator vals_iter;
              std::vector<std::pair<float, float>> all_vals;
              std::vector<std::pair<float, float>>::iterator all_vals_iter;

              all_vals.reserve(aov_current->crypto_hash_map[linear_pixel].size());
              for (vals_iter = aov_current->crypto_hash_map[linear_pixel].begin(); vals_iter != aov_current->crypto_hash_map[linear_pixel].end(); ++vals_iter){
                  all_vals.push_back(*vals_iter);
              }

              std::sort(all_vals.begin(), all_vals.end(), compareTail());

              int iter = 0;
              
              for (all_vals_iter = all_vals.begin(); all_vals_iter != all_vals.end(); ++all_vals_iter) {
                  if (iter == rank) {
                      out.r = all_vals_iter->first;
                      out.g = (all_vals_iter->second / aov_current->crypto_total_weight[linear_pixel]);
                  } else if (iter == rank + 1) {
                      out.b = all_vals_iter->first;
                      out.a = (all_vals_iter->second / aov_current->crypto_total_weight[linear_pixel]);
                  }
                  iter++;
              }
              
              ((AtRGBA*)bucket_data)[in_idx] = out;
            }

            // usualz
            else {
              AtRGBA image = aov_current->buffer[linear_pixel];
              if (((AtRGBA*)bucket_data)[in_idx].a >= 1.0) image /= (image.a == 0.0) ? 1.0 : image.a; // issue here

              ((AtRGBA*)bucket_data)[in_idx] = image;
            }
            break;
          }

          case AI_TYPE_RGB: {
            AtRGBA image = aov_current->buffer[linear_pixel];
            image /= (image.a == 0.0) ? 1.0 : image.a;

            AtRGB final_value = AtRGB(image.r, image.g, image.b);
            ((AtRGB*)bucket_data)[in_idx] = final_value;
            break;
          }

          case AI_TYPE_FLOAT: {
            ((float*)bucket_data)[in_idx] = aov_current->buffer[linear_pixel].r;
            break;
          }

          case AI_TYPE_VECTOR: {
            AtVector final_value (aov_current->buffer[linear_pixel].r, 
                                  aov_current->buffer[linear_pixel].g,
                                  aov_current->buffer[linear_pixel].b);
            ((AtVector*)bucket_data)[in_idx] = final_value;
            break;
          }

          // case AI_TYPE_INT: {
          //   ((int*)bucket_data)[in_idx] = aov_current->buffer[linear_pixel].r;
          //   break;
          // }

          // case AI_TYPE_UINT: {
          //   ((unsigned int*)bucket_data)[in_idx] = std::abs(aov_current->buffer[linear_pixel].r);
          //   break;
          // }
        }
      }
    }
  }

  camera_data->imager_print_once_only = true;
}


driver_write_bucket {}
 
driver_close {}
 
node_finish {}


 void registerLentilImager(AtNodeLib* node) {
    node->methods = (AtNodeMethods*) LentilImagerMtd;
    node->output_type = AI_TYPE_NONE;
    node->name = "imager_lentil";
    node->node_type = AI_NODE_DRIVER;
    strcpy(node->version, AI_VERSION);
}