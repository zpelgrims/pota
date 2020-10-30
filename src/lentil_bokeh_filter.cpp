#include <ai.h>
#include <vector>
#include "lentil.h"
#include "lens.h"
#include "global.h"

 
AI_FILTER_NODE_EXPORT_METHODS(LentilFilterDataMtd);
 

// world to camera space transform, motion blurred
inline Eigen::Vector3d world_to_camera_space_motionblur(const AtVector sample_pos_ws, const float time_start, const float time_end){
  AtMatrix world_to_camera_matrix_motionblurred;
  float currenttime = linear_interpolate(xor128() / 4294967296.0, time_start, time_end); // should I create new random sample, or can I re-use another one?
  AiWorldToCameraMatrix(AiUniverseGetCamera(), currenttime, world_to_camera_matrix_motionblurred);
  const AtVector camera_space_sample_position_mb = AiM4PointByMatrixMult(world_to_camera_matrix_motionblurred, sample_pos_ws);
  Eigen::Vector3d camera_space_sample_position_mb_eigen (camera_space_sample_position_mb.x, camera_space_sample_position_mb.y, camera_space_sample_position_mb.z);
  return camera_space_sample_position_mb_eigen;
}

inline Eigen::Vector2d sensor_to_pixel_position(const Eigen::Vector2d sensor_position, const float sensor_width, const float frame_aspect_ratio, const double xres, const double yres){
  // convert sensor position to pixel position
  const Eigen::Vector2d s(sensor_position(0) / (sensor_width * 0.5), sensor_position(1) / (sensor_width * 0.5) * frame_aspect_ratio);
  const Eigen::Vector2d pixel((( s(0) + 1.0) / 2.0) * xres, 
                              ((-s(1) + 1.0) / 2.0) * yres);
  return pixel;
}


node_parameters 
{
  AiMetaDataSetBool(nentry, nullptr, "force_update", true);
}
 
node_initialize
{


  static const char *required_aovs[] = {"RGBA RGBA", "VECTOR P", "FLOAT Z", "RGBA transmission", "RGBA lentil_bidir_ignore", NULL};
  AiFilterInitialize(node, true, required_aovs);
  AiNodeSetLocalData(node, new LentilFilterData());
}
 
