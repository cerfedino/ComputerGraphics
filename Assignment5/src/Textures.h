//
//  Textures.h
//  Raytracer
//
//  Created by Piotr Didyk on 19.08.21.
//

#ifndef Textures_h
#define Textures_h


#include "glm/glm.hpp"

glm::vec3 checkerboardTexture(glm::vec2 uv){
    int n = 30;
    return glm::vec3(abs(int(floor(n*uv.x) + floor(n*uv.y)) % 2));
}

glm::vec3 rainbowTexture(glm::vec2 uv){

    int n = 20;
    int f = abs(int(n*(uv.t + uv.s)) % 3);
    switch(f) {
        case 0:  return glm::vec3(1, 0, 0);
        case 1:  return glm::vec3(0, 1, 0);
        default: return glm::vec3(0, 0, 1);
    }
}

#endif /* Textures_h */
