#include <string>
#include <vector>
#include <cstdlib>

#include <ai.h>
#include <stdio.h>

#include "global.h"

// adds the correct filter for all aov's

AI_OPERATOR_NODE_EXPORT_METHODS(LentilOperatorMtd);

// struct LentilOperatorData
// {
//     AtNode *filter;
// };

node_parameters 
{
    AiMetaDataSetBool(nentry, nullptr, "force_update", true);
}

operator_init
{
    // LentilOperatorData* operator_data = new LentilOperatorData();
    AtUniverse *universe = AiNodeGetUniverse(op);

    bool cam_is_lentil = true;
    AtNode *camera_node = AiUniverseGetCamera(universe);
    AtNodeEntry *camera_ne = AiNodeGetNodeEntry(camera_node);
    if (AiNodeEntryGetNameAtString != AtString("lentil_camera")) cam_is_lentil = false;

    if (cam_is_lentil) {
        Camera *camera_data = (Camera*)AiNodeGetLocalData(camera_node);
        camera_data->setup_all(universe);
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
    // LentilOperatorData* operator_data = (LentilOperatorData*)AiNodeGetLocalData(op);
    // delete operator_data;
    return true;
}



void registerLentilOperator(AtNodeLib* node) {
    node->methods = (AtNodeMethods*)LentilOperatorMtd;
    node->name = "lentil_operator";
    node->node_type = AI_NODE_OPERATOR;
    strcpy(node->version, AI_VERSION);
}