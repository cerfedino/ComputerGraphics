/**
@file main.cpp
*/

#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include <vector>
#include <algorithm>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/string_cast.hpp"
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
	 Contructor of the ray
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
	Hit intersect(Ray ray){
		
		Hit hit;
		hit.hit = false;

		// Transform ray into local coordinate system
        Ray localRay = toLocalRay(ray);

        float tan = (radius / height) * (radius / height);

        float a = (localRay.direction.x * localRay.direction.x) +
                  (localRay.direction.z * localRay.direction.z) -
                  (tan*(localRay.direction.y * localRay.direction.y));
        float b = (2*localRay.origin.x*localRay.direction.x) +
                  (2*localRay.origin.z*localRay.direction.z) +
                  (2*tan*(height-localRay.origin.y)*localRay.direction.y);
        float c = (localRay.origin.x*localRay.origin.x) +
                  (localRay.origin.z*localRay.origin.z) -
                  (tan*((height-localRay.origin.y)*(height-localRay.origin.y)));

        float delta = b*b - 4*a*c;
        if (delta < 0) {
            return hit;
        }

        float t1 = (-b - sqrt(delta)) / (2*a);
        float t2 = (-b + sqrt(delta)) / (2*a);

        float closest_t;
        if (t1 < 0 && t2 < 0) {
            return hit;
        } else if (t1 < 0) {
            closest_t = t2;
        } else if (t2 < 0) {
            closest_t = t1;
        } else {
            closest_t = t1 < t2 ? t1 : t2;
        }
        hit.distance = closest_t;
        hit.intersection = localRay.origin + localRay.direction * hit.distance;

        // create a plain at the base of the cone
        Plane base = Plane(material, genTRMat(glm::vec3(0),glm::vec3(180.0, 0, 0),glm::vec3(0.5f)));
        Hit baseHit = base.intersect(localRay);

        if (baseHit.hit && baseHit.distance < closest_t || (hit.intersection.y < 0 || hit.intersection.y > height)) {
            float distance = glm::distance(baseHit.intersection, O);
            if (distance <= radius) {
                return toGlobalHit(baseHit, ray);
            }
        }

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

/** Function for computing color of an object according to the Phong Model
 @param point A point belonging to the object for which the color is computed
 @param normal A normal vector the the point
 @param uv Texture coordinates
 @param view_direction A normalized direction from the point to the viewer/camera
 @param material A material structure representing the material of the object
*/
glm::vec3 PhongModel(glm::vec3 point, glm::vec3 normal, glm::vec2 uv, glm::vec3 view_direction, Material material){

	glm::vec3 color(0.0);

    for(Light* light : lights) {
        glm::vec3 l = glm::normalize(light->position - point); // direction from the point to the light source

		// If there is a texture, takes the diffuse color of the texture instead.
		glm::vec3 diffuse_color = material.texture != NULL ? material.texture(uv) : material.diffuse;

        const float diffuse = max(0.0f, glm::dot(l, normal));

        glm::vec3 h = glm::normalize(l + view_direction); // half vector
        const float specular = max(0.0f, glm::pow(glm::dot(h,normal), 4*material.shininess));
		
		// Attenuation
		const float distance = max(0.1f, glm::distance(point, light->position));
		const float attenuation = 1/ glm::pow(distance, 2);
		//
        color += attenuation * light->color * (diffuse_color*diffuse + material.specular*specular);
    }

    color += ambient_light*material.ambient;

	// The final color has to be clamped so the values do not go beyond 0 and 1.
	return glm::clamp(color, glm::vec3(0.0), glm::vec3(1.0));
}


/**
 Functions that computes a color along the ray
 @param ray Ray that should be traced through the scene
 @return Color at the intersection point
 */
glm::vec3 trace_ray(Ray ray){

	// hit structure representing the closest intersection
	Hit closest_hit;

	closest_hit.hit = false;
	closest_hit.distance = INFINITY;

	//Loop over all objects to find the closest intersection
	for (int k = 0; k<objects.size(); k++) {
		Hit hit = objects[k]->intersect(ray);
		if (hit.hit == true && hit.distance < closest_hit.distance)
			closest_hit = hit;
	}

	glm::vec3 color(0.0);

	if (closest_hit.hit) {
        color = PhongModel(closest_hit.intersection, closest_hit.normal, closest_hit.uv, glm::normalize(-ray.direction), closest_hit.object->getMaterial());
	} else {
		color = glm::vec3(0.0, 0.0, 0.0);
	}
	return color;
}
/**
 Function defining the default scene;
 */
struct Scene sceneDefinition() {
    vector<Light *> lights;
    glm::vec3 ambient_light(1.7f);
    vector<Object *> objects;

    // Materials -------------------------------------------------------------------------------------------------------
    Material blue;
    blue.ambient = glm::vec3(0.07f, 0.07f, 0.1f);
    blue.diffuse = glm::vec3(0.7f, 0.7f, 1.0f);
    blue.specular = glm::vec3(0.6);
    blue.shininess = 100.0;

    Material red;
    red.ambient = glm::vec3(0.01f, 0.03f, 0.03f);
    red.diffuse = glm::vec3(1.0f, 0.3f, 0.3f);
    red.specular = glm::vec3(0.5);
    red.shininess = 10.0;

    Material green;
    green.ambient = glm::vec3(0.07f, 0.09f, 0.07f);
    green.diffuse = glm::vec3(0.7f, 1.0f, 0.7f);
    green.specular = glm::vec3(0.0);
    green.shininess = 0.0;

    Material yellow;
    yellow.ambient = glm::vec3(0.07f, 0.07f, 0.07f);
    yellow.diffuse = glm::vec3(1.0f, 1.0f, 0.0f);
    yellow.specular = glm::vec3(1.0);
    yellow.shininess = 100.0;

	Material white;
	white.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
	white.diffuse = glm::vec3(0.9f, 0.9f, 0.9f);
	white.specular = glm::vec3(0.5);
	white.shininess = 10.0;

    // Procedural textures
    Material checkerboard;
    checkerboard.texture = &checkerboardTexture;
	checkerboard.ambient = glm::vec3(0.0f);
	checkerboard.specular = glm::vec3(0.0);
	checkerboard.shininess = 0.0;

	Material rainbow;
	rainbow.texture = &rainbowTexture;
	rainbow.ambient = glm::vec3(0.0f);
	rainbow.specular = glm::vec3(1.0);
	rainbow.shininess = 10.0;


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
//    objects.push_back(new Sphere(7.0, glm::vec3(-6,4,23), checkerboard));
//    objects.push_back(new Sphere(5.0, glm::vec3(7,3,23), rainbow));

    glm::mat4 sp4_t = genTRMat(glm::vec3(-6,4,23),glm::vec3(0.0),glm::vec3(7.0f));
    Sphere *sp4 = new Sphere(checkerboard, sp4_t);
    glm::mat4 sp5_t = genTRMat(glm::vec3(-6,0.0,16),glm::vec3(0.0),glm::vec3(3.0f));
    Sphere *sp5 = new Sphere(rainbow, sp5_t);

    objects.push_back(sp4);
    objects.push_back(sp5);

    glm::mat4 sp6_t = genTRMat(glm::vec3(0.0, 27.0, 0.0),glm::vec3(0.0),glm::vec3(1.0f));
    Plane *sp6 = new Plane(blue,sp6_t);
    objects.push_back(sp6);

    // PLANES ----------------------------------------------------------------------------------------------------------
    // y
    glm::mat4 p1_t = genTRMat(glm::vec3(0.0, -3.0, 0.0),glm::vec3(0.0),glm::vec3(5.0f));
    Plane *p1 = new Plane(green,p1_t);
    objects.push_back(p1);

    glm::mat4 p2_t = genTRMat(glm::vec3(0.0, 27.0, 0.0),glm::vec3(180.0),glm::vec3(1.0f));
    Plane *p2 = new Plane(blue,p2_t);
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
    lights.push_back(new Light(glm::vec3(0.0, 26.0, 5.0), glm::vec3(50.0)));
    lights.push_back(new Light(glm::vec3(0.0, 3.0, 12.0), glm::vec3(30.0)));
    lights.push_back(new Light(glm::vec3(0.0, 5.0, 1.0), glm::vec3(35.0)));

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
glm::vec3 toneMapping(glm::vec3 intensity){
	// Tonemapping parameters
	const float alpha = 1.2f;
	const float beta = 1.8f;
	const float gamma = 1.8f;

	const glm::vec3 tonemapped = alpha * glm::pow(glm::pow(intensity, glm::vec3(beta)), glm::vec3(1.0/gamma));
	return glm::clamp(tonemapped, glm::vec3(0.0), glm::vec3(1.0));
}


/**
 * Renders a Scene and outputs the result into a file.
 * 
 * @param renderScene the Scene to render.
 * @param filename the output file filename.
 * @return int the exit status.
 */
int render(struct Scene renderScene, string filename) {
    clock_t t = clock(); // variable for keeping the time of the rendering

    int width = 960; //width of the image
    int height = 540; // height of the image
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

    // Loop over all pixels and traverse the rays through the scene
    for (int i = 0; i < width ; i++) {
        for (int j = 0; j < height ; j++) {

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
    t = clock() - t;
    cout<<"It took " << ((float)t)/CLOCKS_PER_SEC<< " seconds to render the image."<< endl;
    cout<<"I could render at "<< (float)CLOCKS_PER_SEC/((float)t) << " frames per second."<<endl;

    // Writing the final results of the rendering
    cout << "Saving render to file '"<< filename << "'\n";
    if (!filename.empty()) {
        image.writeImage(filename.c_str());
    } else {
        image.writeImage("./render/result.ppm");
    }

    return 0;
}