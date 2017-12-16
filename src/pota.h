#ifndef POTA_H
#define POTA_H

#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>

// enum to switch between lens models in interface dropdown
enum LensModel{
    fisheye,
    fisheye_aspherical,
    double_gauss,
    double_gauss_angenieux,
    petzval,
    NONE
};

struct drawData{
    std::ofstream myfile;
    bool draw;
    int counter;

    drawData()
        : draw(false), counter(0){
    }
};


struct MyCameraData
{
	LensModel lensModel;
    drawData draw;


	float sensor_width;
	float sensor_height;
	float fstop;
    float max_fstop;
	float focus_distance;
	float aperture_radius;
	float sensor_shift;
	float aperture_colorshift;
	float lambda;
    int aperture_blades;
	bool dof;

	int rays_succes;
	int rays_fail;
};

extern struct MyCameraData data;





#endif