node_update 
{
  LentilFilterData *bokeh = (LentilFilterData*)AiNodeGetLocalData(node);
  
  bokeh->enabled = true;

  AtNode *cameranode = AiUniverseGetCamera();
  // disable for non-lentil cameras
  if (!AiNodeIs(cameranode, AtString("lentil"))) {
    AiMsgError("[LENTIL FILTER PO] Camera is not of type lentil. A full scene update is required.");
    bokeh->enabled = false;
    return;
  }

  Camera *po = (Camera*)AiNodeGetLocalData(AiUniverseGetCamera());


  // const AtNodeEntry *nentry = AiNodeGetNodeEntry(node);
  // if (AiNodeEntryGetCount(nentry) > 1){
  //   AiMsgError("[LENTIL BIDIRECTIONAL ERROR]: Multiple nodes of type lentil_bokeh_driver exist. "
  //              "Use the lentil_operator to avoid this.");
  // }



  // get camera params & recompute the node_update section to avoid race condition when sharing datastruct
  // note this currently is DOUBLE code!! find a fix!!
 
  po->unitModel = (UnitModel) AiNodeGetInt(cameranode, "unitsPO");
  po->sensor_width = AiNodeGetFlt(cameranode, "sensor_widthPO");
  po->input_fstop = AiNodeGetFlt(cameranode, "fstopPO");
  po->focus_distance = AiNodeGetFlt(cameranode, "focus_distancePO") * 10.0; //converting to mm
  po->lensModel = (LensModel) AiNodeGetInt(cameranode, "lens_modelPO");
  po->bokeh_aperture_blades = AiNodeGetInt(cameranode, "bokeh_aperture_bladesPO");
  po->dof = AiNodeGetBool(cameranode, "dofPO");
  po->vignetting_retries = AiNodeGetInt(cameranode, "vignetting_retriesPO");
  po->bidir_min_luminance = AiNodeGetFlt(cameranode, "bidir_min_luminancePO");
  po->proper_ray_derivatives = AiNodeGetBool(cameranode, "proper_ray_derivativesPO");
  po->bokeh_enable_image = AiNodeGetBool(cameranode, "bokeh_enable_imagePO");
  po->bokeh_image_path = AiNodeGetStr(cameranode, "bokeh_image_pathPO");
  
  po->bidir_sample_mult = AiNodeGetInt(cameranode, "bidir_sample_multPO");
  po->bidir_add_luminance = AiNodeGetFlt(cameranode, "bidir_add_luminancePO");
  po->bidir_add_luminance_transition = AiNodeGetFlt(cameranode, "bidir_add_luminance_transitionPO");

  po->lambda = AiNodeGetFlt(cameranode, "wavelengthPO") * 0.001;
  po->extra_sensor_shift = AiNodeGetFlt(cameranode, "extra_sensor_shiftPO");

  #include "node_update_po.h"

  
  bokeh->xres = AiNodeGetInt(AiUniverseGetOptions(), "xres");
  bokeh->yres = AiNodeGetInt(AiUniverseGetOptions(), "yres");
  bokeh->filter_width = 2.0;
  bokeh->rgba_string = AtString("RGBA");
  bokeh->framenumber = static_cast<int>(AiNodeGetFlt(AiUniverseGetOptions(), "frame"));

  bokeh->zbuffer.clear();
  bokeh->zbuffer.resize(bokeh->xres * bokeh->yres);

  bokeh->time_start = AiCameraGetShutterStart();
  bokeh->time_end = AiCameraGetShutterEnd();


  if (po->bidir_sample_mult == 0){
    bokeh->enabled = false;
    return;
  }


  // prepare framebuffers for all AOVS
  bokeh->aov_list_name.clear();
  bokeh->aov_list_type.clear();

  AtNode* options = AiUniverseGetOptions();
  AtArray* outputs = AiNodeGetArray(options, "outputs");
  for (size_t i=0; i<AiArrayGetNumElements(outputs); ++i) {
    std::string output_string = AiArrayGetStr(outputs, i).c_str();
    std::string lentil_str = "lentil_replaced_filter";

    if (output_string.find(lentil_str) != std::string::npos){
     
      std::string name = split_str(output_string, std::string(" ")).end()[-4];
      std::string type = split_str(output_string, std::string(" ")).end()[-3];

      bokeh->aov_list_name.push_back(AtString(name.c_str()));
      bokeh->aov_list_type.push_back(string_to_arnold_type(type));
      bokeh->image[AtString(name.c_str())].clear();
      bokeh->image[AtString(name.c_str())].resize(bokeh->xres * bokeh->yres);
      bokeh->image_redist[AtString(name.c_str())].clear();
      bokeh->image_redist[AtString(name.c_str())].resize(bokeh->xres * bokeh->yres);
      bokeh->image_unredist[AtString(name.c_str())].clear();
      bokeh->image_unredist[AtString(name.c_str())].resize(bokeh->xres * bokeh->yres);
      bokeh->redist_weight_per_pixel[AtString(name.c_str())].clear();
      bokeh->redist_weight_per_pixel[AtString(name.c_str())].resize(bokeh->xres * bokeh->yres);
      bokeh->unredist_weight_per_pixel[AtString(name.c_str())].clear();
      bokeh->unredist_weight_per_pixel[AtString(name.c_str())].resize(bokeh->xres * bokeh->yres);
    }
  }

  bokeh->pixel_already_visited.clear();
  bokeh->pixel_already_visited.resize(bokeh->xres*bokeh->yres);
  for (int i=0;i<bokeh->pixel_already_visited.size(); ++i) bokeh->pixel_already_visited[i] = false; // not sure if i have to
  
  bokeh->current_inv_density = 0.0;

  if (bokeh->enabled) AiMsgInfo("[LENTIL FILTER PO] Starting bidirectional sampling.");

  AiFilterUpdate(node, 2.0);
}
 
filter_output_type
{
   switch (input_type)
   {
      case AI_TYPE_RGBA:
         return AI_TYPE_RGBA;
      case AI_TYPE_RGB:
         return AI_TYPE_RGB;
      case AI_TYPE_VECTOR:
        return AI_TYPE_VECTOR;
      case AI_TYPE_FLOAT:
        return AI_TYPE_FLOAT;
      case AI_TYPE_INT:
        return AI_TYPE_INT;
      default:
         return AI_TYPE_NONE;
   }
}


