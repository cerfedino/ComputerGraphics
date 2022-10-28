# Assignment 5 - Cone and transformations
<image src="./render/result.gif" width="60%">

## Authors
- Alessandro Gobbetti
- Albert Cerfeda

## Solved exercises
- Exercise 1
- Exercise 2
- Exercise 3

## How to compile and run
A `Makefile` is provided, to compile and run the project run the following command:
```bash
make
```
This will create a folder called `render` where the output images will be stored.
The `.ppm` file is also converted to `.png` for easier viewing.

By default, the program will render an image of the scene described by the `sceneDefinition()` function in `renderer.h`.

To render an animation, run the following command:
```bash
make animation
```
A `.gif` loop animation and a `.mp4` video will be created.
All the frames are stored in a new folder called `render/animation`.
The animation is defined in the `animation.cpp` file.

To render both the image and the animation, run the following command:
```bash
make renderall
```


By running the following command :
```bash
make clean
```
The `render` folder and all the executables will be deleted.
