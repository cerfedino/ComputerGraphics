# Assignment 5 - Cone and transformations
<image src="./render/result.gif" width="60%">

## Authors
- Alessandro Gobbetti
- Albert Cerfeda

## Solved exercises
- Exercise 1
- Exercise 2
- Exercise 3

## Rendering
1. `cd` into the target assignment folder
```bash
cd Assignment1
```

2. A `Makefile` is provided, for compiling and rendering:


- To render the **still image**, run the default recipe;
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

All the frames are stored in a new folder called `render/animation`.
The animation frames are joined together by `ffmpeg` into a `.gif` loop animation and an `.mp4` file.
\
\
\
By default, the program will render an image of the scene described by the `sceneDefinition()` function in `renderer.h`.

The animation is defined in the `animation.cpp` file.