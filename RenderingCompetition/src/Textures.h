//
//  Textures.h
//  Raytracer
//
#pragma once

#include <string>
#include <iostream>

#include "glm/glm.hpp"
#include "stb_image.h"

using namespace std;

//  Procedural textures
inline glm::vec3 checkerboardTexture(glm::vec2 uv) {
    int n = 30;
    return glm::vec3(abs(int(floor(n * uv.x) + floor(n * uv.y)) % 2));
};

inline glm::vec3 rainbowTexture(glm::vec2 uv) {
    int n = 20;
    int f = abs(int(n * (uv.t + uv.s)) % 3);
    switch (f) {
        case 0:
            return glm::vec3(1, 0, 0);
        case 1:
            return glm::vec3(0, 1, 0);
        default:
            return glm::vec3(0, 0, 1);
    }
};

class TextureImage {
   public:
    bool is_enabled = true;

   private:
    int width;
    int height;
    int channels = 3;
    float *image;

   public:
   TextureImage() {
    is_enabled = false;
   }

    TextureImage(string filename) {
        cout << "Reading:" << filename << endl;

        this->image = stbi_loadf(filename.c_str(), &this->width, &this->height,
                                 nullptr, channels);
        // Return if image is not loaded
        if (this->image == NULL) {
            throw std::invalid_argument("Failed to load image: " + filename);
        }
    }

    glm::vec3 getPixelFromUV(glm::vec2 uv) {

        size_t x = uv.x * width;
        size_t y = uv.y * height;

        x = x % width;
        y = y % height;
        
        const float *p = image + (channels * (y * width + (x)));
        return glm::vec3(p[0], p[1], p[2]); // FIXME
    }
};

