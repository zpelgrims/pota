#include <ai.h>
#include "../include/lens.h"
#include "pota.h"


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


enum
{
	p_lensModel,
	p_sensor_width,
    p_fstop,
    p_focus_distance,
    p_aperture_blades
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
    const float target[3] = { 0.0, 0.0, dist};
    float sensor[5] = {0.0f};
    float out[5] = {0.0f};
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
            AiMsgInfo("\t%s  [POTA] raytraced sensor shift at aperture[%f, %f]: %f", emoticon, aperture[0], aperture[1], sensor[k]/sensor[2+k]);
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
			AiMsgInfo("%s  [POTA] sensor offset bigger than maxlimit: %f > %f", emoticon, offset, limit);
          	return limit;
		} else if(offset < -limit){
			AiMsgInfo("%s  [POTA] sensor offset smaller than minlimit: %f < %f", emoticon, offset, -limit);
        	return -limit;
		} else {
			AiMsgInfo("%s  [POTA] sensor offset inside of limits: %f", emoticon, offset);
			return offset; // in mm
		}
    }
}


node_parameters
{
    AiParameterEnum("lensModel", double_gauss_angenieux, LensModelNames);
    AiParameterFlt("sensor_width", 36.0); // 35mm film
    AiParameterFlt("fstop", 0.0);
    AiParameterFlt("focus_distance", 1500.0); // in mm
    AiParameterInt("aperture_blades", 0);
    AiParameterFlt("aperture_colorshift", 0.0);
    AiParameterBool("dof", true);
    AiParameterInt("backward_samples", 1000);
    AiParameterFlt("minimum_rgb", 3.0f);
    AiParameterStr("bokeh_exr_path", "Users/zeno/pota/tests/image/pota_bokeh.exr");
}


node_initialize
{
	AiCameraInitialize(node);
	AiNodeSetLocalData(node, new MyCameraData());
}


