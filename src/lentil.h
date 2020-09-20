#pragma once

#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <vector>

#include "../../polynomial-optics/src/lenssystem.h"
#include "../../polynomial-optics/src/raytrace.h"
#include "../../Eigen/Eigen/Dense"

#include "imagebokeh.h"


// enum to switch between lens models in interface dropdown
enum LensModel{
    #include "../include/auto_generated_lens_includes/pota_h_lenses.h"
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

struct CameraRaytraced 
{
    std::string id;
    int lenses_cnt;
    std::vector<lens_element_t> lenses;
    double first_element_housing_radius;
    double zoom;
    double lens_focal_length;
    double thickness_original;
    double total_lens_length;
};

struct Timing
{
    long long int total_duration;
    long long int execution_counter;
};

struct Camera
{
	LensModel lensModel;
    CameraRaytraced camera_rt;
    imageData image;

    #ifdef TIMING
        Timing timing;
    #endif

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
    int bokeh_aperture_blades;
	bool dof;
    double bidir_min_luminance;
    AtString bidir_output_path;

    bool run_intersection_tests;
    int count;

    bool proper_ray_derivatives;

    double random1;
    double random2;

    //camera_reverse_ray
    double tan_fov;

    double anamorphic_stretch;

    bool bokeh_enable_image;
    AtString bokeh_image_path;

    float empirical_ca_dist;
    float bidir_add_luminance;
    float bidir_add_luminance_transition;

    int bidir_sample_mult;

    float extra_sensor_shift;
};

extern struct Camera camera;