# Computer Graphics assignments

### Assignment 1 - Ray Sphere Intersection
<image src="./Assignment1/render/result.png" width="60%">

### Assignment 2 - Phong Lighting model
<image src="./Assignment2/render/result.gif" width="60%">

### Assignment 3 - Ray-plane intersection, texturing, tone mapping
<image src="./Assignment3/render/result.png" width="60%">

### Assignment 5 - Cone and transformations
<image src="./Assignment5/render/result.gif" width="60%">

### Assignment 6 - Shadows, Reﬂection, and Refraction
<image src="./Assignment6/render/result.gif" width="60%">

### Assignment 8 - WebGL
<image style="display: inline-block" src="./Assignment8/render/cube.png" width="30%">
<image src="./Assignment8/render/triangle.png" width="30%">

### Assignment 9 - Transformations and Lighting
<image src="./Assignment8/render/triangle.png" width="30%">


### Assignment 10 - Texturing
<image src="./Assignment10/render/result.gif" width="60%">


### Rendering Competition
<image src="./RenderingCompetition/render/result1.gif" width="60%">
<image src="./RenderingCompetition/render/result2.png" width="60%">

### Assignment 11 - Mass-spring System
<image src="./Assignment11/render/result.gif" width="60%">




# Rendering
1. `cd` into the target assignment folder
```bash
cd Assignment1
```

2. A `Makefile` is provided, for compiling and rendering:


- To render the **still image**, run the default recipe:
```bash
make
```
- To render the **animation**, run the following command:
```bash
make animation
```
- To render both:
```bash
make all
```
- To remove the `render` folder and all the executables
```bash
make clean
```
\
\
A folder `render` containing the rendered material is created.

We use `ffmpeg` to convert the rendered `.ppm` file into `.png`.

All the frames are stored in `render/animation`.
The animation frames are joined together by `ffmpeg` into a `.gif` loop animation and an `.mp4` file.
\
\
\
By default, the program will render an image of the scene described by the `sceneDefinition()` function in `renderer.h`.

The animation is defined in the `animation.cpp` file.
