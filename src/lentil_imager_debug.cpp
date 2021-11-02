#include <ai.h>
#include <vector>

#define AI_DRIVER_SCHEDULE_FULL 0x02

AI_DRIVER_NODE_EXPORT_METHODS(LentilImagerMtd);


inline std::vector<std::string> split_str(std::string str, std::string token)
{
    std::vector<std::string>result;
    while(str.size())
    {
        size_t index = static_cast<size_t>(str.find(token));
        
        if(index != std::string::npos)
        {
            result.push_back(str.substr(0, index));
            str = str.substr(index+token.size());
            
            if(str.size() == 0)
                result.push_back(str);
        }
        else
        {
            result.push_back(str);
            str = "";
        }
    }
    return result;
}



node_parameters 
{
  AiMetaDataSetStr(nentry, nullptr, AtString("subtype"), AtString("imager"));
  // AiParameterStr(AtString("layer_selection"), AtString("*")); // if enabled, mtoa/c4dtoa will only run over rgba (hardcoded for now)
  AiParameterBool(AtString("enable"), true);
  AiMetaDataSetBool(nentry, nullptr, "force_update", true);
}
 
node_initialize
{
  // AiNodeSetLocalData(node, new LentilFilterData());
  AiDriverInitialize(node, false);
}
 
node_update 
{
  AiRenderSetHintInt(AtString("imager_schedule"), AI_DRIVER_SCHEDULE_FULL);
  AiRenderSetHintInt(AtString("imager_padding"), 0);

  AtUniverse *uni = AiNodeGetUniverse(node);
  AtNode* options = AiUniverseGetOptions(uni);
  AtArray* outputs = AiNodeGetArray(options, "outputs");
  for (size_t i=0; i<AiArrayGetNumElements(outputs); ++i) {
    std::string output_string = AiArrayGetStr(outputs, i).c_str();
    std::string lentil_str = "lentil_replaced_filter";

    if (output_string.find(lentil_str) != std::string::npos){
     
      std::string name = split_str(output_string, std::string(" ")).begin()[0];
      std::string type = split_str(output_string, std::string(" ")).begin()[1];
      AtString name_as = AtString(name.c_str());

      AiMsgInfo("output string: %s", output_string.c_str());
      AiMsgInfo("[LENTIL IMAGER] Adding aov %s of type %s", name.c_str(), type.c_str());
    }
  }

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


 
driver_process_bucket {}


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