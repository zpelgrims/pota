#include <ai.h>
#include <string.h>

// ca
#include <cmath>

AI_CAMERA_NODE_EXPORT_METHODS(lentil_thinlensMethods)

enum
{
    p_sensorWidth,
    p_sensorHeight,
    p_focal_length,
    p_fStop,
    p_focal_distance
};

struct Camera
{
    float fov;
    float tan_fov;
    float sensorWidth;
    float sensorHeight;
    float focal_length;
    float fStop;
    float focal_distance;
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
    AiParameterFlt("focal_length", 3.5); // in cm
    AiParameterFlt("fStop", 1.4);
    AiParameterFlt("focal_distance", 100.0);
}


node_initialize
{
    AiCameraInitialize(node);
    AiNodeSetLocalData(node, new Camera());
}


node_update
{
    Camera* data = (Camera*)AiNodeGetLocalData(node);

    data->sensorWidth = AiNodeGetFlt(node, "sensorWidth");
    data->sensorHeight = AiNodeGetFlt(node, "sensorHeight");
    data->focal_length = AiNodeGetFlt(node, "focal_length");
    data->fStop = AiNodeGetFlt(node, "fStop");
    data->focal_distance = AiNodeGetFlt(node, "focal_distance");

    data->fov = 2.0f * atan((data->sensorWidth / (2.0f * data->focal_length))); // in radians
    data->tan_fov = tanf(data->fov / 2.0f);
    data->apertureRadius = (data->focal_length) / (2.0f * data->fStop);

    AiMsgInfo("[LENTIL_THINLENS] fov: %f", data->fov);


    AiCameraUpdate(node, false);
}

node_finish
{
    Camera* data = (Camera*)AiNodeGetLocalData(node);
    delete data;
}


// xorshift fast random number generator
inline uint32_t xor128(void){
  static uint32_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
  uint32_t t = x ^ (x << 11);
  x = y; y = z; z = w;
  return w = (w ^ (w >> 19) ^ t ^ (t >> 8));
}


camera_create_ray
{
    const Camera* data = (Camera*)AiNodeGetLocalData(node);

    // create point on lens
    AtVector p(input.sx * data->tan_fov, input.sy * data->tan_fov, 1.0);

    // calculate direction vector from origin to point on lens
    output.dir = AiV3Normalize(p - output.origin);

    // either get uniformly distributed points on the unit disk or bokeh image
    AtVector2 lens(0.0, 0.0);
    concentricDiskSample(input.lensx, input.lensy, &lens);

    // scale points in [-1, 1] domain to actual aperture radius
    lens *= data->apertureRadius;

    // ca
    AtVector2 p2(p.x, p.y);
    float distance_to_center = AiV2Dist(AtVector2(0.0, 0.0), p2);
    int random_aperture = static_cast<int>(std::floor((xor128() / 4294967296.0) * 3.0));
    float chr_abb_mult = 3.0;
    AtVector2 aperture_0_center(0.0, 0.0);
    AtVector2 aperture_1_center(- p2 * distance_to_center * chr_abb_mult);
    AtVector2 aperture_2_center(p2 * distance_to_center * chr_abb_mult);
    output.weight = AtRGB(1.0);
    if (random_aperture == 0) {
        if (std::pow(lens.x-aperture_1_center.x, 2) + std::pow(lens.y - aperture_1_center.y, 2) > std::pow(data->apertureRadius, 2)) {
            output.weight.r = 0.0;
        }
        if (std::pow(lens.x-aperture_0_center.x, 2) + std::pow(lens.y - aperture_0_center.y, 2) > std::pow(data->apertureRadius, 2)) {
            output.weight.b = 0.0;
        }
        if (std::pow(lens.x-aperture_2_center.x, 2) + std::pow(lens.y - aperture_2_center.y, 2) > std::pow(data->apertureRadius, 2)) {
            output.weight.g = 0.0;
        }
    } else if (random_aperture == 1) {
        lens += aperture_1_center;
        if (std::pow(lens.x-aperture_1_center.x, 2) + std::pow(lens.y - aperture_1_center.y, 2) > std::pow(data->apertureRadius, 2)) {
            output.weight.r = 0.0;
        }
        if (std::pow(lens.x-aperture_0_center.x, 2) + std::pow(lens.y - aperture_0_center.y, 2) > std::pow(data->apertureRadius, 2)) {
            output.weight.b = 0.0;
        }
        if (std::pow(lens.x-aperture_2_center.x, 2) + std::pow(lens.y - aperture_2_center.y, 2) > std::pow(data->apertureRadius, 2)) {
            output.weight.g = 0.0;
        }
    } else if (random_aperture == 2) {
        lens += aperture_2_center;
        if (std::pow(lens.x-aperture_1_center.x, 2) + std::pow(lens.y - aperture_1_center.y, 2) > std::pow(data->apertureRadius, 2)) {
            output.weight.r = 0.0;
        }
        if (std::pow(lens.x-aperture_0_center.x, 2) + std::pow(lens.y - aperture_0_center.y, 2) > std::pow(data->apertureRadius, 2)) {
            output.weight.b = 0.0;
        }
        if (std::pow(lens.x-aperture_2_center.x, 2) + std::pow(lens.y - aperture_2_center.y, 2) > std::pow(data->apertureRadius, 2)) {
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
    float intersection = std::abs(data->focal_distance / output.dir.z);
    AtVector focusPoint = output.dir * intersection;
    output.dir = AiV3Normalize(focusPoint - output.origin);

    // now looking down -Z
    output.dir.z *= -1.0;
}


camera_reverse_ray
{
    // const Camera* data = (Camera*)AiNodeGetLocalData(node);
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