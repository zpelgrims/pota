#include <ai.h>
#include <string.h>

#include "../polynomialOptics/render/lens.h"


AI_CAMERA_NODE_EXPORT_METHODS(potaMethods)

enum
{
	p_sensorWidth,
    p_sensorHeight,
    p_focalLength,
    p_fStop,
    p_focalDistance
};

struct MyCameraData
{
	float fov;
	float tan_fov;
	float sensorWidth;
	float sensorHeight;
	float focalLength;
	float fStop;
	float focalDistance;
	float apertureRadius;
	float sensorShift;
};


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
    const float dm2mm = 100.0f; //default is 100.0 (dmtomm)

    // camera space vector to v1:
    const float target[3] = { 0.0, 0.0, dist }; //dist*dm2mm

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
			return offset; // in mm
		}
    }
}


node_parameters
{
    AiParameterFlt("sensorWidth", 36.0); // 35mm film
    AiParameterFlt("sensorHeight", 24.0); // 35 mm film
    AiParameterFlt("focalLength", 14.0); // in cm
    AiParameterFlt("fStop", 1.4);
    AiParameterFlt("focalDistance", 10000.0); // 100cm in mm
}


node_initialize
{
	AiCameraInitialize(node);
	AiNodeSetLocalData(node, new MyCameraData());
}


node_update
{
	MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);

	data->sensorWidth = AiNodeGetFlt(node, "sensorWidth");
	data->sensorHeight = AiNodeGetFlt(node, "sensorHeight");
	data->focalLength = AiNodeGetFlt(node, "focalLength");
	data->fStop = AiNodeGetFlt(node, "fStop");
	data->focalDistance = AiNodeGetFlt(node, "focalDistance");

    //data->fov = 2.0f * atan((data->sensorWidth / (2.0f * data->focalLength))); // in radians
    //data->tan_fov = tanf(data->fov / 2.0f);
    data->apertureRadius = (data->focalLength) / (2.0f * data->fStop);


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

	data->sensorShift = camera_set_focus(data->focalDistance, lens_inner_pupil_radius);
	AiMsgInfo("[POTA] sensor_shift to focus at %f: %f", data->focalDistance, data->sensorShift);

	AiCameraUpdate(node, false);
}

node_finish
{
	MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);
	delete data;
}

camera_create_ray
{
	const MyCameraData* data = (MyCameraData*)AiNodeGetLocalData(node);

  // polynomial optics

    // need to generate a random lambda, or start 550mm and then see the shift?
    // transmittance is probably just a multiplier on the wavelength
    float lambda = .550f;
    float in[5] = {0.0f};
    float out[5] = {0.0f};

    // set sensor coords
    // this shit is probably wrong as fuck
    in[0] = input.sx * (data->sensorWidth * 0.5);
    in[1] = input.sy * (data->sensorWidth * 0.5);
    //in[0] = input.sx * tanf(data->fov);
    //in[1] = input.sy * tanf(data->fov);
    // do I not need to set the origin.z? Or should i translate the sensor later on?


    // sample a point on the lens, supply the -1 to 1 random coordinates?
    // ideally use the supplied function: lens_sample_aperture(float *x, float *y, float r1, float r2, const float radius, const int blades)
    // but for now concentric mapping will do
    AtVector2 lens(0.0f, 0.0f);
    concentricDiskSample(input.lensx, input.lensy, &lens);

    // think i should be scaling by the sensor radius
    //lens *= lens_inner_pupil_radius; //replace this for radius calculated using fstop, for now the aperture is fully open
    //out[0] = lens.x;
    //out[1] = lens.y;
   	out[0] = lens.x * 0.05; //tmp
    out[1] = lens.y * 0.05; //tmp

    // solves for the two directions [dx,dy], keeps the two positions [x,y] and the
	// wavelength, such that the path through the lens system will be valid, i.e.
	// lens_evaluate_aperture(in, out) will yield the same out given the solved for in.
	// in: point on sensor. out: point on aperture.
	lens_pt_sample_aperture(in, out, data->focalDistance);

	// evaluates from sensor (in) to outer pupil (out).
	// input arrays are 5d [x,y,dx,dy,lambda] where dx and dy are the direction in
	// two-plane parametrization (that is the third component of the direction would be 1.0).
	// units are millimeters for lengths and micrometers for the wavelength (so visible light is about 0.4--0.7)
	// returns the transmittance computed from the polynomial.
    float transmittance = lens_evaluate(in, out);

    // converts sphere/sphere coords to camera coords
    float inpos[2] = {in[0], in[1]};
    float indir[2] = {in[2], in[3]};
    float outpos[2] = {out[0], out[1]};
    float outdir[2] = {out[2], out[3]};

    // HOW TO QUERY sphereCenter??
    // lens_sphereToCs(const float *inpos, const float *indir, float *outpos, float *outdir, const float sphereCenter, const float sphereRad);
    lens_sphereToCs(inpos, indir, outpos, outdir, 0.0f, lens_outer_pupil_curvature_radius); // 0.0f could also be -lens_outer_pupil_curvature_radius
    
    output.origin.x = outpos[0];
    output.origin.y = outpos[1];
    output.origin.z = data->sensorShift; // not sure if this should be done here

    output.dir.x = outdir[0];
    output.dir.y = outdir[1];
    output.dir.z = -1.0f; // NOT SURE IF CORRECT

    // convert wavelength shift into rgb shift
    output.weight = 1.0f;

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