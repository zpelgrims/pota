#include <ai.h>


AI_FILTER_NODE_EXPORT_METHODS(LentilFilterDataMtd);
 

struct InternalFilterData {
  AtNode *imager_node;
};



node_parameters 
{
  AiMetaDataSetBool(nentry, nullptr, "force_update", true);
}
 
node_initialize
{
  static const char *required_aovs[] = {"RGBA RGBA", "VECTOR P", "FLOAT Z", "RGB opacity", "RGBA transmission", "FLOAT lentil_bidir_ignore", NULL};
  AiFilterInitialize(node, true, required_aovs);
  AiNodeSetLocalData(node, new InternalFilterData());
}


node_update {
  AiFilterUpdate(node, 2.0);
}
 
filter_output_type
{
   switch (input_type)
   {
      case AI_TYPE_RGBA:
         return AI_TYPE_RGBA;
      case AI_TYPE_RGB:
         return AI_TYPE_RGB;
      case AI_TYPE_VECTOR:
        return AI_TYPE_VECTOR;
      // case AI_TYPE_FLOAT:
      //   return AI_TYPE_FLOAT; // ORIG
      case AI_TYPE_FLOAT:
        return AI_TYPE_RGBA; // CRYPTO TEST
      // case AI_TYPE_INT:
      //   return AI_TYPE_INT;
      // case AI_TYPE_UINT:
      //   return AI_TYPE_UINT;
      // case AI_TYPE_POINTER:
      //   return AI_TYPE_POINTER;
      default:
         return AI_TYPE_NONE;
   }
}


filter_pixel
{
  int px, py;
  AiAOVSampleIteratorGetPixel(iterator, px, py);
  
  for (int sampleid=0; AiAOVSampleIteratorGetNext(iterator)==true; sampleid++) {
    AtRGBA sample = AiAOVSampleIteratorGetRGBA(iterator);
    const float inv_density = AiAOVSampleIteratorGetInvDensity(iterator);
    AiMsgInfo("inv_density: %f", inv_density); // why does this always return 1? happening since 7.0.0.0
  }
}

 
node_finish {}


void registerLentilFilterPO(AtNodeLib* node) {
    node->methods = (AtNodeMethods*) LentilFilterDataMtd;
    node->output_type = AI_TYPE_NONE;
    node->name = "lentil_filter";
    node->node_type = AI_NODE_FILTER;
    strcpy(node->version, AI_VERSION);
}