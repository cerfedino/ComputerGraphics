/**
@file main.cpp
*/

#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include <vector>
#include <algorithm>
#include <omp.h>
#include <chrono>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/vector_angle.hpp"
#include <string>
#include "Image.h"
#include "Material.h"


using namespace std;

/**
 Class representing a single ray.
 */
class Ray{
public:
    glm::vec3 origin; ///< Origin of the ray
    glm::vec3 direction; ///< Direction of the ray
	/**
	 Constructor of the ray
	 @param origin Origin of the ray
	 @param direction Direction of the ray
	 */
    Ray(glm::vec3 origin, glm::vec3 direction) : origin(origin), direction(direction){
    }
};

/**
 * Generates a 4x4 transformation matrix given Translation (t*), Rotation (r*), Scaling (s*) parameters.
 * @return
 */
glm::mat4 genTRMat(glm::vec3 t, // Translation
                   glm::vec3 r, // Rotation
                   glm::vec3 s  // Scaling
) {
    glm::mat4 trans = glm::translate(glm::mat4(1.0f),t);
    glm::mat4 rotat = glm::rotate(trans,glm::radians(r.x), glm::vec3(1,0,0));
    rotat = glm::rotate(rotat,glm::radians(r.y), glm::vec3(0,1,0));
    rotat = glm::rotate(rotat,glm::radians(r.z), glm::vec3(0,0,1));
    glm::mat4 scale = glm::scale(rotat, s);
    return scale;
}


class Object;

/**
 Structure representing the even of hitting an object
 */
struct Hit{
    bool hit; ///< Boolean indicating whether there was or there was no intersection with an object
    glm::vec3 normal; ///< Normal vector of the intersected object at the intersection point
    glm::vec3 intersection; ///< Point of Intersection
    float distance; ///< Distance from the origin of the ray to the intersection point
    Object *object; ///< A pointer to the intersected object
	glm::vec2 uv; ///< Coordinates for computing the texture (texture coordinates)
};

/**
 General class for the object
 */
class Object{
	
protected:
	glm::mat4 transformationMatrix = glm::mat4(1.0f); ///< Matrix representing the transformation from the local to the global coordinate system
	glm::mat4 inverseTransformationMatrix = glm::mat4(1.0f); ///< Matrix representing the transformation from the global to the local coordinate system
	glm::mat4 normalMatrix = glm::mat4(1.0f); ///< Matrix for transforming normal vectors from the local to the global coordinate system
	
public:
	glm::vec3 color; ///< Color of the object
	Material material; ///< Structure describing the material of the object
	/** A function computing an intersection, which returns the structure Hit */
    virtual Hit intersect(Ray ray) = 0;

	/** Function that returns the material struct of the object*/
	Material getMaterial(){
		return material;
	}
	/** Function that set the material
	 @param material A structure describing the material of the object
	*/
	void setMaterial(Material material){
		this->material = material;
	}

	/** Functions for setting up all the transformation matrices
	 @param matrix The matrix representing the transformation of the object in the global coordinates */
	void setTransformation(glm::mat4 matrix){
		transformationMatrix = matrix;
		inverseTransformationMatrix = glm::inverse(transformationMatrix);
		normalMatrix = glm::transpose(inverseTransformationMatrix);
	}

    Ray toLocalRay(Ray ray) {
        ray.origin = glm::vec3(inverseTransformationMatrix * glm::vec4(ray.origin, 1.0f ));
        ray.direction = glm::normalize(glm::vec3(inverseTransformationMatrix * glm::vec4(ray.direction, 0.0f)));
        return ray;
    }

    Hit toGlobalHit(Hit localHit, Ray ray) {
        if(!localHit.hit) { return localHit; }

		Hit globalHit = localHit;
        globalHit.normal = glm::normalize(glm::vec3(normalMatrix * glm::vec4(globalHit.normal, 0.0f)));
        globalHit.intersection = glm::vec3(transformationMatrix * glm::vec4(globalHit.intersection, 1.0f));

        globalHit.distance = glm::length(globalHit.intersection - ray.origin);

        return globalHit;
    }



};

