CONTAINER AITAG_LENTIL
{
   NAME aitag_lentil;

   GROUP C4DAI_LENTIL_MAIN_GRP
   {
      
      GROUP C4DAI_LENTIL_GENERAL_GRP
      {
         DEFAULT 1;

         AIPARAM C4DAIP_LENTIL_UNITS {}
         AIPARAM C4DAIP_LENTIL_LENS_MODEL {}
         AIPARAM C4DAIP_LENTIL_SENSOR_WIDTH {}
         AIPARAM C4DAIP_LENTIL_WAVELENGTH {}
         AIPARAM C4DAIP_LENTIL_DOF {}
         AIPARAM C4DAIP_LENTIL_FSTOP {}
         AIPARAM C4DAIP_LENTIL_FOCUS_DISTANCE {}
         AIPARAM C4DAIP_LENTIL_EXTRA_SENSOR_SHIFT {}
         AIPARAM C4DAIP_LENTIL_BOKEH_APERTURE_BLADES {}
      }
      
      GROUP C4DAI_LENTIL_BOKEH_IMAGE_GRP
      {
         DEFAULT 1;

         AIPARAM C4DAIP_LENTIL_BOKEH_ENABLE_IMAGE {}
         AIPARAM C4DAIP_LENTIL_BOKEH_IMAGE_PATH {}
      }

      GROUP C4DAI_LENTIL_BIDIRECTIONAL_GRP
      {
         DEFAULT 1;

         AIPARAM C4DAIP_LENTIL_BIDIR_MIN_LUMINANCE {}
         AIPARAM C4DAIP_LENTIL_BIDIR_SAMPLE_MULT {}
         AIPARAM C4DAIP_LENTIL_BIDIR_ADD_LUMINANCE {}
         AIPARAM C4DAIP_LENTIL_BIDIR_ADD_LUMINANCE_TRANSITION {}
         AIPARAM C4DAIP_LENTIL_BIDIR_DEBUG {}
      }
         
      GROUP C4DAI_LENTIL_ADVANCED_GRP
      {
         DEFAULT 0;

         AIPARAM C4DAIP_LENTIL_VIGNETTING_RETRIES {}
      }
         

      GROUP C4DAI_LENTIL_ARNOLDNATIVE_GRP
      {
         DEFAULT 0;

         AIPARAM C4DAIP_LENTIL_POSITION {}
         AIPARAM C4DAIP_LENTIL_LOOK_AT {}
         AIPARAM C4DAIP_LENTIL_UP {}
         AIPARAM C4DAIP_LENTIL_MATRIX {}
         AIPARAM C4DAIP_LENTIL_HANDEDNESS {}
         AIPARAM C4DAIP_LENTIL_NEAR_CLIP {}
         AIPARAM C4DAIP_LENTIL_FAR_CLIP {}
         AIPARAM C4DAIP_LENTIL_SCREEN_WINDOW_MIN {}
         AIPARAM C4DAIP_LENTIL_SCREEN_WINDOW_MAX {}
         AIPARAM C4DAIP_LENTIL_SHUTTER_START {}
         AIPARAM C4DAIP_LENTIL_SHUTTER_END {}
         AIPARAM C4DAIP_LENTIL_SHUTTER_TYPE {}
         AIPARAM C4DAIP_LENTIL_SHUTTER_CURVE {}
         AIPARAM C4DAIP_LENTIL_ROLLING_SHUTTER {}
         AIPARAM C4DAIP_LENTIL_ROLLING_SHUTTER_DURATION {}
         AIPARAM C4DAIP_LENTIL_MOTION_START {}
         AIPARAM C4DAIP_LENTIL_MOTION_END {}
         AIPARAM C4DAIP_LENTIL_EXPOSURE {}
         AIPARAM C4DAIP_LENTIL_FILTERMAP {}
      }
   }
}

