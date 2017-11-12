// proposed strategy

// read in lens data
	//if(argc > 1) strncpy(lensfilename, argv[1], 512);
	lensfilename = c++string.c_str();

	// normalize lensname
	lens_canonicalize_name(lensfilename, lens_name);

	// add .fit to lensname
	char fname[1024];
	snprintf(fname, 1024, "%s.fit", lensfilename);

	if(poly_system_read(&poly, fname))
	{
		fprintf(stderr, "[polynomialoptics] could not read poly system `%s'\n", fname);
		// exit(0);
	}
	snprintf(fname, 1024, "%s_ap.fit", lensfilename);

	if(poly_system_read(&poly_aperture, fname))
	{
		fprintf(stderr, "[polynomialoptics] could not read poly system `%s'\n", fname);
		// exit(0);




// solves for the two directions [dx,dy], keeps the two positions [x,y] and the
// wavelength, such that the path through the lens system will be valid, i.e.
// lens_evaluate_aperture(in, out) will yield the same out given the solved for in.
// in: point on sensor. out: point on aperture.
lens_pt_sample_aperture(float *in, float *out, float dist);

// ray origin on sensor in in[0][1]: What's the coordinate frame here? -1 to 1, or 0 to 1?
// wavelength in in[4]: In a non-spectral path tracer, what is the proposed strategy? Choosing a random value in the visible spectrum?
// aperture position in out[0][1]: I imagine I want to use some concentric mapping here to generate a point on the unit circle? Or should I use lens_sample_aperture?



// call lens_evaluate to generate a ray on the exit pupil in out[]. This ray is still in tangent space, right? So then I have to convert it back to camera space? This also returns transmittance, which is a multiplier on the ray weight/color?

// evaluates from sensor (in) to outer pupil (out).
// input arrays are 5d [x,y,dx,dy,lambda] where dx and dy are the direction in
// two-plane parametrization (that is the third component of the direction would be 1.0).
// units are millimeters for lengths and micrometers for the wavelength (so visible light is about 0.4--0.7)
// returns the transmittance computed from the polynomial.
lens_evaluate(const float *in, float *out);