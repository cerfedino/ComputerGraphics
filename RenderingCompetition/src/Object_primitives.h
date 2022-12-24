#pragma once

#include "Object.h"

/**
 Implementation of the class Object for sphere shape.
 */
class Sphere : public Object {
   private:
    const float radius = 1;                    ///< Local radius of the sphere
    const glm::vec3 center = glm::vec3(0.0f);  ///< Local center of the sphere

   public:
    /**
     The constructor of the sphere
     @param center Center of the sphere
     @param color Color of the sphere
     */
    Sphere(glm::vec3 color) {
        this->color = color;
    }
    Sphere(glm::vec3 color, glm::mat4 transformation) {
        this->color = color;
        setTransformation(transformation);
    }
    Sphere(Material material, glm::mat4 transformation) {
        this->material = material;
        setTransformation(transformation);
    }
    /** Implementation of the intersection function*/
    Hit intersect(Ray ray) {
        Hit hit;
        hit.hit = false;  // by default, there is no intersection

        Ray localRay = toLocalRay(ray);

        const glm::vec3 c = center - localRay.origin;  // center of the sphere in the coordinate system of the ray

        const float a = glm::dot(c, localRay.direction);

        const float D_squared = glm::dot(c, c) - a * a;
        const float D = sqrt(D_squared);  // distance from the center of the sphere to the ray

        float closest_t;
        // No intersection
        if (D > radius) {
            return hit;

            // Two intersections
        } else if (D < radius) {
            // Distance of first point (always the closest)
            const float t1 = a - sqrt(radius * radius - D_squared);
            // float t2 = a + sqrt(radius*radius - D_squared);

            // We only get the smallest positive between the two points
            closest_t = t1 >= 0 ? t1 : a + sqrt(radius * radius - D_squared);

            // One intersection
        } else {
            closest_t = a + sqrt(radius * radius - D_squared);
        }

        // We ignore intersection points behind the camera
        if (closest_t < 0) {
            return hit;
        }

        hit.hit = true;
        hit.distance = closest_t;
        hit.intersection = localRay.origin + localRay.direction * hit.distance;
        hit.normal = glm::normalize(hit.intersection - center);
        hit.normal_shading = hit.normal;
        hit.material = this->getMaterial();

        // Texture mapping
        hit.uv.y = 0.5 - asin(hit.normal.y) / M_PI;
        hit.uv.x = 2 * (0.5 + atan2(hit.normal.z, hit.normal.x) / (2 * M_PI));

        // remap the shading normals to the normal map
        if (material.hasNormalMap()) {
            glm::vec3 tangent = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), hit.intersection));
            glm::vec3 bitangent = glm::normalize(glm::cross(hit.normal, tangent));

            glm::vec3 normal_map = glm::normalize(material.getNormal(hit.uv));

            glm::mat3 TBN = glm::mat3(tangent, bitangent, hit.normal);

            hit.normal_shading = glm::normalize(TBN * normal_map);
        }

        return toGlobalHit(hit, ray);
    }
};

class Plane : public Object {
   private:
    glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);  ///< Local normal of the plane
    glm::vec3 point = glm::vec3(0.0f);               ///< Local point on the plane

   public:
    //	Plane(glm::vec3 point, glm::vec3 normal) : point(point), normal(normal){
    //	}
    //	Plane(glm::vec3 point, glm::vec3 normal, Material material) : point(point), normal(normal){
    //		this->material = material;
    //	}
    Plane(Material material, glm::mat4 transformation) {
        this->material = material;
        setTransformation(transformation);
    }
    Hit intersect(Ray ray) {
        Hit hit;
        hit.hit = false;

        // Transform ray into local coordinate system
        Ray localRay = toLocalRay(ray);

        float t = glm::dot(point - localRay.origin, normal) / glm::dot(localRay.direction, normal);

        if (t > 0) {
            hit.hit = true;
            hit.intersection = localRay.origin + t * localRay.direction;
            hit.normal = glm::normalize(normal);
            hit.normal_shading = hit.normal;
            hit.distance = glm::distance(localRay.origin, hit.intersection);
            hit.material = this->getMaterial();
        }

        // Texture mapping
        hit.uv.x = 0.1 * hit.intersection.x;
        hit.uv.y = 0.1 * hit.intersection.z;

        // remap the normals to the normal map
        if (material.hasNormalMap()) {
            glm::vec3 tangent = glm::vec3(0, 0, 1);
            glm::vec3 bitangent = glm::vec3(1, 0, 0);

            glm::vec3 normal_map = glm::normalize(material.getNormal(hit.uv));

            glm::mat3 TBN = glm::mat3(tangent, bitangent, normal);

            hit.normal_shading = glm::normalize(TBN * normal_map);
        }

        return toGlobalHit(hit, ray);
    }
};

