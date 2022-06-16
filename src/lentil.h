#pragma once

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

#include "../../Eigen/Eigen/Dense"

#include "imagebokeh.h"
#include "lens.h"
#include "global.h"

#include "../CryptomatteArnold/cryptomatte/cryptomatte.h"
#include <chrono>
#include <thread>
#include <regex>

extern AtCritSec l_critsec;
extern bool l_critsec_active;


///////////////////////////////////////////////
//
//      Crit sec utilities
//
///////////////////////////////////////////////

inline bool lentil_crit_sec_init() {
    // Called in node_plugin_initialize. Returns true as a convenience.
    l_critsec_active = true;
    AiCritSecInit(&l_critsec);
    return true;
}

inline void lentil_crit_sec_close() {
    // Called in node_plugin_cleanup
    l_critsec_active = false;
    AiCritSecClose(&l_critsec);
}

inline void lentil_crit_sec_enter() {
    // If the crit sec has not been inited since last close, we simply do not enter.
    // (Used by Cryptomatte filter.)
    if (l_critsec_active)
        AiCritSecEnter(&l_critsec);
}

inline void lentil_crit_sec_leave() {
    // If the crit sec has not been inited since last close, we simply do not enter.
    // (Used by Cryptomatte filter.)
    if (l_critsec_active)
        AiCritSecLeave(&l_critsec);
}



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

// enum to switch between units in interface dropdown
enum CameraType{
    ThinLens,
    PolynomialOptics
};

// enum to switch between units in interface dropdown
enum ChromaticType{
    green_magenta,
    red_cyan
};



// kindly borrowed from cryptomatte, thanks you honeyboo
struct TokenizedOutputLentil {
    std::string camera_tok = "";
    std::string aov_name_tok = "";
    std::string aov_type_tok = "";
    std::string filter_tok = "";
    std::string driver_tok = "";
    bool half_flag = false;
    AtNode* raw_driver = nullptr;
    AtUniverse *universe = nullptr;

private:
    AtNode* driver = nullptr;

public:
    TokenizedOutputLentil() {}

    TokenizedOutputLentil(AtUniverse *universe_in, AtNode* raw_driver_in) { universe = universe_in; raw_driver = raw_driver_in; }

    TokenizedOutputLentil(AtUniverse *universe_in, AtString output_string) {
        universe = universe_in;
        std::string temp_string = output_string.c_str();
        std::string regex_string = " ";
        auto tokens = split(temp_string, regex_string);

        std::string c0 = "";
        std::string c1 = "";
        std::string c2 = "";
        std::string c3 = "";
        std::string c4 = "";
        std::string c5 = "";
        
        if (tokens.size() >= 4) {
            c0 = tokens[0];
            c1 = tokens[1];
            c2 = tokens[2];
            c3 = tokens[3];
        }
        
        if (tokens.size() >= 5) {
            c4 = tokens[4];
        }

        if (tokens.size() >= 6) {
            c5 = tokens[5];
        }

        const bool no_camera = c4.empty() || c4 == std::string("HALF");

        half_flag = (no_camera ? c4 : c5) == std::string("HALF");

        camera_tok = no_camera ? "" : c0;
        aov_name_tok = no_camera ? c0 : c1;
        aov_type_tok = no_camera ? c1 : c2;
        filter_tok = no_camera ? c2 : c3;
        driver_tok = no_camera ? c3 : c4;

        driver = AiNodeLookUpByName(universe, driver_tok.c_str());
    }

    std::string rebuild_output() const {
        if (raw_driver)
            return std::string(AiNodeGetName(raw_driver));

        std::string output_str("");
        if (!camera_tok.empty()) {
            output_str.append(camera_tok);
            output_str.append(" ");
        }
        output_str.append(aov_name_tok);
        output_str.append(" ");
        output_str.append(aov_type_tok);
        output_str.append(" ");
        output_str.append(filter_tok);
        output_str.append(" ");
        output_str.append(driver_tok);
        if (half_flag) // output was already flagged half
            output_str.append(" HALF");
        return output_str;
    }

    AtNode* get_driver() const {
        if (driver && driver_tok == std::string(AiNodeGetName(driver)))
            return driver;
        else if (!driver_tok.empty())
            return AiNodeLookUpByName(universe, driver_tok.c_str());
        else
            return nullptr;
    }

    bool aov_matches(const char* str) const { return aov_name_tok == std::string(str); }

    std::vector<std::string> split(const std::string str, const std::string regex_str)
    {
        std::regex regexz(regex_str);
        std::vector<std::string> list(std::sregex_token_iterator(str.begin(), str.end(), regexz, -1),
                                    std::sregex_token_iterator());
        return list;
    }
};



struct AOVData {
public:
    std::vector<AtRGBA> buffer;
    TokenizedOutputLentil to;

    AtString name = AtString("");
    unsigned int type = 0;
    bool is_duplicate = false;
    int index = 0;

    // crypto
    bool is_crypto = false;
    std::vector<std::map<float, float>> crypto_hash_map;
    std::vector<float> crypto_total_weight;



    AOVData(AtUniverse *universe, std::string output_string) {
        to = TokenizedOutputLentil(universe, AtString(output_string.c_str()));
        name = AtString(to.aov_name_tok.c_str());
        type = string_to_arnold_type(to.aov_type_tok);
    }


    void allocate_regular_buffers(int xres, int yres) {
        buffer.clear();
        buffer.resize(xres*yres);
    }

    void allocate_cryptomatte_buffers(int xres, int yres) {
        crypto_hash_map.clear();
        crypto_hash_map.resize(xres*yres);
        crypto_total_weight.clear();
        crypto_total_weight.resize(xres*yres);
    }

    ~AOVData() {
        destroy_buffers();
    }

private:

    void destroy_buffers() {
        buffer.clear();
        crypto_hash_map.clear();
        crypto_total_weight.clear();
    }
};


struct Camera
{
	LensModel lensModel;
    UnitModel unitModel;
    CameraType cameraType;
    ChromaticType abb_chromatic_type;
    imageData image;

    std::vector<float> zbuffer;
    std::vector<float> zbuffer_debug; // separate zbuffer for the debug AOV, which only tracks redistributed depth values
    std::vector<AOVData> aovs;
    std::vector<float> filter_weight_buffer;

    // lens constants PO
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

    double focus_distance;
	double sensor_width;
	double input_fstop;
    bool enable_dof;
    int vignetting_retries;
    int bokeh_aperture_blades;
    bool bokeh_enable_image;
    AtString bokeh_image_path;
    int bidir_sample_mult;
    double bidir_add_energy_minimum_luminance;
    float bidir_add_energy;
    float bidir_add_energy_transition;
    bool enable_bidir_transmission;
    bool enable_skydome;
    float exposure;
    double lambda;
    float extra_sensor_shift;
    float focal_length;
    float optical_vignetting_distance;
    float optical_vignetting_radius;
    float abb_spherical;
    float abb_coma;
    float abb_distortion;
    float abb_chromatic;
    float circle_to_square;
    float bokeh_anamorphic;
    float fov;
    double tan_fov;
    double max_fstop;
	double aperture_radius;
	double sensor_shift;
    
    AtNode *filter_node;
    AtNode *camera_node;
    AtNode *imager_node;
    AtNode *options_node;

    // filter/imager data
    unsigned xres;
    unsigned yres;
    unsigned xres_without_region;
    unsigned yres_without_region;
    int region_min_x;
    int region_min_y;
    int region_max_x;
    int region_max_y;
    int shift_x;
    int shift_y;
    int samples;
    bool redistribution;
    float current_inv_density;
    float filter_width;
    float time_start;
    float time_end;

    unsigned aovcount;

    const AtString atstring_rgba = AtString("RGBA");
    const AtString atstring_p = AtString("P");
    const AtString atstring_z = AtString("Z");
    const AtString atstring_transmission = AtString("transmission");
    const AtString atstring_opacity = AtString("opacity");
    const AtString atstring_lentil_ignore = AtString("lentil_ignore");
    const AtString atstring_motionvector = AtString("lentil_object_motion_vector");
    const AtString atstring_time = AtString("lentil_time");
    const AtString atstring_lentil_debug = AtString("lentil_debug");

    bool cryptomatte_lentil = false;
    bool imager_print_once_only = false;
    bool crypto_in_same_queue = false;


public:

    Camera() {
        if (!l_critsec_active) AiMsgError("[Lentil] Critical section was not initialized. ");
        crypto_in_same_queue = false;
    }

    ~Camera() {
        image.invalidate();
        destroy_buffers();
    }


