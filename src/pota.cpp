#include <ai.h>
#include "pota.h"
#include "lens.h"
#include <vector>
#include <cmath>


AI_CAMERA_NODE_EXPORT_METHODS(potaMethods)


enum
{
	p_lensModel,
	p_sensor_width,
	p_wavelength,
	p_dof,
    p_fstop,
    p_focus_distance,
    p_extra_sensor_shift,
    p_vignetting_retries,
    p_aperture_blades,
    p_backward_samples,
    p_minimum_rgb,
    p_bokeh_exr_path,
    p_proper_ray_derivatives
};


// to switch between lens models in interface dropdown
static const char* LensModelNames[] =
{
    "takumar_1969",
    "zeiss_biotar_1927",
    "fisheye",
    "fisheye_aspherical",
    "double_gauss",
    "double_gauss_angenieux",
    "petzval",
    "tessar_anamorphic",
    "wideangle",
    "canon_anamorphic",
    NULL
};


// returns sensor offset in mm
// traces rays backwards through the lens
float camera_set_focus(float dist, MyCameraData *camera_data)
{
    const float target[3] = { 0.0, 0.0, dist};
    float sensor[5] = {0.0f};
    float out[5] = {0.0f};
    sensor[4] = camera_data->lambda;
    float offset = 0.0f;
    int count = 0;
    float scale_samples = 0.1f;
    float aperture[2] = {0.0f, 0.0f};

    const int S = 4;

    // trace a couple of adjoint rays from there to the sensor and
    // see where we need to put the sensor plane.
    for(int s=1; s<=S; s++){
      for(int k=0; k<2; k++){
      	
        // reset aperture
        aperture[0] = aperture[1] = 0.0f;

        aperture[k] = camera_data->lens_aperture_housing_radius * (s/(S+1.0f) * scale_samples); // (1to4)/(4+1) = (.2, .4, .6, .8) * scale_samples

        lens_lt_sample_aperture(target, aperture, sensor, out, camera_data->lambda, camera_data);

        if(sensor[2+k] > 0){
            offset += sensor[k]/sensor[2+k];
            AiMsgInfo("\t[POTA] raytraced sensor shift at aperture[%f, %f]: %f", aperture[0], aperture[1], sensor[k]/sensor[2+k]);
            count ++;
        }
      }
    }

    // average guesses
    offset /= count; 
    
    // the focus plane/sensor offset:
    // negative because of reverse direction
    if(offset == offset){ // check NaN cases
		const float limit = 45.0f;
		if(offset > limit){
			AiMsgInfo("[POTA] sensor offset bigger than maxlimit: %f > %f", offset, limit);
          	return limit;
		} else if(offset < -limit){
			AiMsgInfo("[POTA] sensor offset smaller than minlimit: %f < %f", offset, -limit);
        	return -limit;
		} else {
			return offset; // in mm
		}
    }
}



// returns sensor offset in mm
float camera_set_focus_infinity(MyCameraData *camera_data)
{
	float parallel_ray_height = camera_data->lens_aperture_housing_radius * 0.1;
    const float target[3] = { 0.0, parallel_ray_height, AI_BIG};
    float sensor[5] = {0.0f};
    float out[5] = {0.0f};
    sensor[4] = camera_data->lambda;
    float offset = 0.0f;
    int count = 0;

    // just point through center of aperture
    float aperture[2] = {0.0f, parallel_ray_height};

    const int S = 4;

    // trace a couple of adjoint rays from there to the sensor and
    // see where we need to put the sensor plane.
    for(int s=1; s<=S; s++){
      for(int k=0; k<2; k++){
      	
        // reset aperture
        aperture[0] = 0.0f;
        aperture[1] = parallel_ray_height;

        lens_lt_sample_aperture(target, aperture, sensor, out, camera_data->lambda, camera_data);

        if(sensor[2+k] > 0){
            offset += sensor[k]/sensor[2+k];
            count ++;
        }
      }
    }

    offset /= count; 
    
    // check NaN cases
    if(offset == offset) return offset;
}


std::vector<float> logarithmic_values ()
{
    float min = 0.0;
    float max = 45.0;
    float exponent = 2.0; // sharpness
    std::vector<float> log;

    for(float i = -1.0; i <= 1.0; i += 0.0001) {
        log.push_back(std::pow(i, exponent) * (max - min) + min);
    }

    return log;
}



