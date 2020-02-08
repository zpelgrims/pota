#include <ai.h>
#include "lentil_thinlens.h"

AI_CAMERA_NODE_EXPORT_METHODS(lentil_thinlensMethods)

enum
{
    p_sensor_widthTL,
    p_focal_lengthTL,
    p_fstopTL,
    p_focus_distanceTL,
    // p_emperical_ca_distTL,
    p_optical_vignetting_distanceTL,
    p_optical_vignetting_radiusTL,

    p_abb_sphericalTL,
    // p_abb_comaTL,

    p_bokeh_circle_to_squareTL,
    p_bokeh_anamorphicTL,

    p_bokeh_enable_imageTL,
    p_bokeh_image_pathTL,

    p_bidir_min_luminanceTL,
    p_bidir_output_pathTL,
    p_bidir_sample_multTL,

    p_bidir_add_luminanceTL,
    p_bidir_add_luminance_transitionTL,

    p_vignetting_retriesTL,
    p_proper_ray_derivativesTL
};

node_parameters
{
    AiParameterFlt("sensor_widthTL", 3.6); // 35mm film
    AiParameterFlt("focal_lengthTL", 3.5); // in cm
    AiParameterFlt("fstopTL", 1.4);
    AiParameterFlt("focus_distanceTL", 100.0);

    // AiParameterFlt("emperical_ca_distTL", 0.0);
    AiParameterFlt("optical_vignetting_distanceTL", 0.0);
    AiParameterFlt("optical_vignetting_radiusTL", 2.0);

    AiParameterFlt("abb_sphericalTL", 0.5);
    // AiParameterFlt("abb_comaTL", 0.1);

    AiParameterFlt("bokeh_circle_to_squareTL", 0.0);
    AiParameterFlt("bokeh_anamorphicTL", 1.0);
    AiParameterBool("bokeh_enable_imageTL", false);
    AiParameterStr("bokeh_image_pathTL", "");

    AiParameterFlt("bidir_min_luminanceTL", 0.2);
    AiParameterStr("bidir_output_pathTL", "");
    AiParameterInt("bidir_sample_multTL", 10);
    AiParameterFlt("bidir_add_luminanceTL", 0.0);
    AiParameterFlt("bidir_add_luminance_transitionTL", 1.0);

    AiParameterInt("vignetting_retriesTL", 30);
    AiParameterBool("proper_ray_derivativesTL", true);
    
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

    
    // THIS IS DOUBLE CODE, also in driver!
    tl->sensor_width = AiNodeGetFlt(node, "sensor_widthTL");
    tl->focal_length = AiNodeGetFlt(node, "focal_lengthTL");
    tl->focal_length = clamp_min(tl->focal_length, 0.01);

    tl->fstop = AiNodeGetFlt(node, "fstopTL");
    tl->fstop = clamp_min(tl->fstop, 0.01);

    tl->focus_distance = AiNodeGetFlt(node, "focus_distanceTL");

    tl->fov = ((tl->sensor_width*0.5));
    tl->aperture_radius = (tl->focal_length) / (2.0 * tl->fstop);

    tl->bidir_min_luminance = AiNodeGetFlt(node, "bidir_min_luminanceTL");
    tl->bidir_output_path = AiNodeGetStr(node, "bidir_output_pathTL");

    // tl->emperical_ca_dist = AiNodeGetFlt(node, "emperical_ca_distTL");
    tl->optical_vignetting_distance = AiNodeGetFlt(node, "optical_vignetting_distanceTL");
    tl->optical_vignetting_radius = AiNodeGetFlt(node, "optical_vignetting_radiusTL");

    tl->abb_spherical = AiNodeGetFlt(node, "abb_sphericalTL");
    tl->abb_spherical = clamp(tl->abb_spherical, 0.001, 0.999);

    // tl->abb_coma = AiNodeGetFlt(node, "abb_comaTL");

    tl->circle_to_square = AiNodeGetFlt(node, "bokeh_circle_to_squareTL");
    tl->circle_to_square = clamp(tl->circle_to_square, 0.01, 0.99);
    tl->bokeh_anamorphic = AiNodeGetFlt(node, "bokeh_anamorphicTL");
    tl->bokeh_anamorphic = clamp(tl->bokeh_anamorphic, 0.01, 99999.0);

    tl->bokeh_enable_image = AiNodeGetBool(node, "bokeh_enable_imageTL");
    tl->bokeh_image_path = AiNodeGetStr(node, "bokeh_image_pathTL");

    tl->bidir_sample_mult = AiNodeGetInt(node, "bidir_sample_multTL");

    tl->bidir_add_luminance = AiNodeGetFlt(node, "bidir_add_luminanceTL");
    tl->bidir_add_luminance_transition = AiNodeGetFlt(node, "bidir_add_luminance_transitionTL");

    tl->vignetting_retries = AiNodeGetInt(node, "vignetting_retriesTL");
    tl->proper_ray_derivatives = AiNodeGetBool(node, "proper_ray_derivativesTL");

    // make probability functions of the bokeh image
    // if (parms.bokehChanged(camera->params)) {
        tl->image.invalidate();
        if (tl->bokeh_enable_image && !tl->image.read(tl->bokeh_image_path.c_str())){
        AiMsgError("[LENTIL] Couldn't open bokeh image!");
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
    // const CameraThinLens* data = (CameraThinLens*)AiNodeGetLocalData(node);
    return false;
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