tl->sensor_width = AiNodeGetFlt(cameranode, "sensor_widthTL");
tl->focal_length = AiNodeGetFlt(cameranode, "focal_lengthTL");
tl->focal_length = clamp_min(tl->focal_length, 0.01);

tl->fstop = AiNodeGetFlt(cameranode, "fstopTL");
tl->fstop = clamp_min(tl->fstop, 0.01);

tl->focus_distance = AiNodeGetFlt(cameranode, "focus_distanceTL");

tl->enable_dof = AiNodeGetBool(cameranode, "enable_dofTL");

tl->fov = 2.0 * std::atan(tl->sensor_width / (2.0*tl->focal_length));
tl->tan_fov = std::tan(tl->fov/2.0);
tl->aperture_radius = (tl->focal_length / (2.0 * tl->fstop)) / 10.0;

tl->bidir_min_luminance = AiNodeGetFlt(cameranode, "bidir_min_luminanceTL");

tl->optical_vignetting_distance = AiNodeGetFlt(cameranode, "optical_vignetting_distanceTL");
tl->optical_vignetting_radius = AiNodeGetFlt(cameranode, "optical_vignetting_radiusTL");

tl->abb_spherical = AiNodeGetFlt(cameranode, "abb_sphericalTL");
tl->abb_spherical = clamp(tl->abb_spherical, 0.001, 0.999);
tl->abb_distortion = AiNodeGetFlt(cameranode, "abb_distortionTL");
tl->abb_coma = AiNodeGetFlt(cameranode, "abb_comaTL");

tl->bokeh_aperture_blades = AiNodeGetInt(cameranode, "bokeh_aperture_bladesTL");
tl->circle_to_square = AiNodeGetFlt(cameranode, "bokeh_circle_to_squareTL");
tl->circle_to_square = clamp(tl->circle_to_square, 0.01, 0.99);
tl->bokeh_anamorphic = AiNodeGetFlt(cameranode, "bokeh_anamorphicTL");
tl->bokeh_anamorphic = clamp(tl->bokeh_anamorphic, 0.01, 99999.0);

tl->bokeh_enable_image = AiNodeGetBool(cameranode, "bokeh_enable_imageTL");
tl->bokeh_image_path = AiNodeGetStr(cameranode, "bokeh_image_pathTL");

tl->bidir_sample_mult = AiNodeGetInt(cameranode, "bidir_sample_multTL");

tl->bidir_add_luminance = AiNodeGetFlt(cameranode, "bidir_add_luminanceTL");
tl->bidir_add_luminance_transition = AiNodeGetFlt(cameranode, "bidir_add_luminance_transitionTL");

tl->vignetting_retries = AiNodeGetInt(cameranode, "vignetting_retriesTL");
tl->proper_ray_derivatives = AiNodeGetBool(cameranode, "proper_ray_derivativesTL");

// make probability functions of the bokeh image
// if (parms.bokehChanged(camera->params)) {
    tl->image.invalidate();
    if (tl->bokeh_enable_image && !tl->image.read(tl->bokeh_image_path.c_str())){
    AiMsgError("[LENTIL CAMERA TL] Couldn't open bokeh image!");
    AiRenderAbort();
    }
// }