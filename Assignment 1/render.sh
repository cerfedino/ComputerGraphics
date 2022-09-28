#!/bin/sh
if [[ $1 -eq "--render" ]]
then
  g++ src/main.cpp && ./a.out && ffmpeg -i "result.ppm" "result.png" && (xdg-open result.png &> /dev/null || open result.png &> /dev/null)
else
  g++ src/main.cpp && ./a.out && ffmpeg -i "result.ppm" "result.png"
fi