/**
 Implementation of the class Object for sphere shape.
 */
class Sphere : public Object{
private:
    const float radius = 1; ///< Local radius of the sphere
    const glm::vec3 center = glm::vec3(0.0f); ///< Local center of the sphere

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
		hit.hit = false; // by default, there is no intersection

        Ray localRay = toLocalRay(ray);

        const glm::vec3 c = center - localRay.origin; // center of the sphere in the coordinate system of the ray

		const float a = glm::dot(c, localRay.direction);

		const float D_squared = glm::dot(c, c) - a*a;
		const float D = sqrt(D_squared); // distance from the center of the sphere to the ray

		float closest_t;
		// No intersection
		if(D > radius) {
			return hit;

		// Two intersections
		} else if (D < radius) {
			// Distance of first point (always the closest)
			const float t1 = a - sqrt(radius*radius - D_squared);
			// float t2 = a + sqrt(radius*radius - D_squared);

			// We only get the smallest positive between the two points
			closest_t = t1 >= 0 ? t1 : a + sqrt(radius*radius - D_squared);

		// One intersection
		} else {
			closest_t = a + sqrt(radius*radius - D_squared);
		}

		// We ignore intersection points behind the camera
		if(closest_t < 0) {
			return hit;
		}

		hit.hit = true;
		hit.distance = closest_t;
		hit.intersection = localRay.origin + localRay.direction * hit.distance;
		hit.normal = glm::normalize(hit.intersection - center);
		hit.object = this;

		// Texture mapping
		hit.uv.x = 0.5 - asin(hit.normal.y) / M_PI;
		hit.uv.y = 2*(0.5 + atan2(hit.normal.z, hit.normal.x) / (2 * M_PI));

        // remap the normals to the normal map
        if(material.hasNormalMap()) {


            // // convert intersection point to sphere coordinates
            // float theta = acos(hit.normal.y);
            // float phi = atan2(hit.normal.z, hit.normal.x);
            // float x = sin(theta) * cos(phi);
            // float y = sin(theta);
            // float z = cos(theta) * sin(phi);

            // glm::vec3 tangent = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(x, y, z)));
            // glm::vec3 bitangent = glm::normalize(glm::cross(hit.normal, tangent));


            glm::vec3 tangent = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), hit.intersection));
            glm::vec3 bitangent = glm::normalize(glm::cross(hit.normal, tangent));

            glm::vec3 normal_map = glm::normalize(material.getNormal(hit.uv));

            glm::mat3 TBN = glm::mat3(tangent, bitangent, hit.normal);

            hit.normal = glm::normalize(TBN * normal_map);
        }

		return toGlobalHit(hit, ray);
    }
};


class Plane : public Object{

private:
	glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f); ///< Local normal of the plane
	glm::vec3 point = glm::vec3(0.0f); ///< Local point on the plane

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
	Hit intersect(Ray ray){

		Hit hit;
		hit.hit = false;

        // Transform ray into local coordinate system
        Ray localRay = toLocalRay(ray);

		float t = glm::dot(point - localRay.origin, normal) / glm::dot(localRay.direction, normal);

		if (t > 0) {
			hit.hit = true;
			hit.intersection = localRay.origin + t * localRay.direction;
			hit.normal = glm::normalize(normal);
			hit.distance = glm::distance(localRay.origin, hit.intersection);
			hit.object = this;
		}

        // Texture mapping
        hit.uv.x = 0.1*hit.intersection.x;
        hit.uv.y = 0.1*hit.intersection.z;

        // remap the normals to the normal map
        if(material.hasNormalMap()) {
            glm::vec3 tangent = glm::vec3(0, 0, 1);
            glm::vec3 bitangent = glm::vec3(1, 0, 0);

            glm::vec3 normal_map = glm::normalize(material.getNormal(hit.uv));

            glm::mat3 TBN = glm::mat3(tangent, bitangent, normal);

            hit.normal = glm::normalize(TBN * normal_map);
        }

		return toGlobalHit(hit, ray);
	}
};

