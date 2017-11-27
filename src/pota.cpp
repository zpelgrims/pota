#include <ai.h>
#include <iostream>
#include <string.h>
#include <algorithm>



#include <cstdint>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <cstdlib>

#include "../polynomialOptics/render/lens.h"


#define CACTUS 1
#ifdef CACTUS
#  define WORK_ONLY(block) block
std::string DRAW_DIRECTORY="/Users/zeno/pota/tests/";
#else
#  define WORK_ONLY(block)
#endif

#define DRAW 1
#ifdef DRAW
#  define DRAW_ONLY(block) block
#else
#  define DRAW_ONLY(block)
#endif



AI_CAMERA_NODE_EXPORT_METHODS(potaMethods)



struct drawData{
    std::ofstream myfile;
    bool draw;
    int counter;

    drawData()
        : draw(false), counter(0){
    }
};


enum
{
	p_lensModel,
	p_sensorWidth,
    p_sensorHeight,
    p_fStop,
    p_focusDistance,
    p_sensor_shift_manual
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
	float fStop;
	float focusDistance;
	float apertureRadius;
	float sensorShift;
	int counter;
    LensModel lensModel;
    float sensor_shift_manual;

    float max_intersection_distance;
    float min_intersection_distance;
    long double average_intersection_distance;
    int average_intersection_distance_cnt;


    drawData draw;

};



// to switch between lens models in interface dropdown
static const char* LensModelNames[] =
{
    "fisheye_aspherical",
    "double_gauss",
    NULL
};


// xorshift fast random number generator
inline uint32_t xor128(void){
    static uint32_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
    uint32_t t = x ^ (x << 11);
    x = y; y = z; z = w;
    return w = (w ^ (w >> 19) ^ t ^ (t >> 8));
}



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



// line plane intersection with fixed intersection at y = 0, for finding the focal length and sensor shift
AtVector linePlaneIntersection(AtVector rayOrigin, AtVector rayDirection) {
    AtVector coord(100.0, 0.0, 100.0);
    AtVector planeNormal(0.0, 1.0, 0.0);
    rayDirection = AiV3Normalize(rayDirection);
    coord = AiV3Normalize(coord);
    return rayOrigin + (rayDirection * (AiV3Dot(coord, planeNormal) - AiV3Dot(planeNormal, rayOrigin)) / AiV3Dot(planeNormal, rayDirection));
}



// returns sensor offset in mm
float camera_set_focus(float dist, float aperture_radius){

    // camera space vector to v1:
    const float target[3] = { 0.0, 0.0, dist};

    // initialize 5d light fields
    float sensor[5] = {0.0f};
    float out[5] = {0.0f};

    // set wavelength
    sensor[4] = .55f;

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
        aperture[0] = 0.0f;
        aperture[1] = 0.0f;

        aperture[k] = aperture_radius * s/(S+1.0f); // (1to4)/(4+1) = .2, .4, .6, .8

        lens_lt_sample_aperture(target, aperture, sensor, out, .55f);

        if(sensor[2+k] > 0){
            offset += sensor[k]/sensor[2+k];
            AiMsgInfo("[POTA] raytraced sensor shift: %f", sensor[k]/sensor[2+k]);
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
			AiMsgInfo("[POTA] sensor offset smaller than minlimit: %f < %f", offset, -limit);
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
    AiParameterFlt("fStop", 1.4);
    AiParameterFlt("focusDistance", 1500.0); // in mm
    AiParameterFlt("sensor_shift_manual", 0.0f);
}


node_initialize
{
	AiCameraInitialize(node);
	AiNodeSetLocalData(node, new MyCameraData());
}


node_update
{	

	MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);


    drawData &dd = data->draw;
	DRAW_ONLY({
        // create file to transfer data to python drawing module
        dd.myfile.open(DRAW_DIRECTORY + "draw.pota", std::ofstream::out | std::ofstream::trunc);
        dd.myfile << "RAYS{";
    })


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
	data->fStop = AiNodeGetFlt(node, "fStop");
	data->focusDistance = AiNodeGetFlt(node, "focusDistance");
	data->lensModel = (LensModel) AiNodeGetInt(node, "lensModel");

    // data->apertureRadius = fminf(lens_aperture_housing_radius, (lens_focal_length) / (2.0f * data->fStop));
	// data->apertureRadius = lens_aperture_housing_radius;

	data->sensorShift = camera_set_focus(data->focusDistance, lens_aperture_housing_radius);
	AiMsgInfo("[POTA] sensor_shift to focus at %f: %f", data->focusDistance, data->sensorShift);

	data->sensor_shift_manual = AiNodeGetFlt(node, "sensor_shift_manual");

	data->min_intersection_distance = 999999.0;
	data->max_intersection_distance = -999999.0;

	AiCameraUpdate(node, false);
}

