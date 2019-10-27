#include <ai.h>
#include "lentil_thinlens.h"

AI_CAMERA_NODE_EXPORT_METHODS(lentil_thinlensMethods)

enum
{
    p_sensor_widthTL,
    p_focal_lengthTL,
    p_fstopTL,
    p_focus_distanceTL,
    p_minimum_rgbTL,
    p_bokeh_exr_pathTL,
    p_emperical_ca_distTL,
    p_optical_vignetting_distanceTL,
    p_optical_vignetting_radiusTL,
    p_biasTL,
    p_invertTL,
    p_squareTL,
    p_squeezeTL,
    p_use_imageTL,
    p_bokeh_input_pathTL,
    p_bokeh_samples_multTL
};

node_parameters
{
    AiParameterFlt("sensor_widthTL", 3.6); // 35mm film
    AiParameterFlt("focal_lengthTL", 3.5); // in cm
    AiParameterFlt("fstopTL", 1.4);
    AiParameterFlt("focus_distanceTL", 100.0);
    AiParameterFlt("minimum_rgbTL", 0.2);
    AiParameterStr("bokeh_exr_pathTL", "");
    AiParameterFlt("emperical_ca_distTL", 0.0);
    AiParameterFlt("optical_vignetting_distanceTL", 0.0);
    AiParameterFlt("optical_vignetting_radiusTL", 2.0);

    AiParameterFlt("biasTL", 0.5);
    AiParameterBool("invertTL", false);

    AiParameterFlt("squareTL", 0.0);
    AiParameterFlt("squeezeTL", 1.0);

    AiParameterBool("use_imageTL", false);
    AiParameterStr("bokeh_input_pathTL", "");

    AiParameterInt("bokeh_samples_multTL", 10);

    AiParameterFlt("additional_luminanceTL", 0.0);
    AiParameterFlt("luminance_remap_transition_widthTL", 1.0);
}


node_initialize
{
    AiCameraInitialize(node);
    AiNodeSetLocalData(node, new CameraThinLens());
}


node_update
{
    CameraThinLens* tl = (CameraThinLens*)AiNodeGetLocalData(node);

    tl->sensor_width = AiNodeGetFlt(node, "sensor_widthTL");
    tl->focal_length = AiNodeGetFlt(node, "focal_lengthTL");
    tl->focal_length = clamp_min(tl->focal_length, 0.01);

    tl->fstop = AiNodeGetFlt(node, "fstopTL");
    tl->fstop = clamp_min(tl->fstop, 0.01);

    tl->focus_distance = AiNodeGetFlt(node, "focus_distanceTL");

    tl->fov = 2.0 * atan((tl->sensor_width / (2.0 * tl->focal_length))); // in radians
    tl->tan_fov = tanf(tl->fov / 2.0);
    tl->aperture_radius = (tl->focal_length) / (2.0 * tl->fstop);

    tl->minimum_rgb = AiNodeGetFlt(node, "minimum_rgbTL");
    tl->bokeh_exr_path = AiNodeGetStr(node, "bokeh_exr_pathTL");

    tl->emperical_ca_dist = AiNodeGetFlt(node, "emperical_ca_distTL");
    tl->optical_vignetting_distance = AiNodeGetFlt(node, "optical_vignetting_distanceTL");
    tl->optical_vignetting_radius = AiNodeGetFlt(node, "optical_vignetting_radiusTL");

    tl->bias = AiNodeGetFlt(node, "biasTL");
    tl->bias = clamp(tl->bias, 0.01, 0.99);
    tl->invert = AiNodeGetBool(node, "invertTL");

    tl->square = AiNodeGetFlt(node, "squareTL");
    tl->square = clamp(tl->square, 0.01, 0.99);
    tl->squeeze = AiNodeGetFlt(node, "squeezeTL");
    tl->squeeze = clamp(tl->squeeze, 0.01, 99999.0);

    tl->use_image = AiNodeGetBool(node, "use_imageTL");
    tl->bokeh_input_path = AiNodeGetStr(node, "bokeh_input_pathTL");

    tl->bokeh_samples_mult = AiNodeGetInt(node, "bokeh_samples_multTL");

    tl->additional_luminance = AiNodeGetFlt(node, "additional_luminanceTL");
    tl->luminance_remap_transition_width = AiNodeGetFlt(node, "luminance_remap_transition_widthTL");



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
                tl->image.bokehSample(input.lensx, input.lensy, unit_disk, xor128() / 4294967296.0, xor128() / 4294967296.0);
            } else {
                concentricDiskSample(input.lensx, input.lensy, unit_disk, tl->bias, tl->square, tl->squeeze);
            }
        } else {
            float r1 = xor128() / 4294967296.0;
            float r2 = xor128() / 4294967296.0;

            if (tl->use_image) {
                tl->image.bokehSample(r1, r2, unit_disk, xor128() / 4294967296.0, xor128() / 4294967296.0);
            } else {
                concentricDiskSample(r1, r2, unit_disk, tl->bias, tl->square, tl->squeeze);
            }
        }

        unit_disk(0) *= tl->squeeze;

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
            if (!empericalOpticalVignettingSquare(output.origin, output.dir, tl->aperture_radius, tl->optical_vignetting_radius, tl->optical_vignetting_distance, lerp_squircle_mapping(tl->square))){
                ++tries;
                continue;
            }
        }

        // this is most likely wrong!
        float coc = std::abs((tl->aperture_radius*unit_disk(0)) * (tl->focal_length * (tl->focus_distance - focusPoint.z)) / (tl->focus_distance * (focusPoint.z - tl->focal_length)));
        // CoC = abs(aperture * (focallength * (objectdistance - planeinfocus)) /
        //   (objectdistance * (planeinfocus - focallength)))
        AtRGB weight = AI_RGB_WHITE;
        if (tl->emperical_ca_dist > 0.0){
            const AtVector2 p2(p.x, p.y);
            const float distance_to_center = AiV2Dist(AtVector2(0.0, 0.0), p2);
            const int random_aperture = static_cast<int>(std::floor((xor128() / 4294967296.0) * 3.0));
            AtVector2 aperture_0_center(0.0, 0.0);
            AtVector2 aperture_1_center(- p2 * coc * tl->emperical_ca_dist); //previous: change coc for dist_to_center
            AtVector2 aperture_2_center(p2 * coc * tl->emperical_ca_dist);//previous: change coc for dist_to_center
            

            if (random_aperture == 1)      lens += aperture_1_center;
            else if (random_aperture == 2) lens += aperture_2_center;

            if (std::pow(lens.x-aperture_1_center.x, 2) + std::pow(lens.y - aperture_1_center.y, 2) > std::pow(tl->aperture_radius, 2)) {
                weight.r = 0.0;
            }
            if (std::pow(lens.x-aperture_0_center.x, 2) + std::pow(lens.y - aperture_0_center.y, 2) > std::pow(tl->aperture_radius, 2)) {
                weight.b = 0.0;
            }
            if (std::pow(lens.x-aperture_2_center.x, 2) + std::pow(lens.y - aperture_2_center.y, 2) > std::pow(tl->aperture_radius, 2)) {
                weight.g = 0.0;
            }

            if (weight == AI_RGB_ZERO){
                ++tries;
                continue;
            }
        
        //     //ca, not sure if this should be done, evens out the intensity?
        //     // float sum = (output.weight.r + output.weight.g + output.weight.b) / 3.0;
        //     // output.weight.r /= sum;
        //     // output.weight.g /= sum;
        //     // output.weight.b /= sum;
        }
        
        output.weight = weight;

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