#include <string>
#include <vector>
#include <cstdlib>

#include <ai.h>
#include <stdio.h>

#include "global.h"

// adds the correct filter for all aov's

AI_OPERATOR_NODE_EXPORT_METHODS(LentilOperatorMtd);

struct LentilOperatorData
{
    AtNode *filter;
    AtNode *camera_node;
    AtString camera_node_type;
    bool cook;
    bool debug;
};

node_parameters 
{
    AiMetaDataSetBool(nentry, nullptr, "force_update", true);
    AiParameterInt("call_me_dirty", 0);
}

operator_init
{
    LentilOperatorData* operator_data = (LentilOperatorData*)AiMalloc(sizeof(LentilOperatorData));


    operator_data->camera_node = AiUniverseGetCamera();
    const AtNodeEntry *nentry = AiNodeGetNodeEntry(operator_data->camera_node);
    operator_data->camera_node_type = AtString(AiNodeEntryGetName(nentry));

    operator_data->debug = AiNodeGetBool(operator_data->camera_node, "bidir_debug");

    operator_data->cook = false;

    if (operator_data->debug) {
        if (operator_data->camera_node_type == AtString("lentil_thinlens") || operator_data->camera_node_type == AtString("lentil")){
            operator_data->filter = AiNode("lentil_debug_filter", AtString("lentil_debug_filter"));
            operator_data->cook = true;
        }
    } else {
        if (operator_data->camera_node_type == AtString("lentil_thinlens")){
            operator_data->filter = AiNode("lentil_thin_lens_bokeh_filter", AtString("lentil_replaced_filter"));
            operator_data->cook = true;
        } else if (operator_data->camera_node_type == AtString("lentil")){
            operator_data->filter = AiNode("lentil_bokeh_filter", AtString("lentil_replaced_filter"));
            operator_data->cook = true;
        }
    }
    

    AiNodeSetLocalData(op, operator_data);
    return true;
}

operator_cook
{
    LentilOperatorData* operator_data = (LentilOperatorData*)AiNodeGetLocalData(op);

    if (operator_data->cook == false) {
        AiMsgWarning("[LENTIL OPERATOR] Camera is not of type lentil or lentil_thinlens");
        return false;
    }

    // const AtNodeEntry *ne_filterdebug = AiNodeEntryLookUp("lentil_debug_operator");
    // if (AiNodeEntryGetCount(ne_filterdebug) != 0) return false;

    crypto_crit_sec_enter();
    AtNode* options = AiUniverseGetOptions();
    AtArray* outputs = AiNodeGetArray(options, "outputs");

    const int elements = AiArrayGetNumElements(outputs);
    for (int i=0; i<elements; ++i) {
        std::string output_string = AiArrayGetStr(outputs, i).c_str();
        std::string filter = split_str(output_string, std::string(" ")).begin()[2]; // one before last, which is the filter
        std::string name = split_str(output_string, std::string(" ")).front();
        
        output_string.replace(output_string.find(filter), filter.length(), AiNodeGetStr(operator_data->filter, "name"));
        AiMsgInfo("[LENTIL OPERATOR] Added lentil_filter automatically to cloned AOV: %s", output_string.c_str());
        
        AiArraySetStr(outputs, i, AtString(output_string.c_str()));
    }
    crypto_crit_sec_leave();
    
    return true;
}

operator_post_cook
{
    return true;
}

operator_cleanup
{
    LentilOperatorData* operator_data = (LentilOperatorData*)AiNodeGetLocalData(op);
    AiFree(operator_data);
    return true;
}



void registerLentilOperator(AtNodeLib* node) {
    node->methods = (AtNodeMethods*)LentilOperatorMtd;
    node->name = "lentil_operator";
    node->node_type = AI_NODE_OPERATOR;
    strcpy(node->version, AI_VERSION);
}