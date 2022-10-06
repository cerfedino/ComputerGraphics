#!/bin/sh
mkdir ./render &> /dev/null;
if [[ $1 -eq "--open" ]]
then
  g++ src/main.cpp && ./a.out && ffmpeg -y -i "render/result.ppm" "render/result.png"&> /dev/null && (xdg-open render/result.png | open render/result.png )
else
  g++ src/main.cpp && ./a.out && ffmpeg -y -i "render/result.ppm" "render/result.png"&> /dev/null
fi