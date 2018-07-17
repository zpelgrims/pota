#pragma once

#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>


// enum to switch between lens models in interface dropdown
// will need a way to fill this automatically
enum LensModel{
    takumar_1969_50mm,
    zeiss_biotar_1927_58mm,
    zeiss_flektagon_1954_35mm,
    primoplan_1936_58mm,
    fisheye,
    fisheye_aspherical,
    doublegauss_100mm,
    angenieux_doublegauss_1953_49mm,
    petzval_1900_66mm,
    wideangle
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