# Pin Transmittance Sampling
Implementation of my masters thesis at KIT (see thesis.pdf).

## Screenshots:
![Alt text](/media/screenshot_1.png?raw=true "path_tracer_with_gui")


## Abstract
 We introduce a novel method to represent heterogeneous participating media for Monte
 Carlo path tracing on the GPU. Instead of storing the medium as a scalar field, we represent
 it as a set of lines passing through the volume. These ’Pins’ are indexed as a low resolution
 5D grid. Tracking is replaced by indexing the closest pin and using it to estimate the
 transmittance or to sample from a free path distribution in either direction along the line.
 Both operations are implemented using bitwise operations and come at a constant low
 cost. Because of the heavy quantization to the grid, structured artifacts are introduced.
 But, when using pins only for multiple scattering, these result in a blur, which is far less
 visible. While contrast is lost around hard boundaries, the effect is barely visible for diffuse
 volumes.
 We evaluate our algorithm for multiple scattering. For low resolution diffuse volumes we
 obtain similar results compared to ray marching. In our best case, we manage to halve the
 total run time, when rendering many instances of overlapping Perlin Noise. Here, we use
 a grid size, that adds a memory overhead of only 10% to the size of the original texture.
 We only evaluate small volumes (≤ 0.25 GB) for our test cases, but suggest exploring the
 application to large volumes for future work


 ## Dependencies

### Contained in repo:
- glfw
- ImGui
- nlohmann_json
- stb
- tiny_obj_loader
- vma
- CVSWriter
- ImPlot
- ImFile
- ktx
- tiny_exr

### Other:
- Vulkan 1.3
- CMake

### C++ Standard
- C++17

## Installing

* Install Latest Vulkan Version (https://vulkan.lunarg.com/)
* clone
```
git clone --recurse-submodules https://github.com/scratlantis/vka.git
```
* Build with CMake


## Authors
Sidney Hansen (Contact: scratlantis@protonmail.com)


## License

This project is licensed under the MIT License - see the LICENSE.md file for details