// line plane intersection with fixed intersection at y = 0, for finding the focal length and sensor shift
AtVector line_plane_intersection(AtVector rayOrigin, AtVector rayDirection)
{
    AtVector coord(100.0, 0.0, 100.0);
    AtVector planeNormal(0.0, 1.0, 0.0);
    rayDirection = AiV3Normalize(rayDirection);
    coord = AiV3Normalize(coord);
    return rayOrigin + (rayDirection * (AiV3Dot(coord, planeNormal) - AiV3Dot(planeNormal, rayOrigin)) / AiV3Dot(planeNormal, rayDirection));
}


void camera_get_y0_intersection_distance(float sensor_shift, float &intersection_distance, MyCameraData *camera_data)
{
	float sensor[5] = {0.0f};
    float aperture[5] = {0.0f};
    float out[5] = {0.0f};
    sensor[4] = camera_data->lambda;
    aperture[1] = camera_data->lens_aperture_housing_radius * 0.1;

    lens_pt_sample_aperture(sensor, aperture, sensor_shift, camera_data);

    sensor[0] += sensor[2] * sensor_shift;
	sensor[1] += sensor[3] * sensor_shift;

	float transmittance = lens_evaluate(sensor, out, camera_data);

	// convert from sphere/sphere space to camera space
	float camera_space_pos[3];
	float camera_space_omega[3];
	lens_sphereToCs(out, out+2, camera_space_pos, camera_space_omega, -camera_data->lens_outer_pupil_curvature_radius, camera_data->lens_outer_pupil_curvature_radius);

	AtVector ray_origin(camera_space_pos[0], camera_space_pos[1], camera_space_pos[2]);
	AtVector ray_dir(camera_space_omega[0], camera_space_omega[1], camera_space_omega[2]);

    intersection_distance = line_plane_intersection(ray_origin, ray_dir).z;

    ray_origin *= -0.1;
    ray_dir *= -0.1;

}


// focus_distance is in mm
void logarithmic_focus_search(const float focus_distance, float &best_sensor_shift, float &closest_distance, MyCameraData *camera_data){
    std::vector<float> log = logarithmic_values();

    for (float sensorshift : log){
    	float intersection_distance = 0.0;
        //AiMsgInfo("sensorshift: %f", sensorshift);

        camera_get_y0_intersection_distance(sensorshift, intersection_distance, camera_data);
        //AiMsgInfo("intersection_distance: %f at sensor_shift: %f", intersection_distance, sensorshift);
        float new_distance = focus_distance - intersection_distance;

        if (new_distance < closest_distance && new_distance > 0.0){
            closest_distance = new_distance;
            best_sensor_shift = sensorshift;
            //AiMsgInfo("best_sensor_shift: %f", best_sensor_shift);
        }
    }
}



inline bool trace_ray_focus_check(float sensor_shift, MyCameraData *camera_data)
{

    float sensor[5] = {0.0f};
    float aperture[5] = {0.0f};
    float out[5] = {0.0f};
    sensor[4] = camera_data->lambda;
    aperture[1] = camera_data->lens_aperture_housing_radius * 0.1;

	lens_pt_sample_aperture(sensor, aperture, sensor_shift, camera_data);

    // move to beginning of polynomial
	sensor[0] += sensor[2] * sensor_shift;
	sensor[1] += sensor[3] * sensor_shift;


	// propagate ray from sensor to outer lens element
    float transmittance = lens_evaluate(sensor, out, camera_data);
    if(transmittance <= 0.0f){
        return false;
    }


    // crop out by outgoing pupil
    if( out[0]*out[0] + out[1]*out[1] > camera_data->lens_outer_pupil_radius*camera_data->lens_outer_pupil_radius){
        return false;
    }


    // crop at inward facing pupil
    const float px = sensor[0] + sensor[2] * camera_data->lens_focal_length;
    const float py = sensor[1] + sensor[3] * camera_data->lens_focal_length;
    if (px*px + py*py > camera_data->lens_inner_pupil_radius*camera_data->lens_inner_pupil_radius){
        return false;
    }


	// convert from sphere/sphere space to camera space
	float camera_space_pos[3];
	float camera_space_omega[3];
	lens_sphereToCs(out, out+2, camera_space_pos, camera_space_omega, -camera_data->lens_outer_pupil_curvature_radius, camera_data->lens_outer_pupil_curvature_radius);

    AtVector origin ( camera_space_pos[0], camera_space_pos[1], camera_space_pos[2] );
    AtVector direction ( camera_space_omega[0], camera_space_omega[1], camera_space_omega[2] );

    float y0 = line_plane_intersection(origin, direction).z;
    AiMsgInfo("[POTA] y=0 ray plane intersection: %f", y0);

	origin *= -0.1; // convert to cm
    direction *= -0.1; //reverse rays and convert to cm

    return true;
}





