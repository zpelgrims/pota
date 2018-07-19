#pragma once

#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>


// enum to switch between lens models in interface dropdown
enum LensModel{
    #ifdef LENS_ID_FREE
        1969_asahi_takumar_50mm,
        1927_zeiss_biotar_58mm,
        1954_zeiss_flektagon_35mm,
        1936_meyer_optik_goerlitz_primoplan_58mm,
        fisheye,
        fisheye_aspherical,
        doublegauss_100mm,
        1953_angenieux_doublegauss_49mm,
        1900_petzval_66mm,
        wideangle
    #endif
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