/**
@file main.cpp
*/

#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include <vector>
#include "glm/glm.hpp"

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
};

/**
 General class for the object
 */
class Object{
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
	 @param material A structure desribing the material of the object
	*/
	void setMaterial(Material material){
		this->material = material;
	}
};

/**
 Implementation of the class Object for sphere shape.
 */
class Sphere : public Object{
private:
    float radius; ///< Radius of the sphere
    glm::vec3 center; ///< Center of the sphere

public:
	/**
	 The constructor of the sphere
	 @param radius Radius of the sphere
	 @param center Center of the sphere
	 @param color Color of the sphere
	 */
    Sphere(float radius, glm::vec3 center, glm::vec3 color) : radius(radius), center(center){
		this->color = color;
    }
	Sphere(float radius, glm::vec3 center, Material material) : radius(radius), center(center){
		this->material = material;
	}
	/** Implementation of the intersection function*/
    Hit intersect(Ray ray) {

		Hit hit;
		hit.hit = false; // by default, there is no intersection

		const glm::vec3 c = center - ray.origin; // center of the sphere in the coordinate system of the ray

		const float a = glm::dot(c, ray.direction);

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
		hit.intersection = ray.origin + ray.direction * hit.distance;
		hit.normal = glm::normalize(hit.intersection - center);
		hit.object = this;

		return hit;
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

vector<Light *> lights; ///< A list of lights in the scene
glm::vec3 ambient_light(1.0,1.0,1.0);
vector<Object *> objects; ///< A list of all objects in the scene


/** Function for computing color of an object according to the Phong Model
 @param point A point belonging to the object for which the color is computed
 @param normal A normal vector the the point
 @param view_direction A normalized direction from the point to the viewer/camera
 @param material A material structure representing the material of the object
*/
glm::vec3 PhongModel(glm::vec3 point, glm::vec3 normal, glm::vec3 view_direction, Material material){

	glm::vec3 color(0.0);

    for(Light* light : lights) {
        glm::vec3 l = glm::normalize(light->position - point); // direction from the point to the light source
        float diffuse = glm::dot(l, normal);
        if (diffuse < 0) diffuse = 0;

        glm::vec3 h = glm::normalize(l + view_direction); // half vector
        float specular = glm::pow(glm::dot(h,normal), 4*material.shininess);
        if (specular < 0) specular = 0;

        color += light->color * (material.diffuse*diffuse + material.specular*specular);
    }
    color += ambient_light*material.ambient;

	// The final color has to be clamped so the values do not go beyond 0 and 1.
	color = glm::clamp(color, glm::vec3(0.0), glm::vec3(1.0));
	return color;
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
		color = PhongModel(closest_hit.intersection, closest_hit.normal, glm::normalize(-ray.direction), closest_hit.object->getMaterial());
	} else {
		color = glm::vec3(0.0, 0.0, 0.0);
	}
	return color;
}
/**
 Function defining the scene
 */
void sceneDefinition() {

    // materials
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
    green.diffuse = glm::vec3(0.7f, 0.9f, 0.7f);
    green.specular = glm::vec3(0.0);
    green.shininess = 0.0;


    // objects
    objects.push_back(new Sphere(1.0, glm::vec3(1.0, -2.0, 8.0), blue));
    objects.push_back(new Sphere(0.5, glm::vec3(-1.0, -2.5, 6.0), red));
    objects.push_back(new Sphere(1.0, glm::vec3(3.0, -2.0, 6.0), green));

    // lights
    lights.push_back(new Light(glm::vec3(0.0, 26.0, 5.0), glm::vec3(0.4)));
    lights.push_back(new Light(glm::vec3(0.0, 1.0, 12.0), glm::vec3(0.4)));
    lights.push_back(new Light(glm::vec3(0.0, 5.0, 1.0), glm::vec3(0.4)));
}

int main(int argc, const char * argv[]) {

    clock_t t = clock(); // variable for keeping the time of the rendering

    int width = 1024; //width of the image
    int height = 768; // height of the image
    float fov = 90; // field of view

	sceneDefinition(); // Let's define the scene

	if (argc > 2) {
		// rotate the light to the desired angle based on the frame to render
		float angle = 2*M_PI/60.0 * stoi(argv[2]);
		printf("Rendering frame %s, angle %f \n", argv[2], angle);
		float x = lights[1]->position.x * cos(angle) - lights[1]->position.z * sin(angle);
		float z = lights[1]->position.x * sin(angle) + lights[1]->position.z * cos(angle);
		lights[1]->position = glm::vec3(x, lights[1]->position.y, z);
	}

	Image image(width,height); // Create an image where we will store the result

	const float s =(2 * tan(fov/2)) / width; // size of one pixel
	const float X = -(width * s) / 2; // X coordinate of the first pixel
	const float Y = (height * s) / 2; // Y coordinate of the first pixel
    const float Z = 1;
	glm::vec3 origin = glm::vec3(0.0, 0.0, 0.0); // origin of the camera

	// Loop over all pixels and traverce the rays through the scene
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
			image.setPixel(i, j, trace_ray(ray));
        }
	}
    t = clock() - t;
    cout<<"It took " << ((float)t)/CLOCKS_PER_SEC<< " seconds to render the image."<< endl;
    cout<<"I could render at "<< (float)CLOCKS_PER_SEC/((float)t) << " frames per second."<<endl;

	// Writing the final results of the rendering
	if (argc > 2) {
		image.writeImage(argv[1]);
	} else {
		image.writeImage("./render/result.ppm");
	}

    return 0;
}
