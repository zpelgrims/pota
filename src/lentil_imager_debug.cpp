#include <ai.h>

AI_DRIVER_NODE_EXPORT_METHODS(LentilImagerMtd);

node_parameters 
{
  AiMetaDataSetStr(nentry, nullptr, AtString("subtype"), AtString("imager"));
  AiParameterBool(AtString("enable"), true);
  AiMetaDataSetBool(nentry, nullptr, "force_update", true);
}
 
node_initialize
{
  AiDriverInitialize(node, false);
}
 
node_update 
{
  AtUniverse *arnold_universe = AiNodeGetUniverse(node);

  // imager setup
  AiRenderSetHintInt(render_session, AtString("imager_padding"), 0);
  AiRenderSetHintInt(render_session, AtString("imager_schedule"), 0x02); // CAUSES CRASH WHEN NEGATIVE PIXEL REGIONS ARE USED
}
 
driver_supports_pixel_type 
{
  return  pixel_type == AI_TYPE_RGBA;
}
 
driver_open {}
 
driver_extension
{
   static const char *extensions[] = {NULL};
   return extensions;
}
 
driver_needs_bucket
{
   return true;
}
 
driver_prepare_bucket {} // called before a bucket is rendered
 
driver_process_bucket {}
  
driver_write_bucket {}
 
driver_close {}
 
node_finish {}


 void registerLentilImagerDebug(AtNodeLib* node) {
    node->methods = (AtNodeMethods*) LentilImagerMtd;
    node->output_type = AI_TYPE_NONE;
    node->name = "imager_lentil";
    node->node_type = AI_NODE_DRIVER;
    strcpy(node->version, AI_VERSION);
}