class Cone : public Object{

private:
    const float height = 1.0; ///< Local height of the cone
    const glm::vec3 H = glm::vec3(0.0f, height, 0.0f); ///< Local height of the cone
    const glm::vec3 O = glm::vec3(0.0f); ///< origin of the cone
    const float radius = 1.0; ///< Local radius of the cone

public:
	Cone(Material material){
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
        Plane base = Plane(material, genTRMat(glm::vec3(0),glm::vec3(180.0, 0, 0),glm::vec3(0.5f)));
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
        hit.object = this;

        // Texture mapping
        hit.uv.x = 0.5 - asin(hit.normal.y) / M_PI;
        hit.uv.y = 2*(0.5 + atan2(hit.normal.z, hit.normal.x) / (2 * M_PI));

        return toGlobalHit(hit, ray);
	}
};


/**
 Light class
 */
class Light{
public:
	glm::vec3 position; ///< Position of the light source
	glm::vec3 color; ///< Color/intensity of the light source
	Light(glm::vec3 position): position(position){
		color = glm::vec3(1.0);
	}
	Light(glm::vec3 position, glm::vec3 color): position(position), color(color){
	}
};

struct Scene {
    vector<Light *> lights; 
    glm::vec3 ambient_light;
    vector<Object *> objects;
};
vector<Light *> lights; ///< A list of lights in the scene
glm::vec3 ambient_light;
vector<Object *> objects; ///< A list of all objects in the scene

/**
 * Given a Ray returns the closest Hit structure.
 * @param ray the ray to intersect with the Objects of the scene
 * @return
 */
Hit closest_intersection(Ray ray) {
    // hit structure representing the closest intersection
    Hit closest_hit;

    closest_hit.hit = false;
    closest_hit.distance = INFINITY;

    //Loop over all objects to find the closest intersection
    for (Object *object : objects) {
        Hit hit = object->intersect(ray);
        if (hit.hit && hit.distance < closest_hit.distance) {
            closest_hit = hit;
        }
    }

    return closest_hit;
}

/**
 * @brief Returns true if there is an occluding object before a given distance from the start point along the direction.
 * 
 * @param point the start point of the ray/
 * @param direction the direction of the ray.
 * @param distance the distance to check for occlusion.
 * @return true 
 * @return false 
 */
bool is_shadowed(glm::vec3 point, glm::vec3 normal, glm::vec3 direction, const float distance) {
    // avoid sending shadow rays towards lights behind the surface
    if (glm::dot(normal, direction) < 0) {
        return true;
    }

    // origin of the shadow ray is moved a little to avoid self intersection
    Ray shadowRay = Ray(point + 0.001f * direction, direction);
    for (Object *object : objects) {
        Hit hit = object->intersect(shadowRay);
        if (hit.hit && hit.distance <= distance) {
            return true;
        }
    }
    return false;
}

