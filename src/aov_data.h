#pragma once

#include <ai.h>
#include <string>
#include <vector>

#include "global.h"



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