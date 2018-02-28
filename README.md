**POTA** is an implementation of [Sparse high-degree polynomials for wide-angle lenses [2016]](https://cg.ivd.kit.edu/publications/2016/lens_sparse_poly/2016_optics.pdf) for the [Arnold renderer](www.solidangle.com). It renders images with high-order aberrations, at a fraction of the cost of tracing rays through lens elements. It is done by pre-calculating fitted polynomials, which serve as a black-box to transform the rays on the sensor to rays on the outer pupil. All credit goes out to the authors of the paper, I only wrote the implementation for Arnold.

**See the full documentation [here](http://zenopelgrims.com/polynomial-optics-arnold/)**


![pota_comparison_thinlens_polynomial_optics_zeiss_biotar](https://raw.githubusercontent.com/zpelgrims/pota/master/tests/website_comparison_images/pota_arnold_camera_shader_comparison_biotar_thinlens.gif)


***

**Compile instructions:**
Base requirement: Arnold >5.0.2.0

>OSX:
```
open makefile #change arnold sdk path
mkdir bin
make
```

>Windows

```
using the "x64 native tools command prompt for VS 2017"

cl /LD /I ...\Arnold-5.0.2.0-windows\include /EHsc /O2 pota.cpp /link /LIBPATH:...\Arnold-5.0.2.0-windows\lib ai.lib
```