/** Function for computing color of an object according to the Phong Model and reflection/refraction.
 @param point A point belonging to the object for which the color is computed
 @param normal A normal vector the the point
 @param uv Texture coordinates
 @param view_direction A normalized direction from the point to the viewer/camera
 @param material A material structure representing the material of the object
 @param maxBounces compute reflection rays for a maximum amount of levels. Limits recursive calls.
*/
glm::vec3 IlluminationModel(glm::vec3 point, glm::vec3 normal, glm::vec2 uv, glm::vec3 view_direction, Material material, const int maxBounces){

	glm::vec3 color(0.0);


    // if (material.hasNormalMap()) {
    //     // compute tangent and bitangent vectors
    //     glm::vec3 tangent, bitangent;

    //     // // compute tangent vector
    //     // tangent = glm::normalize(glm::cross(glm::vec3(0, -1, 0), normal));
    //     // // compute bitangent vector
    //     // bitangent = glm::normalize(glm::cross(normal, tangent));
    //     tangent = glm::normalize(glm::cross(normal, view_direction));
    //     // if (abs(normal.x) > abs(normal.y)) {
    //     //     tangent = glm::vec3(normal.z, 0, -normal.x) / sqrt(normal.x * normal.x + normal.z * normal.z);
    //     // } else {
    //     //     tangent = glm::vec3(0, -normal.z, normal.y) / sqrt(normal.y * normal.y + normal.z * normal.z);
    //     // }
    //     bitangent = glm::normalize(glm::cross(normal, tangent));

    //     // compute TBN matrix
    //     glm::mat3 TBN = glm::mat3(tangent, bitangent, normal);

    //     // get the normal from the normal map
    //     glm::vec3 normal_map = material.getNormal(uv);

    //     // convert the normal_map from tangent space to local space
    //     normal = glm::normalize(TBN * normal_map);
    // }

    // normal = glm::vec3(0.0);



    for(Light* light : lights) {
        glm::vec3 l = glm::normalize(light->position - point); // direction from the point to the light source

        // Shadows
        const float distance_from_light = glm::distance(point, light->position);
        // If the shadow ray hits an object, the point is in shadow and we don/t compute ambient/diffuse.
        if (!is_shadowed(point, normal, l, distance_from_light)) {
            // If there is a texture, takes the diffuse color of the texture instead.
            glm::vec3 diffuse_color = material.getDiffuse(uv);

            const float diffuse = max(0.0f, glm::dot(l, normal));

            glm::vec3 h = glm::normalize(l + view_direction); // half vector
            const float specular = max(0.0f, glm::pow(glm::dot(h,normal), 4*material.getShininess(uv)));

            // Attenuation
            const float distance = max(0.1f, distance_from_light);
            const float attenuation = 1/ glm::pow(distance, 2);
            //
            color += attenuation * light->color * (diffuse_color*diffuse + material.getSpecular()*specular);
        }
    }

    // Limit reflection and refraction bounces to avoid infinite recursion.
    if (maxBounces > 0) {
        
        // Reflection
        glm::vec3 reflection(0.0f);
        if (material.getReflectivity() > 0) {
            color *= 1 - material.getReflectivity();
            glm::vec3 reflection_direction = glm::reflect(-view_direction, normal);
            Ray reflection_ray = Ray(point + 0.001f * reflection_direction, reflection_direction);
            Hit closest_hit = closest_intersection(reflection_ray);
            if (closest_hit.hit) {
                reflection = material.getReflectivity()*IlluminationModel(closest_hit.intersection, closest_hit.normal, closest_hit.uv, glm::normalize(-reflection_direction), closest_hit.object->getMaterial(), maxBounces-1);
            }
        }

        // Refraction
        glm::vec3 refraction(0.0f);
        if (material.getRefractivity() > 0) {
            color *= (1 - material.getRefractivity());
            const bool is_entering = glm::dot(normal, -view_direction) < 0.0f;
            /* TODO:    We assume that one of the two materials is air, whether we are entering or exiting.
                        How would we compute a glass sphere submerged in water ? */
            const float n1 = is_entering ? 1.0f : material.getRefractionIndex();
            const float n2 = is_entering ? material.getRefractionIndex() : 1.0f;
            const float eta = n1/n2;
            glm::vec3 refraction_direction = glm::refract(-view_direction, is_entering ? normal : -normal, eta);
            Ray refraction_ray = Ray(point + 0.001f * refraction_direction, refraction_direction);

            Hit closest_hit = closest_intersection(refraction_ray);
            if (closest_hit.hit) {
                refraction = material.getRefractivity()*IlluminationModel(closest_hit.intersection, closest_hit.normal, closest_hit.uv, glm::normalize(-refraction_direction), closest_hit.object->getMaterial(), maxBounces-1);
                // Fresnel effect
                float O1 = cos(glm::angle(normal, view_direction));
                float O2 = cos(glm::angle(-normal, refraction_direction));

                float R = 0.5*(pow((n1*O1 - n2*O2)/(n1*O1 + n2*O2), 2) + pow((n1*O2 - n2*O1)/(n1*O2 + n2*O1), 2));
                float T = 1-R;

                reflection *= R;
                refraction *= T;
            }
        }
        color += reflection + refraction;
    }   

    color += ambient_light*material.getAmbient(uv);

    // This is no more the final color since we recur call IlluminationModel for reflection and refraction.
    // Therefore, the color should not be clamped to 0 and 1.
    // The final color is clamped only for display. See the trace_ray function.
	return color; 
}