    void setup_all (AtUniverse *universe) {
        lentil_crit_sec_enter();

        destroy_buffers();

        options_node = AiUniverseGetOptions(universe);
        camera_node = AiUniverseGetCamera(universe);
        get_lentil_camera_params();
        camera_model_specific_setup();

        // make probability functions of the bokeh image
        // if (!(po->stored_useImage == AiNodeGetBool(node, "bokeh_enable_imagePO") && po->stored_path == AiNodeGetStr(node, "bokeh_image_pathPO")) {
        image.invalidate();
        if (bokeh_enable_image && !image.read(bokeh_image_path.c_str())){
            AiMsgError("[LENTIL CAMERA PO] Couldn't open bokeh image!");
            AiRenderAbort();
        }

        get_arnold_options();

        #ifdef CM_VERSION
            AiMsgInfo("[LENTIL] Version: %s", CM_VERSION);
        #endif
        
        

        redistribution = get_bidirectional_status(universe); // this should include AA level test
        if (redistribution) {
            
            crypto_in_same_queue = false;

            // get cryptomatte node
            AtNode *crypto_node = nullptr;
            AtArray* aov_shaders_array = AiNodeGetArray(options_node, AtString("aov_shaders"));
            for (size_t i=0; i<AiArrayGetNumElements(aov_shaders_array); ++i) {
                AtNode* aov_node = static_cast<AtNode*>(AiArrayGetPtr(aov_shaders_array, i));
                if (AiNodeEntryGetNameAtString(AiNodeGetNodeEntry(aov_node)) == AtString("cryptomatte")) {
                    crypto_node = aov_node;
                }
            }

            // POTENTIAL BUG: what if lentil updates on the same thread as cryptomatte, and is in the queue before crypto? That would be a deadlock.
            // this is an ugly solution, continually checking if cryptomatte already did the setup.
            if (crypto_node){
                CryptomatteData* crypto_data = reinterpret_cast<CryptomatteData*>(AiNodeGetLocalData(crypto_node));
                int time_cnt = 0;
                while (!crypto_data->is_setup_completed) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    ++time_cnt;

                    if (time_cnt == 500) { // guess that crypto is in the same queue, behind lentil
                        crypto_in_same_queue = true;
                        AiMsgInfo("[LENTIL] Waiting for Cryptomatte setup reached time-out.");
                        break;
                    }
                }
                cryptomatte_lentil = true;
            }


            // once crypto has been setup I can do my own setup
            if (!crypto_in_same_queue){
                setup_aovs(universe);
                setup_filter(universe);
            }
        }

