#include <ai.h>
#include <vector>
#include <iostream>

struct bufferstruct {
    std::vector<std::vector<AtRGBA> > samples;
};

int main() {
    bufferstruct *buffer = new bufferstruct();

    const int xres = 50;
    const int yres = 50;

    buffer->samples.clear();
    buffer->samples.resize(xres*yres);

    for (int i = 0; i < xres*yres; i++) {
        for (int j = 0; j < 3; j++){
            AtRGBA value = AI_RGBA_ZERO + j;
            buffer->samples[i].push_back(value);
        }
    }
    
    for (int k = 0; k < buffer->samples.size(); k++) {
        for (int l = 0; l< buffer->samples[k].size(); l++) {
            std::cout << buffer->samples[k][l].r << std::endl;
        }
    }
}