/**
 Functions that computes a color along the ray
 @param ray Ray that should be traced through the scene
 @return Color at the intersection point
 */
glm::vec3 trace_ray(Ray ray) {

    Hit closest_hit = closest_intersection(ray);

	glm::vec3 color(0.0);

	if (closest_hit.hit) {
        color = IlluminationModel(closest_hit.intersection, closest_hit.normal, closest_hit.uv, glm::normalize(-ray.direction), closest_hit.object->getMaterial(), 5);
	}
    // clamp the final color to [0,1]
	return  glm::clamp(color, glm::vec3(0.0), glm::vec3(1.0));
}

/**
 Function defining the default scene;
 */
struct Scene sceneDefinition() {
    vector<Light *> lights;
    glm::vec3 ambient_light(0.02f);
    vector<Object *> objects;

    // Materials -------------------------------------------------------------------------------------------------------
    Material blue;
    blue.setAmbient(glm::vec3(0.07f, 0.07f, 0.1f));
    blue.setDiffuse(glm::vec3(0.7f, 0.7f, 1.0f));
    blue.setSpecular(glm::vec3(0.6));
    blue.setRoughness(0.0);
    blue.setReflectivity(1.0f);

    // blue.ambient = glm::vec3(0.07f, 0.07f, 0.1f);
    // blue.diffuse = glm::vec3(0.7f, 0.7f, 1.0f);
    // blue.specular = glm::vec3(0.6);
    // blue.shininess = 100.0;
    // blue.reflectivity = 1.0f;

    Material red;
    red.setAmbient(glm::vec3(0.01f, 0.03f, 0.03f));
    red.setDiffuse(glm::vec3(1.0f, 0.3f, 0.3f));
    red.setSpecular(glm::vec3(0.5));
    red.setRoughness(10.0);

    // red.ambient = glm::vec3(0.01f, 0.03f, 0.03f);
    // red.diffuse = glm::vec3(1.0f, 0.3f, 0.3f);
    // red.specular = glm::vec3(0.5);
    // red.shininess = 10.0;

    Material green;
    green.setAmbient(glm::vec3(0.07f, 0.09f, 0.07f));
    green.setDiffuse(glm::vec3(0.7f, 1.0f, 0.7f));
    green.setSpecular(glm::vec3(0.0));
    green.setRoughness(100.0);

    // green.ambient = glm::vec3(0.07f, 0.09f, 0.07f);
    // green.diffuse = glm::vec3(0.7f, 1.0f, 0.7f);
    // green.specular = glm::vec3(0.0);
    // green.shininess = 0.0;

    Material yellow;
    yellow.setAmbient(glm::vec3(0.07f, 0.07f, 0.07f));
    yellow.setDiffuse(glm::vec3(1.0f, 1.0f, 0.0f));
    yellow.setSpecular(glm::vec3(1.0));
    yellow.setRoughness(0.0);
    yellow.setReflectivity(0.5f);

    // yellow.ambient = glm::vec3(0.07f, 0.07f, 0.07f);
    // yellow.diffuse = glm::vec3(1.0f, 1.0f, 0.0f);
    // yellow.specular = glm::vec3(1.0);
    // yellow.shininess = 100.0;
    // yellow.reflectivity = 0.5f;

	Material white;
    white.setAmbient(glm::vec3(0.1f, 0.1f, 0.1f));
    white.setDiffuse(glm::vec3(0.9f, 0.9f, 0.9f));
    white.setSpecular(glm::vec3(0.5));
    white.setRoughness(10.0);

