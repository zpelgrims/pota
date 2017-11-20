#include <ai.h>
#include <iostream>
#include <string.h>
#include <algorithm>
#include "../polynomialOptics/render/lens.h"



AI_CAMERA_NODE_EXPORT_METHODS(potaMethods)

enum
{
	p_lensModel,
	p_sensorWidth,
    p_sensorHeight,
    p_focalLength,
    p_fStop,
    p_focusDistance
};


// enum to switch between lens models in interface dropdown
enum LensModel{
    fisheye_aspherical,
    double_gauss,
    NONE
};



struct MyCameraData
{
	float fov;
	float tan_fov;
	float sensorWidth;
	float sensorHeight;
	float focalLength;
	float fStop;
	float focusDistance;
	float apertureRadius;
	float sensorShift;
	int counter;
    LensModel lensModel;

};



// to switch between lens models in interface dropdown
static const char* LensModelNames[] =
{
    "fisheye_aspherical",
    "double_gauss",
    NULL
};



inline float Lerp(float t, float v1, float v2)
{
	return (1 - t) * v1 + t * v2;
}

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


// returns sensor offset in mm
float camera_set_focus(float dist, float aperture_radius){

	// NOTE: what are my units? if it is cm then this should be 10..
    //const float dm2mm = 100.0f; //default is 100.0 (dmtomm)

    // camera space vector to v1:
    // const float target[3] = { 0.0, 0.0, dist*dm2mm }; //dist*dm2mm
    const float target[3] = { 0.0, 0.0, dist}; //dist*dm2mm

    // initialize 5d light fields
    float sensor[5] = {0.0f}, out[5] = {0.0f};

    // set wavelength
    sensor[4] = .5f;

    float offset = 0.0f;
    int cnt = 0;

    // just point through center of aperture
    float aperture[2] = {0.0f, 0.0f};

    const int S = 4;

    // trace a couple of adjoint rays from there to the sensor and
    // see where we need to put the sensor plane.
    for(int s=1; s<=S; s++){
      for(int k=0; k<2; k++){

        // reset aperture
        aperture[0] = aperture[1] = 0.0f;

        aperture[k] = aperture_radius * s/(S+1.0f); // (1to4)/(4+1) = .2, .4, .6, .8

        lens_lt_sample_aperture(target, aperture, sensor, out, .5f);

        if(sensor[2+k] > 0){
            offset += sensor[k]/sensor[2+k];
            cnt ++;
        }
      }
    }

    // average guesses
    offset /= cnt; 

    // the focus plane/sensor offset:
    // negative because of reverse direction
    if(offset == offset){ // check NaN cases
		const float limit = 45.0f; // why this hard limit? Where does it come from?
		if(offset > limit){
			AiMsgInfo("[POTA] sensor offset bigger than maxlimit: %f > %f", offset, limit);
          	return limit;
		} else if(offset < -limit){
			AiMsgInfo("[POTA] sensor offset smaller than minlimit: %f > %f", offset, -limit);
        	return -limit;
		} else {
			AiMsgInfo("[POTA] sensor offset inside of limits: %f", offset);
			return offset; // in mm
		}
    }
}


node_parameters
{
    AiParameterEnum("lensModel", double_gauss, LensModelNames);
    AiParameterFlt("sensorWidth", 36.0); // 35mm film
    AiParameterFlt("sensorHeight", 24.0); // 35 mm film
    AiParameterFlt("focalLength", 7.5); // in cm
    AiParameterFlt("fStop", 1.4);
    AiParameterFlt("focusDistance", 1000.0); // in mm?

}


node_initialize
{
	AiCameraInitialize(node);
	AiNodeSetLocalData(node, new MyCameraData());
}


