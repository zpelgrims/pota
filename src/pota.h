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
    #include "../include/auto_generated_lens_includes/pota_h_lenses.h"
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
    double lens_outer_pupil_radius;
    double lens_inner_pupil_radius;
    double lens_length;
    double lens_back_focal_length;
    double lens_effective_focal_length;
    double lens_aperture_pos;
    double lens_aperture_housing_radius;
    double lens_inner_pupil_curvature_radius;
    double lens_outer_pupil_curvature_radius;
    double lens_field_of_view;
    double lens_fstop;
    double lens_aperture_radius_at_fstop;
    std::string lens_inner_pupil_geometry;
    std::string lens_outer_pupil_geometry;


	double sensor_width;
	double input_fstop;
    
    //debug
    double max_fstop;

	double focus_distance;
	double aperture_radius;
	double sensor_shift;
    int vignetting_retries;
	double lambda;
    int aperture_blades;
	bool dof;
    int backward_samples;
    double minimum_rgb;
    AtString bokeh_exr_path;

    bool run_intersection_tests;
    int count;

    bool proper_ray_derivatives;

    double random1;
    double random2;

    //camera_reverse_ray
    double tan_fov;

    double anamorphic_stretch;
};

extern struct Camera camera;
