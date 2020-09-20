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
    p_proper_ray_derivativesTL,

    p_abb_comaTL
};

node_parameters
{
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
    AiParameterBool("proper_ray_derivativesTL", true);

    // experimental
    AiParameterFlt("abb_comaTL", 0.0);
    
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

    // this pointer could be buggy (untested)
    AtNode* cameranode = node;
    #include "node_update_thinlens.h"
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
    AtRGB weight (0, 0, 0);
    
    trace_ray_fw_thinlens(true, tries, input.sx, input.sy, input.lensx, input.lensy, origin, dir, weight, r1, r2, tl);

    
    if (tries > 0){
        if (!tl->proper_ray_derivatives) {
            output.dOdx = origin;
            output.dOdy = origin;
            output.dDdx = dir;
            output.dDdy = dir;
        } else { // calculating the derivative rays here

            float step = 0.001;
            AtCameraInput input_dx = input;
            AtCameraInput input_dy = input;
            AtCameraOutput output_dx;
            AtCameraOutput output_dy;

            input_dx.sx += input.dsx * step;
            input_dy.sy += input.dsy * step;

            trace_ray_fw_thinlens(false, tries, input_dx.sx, input_dx.sy, r1, r2, output_dx.origin, output_dx.dir, output_dx.weight, r1, r2, tl);
            trace_ray_fw_thinlens(false, tries, input_dy.sx, input_dy.sy, r1, r2, output_dy.origin, output_dy.dir, output_dy.weight, r1, r2, tl);

            output.dOdx = (output_dx.origin - origin) / step;
            output.dOdy = (output_dy.origin - origin) / step;
            output.dDdx = (output_dx.dir - dir) / step;
            output.dDdy = (output_dy.dir - dir) / step;
        }
    }


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