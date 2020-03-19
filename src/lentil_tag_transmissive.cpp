#include <ai.h>
#include <vector>
#include "lentil_thinlens.h"
#include "lentil.h"


#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

AI_SHADER_NODE_EXPORT_METHODS(TransmittedZMtd);


struct PotaBokehAOV {
    AtString camera_node_type;
    AtString lentil_thinlens_string;
    AtString lentil_po_string;
    AtNode *camera_node;
};
 

enum SampleBokehParams {};


node_parameters {}

 
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
        for (unsigned si = 0; si < tl->tag_transmissive[pixelnumber].size(); si++){
            pixelvalue += tl->tag_transmissive[pixelnumber][si] == true ? 1.0 : 0.0;
        }
        pixelvalue /= double(tl->tag_transmissive[pixelnumber].size());
        
        image[++offset] = pixelvalue;
        image[++offset] = pixelvalue;
        image[++offset] = pixelvalue;
        image[++offset] = pixelvalue;
    }
      
    SaveEXR(image.data(), xres, yres, 4, 0, "/home/cactus/lentil/pota/tests/zdepth_transmission/tag_transmissive_buffer.exr");
    
    delete local_data;
}


shader_evaluate
{
    PotaBokehAOV *local_data = (PotaBokehAOV*)AiNodeGetLocalData(node);
    CameraThinLens *tl = (CameraThinLens*)AiNodeGetLocalData(local_data->camera_node);
    Camera *po = (Camera*)AiNodeGetLocalData(local_data->camera_node);

    unsigned pixelnumber = (sg->y * tl->xres) + sg->x;

    AtNode *curr_shader = sg->shader;
    const AtNodeEntry *ne_curr_shader = AiNodeGetNodeEntry(curr_shader);
    // AiMsgInfo("AiNodeEntryGetName(ne_curr_shader): %s", AiNodeEntryGetName(ne_curr_shader));

    if (std::strcmp(AiNodeEntryGetName(ne_curr_shader), "standard_surface") != 0) return; // out when we're not dealing with standard surface shader

    if (AiNodeGetFlt(curr_shader, "transmission") > 0.0 || AiNodeIsLinked(curr_shader, "transmission")){
        tl->tag_transmissive[pixelnumber][sg->si] = true;
    }
    
}
 
node_loader
{
  if (i != 0) return false;
  node->methods     = TransmittedZMtd;
  node->output_type = AI_TYPE_NONE;
  node->name        = "lentil_tag_transmissive";
  node->node_type   = AI_NODE_SHADER;
  strcpy(node->version, AI_VERSION);
  return true;
}