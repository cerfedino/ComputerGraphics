//
//  Material.h
//  Raytracer
//
//  Created by Piotr Didyk on 14.07.21.
//

#ifndef Material_h
#define Material_h

#include "glm/glm.hpp"

/**
 Structure describing a material of an object
 */
struct Material{
    glm::vec3 ambient = glm::vec3(0.0); ///< Ambient coefficient
    glm::vec3 diffuse = glm::vec3(1.0); ///< Diffuse coefficient
    glm::vec3 specular = glm::vec3(0.0); ///< Specular coefficient
    float shininess = 0.0; ///< Exponent for Phong model
};

#endif /* Material_h */
