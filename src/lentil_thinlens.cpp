#include <ai.h>
#include "lentil_thinlens.h"

AI_CAMERA_NODE_EXPORT_METHODS(lentil_thinlensMethods)

enum
{
    p_sensor_width,
    p_sensor_height,
    p_focal_length,
    p_fstop,
    p_focus_distance,
    p_minimum_rgb,
    p_bokeh_exr_path,
    p_emperical_ca_dist,
    p_optical_vignetting_distance,
    p_optical_vignetting_radius,
    p_bias,
    p_gain,
    p_invert,
    p_square,
    p_squeeze,
    p_use_image,
    p_bokeh_input_path,
    p_bokeh_samples_mult
};

node_parameters
{
    AiParameterFlt("sensor_width", 3.6); // 35mm film
    AiParameterFlt("sensor_height", 2.4); // 35 mm film
    AiParameterFlt("focal_length", 3.5); // in cm
    AiParameterFlt("fstop", 1.4);
    AiParameterFlt("focus_distance", 100.0);
    AiParameterFlt("minimum_rgb", 0.5);
    AiParameterStr("bokeh_exr_path", "");
    AiParameterFlt("emperical_ca_dist", 0.0);
    AiParameterFlt("optical_vignetting_distance", 0.0);
    AiParameterFlt("optical_vignetting_radius", 1.0);

    AiParameterFlt("bias", 0.5);
    AiParameterFlt("gain", 0.5);
    AiParameterBool("invert", false);

    AiParameterFlt("square", 0.0);
    AiParameterFlt("squeeze", 1.0);

    AiParameterBool("use_image", false);
    AiParameterStr("bokeh_input_path", "");

    AiParameterInt("bokeh_samples_mult", 10);
}


node_initialize
{
    AiCameraInitialize(node);
    AiNodeSetLocalData(node, new CameraThinLens());
}


node_update
{
    CameraThinLens* tl = (CameraThinLens*)AiNodeGetLocalData(node);

    tl->sensor_width = AiNodeGetFlt(node, "sensor_width");
    tl->sensor_height = AiNodeGetFlt(node, "sensor_height");
    tl->focal_length = AiNodeGetFlt(node, "focal_length");
    tl->fstop = AiNodeGetFlt(node, "fstop");
    tl->focus_distance = AiNodeGetFlt(node, "focus_distance");

    tl->fov = 2.0 * atan((tl->sensor_width / (2.0 * tl->focal_length))); // in radians
    tl->tan_fov = tanf(tl->fov / 2.0);
    tl->aperture_radius = (tl->focal_length) / (2.0 * tl->fstop);

    tl->minimum_rgb = AiNodeGetFlt(node, "minimum_rgb");
    tl->bokeh_exr_path = AiNodeGetStr(node, "bokeh_exr_path");

    tl->emperical_ca_dist = AiNodeGetFlt(node, "emperical_ca_dist");
    tl->optical_vignetting_distance = AiNodeGetFlt(node, "optical_vignetting_distance");
    tl->optical_vignetting_radius = AiNodeGetFlt(node, "optical_vignetting_radius");

    tl->bias = AiNodeGetFlt(node, "bias");
    tl->gain = AiNodeGetFlt(node, "gain");
    tl->invert = AiNodeGetBool(node, "invert");

    tl->square = AiNodeGetFlt(node, "square");
    tl->squeeze = AiNodeGetFlt(node, "squeeze");

    tl->use_image = AiNodeGetBool(node, "use_image");
    tl->bokeh_input_path = AiNodeGetStr(node, "bokeh_input_path");

    tl->bokeh_samples_mult = AiNodeGetInt(node, "bokeh_samples_mult");

    // make probability functions of the bokeh image
    // if (parms.bokehChanged(camera->params)) {
        tl->image.invalidate();
        if (tl->use_image && !tl->image.read(tl->bokeh_input_path.c_str())){
        AiMsgError("[LENTIL] Couldn't open bokeh image!");
        AiRenderAbort();
        }
    // }

    AiCameraUpdate(node, false);
}

node_finish
{
    CameraThinLens* tl = (CameraThinLens*)AiNodeGetLocalData(node);
    delete tl;
}


