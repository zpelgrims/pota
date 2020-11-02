#pragma once
#include <string>
#include "../../Eigen/Eigen/Dense"

#include "imagebokeh.h"
#include "global.h"


struct CameraThinLens
{
    imageData image;
    UnitModel unitModel;

    float sensor_width;
    float focal_length;
	float fov;
    float tan_fov;
    float fstop;
    float focus_distance;
    float aperture_radius;

    bool enable_dof;

    // float emperical_ca_dist;
    float optical_vignetting_distance;
    float optical_vignetting_radius;

    float abb_spherical;
    float abb_coma;
    float abb_distortion;

    int bokeh_aperture_blades;
    float circle_to_square;
    float bokeh_anamorphic;
    
    bool bokeh_enable_image;
    AtString bokeh_image_path;


    float bidir_min_luminance;
    unsigned int bidir_sample_mult;
    float bidir_add_luminance;
    float bidir_add_luminance_transition;

    int vignetting_retries;
    bool proper_ray_derivatives;

    AtNode *filter_node;
};

extern struct CameraThinLens tl;



// Improved concentric mapping code by Dave Cline [peter shirley´s blog]
// maps points on the unit square onto the unit disk uniformly
inline void concentricDiskSample(float ox, float oy, Eigen::Vector2d &lens, float bias, float squarelerp, float squeeze_x)
{
    if (ox == 0.0 && oy == 0.0){
        lens(0) = 0.0;
        lens(1) = 0.0;
        return;
    }

    float phi, r;

    // switch coordinate space from [0, 1] to [-1, 1]
    const float a = 2.0 * ox - 1.0;
    const float b = 2.0 * oy - 1.0;

    if ((a * a) > (b * b)){
        r = a;
        phi = 0.78539816339 * (b / a);
    }
    else {
        r = b;
        phi = (AI_PIOVER2) - ((0.78539816339) * (a / b));
    }

    if (bias != 0.5) r = AiBias(std::abs(r), bias) * (r < 0 ? -1 : 1);


    bool fast_trigo = true;

    const float cos_phi = fast_trigo ? fast_cos(phi) : std::cos(phi);
    const float sin_phi = fast_trigo ? fast_sin(phi) : std::sin(phi);
    lens(0) = r * cos_phi;
    lens(1) = r * sin_phi;

    if (squarelerp > 0.0){
        lens(0) = linear_interpolate(squarelerp, lens(0), a);
        lens(1) = linear_interpolate(squarelerp, lens(1), b);
    }
}


// creates a secondary, virtual aperture resembling the exit pupil on a real lens
inline bool empericalOpticalVignetting(AtVector origin, AtVector direction, float apertureRadius, float opticalVignettingRadius, float opticalVignettingDistance){
    // because the first intersection point of the aperture is already known, I can just linearly scale it by the distance to the second aperture
    float intersection = std::abs(opticalVignettingDistance / direction.z);
    AtVector opticalVignetPoint = (direction * intersection) - origin;
    float pointHypotenuse = std::sqrt((opticalVignetPoint.x * opticalVignetPoint.x) + (opticalVignetPoint.y * opticalVignetPoint.y));
    float virtualApertureTrueRadius = apertureRadius * opticalVignettingRadius;

    return std::abs(pointHypotenuse) < virtualApertureTrueRadius;
}

inline bool empericalOpticalVignettingSquare(AtVector origin, AtVector direction, float apertureRadius, float opticalVignettingRadius, float opticalVignettingDistance, float squarebias){
    float intersection = std::abs(opticalVignettingDistance / direction.z);
    AtVector opticalVignetPoint = (direction * intersection) - origin;

    float power = 1.0 + squarebias;
    float radius = apertureRadius * opticalVignettingRadius;
    float dist = std::pow(std::abs(opticalVignetPoint.x), power) + std::pow(std::abs(opticalVignetPoint.y), power);
   
	return !(dist > std::pow(radius, power));
}

// emperical mapping
inline float lerp_squircle_mapping(float amount) {
    return 1.0 + std::log(1.0+amount)*std::exp(amount*3.0);
}