        lentil_crit_sec_leave();
    }


    inline void trace_ray_fw_po(int &tries, 
                                const double sx, const double sy,
                                AtVector &origin, AtVector &direction, AtRGB &weight, 
                                double &r1, double &r2, const bool deriv_ray)
    {

        tries = 0;
        bool ray_succes = false;

        Eigen::VectorXd sensor(5); sensor.setZero();
        Eigen::VectorXd aperture(5); aperture.setZero();
        Eigen::VectorXd out(5); out.setZero();

        while(!ray_succes && tries <= vignetting_retries){

            // set sensor position coords
            sensor(0) = sx * (sensor_width * 0.5);
            sensor(1) = sy * (sensor_width * 0.5);
            sensor(2) = sensor(3) = 0.0;
            sensor(4) = lambda;

            aperture.setZero();
            out.setZero();


            if (!enable_dof) aperture(0) = aperture(1) = 0.0; // no dof, all rays through single aperture point
            
            Eigen::Vector2d unit_disk(0.0, 0.0);
            
            if (enable_dof) {
                if (!deriv_ray && tries > 0){ // first iteration comes from arnold blue noise sampler
                    r1 = xor128() / 4294967296.0;
                    r2 = xor128() / 4294967296.0;
                }
                
                if (bokeh_enable_image) {
                    image.bokehSample(r1, r2, unit_disk, xor128() / 4294967296.0, xor128() / 4294967296.0);
                } else if (bokeh_aperture_blades < 2) {
                    concentric_disk_sample(r1, r2, unit_disk, true);
                } else {
                    lens_sample_triangular_aperture(unit_disk(0), unit_disk(1), r1, r2, 1.0, bokeh_aperture_blades);
                }
            }

            aperture(0) = unit_disk(0) * aperture_radius;
            aperture(1) = unit_disk(1) * aperture_radius;
            
            


            // if (empirical_ca_dist > 0.0) {
            //   Eigen::Vector2d sensor_pos(sensor[0], sensor[1]);
            //   Eigen::Vector2d aperture_pos(aperture[0], aperture[1]);
            //   weight = chromatic_abberration_empirical(sensor_pos, empirical_ca_dist, aperture_pos, aperture_radius);
            //   aperture(0) = aperture_pos(0);
            //   aperture(1) = aperture_pos(1);
            // }
            

            if (enable_dof) {
                // aperture sampling, to make sure ray is able to propagate through whole lens system
                lens_pt_sample_aperture(sensor, aperture, sensor_shift);
            }
            

            // move to beginning of polynomial
            sensor(0) += sensor(2) * sensor_shift;
            sensor(1) += sensor(3) * sensor_shift;


            // propagate ray from sensor to outer lens element
            double transmittance = lens_evaluate(sensor, out);
            if(transmittance <= 0.0) {
                ++tries;
                continue;
            }


            // crop out by outgoing pupil
            if( out(0)*out(0) + out(1)*out(1) > lens_outer_pupil_radius*lens_outer_pupil_radius){
                ++tries;
                continue;
            }


            // crop at inward facing pupil
            const double px = sensor(0) + sensor(2) * lens_back_focal_length;
            const double py = sensor(1) + sensor(3) * lens_back_focal_length; //(note that lens_back_focal_length is the back focal length, i.e. the distance unshifted sensor -> pupil)
            if (px*px + py*py > lens_inner_pupil_radius*lens_inner_pupil_radius) {
                ++tries;
                continue;
            }
                
            ray_succes = true;
        }

        if (ray_succes == false) weight = AI_RGB_ZERO;


        // convert from sphere/sphere space to camera space
        Eigen::Vector2d outpos(out[0], out[1]);
        Eigen::Vector2d outdir(out[2], out[3]);
        Eigen::Vector3d cs_origin(0,0,0);
        Eigen::Vector3d cs_direction(0,0,0);
        if (lens_outer_pupil_geometry == "cyl-y") cylinderToCs(outpos, outdir, cs_origin, cs_direction, -lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius, true);
        else if (lens_outer_pupil_geometry == "cyl-x") cylinderToCs(outpos, outdir, cs_origin, cs_direction, -lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius, false);
        else sphereToCs(outpos, outdir, cs_origin, cs_direction, -lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius);
        
        origin = AtVector(cs_origin(0), cs_origin(1), cs_origin(2));
        direction = AtVector(cs_direction(0), cs_direction(1), cs_direction(2));


        switch (unitModel){
            case mm:
            {
                origin *= -1.0; // reverse rays and convert to cm (from mm)
                direction *= -1.0; //reverse rays and convert to cm (from mm)
            } break;
            case cm:
            { 
                origin *= -0.1; // reverse rays and convert to cm (from mm)
                direction *= -0.1; //reverse rays and convert to cm (from mm)
            } break;
            case dm:
            {
                origin *= -0.01; // reverse rays and convert to cm (from mm)
                direction *= -0.01; //reverse rays and convert to cm (from mm)
            } break;
            case m:
            {
                origin *= -0.001; // reverse rays and convert to cm (from mm)
                direction *= -0.001; //reverse rays and convert to cm (from mm)
            }
        }

        direction = AiV3Normalize(direction);

        // Nan bailout
        if (origin[0] != origin[0] || origin[1] != origin[1] || origin[2] != origin[2] || 
            direction[0] != direction[0] || direction[1] != direction[1] || direction[2] != direction[2])
        {
            weight = AI_RGB_ZERO;
        }

    }



    inline void trace_ray_fw_thinlens(int &tries, 
                                    const double sx, const double sy,
                                    AtVector &origin, AtVector &dir, AtRGB &weight,
                                    double &r1, double &r2, const bool deriv_ray){
        tries = 0;
        bool ray_succes = false;

        while (!ray_succes && tries <= vignetting_retries){
            
            // distortion
            AtVector s(sx, sy, 0.0);
            if (abb_distortion > 0.0){
                AtVector2 s2 = barrelDistortion(AtVector2(sx, sy), abb_distortion);
                s = {s2.x, s2.y, 0.0};
            }
            

            // create point on sensor (camera space)
            const AtVector p(s.x * (sensor_width*0.5), 
                             s.y * (sensor_width*0.5), 
                            -focal_length);
                

            // calculate direction vector from origin to point on lens
            AtVector dir_from_center = AiV3Normalize(p); // or norm(p-origin)

            // either get uniformly distributed points on the unit disk or bokeh image
            Eigen::Vector2d unit_disk(0, 0);
            
            if (enable_dof) {
                if (!deriv_ray && tries > 0){ // first iteration comes from arnold blue noise sampler
                    r1 = xor128() / 4294967296.0;
                    r2 = xor128() / 4294967296.0;
                }
                
                if (bokeh_enable_image) {
                    image.bokehSample(r1, r2, unit_disk, xor128() / 4294967296.0, xor128() / 4294967296.0);
                } else if (bokeh_aperture_blades < 2) {
                    concentricDiskSample(r1, r2, unit_disk, abb_spherical, circle_to_square, bokeh_anamorphic);
                } else {
                    lens_sample_triangular_aperture(unit_disk(0), unit_disk(1), r1, r2, 1.0, bokeh_aperture_blades);
                }
            }


            unit_disk(0) *= bokeh_anamorphic;


            // aberration inputs
            float abb_field_curvature = 0.0;


            AtVector lens(unit_disk(0) * aperture_radius, unit_disk(1) * aperture_radius, 0.0);
            const float intersection = std::abs(focus_distance / linear_interpolate(abb_field_curvature, dir_from_center.z, 1.0));
            const AtVector focusPoint = dir_from_center * intersection;
            AtVector dir_from_lens = AiV3Normalize(focusPoint - lens);
            

            // perturb ray direction to simulate coma aberration
            float abb_coma_multiplied = abb_coma * abb_coma_multipliers(sensor_width, focal_length, dir_from_center, unit_disk);
            dir_from_lens = abb_coma_perturb(dir_from_lens, dir_from_lens, abb_coma_multiplied, false);


            if (optical_vignetting_distance > 0.0 && !deriv_ray){
                if (!empericalOpticalVignettingSquare(lens, dir_from_lens, aperture_radius, optical_vignetting_radius, optical_vignetting_distance, lerp_squircle_mapping(circle_to_square))){
                    ++tries;
                    continue;
                }
            }


            // weight = AI_RGB_WHITE;
            // if (emperical_ca_dist > 0.0){
            //     const AtVector2 p2(p.x, p.y);
            //     const float distance_to_center = AiV2Dist(AtVector2(0.0, 0.0), p2);
            //     const int random_aperture = static_cast<int>(std::floor((xor128() / 4294967296.0) * 3.0));
            //     AtVector2 aperture_0_center(0.0, 0.0);
            //     AtVector2 aperture_1_center(- p2 * coc * emperical_ca_dist); //previous: change coc for dist_to_center
            //     AtVector2 aperture_2_center(p2 * coc * emperical_ca_dist);//previous: change coc for dist_to_center
                

            //     if (random_aperture == 1)      lens += aperture_1_center;
            //     else if (random_aperture == 2) lens += aperture_2_center;

            //     if (std::pow(lens.x-aperture_1_center.x, 2) + std::pow(lens.y - aperture_1_center.y, 2) > std::pow(aperture_radius, 2)) {
            //         weight.r = 0.0;
            //     }
            //     if (std::pow(lens.x-aperture_0_center.x, 2) + std::pow(lens.y - aperture_0_center.y, 2) > std::pow(aperture_radius, 2)) {
            //         weight.b = 0.0;
            //     }
            //     if (std::pow(lens.x-aperture_2_center.x, 2) + std::pow(lens.y - aperture_2_center.y, 2) > std::pow(aperture_radius, 2)) {
            //         weight.g = 0.0;
            //     }

            //     if (weight == AI_RGB_ZERO){
            //         ++tries;
            //         continue;
            //     }
            
            // //     //ca, not sure if this should be done, evens out the intensity?
            // //     // float sum = (output.weight.r + output.weight.g + output.weight.b) / 3.0;
            // //     // output.weight.r /= sum;
            // //     // output.weight.g /= sum;
            // //     // output.weight.b /= sum;
            // }

            origin = lens;
            dir = dir_from_lens;

            switch (unitModel){
                case mm:
                {
                    origin *= 10.0; // reverse rays and convert to cm (from mm)
                    dir *= 10.0; //reverse rays and convert to cm (from mm)
                } break;
                case cm:
                { 
                    origin *= 1.0; // reverse rays and convert to cm (from mm)
                    dir *= 1.0; //reverse rays and convert to cm (from mm)
                } break;
                case dm:
                {
                    origin *= 0.1; // reverse rays and convert to cm (from mm)
                    dir *= 0.1; //reverse rays and convert to cm (from mm)
                } break;
                case m:
                {
                    origin *= 0.01; // reverse rays and convert to cm (from mm)
                    dir *= 0.01; //reverse rays and convert to cm (from mm)
                }
            }

           
            // weight = AI_RGB_WHITE;
            ray_succes = true;
        }
        dir = AiV3Normalize(dir);
        if (!ray_succes) weight = AI_RGB_BLACK;
    }


    // given camera space scene point, return point on sensor
    inline bool trace_ray_bw_po(Eigen::Vector3d target,
                                Eigen::Vector2d &sensor_position,
                                const int px, 
                                const int py,
                                const int total_samples_taken,
                                AtMatrix cam_to_world,
                                AtVector sample_pos_ws,
                                AtShaderGlobals *sg, 
                                float lambda_in)
    {
        int tries = 0;
        bool ray_succes = false;

        // initialize 5d light fields
        Eigen::VectorXd sensor(5); sensor << 0,0,0,0, lambda_in;
        Eigen::VectorXd out(5); out << 0,0,0,0, lambda_in;//out.setZero();
        Eigen::Vector2d aperture(0,0);
        
        while(ray_succes == false && tries <= vignetting_retries){

            Eigen::Vector2d unit_disk(0.0, 0.0);

            if (!enable_dof) aperture(0) = aperture(1) = 0.0; // no dof, all rays through single aperture point
            else if (enable_dof && bokeh_aperture_blades <= 2) {
                unsigned int seed = tea<8>(px*py+px, total_samples_taken+tries);

                if (bokeh_enable_image) image.bokehSample(rng(seed), rng(seed), unit_disk, rng(seed), rng(seed));
                else concentric_disk_sample(rng(seed), rng(seed), unit_disk, true);

                aperture(0) = unit_disk(0) * aperture_radius;
                aperture(1) = unit_disk(1) * aperture_radius;
            } 
            else if (enable_dof && bokeh_aperture_blades > 2) {
                unsigned int seed = tea<8>(px*py+px, total_samples_taken+tries);
                lens_sample_triangular_aperture(aperture(0), aperture(1), rng(seed), rng(seed), aperture_radius, bokeh_aperture_blades);
            }


            // raytrace for scene/geometrical occlusions along the ray
            AtVector lens_correct_scaled = AtVector(-aperture(0)*0.1, -aperture(1)*0.1, 0.0);
            switch (unitModel){
                case mm: { lens_correct_scaled /= 0.1; } break;
                case cm: { lens_correct_scaled /= 1.0; } break;
                case dm: { lens_correct_scaled /= 10.0;} break;
                case m:  { lens_correct_scaled /= 100.0;}
            }
            AtVector cam_pos_ws = AiM4PointByMatrixMult(cam_to_world, lens_correct_scaled);
            AtVector ws_direction = AiV3Normalize(cam_pos_ws - sample_pos_ws);
            AtRay ray = AiMakeRay(AI_RAY_SHADOW, sample_pos_ws, &ws_direction, AiV3Dist(cam_pos_ws, sample_pos_ws), sg);
            AtScrSample hit = AtScrSample();
            if (AiTrace(ray, AI_RGB_WHITE, hit)){
                ++tries;
                continue;
            }

            sensor(0) = sensor(1) = 0.0;

            float transmittance = lens_lt_sample_aperture(target, aperture, sensor, out, lambda_in);
            if(transmittance <= 0) {
                ++tries;
                continue;
            }

            // crop at inward facing pupil, not needed to crop by outgoing because already done in lens_lt_sample_aperture()
            const double px = sensor(0) + sensor(2) * lens_back_focal_length;
            const double py = sensor(1) + sensor(3) * lens_back_focal_length; //(note that lens_focal_length is the back focal length, i.e. the distance unshifted sensor -> pupil)
            if (px*px + py*py > lens_inner_pupil_radius*lens_inner_pupil_radius) {
                ++tries;
                continue;
            }

            ray_succes = true;
        }

        // need to account for the ray_success==false case
        if (!ray_succes) return false;

        // shift sensor
        sensor(0) += sensor(2) * -sensor_shift;
        sensor(1) += sensor(3) * -sensor_shift;

        sensor_position(0) = sensor(0);
        sensor_position(1) = sensor(1);

        return true;
    }



    inline float get_image_dist_focusdist_thinlens(){
        return (-focal_length * -focus_distance) / (-focal_length + -focus_distance);
    }

    inline float get_image_dist_focusdist_thinlens_abberated(const float shift){
        return (-focal_length * -(focus_distance+shift)) / (-focal_length + -(focus_distance+shift));
    }


    inline float get_coc_thinlens(AtVector camera_space_sample_position){
        // need to account for the differences in setup between the two methods, since the inputs are scaled differently in the camera shader
        float _focus_distance = focus_distance;
        float _aperture_radius = aperture_radius;
        switch (cameraType){
            case PolynomialOptics:
            { 
                _focus_distance /= 10.0;
            } break;
            case ThinLens:
            {
                _aperture_radius *= 10.0;
            } break;
        }
        
        const float image_dist_samplepos = (-focal_length * camera_space_sample_position.z) / (-focal_length + camera_space_sample_position.z);
        const float image_dist_focusdist = (-focal_length * -_focus_distance) / (-focal_length + -_focus_distance);
        return std::abs((_aperture_radius * (image_dist_samplepos - image_dist_focusdist))/image_dist_samplepos); // coc diameter
    }



    AtRGBA filter_closest_complete(AtAOVSampleIterator *iterator, const uint8_t aov_type){
        AtRGBA pixel_energy = AI_RGBA_ZERO;
        float z = 0.0;

        while (AiAOVSampleIteratorGetNext(iterator))
        {
            float depth = AiAOVSampleIteratorGetAOVFlt(iterator, atstring_z);
            if ((std::abs(depth) <= z) || z == 0.0){
                
                z = std::abs(depth);

                switch (aov_type){
                case AI_TYPE_VECTOR: {
                    const AtVector sample_energy = AiAOVSampleIteratorGetVec(iterator);
                    pixel_energy = AtRGBA(sample_energy.x, sample_energy.y, sample_energy.z, 1.0);
                    break;
                }
                case AI_TYPE_FLOAT: {
                    const float sample_energy = AiAOVSampleIteratorGetFlt(iterator);
                    pixel_energy = AtRGBA(sample_energy, sample_energy, sample_energy, 1.0);
                    break;
                }
                // case AI_TYPE_INT: {
                //   const int sample_energy = AiAOVSampleIteratorGetInt(iterator);
                //   pixel_energy = AtRGBA(sample_energy, sample_energy, sample_energy, 1.0);
                //   break;
                // }
                // case AI_TYPE_UINT: {
                //   const unsigned sample_energy = AiAOVSampleIteratorGetUInt(iterator);
                //   pixel_energy = AtRGBA(sample_energy, sample_energy, sample_energy, 1.0);
                //   break;
                // }
                }
            }
        }

        return pixel_energy;
    }


    AtRGBA filter_gaussian_complete(AtAOVSampleIterator *iterator, const uint8_t aov_type, const float inverse_sample_density, const bool adaptive_sampling){
        float aweight = 0.0f;
        AtRGBA avalue = AI_RGBA_ZERO;
        float inv_density = inverse_sample_density;

        while (AiAOVSampleIteratorGetNext(iterator))
        {
            // take into account adaptive sampling
            if (adaptive_sampling) inv_density = AiAOVSampleIteratorGetInvDensity(iterator);
            if (inv_density <= 0.f) continue;

            // determine distance to filter center
            const AtVector2& offset = AiAOVSampleIteratorGetOffset(iterator);
            const float r = AiSqr(2 / filter_width) * (AiSqr(offset.x) + AiSqr(offset.y));
            if (r > 1.0f) continue;

            // gaussian filter weight
            const float weight = AiFastExp(2 * -r) * inv_density;

            // accumulate weights and colors
            AtRGBA sample_energy = AI_RGBA_ZERO;
            switch (aov_type){
                case AI_TYPE_RGBA: {
                    sample_energy = AiAOVSampleIteratorGetRGBA(iterator);
                } break;
                case AI_TYPE_RGB: {
                    AtRGB sample_energy_rgb = AiAOVSampleIteratorGetRGB(iterator);
                    sample_energy = AtRGB(sample_energy_rgb.r, sample_energy_rgb.g, sample_energy_rgb.b);
                } break;
            }
            
            avalue += weight * sample_energy;
            aweight += weight;
        }
        
        // compute final filtered color
        if (aweight != 0.0f) avalue /= aweight;

        return avalue;
    }



    // get all depth samples so i can re-use them
    void cryptomatte_construct_cache(std::vector<std::map<float, float>> &crypto_hashmap_cache,
                                    struct AtAOVSampleIterator* sample_iterator, 
                                    const int sampleid) {

        for (auto &aov : aovs) {
            if (aov.is_crypto){
                float iterative_transparency_weight = 1.0f;
                float quota = 1.0;
                float sample_value = 0.0f;

                while (AiAOVSampleIteratorGetNextDepth(sample_iterator)) {
                    const float sub_sample_opacity = AiColorToGrey(AiAOVSampleIteratorGetAOVRGB(sample_iterator, atstring_opacity));
                    sample_value = AiAOVSampleIteratorGetAOVFlt(sample_iterator, aov.name);
                    const float sub_sample_weight = sub_sample_opacity * iterative_transparency_weight;

                    // so if the current sub sample is 80% opaque, it means 20% of the weight will remain for the next subsample
                    iterative_transparency_weight *= (1.0f - sub_sample_opacity);

                    quota -= sub_sample_weight;

                    crypto_hashmap_cache[aov.index][sample_value] += sub_sample_weight;
                }

                // the remaining values gets allocated to the last sample
                if (quota > 0.0) crypto_hashmap_cache[aov.index][sample_value] += quota;

                // reset is required because AiAOVSampleIteratorGetNextDepth() automatically moves to next sample after final depth sample
                // still need to use the iterator afterwards, so need to do a reset to the current sample id
                reset_iterator_to_id(sample_iterator, sampleid);
            }
        }
    }


    inline void add_to_buffer_cryptomatte(AOVData &aov, int px, std::map<float, float> &cryptomatte_cache, const float sample_weight) {
        aov.crypto_total_weight[px] += sample_weight;
        for (auto const& sample : cryptomatte_cache) {
            aov.crypto_hash_map[px][sample.first] += sample.second * sample_weight;
        }
    }

    

    inline void add_to_buffer(AOVData &aov, const int px, const AtRGBA aov_value,
                            const float fitted_bidir_add_energy, const float depth,
                            struct AtAOVSampleIterator* sample_iterator, const float filter_weight, const AtRGB rgb_weight) {
        switch(aov.type){

            case AI_TYPE_RGBA: {
                if (aov.name == atstring_rgba) filter_weight_buffer[px] += filter_weight;

                AtRGBA rgba_energy = aov_value;
                aov.buffer[px] += (rgba_energy+fitted_bidir_add_energy) * filter_weight * rgb_weight;
                
                break;
            }

            case AI_TYPE_RGB: {
                const AtRGBA rgba_energy = aov_value;
                aov.buffer[px] += (rgba_energy+fitted_bidir_add_energy) * filter_weight * rgb_weight;
                
                break;
            }

            case AI_TYPE_VECTOR: {
                if ((std::abs(depth) <= zbuffer[px]) || zbuffer[px] == 0.0){
                    aov.buffer[px] = aov_value;
                    zbuffer[px] = std::abs(depth);
                }

                break;
            }

            case AI_TYPE_FLOAT: {
                if (aov.name != atstring_lentil_debug) {
                    if ((std::abs(depth) <= zbuffer[px]) || zbuffer[px] == 0.0){
                        aov.buffer[px] = aov_value;
                        zbuffer[px] = std::abs(depth);
                    } 
                } else {
                    if ((std::abs(depth) <= zbuffer_debug[px]) || zbuffer_debug[px] == 0.0){
                        if (aov_value.r != 0.0){
                            aov.buffer[px] = aov_value;
                            zbuffer_debug[px] = std::abs(depth);
                        }
                    }
                }

                break;
            }

            // case AI_TYPE_INT: {
            //   if ((std::abs(depth) <= zbuffer[px]) || zbuffer[px] == 0.0){
            //     const int int_energy = AiAOVSampleIteratorGetAOVInt(sample_iterator, aov_name);
            //     const AtRGBA rgba_energy = AtRGBA(int_energy, int_energy, int_energy, 1.0);
            //     aov.buffer[px] = rgba_energy;
            //     zbuffer[px] = std::abs(depth);
            //   }

            //   break;
            // }

            // case AI_TYPE_UINT: {
            //   if ((std::abs(depth) <= zbuffer[px]) || zbuffer[px] == 0.0){
            //     const unsigned uint_energy = AiAOVSampleIteratorGetAOVUInt(sample_iterator, aov_name);
            //     const AtRGBA rgba_energy = AtRGBA(uint_energy, uint_energy, uint_energy, 1.0);
            //     aov.buffer[px] = rgba_energy;
            //     zbuffer[px] = std::abs(depth);
            //   }

            //   break;
            // }

            // case AI_TYPE_POINTER: {
            //   if ((std::abs(depth) <= zbuffer[px]) || zbuffer[px] == 0.0){
            //     const void *ptr_energy = AiAOVSampleIteratorGetAOVPtr(sample_iterator, aov_name);
            //     image_ptr_types[aov_name][px] = ptr_energy;
            //     zbuffer[px] = std::abs(depth);
            //   }

            //   break;
            // }
        }
    }


    // inline float filter_weight_gaussian(AtVector2 p, float width) {
    //     const float r = std::pow(2.0 / width, 2.0) * (std::pow(p.x, 2) + std::pow(p.y, 2));
    //     if (r > 1.0f) return 0.0;
    //     return AiFastExp(2 * -r);
    // }

    inline void filter_and_add_to_buffer_new(int px, int py,
                                        float depth,
                                        struct AtAOVSampleIterator* iterator,
                                        std::vector<std::map<float, float>> &cryptomatte_cache, std::vector<AtRGBA> &aov_values, float inv_density){


        const AtVector2 &subpixel_position = AiAOVSampleIteratorGetOffset(iterator); // offset within original pixel
        const unsigned pixelnumber = xres * py + px;
        
        // float filter_weight = filter_weight_gaussian(subpixel_position, filter_width);
        // if (filter_weight == 0) return;
        float filter_weight = 1.0;

        for (auto &aov : aovs){
            if (aov.is_crypto) add_to_buffer_cryptomatte(aov, pixelnumber, cryptomatte_cache[aov.index], inv_density);
            else add_to_buffer(aov, pixelnumber, aov_values[aov.index], 0.0, depth, iterator, filter_weight * inv_density, AI_RGB_WHITE); 
        }
    }


    inline int coords_to_linear_pixel(const int x, const int y) {
        return x + (y * xres);
    }



    inline void lens_sample_triangular_aperture(double &x, double &y, double r1, double r2, const double radius, const int blades){
        const int tri = (int)(r1*blades);

        // rescale:
        r1 = r1*blades - tri;

        // sample triangle:
        double a = std::sqrt(r1);
        double b = (1.0f-r2)*a;
        double c = r2*a;

        double p1[2], p2[2];

        common_sincosf(2.0f*AI_PI/blades * (tri+1), p1, p1+1);
        common_sincosf(2.0f*AI_PI/blades * tri, p2, p2+1);

        x = radius * (b * p1[1] + c * p2[1]);
        y = radius * (b * p1[0] + c * p2[0]);
    }


    
    void setup_filter(AtUniverse *universe) {
        xres_without_region = AiNodeGetInt(options_node, AtString("xres"));
        yres_without_region = AiNodeGetInt(options_node, AtString("yres"));
        region_min_x = AiNodeGetInt(options_node, AtString("region_min_x"));
        region_min_y = AiNodeGetInt(options_node, AtString("region_min_y"));
        region_max_x = AiNodeGetInt(options_node, AtString("region_max_x"));
        region_max_y = AiNodeGetInt(options_node, AtString("region_max_y"));

        // need to check if the render region option is used, if not, set it to default
        if (region_min_x == INT32_MIN || region_min_x == INT32_MAX ||
            region_max_x == INT32_MIN || region_max_x == INT32_MAX ||
            region_min_y == INT32_MIN || region_min_y == INT32_MAX ||
            region_max_y == INT32_MIN || region_max_y == INT32_MAX ) {
              region_min_x = 0;
              region_min_y = 0;
              region_max_x = xres_without_region;
              region_max_y = yres_without_region;
        }

        xres = region_max_x - region_min_x + 1;
        yres = region_max_y - region_min_y + 1;
        

        // if ((region_min_x != INT32_MIN && region_min_x != INT32_MAX && region_min_x != 0) || 
        //     (region_min_y != INT32_MIN && region_min_y != INT32_MAX && region_min_y != 0)) {
        //         AiMsgError("[ARNOLD BUG] 0x02-type Imagers currently do not work when region_min_x/y is set. Erroring out to avoid crash.(ARNOLD-11835, filed 2021/11/16).");
        // }


        filter_width = 1.5;
        time_start = AiCameraGetShutterStart();
        time_end = AiCameraGetShutterEnd();
        imager_print_once_only = false;
        current_inv_density = 0.0;


        zbuffer.resize(xres * yres);
        zbuffer_debug.resize(xres * yres);
        filter_weight_buffer.resize(xres * yres);


        // creates buffers for each AOV with lentil_filter (lentil_replaced_filter)
        for (auto &aov : aovs) {
            if (aov.to.filter_tok == "lentil_replaced_filter"){
                
                // crypto does this check to avoid "actually" doing the work unless we're writing an exr to disk
                // this speeds up the IPR sessions.
                bool driver_is_exr = false;
                AtNode *driver_node = aov.to.get_driver();
                if (driver_node && AiNodeIs(driver_node, AtString("driver_exr"))){
                    driver_is_exr = true;
                }

                if (aov.to.aov_name_tok.find("crypto_") != std::string::npos && driver_is_exr){
                    aov.allocate_cryptomatte_buffers(xres, yres);
                } else {
                    aov.allocate_regular_buffers(xres, yres);
                }

                AiMsgInfo("[LENTIL BIDIRECTIONAL] Driver '%s' -- Adding aov %s of type %s", aov.to.driver_tok.c_str(), aov.to.aov_name_tok.c_str(), aov.to.aov_type_tok.c_str());
            }
        }
    }


    void setup_aovs(AtUniverse *universe) {
        filter_node = AiNodeLookUpByName(universe, AtString("lentil_replaced_filter"));
        if (!filter_node) filter_node = AiNode(universe, AtString("lentil_filter"), AtString("lentil_replaced_filter"));

        AtArray* outputs = AiNodeGetArray(AiUniverseGetOptions(universe), AtString("outputs"));
        const int elements = AiArrayGetNumElements(outputs);
        std::vector<std::string> output_strings;
        bool lentil_time_found = false;
        bool lentil_debug_found = false;
        bool lentil_raydir_found = false;

        for (int i=0; i<elements; i++) {
            std::string output_string = AiArrayGetStr(outputs, i).c_str();

            AOVData aov(universe, output_string);

            bool replace_filter = true;

            // lentil unsupported
            if (aov.to.aov_type_tok != "RGBA" && 
                aov.to.aov_type_tok != "RGB" && 
                aov.to.aov_type_tok != "FLOAT" && 
                aov.to.aov_type_tok != "VECTOR") {
                replace_filter = false;
            }

            // never attach filter to the unranked crypto AOVs, they're just for display purposes.
            // ranked aov's are e.g: crypto_material00, crypto_material01, ...
            if (aov.to.aov_name_tok == "crypto_material" || 
                aov.to.aov_name_tok == "crypto_asset" || 
                aov.to.aov_name_tok == "crypto_object"){
                replace_filter = false;
            } else if (aov.to.aov_name_tok.find("crypto_") != std::string::npos){
                aov.is_crypto = true;
            }

            if (replace_filter && aov.to.aov_name_tok != "lentil_replaced_filter"){
                aov.to.filter_tok = "lentil_replaced_filter";
            }

            if (aov.to.aov_name_tok == "lentil_time"){
                lentil_time_found = true;
            }

            if (aov.to.aov_name_tok == "lentil_debug"){
                lentil_debug_found = true;
            }

            if (aov.to.aov_name_tok == "lentil_raydir"){
                lentil_raydir_found = true;
            }

            // identify as duplicate
            for (auto &element : aovs) {
                if (aov.to.aov_name_tok == element.to.aov_name_tok) {
                    aov.is_duplicate = true;
                }
            }
            
            aovs.push_back(aov);
        }


        // make a copy of RGBA aov and use it as the basis for the lentil_debug AOV
        // doing this to make sure the whole AOV string is correct, including all the options
        // because some stuff happens in the constructor and we're skipping that here, we need to set these manually
        if (!lentil_debug_found){
            AOVData aov_lentil_debug = aovs[0];
            aov_lentil_debug.to.aov_type_tok = "FLOAT";
            aov_lentil_debug.to.aov_name_tok = "lentil_debug";
            aov_lentil_debug.to = TokenizedOutputLentil(universe, AtString(aov_lentil_debug.to.rebuild_output().c_str()));
            aov_lentil_debug.name = AtString("lentil_debug");
            aov_lentil_debug.type = string_to_arnold_type(aov_lentil_debug.to.aov_type_tok);
            aovs.push_back(aov_lentil_debug);
        }
        
        if (!lentil_time_found) {
            AOVData aov_lentil_time = aovs[0];
            aov_lentil_time.to.aov_type_tok = "FLOAT";
            aov_lentil_time.to.aov_name_tok = "lentil_time";
            aov_lentil_time.to = TokenizedOutputLentil(universe, AtString(aov_lentil_time.to.rebuild_output().c_str()));
            aov_lentil_time.name = AtString("lentil_time");
            aov_lentil_time.type = string_to_arnold_type(aov_lentil_time.to.aov_type_tok);
            aovs.push_back(aov_lentil_time);
        }

        if (!lentil_raydir_found) {
            AOVData aov_lentil_raydir = aovs[0];
            aov_lentil_raydir.to.aov_type_tok = "RGB";
            aov_lentil_raydir.to.aov_name_tok = "lentil_raydir";
            aov_lentil_raydir.to = TokenizedOutputLentil(universe, AtString(aov_lentil_raydir.to.rebuild_output().c_str()));
            aov_lentil_raydir.name = AtString("lentil_raydir");
            aov_lentil_raydir.type = string_to_arnold_type(aov_lentil_raydir.to.aov_type_tok);
            aovs.push_back(aov_lentil_raydir);
        }
        
        AtArray *final_outputs = AiArrayAllocate(aovs.size(), 1, AI_TYPE_STRING);
        uint32_t i = 0;
        for (auto &output : aovs){
            AiArraySetStr(final_outputs, i++, output.to.rebuild_output().c_str());
            output.index = i;

            if (output.to.aov_name_tok == "lentil_time" || output.to.aov_name_tok == "lentil_debug") {
                AiAOVRegister(output.to.aov_name_tok.c_str(), string_to_arnold_type(output.to.aov_type_tok), AI_AOV_BLEND_NONE); // think i should only do this for the new layer (lentil_time, lentil_debug)?
            }
        }
        AiNodeSetArray(AiUniverseGetOptions(universe), AtString("outputs"), final_outputs);
        aovcount = aovs.size()+1;


        // remove duplicate aov's by name, also remove aovs that aren't filtered by lentil
        std::vector<AOVData>::iterator it = aovs.begin();
        while(it != aovs.end()) {
            if(it->is_duplicate || it->to.filter_tok != "lentil_replaced_filter") {
                it = aovs.erase(it);
            }
            else ++it;
        }


        // need to add an entry to the aov_shaders (NODE)
        AtArray* aov_shaders_array = AiNodeGetArray(AiUniverseGetOptions(universe), AtString("aov_shaders"));
        int aov_shader_array_size = AiArrayGetNumElements(aov_shaders_array);

        if (!lentil_time_found){
            AtNode *time_write = AiNode(universe, AtString("aov_write_float"), AtString("lentil_time_write"));
            AtNode *time_read = AiNode(universe, AtString("state_float"), AtString("lentil_time_read"));

            // set time node params/linking
            AiNodeSetStr(time_read, AtString("variable"), AtString("time"));
            AiNodeSetStr(time_write, AtString("aov_name"), AtString("lentil_time"));
            AiNodeLink(time_read, AtString("aov_input"), time_write);

            AiArrayResize(aov_shaders_array, aov_shader_array_size+1, 1);
            AiArraySetPtr(aov_shaders_array, aov_shader_array_size, (void*)time_write);
            AiNodeSetArray(AiUniverseGetOptions(universe), AtString("aov_shaders"), aov_shaders_array);
        }

        if (!lentil_raydir_found){
            AtNode *raydir_write = AiNode(universe, AtString("aov_write_rgb"), AtString("lentil_raydir_write"));
            AtNode *raydir_read = AiNode(universe, AtString("state_vector"), AtString("lentil_raydir_read"));

            // set time node params/linking
            AiNodeSetStr(raydir_read, AtString("variable"), AtString("Rd"));
            AiNodeSetStr(raydir_write, AtString("aov_name"), AtString("lentil_raydir"));
            AiNodeLink(raydir_read, AtString("aov_input"), raydir_write);

            AiArrayResize(aov_shaders_array, aov_shader_array_size+1, 1);
            AiArraySetPtr(aov_shaders_array, aov_shader_array_size, (void*)raydir_write);
            AiNodeSetArray(AiUniverseGetOptions(universe), AtString("aov_shaders"), aov_shaders_array);
        }
    }



    inline float additional_luminance_soft_trans(const float sample_luminance){
        // additional luminance with soft transition
        if (sample_luminance > bidir_add_energy_minimum_luminance && sample_luminance < bidir_add_energy_minimum_luminance+bidir_add_energy_transition){
            float perc = (sample_luminance - bidir_add_energy_minimum_luminance) / bidir_add_energy_transition;
            return bidir_add_energy * perc;          
        } else if (sample_luminance > bidir_add_energy_minimum_luminance+bidir_add_energy_transition) {
            return bidir_add_energy;
        }

        return 0.0;
    }


