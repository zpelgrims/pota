#include <ai.h>
#include <algorithm>
#include "global.h"

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
  AiMetaDataSetBool(nentry, nullptr, "force_update", true);
}
 
node_initialize
{
  AiNodeSetLocalData(node, new LentilFilterData());
  AiDriverInitialize(node, false);
}
 
node_update 
{
  LentilFilterData *bokeh = (LentilFilterData*)AiNodeGetLocalData(node);
  bokeh->arnold_universe = AiNodeGetUniverse(node);
  bokeh->camera = AiUniverseGetCamera(bokeh->arnold_universe);
  bokeh->enabled = true;


  // imager setup
  AtRenderSession *render_session = AiUniverseGetRenderSession(bokeh->arnold_universe);
  AiRenderSetHintInt(render_session, AtString("imager_padding"), 0);
  AiRenderSetHintInt(render_session, AtString("imager_schedule"), 0x02);


  // disable for non-lentil cameras
  if (!AiNodeIs(bokeh->camera, AtString("lentil_camera"))) {
    bokeh->enabled = false;
    AiMsgError("[LENTIL FILTER] Camera is not of type lentil. A full scene update is required.");
    AiRenderAbort();
    return;
  }

  // if progressive rendering is on, don't redistribute
  if (AiNodeGetBool(AiUniverseGetOptions(bokeh->arnold_universe), "enable_progressive_render")) {
    bokeh->enabled = false;
    AiMsgError("[LENTIL FILTER] Progressive rendering is not supported.");
    AiRenderAbort();
    return;
  }

  if (!AiNodeGetBool(bokeh->camera, "enable_dof")) {
    AiMsgWarning("[LENTIL FILTER] Depth of field is disabled, therefore disabling bidirectional sampling.");
    bokeh->enabled = false;
  }

  if (AiNodeGetInt(bokeh->camera, "bidir_sample_mult") == 0){
    bokeh->enabled = false;
    AiMsgWarning("[LENTIL FILTER] Bidirectional samples are set to 0, filter will not execute.");
  }

  if (AiNodeGetBool(bokeh->camera, "bidir_debug")) {
    bokeh->enabled = false;
    AiMsgWarning("[LENTIL FILTER] Bidirectional Debug mode is on, no redistribution.");
  }

  
  bokeh->xres = AiNodeGetInt(AiUniverseGetOptions(bokeh->arnold_universe), "xres");
  bokeh->yres = AiNodeGetInt(AiUniverseGetOptions(bokeh->arnold_universe), "yres");
  bokeh->region_min_x = AiNodeGetInt(AiUniverseGetOptions(bokeh->arnold_universe), "region_min_x");
  bokeh->region_min_y = AiNodeGetInt(AiUniverseGetOptions(bokeh->arnold_universe), "region_min_y");
  bokeh->filter_width = 2.0;
  bokeh->time_start = AiCameraGetShutterStart();
  bokeh->time_end = AiCameraGetShutterEnd();

  bokeh->zbuffer.clear();
  bokeh->zbuffer.resize(bokeh->xres * bokeh->yres);



  // prepare framebuffers for all AOVS
  bokeh->aov_list_name.clear();
  bokeh->aov_list_type.clear();
  bokeh->aov_duplicates.clear();
  bokeh->crypto_hash_map.clear();
  bokeh->crypto_total_weight.clear();
  bokeh->image_data_types.clear();
  bokeh->image_col_types.clear();
  bokeh->image_ptr_types.clear();
  

  AtNode* options = AiUniverseGetOptions(bokeh->arnold_universe);
  AtArray* outputs = AiNodeGetArray(options, "outputs");
  for (size_t i=0; i<AiArrayGetNumElements(outputs); ++i) {
    std::string output_string = AiArrayGetStr(outputs, i).c_str();
    std::string lentil_str = "lentil_replaced_filter";

    if (output_string.find(lentil_str) != std::string::npos){
     
      std::string name = split_str(output_string, std::string(" ")).begin()[0];
      std::string type = split_str(output_string, std::string(" ")).begin()[1];
      AtString name_as = AtString(name.c_str());

      bokeh->aov_list_name.push_back(name_as);
      bokeh->aov_list_type.push_back(string_to_arnold_type(type));

      ++bokeh->aov_duplicates[name_as];

      bokeh->image_data_types[name_as].clear();
      bokeh->image_data_types[name_as].resize(bokeh->xres * bokeh->yres);
      bokeh->image_col_types[name_as].clear();
      bokeh->image_col_types[name_as].resize(bokeh->xres * bokeh->yres);
      bokeh->image_ptr_types[name_as].clear();
      bokeh->image_ptr_types[name_as].resize(bokeh->xres * bokeh->yres);

      AiMsgInfo("[LENTIL IMAGER] Adding aov %s of type %s", name.c_str(), type.c_str());
    }
  }


  // check if a cryptomatte aovshader is present
  bool cryptomatte_automatic_detection = false;
  AtArray* aov_shaders_array = AiNodeGetArray(options, "aov_shaders");
  for (size_t i=0; i<AiArrayGetNumElements(aov_shaders_array); ++i) {
    AtNode* aov_node = static_cast<AtNode*>(AiArrayGetPtr(aov_shaders_array, i));
    if (AiNodeEntryGetNameAtString(AiNodeGetNodeEntry(aov_node)) == AtString("cryptomatte")) {
      cryptomatte_automatic_detection = true;
    }
  }
  
  if (AiNodeGetBool(bokeh->camera, "cryptomatte") && cryptomatte_automatic_detection == false) {
    AiMsgInfo("[LENTIL IMAGER] Could not find a cryptomatte AOV shader, therefore disabling cryptomatte altogether.");
  }

  // crypto setup, still hardcoded
  bokeh->cryptomatte_aov_names.clear();
  bokeh->crypto_hash_map.clear();
  bokeh->crypto_total_weight.clear();
  if (AiNodeGetBool(bokeh->camera, "cryptomatte") && cryptomatte_automatic_detection == true) {
    std::vector<std::string> crypto_types{"crypto_asset", "crypto_material", "crypto_object"};
    std::vector<std::string> crypto_ranks{"00", "01", "02"};
    for (const auto& crypto_type: crypto_types) {
      for (const auto& crypto_rank : crypto_ranks) {
        std::string name_as_s = crypto_type+crypto_rank;
        AtString name_as = AtString(name_as_s.c_str());

        bokeh->aov_list_name.push_back(name_as);
        bokeh->aov_list_type.push_back(AI_TYPE_FLOAT);
        bokeh->crypto_hash_map[name_as].clear();
        bokeh->crypto_hash_map[name_as].resize(bokeh->xres * bokeh->yres);
        bokeh->crypto_total_weight[name_as].clear();
        bokeh->crypto_total_weight[name_as].resize(bokeh->xres * bokeh->yres);

        bokeh->cryptomatte_aov_names.push_back(name_as);
        AiMsgInfo("[LENTIL IMAGER] Adding aov %s of type %s", name_as.c_str(), "AI_TYPE_FLOAT");
      }
    }
  }
  

  bokeh->pixel_already_visited.clear();
  bokeh->pixel_already_visited.resize(bokeh->xres*bokeh->yres); // n amount of unique_ptrs, they point to nothing
  // init the vector with unique_ptrs that actually point to atomics
  for (auto& p : bokeh->pixel_already_visited) {
      p = std::make_unique<std::atomic<bool>>(false);   // init atomic bools to false
  }
  
  bokeh->current_inv_density = 0.0;

  if (bokeh->enabled) AiMsgInfo("[LENTIL IMAGER] Setup completed, starting bidirectional sampling.");


  // crypto_crit_sec_leave();
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

  // const AtNode *bokeh_filter_node = AiNodeLookUpByName("lentil_replaced_filter");
  // // don't run if lentil_replaced_filter node is not present
  // if (bokeh_filter_node == nullptr) {
  //   AiMsgInfo("[LENTIL IMAGER] Skipping imager, could not find lentil_filter");
  //   return;
  // }

  // LentilFilterData *filter_data = (LentilFilterData*)AiNodeGetLocalData(bokeh_filter_node);
  LentilFilterData *filter_data = (LentilFilterData*)AiNodeGetLocalData(node);
  if (!filter_data->enabled) {
    AiMsgInfo("[LENTIL IMAGER] Skipping imager");
    return;
  }

  // BUG: this could potentially fail when using adaptive sampling?
  if (filter_data->current_inv_density > 0.2) {
    AiMsgInfo("[LENTIL IMAGER] Skipping imager, AA samples < 3, current inv density: %f", filter_data->current_inv_density);
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



  AtString aov_name;
  int aov_type = 0;
  const void *bucket_data;

  while (AiOutputIteratorGetNext(iterator, &aov_name, &aov_type, &bucket_data)){
    AiMsgInfo("[LENTIL IMAGER] Imager found AOV %s of type %s", aov_name.c_str(), AiParamGetTypeName(aov_type));
    if (std::find(filter_data->aov_list_name.begin(), filter_data->aov_list_name.end(), aov_name) != filter_data->aov_list_name.end()){
      if (aov_name == AtString("transmission")) continue;
      if (aov_name == AtString("lentil_ignore")) continue;
      AiMsgInfo("[LENTIL IMAGER] %s writing to: %s", AiNodeGetName(node), aov_name.c_str());

      for (int j = 0; j < bucket_size_y; ++j) {
        for (int i = 0; i < bucket_size_x; ++i) {
          int y = j + bucket_yo;
          int x = i + bucket_xo;
          int in_idx = j * bucket_size_x + i;
          int linear_pixel = x + (y * (double)filter_data->xres);

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

              // usualz
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
  LentilFilterData *bokeh = (LentilFilterData*)AiNodeGetLocalData(node);
  delete bokeh;
}


 void registerLentilImager(AtNodeLib* node) {
    node->methods = (AtNodeMethods*) LentilImagerMtd;
    node->output_type = AI_TYPE_NONE;
    node->name = "imager_lentil";
    node->node_type = AI_NODE_DRIVER;
    strcpy(node->version, AI_VERSION);
}