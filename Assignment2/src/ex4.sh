#!/bin/bash

# create a folder to store all the images
mkdir images

# compile 
g++ ./main_ex4.cpp

# run the program and convert rendered images to jpg
for i in {00..120}
do
  ./a.out "images/frame${i}.ppm" ${i} && ffmpeg -y -i "images/frame${i}.ppm" "images/frame${i}.jpg"
done



# render mp4 video from all images
ffmpeg -framerate 60 -pattern_type glob -i 'images/*.jpg' -c:v libx264 -pix_fmt yuv420p out.mp4

# render gif animation from all images
ffmpeg -framerate 60 -pattern_type glob -i 'images/*.jpg' -loop 0 out.gif