private:

    void destroy_buffers() {
        zbuffer.clear();
        zbuffer_debug.clear();
        aovs.clear();
        filter_weight_buffer.clear();
    }


     bool get_bidirectional_status(AtUniverse *universe) {

        // if progressive rendering is on, don't redistribute
        if (AiNodeGetBool(AiUniverseGetOptions(universe), AtString("enable_progressive_render"))) {
            AiMsgError("[LENTIL BIDIRECTIONAL] Progressive rendering is not supported. Arnold does not yet provide enough API functionality for this to be implemented as it should.");
            AiRenderAbort();
            return false;
        }

        if (!enable_dof) {
            AiMsgWarning("[LENTIL BIDIRECTIONAL] Depth of field is disabled, therefore disabling bidirectional sampling.");
            return false;
        }

        if (bidir_sample_mult == 0){
            AiMsgWarning("[LENTIL BIDIRECTIONAL] Bidirectional samples are set to 0, filter will not execute.");
            return false;
        }

        // should include an AA sample level test, if not final sample level, skip! currently i have to do ugly counting inside the filter to get the current AA level.

        return true;
    }



    inline void reset_iterator_to_id(AtAOVSampleIterator* iterator, int id){
        AiAOVSampleIteratorReset(iterator);
        
        for (int i = 0; AiAOVSampleIteratorGetNext(iterator) == true; i++){
            if (i == id) return;
        }

        return;
    }


    void get_lentil_camera_params() {
        cameraType = (CameraType) AiNodeGetInt(camera_node, AtString("camera_type"));

        unitModel = (UnitModel) AiNodeGetInt(camera_node, AtString("units"));
        if (unitModel == static_cast<UnitModel>(4)) { // "disable"
            const float meters_per_unit = AiNodeGetFlt(options_node, AtString("meters_per_unit"));
            if (meters_per_unit == 1.0) unitModel = static_cast<UnitModel>(3);
            else if (meters_per_unit == 0.1) unitModel = static_cast<UnitModel>(2);
            else if (meters_per_unit == 0.01) unitModel = static_cast<UnitModel>(1);
            else if (meters_per_unit == 0.001) unitModel = static_cast<UnitModel>(0);
        }

        sensor_width = AiNodeGetFlt(camera_node, AtString("sensor_width"));
        
        enable_dof = AiNodeGetBool(camera_node, AtString("enable_dof"));
        if (AiNodeGetBool(options_node, AtString("ignore_dof"))) enable_dof = false;

        input_fstop = clamp_min(AiNodeGetFlt(camera_node, AtString("fstop")), 0.01);
        focus_distance = AiNodeGetFlt(camera_node, AtString("focus_dist")); //converting to mm
        bokeh_aperture_blades = AiNodeGetInt(camera_node, AtString("aperture_blades_lentil"));
        exposure = AiNodeGetFlt(camera_node, AtString("exp"));

        // po-specific params
        lensModel = (LensModel) AiNodeGetInt(camera_node, AtString("lens_model"));
        lambda = AiNodeGetFlt(camera_node, AtString("wavelength")) * 0.001;
        extra_sensor_shift = AiNodeGetFlt(camera_node, AtString("extra_sensor_shift"));

        // tl specific params
        focal_length = clamp_min(AiNodeGetFlt(camera_node, AtString("focal_length_lentil")), 0.01);
        optical_vignetting_distance = AiNodeGetFlt(camera_node, AtString("optical_vignetting"));
        optical_vignetting_radius = 1.0;//AiNodeGetFlt(camera_node, AtString("optical_vignetting_radius"));
        abb_spherical = AiNodeGetFlt(camera_node, AtString("abb_spherical"));
        abb_spherical = clamp(abb_spherical, 0.001, 0.999);
        abb_distortion = AiNodeGetFlt(camera_node, AtString("abb_distortion"));
        abb_coma = AiNodeGetFlt(camera_node, AtString("abb_coma"));
        abb_chromatic = AiNodeGetFlt(camera_node, AtString("abb_chromatic"));
        abb_chromatic_type = (ChromaticType) AiNodeGetInt(camera_node, AtString("abb_chromatic_type"));
        circle_to_square = AiNodeGetFlt(camera_node, AtString("bokeh_circle_to_square"));
        circle_to_square = clamp(circle_to_square, 0.01, 0.99);
        bokeh_anamorphic = 1.0 - AiNodeGetFlt(camera_node, AtString("bokeh_anamorphic"));
        bokeh_anamorphic = clamp(bokeh_anamorphic, 0, 1.0);

        // bidir params
        bokeh_enable_image = AiNodeGetBool(camera_node, AtString("bokeh_enable_image"));
        bokeh_image_path = AiNodeGetStr(camera_node, AtString("bokeh_image_path"));
        bidir_sample_mult = AiNodeGetInt(camera_node, AtString("bidir_sample_mult"));
        bidir_add_energy_minimum_luminance = AiNodeGetFlt(camera_node, AtString("bidir_add_energy_minimum_luminance"));
        bidir_add_energy = AiNodeGetFlt(camera_node, AtString("bidir_add_energy"));
        bidir_add_energy_transition = AiNodeGetFlt(camera_node, AtString("bidir_add_energy_transition"));
        vignetting_retries = AiNodeGetInt(camera_node, AtString("vignetting_retries"));
        enable_bidir_transmission = AiNodeGetBool(camera_node, AtString("enable_bidir_transmission"));
        enable_skydome = AiNodeGetBool(camera_node, AtString("enable_skydome"));

        
    }


    void get_arnold_options() {
        xres = AiNodeGetInt(options_node, AtString("xres"));
        yres = AiNodeGetInt(options_node, AtString("yres"));
    }


    // evaluates from sensor (in) to outer pupil (out).
    // input arrays are 5d [x,y,dx,dy,lambda] where dx and dy are the direction in
    // two-plane parametrization (that is the third component of the direction would be 1.0).
    // units are millimeters for lengths and micrometers for the wavelength (so visible light is about 0.4--0.7)
    // returns the transmittance computed from the polynomial.
    inline double lens_evaluate(const Eigen::VectorXd in, Eigen::VectorXd &out)
    {
        const double x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];
        double out_transmittance = 0.0;
        switch (lensModel){
            #include "../include/auto_generated_lens_includes/load_pt_evaluate.h"
        }

        return std::max(0.0, out_transmittance);
    }

    // solves for the two directions [dx,dy], keeps the two positions [x,y] and the
    // wavelength, such that the path through the lens system will be valid, i.e.
    // lens_evaluate_aperture(in, out) will yield the same out given the solved for in.
    // in: point on sensor. out: point on aperture.
    inline void lens_pt_sample_aperture(Eigen::VectorXd &in, Eigen::VectorXd &out, double dist)
    {
        double out_x = out[0], out_y = out[1], out_dx = out[2], out_dy = out[3], out_transmittance = 1.0f;
        double x = in[0], y = in[1], dx = in[2], dy = in[3], lambda = in[4];

        switch (lensModel){
            #include "../include/auto_generated_lens_includes/load_pt_sample_aperture.h"
        }

        // directions may have changed, copy all to be sure.
        out[0] = out_x; // dont think this is needed
        out[1] = out_y; // dont think this is needed
        out[2] = out_dx;
        out[3] = out_dy;

        in[0] = x; // dont think this is needed
        in[1] = y; // dont think this is needed
        in[2] = dx;
        in[3] = dy;
    }
    

    // solves for a sensor position given a scene point and an aperture point
    // returns transmittance from sensor to outer pupil
    inline double lens_lt_sample_aperture(
        const Eigen::Vector3d scene,    // 3d point in scene in camera space
        const Eigen::Vector2d ap,       // 2d point on aperture (in camera space, z is known)
        Eigen::VectorXd &sensor,        // output point and direction on sensor plane/plane
        Eigen::VectorXd &out,           // output point and direction on outer pupil
        const double lambda)            // wavelength   
    {
        const double scene_x = scene[0], scene_y = scene[1], scene_z = scene[2];
        const double ap_x = ap[0], ap_y = ap[1];
        double x = 0, y = 0, dx = 0, dy = 0;

        switch (lensModel){
            #include "../include/auto_generated_lens_includes/load_lt_sample_aperture.h"    
        }

        sensor[0] = x; sensor[1] = y; sensor[2] = dx; sensor[3] = dy; sensor[4] = lambda;
        return std::max(0.0, out[4]);
    }


    inline bool trace_ray_focus_check(double sensor_shift, double &test_focus_distance)
    {
        Eigen::VectorXd sensor(5); sensor.setZero();
        Eigen::VectorXd aperture(5); aperture.setZero();
        Eigen::VectorXd out(5); out.setZero();
        sensor(4) = lambda;
        aperture(1) = lens_aperture_housing_radius * 0.25;

        lens_pt_sample_aperture(sensor, aperture, sensor_shift);

        // move to beginning of polynomial
        sensor(0) += sensor(2) * sensor_shift;
        sensor(1) += sensor(3) * sensor_shift;

            // propagate ray from sensor to outer lens element
        double transmittance = lens_evaluate(sensor, out);
        if(transmittance <= 0.0) return false;

        // crop out by outgoing pupil
        if( out(0)*out(0) + out(1)*out(1) > lens_outer_pupil_radius*lens_outer_pupil_radius){
            return false;
        }

        // crop at inward facing pupil
        const double px = sensor(0) + sensor(2) * lens_back_focal_length;
        const double py = sensor(1) + sensor(3) * lens_back_focal_length;
        if (px*px + py*py > lens_inner_pupil_radius*lens_inner_pupil_radius){
            return false;
        }

            // convert from sphere/sphere space to camera space
        Eigen::Vector2d outpos(out(0), out(1));
        Eigen::Vector2d outdir(out(2), out(3));
        Eigen::Vector3d camera_space_pos(0,0,0);
        Eigen::Vector3d camera_space_omega(0,0,0);
        if (lens_outer_pupil_geometry == "cyl-y") cylinderToCs(outpos, outdir, camera_space_pos, camera_space_omega, -lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius, true);
            else if (lens_outer_pupil_geometry == "cyl-x") cylinderToCs(outpos, outdir, camera_space_pos, camera_space_omega, -lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius, false);
        else sphereToCs(outpos, outdir, camera_space_pos, camera_space_omega, -lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius);

        test_focus_distance = line_plane_intersection(camera_space_pos, camera_space_omega)(2);
        return true;
    }



    inline double camera_get_y0_intersection_distance(double sensor_shift, double intersection_distance)
    {
        Eigen::VectorXd sensor(5); sensor.setZero();
        Eigen::VectorXd aperture(5); aperture.setZero();
        Eigen::VectorXd out(5); out.setZero();
        sensor(4) = lambda;
        aperture(1) = lens_aperture_housing_radius * 0.25;

        lens_pt_sample_aperture(sensor, aperture, sensor_shift);

        sensor(0) += sensor(2) * sensor_shift;
            sensor(1) += sensor(3) * sensor_shift;

            double transmittance = lens_evaluate(sensor, out);

            // convert from sphere/sphere space to camera space
        Eigen::Vector2d outpos(out(0), out(1));
        Eigen::Vector2d outdir(out(2), out(3));
            Eigen::Vector3d camera_space_pos(0,0,0);
            Eigen::Vector3d camera_space_omega(0,0,0);
        if (lens_outer_pupil_geometry == "cyl-y") cylinderToCs(outpos, outdir, camera_space_pos, camera_space_omega, -lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius, true);
            else if (lens_outer_pupil_geometry == "cyl-x") cylinderToCs(outpos, outdir, camera_space_pos, camera_space_omega, -lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius, false);
        else sphereToCs(outpos, outdir, camera_space_pos, camera_space_omega, -lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius);
        
        return line_plane_intersection(camera_space_pos, camera_space_omega)(2);
    }


    // note that this is all with an unshifted sensor
    inline void trace_backwards_for_fstop(const double fstop_target, double &calculated_fstop, double &calculated_aperture_radius) {
        const int maxrays = 1000;
        double best_valid_fstop = 0.0;
        double best_valid_aperture_radius = 0.0;

        for (int i = 1; i < maxrays; i++)
        {
            const double parallel_ray_height = (static_cast<double>(i)/static_cast<double>(maxrays)) * lens_outer_pupil_radius;
            const Eigen::Vector3d target(0, parallel_ray_height, AI_BIG);
            Eigen::VectorXd sensor(5); sensor << 0,0,0,0, lambda;
            Eigen::VectorXd out(5); out.setZero();

            // just point through center of aperture
            Eigen::Vector2d aperture(0.01, parallel_ray_height);

            // if (!lens_lt_sample_aperture(target, aperture, sensor, out, lambda, camera)) continue;
            if(lens_lt_sample_aperture(target, aperture, sensor, out, lambda) <= 0.0) continue;

            // crop at inner pupil
            const double px = sensor(0) + (sensor(2) * lens_back_focal_length);
            const double py = sensor(1) + (sensor(3) * lens_back_focal_length);
            if (px*px + py*py > lens_inner_pupil_radius*lens_inner_pupil_radius) continue;

            // somehow need to get last vertex positiondata.. don't think what i currently have is correct
            Eigen::Vector3d out_cs_pos(0,0,0);
            Eigen::Vector3d out_cs_dir(0,0,0);
            Eigen::Vector2d outpos(out(0), out(1));
            Eigen::Vector2d outdir(out(2), out(3)); 
            if (lens_inner_pupil_geometry == "cyl-y") {
                cylinderToCs(outpos, outdir, out_cs_pos, out_cs_dir, - lens_inner_pupil_curvature_radius + lens_back_focal_length, lens_inner_pupil_curvature_radius, true);
            }
            else if (lens_inner_pupil_geometry == "cyl-x") {
                cylinderToCs(outpos, outdir, out_cs_pos, out_cs_dir, - lens_inner_pupil_curvature_radius + lens_back_focal_length, lens_inner_pupil_curvature_radius, false);
            }
            else sphereToCs(outpos, outdir, out_cs_pos, out_cs_dir, - lens_inner_pupil_curvature_radius + lens_back_focal_length, lens_inner_pupil_curvature_radius);

            const double theta = std::atan(out_cs_pos(1) / out_cs_pos(2));
            const double fstop = 1.0 / (std::sin(theta)* 2.0);

            if (fstop < fstop_target) {
                calculated_fstop = best_valid_fstop;
                calculated_aperture_radius = best_valid_aperture_radius;
                return;
            } else {
                best_valid_fstop = fstop;
                best_valid_aperture_radius = parallel_ray_height;
            }
        }

        calculated_fstop = best_valid_fstop;
        calculated_aperture_radius = best_valid_aperture_radius;
    }


    // focal_distance is in mm
    inline double logarithmic_focus_search(const double focal_distance){
        double closest_distance = 999999999.0;
        double best_sensor_shift = 0.0;
        for (double sensorshift : logarithmic_values()){
            double intersection_distance = 0.0;
            intersection_distance = camera_get_y0_intersection_distance(sensorshift, intersection_distance);
            double new_distance = focal_distance - intersection_distance;

            if (new_distance < closest_distance && new_distance > 0.0){
                closest_distance = new_distance;
                best_sensor_shift = sensorshift;
            }
        }

        return best_sensor_shift;
    }



    // returns sensor offset in mm
    // traces rays backwards through the lens
    inline double camera_set_focus(double dist)
    {
        const Eigen::Vector3d target(0, 0, dist);
        Eigen::VectorXd sensor(5); sensor.setZero();
        Eigen::VectorXd out(5); out.setZero();
        sensor(4) = lambda;
        double offset = 0.0;
        int count = 0;
        const double scale_samples = 0.1;
        Eigen::Vector2d aperture(0,0);

        const int S = 4;

        // trace a couple of adjoint rays from there to the sensor and
        // see where we need to put the sensor plane.
        for(int s=1; s<=S; s++){
            for(int k=0; k<2; k++){
            
                // reset aperture
                aperture.setZero();

                aperture[k] = lens_aperture_housing_radius * (s/(S+1.0) * scale_samples); // (1to4)/(4+1) = (.2, .4, .6, .8) * scale_samples

                lens_lt_sample_aperture(target, aperture, sensor, out, lambda);

                if(sensor(2+k) > 0){
                    offset += sensor(k)/sensor(2+k);
                    printf("\t[LENTIL] raytraced sensor shift at aperture[%f, %f]: %f", aperture(0), aperture(1), sensor(k)/sensor(2+k));
                    count ++;
                }
            }
        }

        // average guesses
        offset /= count; 
        
        // the focus plane/sensor offset:
        // negative because of reverse direction
        if(offset == offset){ // check NaN cases
            const double limit = 45.0;
            if(offset > limit){
                printf("[LENTIL] sensor offset bigger than maxlimit: %f > %f", offset, limit);
                return limit;
            } else if(offset < -limit){
                printf("[LENTIL] sensor offset smaller than minlimit: %f < %f", offset, -limit);
                return -limit;
            } else {
                return offset; // in mm
            }
        } else {
            return 0.0;
        }

    }



    // returns sensor offset in mm
    inline double camera_set_focus_infinity()
    {
        double parallel_ray_height = lens_aperture_housing_radius * 0.1;
        const Eigen::Vector3d target(0.0, parallel_ray_height, AI_BIG);
        Eigen::VectorXd sensor(5); sensor.setZero();
        Eigen::VectorXd out(5); out.setZero();
        sensor[4] = lambda;
        double offset = 0.0;
        int count = 0;

        // just point through center of aperture
        Eigen::Vector2d aperture(0, parallel_ray_height);

        const int S = 4;

        // trace a couple of adjoint rays from there to the sensor and
        // see where we need to put the sensor plane.
        for(int s=1; s<=S; s++){
            for(int k=0; k<2; k++){
            
            // reset aperture
            aperture(0) = 0.0f;
            aperture(1) = parallel_ray_height;

            lens_lt_sample_aperture(target, aperture, sensor, out, lambda);

            if(sensor(2+k) > 0){
                offset += sensor(k)/sensor(2+k);
                count ++;
            }
            }
        }

        offset /= count; 
        
        // check NaN cases
        if(offset == offset){
            return offset;
        } else {return 0.0;}
    }


    

    void camera_model_specific_setup () {

        switch (cameraType){
            case PolynomialOptics:
            {
                focus_distance *= 10.0;
                
                switch (lensModel){
                    #include "../include/auto_generated_lens_includes/load_lens_constants.h"
                }

                AiMsgInfo("[LENTIL CAMERA PO] ----------  LENS CONSTANTS  -----------");
                AiMsgInfo("[LENTIL CAMERA PO] Lens Name: %s", lens_name);
                AiMsgInfo("[LENTIL CAMERA PO] Lens F-Stop: %f", lens_fstop);
                #ifdef DEBUG_LOG
                AiMsgInfo("[LENTIL CAMERA PO] lens_outer_pupil_radius: %f", lens_outer_pupil_radius);
                AiMsgInfo("[LENTIL CAMERA PO] lens_inner_pupil_radius: %f", lens_inner_pupil_radius);
                AiMsgInfo("[LENTIL CAMERA PO] lens_length: %f", lens_length);
                AiMsgInfo("[LENTIL CAMERA PO] lens_back_focal_length: %f", lens_back_focal_length);
                AiMsgInfo("[LENTIL CAMERA PO] lens_effective_focal_length: %f", lens_effective_focal_length);
                AiMsgInfo("[LENTIL CAMERA PO] lens_aperture_pos: %f", lens_aperture_pos);
                AiMsgInfo("[LENTIL CAMERA PO] lens_aperture_housing_radius: %f", lens_aperture_housing_radius);
                AiMsgInfo("[LENTIL CAMERA PO] lens_inner_pupil_curvature_radius: %f", lens_inner_pupil_curvature_radius);
                AiMsgInfo("[LENTIL CAMERA PO] lens_outer_pupil_curvature_radius: %f", lens_outer_pupil_curvature_radius);
                AiMsgInfo("[LENTIL CAMERA PO] lens_inner_pupil_geometry: %s", lens_inner_pupil_geometry.c_str());
                AiMsgInfo("[LENTIL CAMERA PO] lens_outer_pupil_geometry: %s", lens_outer_pupil_geometry.c_str());
                AiMsgInfo("[LENTIL CAMERA PO] lens_field_of_view: %f", lens_field_of_view);
                AiMsgInfo("[LENTIL CAMERA PO] lens_aperture_radius_at_fstop: %f", lens_aperture_radius_at_fstop);
                #endif
                AiMsgInfo("[LENTIL CAMERA PO] --------------------------------------");


                
                AiMsgInfo("[LENTIL CAMERA PO] wavelength: %f nm", lambda);


                if (input_fstop == 0.0) {
                    aperture_radius = lens_aperture_radius_at_fstop;
                } else {
                    double calculated_fstop = 0.0;
                    double calculated_aperture_radius = 0.0;
                    trace_backwards_for_fstop(input_fstop, calculated_fstop, calculated_aperture_radius);
                    
                    AiMsgInfo("[LENTIL CAMERA PO] calculated fstop: %f", calculated_fstop);
                    AiMsgInfo("[LENTIL CAMERA PO] calculated aperture radius: %f mm", calculated_aperture_radius);
                    
                    aperture_radius = std::min(lens_aperture_radius_at_fstop, calculated_aperture_radius);
                }

                AiMsgInfo("[LENTIL CAMERA PO] lens wide open f-stop: %f", lens_fstop);
                AiMsgInfo("[LENTIL CAMERA PO] lens wide open aperture radius: %f mm", lens_aperture_radius_at_fstop);
                AiMsgInfo("[LENTIL CAMERA PO] fstop-calculated aperture radius: %f mm", aperture_radius);
                AiMsgInfo("[LENTIL CAMERA PO] --------------------------------------");


                AiMsgInfo("[LENTIL CAMERA PO] user supplied focus distance: %f mm", focus_distance);

                /*
                AiMsgInfo("[LENTIL CAMERA PO] calculating sensor shift at focus distance:");
                sensor_shift = camera_set_focus(focus_distance, po);
                AiMsgInfo("[LENTIL CAMERA PO] sensor_shift to focus at %f: %f", focus_distance, sensor_shift);
                */

                // logartihmic focus search
                double best_sensor_shift = logarithmic_focus_search(focus_distance);
                AiMsgInfo("[LENTIL CAMERA PO] sensor_shift using logarithmic search: %f mm", best_sensor_shift);
                sensor_shift = best_sensor_shift + extra_sensor_shift;

                /*
                // average guesses infinity focus search
                double infinity_focus_sensor_shift = camera_set_focus(AI_BIG, po);
                AiMsgInfo("[LENTIL CAMERA PO] sensor_shift [average guesses backwards light tracing] to focus at infinity: %f", infinity_focus_sensor_shift);
                */

                // logarithmic infinity focus search
                double best_sensor_shift_infinity = logarithmic_focus_search(999999999.0);
                AiMsgInfo("[LENTIL CAMERA PO] sensor_shift [logarithmic forward tracing] to focus at infinity: %f mm", best_sensor_shift_infinity);
                    
                // bidirectional parallel infinity focus search
                double infinity_focus_parallel_light_tracing = camera_set_focus_infinity();
                AiMsgInfo("[LENTIL CAMERA PO] sensor_shift [parallel backwards light tracing] to focus at infinity: %f mm", infinity_focus_parallel_light_tracing);

                // double check where y=0 intersection point is, should be the same as focus distance
                double test_focus_distance = 0.0;
                bool focus_test = trace_ray_focus_check(sensor_shift, test_focus_distance);
                AiMsgInfo("[LENTIL CAMERA PO] focus test ray: %f mm", test_focus_distance);
                if(!focus_test){
                    AiMsgWarning("[LENTIL CAMERA PO] focus check failed. Either the lens system is not correct, or the sensor is placed at a wrong distance.");
                }

                tan_fov = std::tan(lens_field_of_view / 2.0);

                AiMsgInfo("[LENTIL CAMERA PO] --------------------------------------");

            } break;
            case ThinLens:
            {
                fov = 2.0 * std::atan(sensor_width / (2.0*focal_length));
                tan_fov = std::tan(fov/2.0);
                aperture_radius = (focal_length / (2.0 * input_fstop)) / 10.0;
            } break;
        }
    }
};