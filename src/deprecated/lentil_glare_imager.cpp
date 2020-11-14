#include <ai.h>
#include <vector>
#include <iostream>
#include <map>

#include <OpenImageIO/imageio.h>
using namespace OIIO;

#define AI_DRIVER_SCHEDULE_FULL 0x02

AI_DRIVER_NODE_EXPORT_METHODS(LentilImagerMtd);


struct LentilImagerData {
    AtString camera_node_type;
    AtString lentil_thinlens_string;
    AtString lentil_po_string;
    AtNode *camera_node;

};



node_parameters 
{
  AiMetaDataSetStr(nentry, nullptr, AtString("subtype"), AtString("imager"));
  AiParameterStr(AtString("layer_selection"), AtString("*"));
  AiParameterBool(AtString("enable"), true);
  AiMetaDataSetBool(nentry, nullptr, "force_update", true);


}
 
node_initialize
{
  LentilImagerData* imager_data = (LentilImagerData*)AiMalloc(sizeof(LentilImagerData));
  AiNodeSetLocalData(node, imager_data);  
  AiDriverInitialize(node, false);
}
 
node_update 
{
  AiRenderSetHintInt(AtString("imager_schedule"), AI_DRIVER_SCHEDULE_FULL);

  LentilImagerData* imager_data = (LentilImagerData*)AiNodeGetLocalData(node);
  // imager_data->exposure = AiNodeGetFlt(node, "exposure");
  // const AtNode *bokeh_filter_node = AiNodeLookUpByName("lentil_replaced_filter");
  // LentilFilterData *filter_data = (LentilFilterData*)AiNodeGetLocalData(bokeh_filter_node);
  // if (filter_data->enabled) AiMsgInfo("[LENTIL BIDIRECTIONAL TL] Starting Imager.");

  // do i need to resize the 

  // Eigen::MatrixXcf fft_image = fft()
  // imager_data->image_aperture->fft();
  // calculate obstacle picture (fft(aperture*obstacle))
  // will need to care about offsets, should be different in screenspace.
  // first try putting it here, could be too slow
}
 
driver_supports_pixel_type 
{
  return pixel_type == AI_TYPE_RGBA;
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
  LentilImagerData* imager_data = (LentilImagerData*)AiNodeGetLocalData(node);

  const char *aov_name_cstr = 0;
  int aov_type = 0;
  const void *bucket_data;

  // loop over image, store values
  std::vector<std::vector<AtRGBA>> image_values(bucket_size_y, std::vector<AtRGBA> (bucket_size_x, AI_RGBA_ZERO));
  AiMsgInfo("Created vector");

  while (AiOutputIteratorGetNext(iterator, &aov_name_cstr, &aov_type, &bucket_data)){
    for (int j = 0; j < bucket_size_y; ++j) {
      for (int i = 0; i < bucket_size_x; ++i) {
        
        int y = j + bucket_yo;
        int x = i + bucket_xo;
        int in_idx = j * bucket_size_x + i;
        
        switch (aov_type){
          case AI_TYPE_RGBA: {
            image_values[y][x] = ((AtRGBA*)bucket_data)[in_idx];
            break;
          }
        }
      }
    }
  }

  AiOutputIteratorReset(iterator);
  AiMsgInfo("reset the iterator, finished section 1");


  // do processing
  ImageBuf zero = ImageBufAlgo::zero (ROI(0,512,0,512,0,1,0,3));
  
  while (AiOutputIteratorGetNext(iterator, &aov_name_cstr, &aov_type, &bucket_data)){
    for (int j = 0; j < bucket_size_y; ++j) {
      for (int i = 0; i < bucket_size_x; ++i) {
        

        int y = j + bucket_yo;
        int x = i + bucket_xo;
        int in_idx = j * bucket_size_x + i;
        // int linear_pixel = x + (y * (double)filter_data->xres);
        
        switch (aov_type){
          case AI_TYPE_RGBA: {
            ((AtRGBA*)bucket_data)[in_idx] = image_values[y][x];
            break;
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
  node->methods = (AtNodeMethods*) LentilImagerMtd;
  node->output_type = AI_TYPE_NONE;
  node->name = "lentil_glare_imager";
  node->node_type = AI_NODE_DRIVER;
  strcpy(node->version, AI_VERSION);
  return true;
}
 