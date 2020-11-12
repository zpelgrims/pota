#include <ai.h>
#include <string.h>
#include "lentil_thinlens.h"
#include "lentil.h"
 
AI_FILTER_NODE_EXPORT_METHODS(LentilDebugFilterMtd);
 
struct LentilDebugFilterData {
    AtString camera_node_type;
    AtString lentil_thinlens_string;
    AtString lentil_po_string;
    AtNode *camera_node;
};
 
node_parameters 
{
    AiMetaDataSetBool(nentry, nullptr, "force_update", true);
}
 
node_initialize
{
    AiFilterInitialize(node, false, NULL);
    AiNodeSetLocalData(node, new LentilDebugFilterData());
}
 
node_update
{
    LentilDebugFilterData* filter_data = (LentilDebugFilterData*)AiNodeGetLocalData(node);
       
    filter_data->camera_node = AiUniverseGetCamera();
    const AtNodeEntry *nentry = AiNodeGetNodeEntry(filter_data->camera_node);
    filter_data->camera_node_type = AtString(AiNodeEntryGetName(nentry));

    filter_data->lentil_thinlens_string = AtString("lentil_thinlens");
    filter_data->lentil_po_string = AtString("lentil");

    AiFilterUpdate(node, 1.65);
}
 
node_finish
{
   LentilDebugFilterData* filter_data = (LentilDebugFilterData*)AiNodeGetLocalData(node);
   delete filter_data;
}
 
filter_output_type
{
   switch (input_type)
   {
      case AI_TYPE_RGBA:
         return AI_TYPE_RGBA;
      default:
         return AI_TYPE_NONE;
   }
}
 
filter_pixel
{
    LentilDebugFilterData* filter_data = (LentilDebugFilterData*)AiNodeGetLocalData(node);
    
    const float bidir_min_luminance = AiNodeGetFlt(filter_data->camera_node, "bidir_min_luminance");
    AtRGBA avalue = AI_RGBA_ZERO;

    while (AiAOVSampleIteratorGetNext(iterator))
    {
        AtRGBA sample_energy = AiAOVSampleIteratorGetRGBA(iterator);
        const float sample_luminance = sample_energy.r*0.21 + sample_energy.g*0.71 + sample_energy.b*0.072;

        if (sample_luminance > bidir_min_luminance) {
            avalue = AtRGBA(1.0, 1.0, 1.0, 1.0);
        }
   }
 
   *((AtRGBA*)data_out) = avalue;
}
 
node_loader
{
   if (i>0)
      return false;
 
   node->methods      = LentilDebugFilterMtd;
   node->output_type  = AI_TYPE_NONE;
   node->name         = "lentil_debug_filter";
   node->node_type    = AI_NODE_FILTER;
   strcpy(node->version, AI_VERSION);
   return true;
}