camera_create_ray
{
    CameraThinLens* tl = (CameraThinLens*)AiNodeGetLocalData(node);

    bool success = false;
    int tries = 0;
    int maxtries = 15;
    while (!success && tries <= maxtries){
        // create point on sensor (camera space)
        const AtVector p(input.sx, input.sy, -1.0/tl->tan_fov);
        // calculate direction vector from origin to point on lens
        output.dir = AiV3Normalize(p); // or norm(p-origin)

        // either get uniformly distributed points on the unit disk or bokeh image
        
        Eigen::Vector2d unit_disk(0, 0);
        if (tries == 0) {
            if (tl->use_image) {
                tl->image.bokehSample(input.lensx, input.lensy, unit_disk);
            } else {
                concentricDiskSample(input.lensx, input.lensy, unit_disk, tl->bias, tl->square, tl->squeeze);
            }
        } else {
            float r1 = xor128() / 4294967296.0;
            float r2 = xor128() / 4294967296.0;

            if (tl->use_image) {
                tl->image.bokehSample(r1, r2, unit_disk);
            } else {
                concentricDiskSample(r1, r2, unit_disk, tl->bias, tl->square, tl->squeeze);
            }
        }

        unit_disk(0) *= tl->squeeze;
        unit_disk *= -1.0;

        // tmp copy
        AtVector2 lens(unit_disk(0), unit_disk(1));

        // scale points in [-1, 1] domain to actual aperture radius
        lens *= tl->aperture_radius;

        // new origin is these points on the lens
        output.origin.x = lens.x;
        output.origin.y = lens.y;
        output.origin.z = 0.0;


        // Compute point on plane of focus, intersection on z axis
        const float intersection = std::abs(tl->focus_distance / output.dir.z); // or tl->focus_distance; (spherical/plane, test!)
        const AtVector focusPoint = output.dir * intersection;
        output.dir = AiV3Normalize(focusPoint - output.origin);

        if (tl->optical_vignetting_distance > 0.0){
            if (!empericalOpticalVignetting(output.origin, output.dir, tl->aperture_radius, tl->optical_vignetting_radius, tl->optical_vignetting_distance)){
                ++tries;
                continue;
            }
        }


        // ca .. not sure about this technique, test on a focused scene.
        // can't have this shift to the green aperture when everything is focused.
        // AtRGB weight = AI_RGB_WHITE;
        // if (tl->emperical_ca_dist > 0.0){
        //     const AtVector2 p2(p.x, p.y);
        //     const float distance_to_center = AiV2Dist(AtVector2(0.0, 0.0), p2);
        //     const int random_aperture = static_cast<int>(std::floor((xor128() / 4294967296.0) * 3.0));
        //     AtVector2 aperture_0_center(0.0, 0.0);
        //     AtVector2 aperture_1_center(- p2 * distance_to_center * tl->emperical_ca_dist);
        //     AtVector2 aperture_2_center(p2 * distance_to_center * tl->emperical_ca_dist);
            

        //     if (random_aperture == 1)      lens += aperture_1_center;
        //     else if (random_aperture == 2) lens += aperture_2_center;

        //     if (std::pow(lens.x-aperture_1_center.x, 2) + std::pow(lens.y - aperture_1_center.y, 2) > std::pow(tl->aperture_radius, 2)) {
        //         weight.r = 0.0;
        //     }
        //     if (std::pow(lens.x-aperture_0_center.x, 2) + std::pow(lens.y - aperture_0_center.y, 2) > std::pow(tl->aperture_radius, 2)) {
        //         weight.b = 0.0;
        //     }
        //     if (std::pow(lens.x-aperture_2_center.x, 2) + std::pow(lens.y - aperture_2_center.y, 2) > std::pow(tl->aperture_radius, 2)) {
        //         weight.g = 0.0;
        //     }

        //     if (weight == AI_RGB_ZERO){
        //         ++tries;
        //         continue;
        //     }
        
        //     //ca, not sure if this should be done, evens out the intensity?
        //     // float sum = (output.weight.r + output.weight.g + output.weight.b) / 3.0;
        //     // output.weight.r /= sum;
        //     // output.weight.g /= sum;
        //     // output.weight.b /= sum;
        // }
        
        // output.weight = weight;

        // this will fuck up all kinds of optimisations, calculate proper derivs!
        output.dOdx = output.origin;
        output.dOdy = output.origin;
        output.dDdx = output.dir;
        output.dDdy = output.dir;

        success = true;
    }
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