	// white.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
	// white.diffuse = glm::vec3(0.9f, 0.9f, 0.9f);
	// white.specular = glm::vec3(0.5);
	// white.shininess = 10.0;

    Material dark_blue;
    dark_blue.setAmbient(glm::vec3(0.07f, 0.07f, 0.1f));
    dark_blue.setDiffuse(glm::vec3(0.0f, 0.0f, 0.5f));
    dark_blue.setSpecular(glm::vec3(0.0));

    // dark_blue.ambient = glm::vec3(0.07f, 0.07f, 0.1f);
    // dark_blue.diffuse = glm::vec3(0.0f, 0.0f, 0.5f);
    // dark_blue.specular = glm::vec3(0.0);

    // Procedural textures
    Material checkerboard;
    // checkerboard.setDiffuse(checkerboardTexture);
    checkerboard.setDiffuse(glm::vec3(1.0f));
    // checkerboard.setAmbient("src/Textures/fur_ambientOcclusion.jpg");
    // checkerboard.setRoughness("src/Textur÷es/fur_roughness.jpg");
    checkerboard.setNormal("src/Textures/fur_normal.jpg");
    // checkerboard.setSpecular(glm::vec3(1.0));
    // checkerboard.setShininess(0.0);
    // checkerboard.setReflectivity(0.4f);

    // checkerboard.texture = &checkerboardTexture;
	// checkerboard.ambient = glm::vec3(0.0f);
	// checkerboard.specular = glm::vec3(1.0);
	// checkerboard.shininess = 100.0;
    // checkerboard.reflectivity = 0.4f;


	Material rainbow;
    rainbow.setDiffuse(rainbowTexture);
    rainbow.setAmbient(glm::vec3(1.0f));
    rainbow.setSpecular(glm::vec3(1.0));
    rainbow.setRoughness(0.1f);

	// rainbow.texture = &rainbowTexture;
	// rainbow.ambient = glm::vec3(0.0f);
	// rainbow.specular = glm::vec3(1.0);
	// rainbow.shininess = 10.0;

    Material refractive;
    refractive.setSpecular(glm::vec3(1.0));
    refractive.setRoughness(10.0);
    refractive.setReflectivity(1.0f);
    refractive.setRefractivity(1.0f);
    refractive.setRefractionIndex(2.0f);
    refractive.setNormal("src/Textures/fur_normal.jpg");

    // refractive.specular = glm::vec3(1.0);
    // refractive.shininess = 10.0;
    // refractive.reflectivity = 1.0f;
    // refractive.refractivity = 1.0f;
    // refractive.refraction_index = 2.0f;


    // SPHERES ---------------------------------------------------------------------------------------------------------
    glm::mat4 sp1_t = genTRMat(glm::vec3(1.0, -2.0, 8.0),glm::vec3(0.0),glm::vec3(1.0f));
    Sphere *sp1 = new Sphere(blue,sp1_t);

    glm::mat4 sp2_t = genTRMat(glm::vec3(-1.0, -2.5, 6.0),glm::vec3(0.0),glm::vec3(0.5f));
    Sphere *sp2 = new Sphere(red,sp2_t);

    // glm::mat4 sp3_t = genTRMat(glm::vec3(3.0, -2.0, 6.0),glm::vec3(0.0),glm::vec3(1.0f));
    // Sphere *sp3 = new Sphere(green,sp3_t);

    objects.push_back(sp1);
    objects.push_back(sp2);
    // objects.push_back(sp3);

    // textured spheres

    glm::mat4 sp4_t = genTRMat(glm::vec3(-6,4,23),glm::vec3(30.0),glm::vec3(7.0f));
    Sphere *sp4 = new Sphere(checkerboard, sp4_t);
    glm::mat4 sp5_t = genTRMat(glm::vec3(-6,0.0,16),glm::vec3(0.0),glm::vec3(3.0f));
    Sphere *sp5 = new Sphere(rainbow, sp5_t);