node_update
{	

	MyCameraData* camera_data = (MyCameraData*)AiNodeGetLocalData(node);

	/*
	DRAW_ONLY({
		drawData &dd = camera_data->draw;

        // create file to transfer data to python drawing module
        dd.myfile.open(DRAW_DIRECTORY + "draw.pota", std::ofstream::out | std::ofstream::trunc);
        dd.myfile << "RAYS{";
    })
    */


	AiMsgInfo("%s  [POTA] ----------  LENS CONSTANTS  -----------", emoticon);
	AiMsgInfo("%s  [POTA] lens_length: %s", emoticon, lens_name);
    AiMsgInfo("%s  [POTA] lens_outer_pupil_radius: %f", emoticon, lens_outer_pupil_radius);
    AiMsgInfo("%s  [POTA] lens_inner_pupil_radius: %f", emoticon, lens_inner_pupil_radius);
    AiMsgInfo("%s  [POTA] lens_length: %f", emoticon, lens_length);
	AiMsgInfo("%s  [POTA] lens_focal_length: %f", emoticon, lens_focal_length);
	AiMsgInfo("%s  [POTA] lens_aperture_pos: %f", emoticon, lens_aperture_pos);
	AiMsgInfo("%s  [POTA] lens_aperture_housing_radius: %f", emoticon, lens_aperture_housing_radius);
	AiMsgInfo("%s  [POTA] lens_outer_pupil_curvature_radius: %f", emoticon, lens_outer_pupil_curvature_radius);
	AiMsgInfo("%s  [POTA] lens_focal_length: %f", emoticon, lens_focal_length);
	AiMsgInfo("%s  [POTA] lens_field_of_view: %f", emoticon, lens_field_of_view);
	AiMsgInfo("%s  [POTA] --------------------------------------", emoticon);

	camera_data->sensor_width = AiNodeGetFlt(node, "sensor_width");
	camera_data->fstop = AiNodeGetFlt(node, "fstop");
	camera_data->focus_distance = AiNodeGetFlt(node, "focus_distance");
	camera_data->lensModel = (LensModel) AiNodeGetInt(node, "lensModel");
	camera_data->aperture_blades = AiNodeGetInt(node, "aperture_blades");
	camera_data->dof = AiNodeGetBool(node, "dof");
	camera_data->aperture_colorshift = AiNodeGetFlt(node, "aperture_colorshift");
	camera_data->backward_samples = AiNodeGetInt(node, "backward_samples");
	camera_data->minimum_rgb = AiNodeGetFlt(node, "minimum_rgb");
	camera_data->bokeh_exr_path = AiNodeGetStr(node, "bokeh_exr_path");
	
	camera_data->lambda = .55f;


	camera_data->max_fstop = lens_focal_length / (lens_aperture_housing_radius * 2.0f);
	AiMsgInfo("%s  [POTA] lens wide open f-stop: %f", emoticon, camera_data->max_fstop);

	if (camera_data->fstop == 0.0f){
		camera_data->aperture_radius = lens_aperture_housing_radius;
	} else {
		camera_data->aperture_radius = fminf(lens_aperture_housing_radius, lens_focal_length / (2.0f * camera_data->fstop));
	}

	AiMsgInfo("%s  [POTA] full aperture radius: %f", emoticon, lens_aperture_housing_radius);
	AiMsgInfo("%s  [POTA] fstop-calculated aperture radius: %f", emoticon, camera_data->aperture_radius);
	AiMsgInfo("%s  [POTA] --------------------------------------", emoticon);

	// focus test, calculate sensor shift for correct focusing
    AiMsgInfo("%s  [POTA] calculating sensor shift at infinity focus:", emoticon);
	float infinity_focus_sensor_shift = camera_set_focus(AI_BIG, lens_aperture_housing_radius, camera_data->lambda);

    AiMsgInfo("%s  [POTA] calculating sensor shift at focus distance:", emoticon);
	camera_data->sensor_shift = camera_set_focus(camera_data->focus_distance, lens_aperture_housing_radius, camera_data->lambda);
	AiMsgInfo("%s  [POTA] sensor_shift to focus at infinity: %f", emoticon, infinity_focus_sensor_shift);
	AiMsgInfo("%s  [POTA] sensor_shift to focus at %f: %f", emoticon, camera_data->focus_distance, camera_data->sensor_shift);

	/*
	camera_data->rays_succes = 0;
	camera_data->rays_fail = 0;
	*/

	/*
	DRAW_ONLY({
		camera_data->min_intersection_distance = 999999.0;
		camera_data->max_intersection_distance = -999999.0;
    })
    */


	AiCameraUpdate(node, false);
}


node_finish
{

	MyCameraData* camera_data = (MyCameraData*)AiNodeGetLocalData(node);

	/*
	AiMsgInfo("%s  [POTA] rays passed %d", emoticon, camera_data->rays_succes);
	AiMsgInfo("%s  [POTA] rays blocked %d", emoticon, camera_data->rays_fail);
	AiMsgInfo("%s  [POTA] rays blocked percentage %f", emoticon, (float)camera_data->rays_fail / (float)(camera_data->rays_succes + camera_data->rays_fail));
	*/


	/*
    DRAW_ONLY({

    	// debug only
	    float average_intersection_distance_normalized = camera_data->average_intersection_distance / (float)camera_data->average_intersection_distance_cnt;
	    std::cout << "average_intersection_distance: " << camera_data->average_intersection_distance << std::endl;
	    std::cout << "average_intersection_distance_cnt: " << camera_data->average_intersection_distance_cnt << std::endl;
	    std::cout << "average_intersection_distance_normalized: " << average_intersection_distance_normalized << std::endl;
	    std::cout << "max_intersection_distance: " << camera_data->max_intersection_distance << std::endl;
	    std::cout << "min_intersection_distance: " << camera_data->min_intersection_distance << std::endl;

    	drawData &dd = camera_data->draw;
    	dd.myfile << "}";
        dd.myfile.close();
    })
    */

}




