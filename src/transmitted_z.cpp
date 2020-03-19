#include <ai.h>
#include <vector>
#include "lentil_thinlens.h"
#include "lentil.h"


#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

AI_SHADER_NODE_EXPORT_METHODS(TransmittedZMtd);


struct PotaBokehAOV
{
    AtString camera_node_type;
    AtString lentil_thinlens_string;
    AtString lentil_po_string;
    AtNode *camera_node;
    unsigned bs;
    std::vector<std::vector<float>> first_hit_dist;
    std::vector<std::vector<bool>> zbuffer_stop_ray;
};
 

enum SampleBokehParams
{
   p_input
};


node_parameters
{
    AiParameterClosure("input");
}

 
node_initialize
{
   AiNodeSetLocalData(node, new PotaBokehAOV());
}

 
node_update
{
    PotaBokehAOV *local_data = (PotaBokehAOV*)AiNodeGetLocalData(node);


    local_data->camera_node = AiUniverseGetCamera();
    const AtNodeEntry *nentry = AiNodeGetNodeEntry(local_data->camera_node);
    local_data->camera_node_type = AtString(AiNodeEntryGetName(nentry));

    local_data->lentil_thinlens_string = AtString("lentil_thinlens");
    local_data->lentil_po_string = AtString("lentil");

    local_data->bs = AiNodeGetInt(AiUniverseGetOptions(), "bucket_size");


    unsigned aa = AiNodeGetInt(AiUniverseGetOptions(), "AA_samples");
    unsigned xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
    unsigned yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");
    local_data->first_hit_dist.clear();
    local_data->first_hit_dist.resize(xres * yres);
    for (auto &element : local_data->first_hit_dist) {
        element.resize(aa*aa); // not sure about this, what about adaptive sampling? Unsure if i'm able to allocate this correctly for every case
    }
    local_data->zbuffer_stop_ray.clear();
    local_data->zbuffer_stop_ray.resize(xres * yres);
    for (auto &element : local_data->zbuffer_stop_ray) {
        element.resize(aa*aa); // not sure about this, what about adaptive sampling? Unsure if i'm able to allocate this correctly for every case
    }

}


node_finish
{
    PotaBokehAOV *local_data = (PotaBokehAOV*)AiNodeGetLocalData(node);
    CameraThinLens *tl = (CameraThinLens*)AiNodeGetLocalData(local_data->camera_node);

    unsigned xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
    unsigned yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");
    std::vector<float> image(yres * xres * 4);
    int offset = -1;

    for(unsigned pixelnumber = 0; pixelnumber < (xres * yres); pixelnumber++){
        float pixelvalue = 0.0;
        for (unsigned si = 0; si < tl->zbuffer_transmitted[pixelnumber].size(); si++){
            pixelvalue += tl->zbuffer_transmitted[pixelnumber][si];
        }
        pixelvalue /= double(tl->zbuffer_transmitted[pixelnumber].size());
        
        image[++offset] = pixelvalue;
        image[++offset] = pixelvalue;
        image[++offset] = pixelvalue;
        image[++offset] = pixelvalue;
    }
      
    SaveEXR(image.data(), xres, yres, 4, 0, "/home/cactus/lentil/pota/tests/zdepth_transmission/zdepth_transmitted_buffer.exr");
    
    delete local_data;
}


shader_evaluate
{
    PotaBokehAOV *local_data = (PotaBokehAOV*)AiNodeGetLocalData(node);
    CameraThinLens *tl = (CameraThinLens*)AiNodeGetLocalData(local_data->camera_node);
    Camera *po = (Camera*)AiNodeGetLocalData(local_data->camera_node);

    unsigned pixelnumber = (sg->y * tl->xres) + sg->x;    

    if (sg->bounces == 0){
        local_data->first_hit_dist[pixelnumber][sg->si] = AiV3Dist(sg->Ro, sg->P);
    }
    
    if (sg->Rt & AI_RAY_ALL_TRANSMIT){

        AtNode *curr_shader = sg->shader;
        // AtNode *linked_shader = AiNodeGetLink(curr_shader, "input", NULL);
        // const char *node_name = AiNodeGetName(linked_shader);
        // AiMsgInfo("nodename: %s", node_name);
        // AiMsgInfo("transm_val: %f", AiNodeGetFlt(linked_shader, "transmission"));
        // if (local_data->zbuffer_stop_ray[pixelnumber][sg->si] == false){
            // tl->zbuffer_transmitted[pixelnumber][sg->si] += local_data->zbuffer_intermediate[pixelnumber][sg->si];

            float overscan_normalize = 1.0;
            if (sg->x % local_data->bs == 0 || sg->x % local_data->bs == local_data->bs-1 || sg->y % local_data->bs == 0 || sg->y % local_data->bs == local_data->bs-1) overscan_normalize = .7;
            if ((sg->x % local_data->bs == 0 || sg->x % local_data->bs == local_data->bs-1) && (sg->y % local_data->bs == 0 || sg->y % local_data->bs == local_data->bs-1)) overscan_normalize = 0.5;

            if (local_data->camera_node_type == local_data->lentil_thinlens_string){
                if (tl->zbuffer_transmitted[pixelnumber][sg->si] == 0.0) tl->zbuffer_transmitted[pixelnumber][sg->si] += local_data->first_hit_dist[pixelnumber][sg->si];
                // local_data->zbuffer_intermediate[pixelnumber][sg->si] = sg->Rl * overscan_normalize;
                tl->zbuffer_transmitted[pixelnumber][sg->si] += sg->Rl * overscan_normalize;
                // if (AiNodeGetFlt(linked_shader, "transmission") > 0.0) local_data->zbuffer_stop_ray[pixelnumber][sg->si] = true;
                
                // AiMsgInfo("sg->si: %d", sg->si);
            } 
            // else if (local_data->camera_node_type == local_data->lentil_po_string){
            //     po->zbuffer_transmitted[pixelnumber][sg->si] += sg->Rl * overscan_normalize;
            // }
        // }
        
    }

    sg->out.CLOSURE() = AiShaderEvalParamClosure(p_input);
}
 
node_loader
{
  if (i != 0) return false;
  node->methods     = TransmittedZMtd;
  node->output_type = AI_TYPE_CLOSURE;
  node->name        = "lentil_transmitted_z";
  node->node_type   = AI_NODE_SHADER;
  strcpy(node->version, AI_VERSION);
  return true;
}