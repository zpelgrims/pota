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
    p_chr_abb_mult,
    p_optical_vignetting_distance,
    p_optical_vignetting_radius,
    p_bias,
    p_gain,
    p_invert
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
    AiParameterFlt("chr_abb_mult", 0.0);
    AiParameterFlt("optical_vignetting_distance", 0.0);
    AiParameterFlt("optical_vignetting_radius", 1.0);

    AiParameterFlt("bias", 0.5);
    AiParameterFlt("gain", 0.5);
    AiParameterBool("invert", false);
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

    tl->chr_abb_mult = AiNodeGetFlt(node, "chr_abb_mult");
    tl->optical_vignetting_distance = AiNodeGetFlt(node, "optical_vignetting_distance");
    tl->optical_vignetting_radius = AiNodeGetFlt(node, "optical_vignetting_radius");

    tl->bias = AiNodeGetFlt(node, "bias");
    tl->gain = AiNodeGetFlt(node, "gain");
    tl->invert = AiNodeGetBool(node, "invert");

    AiCameraUpdate(node, false);
}

node_finish
{
    CameraThinLens* tl = (CameraThinLens*)AiNodeGetLocalData(node);
    delete tl;
}


camera_create_ray
{
    const CameraThinLens* tl = (CameraThinLens*)AiNodeGetLocalData(node);

    bool success = false;
    int tries = 0;
    int maxtries = 15;
    while (!success && tries <= maxtries){
        // create point on sensor (camera space)
        const AtVector p(input.sx * tl->tan_fov, input.sy * tl->tan_fov, -1.0);
        // calculate direction vector from origin to point on lens
        output.dir = AiV3Normalize(p); // or norm(p-origin)

        // either get uniformly distributed points on the unit disk or bokeh image
        AtVector2 lens(0.0, 0.0);
        if (tries == 0) concentricDiskSample(input.lensx, input.lensy, &lens, tl->bias);
        else concentricDiskSample(xor128() / 4294967296.0, xor128() / 4294967296.0, &lens, tl->bias);

        // scale points in [-1, 1] domain to actual aperture radius
        lens *= tl->aperture_radius;

        // ca
        const AtVector2 p2(p.x, p.y);
        const float distance_to_center = AiV2Dist(AtVector2(0.0, 0.0), p2);
        const int random_aperture = static_cast<int>(std::floor((xor128() / 4294967296.0) * 3.0));
        AtVector2 aperture_0_center(0.0, 0.0);
        AtVector2 aperture_1_center(- p2 * distance_to_center * tl->chr_abb_mult);
        AtVector2 aperture_2_center(p2 * distance_to_center * tl->chr_abb_mult);
        output.weight = AtRGB(1.0);
        if (random_aperture == 0) {
            if (std::pow(lens.x-aperture_1_center.x, 2) + std::pow(lens.y - aperture_1_center.y, 2) > std::pow(tl->aperture_radius, 2)) {
                output.weight.r = 0.0;
            }
            if (std::pow(lens.x-aperture_0_center.x, 2) + std::pow(lens.y - aperture_0_center.y, 2) > std::pow(tl->aperture_radius, 2)) {
                output.weight.b = 0.0;
            }
            if (std::pow(lens.x-aperture_2_center.x, 2) + std::pow(lens.y - aperture_2_center.y, 2) > std::pow(tl->aperture_radius, 2)) {
                output.weight.g = 0.0;
            }
        } else if (random_aperture == 1) {
            lens += aperture_1_center;
            if (std::pow(lens.x-aperture_1_center.x, 2) + std::pow(lens.y - aperture_1_center.y, 2) > std::pow(tl->aperture_radius, 2)) {
                output.weight.r = 0.0;
            }
            if (std::pow(lens.x-aperture_0_center.x, 2) + std::pow(lens.y - aperture_0_center.y, 2) > std::pow(tl->aperture_radius, 2)) {
                output.weight.b = 0.0;
            }
            if (std::pow(lens.x-aperture_2_center.x, 2) + std::pow(lens.y - aperture_2_center.y, 2) > std::pow(tl->aperture_radius, 2)) {
                output.weight.g = 0.0;
            }
        } else if (random_aperture == 2) {
            lens += aperture_2_center;
            if (std::pow(lens.x-aperture_1_center.x, 2) + std::pow(lens.y - aperture_1_center.y, 2) > std::pow(tl->aperture_radius, 2)) {
                output.weight.r = 0.0;
            }
            if (std::pow(lens.x-aperture_0_center.x, 2) + std::pow(lens.y - aperture_0_center.y, 2) > std::pow(tl->aperture_radius, 2)) {
                output.weight.b = 0.0;
            }
            if (std::pow(lens.x-aperture_2_center.x, 2) + std::pow(lens.y - aperture_2_center.y, 2) > std::pow(tl->aperture_radius, 2)) {
                output.weight.g = 0.0;
            }
        }
        //ca, not sure if this should be done, evens out the intensity
        // float sum = (output.weight.r + output.weight.g + output.weight.b) / 3.0;
        // output.weight.r /= sum;
        // output.weight.g /= sum;
        // output.weight.b /= sum;

        // new origin is these points on the lens
        output.origin.x = lens.x;
        output.origin.y = lens.y;
        output.origin.z = 0.0;

        // Compute point on plane of focus, intersection on z axis
        const float intersection = std::abs(tl->focus_distance / output.dir.z);
        const AtVector focusPoint = output.dir * intersection;
        output.dir = AiV3Normalize(focusPoint - output.origin);

        if (tl->optical_vignetting_distance > 0.0){
            if (!empericalOpticalVignetting(output.origin, output.dir, tl->aperture_radius, tl->optical_vignetting_radius, tl->optical_vignetting_distance)){
                ++tries;
                continue;
            }
        }

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