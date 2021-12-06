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
}

operator_init
{
    LentilOperatorData* operator_data = new LentilOperatorData();

    operator_data->cook = false;

     // lentil aov setup
    const AtNodeEntry *crypto_ne = AiNodeEntryLookUp(AtString("cryptomatte"));
    if (AiNodeEntryGetCount(crypto_ne) == 0) {
        operator_data->cook = true;
    }

    AtUniverse *universe = AiNodeGetUniverse(op);

    if (operator_data->cook){
        bool lentil_filter_found = false;
        AtNode *filternode = nullptr;
        if (AiNodeEntryGetCount(AiNodeEntryLookUp("lentil_filter")) == 0){
            filternode = AiNode(universe, "lentil_filter", AtString("lentil_replaced_filter"));
        } else {
            lentil_filter_found = true;
        }

        AtArray* outputs = AiNodeGetArray(AiUniverseGetOptions(universe), "outputs");
        std::vector<std::string> output_strings;
        bool lentil_time_found = false;

        const int elements = AiArrayGetNumElements(outputs);
        for (int i=0; i<elements; i++) {
            
            std::string output_string = AiArrayGetStr(outputs, i).c_str();
            
            auto [filter_index, output_string_split] = find_filter_index_in_aov_string(output_string, universe);
            std::string filter = output_string_split[filter_index];
            std::string type = output_string_split[filter_index-1];
            std::string name = output_string_split[filter_index-2];

            if (name == "lentil_time"){
                lentil_time_found = true;
                output_strings.push_back(output_string);
                continue;
            }

            // lentil unsupported
            if (type != "RGBA" && type != "RGB" && type != "FLOAT" && type != "VECTOR") {
                output_strings.push_back(output_string);
                continue;
            }

            if (name.find("crypto_") != std::string::npos){
                output_strings.push_back(output_string);
                continue;
            }
            
            if (filter != "lentil_replaced_filter") {
                output_string.replace(output_string.find(filter), filter.length(), AiNodeGetStr(filternode, "name"));        
            }
            
            output_strings.push_back(output_string);
        }
        

        if (!lentil_time_found) {
            std::string tmp_first_aov = output_strings[0];
            auto [filter_index, output_string_split] = find_filter_index_in_aov_string(tmp_first_aov, universe);
            std::string type = output_string_split[filter_index-1];
            std::string name = output_string_split[filter_index-2];
            tmp_first_aov.replace(tmp_first_aov.find(name), name.length(), "lentil_time");
            tmp_first_aov.replace(tmp_first_aov.find(type), type.length(), "FLOAT");
            output_strings.push_back(tmp_first_aov);
        }

        AtArray *final_outputs = AiArrayAllocate(output_strings.size(), 1, AI_TYPE_STRING);
        uint32_t i = 0;
        for (auto &output : output_strings){
            AiArraySetStr(final_outputs, i++, output.c_str());
            auto [filter_index, output_string_split] = find_filter_index_in_aov_string(output, universe);
            std::string type = output_string_split[filter_index-1];
            AiAOVRegister(output.c_str(), string_to_arnold_type(type), AI_AOV_BLEND_NONE); // watch out for type here!!
        }
        AiNodeSetArray(AiUniverseGetOptions(universe), "outputs", final_outputs);



        // need to add an entry to the aov_shaders (NODE)
        AtArray* aov_shaders_array = AiNodeGetArray(AiUniverseGetOptions(universe), "aov_shaders");
        int aov_shader_array_size = AiArrayGetNumElements(aov_shaders_array);

        if (!lentil_filter_found){
            AtNode *time_write = AiNode(universe, "aov_write_float", AtString("lentil_time_write"));
            AtNode *time_read = AiNode(universe, "state_float", AtString("lentil_time_read"));

            // set time node params/linking
            AiNodeSetStr(time_read, AtString("variable"), AtString("time"));
            AiNodeSetStr(time_write, AtString("aov_name"), AtString("lentil_time"));
            AiNodeLink(time_read, "aov_input", time_write);

            AiArrayResize(aov_shaders_array, aov_shader_array_size+1, 1);
            AiArraySetPtr(aov_shaders_array, aov_shader_array_size, (void*)time_write);
            AiNodeSetArray(AiUniverseGetOptions(universe), "aov_shaders", aov_shaders_array);
        }
    }




    // to debug what the operator did
    // AiASSWrite(uni, "/home/cactus/lentil/pota/tests/ilyas_motion_blur_bug/everything.ass", AI_NODE_ALL, 1, 0); 

    AiNodeSetLocalData(op, operator_data);
    return true;
}

operator_cook
{
    return true;
}

operator_post_cook
{
    return true;
}

operator_cleanup
{
    LentilOperatorData* operator_data = (LentilOperatorData*)AiNodeGetLocalData(op);
    delete operator_data;
    return true;
}



void registerLentilOperator(AtNodeLib* node) {
    node->methods = (AtNodeMethods*)LentilOperatorMtd;
    node->name = "lentil_operator";
    node->node_type = AI_NODE_OPERATOR;
    strcpy(node->version, AI_VERSION);
}