filter_pixel
{
  LentilFilterData *bokeh = (LentilFilterData*)AiNodeGetLocalData(node);
  Camera *po = (Camera*)AiNodeGetLocalData(AiUniverseGetCamera());

  int cnt = 0;

  if (bokeh->enabled){
    const double xres = (double)bokeh->xres;
    const double yres = (double)bokeh->yres;
    const double frame_aspect_ratio = xres/yres;

    int px, py;
    AiAOVSampleIteratorGetPixel(iterator, px, py);

    // hack to try avoid running over same pixel twice
    int linear_pixel = px + (py * (double)bokeh->xres);
    if (bokeh->pixel_already_visited[linear_pixel]) {
        return;
    } else bokeh->pixel_already_visited[linear_pixel] = true;


    while (AiAOVSampleIteratorGetNext(iterator)) {
      
      bool redistribute = true;
      bool partly_redistributed = false;

      AtRGBA sample = AiAOVSampleIteratorGetRGBA(iterator);
      const float inv_density = AiAOVSampleIteratorGetInvDensity(iterator);
      bokeh->current_inv_density = inv_density;
      AtVector sample_pos_ws = AiAOVSampleIteratorGetAOVVec(iterator, bokeh->atstring_p);
      float depth = AiAOVSampleIteratorGetAOVFlt(iterator, bokeh->atstring_z); // what to do when values are INF?
    
      if (inv_density <= 0.f) continue; // does this every happen? test
      if (inv_density > 0.2){
        continue; // skip when aa samples are below 3
      }
      
      const float filter_width_half = std::ceil(bokeh->filter_width * 0.5);


      const AtRGBA sample_transmission = AiAOVSampleIteratorGetAOVRGBA(iterator, bokeh->atstring_transmission);
      bool transmitted_energy_in_sample = (AiColorMaxRGB(sample_transmission) > 0.0);
      if (transmitted_energy_in_sample){
        sample.r -= sample_transmission.r;
        sample.g -= sample_transmission.g;
        sample.b -= sample_transmission.b;
      }

      const float sample_luminance = sample.r*0.21 + sample.g*0.71 + sample.b*0.072;
      if (sample_luminance < po->bidir_min_luminance) redistribute = false;
      if (!std::isfinite(depth)) redistribute = false; // not sure if this works.. Z AOV has inf values at skydome hits
      if (AiV3IsSmall(sample_pos_ws)) redistribute = false; // not sure if this works .. position is 0,0,0 at skydome hits
      if (AiAOVSampleIteratorHasAOVValue(iterator, bokeh->atstring_lentil_bidir_ignore, AI_TYPE_RGBA)) redistribute = false;
      
      
      AtMatrix world_to_camera_matrix_static;
      float time_middle = linear_interpolate(0.5, bokeh->time_start, bokeh->time_end);
      AiWorldToCameraMatrix(AiUniverseGetCamera(), time_middle, world_to_camera_matrix_static);
      AtVector camera_space_sample_position_static = AiM4PointByMatrixMult(world_to_camera_matrix_static, sample_pos_ws); // just for CoC size calculation
      switch (po->unitModel){
        case mm:
        {
          camera_space_sample_position_static *= 0.1;
        } break;
        case cm:
        { 
          camera_space_sample_position_static *= 1.0;
        } break;
        case dm:
        {
          camera_space_sample_position_static *= 10.0;
        } break;
        case m:
        {
          camera_space_sample_position_static *= 100.0;
        }
      }

      if (std::abs(camera_space_sample_position_static.z) < (po->lens_length*0.1)) redistribute = false; // sample can't be inside of lens

      

    // ENERGY REDISTRIBUTION
      if (redistribute) {
        
        // additional luminance with soft transition
        float fitted_bidir_add_luminance = 0.0;
        if (po->bidir_add_luminance > 0.0) fitted_bidir_add_luminance = additional_luminance_soft_trans(sample_luminance, po->bidir_add_luminance, po->bidir_add_luminance_transition, po->bidir_min_luminance);

        Eigen::Vector2d sensor_position(0, 0);


      // PROBE RAYS samples to determine size of bokeh & subsequent sample count
        AtVector2 bbox_min (0, 0);
        AtVector2 bbox_max (0, 0);
        int proberays_total_samples = 0;
        int proberays_base_rays = 32;
        int proberays_max_rays = proberays_base_rays * 2;
        std::vector<AtVector2> pixelcache;

        for(int count=0; count<proberays_base_rays; count++) {
          ++proberays_total_samples;
          if (proberays_total_samples >= proberays_max_rays) continue;

          Eigen::Vector3d camera_space_sample_position_mb_eigen = world_to_camera_space_motionblur(sample_pos_ws, bokeh->time_start, bokeh->time_end); //could check if motionblur is enabled
          switch (po->unitModel){
            case mm:
            {
              camera_space_sample_position_mb_eigen *= 0.1;
            } break;
            case cm:
            { 
              camera_space_sample_position_mb_eigen *= 1.0;
            } break;
            case dm:
            {
              camera_space_sample_position_mb_eigen *= 10.0;
            } break;
            case m:
            {
              camera_space_sample_position_mb_eigen *= 100.0;
            }
          }
        
          if(!trace_backwards(-camera_space_sample_position_mb_eigen * 10.0, po->aperture_radius, po->lambda, sensor_position, po->sensor_shift, po, px, py, proberays_total_samples)) {
            --count;
            continue;
          }

          const Eigen::Vector2d pixel = sensor_to_pixel_position(sensor_position, po->sensor_width, frame_aspect_ratio, xres, yres);

          //figure out why sometimes pixel is nan, can't just skip it
          if ((pixel(0) > xres) || (pixel(0) < 0) || (pixel(1) > yres) || (pixel(1) < 0) || 
              (pixel(0) != pixel(0)) || (pixel(1) != pixel(1))) // nan checking
          {
            --count;
            continue;
          }

          // expand bbox
          if (count == 0) {
            bbox_min = {(float)pixel(0), (float)pixel(1)};
            bbox_max = {(float)pixel(0), (float)pixel(1)};
          } else {
            bbox_min = {std::min(bbox_min.x, (float)pixel(0)), std::min(bbox_min.y, (float)pixel(1))};
            bbox_max = {std::max(bbox_max.x, (float)pixel(0)), std::max(bbox_max.y, (float)pixel(1))};
          }

          pixelcache.push_back(AtVector2(pixel(0), pixel(1))); // store for re-use later
        }


        double bbox_area = (bbox_max.x - bbox_min.x) * (bbox_max.y - bbox_min.y);
        if (bbox_area < 20.0) goto no_redist; //might have to increase this?
        int samples = std::floor(bbox_area * po->bidir_sample_mult * 0.001);
        samples = std::ceil((double)(samples) / inv_density);
        samples = std::clamp(samples, 25, 10000); // not sure if a million is actually ever hit.. 75 seems high but is needed to remove stochastic noise
        float inv_samples = 1.0 / static_cast<double>(samples);
        unsigned int total_samples_taken = 0;
        unsigned int max_total_samples = samples*5;
        

        // redist samples
        for(int count=0; count<samples && total_samples_taken < max_total_samples; ++count, ++total_samples_taken) {
          
          Eigen::Vector2d pixel;
          
          if (count < pixelcache.size()){ // redist already taken samples from probe rays
            pixel(0) = pixelcache[count].x; pixel(1) = pixelcache[count].y;
          } else {

            Eigen::Vector3d camera_space_sample_position_mb_eigen = world_to_camera_space_motionblur(sample_pos_ws, bokeh->time_start, bokeh->time_end);  //could check if motionblur is enabled
            switch (po->unitModel){
              case mm:
              {
                camera_space_sample_position_mb_eigen *= 0.1;
              } break;
              case cm:
              { 
                camera_space_sample_position_mb_eigen *= 1.0;
              } break;
              case dm:
              {
                camera_space_sample_position_mb_eigen *= 10.0;
              } break;
              case m:
              {
                camera_space_sample_position_mb_eigen *= 100.0;
              }
            }

            if(!trace_backwards(-camera_space_sample_position_mb_eigen*10.0, po->aperture_radius, po->lambda, sensor_position, po->sensor_shift, po, px, py, total_samples_taken)) {
              --count;
              continue;
            }

            pixel = sensor_to_pixel_position(sensor_position, po->sensor_width, frame_aspect_ratio, xres, yres);

            // if outside of image
            if ((pixel(0) >= xres) || (pixel(0) < 0) || (pixel(1) >= yres) || (pixel(1) < 0) ||
                (pixel(0) != pixel(0)) || (pixel(1) != pixel(1))) // nan checking
            {
              --count; // much room for improvement here, potentially many samples are wasted outside of frame
              continue;
            }
          }

          // >>>> currently i've decided not to filter the redistributed energy. If needed, there's an old prototype in github issue #230

          // write sample to image
          unsigned pixelnumber = static_cast<int>(bokeh->xres * floor(pixel(1)) + floor(pixel(0)));

          for (unsigned i=0; i<bokeh->aov_list_name.size(); i++){
            add_to_buffer(sample, pixelnumber, bokeh->aov_list_type[i], bokeh->aov_list_name[i], 
                          inv_samples, inv_density, fitted_bidir_add_luminance, depth, iterator,
                          bokeh->image_redist, bokeh->redist_weight_per_pixel, bokeh->image, bokeh->zbuffer, 
                          bokeh->rgba_string);
          }
        }

        if (transmitted_energy_in_sample) {
          partly_redistributed = true;
          goto no_redist;
        }
      }



      else { // COPY ENERGY IF NO REDISTRIBUTION IS REQUIRED
      no_redist:

        if (transmitted_energy_in_sample && partly_redistributed) {
          sample.r = sample_transmission.r;
          sample.g = sample_transmission.g;
          sample.b = sample_transmission.b;
        } else if (transmitted_energy_in_sample && !partly_redistributed) {
          sample.r += sample_transmission.r;
          sample.g += sample_transmission.g;
          sample.b += sample_transmission.b;
        }


        // loop over all pixels in filter radius, then compute the filter weight based on the offset not to the original pixel (px, py), but the filter pixel (x, y)
        for (unsigned y = py - filter_width_half; y <= py + filter_width_half; y++) {
          for (unsigned x = px - filter_width_half; x <= px + filter_width_half; x++) {
            
            if (y < 0 || y >= bokeh->yres) continue; // edge fix
            if (x < 0 || x >= bokeh->xres) continue; // edge fix

            const unsigned pixelnumber = static_cast<int>(bokeh->xres * y + x);
            
            const AtVector2 &subpixel_position = AiAOVSampleIteratorGetOffset(iterator); // offset within original pixel
            AtVector2 subpixel_pos_dist = AtVector2((px+subpixel_position.x) - x, (py+subpixel_position.y) - y);
            float filter_weight = filter_gaussian(subpixel_pos_dist, bokeh->filter_width);
            if (filter_weight == 0) continue;

            for (size_t i=0; i<bokeh->aov_list_name.size(); i++){
              add_to_buffer(sample, pixelnumber, bokeh->aov_list_type[i], bokeh->aov_list_name[i], 
                            1.0, inv_density, 0.0, depth, iterator,
                            bokeh->image_unredist, bokeh->unredist_weight_per_pixel, bokeh->image, bokeh->zbuffer, 
                            bokeh->rgba_string);
            }
          }
        }
      }
    }
  }

  // do regular filtering (passthrough) for display purposes
  AiAOVSampleIteratorReset(iterator);
  AtRGBA filtered_value = filter_gaussian_complete(iterator, 2.0);
  *((AtRGBA*)data_out) = filtered_value;
}

 
node_finish
{
   LentilFilterData *bokeh = (LentilFilterData*)AiNodeGetLocalData(node);
   delete bokeh;
}

node_loader
{
   if (i>0) return false;
   node->methods = (AtNodeMethods*) LentilFilterDataMtd;
   node->output_type = AI_TYPE_NONE;
   node->name = "lentil_bokeh_filter";
   node->node_type = AI_NODE_FILTER;
   strcpy(node->version, AI_VERSION);
   return true;
}
 