//
//  Material.h
//  Raytracer
//
//  Created by Piotr Didyk on 14.07.21.
//

#pragma once

#include <string>
#include "glm/glm.hpp"
#include "Textures.h"

class Material {
   private:
    // Unmapped parameters
    glm::vec3 ambient = glm::vec3(0.0);
    glm::vec3 diffuse = glm::vec3(1.0);
    glm::vec3 specular = glm::vec3(0.0);
    float roughness = 0.0;
    float reflectivity = 0.0;  // 0.0 - 1.0

    float refractivity = 0.0;  // 0.0 - 1.0
    // Air index by default
    float refraction_index = 1.0;  // 1.0 - n

    glm::vec3 (*diffuse_procedural)(glm::vec2 uv) = NULL;

    // Mapped parameters
    TextureImage ambient_map;
    TextureImage diffuse_map;

    TextureImage normal_map;
    TextureImage roughness_map;
    TextureImage displacement_map;

   public:
    Material() {}

    void setAmbient(glm::vec3 ambient) {
        this->ambient = ambient;
        this->ambient_map.is_enabled = false;
    }
    void setAmbient(string ambient_file) {
        this->ambient_map = TextureImage(ambient_file);
    }

    void setDiffuse(glm::vec3 diffuse) {
        this->diffuse = diffuse;
        this->diffuse_map.is_enabled = false;
    }
    void setDiffuse(string ambient_file) {
        this->diffuse_map = TextureImage(ambient_file);
    }
    void setDiffuse(glm::vec3 (*procedural_texture)(glm::vec2)) {
        this->diffuse_procedural = procedural_texture;
        this->diffuse_map.is_enabled = false;
    }

    void setRoughness(float roughness) {
        this->roughness = roughness;
        this->roughness_map.is_enabled = false;
    }
    void setRoughness(string roughness_file) {
        this->roughness_map = TextureImage(roughness_file);
    }
    void setShininess(float shininess) {
        setRoughness(glm::pow(0.5/(shininess+0.5), 1.0/4.0));
    }

    void setSpecular(glm::vec3 specular) { this->specular = specular; }


    void setReflectivity(float reflectivity) {
        this->reflectivity = reflectivity;
    }

    void setRefractivity(float refractivity) {
        this->refractivity = refractivity;
    }

    void setRefractionIndex(float refraction_index) {
        this->refraction_index = refraction_index;
    }

    void setNormal(string normal_file) {
        this->normal_map = TextureImage(normal_file);
    }

    

    void setDisplacement(string displacement_file) {
        this->displacement_map = TextureImage(displacement_file);
    }

    // GETTERS
    glm::vec3 getSpecular() { return specular; }
    float getReflectivity() { return reflectivity; }
    float getRefractivity() { return refractivity; }
    float getRefractionIndex() { return refraction_index; }

    glm::vec3 getDiffuse(glm::vec2 uv) {
        if (diffuse_map.is_enabled) {
            return diffuse_map.getPixelFromUV(uv)*10.0f;
        } else if (diffuse_procedural != NULL) {
            return diffuse_procedural(uv);
        } else {
            return diffuse;
        }
    }

    glm::vec3 getAmbient(glm::vec2 uv) {
        if (ambient_map.is_enabled) {
            return ambient_map.getPixelFromUV(uv);
        } else {
            return ambient;
        }
    }

    glm::vec3 getNormal(glm::vec2 uv) {
        if (normal_map.is_enabled) {
            return glm::normalize(2.0f*normal_map.getPixelFromUV(uv) -1.0f);
        } else {
            // TODO: Default value if normal map absent
            return glm::vec3(1);
        }
    }

    bool hasNormalMap() {
        return normal_map.is_enabled;
    }


    float getShininess(glm::vec2 uv) {
        if (roughness_map.is_enabled) {
            glm::vec3 r = roughness_map.getPixelFromUV(uv);
            float roughness = (r.x+r.y+r.z)/3.0;
            return 0.5/glm::pow(roughness,4) -0.5;
        } else {
            // cout << 0.5/glm::pow(roughness,4) -0.5 << endl;
            return (0.5/glm::pow(roughness,4)) -0.5;
        }
    }

    

    float getDisplacement(glm::vec2 uv) {
        if (displacement_map.is_enabled) {
            glm::vec3 d = displacement_map.getPixelFromUV(uv);
            return (d.x+d.y+d.z)/3.0;
        } else {
            return 0.0;
        }
    }
};