inline AtVector2 barrelDistortion(AtVector2 uv, float distortion) {    
    uv *= 1. + AiV2Dot(uv, uv) * distortion;
    return uv;
}

inline AtVector2 inverseBarrelDistortion(AtVector2 uv, float distortion) {    
    
    float b = distortion;
    float l = AiV2Length(uv);
    
    float x0 = std::pow(9.*b*b*l + std::sqrt(3.) * std::sqrt(27.*b*b*b*b*l*l + 4.*b*b*b), 1./3.);
    float x = x0 / (std::pow(2., 1./3.) * std::pow(3., 2./3.) * b) - std::pow(2./3., 1./3.) / x0;
       
    return uv * (x / l);
}


// idea is to use the middle ray (does not get perturbed) as a measurement of how much coma there needs to be
inline float abb_coma_multipliers(const float sensor_width, const float focal_length, const AtVector dir_from_center, const Eigen::Vector2d unit_disk){
    const AtVector maximal_perturbed_ray(1.0 * (sensor_width*0.5), 1.0 * (sensor_width*0.5), -focal_length);
    float maximal_projection = AiV3Dot(AiV3Normalize(maximal_perturbed_ray), AtVector(0.0, 0.0, -1.0));
    float current_projection = AiV3Dot(dir_from_center, AtVector(0.0, 0.0, -1.0));
    float projection_perc = ((current_projection - maximal_projection)/(1.0-maximal_projection) - 0.5) * 2.0;
    float dist_from_sensor_center = 1.0 - projection_perc;
    float dist_from_aperture = unit_disk.norm();
    return dist_from_sensor_center * dist_from_aperture;
}


// rotate vector on axis orthogonal to ray dir
inline AtVector abb_coma_perturb(AtVector dir_from_lens, AtVector ray_to_perturb, float abb_coma, bool reverse){
    AtVector axis_tmp = AiV3Normalize(AiV3Cross(dir_from_lens, AtVector(0.0, 0.0, -1.0)));
    Eigen::Vector3d axis(axis_tmp.x, axis_tmp.y, axis_tmp.z);
    Eigen::Matrix3d rot(Eigen::AngleAxisd((abb_coma*2.3456*AI_PI)/180.0, axis)); // first arg is angle in degrees, constant is arbitrary
    Eigen::Vector3d raydir(ray_to_perturb.x, ray_to_perturb.y, ray_to_perturb.z);
    Eigen::Vector3d rotated_vector = (reverse ? rot.inverse() : rot) * raydir;
    return AtVector(rotated_vector(0), rotated_vector(1), rotated_vector(2));
}



