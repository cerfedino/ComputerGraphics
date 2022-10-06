#!/bin/sh
if [[ $1 -eq "--open" ]]
then
  g++ src/main.cpp && ./a.out && ffmpeg -y -i "result.ppm" "result.png"&> /dev/null && (xdg-open result.png &> /dev/null || open result.png &> /dev/null)
else
  g++ src/main.cpp && ./a.out && ffmpeg -y -i "result.ppm" "result.png"&> /dev/null
fi