inline void trace_ray(bool original_ray, int &tries, const float input_sx, const float input_sy, const float input_lensx, const float input_lensy, float &r1, float &r2, AtRGB &weight, AtVector &origin, AtVector &direction, MyCameraData *camera_data)
{

    bool ray_succes = false;
    tries = 0;
    float sensor[5] = {0.0f};
    float aperture[5] = {0.0f};
    float out[5] = {0.0f};

    while(ray_succes == false && tries <= camera_data->vignetting_retries){

    	// set sensor position coords
	    sensor[0] = input_sx * (camera_data->sensor_width * 0.5f);
	    sensor[1] = input_sy * (camera_data->sensor_width * 0.5f);
    	sensor[2] = sensor[3] = 0.0f;
	    sensor[4] = camera_data->lambda;

	    aperture[0] = aperture[1] = aperture[2] = aperture[3]  = aperture[4] = 0.0f;
	    out[0] = out[1] = out[2] = out[3] = out[4] = 0.0f;

	    
	    if (!camera_data->dof) // no dof, all rays through single aperture point
	    { 
	    	aperture[0] = aperture[1] = 0.0;

	    } 
	    else if (camera_data->dof && camera_data->aperture_blades < 2)
	    {
			// transform unit square to unit disk
		    AtVector2 unit_disk(0.0f, 0.0f);
		    if (tries == 0) concentric_disk_sample(input_lensx, input_lensy, unit_disk, false);
		    else {
		    	if (original_ray){
				    r1 = xor128() / 4294967296.0f;
				    r2 = xor128() / 4294967296.0f;
			    }

		    	concentric_disk_sample(r1, r2, unit_disk, true);
		    }

            aperture[0] = unit_disk.x * camera_data->aperture_radius;
            aperture[1] = unit_disk.y * camera_data->aperture_radius;
	    } 
	    else if (camera_data->dof && camera_data->aperture_blades > 2)
	    {
	    	if (tries == 0) lens_sample_aperture(&aperture[0], &aperture[1], input_lensx, input_lensy, camera_data->aperture_radius, camera_data->aperture_blades);
	    	else {
	    		if (original_ray)
	    		{
		    		r1 = xor128() / 4294967296.0f;
		    		r2 = xor128() / 4294967296.0f;
	    		}

	    		lens_sample_aperture(&aperture[0], &aperture[1], r1, r2, camera_data->aperture_radius, camera_data->aperture_blades);
	    	}
	    }

	    if (camera_data->dof)
	    {
	    	// aperture sampling, to make sure ray is able to propagate through whole lens system
	    	lens_pt_sample_aperture(sensor, aperture, camera_data->sensor_shift, camera_data);
	    }
	    

	    // move to beginning of polynomial
		sensor[0] += sensor[2] * camera_data->sensor_shift;
		sensor[1] += sensor[3] * camera_data->sensor_shift;


		// propagate ray from sensor to outer lens element
	    float transmittance = lens_evaluate(sensor, out, camera_data);
		if(transmittance <= 0.0f){
			++tries;
			continue;
		}


		// crop out by outgoing pupil
		if( out[0]*out[0] + out[1]*out[1] > camera_data->lens_outer_pupil_radius*camera_data->lens_outer_pupil_radius){
			++tries;
			continue;
		}


		// crop at inward facing pupil
		const float px = sensor[0] + sensor[2] * camera_data->lens_focal_length;
		const float py = sensor[1] + sensor[3] * camera_data->lens_focal_length; //(note that lens_focal_length is the back focal length, i.e. the distance unshifted sensor -> pupil)
		if (px*px + py*py > camera_data->lens_inner_pupil_radius*camera_data->lens_inner_pupil_radius){
			++tries;
			continue;
		}
		
		ray_succes = true;
	}

	if (ray_succes == false) weight = 0.0f;


	// convert from sphere/sphere space to camera space
	float camera_space_pos[3];
	float camera_space_omega[3];
	lens_sphereToCs(out, out+2, camera_space_pos, camera_space_omega, -camera_data->lens_outer_pupil_curvature_radius, camera_data->lens_outer_pupil_curvature_radius);

    origin = {camera_space_pos[0], camera_space_pos[1], camera_space_pos[2]};
    direction = {camera_space_omega[0], camera_space_omega[1], camera_space_omega[2]};

	origin *= -0.1; // reverse rays and convert to cm
    direction *= -0.1; //reverse rays and convert to cm

    // Nan bailout
    if (origin.x != origin.x || origin.y != origin.y || origin.z != origin.z || 
        direction.x != direction.x || direction.y != direction.y || direction.z != direction.z)
    {
        weight = 0.0f;
    }

}



