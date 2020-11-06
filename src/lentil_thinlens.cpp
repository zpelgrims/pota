#include <ai.h>
#include "lentil_thinlens.h"

AI_CAMERA_NODE_EXPORT_METHODS(lentil_thinlensMethods)

enum
{
    p_sensor_widthTL,
    p_focal_lengthTL,
    p_fstopTL,
    p_focus_distanceTL,
    p_enable_dof,
    // p_emperical_ca_distTL,
    p_optical_vignetting_distanceTL,
    p_optical_vignetting_radiusTL,

    p_abb_sphericalTL,
    p_abb_distortionTL,
    

    p_bokeh_aperture_bladesTL,
    p_bokeh_circle_to_squareTL,
    p_bokeh_anamorphicTL,

    p_bokeh_enable_imageTL,
    p_bokeh_image_pathTL,

    p_bidir_min_luminanceTL,
    p_bidir_sample_multTL,

    p_bidir_add_luminanceTL,
    p_bidir_add_luminance_transitionTL,

    p_vignetting_retriesTL,

    p_abb_comaTL
};


static const char* Units[] = {"mm", "cm", "dm", "m", NULL};


node_parameters
{
    AiParameterEnum("unitsTL", cm, Units);

    AiParameterFlt("sensor_widthTL", 36.0); // 35mm film
    AiParameterFlt("focal_lengthTL", 35.0); // in mm
    AiParameterFlt("fstopTL", 1.4);
    AiParameterFlt("focus_distanceTL", 100.0); // in cm
    AiParameterBool("enable_dofTL", true);

    // AiParameterFlt("emperical_ca_distTL", 0.0);
    AiParameterFlt("optical_vignetting_distanceTL", 0.0);
    AiParameterFlt("optical_vignetting_radiusTL", 2.0);

    AiParameterFlt("abb_sphericalTL", 0.5);
    AiParameterFlt("abb_distortionTL", 0.0);

    AiParameterInt("bokeh_aperture_bladesTL", 0);
    AiParameterFlt("bokeh_circle_to_squareTL", 0.0);
    AiParameterFlt("bokeh_anamorphicTL", 1.0);
    AiParameterBool("bokeh_enable_imageTL", false);
    AiParameterStr("bokeh_image_pathTL", "");

    AiParameterFlt("bidir_min_luminanceTL", 1.0);
    AiParameterInt("bidir_sample_multTL", 20);
    AiParameterFlt("bidir_add_luminanceTL", 0.0);
    AiParameterFlt("bidir_add_luminance_transitionTL", 1.0);

    AiParameterInt("vignetting_retriesTL", 30);

    // experimental
    AiParameterFlt("abb_comaTL", 0.0);
    
    AiMetaDataSetBool(nentry, nullptr, "force_update", true);
}


node_initialize
{
    AiCameraInitialize(node);
    AiNodeSetLocalData(node, new CameraThinLens());
}


node_update
{
    AiCameraUpdate(node, false);
    CameraThinLens* tl = (CameraThinLens*)AiNodeGetLocalData(node);

    // try to force recomputation of the operator, if i don't write some data to it, it only runs on scene init
    AtNode *operator_node = (AtNode*)AiNodeGetPtr(AiUniverseGetOptions(), "operator");
    if (operator_node != nullptr){
        if (AiNodeIs(operator_node, AtString("lentil_operator"))){
            AiNodeSetInt(operator_node, "call_me_dirty", rand());
        }
    }

    // this pointer could be buggy (untested)
    AtNode* cameranode = node;
    #include "node_update_thinlens.h"

    // make probability functions of the bokeh image
    // if (parms.bokehChanged(camera->params)) {
    tl->image.invalidate();
    if (tl->bokeh_enable_image && !tl->image.read(tl->bokeh_image_path.c_str())){
    AiMsgError("[LENTIL CAMERA TL] Couldn't open bokeh image!");
    AiRenderAbort();
    }
    // }
}

node_finish
{
    CameraThinLens* tl = (CameraThinLens*)AiNodeGetLocalData(node);
    delete tl;
}



camera_create_ray
{
    CameraThinLens* tl = (CameraThinLens*)AiNodeGetLocalData(node);

    
    int tries = 0;
    float r1 = 0.0, r2 = 0.0;
    AtVector origin (0, 0, 0);
    AtVector dir (0, 0, 0);
    AtRGB weight (1, 1, 1);
    
    trace_ray_fw_thinlens(true, tries, input.sx, input.sy, input.lensx, input.lensy, origin, dir, weight, r1, r2, tl);

    output.origin = origin;
    output.dir = dir;
    output.weight = weight;
    
}


camera_reverse_ray
{
    const CameraThinLens* tl = (CameraThinLens*)AiNodeGetLocalData(node);

    double coeff = 1.0 / std::max(std::abs(Po.z * tl->tan_fov), 1e-3f);
    Ps.x = Po.x * coeff;
    Ps.y = Po.y * coeff;

    return true;
}

node_loader
{
    if (i != 0) return false;
    node->methods = lentil_thinlensMethods;
    node->output_type = AI_TYPE_UNDEFINED;
    node->name = "lentil_thinlens";
    node->node_type = AI_NODE_CAMERA;
    strcpy(node->version, AI_VERSION);
    return true;
}