    objects.push_back(sp4);
    objects.push_back(sp5);
//
//    glm::mat4 sp6_t = genTRMat(glm::vec3(0.0, 27.0, 0.0),glm::vec3(0.0),glm::vec3(1.0f));
//    Plane *sp6 = new Plane(blue,sp6_t);
//    objects.push_back(sp6);

    glm::mat4 sp7_t = genTRMat(glm::vec3(-3.0, -1.0, 8.0),glm::vec3(0.0),glm::vec3(2.0f));
    Sphere *sp7 = new Sphere(refractive,sp7_t);
    objects.push_back(sp7);

    // PLANES ----------------------------------------------------------------------------------------------------------
    // y
    glm::mat4 p1_t = genTRMat(glm::vec3(0.0, -3.0, 0.0),glm::vec3(0.0),glm::vec3(5.0f));
    Plane *p1 = new Plane(checkerboard,p1_t);
    objects.push_back(p1);

    glm::mat4 p2_t = genTRMat(glm::vec3(0.0, 27.0, 0.0),glm::vec3(180.0, 0, 0),glm::vec3(1.0f));
    Plane *p2 = new Plane(dark_blue,p2_t);
    objects.push_back(p2);

    // x
    glm::mat4 p3_t = genTRMat(glm::vec3(-15.0, 0.0, 0.0),glm::vec3(0.0, 0, -90),glm::vec3(1.0f));
    Plane *p3 = new Plane(blue,p3_t);
    objects.push_back(p3);

    glm::mat4 p4_t = genTRMat(glm::vec3(15.0, 0.0, 0.0),glm::vec3(0.0, 0, 90),glm::vec3(1.0f));
    Plane *p4 = new Plane(red,p4_t);
    objects.push_back(p4);

    // z
    glm::mat4 p5_t = genTRMat(glm::vec3(0.0, 0.0, -0.01),glm::vec3(90.0, 0, 0),glm::vec3(1.0f));
    Plane *p5 = new Plane(green,p5_t);
    objects.push_back(p5);

    glm::mat4 p6_t = genTRMat(glm::vec3(0.0, 0.0, 30.0),glm::vec3(-90.0, 0, 0),glm::vec3(1.0f));
    Plane *p6 = new Plane(rainbow,p6_t);
    objects.push_back(p6);


    // CONES -----------------------------------------------------------------------------------------------------------
    glm::mat4 cone1_t = genTRMat(glm::vec3(5.0, -3.0, 14.0),glm::vec3(0.0),glm::vec3(3.0f, 12.0f, 3.0f));
    Cone *cone1 = new Cone(yellow,cone1_t);
    objects.push_back(cone1);

    glm::mat4 cone2_t = genTRMat(glm::vec3(3.0, -2.0, 9.0),glm::vec3(0.0, 0.0, -110.0),glm::vec3(1.0f, 3.0f, 1.0f));
    Cone *cone2 = new Cone(checkerboard,cone2_t);
    objects.push_back(cone2);

    // lights
    lights.push_back(new Light(glm::vec3(0.0, 26.0, 5.0), glm::vec3(4.0)));
    lights.push_back(new Light(glm::vec3(1.0, 3.0, 12.0), glm::vec3(2.0)));
    lights.push_back(new Light(glm::vec3(0.0, 5.0, 2.0), glm::vec3(1.0)));
//    lights.push_back(new Light(glm::vec3(0.0, 20.0, 16.0), glm::vec3(1.0)));

    struct Scene newscene;
    newscene.objects = objects;
    newscene.lights = lights;
    newscene.ambient_light = ambient_light;
    return newscene;
}


/**
 Function performing tonemapping of the intensities computed using the raytracer
 @param intensity Input intensity.
 @return Tonemapped intensity in range (0,1)
 */
