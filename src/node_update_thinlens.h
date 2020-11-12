tl->unitModel = (UnitModel) AiNodeGetInt(cameranode, "units");
tl->sensor_width = AiNodeGetFlt(cameranode, "sensor_width");
tl->focal_length = AiNodeGetFlt(cameranode, "focal_length");
tl->focal_length = clamp_min(tl->focal_length, 0.01);

tl->fstop = AiNodeGetFlt(cameranode, "fstop");
tl->fstop = clamp_min(tl->fstop, 0.01);

tl->focus_distance = AiNodeGetFlt(cameranode, "focus_distance");

tl->enable_dof = AiNodeGetBool(cameranode, "enable_dof");

tl->fov = 2.0 * std::atan(tl->sensor_width / (2.0*tl->focal_length));
tl->tan_fov = std::tan(tl->fov/2.0);
tl->aperture_radius = (tl->focal_length / (2.0 * tl->fstop)) / 10.0;

tl->bidir_min_luminance = AiNodeGetFlt(cameranode, "bidir_min_luminance");

tl->optical_vignetting_distance = AiNodeGetFlt(cameranode, "optical_vignetting_distance");
tl->optical_vignetting_radius = AiNodeGetFlt(cameranode, "optical_vignetting_radius");

tl->abb_spherical = AiNodeGetFlt(cameranode, "abb_spherical");
tl->abb_spherical = clamp(tl->abb_spherical, 0.001, 0.999);
tl->abb_distortion = AiNodeGetFlt(cameranode, "abb_distortion");
tl->abb_coma = AiNodeGetFlt(cameranode, "abb_coma");

tl->bokeh_aperture_blades = AiNodeGetInt(cameranode, "bokeh_aperture_blades");
tl->circle_to_square = AiNodeGetFlt(cameranode, "bokeh_circle_to_square");
tl->circle_to_square = clamp(tl->circle_to_square, 0.01, 0.99);
tl->bokeh_anamorphic = AiNodeGetFlt(cameranode, "bokeh_anamorphic");
tl->bokeh_anamorphic = clamp(tl->bokeh_anamorphic, 0.01, 99999.0);

tl->bokeh_enable_image = AiNodeGetBool(cameranode, "bokeh_enable_image");
tl->bokeh_image_path = AiNodeGetStr(cameranode, "bokeh_image_path");

tl->bidir_sample_mult = AiNodeGetInt(cameranode, "bidir_sample_mult");

tl->bidir_add_luminance = AiNodeGetFlt(cameranode, "bidir_add_luminance");
tl->bidir_add_luminance_transition = AiNodeGetFlt(cameranode, "bidir_add_luminance_transition");

tl->vignetting_retries = AiNodeGetInt(cameranode, "vignetting_retries");

