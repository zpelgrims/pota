  AiMsgInfo("");

  load_lens_constants(po);
  AiMsgInfo("[LENTIL CAMERA PO] ----------  LENS CONSTANTS  -----------");
  AiMsgInfo("[LENTIL CAMERA PO] Lens Name: %s", po->lens_name);
  AiMsgInfo("[LENTIL CAMERA PO] Lens F-Stop: %f", po->lens_fstop);
#ifdef DEBUG_LOG
  AiMsgInfo("[LENTIL CAMERA PO] lens_outer_pupil_radius: %f", po->lens_outer_pupil_radius);
  AiMsgInfo("[LENTIL CAMERA PO] lens_inner_pupil_radius: %f", po->lens_inner_pupil_radius);
  AiMsgInfo("[LENTIL CAMERA PO] lens_length: %f", po->lens_length);
  AiMsgInfo("[LENTIL CAMERA PO] lens_back_focal_length: %f", po->lens_back_focal_length);
  AiMsgInfo("[LENTIL CAMERA PO] lens_effective_focal_length: %f", po->lens_effective_focal_length);
  AiMsgInfo("[LENTIL CAMERA PO] lens_aperture_pos: %f", po->lens_aperture_pos);
  AiMsgInfo("[LENTIL CAMERA PO] lens_aperture_housing_radius: %f", po->lens_aperture_housing_radius);
  AiMsgInfo("[LENTIL CAMERA PO] lens_inner_pupil_curvature_radius: %f", po->lens_inner_pupil_curvature_radius);
  AiMsgInfo("[LENTIL CAMERA PO] lens_outer_pupil_curvature_radius: %f", po->lens_outer_pupil_curvature_radius);
  AiMsgInfo("[LENTIL CAMERA PO] lens_inner_pupil_geometry: %s", po->lens_inner_pupil_geometry.c_str());
  AiMsgInfo("[LENTIL CAMERA PO] lens_outer_pupil_geometry: %s", po->lens_outer_pupil_geometry.c_str());
  AiMsgInfo("[LENTIL CAMERA PO] lens_field_of_view: %f", po->lens_field_of_view);
  AiMsgInfo("[LENTIL CAMERA PO] lens_aperture_radius_at_fstop: %f", po->lens_aperture_radius_at_fstop);
#endif
  AiMsgInfo("[LENTIL CAMERA PO] --------------------------------------");


  
  AiMsgInfo("[LENTIL CAMERA PO] wavelength: %f nm", po->lambda);


  if (po->input_fstop == 0.0) {
    po->aperture_radius = po->lens_aperture_radius_at_fstop;
  } else {
    double calculated_fstop = 0.0;
    double calculated_aperture_radius = 0.0;
    trace_backwards_for_fstop(po, po->input_fstop, calculated_fstop, calculated_aperture_radius);
    
    AiMsgInfo("[LENTIL CAMERA PO] calculated fstop: %f", calculated_fstop);
    AiMsgInfo("[LENTIL CAMERA PO] calculated aperture radius: %f mm", calculated_aperture_radius);
    
    po->aperture_radius = std::min(po->lens_aperture_radius_at_fstop, calculated_aperture_radius);
  }

  AiMsgInfo("[LENTIL CAMERA PO] lens wide open f-stop: %f", po->lens_fstop);
  AiMsgInfo("[LENTIL CAMERA PO] lens wide open aperture radius: %f mm", po->lens_aperture_radius_at_fstop);
  AiMsgInfo("[LENTIL CAMERA PO] fstop-calculated aperture radius: %f mm", po->aperture_radius);
  AiMsgInfo("[LENTIL CAMERA PO] --------------------------------------");


  AiMsgInfo("[LENTIL CAMERA PO] user supplied focus distance: %f mm", po->focus_distance);

  /*
  AiMsgInfo("[LENTIL CAMERA PO] calculating sensor shift at focus distance:");
  po->sensor_shift = camera_set_focus(po->focus_distance, po);
  AiMsgInfo("[LENTIL CAMERA PO] sensor_shift to focus at %f: %f", po->focus_distance, po->sensor_shift);
  */

  // logartihmic focus search
  double best_sensor_shift = logarithmic_focus_search(po->focus_distance, po);
  AiMsgInfo("[LENTIL CAMERA PO] sensor_shift using logarithmic search: %f mm", best_sensor_shift);
  po->sensor_shift = best_sensor_shift + po->extra_sensor_shift;

  /*
  // average guesses infinity focus search
  double infinity_focus_sensor_shift = camera_set_focus(AI_BIG, po);
  AiMsgInfo("[LENTIL CAMERA PO] sensor_shift [average guesses backwards light tracing] to focus at infinity: %f", infinity_focus_sensor_shift);
  */

  // logarithmic infinity focus search
  double best_sensor_shift_infinity = logarithmic_focus_search(999999999.0, po);
  AiMsgInfo("[LENTIL CAMERA PO] sensor_shift [logarithmic forward tracing] to focus at infinity: %f mm", best_sensor_shift_infinity);
      
  // bidirectional parallel infinity focus search
  double infinity_focus_parallel_light_tracing = camera_set_focus_infinity(po);
  AiMsgInfo("[LENTIL CAMERA PO] sensor_shift [parallel backwards light tracing] to focus at infinity: %f mm", infinity_focus_parallel_light_tracing);

  // double check where y=0 intersection point is, should be the same as focus distance
  double test_focus_distance = 0.0;
  bool focus_test = trace_ray_focus_check(po->sensor_shift, test_focus_distance, po);
  AiMsgInfo("[LENTIL CAMERA PO] focus test ray: %f mm", test_focus_distance);
  if(!focus_test){
    AiMsgWarning("[LENTIL CAMERA PO] focus check failed. Either the lens system is not correct, or the sensor is placed at a wrong distance.");
  }

  po->tan_fov = std::tan(po->lens_field_of_view / 2.0);

  AiMsgInfo("[LENTIL CAMERA PO] --------------------------------------");
  

  // make probability functions of the bokeh image
  // if (parms.bokehChanged(po->params)) {
    po->image.invalidate();
    if (po->bokeh_enable_image && !po->image.read(po->bokeh_image_path.c_str())){
      AiMsgError("[LENTIL CAMERA PO] Couldn't open bokeh image!");
      AiRenderAbort();
    }
  // }
  AiMsgInfo("");