node_parameters
{
    AiParameterEnum("lensModel", petzval, LensModelNames);
    AiParameterFlt("sensor_width", 36.0); // 35mm film
    AiParameterFlt("wavelength", 550.0); // wavelength in nm
    AiParameterBool("dof", true);
    AiParameterFlt("fstop", 0.0);
    AiParameterFlt("focus_distance", 150.0); // in cm to be consistent with arnold core
    AiParameterFlt("extra_sensor_shift", 0.0); // tmp remove
    AiParameterInt("vignetting_retries", 15);
    AiParameterInt("aperture_blades", 0);
    AiParameterInt("backward_samples", 3);
    AiParameterFlt("minimum_rgb", 3.0f);
    AiParameterStr("bokeh_exr_path", "");
    AiParameterBool("proper_ray_derivatives", true);
}


node_initialize
{
	AiCameraInitialize(node);
	AiNodeSetLocalData(node, new MyCameraData());
}


node_update
{	
	MyCameraData* camera_data = (MyCameraData*)AiNodeGetLocalData(node);

	camera_data->sensor_width = AiNodeGetFlt(node, "sensor_width");
	camera_data->fstop = AiNodeGetFlt(node, "fstop");
	camera_data->focus_distance = AiNodeGetFlt(node, "focus_distance") * 10.0f;
	camera_data->lensModel = (LensModel) AiNodeGetInt(node, "lensModel");
	camera_data->aperture_blades = AiNodeGetInt(node, "aperture_blades");
	camera_data->dof = AiNodeGetBool(node, "dof");
    camera_data->vignetting_retries = AiNodeGetInt(node, "vignetting_retries");
	camera_data->backward_samples = AiNodeGetInt(node, "backward_samples");
	camera_data->minimum_rgb = AiNodeGetFlt(node, "minimum_rgb");
	camera_data->bokeh_exr_path = AiNodeGetStr(node, "bokeh_exr_path");
	camera_data->proper_ray_derivatives = AiNodeGetBool(node, "proper_ray_derivatives");
	camera_data->sensor_shift = 0.0;


    AiMsgInfo("");

	load_lens_constants(camera_data);

	camera_data->lambda = AiNodeGetFlt(node, "wavelength") * 0.001;
	AiMsgInfo("[POTA] wavelength: %f", camera_data->lambda);

	camera_data->max_fstop = camera_data->lens_focal_length / (camera_data->lens_aperture_housing_radius * 2.0f);
	AiMsgInfo("[POTA] lens wide open f-stop: %f", camera_data->max_fstop);

	if (camera_data->fstop == 0.0f) camera_data->aperture_radius = camera_data->lens_aperture_housing_radius;
	else camera_data->aperture_radius = fminf(camera_data->lens_aperture_housing_radius, camera_data->lens_focal_length / (2.0f * camera_data->fstop));

	AiMsgInfo("[POTA] full aperture radius: %f", camera_data->lens_aperture_housing_radius);
	AiMsgInfo("[POTA] fstop-calculated aperture radius: %f", camera_data->aperture_radius);
	AiMsgInfo("[POTA] --------------------------------------");


	AiMsgInfo("[POTA] focus distance: %f", camera_data->focus_distance);

    /*
    AiMsgInfo("[POTA] calculating sensor shift at focus distance:");
	camera_data->sensor_shift = camera_set_focus(camera_data->focus_distance, camera_data);
	AiMsgInfo("[POTA] sensor_shift to focus at %f: %f", camera_data->focus_distance, camera_data->sensor_shift);
    */

	// logartihmic focus search
    float best_sensor_shift = 0.0f;
    float closest_distance = AI_BIG;
    logarithmic_focus_search(camera_data->focus_distance, best_sensor_shift, closest_distance, camera_data);
	AiMsgInfo("[POTA] sensor_shift using logarithmic search: %f", best_sensor_shift);
	camera_data->sensor_shift = best_sensor_shift + AiNodeGetFlt(node, "extra_sensor_shift");

    /*
	// average guesses infinity focus search
    float infinity_focus_sensor_shift = camera_set_focus(AI_BIG, camera_data);
    AiMsgInfo("[POTA] sensor_shift [average guesses backwards light tracing] to focus at infinity: %f", infinity_focus_sensor_shift);
    */

	// logarithmic infinity focus search
	float best_sensor_shift_infinity = 0.0f;
	float closest_distance_infinity = AI_BIG;
    logarithmic_focus_search(AI_BIG, best_sensor_shift_infinity, closest_distance_infinity, camera_data);
    AiMsgInfo("[POTA] sensor_shift [logarithmic forward tracing] to focus at infinity: %f", best_sensor_shift_infinity);
    
    // bidirectional parallel infinity focus search
    float infinity_focus_parallel_light_tracing = camera_set_focus_infinity(camera_data);
    AiMsgInfo("[POTA] sensor_shift [parallel backwards light tracing] to focus at infinity: %f", infinity_focus_parallel_light_tracing);


    // double check where y=0 intersection point is, should be the same as focus distance
    if(!trace_ray_focus_check(camera_data->sensor_shift, camera_data)){
        AiMsgWarning("[POTA] focus check failed. Either the lens system is not correct, or the sensor is placed at a wrong distance.");
    }

    AiMsgInfo("");
	AiCameraUpdate(node, false);
}