inline void trace_ray_fw_thinlens(bool original_ray, int &tries, 
                                  float sx, float sy, float lensx, float lensy, 
                                  AtVector &origin, AtVector &dir, AtRGB &weight,
                                  float &r1, float &r2, 
                                  CameraThinLens *tl){
    tries = 0;
    bool ray_succes = false;

    while (!ray_succes && tries <= tl->vignetting_retries){
        
        // distortion
        AtVector s(sx, sy, 0.0);
        if (tl->abb_distortion > 0.0){
            AtVector2 s2 = barrelDistortion(AtVector2(sx, sy), tl->abb_distortion);
            s = {s2.x, s2.y, 0.0};
        }
        

        // create point on sensor (camera space)
        const AtVector p(s.x * (tl->sensor_width*0.5), 
                         s.y * (tl->sensor_width*0.5), 
                         -tl->focal_length);
            

        // calculate direction vector from origin to point on lens
        AtVector dir_from_center = AiV3Normalize(p); // or norm(p-origin)

        // either get uniformly distributed points on the unit disk or bokeh image
        Eigen::Vector2d unit_disk(0, 0);
        if (tries == 0 && tl->enable_dof) { // make use of blue noise sampler in arnold
            if (tl->bokeh_enable_image) {
                tl->image.bokehSample(lensx, lensy, unit_disk, xor128() / 4294967296.0, xor128() / 4294967296.0);
            } else if (tl->bokeh_aperture_blades < 2) {
                concentricDiskSample(lensx, lensy, unit_disk, tl->abb_spherical, tl->circle_to_square, tl->bokeh_anamorphic);
            } else {
                lens_sample_triangular_aperture(unit_disk(0), unit_disk(1), lensx, lensy, 1.0, tl->bokeh_aperture_blades);
            }
        } else if (tries != 0 && tl->enable_dof){
            r1 = xor128() / 4294967296.0;
            r2 = xor128() / 4294967296.0;

            if (tl->bokeh_enable_image) {
                tl->image.bokehSample(r1, r2, unit_disk, xor128() / 4294967296.0, xor128() / 4294967296.0);
            } else if (tl->bokeh_aperture_blades < 2) {
                concentricDiskSample(r1, r2, unit_disk, tl->abb_spherical, tl->circle_to_square, tl->bokeh_anamorphic);
            } else {
                lens_sample_triangular_aperture(unit_disk(0), unit_disk(1), r1, r2, 1.0, tl->bokeh_aperture_blades);
            }
        }

        unit_disk(0) *= tl->bokeh_anamorphic;



        // aberration inputs
        float abb_field_curvature = 0.0;



        AtVector lens(unit_disk(0) * tl->aperture_radius, unit_disk(1) * tl->aperture_radius, 0.0);
        const float intersection = std::abs(tl->focus_distance / linear_interpolate(abb_field_curvature, dir_from_center.z, 1.0));
        const AtVector focusPoint = dir_from_center * intersection;
        AtVector dir_from_lens = AiV3Normalize(focusPoint - lens);
        

        // perturb ray direction to simulate coma aberration
        float abb_coma = tl->abb_coma * abb_coma_multipliers(tl->sensor_width, tl->focal_length, dir_from_center, unit_disk);
        dir_from_lens = abb_coma_perturb(dir_from_lens, dir_from_lens, abb_coma, false);


        if (tl->optical_vignetting_distance > 0.0){
            if (!empericalOpticalVignettingSquare(lens, dir_from_lens, tl->aperture_radius, tl->optical_vignetting_radius, tl->optical_vignetting_distance, lerp_squircle_mapping(tl->circle_to_square))){
                ++tries;
                continue;
            }
        }


        // weight = AI_RGB_WHITE;
        // if (tl->emperical_ca_dist > 0.0){
        //     const AtVector2 p2(p.x, p.y);
        //     const float distance_to_center = AiV2Dist(AtVector2(0.0, 0.0), p2);
        //     const int random_aperture = static_cast<int>(std::floor((xor128() / 4294967296.0) * 3.0));
        //     AtVector2 aperture_0_center(0.0, 0.0);
        //     AtVector2 aperture_1_center(- p2 * coc * tl->emperical_ca_dist); //previous: change coc for dist_to_center
        //     AtVector2 aperture_2_center(p2 * coc * tl->emperical_ca_dist);//previous: change coc for dist_to_center
            

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
        
        // //     //ca, not sure if this should be done, evens out the intensity?
        // //     // float sum = (output.weight.r + output.weight.g + output.weight.b) / 3.0;
        // //     // output.weight.r /= sum;
        // //     // output.weight.g /= sum;
        // //     // output.weight.b /= sum;
        // }

        origin = lens;
        dir = dir_from_lens;

        switch (tl->unitModel){
            case mm:
            {
            origin *= 10.0; // reverse rays and convert to cm (from mm)
            dir *= 10.0; //reverse rays and convert to cm (from mm)
            } break;
            case cm:
            { 
            origin *= 1.0; // reverse rays and convert to cm (from mm)
            dir *= 1.0; //reverse rays and convert to cm (from mm)
            } break;
            case dm:
            {
            origin *= 0.1; // reverse rays and convert to cm (from mm)
            dir *= 0.1; //reverse rays and convert to cm (from mm)
            } break;
            case m:
            {
            origin *= 0.01; // reverse rays and convert to cm (from mm)
            dir *= 0.01; //reverse rays and convert to cm (from mm)
            }
        }

        weight = AI_RGB_WHITE;
        ray_succes = true;
    }

    if (!ray_succes) weight = AI_RGB_BLACK;
}

