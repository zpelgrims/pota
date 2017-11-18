#include <ai.h>
#include <iostream>
#include <string.h>
#include <algorithm>


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


// need to write some default value as the lens name for it to be included
#include "../polynomialOptics/render/lens.h"



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
			AiMsgInfo("[POTA] sensor offset inside of limits: %f > %f", offset, offset);
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
    AiParameterFlt("focusDistance", .2); // 200cm in mm

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

    //data->fov = 2.0f * atan((data->sensorWidth / (2.0f * data->focalLength))); // in radians
    //data->tan_fov = tanf(data->fov / 2.0f);
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

    // need to generate a random lambda, or start 550nm and then see the shift?
    // transmittance is probably just a multiplier on the wavelength
    float lambda = 0.55f; // 550 nanometers
    float in[5] = {0.0f};
    float out[5] = {0.0f};
    in[4] = lambda;

    // set sensor coords
    // might be wrong
    in[0] = input.sx * (data->sensorWidth * 0.5f);
    in[1] = input.sy * (data->sensorHeight * 0.5f);

    if(data->counter == countlimit){
    	std:: cout << "input.sx, input.sy: " << input.sx << ", " << input.sy << std::endl;
		std::cout << "in[0, 1]: " << in[0] << ", " << in[1] << std::endl;
	}

    /*
	const float xres = 36.0;
	const float yres = 24.0;

	AtVector2 s(input.sx/xres, input.sy/yres);

	float diagonal = std::sqrt(xres*xres + yres*yres);
	float aspect = (float)yres / (float)xres;
	float x = std::sqrt(diagonal * diagonal / (1 + aspect * aspect));
	float y = aspect * x;

	AtVector2 physicalExtend1(-x/2, -y/2);
	AtVector2 physicalExtend2(x/2, y/2);
	AtVector2 pMin(fmin(physicalExtend1.x, physicalExtend2.x), fmin(physicalExtend1.y, physicalExtend2.y));
	AtVector2 pMax(fmax(physicalExtend1.x, physicalExtend2.x), fmax(physicalExtend1.y, physicalExtend2.y));

	AtVector2 pFilm2(Lerp(s.x, pMin.x, pMax.x), Lerp(s.y, pMin.y, pMax.y));
	AtVector pFilm(-pFilm2.x, pFilm2.y, 0);

	in[0] = pFilm.x;
	in[1] = pFilm.y;

	if(data->counter == countlimit){
		std::cout << "pFilm.x: " << pFilm.x << ", pFilm.y: " << pFilm.y << std::endl;
	}*/
	
	

    //in[0] = input.sx * tanf(data->fov);
    //in[1] = input.sy * tanf(data->fov);
    // do I not need to set the origin.z? Or should i translate the sensor later on?


    // sample a point on the lens, supply the -1 to 1 random coordinates?
    // ideally use the supplied function: lens_sample_aperture(float *x, float *y, float r1, float r2, const float radius, const int blades)
    // but for now concentric mapping will do
    AtVector2 lens(0.0f, 0.0f);
    concentricDiskSample(input.lensx, input.lensy, &lens);

    //lens = (lens*2.0f) - 0.5f; // transform to -1 to 1 coords?

    // think i should be scaling by the sensor radius
    lens *= lens_inner_pupil_radius; //replace this for radius calculated using fstop, for now the aperture is fully open
    out[0] = lens.x;
    out[1] = lens.y;



    // I BELIEVE ITS LENS_PT_SAMPLE_APERTURE THAT IS FUCKING THINGS UP

    // solves for the two directions [dx,dy], keeps the two positions [x,y] and the
	// wavelength, such that the path through the lens system will be valid, i.e.
	// lens_evaluate_aperture(in, out) will yield the same out given the solved for in.
	// in: point on sensor. out: point on aperture.
	//lens_pt_sample_aperture(in, out, data->focusDistance);
	lens_pt_sample_aperture(in, out, data->focusDistance);
	if(data->counter == countlimit){
		std::cout << "lens_pt_sample_aperture" << std::endl;
		std::cout << "\tin[0, 1](pos): " << in[0] << ", " << in[1] << std::endl;
		std::cout << "\tin[2, 3](dir): " << in[2] << ", " << in[3] << std::endl;
		std::cout << "\tin[4](lambda): " << in[4] << std::endl;

		std::cout << std::endl;
		std::cout << "\tout[0, 1] (pos): " << out[0] << ", " << out[1] << std::endl;
		std::cout << "\tout[2, 3](dir): " << out[2] << ", " << out[3] << std::endl;
		std::cout << "\tout[4](lambda): " << out[4] << std::endl;
	}

	/*
	lens_evaluate_aperture(in, out);
	if(data->counter == countlimit){
		std::cout << "AFTER LENS_EVALUATE_APERTURE" << std::endl;
		std::cout << "in[0]: " << in[0] << ", in[1]: " << in[1] << ", in[2]: " << in[2] << ", in[3]: " << in[3] << ", in[4]: " << in[4] << std::endl;
		std::cout << "out[0]: " << out[0] << ", out[1]: " << out[1] << ", out[2]: " << out[2] << ", out[3]: " << out[3] << ", out[4]: " << out[4] << std::endl;
	}*/



	// evaluates from sensor (in) to outer pupil (out).
	// input arrays are 5d [x,y,dx,dy,lambda] where dx and dy are the direction in
	// two-plane parametrization (that is the third component of the direction would be 1.0).
	// units are millimeters for lengths and micrometers for the wavelength (so visible light is about 0.4--0.7)
	// returns the transmittance computed from the polynomial.

	//returns negative values, hence clipped to 0.. Why? Up to values like -155161..
    float transmittance = lens_evaluate(in, out);
	if(data->counter == countlimit){
		std::cout << "lens_evaluate" << std::endl;
		std::cout << "\tin[0, 1](pos): " << in[0] << ", " << in[1] << std::endl;
		std::cout << "\tin[2, 3](dir): " << in[2] << ", " << in[3] << std::endl;
		std::cout << "\tin[4](lambda): " << in[4] << std::endl;

		std::cout << std::endl;
		std::cout << "\tout[0, 1] (pos): " << out[0] << ", " << out[1] << std::endl;
		std::cout << "\tout[2, 3](dir): " << out[2] << ", " << out[3] << std::endl;
		std::cout << "\tout[4](lambda): " << out[4] << std::endl;

		std::cout << std::endl;
		std::cout << "\ttransmittance: " << transmittance << std::endl;
	}

    // converts sphere/sphere coords to camera coords
    float inpos[2] = {in[0], in[1]};
    float indir[2] = {in[2], in[3]};
    float outpos[2] = {out[0], out[1]};
    float outdir[2] = {out[2], out[3]};

    // HOW TO QUERY sphereCenter?? maybe it's just -lens_outer_pupil_curvature_radius
    // lens_sphereToCs(const float *inpos, const float *indir, float *outpos, float *outdir, const float sphereCenter, const float sphereRad);
    lens_sphereToCs(inpos, indir, outpos, outdir, 0.0f, lens_outer_pupil_curvature_radius); // 0.0f could also be -lens_outer_pupil_curvature_radius
	if(data->counter == countlimit){
		std::cout << "lens_sphereToCs" << std::endl;
		std::cout << "\tin[0, 1](pos): " << in[0] << ", " << in[1] << std::endl;
		std::cout << "\tin[2, 3](dir): " << in[2] << ", " << in[3] << std::endl;

		std::cout << std::endl;
		std::cout << "\toutpos[0, 1]: " << outpos[0] << ", " << outpos[1] << std::endl;
		std::cout << "\toutdir[0, 1]: " << outdir[0] << ", " << outdir[1] << std::endl;
		std::cout << std::endl;
	}



    output.origin.x = outpos[0];
    output.origin.y = outpos[1];
    //output.origin.z = data->sensorShift; // not sure if this should be done 
    output.origin.z = 0.0; // not sure if this should be done here

    output.dir.x = outdir[0];
    output.dir.y = outdir[1];
    output.dir.z = -1.0f; // NOT SURE IF CORRECT

    // convert wavelength shift into rgb shift
    output.weight = 1.0f;


    
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