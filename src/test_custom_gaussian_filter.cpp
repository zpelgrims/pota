#include <ai.h>
#include <string.h>
 
AI_FILTER_NODE_EXPORT_METHODS(CustomGaussianFilterMtd);
 
static AtString WIDTH("width");
 
node_parameters
{
   AiParameterFlt("width", 2.0f);
}
 
node_initialize
{
   AiFilterInitialize(node, false, NULL);
}
 
node_update
{
   AiFilterUpdate(node, 2.0);
}
 
node_finish
{
}
 
filter_output_type
{
   switch (input_type)
   {
      case AI_TYPE_RGBA:
         return AI_TYPE_RGBA;
      default:
         return AI_TYPE_NONE;
   }
}
 
inline float filter_gaussian(AtVector2 p, float width) {
    const float r = std::pow(2.0 / width, 2.0) * (std::pow(p.x, 2) + std::pow(p.y, 2));
    if (r > 1.0f) return 0.0;
    return AiFastExp(2 * -r);
}

filter_pixel
{
   const float width = AiNodeGetFlt(node, WIDTH);
 
   float acc_f_weight = 0.0;
   AtRGBA avalue = AI_RGBA_ZERO;
 
   while (AiAOVSampleIteratorGetNext(iterator))
   {
      // take into account adaptive sampling
      float inv_density = AiAOVSampleIteratorGetInvDensity(iterator);
      if (inv_density <= 0.f) continue;
 
      // determine distance to filter center
      const AtVector2& offset = AiAOVSampleIteratorGetOffset(iterator);
      const float filter_weight = filter_gaussian(offset, width);
 
      // accumulate weights and colors
      avalue += AiAOVSampleIteratorGetRGBA(iterator) * inv_density * filter_weight;
      acc_f_weight += filter_weight * inv_density;
   }
 
   // compute final filtered color
   if (acc_f_weight != 0.0f) avalue /= acc_f_weight;
 
   *((AtRGBA*)data_out) = avalue;
}
 
node_loader
{
   if (i>0)
      return false;
 
   node->methods      = CustomGaussianFilterMtd;
   node->output_type  = AI_TYPE_NONE;
   node->name         = "custom_gaussian_filter";
   node->node_type    = AI_NODE_FILTER;
   strcpy(node->version, AI_VERSION);
 
   return true;
}