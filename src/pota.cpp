#include <ai.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>

#include "../include/lens.h"


#define CACTUS 1
#ifdef CACTUS
#  define WORK_ONLY(block) block
std::string DRAW_DIRECTORY="/Users/zeno/pota/tests/";
#else
#  define WORK_ONLY(block)
#endif

#define DRAW 0
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
	p_sensor_width,
    p_sensor_height,
    p_fstop,
    p_focus_distance,
    p_aperture_sides
};


// enum to switch between lens models in interface dropdown
enum LensModel{
    fisheye,
    fisheye_aspherical,
    double_gauss,
    double_gauss_angenieux,
    petzval,
    NONE
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

    int aperture_sides;
	// int counter;

	bool dof;

	int rays_succes;
	int rays_fail;
    
    // float max_intersection_distance;
    // float min_intersection_distance;
    // long double average_intersection_distance;
    // int average_intersection_distance_cnt;
};



// to switch between lens models in interface dropdown
static const char* LensModelNames[] =
{
    "fisheye",
    "fisheye_aspherical",
    "double_gauss",
    "double_gauss_angenieux",
    "petzval",
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
inline void concentric_disk_sample(float ox, float oy, AtVector2 *lens)
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
float camera_set_focus(float dist, float aperture_radius, float lambda)
{
    // camera space vector to v1:
    const float target[3] = { 0.0, 0.0, dist};

    // initialize 5d light fields
    float sensor[5] = {0.0f};
    float out[5] = {0.0f};

    // set wavelength
    sensor[4] = lambda;

    float offset = 0.0f;
    int count = 0;

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

        lens_lt_sample_aperture(target, aperture, sensor, out, lambda);

        if(sensor[2+k] > 0){
            offset += sensor[k]/sensor[2+k];
            AiMsgInfo("[POTA] raytraced sensor shift: %f", sensor[k]/sensor[2+k]);
            count ++;
        }
      }
    }

    // average guesses
    offset /= count; 
    
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
    AiParameterEnum("lensModel", double_gauss_angenieux, LensModelNames);
    AiParameterFlt("sensor_width", 36.0); // 35mm film
    AiParameterFlt("sensor_height", 24.0); // 35 mm film
    AiParameterFlt("fstop", 0.0);
    AiParameterFlt("focus_distance", 1500.0); // in mm
    AiParameterInt("aperture_sides", 5);
    AiParameterBool("dof", true);
}


node_initialize
{
	AiCameraInitialize(node);
	AiNodeSetLocalData(node, new MyCameraData());
}


node_update
{	

	MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);

	/*
	DRAW_ONLY({
		drawData &dd = data->draw;

        // create file to transfer data to python drawing module
        dd.myfile.open(DRAW_DIRECTORY + "draw.pota", std::ofstream::out | std::ofstream::trunc);
        dd.myfile << "RAYS{";
    })
    */


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

	data->sensor_width = AiNodeGetFlt(node, "sensor_width");
	data->sensor_height = AiNodeGetFlt(node, "sensor_height");
	data->fstop = AiNodeGetFlt(node, "fstop");
	data->focus_distance = AiNodeGetFlt(node, "focus_distance");
	data->lensModel = (LensModel) AiNodeGetInt(node, "lensModel");
	data->aperture_sides = AiNodeGetInt(node, "aperture_sides");
	data->dof = AiNodeGetBool(node, "dof");


	data->max_fstop = lens_focal_length / (lens_aperture_housing_radius * 2.0f);
	AiMsgInfo("[POTA] Lens maximum f-stop: %f", data->max_fstop);

	if (data->fstop == 0.0f){
		data->aperture_radius = lens_aperture_housing_radius;
	} else {
		data->aperture_radius = fminf(lens_aperture_housing_radius, lens_focal_length / (2.0f * data->fstop));
	}

	AiMsgInfo("[POTA] full aperture radius: %f", lens_aperture_housing_radius);
	AiMsgInfo("[POTA] fstop-calculated aperture radius: %f", data->aperture_radius);


	// focus test, calculate sensor shift for correct focusing
	float infinity_focus_sensor_shift = camera_set_focus(AI_BIG, lens_aperture_housing_radius, .55f);
	data->sensor_shift = camera_set_focus(data->focus_distance, lens_aperture_housing_radius, .55f);
	AiMsgInfo("[POTA] sensor_shift to focus at infinity: %f", infinity_focus_sensor_shift);
	AiMsgInfo("[POTA] sensor_shift to focus at %f: %f", data->focus_distance, data->sensor_shift);

	/*
	data->rays_succes = 0;
	data->rays_fail = 0;
	*/

	/*
	DRAW_ONLY({
		data->min_intersection_distance = 999999.0;
		data->max_intersection_distance = -999999.0;
    })
    */


	AiCameraUpdate(node, false);
}


node_finish
{

	MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);


	AiMsgInfo("[POTA] rays passed %d", data->rays_succes);
	AiMsgInfo("[POTA] rays blocked %d", data->rays_fail);
	AiMsgInfo("[POTA] rays blocked percentage %f", (float)data->rays_fail / (float)(data->rays_succes + data->rays_fail));

	/*
    DRAW_ONLY({

    	// debug only
	    float average_intersection_distance_normalized = data->average_intersection_distance / (float)data->average_intersection_distance_cnt;
	    std::cout << "average_intersection_distance: " << data->average_intersection_distance << std::endl;
	    std::cout << "average_intersection_distance_cnt: " << data->average_intersection_distance_cnt << std::endl;
	    std::cout << "average_intersection_distance_normalized: " << average_intersection_distance_normalized << std::endl;
	    std::cout << "max_intersection_distance: " << data->max_intersection_distance << std::endl;
	    std::cout << "min_intersection_distance: " << data->min_intersection_distance << std::endl;

    	drawData &dd = data->draw;
    	dd.myfile << "}";
        dd.myfile.close();
    })
    */

	delete data;
}