node_finish
{

	MyCameraData* camera_data = (MyCameraData*)AiNodeGetLocalData(node);
	delete camera_data;
}


camera_create_ray
{
	MyCameraData* camera_data = (MyCameraData*)AiNodeGetLocalData(node);

	int tries = 0;
	float random1 = 0.0;
	float random2 = 0.0;

    trace_ray(true, tries, input.sx, input.sy, input.lensx, input.lensy, random1, random2, output.weight, output.origin, output.dir, camera_data);

    // calculate new ray derivatives
    // sucks a bit to have to trace 3 rays.. Bit slow
    // is there an analytical solution to this?..
    if (tries > 0){
    	if (!camera_data->proper_ray_derivatives){
	        output.dOdy = output.origin;
	        output.dDdy = output.dir;
		} else {
			float step = 0.001;
		    AtCameraInput input_dx = input;
		    AtCameraInput input_dy = input;
		    AtCameraOutput output_dx;
		    AtCameraOutput output_dy;

		    input_dx.sx += input.dsx * step;
		    input_dy.sy += input.dsy * step;

		    trace_ray(false, tries, input_dx.sx, input_dx.sy, random1, random2, random1, random2, output_dx.weight, output_dx.origin, output_dx.dir, camera_data);
		    trace_ray(false, tries, input_dy.sx, input_dy.sy, random1, random2, random1, random2, output_dy.weight, output_dy.origin, output_dy.dir, camera_data);

		    output.dOdx = (output_dx.origin - output.origin) / step;
			output.dOdy = (output_dy.origin - output.origin) / step;
			output.dDdx = (output_dx.dir - output.dir) / step;
			output.dDdy = (output_dy.dir - output.dir) / step;
		}
    }



	/* 

	NOT NEEDED FOR ARNOLD (convert rays from camera space to world space), GOOD INFO THOUGH FOR OTHER RENDER ENGINES
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
	const float R = camera_data->lens_outer_pupil_curvature_radius;
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