node_update
{
	MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);

	AiMsgInfo("[POTA] lens_length: %s", lens_name);
    AiMsgInfo("[POTA] lens_outer_pupil_radius: %f", lens_outer_pupil_radius);
    AiMsgInfo("[POTA] lens_inner_pupil_radius: %f", lens_inner_pupil_radius);
    AiMsgInfo("[POTA] lens_length: %f", lens_length);
	AiMsgInfo("[POTA] lens_focal_length: %f", lens_focal_length);
	AiMsgInfo("[POTA] lens_aperture_pos: %f", lens_aperture_pos);
	AiMsgInfo("[POTA] lens_aperture_housing_radius: %f", lens_aperture_housing_radius);
	AiMsgInfo("[POTA] lens_outer_pupil_curvature_radius: %f", lens_outer_pupil_curvature_radius);
	AiMsgInfo("[POTA] lens_focal_length: %f", lens_focal_length);
	AiMsgInfo("[POTA] lens_field_of_view: %f", lens_field_of_view);
	AiMsgInfo("[POTA] --------------------------------------");

	data->sensorWidth = AiNodeGetFlt(node, "sensorWidth");
	data->sensorHeight = AiNodeGetFlt(node, "sensorHeight");
	data->focalLength = AiNodeGetFlt(node, "focalLength");
	data->fStop = AiNodeGetFlt(node, "fStop");
	data->focusDistance = AiNodeGetFlt(node, "focusDistance");
	data->lensModel = (LensModel) AiNodeGetInt(node, "lensModel");

    //data->apertureRadius = fminf(lens_aperture_housing_radius, (lens_focal_length) / (2.0f * data->fStop));
	data->apertureRadius = lens_inner_pupil_radius;

	data->sensorShift = camera_set_focus(data->focusDistance, lens_inner_pupil_radius);
	AiMsgInfo("[POTA] sensor_shift to focus at %f: %f", data->focusDistance, data->sensorShift);

	AiCameraUpdate(node, false);
}

node_finish
{
	MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);
	delete data;
}

