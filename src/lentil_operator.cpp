#include <string>
#include <vector>
#include <cstdlib>

#include <ai.h>
#include <stdio.h>

#include "global.h"

// Duplicates all AOVs and changing the filter to the appropriate lentil filter at rendertime
// Doing it this way allows me to specify that all AOVs need to be connected to a singular filter node to avoid re-computation


// needs to be updated for filter usage, now it replaces the driver
// might not need to have duplicate aovs, just swap filter.

// I need two different imagers, one for each output (kick_driver, exr_driver, otherwise it doesn't work)

AI_OPERATOR_NODE_EXPORT_METHODS(LentilOperatorMtd);

struct OpData
{
    AtNode *filter;
    AtNode *camera_node;
    AtString camera_node_type;
    bool cook;
};

node_parameters {}

operator_init
{
    OpData* data = (OpData*)AiMalloc(sizeof(OpData));

    data->camera_node = AiUniverseGetCamera();
    const AtNodeEntry *nentry = AiNodeGetNodeEntry(data->camera_node);
    data->camera_node_type = AtString(AiNodeEntryGetName(nentry));

    data->cook = false;

    if (data->camera_node_type == AtString("lentil_thinlens")){
        data->filter = AiNode("lentil_thin_lens_bokeh_filter", AtString("lentil_filter"));
        data->cook = true;
    } else if (data->camera_node_type == AtString("lentil")){
        data->filter = AiNode("lentil_bokeh_filter", AtString("lentil_filter"));
        data->cook = true;
    }

    AiNodeSetLocalData(op, data);  
    return true;
}

operator_cook
{
    OpData* data = (OpData*)AiNodeGetLocalData(op);

    if (data->cook == false) return false;

    const AtNodeEntry *ne_filterdebug = AiNodeEntryLookUp("lentil_filter_debug_operator");
    if (AiNodeEntryGetCount(ne_filterdebug) != 0) return false;

    AtNode* lentil_imager_exr = AiNode("lentil_thin_lens_bokeh_imager", AtString("lentil_imager_exr"));
    AtNode* lentil_imager_kick = AiNode("lentil_thin_lens_bokeh_imager", AtString("lentil_imager_kick"));
    
    AtNode* options = AiUniverseGetOptions();
    AtArray* outputs = AiNodeGetArray(options, "outputs");

    for (int i=0; i<AiArrayGetNumElements(outputs); ++i) {
        std::string output_string = AiArrayGetStr(outputs, i).c_str();
        std::string filter = split_str(output_string, std::string(" ")).end()[-2]; // one before last, which is the filter
        output_string.replace(output_string.find(filter), filter.length(), AiNodeGetStr(data->filter, "name"));
        AiArraySetStr(outputs, i, AtString(output_string.c_str()));
        AiMsgInfo("[LENTIL] Swapped filter type: %s", output_string.c_str());

        // link imager to driver (could be kick, or exr_driver, etc)
        // think there's currently a bug with the AI_TYPE_NONE/AI_TYPE_NODE in lentil_thin_lens_bokeh_imager.cpp
        // NODE is necessary to link the input param from this operator
        // but when it's set to NODE it's not picked up autoatically (need to set it from kick)
        std::string driver_str = split_str(output_string, std::string(" ")).back();
        AtNode* driver = AiNodeLookUpByName(driver_str.c_str());
        if (!AiNodeIsLinked(driver, "input")){
            // differentiate between kick & exr drivers
            if (AtString(AiNodeEntryGetName(AiNodeGetNodeEntry(driver))) == AtString("driver_kick")){
                AiNodeLink(lentil_imager_kick, "input", driver);
                AiMsgInfo("[LENTIL] Linked lentil_imager_kick to driver: %s", AiNodeEntryGetName(AiNodeGetNodeEntry(driver)));
            } else {
                AiNodeLink(lentil_imager_exr, "input", driver);
                AiMsgInfo("[LENTIL] Linked lentil_imager_exr to driver: %s", AiNodeEntryGetName(AiNodeGetNodeEntry(driver)));
            } 
        }
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
    OpData* data = (OpData*)AiNodeGetLocalData(op);
    AiFree(data);
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