glm::vec3 toneMapping(const glm::vec3 &intensity){
	// Tonemapping parameters
	const float alpha = 5.5f;
	const float beta = 1.3f;
	const float gamma = 1.8f;

	const glm::vec3 tonemapped = alpha * glm::pow(glm::pow(intensity, glm::vec3(beta)), glm::vec3(1.0/gamma));
	return glm::clamp(tonemapped, glm::vec3(0.0), glm::vec3(1.0));
}


/**
 * Renders a Scene and outputs the result into a file.
 * Uses a thread pool to compute individual pixels concurrently.
 * 
 * @param renderScene the Scene to render.
 * @param filename the output file filename.
 * @param threads the amount of system threads to use. Defaults to the maximum available.
 * @return int the exit status.
 */
int render(const struct Scene &renderScene, string filename) {

    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

    cout << "Running on " << omp_get_max_threads() << " threads\n";

    int width = 1920; //width of the image
    int height = 1440; // height of the image

    float fov = 90; // field of view

    lights = renderScene.lights;
    objects = renderScene.objects;
    ambient_light = renderScene.ambient_light;

    Image image(width,height); // Create an image where we will store the result

    const float s = 2*tan(0.5*fov/180*M_PI)/width; // size of one pixel
    const float X = -(width * s) / 2; // X coordinate of the first pixel
    const float Y = (height * s) / 2; // Y coordinate of the first pixel
    const float Z = 1;
    glm::vec3 origin = glm::vec3(0.0, 0.0, 0.0); // origin of the camera

    // Define tiles for parallelization
    // Tiles are a good way to parallelize ray tracing since we can expect that rays in the same tile will behave similarly (i.e. they will hit the same objects).
    const int tile_size = 16;
    const int tiles_x = (width + tile_size - 1) / tile_size; // add one tile if width is not a multiple of tile_size
    const int tiles_y = (height + tile_size - 1) / tile_size; // add one tile if height is not a multiple of tile_size
    const int tile_count = tiles_x * tiles_y;

    // Loop over all tiles and traverse the rays through the scene
    #pragma omp parallel for schedule(dynamic, 1)
    for (int tile = 0; tile < tile_count; tile++) {
        // print thread 1 progress
        if (omp_get_thread_num() == 0) {
            cout << "Progress: " << ceil((float)tile / tile_count * 10000) / 100 << "%\r";
            cout.flush();
        }

        // Compute the tile coordinates
        const int tile_j = tile / tiles_x; // the tile column number
        const int tile_i = tile - tile_j * tiles_x; // the tile row number
        const int tile_i_start = tile_i * tile_size; // the x coordinate of the tile 
        const int tile_j_start = tile_j * tile_size; // the y coordinate of the tile
        const int tile_i_end = min(tile_i_start + tile_size, width); // the x coordinate of the tile + tile_size
        const int tile_j_end = min(tile_j_start + tile_size, height); // the y coordinate of the tile + tile_size

        // Loop over all pixels in the tile
        for (int i = tile_i_start; i < tile_i_end ; i++) {
            for (int j = tile_j_start; j < tile_j_end; j++) {
                // X coordinate of the current pixel
                const float dx = X + i*s + 0.5*s;
                // Y coordinate of the current pixel
                const float dy = Y - j*s - 0.5*s;

                // direction of the ray
                glm::vec3 direction = glm::normalize(glm::vec3(dx, dy, Z));
                // create the ray
                Ray ray(origin, direction);
                // trace the ray and set the color of the pixel
                image.setPixel(i, j, toneMapping(trace_ray(ray)));
            }
        }
    }

    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
    chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(end - start);

    cout<<"It took " << time_span.count() << " seconds to render the image."<< endl;
    // cout<<"I could render at "<< (float)CLOCKS_PER_SEC/((float)t) << " frames per second."<<endl;

    // Writing the final results of the rendering
    cout << "Saving render to file '"<< filename << "'\n";
    if (!filename.empty()) {
        image.writeImage(filename.c_str());
    } else {
        image.writeImage("./render/result.ppm");
    }

    return 0;
}