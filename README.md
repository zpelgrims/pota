
**POTA** is an implementation of [Sparse high-degree polynomials for wide-angle lenses [2016]](https://cg.ivd.kit.edu/publications/2016/lens_sparse_poly/2016_optics.pdf) for the [Arnold renderer](www.solidangle.com). It renders images with high-order aberrations, at a fraction of the cost of tracing rays through lens elements. It is done by pre-calculating fitted polynomials, which serve as a black-box to transform the rays on the sensor to rays on the outer pupil. All credit goes out to the authors of the paper, I only wrote the implementation for Arnold.

**Read the full documentation [here](http://zenopelgrims.com/polynomial-optics-arnold/).**


![pota_comparison_thinlens_polynomial_optics_zeiss_biotar](https://raw.githubusercontent.com/zpelgrims/pota/master/tests/website_comparison_images/pota_arnold_camera_shader_comparison_biotar_thinlens.gif)


***

**Compile instructions:**

Base requirement: Arnold >5.0.2.0

>OSX:
```
open makefile     ### change arnold sdk path
mkdir bin
make
```

>Windows

```
###   using the "x64 native tools command prompt for VS 2017"

cl /LD /I ...\Arnold-5.0.2.0-windows\include /EHsc /O2 pota.cpp /link /LIBPATH:...\Arnold-5.0.2.0-windows\lib ai.lib
```

***

**Roadmap:**
	
1.4:
- Calculate accurate infinity focus before lens generation (lentil)
- Compute focal length before lens generation so I can properly match the lenses (lentil)
- Fix clipping, not sure why but pota's is closer than the perspective camera by a factor of 10


2.0:
- add energy-redistribution bidirectional sampling (aov shader)
- fix nans of double gauss angenieux (energy redistribution)


2.x:
- bidirectional energy redistribution: check for intersections along P->Lens path
- bidirectional energy redistribution: come up with better triggering of backtracing, based on sample intensity, distance from focal point, fstop, ..?
- add support for x and y cylinders for newer anamorphic lenses (lentil)

3.0:
- remove arnold library dependency to make pota renderer agnostic