camera_create_ray
{
	MyCameraData* camera_data = (MyCameraData*)AiNodeGetLocalData(node);
	// drawData &dd = camera_data->draw;

    DRAW_ONLY({
    	/*
        if (dd.counter == 50000){
            dd.draw = true;
            dd.counter = 0;
    	}

    	int countlimit = 500000;
		++camera_data->counter;
		*/
    })
	

  // polynomial optics

    // CHROMATIC ABERRATION STRATEGY
    // no:  trace 1 ray at .55f
    // yes: trace 3 rays at different wavelengths (CIE RGB) -> 3x more expensive :(

    float sensor[5] = {0.0f};
    float aperture[5] = {0.0f};
    float out[5] = {0.0f};
    sensor[4] = camera_data->lambda;

    // set sensor position coords
    sensor[0] = input.sx * (camera_data->sensor_width * 0.5f);
    sensor[1] = input.sy * (camera_data->sensor_width * 0.5f);

    AtVector2 sensor_position_original(sensor[0], sensor[1]);
    bool ray_succes = false;
    int tries = 0;
    int max_tries = 20;

	/*
    // chromatic aberration
    AtRGB aperture_left(0.5f, 0.5f, 0);
    AtRGB aperture_center(0, 0.5f, 0.5f);
    AtRGB aperture_right(0.5f, 0, 0.5f);
    int index = 0;
    float shift_x = camera_data->aperture_colorshift * std::abs(sensor[0]);
    float shift_y = camera_data->aperture_colorshift * std::abs(sensor[1]);
	AtVector2 ca_aperture_sample(input.lensx, input.lensy);
    
    if (input.lensx < 0.33f) {
        index = -1;
        ca_aperture_sample.x /= 0.33f;
    } else if (input.lensx < 0.66f) {
        index = 0;
        ca_aperture_sample.x -= 0.33f;
        ca_aperture_sample.x /= 0.33f;
    } else {
        index = 1;
        ca_aperture_sample.x -= 0.66f;
        ca_aperture_sample.x /= 0.33f;
    }
	*/

    /* 
    // for visual debugging, rays should converge to single point in idealised lens
    	sensor[0] = 0.0f;
    	sensor[1] = 0.0f;
	*/

    while(ray_succes == false && tries <= max_tries){

    	//reset the initial sensor coords
    	sensor[0] = sensor_position_original.x; 
    	sensor[1] = sensor_position_original.y; 
    	sensor[2] = 0.0f; sensor[3] = 0.0f; 
    	sensor[4] = camera_data->lambda;
	    aperture[0] = 0.0f; aperture[1] = 0.0f; aperture[2] = 0.0f; aperture[3] = 0.0f; aperture[4] = 0.0f;
	    out[0] = 0.0f; out[1] = 0.0f; out[2] = 0.0f; out[3] = 0.0f; out[4] = 0.0f;
	    
	    if (!camera_data->dof) // no dof, all rays through single aperture point
	    { 
	    	aperture[0] = 0.0;
	    	aperture[1] = 0.0;

	    } 
	    else if (camera_data->dof && camera_data->aperture_blades < 2)
	    {
			// transform unit square to unit disk
		    AtVector2 unit_disk(0.0f, 0.0f);
		    //if(camera_data->aperture_colorshift > 0.0f) concentric_disk_sample(ca_aperture_sample.x, ca_aperture_sample.y, &unit_disk, true);
		    //else 
		    if (tries == 0) concentric_disk_sample(input.lensx, input.lensy, unit_disk, false);
		    else concentric_disk_sample(xor128() / 4294967296.0f, xor128() / 4294967296.0f, unit_disk, true);

		    aperture[0] = unit_disk.x * camera_data->aperture_radius;
		    aperture[1] = unit_disk.y * camera_data->aperture_radius;
			/*
		    // hacky chromatic aberration
		    if(camera_data->aperture_colorshift > 0.0f)
		    {	
			    AtVector sample3d(index * shift_x + aperture[0], index * shift_y + aperture[1], 0);    
			    AtVector left_center(-shift_x, -shift_y, 0);
			    AtVector middle_center(0, 0, 0);
			    AtVector right_center(shift_x, shift_y, 0);
		    	output.weight = 0.0f;

			    if (AiV3Length(left_center - sample3d) < camera_data->aperture_radius) output.weight += aperture_left;
			    if (AiV3Length(middle_center - sample3d) < camera_data->aperture_radius) output.weight += aperture_center;
		    	if (AiV3Length(right_center - sample3d) < camera_data->aperture_radius) output.weight += aperture_right;

		    	aperture[0] = sample3d.x;
		    	aperture[1] = sample3d.y;
	    	}
			*/
	    } 
	    else if (camera_data->dof && camera_data->aperture_blades > 2)
	    {
	    	lens_sample_aperture(&aperture[0], &aperture[1], input.lensx, input.lensy, camera_data->aperture_radius, camera_data->aperture_blades);
	    }


	    if (camera_data->dof)
	    {
	    	// aperture sampling, to make sure ray is able to propagate through whole lens system
	    	lens_pt_sample_aperture(sensor, aperture, camera_data->sensor_shift);
	    }
	    

	    // move to beginning of polynomial
		sensor[0] += sensor[2] * camera_data->sensor_shift; //sensor.pos.x = sensor.dir.x * sensor_shift
		sensor[1] += sensor[3] * camera_data->sensor_shift; //sensor.pos.y = sensor.dir.y * sensor_shift


		// propagate ray from sensor to outer lens element
	    float transmittance = lens_evaluate(sensor, out);
		if(transmittance <= 0.0f){
			++tries;
			continue;
		}

		// crop out by outgoing pupil
		if( out[0]*out[0] + out[1]*out[1] > lens_outer_pupil_radius*lens_outer_pupil_radius){
			++tries;
			continue;
		}

		// crop at inward facing pupil
		const float px = sensor[0] + sensor[2] * lens_focal_length;
		const float py = sensor[1] + sensor[3] * lens_focal_length; //(note that lens_focal_length is the back focal length, i.e. the distance unshifted sensor -> pupil)
		if (px*px + py*py > lens_inner_pupil_radius*lens_inner_pupil_radius){
			++tries;
			continue;
		}	
		
		ray_succes = true;
	}

	if (ray_succes == false) output.weight = 0.0f;

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


    // EXPERIMENTAL, I KNOW IT IS INCORRECT BUT AT LEAST THE VISUAL PROBLEM IS RESOLVED
    // NOT CALCULATING THE DERIVATIVES PROBS AFFECTS TEXTURE I/O
    if (tries > 0){
        output.dOdy = output.origin;
        output.dDdy = output.dir; 
    }



	/* NOT NEEDED FOR ARNOLD, GOOD INFO THOUGH
	// now let's go world space:
	// initialise an ONB/a frame around the first vertex at the camera position along n=camera lookat direction:

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

	// now need to rotate the normal of the frame, in case you need any cosines later in light transport. if not, leave out:
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
			camera_data->average_intersection_distance += rayplaneintersect.z;
			++camera_data->average_intersection_distance_cnt;
		}

		if(rayplaneintersect.z > camera_data->max_intersection_distance){
			camera_data->max_intersection_distance = rayplaneintersect.z;
		}

		if(rayplaneintersect.z < camera_data->min_intersection_distance){
			camera_data->min_intersection_distance = rayplaneintersect.z;
		}
	}
    
	*/

	/*
    if (camera_data->counter == countlimit){
    	camera_data->counter = 0;
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