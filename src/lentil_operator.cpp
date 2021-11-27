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
    AtNode *time_write;
    AtNode *time_read;
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


    AtUniverse *uni = AiNodeGetUniverse(op);
    operator_data->camera_node = AiUniverseGetCamera(uni);
    const AtNodeEntry *nentry = AiNodeGetNodeEntry(operator_data->camera_node);
    operator_data->camera_node_type = AtString(AiNodeEntryGetName(nentry));

    operator_data->cook = false;

    if (operator_data->camera_node_type == AtString("lentil_camera")){
        operator_data->filter = AiNode(uni, "lentil_filter", AtString("lentil_replaced_filter"));
        operator_data->time_write = AiNode(uni, "aov_write_float", AtString("lentil_time_write"));
        operator_data->time_read = AiNode(uni, "state_float", AtString("lentil_time_read"));
        
        operator_data->cook = true;
    }
    

    AiNodeSetLocalData(op, operator_data);
    return true;
}

operator_cook
{
    LentilOperatorData* operator_data = (LentilOperatorData*)AiNodeGetLocalData(op);

    if (operator_data->cook == false) {
        AiMsgWarning("[LENTIL OPERATOR] Camera is not of type lentil_camera");
        return false;
    }

    AtUniverse *uni = AiNodeGetUniverse(op);
    AtNode* options = AiUniverseGetOptions(uni);
    AtArray* outputs = AiNodeGetArray(options, "outputs");
    
    std::string lentil_time_string;
    
    const int elements = AiArrayGetNumElements(outputs);
    for (int i=0; i<elements; ++i) {
        std::string output_string = AiArrayGetStr(outputs, i).c_str();
        
        auto [filter_index, output_string_split] = find_filter_index_in_aov_string(output_string, uni);
        std::string filter = output_string_split[filter_index];
        std::string type = output_string_split[filter_index-1];
        std::string name = output_string_split[filter_index-2];

        if (type != "RGBA" && type != "RGB" && type != "FLOAT" && type != "VECTOR") continue;        
        if (name.find("crypto") != std::string::npos) continue;
        
        if (filter != "lentil_replaced_filter") {
            output_string.replace(output_string.find(filter), filter.length(), AiNodeGetStr(operator_data->filter, "name"));        
            AiArraySetStr(outputs, i, AtString(output_string.c_str()));
            AiMsgInfo("[LENTIL OPERATOR] Added lentil_filter automatically to cloned AOV: %s", output_string.c_str());

        }

        // Adds lentil_time aov
        // little mechanism to pick up the first aov to modify, but only add the modified version after iterating over all the aovs
        // this is safer than trying to construct a string myself, because it can come in various formats (camera included, etc)
        if (i == 0) {
            lentil_time_string = output_string;
            lentil_time_string.replace(lentil_time_string.find(name), name.length(), "lentil_time");
            lentil_time_string.replace(lentil_time_string.find(type), type.length(), "FLOAT");
        }
        if (i == elements-1) {
            AiArrayResize(outputs, AiArrayGetNumElements(outputs)+1, 1);
            AiArraySetStr(outputs, i+1, AtString(lentil_time_string.c_str()));
        }
    }

    // set time node params/linking
    AiNodeSetStr(operator_data->time_read, AtString("variable"), AtString("time"));
    AiNodeSetStr(operator_data->time_write, AtString("aov_name"), AtString("lentil_time"));
    AiNodeLink(operator_data->time_read, "aov_input", operator_data->time_write);

    // need to add an entry to the aov_shaders (NODE)
    AtArray* aov_shaders_array = AiNodeGetArray(options, "aov_shaders");
    int aov_shader_array_size = AiArrayGetNumElements(aov_shaders_array);
    AiArrayResize(aov_shaders_array, aov_shader_array_size+1, 1);
    AiArraySetPtr(aov_shaders_array, aov_shader_array_size, (void*)operator_data->time_write);
    AiNodeSetArray(options, "aov_shaders", aov_shaders_array);
    

    // to debug what the operator did
    // AiASSWrite(uni, "/home/cactus/lentil/pota/tests/ilyas_motion_blur_bug/everything.ass", AI_NODE_ALL, 1, 0); 

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