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

// should also set the imager

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

    AtNode* lentil_imager = AiNode("lentil_thin_lens_bokeh_imager", AtString("lentil_imager"));
    
    AtNode* options = AiUniverseGetOptions();
    AtArray* outputs = AiNodeGetArray(options, "outputs");

    for (int i=0; i<AiArrayGetNumElements(outputs); ++i) {
        std::string output_string = AiArrayGetStr(outputs, i).c_str();
        std::string filter = split_str(output_string, std::string(" ")).end()[-2]; // one before last, which is the filter
        output_string.replace(output_string.find(filter), filter.length(), AiNodeGetStr(data->filter, "name"));
        AiArraySetStr(outputs, i, AtString(output_string.c_str()));
        AiMsgInfo("[LENTIL] Automatically inserted AOV with correct filter type: %s", output_string.c_str());

        // link imager to driver (could be kick, or exr_driver, etc)
        std::string driver_str = split_str(output_string, std::string(" ")).back();
        AtNode* driver = AiNodeLookUpByName(driver_str.c_str());
        if (!AiNodeIsLinked(driver, "input")){
            AiNodeLink(lentil_imager, "input", driver);
            AiMsgInfo("[LENTIL] Linked lentil_imager to driver: %s", AiNodeGetName(driver));
        }
    }
    
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