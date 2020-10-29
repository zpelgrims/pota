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
};

node_parameters {}

operator_init
{
    LentilOperatorData* operator_data = (LentilOperatorData*)AiMalloc(sizeof(LentilOperatorData));

    operator_data->camera_node = AiUniverseGetCamera();
    const AtNodeEntry *nentry = AiNodeGetNodeEntry(operator_data->camera_node);
    operator_data->camera_node_type = AtString(AiNodeEntryGetName(nentry));

    operator_data->cook = false;

    if (operator_data->camera_node_type == AtString("lentil_thinlens")){
        operator_data->filter = AiNode("lentil_thin_lens_bokeh_filter", AtString("lentil_replaced_filter"));
        operator_data->cook = true;
    } else if (operator_data->camera_node_type == AtString("lentil")){
        operator_data->filter = AiNode("lentil_bokeh_filter", AtString("lentil_replaced_filter"));
        operator_data->cook = true;
    }

    AiNodeSetLocalData(op, operator_data);  
    return true;
}

operator_cook
{
    LentilOperatorData* operator_data = (LentilOperatorData*)AiNodeGetLocalData(op);

    if (operator_data->cook == false) return false;

    const AtNodeEntry *ne_filterdebug = AiNodeEntryLookUp("lentil_debug_operator");
    if (AiNodeEntryGetCount(ne_filterdebug) != 0) return false;

    // AtNode* lentil_imager_exr = AiNode("lentil_imager", AtString("lentil_imager_exr"));
    // AtNode* lentil_imager_kick = AiNode("lentil_imager", AtString("lentil_imager_kick"));
    
    AtNode* options = AiUniverseGetOptions();
    AtArray* outputs = AiNodeGetArray(options, "outputs");

    int offset = 0;
    int elements = AiArrayGetNumElements(outputs);

    // AiArrayResize(outputs, 2 * elements, 0);
    offset = elements;

    for (size_t i=0; i<elements; ++i) {
        std::string output_string = AiArrayGetStr(outputs, i).c_str();
        std::string filter = split_str(output_string, std::string(" ")).end()[-2]; // one before last, which is the filter
        std::string name = split_str(output_string, std::string(" ")).front();

        // AtNode *filter_node = AiNodeLookUpByName(AtString(filter.c_str()));
        // const AtNodeEntry *filter_ne = AiNodeGetNodeEntry(filter_node);
        // AtString filter_ne_name = AiNodeEntryGetNameAtString(filter_ne);
        // if (filter_ne_name == AtString("lentil_thin_lens_bokeh_filter") || filter_ne_name == AtString("lentil_bokeh_filter")){
        output_string.replace(output_string.find(filter), filter.length(), AiNodeGetStr(operator_data->filter, "name"));
        // output_string.replace(output_string.find(name), name.length(), name+std::string("_lentil"));
        AiMsgInfo("[LENTIL OPERATOR] Added lentil_filter automatically to cloned AOV: %s", output_string.c_str());
        // }
        
        AiArraySetStr(outputs, i, AtString(output_string.c_str()));
        

        // link imager to driver (could be kick, or exr_driver, etc)
        // think there's currently a bug with the AI_TYPE_NONE/AI_TYPE_NODE in lentil_thin_lens_bokeh_imager.cpp
        // NODE is necessary to link the input param from this operator
        // but when it's set to NODE it's not picked up autoatically (need to set it from kick)
        // std::string driver_str = split_str(output_string, std::string(" ")).back();
        // AtNode* driver = AiNodeLookUpByName(driver_str.c_str());
        // if (!AiNodeIsLinked(driver, "input")){
        //     // differentiate between kick & exr drivers
        //     if (AtString(AiNodeEntryGetName(AiNodeGetNodeEntry(driver))) == AtString("driver_kick")){
        //         AiNodeLink(lentil_imager_kick, "input", driver);
        //         AiMsgInfo("[LENTIL OPERATOR] Linked lentil_imager_kick to driver: %s", AiNodeEntryGetName(AiNodeGetNodeEntry(driver)));
        //     } else {
        //         AiNodeLink(lentil_imager_exr, "input", driver);
        //         AiMsgInfo("[LENTIL OPERATOR] Linked lentil_imager_exr to driver: %s", AiNodeEntryGetName(AiNodeGetNodeEntry(driver)));
        //     } 
        // }
    }

    // AiASSWrite("/home/cactus/lentil/pota/tests/imagers/test_imagers_02_operated.ass", AI_NODE_ALL, false); 
    
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

node_loader
{
    if (i>0) return 0;
    node->methods = (AtNodeMethods*)LentilOperatorMtd;
    node->name = "lentil_operator";
    node->node_type = AI_NODE_OPERATOR;
    strcpy(node->version, AI_VERSION);
    return true;
}