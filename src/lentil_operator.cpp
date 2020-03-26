#include <string>
#include <vector>
#include <cstdlib>

#include <ai.h>
#include <stdio.h>


#define operator_post_cook \
static bool OperatorPostCook(AtNode* op, void* user_data)


AI_OPERATOR_NODE_EXPORT_METHODS(LentilOperatorMtd);



std::vector<std::string> split_str(std::string str, std::string token)
{
    std::vector<std::string>result;
    while(str.size())
    {
        int index = static_cast<int>(str.find(token));
        
        if(index != std::string::npos)
        {
            result.push_back(str.substr(0, index));
            str = str.substr(index+token.size());
            
            if(str.size() == 0)
                result.push_back(str);
        }
        else
        {
            result.push_back(str);
            str = "";
        }
    }
    return result;
}

struct OpData
{
    AtNode *driver;
    AtNode *camera_node;
    AtString camera_node_type;
    AtString lentil_thinlens_string;
    AtString lentil_po_string;
    bool shouldcook;
};

node_parameters {}

operator_init
{
    OpData* data = (OpData*)AiMalloc(sizeof(OpData));

    data->camera_node = AiUniverseGetCamera();
    const AtNodeEntry *nentry = AiNodeGetNodeEntry(data->camera_node);
    data->camera_node_type = AtString(AiNodeEntryGetName(nentry));

    data->lentil_thinlens_string = AtString("lentil_thinlens");
    data->lentil_po_string = AtString("lentil");

    data->shouldcook = false;

    if (data->camera_node_type == data->lentil_thinlens_string){
        data->driver = AiNode("lentil_thin_lens_bokeh_driver", AtString("lentil_driver"));
        data->shouldcook = true;
    } else if (data->camera_node_type == data->lentil_po_string){
        data->driver = AiNode("lentil_bokeh_driver", AtString("lentil_driver"));
        data->shouldcook = true;
    }

    AiNodeSetLocalData(op, data);    
    return true;
}

operator_cook
{
    OpData* data = (OpData*)AiNodeGetLocalData(op);

    if (data->shouldcook == false) return false;

    AtNode* options = AiUniverseGetOptions();
    AtArray* outputs = AiNodeGetArray(options, "outputs");

    int offset = 0;
    int elements = AiArrayGetNumElements(outputs);
    
    AiArrayResize(outputs, 2 * elements, 0);
    offset = elements;

    for (int i=0; i<elements; ++i) {
        std::string output_string = AiArrayGetStr(outputs, i).c_str();
        std::string name = split_str(output_string, std::string(" ")).back();
        output_string.replace(output_string.find(name), name.length(), AiNodeGetStr(data->driver, "name"));

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