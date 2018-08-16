#pragma once

#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>


// enum to switch between lens models in interface dropdown
enum LensModel{
    #include "auto_generated_lens_includes/pota_h_lenses.h"
};


// enum to switch between units in interface dropdown
enum UnitModel{
    mm,
    cm,
    dm,
    m
};


struct MyCameraData
{
	LensModel lensModel;
    UnitModel unitModel;

    // lens constants
    const char* lens_name;
    float lens_outer_pupil_radius;
    float lens_inner_pupil_radius;
    float lens_length;
    float lens_focal_length;
    float lens_aperture_pos;
    float lens_aperture_housing_radius;
    float lens_outer_pupil_curvature_radius;
    float lens_field_of_view;
    std::string lens_outer_pupil_geometry;


	float sensor_width;
	float fstop;
    float max_fstop;
	float focal_distance;
	float aperture_radius;
	float sensor_shift;
    int vignetting_retries;
	float lambda;
    int aperture_blades;
	bool dof;
    int backward_samples;
    float minimum_rgb;
    AtString bokeh_exr_path;

    bool run_intersection_tests;
    int count;

    bool proper_ray_derivatives;

    float random1;
    float random2;

    //camera_reverse_ray
    float tan_fov;
};

extern struct MyCameraData camera_data;