camera_create_ray
{
	MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);
	// drawData &dd = data->draw;

    DRAW_ONLY({
    	/*
        if (dd.counter == 50000){
            dd.draw = true;
            dd.counter = 0;
    	}

    	int countlimit = 500000;
		++data->counter;
		*/
    })
	

  // polynomial optics

    // CHROMATIC ABERRATION STRATEGY
    // no:  trace 1 ray at .55f
    // yes: trace 3 rays at different wavelengths (CIE RGB) -> 3x more expensive

	// consider 3 apertures instead of one, might have to use an aperture sampling function from lens.h
	// shift them, increasingly towards the edges of the image
	// if through all 3, white, if only through two, ..

	// probably should try to trace another ray instead of setting weight to 0, this introduces noise


    float lambda = 0.55f; // 550 nanometers

    float sensor[5] = {0.0f};
    float aperture[5] = {0.0f};
    float out[5] = {0.0f};
    sensor[4] = lambda;

    // set sensor position coords
    sensor[0] = input.sx * (data->sensor_width * 0.5f);
    sensor[1] = input.sy * (data->sensor_width * 0.5f);


    /* for visual debugging, rays should converge to single point in idealised lens
    	sensor[0] = 0.0f;
    	sensor[1] = 0.0f;
	*/
    
    
    if (!data->dof){
    	aperture[0] = 0.0;
    	aperture[1] = 0.0;

    } else if (data->dof && data->aperture_sides == 0){

		// transform unit square to unit disk
	    AtVector2 unit_disk(0.0f, 0.0f);
	    concentric_disk_sample(input.lensx, input.lensy, &unit_disk);

	    aperture[0] = unit_disk.x * data->aperture_radius;
	    aperture[1] = unit_disk.y * data->aperture_radius;

    } else if (data->dof && data->aperture_sides > 0){
    	lens_sample_aperture(&aperture[0], &aperture[1], input.lensx, input.lensy, data->aperture_radius, data->aperture_sides);
    }


    // aperture sampling, to make sure ray goes through
    lens_pt_sample_aperture(sensor, aperture, data->sensor_shift);


    // move to beginning of polynomial
	sensor[0] += sensor[2] * data->sensor_shift; //sensor.pos.x = sensor.dir.x * sensor_shift
	sensor[1] += sensor[3] * data->sensor_shift; //sensor.pos.y = sensor.dir.y * sensor_shift


	// propagate ray from sensor to outer lens element
    float transmittance = lens_evaluate(sensor, out);
	if(transmittance <= 0.0f){
		output.weight = 0.0f;
	}


	// crop out by outgoing pupil
	if( out[0]*out[0] + out[1]*out[1] > lens_outer_pupil_radius*lens_outer_pupil_radius)
	{
		output.weight = 0.0f;
	}

	// crop at inward facing pupil
	const float px = sensor[0] + sensor[2] * lens_focal_length;
	const float py = sensor[1] + sensor[3] * lens_focal_length; //(note that lens_focal_length is the back focal length, i.e. the distance unshifted sensor -> pupil)

	if (px*px + py*py > lens_inner_pupil_radius*lens_inner_pupil_radius)
	{
		output.weight = 0.0f;
	}
	

	// convert from sphere/sphere space to camera space
	float camera_space_pos[3];
	float camera_space_omega[3];
	lens_sphereToCs(out, out+2, camera_space_pos, camera_space_omega, -lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius);


    output.origin.x = camera_space_pos[0];
    output.origin.y = camera_space_pos[1];
    output.origin.z = camera_space_pos[2];
    output.dir.x = camera_space_omega[0];
    output.dir.y = camera_space_omega[1];
    output.dir.z = camera_space_omega[2];

	output.origin *= -0.1; //reverse rays and convert to cm
    output.dir *= -0.1; //reverse rays and convert to cm

    /*
    if (output.weight.r > 0.0f){
    	++data->rays_succes;
    } else {
    	++ data->rays_fail;
    }*/



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




    /*
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
    */

    /*

    // line plane intersection with fixed intersection at y = 0
	AtVector rayplaneintersect = linePlaneIntersection(output.origin, output.dir);

	if (output.weight.r != 0.0f && output.weight.g != 0.0f && output.weight.b != 0.0f){

		if (rayplaneintersect.z > -9999.0 && rayplaneintersect.z < 9999.0){
			data->average_intersection_distance += rayplaneintersect.z;
			++data->average_intersection_distance_cnt;
		}

		if(rayplaneintersect.z > data->max_intersection_distance){
			data->max_intersection_distance = rayplaneintersect.z;
		}

		if(rayplaneintersect.z < data->min_intersection_distance){
			data->min_intersection_distance = rayplaneintersect.z;
		}
	}
    
	*/

	/*
    if (data->counter == countlimit){
    	data->counter = 0;
    }


	DRAW_ONLY(dd.draw = false;)
    DRAW_ONLY(++dd.counter;)
    */
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