node_finish
{

	MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);
    drawData &dd = data->draw;

    float average_intersection_distance_normalized = data->average_intersection_distance / (float)data->average_intersection_distance_cnt;

    std::cout << "average_intersection_distance: " << data->average_intersection_distance << std::endl;
    std::cout << "average_intersection_distance_cnt: " << data->average_intersection_distance_cnt << std::endl;
    std::cout << "average_intersection_distance_normalized: " << average_intersection_distance_normalized << std::endl;

    std::cout << "max_intersection_distance: " << data->max_intersection_distance << std::endl;
    std::cout << "min_intersection_distance: " << data->min_intersection_distance << std::endl;

    DRAW_ONLY({
    	dd.myfile << "}";
        dd.myfile.close();
    })

	delete data;
}

camera_create_ray
{
	// const MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);
	MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);

	drawData &dd = data->draw;

    DRAW_ONLY({
        if (dd.counter == 50000){
            dd.draw = true;
            dd.counter = 0;
    	}
    })


	int countlimit = 500000;
	++data->counter;

  // polynomial optics

    // CHROMATIC ABERRATION STRATEGY
    // no:  trace 1 ray at .55f
    // yes: trace 3 rays at different wavelengths (CIE RGB) -> 3x more expensive
    float lambda = 0.55f; // 550 nanometers

    float sensor[5] = {0.0f};
    float aperture[5] = {0.0f};
    float out[5] = {0.0f};
    sensor[4] = lambda;

    // set sensor position coords
    sensor[0] = input.sx * (data->sensorWidth * 0.5f);
    sensor[1] = input.sy * (data->sensorWidth * 0.5f);

    if(data->counter == countlimit){

    }

    // for visual debugging, tmp!
    sensor[0] = 0.0f;
    sensor[1] = 0.0f;

    /*
    // transform unit square to unit disk
    AtVector2 unit_disk(0.0f, 0.0f);
    concentricDiskSample(input.lensx, input.lensy, &unit_disk);
    aperture[0] = unit_disk.x * lens_aperture_housing_radius;
    aperture[1] = unit_disk.y * lens_aperture_housing_radius;
	*/

    // xor128 is just a fast random number generator, you're probably familiar
    lens_sample_aperture(&aperture[0], &aperture[1], input.lensx, input.lensy, lens_aperture_housing_radius, 6);
    
    //aperture[0] *= 0.0; // for testing, creates image without dof
    //aperture[1] *= 0.0; // for testing, creates image without dof

    lens_pt_sample_aperture(sensor, aperture, data->sensorShift);

    // move to beginning of polynomial
	sensor[0] += sensor[2] * data->sensorShift; //sensor.pos.x = sensor.dir.x * sensorshift
	sensor[1] += sensor[3] * data->sensorShift; //sensor.pos.y = sensor.dir.y * sensorshift


    float transmittance = lens_evaluate(sensor, out);
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
	

	// convert from sphere/sphere to camera coords
	float camera_space_pos[3];
	float camera_space_omega[3];
	lens_sphereToCs(out, out+2, camera_space_pos, camera_space_omega, -lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius);

    if(data->counter == countlimit){
		std::cout << "lens_sphereToCs" << std::endl;
		std::cout << "\tsensor[0, 1](pos): " << sensor[0] << ", " << sensor[1] << std::endl;
		std::cout << "\tsensor[2, 3](dir): " << sensor[2] << ", " << sensor[3] << std::endl;
		std::cout << std::endl;
		std::cout << "\toutpos[0, 1, 2]: " << camera_space_pos[0] << ", " << camera_space_pos[1] << ", " << camera_space_pos[2] << std::endl;
		std::cout << "\toutdir[0, 1, 2]: " << camera_space_omega[0] << ", " << camera_space_omega[1] << ", " << camera_space_omega[2] << std::endl;
		std::cout << std::endl;
	}


	/* NOT NEEDED FOR ARNOLD, GOOD INFO THOUGH
	//now let's go world space:
	//initialise an ONB/a frame around the first vertex at the camera position along n=camera lookat direction:

	view_cam_init_frame(p, &p->v[0].hit);
	
	for(int k=0;k<3;k++)
	{
		//this is the world space position of the outgoing ray:
	    p->v[0].hit.x[k] +=   camera_space_pos[0] * p->v[0].hit.a[k] 
	                        + camera_space_pos[1] * p->v[0].hit.b[k] 
	                        + camera_space_pos[2] * p->v[0].hit.n[k];

		//this is the world space direction of the outgoing ray:
	    p->e[1].omega[k] =   camera_space_omega[0] * p->v[0].hit.a[k] 
	                       + camera_space_omega[1] * p->v[0].hit.b[k]
	                       + camera_space_omega[2] * p->v[0].hit.n[k];
	}

	//now need to rotate the normal of the frame, in case you need any cosines later in light transport. if not, leave out:
	const float R = lens_outer_pupil_curvature_radius;
	// recompute full frame:
	float n[3] = {0.0f};
	for(int k=0;k<3;k++)
	    n[k] +=   p->v[0].hit.a[k] * out[0]/R 
	            + p->v[0].hit.b[k] * out[1]/R 
	            + p->v[0].hit.n[k] * (out[2] + R)/fabsf(R);

	for(int k=0;k<3;k++) p->v[0].hit.n[k] = n[k];

	// also clip to valid pixel range.
	if(p->sensor.pixel_i < 0.0f || p->sensor.pixel_i >= view_width() ||
		p->sensor.pixel_j < 0.0f || p->sensor.pixel_j >= view_height())
		return 0.0f;
	*/



    output.origin.x = camera_space_pos[0];
    output.origin.y = camera_space_pos[1];
    output.origin.z = camera_space_pos[2];

    output.dir.x = camera_space_omega[0];
    output.dir.y = camera_space_omega[1];
    output.dir.z = camera_space_omega[2];

	output.origin *= -1.0; //reverse rays
    output.dir *= -1.0; //reverse rays


    DRAW_ONLY({
        if (dd.draw == true && output.weight.r != 0.0f && output.weight.g != 0.0f && output.weight.b != 0.0f){
            dd.myfile << std::fixed << std::setprecision(10) << output.origin.x << " ";
            dd.myfile << std::fixed << std::setprecision(10) << output.origin.y << " ";
            dd.myfile << std::fixed << std::setprecision(10) << output.origin.z << " ";
            dd.myfile << std::fixed << std::setprecision(10) << output.dir.x << " ";
            dd.myfile << std::fixed << std::setprecision(10) << output.dir.y << " ";
            dd.myfile << std::fixed << std::setprecision(10) << output.dir.z << " ";
        }
    })

    // convert wavelength shift into rgb shift
    output.weight *= 1.0f;



    // line plane intersection with fixed intersection at y = 0
	AtVector rayplaneintersect = linePlaneIntersection(output.origin, output.dir);

	if (output.weight.r != 0.0f && output.weight.g != 0.0f && output.weight.b != 0.0f){
		//std::cout << "lineplaneintersect: " << rayplaneintersect.x << ", " << rayplaneintersect.y << ", " << rayplaneintersect.z << std::endl;

		if (rayplaneintersect.z > -9999.0 && rayplaneintersect.z < 9999.0){
			data->average_intersection_distance += rayplaneintersect.z;
			//std::cout << data->average_intersection_distance << std::endl;
			++data->average_intersection_distance_cnt;
		}

		if(rayplaneintersect.z > data->max_intersection_distance){
			data->max_intersection_distance = rayplaneintersect.z;
		}

		if(rayplaneintersect.z < data->min_intersection_distance){
			data->min_intersection_distance = rayplaneintersect.z;
		}
	}
    


    
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


	DRAW_ONLY(dd.draw = false;)
    DRAW_ONLY(++dd.counter;)
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