camera_create_ray
{
	// const MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);
	MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);

	int countlimit = 500000;
	++data->counter;

  // polynomial optics

    // CHROMATIC ABERRATION STRATEGY
    // if none: trace 1 ray at .55f
    // if some: trace 3 rays at different wavelengths (CIE RGB) -> 3x more expensive
    float lambda = 0.55f; // 550 nanometers

    float sensor[5] = {0.0f};
    float aperture[5] = {0.0f};
    float out[5] = {0.0f};
    sensor[4] = lambda;

    // set sensor coords, might be wrong
    sensor[0] = input.sx * (data->sensorWidth * 0.5f);
    sensor[1] = input.sy * (data->sensorHeight * 0.5f);

    if(data->counter == countlimit){
    	std:: cout << "input.sx, input.sy: " << input.sx << ", " << input.sy << std::endl;
		std::cout << "sensor[0, 1]: " << sensor[0] << ", " << sensor[1] << std::endl;
	}


    AtVector2 unit_disk(0.0f, 0.0f);
    concentricDiskSample(input.lensx, input.lensy, &unit_disk);

    aperture[0] = unit_disk.x * lens_aperture_housing_radius;
    aperture[1] = unit_disk.y * lens_aperture_housing_radius;


    lens_pt_sample_aperture(sensor, aperture, data->sensorShift);

    // move to beginning of polynomial
	sensor[0] += sensor[2] * data->sensorShift;
	sensor[1] += sensor[3] * data->sensorShift;


	if(data->counter == countlimit){
		std::cout << "lens_pt_sample_aperture" << std::endl;
		std::cout << "\tsensor[0, 1](pos): " << sensor[0] << ", " << sensor[1] << std::endl;
		std::cout << "\tsensor[2, 3](dir): " << sensor[2] << ", " << sensor[3] << std::endl;
		std::cout << "\tsensor[4](lambda): " << sensor[4] << std::endl;

		std::cout << std::endl;
		std::cout << "\tout[0, 1] (pos): " << aperture[0] << ", " << aperture[1] << std::endl;
		std::cout << "\tout[2, 3](dir): " << aperture[2] << ", " << aperture[3] << std::endl;
		std::cout << "\tout[4](lambda): " << aperture[4] << std::endl;
	}



	// evaluates from sensor (sensor) to outer pupil (out).
	// input arrays are 5d [x,y,dx,dy,lambda] where dx and dy are the direction in
	// two-plane parametrization (that is the third component of the direction would be 1.0).
	// units are millimeters for lengths and micrometers for the wavelength (so visible light is about 0.4--0.7)
	// returns the transmittance computed from the polynomial.

    float transmittance = lens_evaluate(sensor, out);
	if(data->counter == countlimit){
		std::cout << "lens_evaluate" << std::endl;
		std::cout << "\tsensor[0, 1](pos): " << sensor[0] << ", " << sensor[1] << std::endl;
		std::cout << "\tsensor[2, 3](dir): " << sensor[2] << ", " << sensor[3] << std::endl;
		std::cout << "\tsensor[4](lambda): " << sensor[4] << std::endl;

		std::cout << std::endl;
		std::cout << "\tout[0, 1] (pos): " << out[0] << ", " << out[1] << std::endl;
		std::cout << "\tout[2, 3](dir): " << out[2] << ", " << out[3] << std::endl;
		std::cout << "\tout[4](lambda): " << out[4] << std::endl;

		std::cout << std::endl;
		std::cout << "\ttransmittance: " << transmittance << std::endl;
	}


	if(transmittance <= 0.0f){
		output.weight = 0.0f;
	}

	// crop out by outgoing pupil and crop at inward facing pupil:
	const float px = sensor[0] + sensor[2] * lens_focal_length;
	const float py = sensor[1] + sensor[3] * lens_focal_length; //(note that lens_focal_length is the back focal length, i.e. the distance unshifted sensor -> pupil)

	if((out[0]*out[0] + out[1]*out[1] > lens_outer_pupil_radius*lens_outer_pupil_radius) ||
	   (px*px + py*py > lens_inner_pupil_radius*lens_inner_pupil_radius))
	{
		output.weight = 0.0f;
	}



    // converts sphere/sphere coords to camera coords
    //float inpos[2] = {in[0], in[1]};
    //float indir[2] = {in[2], in[3]};
    //float outpos[2] = {out[0], out[1]};
    //float outdir[2] = {out[2], out[3]};


	// convert out[4] to camera space:
	float camera_space_pos[3];
	float camera_space_omega[3];
	lens_sphereToCs(out, out+2, camera_space_pos, camera_space_omega, -lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius);



    // HOW TO QUERY sphereCenter?? maybe it's just -lens_outer_pupil_curvature_radius
    if(data->counter == countlimit){
		std::cout << "lens_sphereToCs" << std::endl;
		std::cout << "\tsensor[0, 1](pos): " << sensor[0] << ", " << sensor[1] << std::endl;
		std::cout << "\tsensor[2, 3](dir): " << sensor[2] << ", " << sensor[3] << std::endl;
		std::cout << std::endl;
		std::cout << "\toutpos[0, 1, 2]: " << camera_space_pos[0] << ", " << camera_space_pos[1] << camera_space_pos[2] << std::endl;
		std::cout << "\toutdir[0, 1, 2]: " << camera_space_omega[0] << ", " << camera_space_omega[1] << camera_space_omega[2] << std::endl;
		std::cout << std::endl;
	}



    output.origin.x = camera_space_pos[0];
    output.origin.y = camera_space_pos[1];
    output.origin.z = camera_space_pos[2];

    output.dir.x = camera_space_omega[0];
    output.dir.y = camera_space_omega[1];
    output.dir.z = camera_space_omega[2];

    output.dir *= -1.0;

    // convert wavelength shift into rgb shift
    output.weight *= 1.0f;


    
    if (data->counter == countlimit){
    	data->counter = 0;
    }



    // switch structure
    switch (data->lensModel)
    {
        case double_gauss:
        {
        }
        break;

        case fisheye_aspherical:
        {
        }

        case NONE:
        default:
        
        break;
    }
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