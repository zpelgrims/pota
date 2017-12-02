#include <ai.h>
#include <string.h>

AI_CAMERA_NODE_EXPORT_METHODS(potaMethods)

enum
{
    p_sensorWidth,
    p_sensorHeight,
    p_focalLength,
    p_fStop,
    p_focalDistance
};

struct MyCameraData
{
    float fov;
    float tan_fov;
    float sensorWidth;
    float sensorHeight;
    float focalLength;
    float fStop;
    float focalDistance;
    float apertureRadius;
};


// Improved concentric mapping code by Dave Cline [peter shirley´s blog]
// maps points on the unit square onto the unit disk uniformly
inline void concentricDiskSample(float ox, float oy, AtVector2 *lens)
{
    float phi, r;

    // switch coordinate space from [0, 1] to [-1, 1]
    float a = 2.0 * ox - 1.0;
    float b = 2.0 * oy - 1.0;

    if ((a * a) > (b * b)){
        r = a;
        phi = (0.78539816339f) * (b / a);
    }
    else {
        r = b;
        phi = (AI_PIOVER2)-(0.78539816339f) * (a / b);
    }

    lens->x = r * std::cos(phi);
    lens->y = r * std::sin(phi);
}


node_parameters
{
    AiParameterFlt("sensorWidth", 3.6); // 35mm film
    AiParameterFlt("sensorHeight", 2.4); // 35 mm film
    AiParameterFlt("focalLength", 3.5); // in cm
    AiParameterFlt("fStop", 1.4);
    AiParameterFlt("focalDistance", 100.0);
}


node_initialize
{
    AiCameraInitialize(node);
    AiNodeSetLocalData(node, new MyCameraData());

    const int seed = 3158863998;
    const int nsamples = 2;
    const int ndim = 2;
    AtSampler *sampler = AiSampler(seed, nsamples, ndim);

}


node_update
{
    MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);

    data->sensorWidth = AiNodeGetFlt(node, "sensorWidth");
    data->sensorHeight = AiNodeGetFlt(node, "sensorHeight");
    data->focalLength = AiNodeGetFlt(node, "focalLength");
    data->fStop = AiNodeGetFlt(node, "fStop");
    data->focalDistance = AiNodeGetFlt(node, "focalDistance");

    data->fov = 2.0f * atan((data->sensorWidth / (2.0f * data->focalLength))); // in radians
    data->tan_fov = tanf(data->fov / 2.0f);
    data->apertureRadius = (data->focalLength) / (2.0f * data->fStop);

    AiMsgInfo("[POTA] fov: %f", data->fov);


    AiCameraUpdate(node, false);
}

node_finish
{
    MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);
    delete data;

    AtSampler *sampler = (AtSampler*)AiNodeGetLocalData(node);
    AiSamplerDestroy(sampler);
}

camera_create_ray
{
    const MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);
    AtSampler *sampler = (AtSampler*)AiNodeGetLocalData(node);
    AtSamplerIterator *iterator = AiSamplerIterator(sampler, sg);


    // create point on lens
    AtVector p(input.sx * data->tan_fov, input.sy * data->tan_fov, 1.0);

    // calculate direction vector from origin to point on lens
    output.dir = AiV3Normalize(p - output.origin);

    // either get uniformly distributed points on the unit disk or bokeh image
    AtVector2 lens(0.0, 0.0);
    AiSamplerGetSample(iterator, &lens.x)

    AiMsgInfo("lens.x = ", lens.x);


    concentricDiskSample(input.lensx, input.lensy, &lens);

    // scale points in [-1, 1] domain to actual aperture radius
    lens *= data->apertureRadius;

    // new origin is these points on the lens
    output.origin.x = lens.x;
    output.origin.y = lens.y;
    output.origin.z = 0.0;

    // Compute point on plane of focus, intersection on z axis
    float intersection = std::abs(data->focalDistance / output.dir.z);
    AtVector focusPoint = output.dir * intersection;
    output.dir = AiV3Normalize(focusPoint - output.origin);

    // now looking down -Z
    output.dir.z *= -1.0;
}


camera_reverse_ray
{
    // const MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);
    return false;
}

node_loader
{
    if (i != 0) return false;
    node->methods = potaMethods;
    node->output_type = AI_TYPE_UNDEFINED;
    node->name = "pota";
    node->node_type = AI_NODE_CAMERA;
    strcpy(node->version, AI_VERSION);
    return true;
}