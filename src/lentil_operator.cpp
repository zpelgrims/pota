#include <string>
#include <vector>
#include <cstdlib>

#include <ai.h>
#include <stdio.h>

#include "global.h"

// Duplicates all AOVs and changing the driver to the appropriate lentil driver at rendertime


AI_OPERATOR_NODE_EXPORT_METHODS(LentilOperatorMtd);

struct OpData
{
    AtNode *driver;
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
        data->driver = AiNode("lentil_thin_lens_bokeh_driver", AtString("lentil_driver"));
        data->cook = true;
    } else if (data->camera_node_type == AtString("lentil")){
        data->driver = AiNode("lentil_bokeh_driver", AtString("lentil_driver"));
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
    
    AtNode* options = AiUniverseGetOptions();
    AtArray* outputs = AiNodeGetArray(options, "outputs");

    int offset = 0;
    int elements = AiArrayGetNumElements(outputs);

    AiArrayResize(outputs, 2 * elements, 0);
    offset = elements;

    for (int i=0; i<elements; ++i) {
        std::string output_string = AiArrayGetStr(outputs, i).c_str();
        std::string driver = split_str(output_string, std::string(" ")).back();
        output_string.replace(output_string.find(driver), driver.length(), AiNodeGetStr(data->driver, "name"));

        AiArraySetStr(outputs, i + offset, AtString(output_string.c_str()));
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