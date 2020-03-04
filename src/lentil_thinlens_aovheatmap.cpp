#include <ai.h>
#include "lentil_thinlens.h"

// need to add metadata: add a booleanmetadata named "aov_shader" on the node itself

AI_SHADER_NODE_EXPORT_METHODS(LentilBokehHeatmapMtd);

struct LentilBokehHeatmapData
{
  AtString heatmap_aov_name;
};
 

enum SampleBokehParams
{
};


node_parameters
{
  //  AiMetaDataSetBool(nentry, NULL, "aov_shader", true);
}

 
node_initialize
{
   AiNodeSetLocalData(node, new LentilBokehHeatmapData());
}

 
node_update
{
    LentilBokehHeatmapData *heatmap_data = (LentilBokehHeatmapData*)AiNodeGetLocalData(node);
    heatmap_data->heatmap_aov_name = AtString("lentil_thinlens_heatmap");
    AiAOVRegister(heatmap_data->heatmap_aov_name, AI_TYPE_RGBA, AI_AOV_BLEND_OPACITY);

    // CameraThinLens *tl = (CameraThinLens*)AiNodeGetLocalData(AiUniverseGetCamera());
    // // this pointer could be buggy (untested)
    // AtNode* cameranode = AiUniverseGetCamera();
    // #include "node_update_thinlens.h"
}


node_finish
{
  LentilBokehHeatmapData *heatmap_data = (LentilBokehHeatmapData*)AiNodeGetLocalData(node);
  delete heatmap_data;
}


shader_evaluate
{
    LentilBokehHeatmapData *heatmap_data = (LentilBokehHeatmapData*)AiNodeGetLocalData(node);
    // CameraThinLens *tl = (CameraThinLens*)AiNodeGetLocalData(AiUniverseGetCamera());

    // unsigned int samples = 0;
    // AtRGBA sample_energy = sg->out.RGBA();
    // const float sample_luminance = sample_energy.r*0.21 + sample_energy.g*0.71 + sample_energy.b*0.072;
    // if (sample_luminance > tl->bidir_min_luminance)
    // {
    //     samples = 1;
    // }

    // AtRGBA out = {samples, samples, samples, sample_energy.a};
    // out = sample_energy;

    // write AOV only if in use
    if ((sg->Rt & AI_RAY_CAMERA))
    {
      AiAOVSetRGBA(sg, AtString("lentil_thinlens_heatmap"), sg->out.RGBA());
      //if (AiAOVEnabled(AtString("lentil_thinlens_heatmap"), AI_TYPE_RGBA)) );
    }
    AiAOVSetRGBA(sg, AtString("lentil_thinlens_heatmap"), sg->out.RGBA());
}
 
node_loader
{
  if (i != 0) return false;
  node->methods     = LentilBokehHeatmapMtd;
  node->output_type = AI_TYPE_RGBA;
  node->name        = "lentilBokehHeatmap";
  node->node_type   = AI_NODE_SHADER;
  strcpy(node->version, AI_VERSION);
  return true;
}