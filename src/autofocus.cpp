// (c) Johannes Hanika

void camera_set_focus(camera_t *c, float dist){
    c->focus = dist;

    const float dm2mm = 100.0f;

    // camera space vector to v1:
    const float target[3] = { 0.0, 0.0, dm2mm * dist };

    // initialize 5d light fields
    float sensor[5] = {0.0f}, out[5] = {0.0f};

    // set wavelength
    sensor[4] = .5f;

    float off = 0.0f;
    int cnt = 0;

    // just point through center of aperture
    float aperture[2] = {0.0f, 0.0f};

    const float aperture_radius = camera_aperture_radius(c);
    const int S = 4;

    // trace a couple of adjoint rays from there to the sensor and
    // see where we need to put the sensor plane.
    for(int s=1; s<=S; s++){
      for(int k=0; k<2; k++){

        // reset aperture
        aperture[0] = aperture[1] = 0.0f;

        aperture[k] = aperture_radius * s/(S+1.0f); // (1to4)/(4+1) = .2, .4, .6, .8

        lens_lt_sample_aperture(target, aperture, sensor, out, .5f);

        if(sensor[2+k] > 0){
            off += sensor[k]/sensor[2+k];
            cnt ++;
        }
      }
    }

    off /= cnt; // average guesses

    // the focus plane/sensor offset:
    // negative because of reverse direction
    if(off == off){ // check NaN cases
      const float limit = 45.0f; // why this hard limit?
      if(off > limit){
          c->focus_sensor_offset = limit;
      } else if(off < -limit){
          c->focus_sensor_offset = -limit;
      } else {
          c->focus_sensor_offset = off; // in mm
      }
    }
}