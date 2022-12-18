#pragma once

#include <string>


#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/vector_angle.hpp"

#include "base.h"
#include "Material.h"



/**
 General class for the object
 */
class Object {
   protected:
    glm::mat4 transformationMatrix = glm::mat4(1.0f);         ///< Matrix representing the transformation from the local to the global coordinate system
    glm::mat4 inverseTransformationMatrix = glm::mat4(1.0f);  ///< Matrix representing the transformation from the global to the local coordinate system
    glm::mat4 normalMatrix = glm::mat4(1.0f);                 ///< Matrix for transforming normal vectors from the local to the global coordinate system

   public:
    glm::vec3 color;    ///< Color of the object
    Material material;  ///< Structure describing the material of the object
                        /** A function computing an intersection, which returns the structure Hit */
    virtual Hit intersect(Ray ray) = 0;

    /** Function that returns the material struct of the object*/
    const Material *getMaterial() const {
        return &material;
    }
    /** Function that set the material
     @param material A structure describing the material of the object
    */
    void setMaterial(Material material) {
        this->material = material;
    }

    /** Functions for setting up all the transformation matrices
     @param matrix The matrix representing the transformation of the object in the global coordinates */
    void setTransformation(glm::mat4 matrix) {
        transformationMatrix = matrix;
        inverseTransformationMatrix = glm::inverse(transformationMatrix);
        normalMatrix = glm::transpose(inverseTransformationMatrix);
    }

    Ray toLocalRay(Ray ray) {
        ray.origin = glm::vec3(inverseTransformationMatrix * glm::vec4(ray.origin, 1.0f));
        ray.direction = glm::normalize(glm::vec3(inverseTransformationMatrix * glm::vec4(ray.direction, 0.0f)));
        return ray;
    }

    Hit toGlobalHit(Hit localHit, Ray ray) {
        if (!localHit.hit) {
            return localHit;
        }

        Hit globalHit = localHit;
        globalHit.normal = glm::normalize(glm::vec3(normalMatrix * glm::vec4(globalHit.normal, 0.0f)));
        globalHit.normal_shading = glm::normalize(glm::vec3(normalMatrix * glm::vec4(globalHit.normal_shading, 0.0f)));
        globalHit.intersection = glm::vec3(transformationMatrix * glm::vec4(globalHit.intersection, 1.0f));

        globalHit.distance = glm::length(globalHit.intersection - ray.origin);

        return globalHit;
    }
};





