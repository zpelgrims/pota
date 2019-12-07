#### Lentil is an advanced camera toolkit for the Arnold renderer. It is a set of shaders that extends creative control and physical correctness, while significantly reducing the time (~30x) needed to render bokeh through bidirectional sampling of camera rays.

> ADD COMPARISON OF ARNOLD DEFAULT, THINLENS & POLYNOMIAL OPTICS (inlude rendertimes)

As often is the case in CG, there's a choice between speed (thin-lens) and physical correctness (polynomial optics). This is why multiple camera models are provided:

#### Thin-Lens features:

- Many times faster than default Arnold camera (~30x), achieved by decoupling camera rays from primary rays
- Empirical Cateye bokeh (optical vignetting)
- Anamorphic bokeh
- Image-based bokeh
- Add additional luminance to bokeh only
- Circle to square transitions (useful for anamorphic look)
- Empirical Chromatic aberration

#### Polynomial optics features:

- State-of-the-art in speed/physical correctness trade-off
- Many times faster than [pota]() due to bidirectional sampling
- Takes data from lens patents (render through a 1900's Petzal, 1930's Cooke Speed Panchro or [any of the available lenses]())
- Spectrally correct chromatic aberration
- Physically correct distortion, cat-eye, etc ...
- Prime lenses only



## A few considerations

Lentil has to balance on the cusp of what is possible to do through Arnold's public API. It is not currently possible to feed the bidirectionally sampled bokeh back into an interactive render session. You still get a 1-to-1 preview in interactive sessions (using regular forward tracing), just not the result of the resolved bidirectional tracing.

> **Note:** If an interactive preview of the clean bokeh is required for your workflow, there are some possibilities -- e.g through [aton](). If there is interest in this route, [let us know]().

## Available lenses

> Add all available lenses

> **Note:** Custom lenses can be implemented if the geometrical data is available.

## Documentation

### Bidirectional sampling
The bidirectional component of Lentil comes as a custom driver, which means that it will output additional AOV's which contain the supersampled bokeh *to disk*. To enable this for certain AOVs, add an additional driver to the AOV:

> INSERT IMAGE ON ADDING ADDITIONAL DRIVER

All AOVs that need to contain the supersampled bokeh need to point to the same driver node.

> INSERT GIF ON CONNECTING TO SAME DRIVER

> **Note**: RGBA/RGB type-aov's will be filtered using a gaussian filter with radius specified in the driver settings (not the global settings). All other AOV datatypes will automatically use a closest filter. The filter set next to the driver does not get used.

Lentil will output the images in the location specified in the lentil camera options: [bidir_output_path]().

### Image based bokeh

Custom bokeh images can be used. Since probability functions need to be calculated before rendering, it is advised to use low-resolution images (e.g 250px * 250 px). There is no noticeable visual difference to high resolution images.

We've sourced a few example images that can be used:

> ADD IMAGES OF BOKEH KERNELS
