#pragma once

#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <vector>


#include "../../polynomial-optics/src/lenssystem.h"
#include "../../polynomial-optics/src/raytrace.h"
#include "../../Eigen/Eigen/Dense"


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

/*
struct Draw
{
    std::vector<std::vector<float>> sensor;
    std::vector<std::vector<float>> aperture;
    std::vector<std::vector<float>> pxpy;
    std::vector<std::vector<float>> out;
    std::vector<std::vector<float>> sensor_shifted;
    bool enabled;
    int counter;
    const int max_counter;

    Draw() : enabled(true), counter(0), max_counter(50000){}
};
*/

struct Camera
{
	LensModel lensModel;
    UnitModel unitModel;

    //Draw draw;

    // lens constants
    const char* lens_name;
    float lens_outer_pupil_radius;
    float lens_inner_pupil_radius;
    float lens_length;
    float lens_back_focal_length;
    float lens_effective_focal_length;
    float lens_aperture_pos;
    float lens_aperture_housing_radius;
    float lens_inner_pupil_curvature_radius;
    float lens_outer_pupil_curvature_radius;
    float lens_field_of_view;
    float lens_fstop;
    float lens_aperture_radius_at_fstop;
    std::string lens_inner_pupil_geometry;
    std::string lens_outer_pupil_geometry;


	float sensor_width;
	float input_fstop;
    
    //debug
    float max_fstop;

	float focus_distance;
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

    float anamorphic_stretch;
};

extern struct Camera camera;