class Cone : public Object {
   private:
    const float height = 1.0;                           ///< Local height of the cone
    const glm::vec3 H = glm::vec3(0.0f, height, 0.0f);  ///< Local height of the cone
    const glm::vec3 O = glm::vec3(0.0f);                ///< origin of the cone
    const float radius = 1.0;                           ///< Local radius of the cone

   public:
    Cone(Material material) {
        this->material = material;
    }
    Cone(Material material, glm::mat4 transformation) {
        this->material = material;
        setTransformation(transformation);
    }
    Hit intersect(Ray ray) {
        Hit hit;
        hit.hit = false;

        // Transform ray into local coordinate system
        Ray localRay = toLocalRay(ray);

        float tan = (radius / height) * (radius / height);

        float a = (localRay.direction.x * localRay.direction.x) +
                  (localRay.direction.z * localRay.direction.z) -
                  (tan * (localRay.direction.y * localRay.direction.y));
        float b = (2 * localRay.origin.x * localRay.direction.x) +
                  (2 * localRay.origin.z * localRay.direction.z) +
                  (2 * tan * (height - localRay.origin.y) * localRay.direction.y);
        float c = (localRay.origin.x * localRay.origin.x) +
                  (localRay.origin.z * localRay.origin.z) -
                  (tan * ((height - localRay.origin.y) * (height - localRay.origin.y)));

        float delta = b * b - 4 * a * c;
        if (delta < 0) {
            return hit;
        }

        // Hits on the cone

        float t1 = (-b - sqrt(delta)) / (2 * a);
        float t2 = (-b + sqrt(delta)) / (2 * a);

        float closest_t = t1 < 0 ? t2 : t1;
        if (closest_t < 0) {
            return hit;
        }

        hit.distance = closest_t;
        hit.intersection = localRay.origin + localRay.direction * hit.distance;

        // create a plain at the base of the cone
        Plane base = Plane(material, genTRMat(glm::vec3(0), glm::vec3(180.0, 0, 0), glm::vec3(0.5f)));
        Hit baseHit = base.intersect(localRay);

        // Check for intersections on the rounded base of the cone
        if (baseHit.hit && (baseHit.distance < closest_t || (hit.intersection.y < 0 || hit.intersection.y > height))) {
            float distance = glm::distance(baseHit.intersection, O);
            if (distance <= radius) {
                return toGlobalHit(baseHit, ray);
            }
        }

        // Discards intersections on the direction of the cone sides that are outside the cone shape.
        if (hit.intersection.y < 0 || hit.intersection.y > height) {
            return hit;
        }
        hit.hit = true;
        hit.normal = glm::normalize(hit.intersection);
        hit.normal_shading = hit.normal;
        hit.material = this->getMaterial();

        // Texture mapping
        hit.uv.y = 0.5 - asin(hit.normal.y) / M_PI;
        hit.uv.x = 2 * (0.5 + atan2(hit.normal.z, hit.normal.x) / (2 * M_PI));

        // remap the normals to the normal map
        if (material.hasNormalMap()) {
            glm::vec3 tangent = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), hit.intersection));
            glm::vec3 bitangent = glm::normalize(glm::cross(hit.normal, tangent));

            glm::vec3 normal_map = glm::normalize(material.getNormal(hit.uv));

            glm::mat3 TBN = glm::mat3(tangent, bitangent, hit.normal);

            hit.normal_shading = glm::normalize(TBN * normal_map);
        }

        return toGlobalHit(hit, ray);
    }
};