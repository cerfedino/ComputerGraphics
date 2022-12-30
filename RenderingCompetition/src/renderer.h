/**
@file main.cpp
*/

#include <omp.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <random>

#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/vector_angle.hpp"

#include "base.h"
#include "Image.h"
#include "Material.h"
#include "Object.h"
#include "Object_primitives.h"
#include "Object_triangle_soup.h"


using namespace std;



/**
 Light class
 */
class Light {
   public:
    glm::vec3 position;  ///< Position of the light source
    glm::vec3 color;     ///< Color/intensity of the light source
    Light(glm::vec3 position) : position(position) {
        color = glm::vec3(1.0);
    }
    Light(glm::vec3 position, glm::vec3 color) : position(position), color(color) {
    }
};

struct Scene {
    vector<Light *> lights;
    glm::vec3 ambient_light;
    vector<Object *> objects;
};
vector<Light *> lights;  ///< A list of lights in the scene
glm::vec3 ambient_light;
vector<Object *> objects;  ///< A list of all objects in the scene

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

    // Loop over all objects to find the closest intersection
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
glm::vec3 IlluminationModel(glm::vec3 point, glm::vec3 normal, glm::vec3 normal_shading, glm::vec2 uv, glm::vec3 view_direction, Material material, const int maxBounces) {
    glm::vec3 color(0.0);
    // flip normal if it is pointing away from the view direction
    if (glm::dot(normal, view_direction) < 0) {
        normal = -normal;
    }
    // flip shading normal if it is pointing away from the view direction
    if (glm::dot(normal_shading, view_direction) < 0) {
        normal_shading = -normal_shading;
    }

    for (Light *light : lights) {
        glm::vec3 l = glm::normalize(light->position - point);  // direction from the point to the light source

        // Shadows
        const float distance_from_light = glm::distance(point, light->position);
        // If the shadow ray hits an object, the point is in shadow and we don/t compute ambient/diffuse.
        if (!is_shadowed(point, normal, l, distance_from_light)) {
            // If there is a texture, takes the diffuse color of the texture instead.
            glm::vec3 diffuse_color = material.getDiffuse(uv);

            const float diffuse = max(0.0f, glm::dot(l, normal_shading));

            glm::vec3 h = glm::normalize(l + view_direction);  // half vector
            const float specular = max(0.0f, glm::pow(glm::dot(h, normal_shading), 4 * material.getShininess(uv)));

            // Attenuation
            const float distance = max(0.1f, distance_from_light);
            const float attenuation = 1.0f / glm::pow(distance, 2.0f);
            //
            color += attenuation * light->color * (diffuse_color * diffuse + material.getSpecular() * specular);
        }
    }

    // Limit reflection and refraction bounces to avoid infinite recursion.
    if (maxBounces > 0) {
        // Reflection
        glm::vec3 reflection(0.0f);
        if (material.getReflectivity() > 0) {
            color *= 1 - material.getReflectivity();
            glm::vec3 reflection_direction = glm::reflect(-view_direction, normal_shading);
            Ray reflection_ray = Ray(point + 0.001f * reflection_direction, reflection_direction);
            Hit closest_hit = closest_intersection(reflection_ray);
            if (closest_hit.hit) {
                reflection = material.getReflectivity() * IlluminationModel(closest_hit.intersection, closest_hit.normal, closest_hit.normal_shading, closest_hit.uv, glm::normalize(-reflection_direction), *closest_hit.material, maxBounces - 1);
            }
        }

        // Refraction
        glm::vec3 refraction(0.0f);
        if (material.getRefractivity() > 0) {
            color *= (1 - material.getRefractivity());
            const bool is_entering = glm::dot(normal_shading, -view_direction) < 0.0f;
            /* TODO:    We assume that one of the two materials is air, whether we are entering or exiting.
                        How would we compute a glass sphere submerged in water ? */
            const float n1 = is_entering ? 1.0f : material.getRefractionIndex();
            const float n2 = is_entering ? material.getRefractionIndex() : 1.0f;
            const float eta = n1 / n2;
            glm::vec3 refraction_direction = glm::refract(-view_direction, is_entering ? normal_shading : -normal_shading, eta);
            Ray refraction_ray = Ray(point + 0.001f * refraction_direction, refraction_direction);

            Hit closest_hit = closest_intersection(refraction_ray);
            if (closest_hit.hit) {
                refraction = material.getRefractivity() * IlluminationModel(closest_hit.intersection, closest_hit.normal, closest_hit.normal_shading, closest_hit.uv, glm::normalize(-refraction_direction), *closest_hit.material, maxBounces - 1);
                // Fresnel effect
                float O1 = cos(glm::angle(normal_shading, view_direction));
                float O2 = cos(glm::angle(-normal_shading, refraction_direction));

                float R = 0.5 * (pow((n1 * O1 - n2 * O2) / (n1 * O1 + n2 * O2), 2) + pow((n1 * O2 - n2 * O1) / (n1 * O2 + n2 * O1), 2));
                float T = 1 - R;

                reflection *= R;
                refraction *= T;
            }
        }
        color += reflection + refraction;
    }

    color += ambient_light * material.getAmbient(uv);

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
        color = IlluminationModel(closest_hit.intersection, closest_hit.normal, closest_hit.normal_shading, closest_hit.uv, glm::normalize(-ray.direction), *closest_hit.material, 15);
    }
    // clamp the final color to [0,1]
    return glm::clamp(color, glm::vec3(0.0), glm::vec3(1.0));
}


/**
 Function performing tonemapping of the intensities computed using the raytracer
 @param intensity Input intensity.
 @return Tonemapped intensity in range (0,1)
 */
glm::vec3 toneMapping(const glm::vec3 &intensity) {
    // Tonemapping parameters
    const float alpha = 5.5f;
    const float beta = 1.3f;
    const float gamma = 1.8f;

    const glm::vec3 tonemapped = alpha * glm::pow(glm::pow(intensity, glm::vec3(beta)), glm::vec3(1.0 / gamma));
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

    int width  = 4000;   // width of the image
    int height = 4000;  // height of the image

    float fov = 90;  // field of view

    lights = renderScene.lights;
    objects = renderScene.objects;
    ambient_light = renderScene.ambient_light;

    Image image(width, height);  // Create an image where we will store the result

    const float s = 2 * tan(0.5 * fov / 180 * M_PI) / width;  // size of one pixel
    const float X = -(width * s) / 2;                         // X coordinate of the first pixel
    const float Y = (height * s) / 2;                         // Y coordinate of the first pixel
    const float Z = -1;
    glm::vec3 origin = glm::vec3(0.0, 3.0, 6);  // origin of the camera

    // Define tiles for parallelization
    // Tiles are a good way to parallelize ray tracing since we can expect that rays in the same tile will behave similarly (i.e. they will hit the same objects).
    const int tile_size = 16;
    const int tiles_x = (width + tile_size - 1) / tile_size;   // add one tile if width is not a multiple of tile_size
    const int tiles_y = (height + tile_size - 1) / tile_size;  // add one tile if height is not a multiple of tile_size
    const int tile_count = tiles_x * tiles_y;

    // anti-aliasing parameters
    const int samples_per_pixel_x = 2;
    const int samples_per_pixel_y = samples_per_pixel_x;
    const float subpixel_size_x = s/samples_per_pixel_x;
    const float subpixel_size_y = s/samples_per_pixel_y;

// Loop over all tiles and traverse the rays through the scene
#pragma omp parallel for schedule(dynamic, 1)
    for (int tile = 0; tile < tile_count; tile++) {
        // random number generator, used for anti-aliasing
        std::mt19937 gen(42 + tile); // Standard mersenne_twister_engine
        std::uniform_real_distribution<> jitter(-0.5, 0.5); // Random real number between -0.5 and 0.5

        // print thread 1 progress
        if (omp_get_thread_num() == 0) {
            cout << "Progress: " << ceil((float)tile / tile_count * 10000) / 100 << "%\r";
            cout.flush();
        }

        // Compute the tile coordinates
        const int tile_j = tile / tiles_x;                             // the tile column number
        const int tile_i = tile - tile_j * tiles_x;                    // the tile row number
        const int tile_i_start = tile_i * tile_size;                   // the x coordinate of the tile
        const int tile_j_start = tile_j * tile_size;                   // the y coordinate of the tile
        const int tile_i_end = min(tile_i_start + tile_size, width);   // the x coordinate of the tile + tile_size
        const int tile_j_end = min(tile_j_start + tile_size, height);  // the y coordinate of the tile + tile_size

        // Loop over all pixels in the tile
        for (int i = tile_i_start; i < tile_i_end; i++) {
            for (int j = tile_j_start; j < tile_j_end; j++) {
                // X coordinate of the current pixel
                const float dx = X + i * s + 0.5 * s;
                // Y coordinate of the current pixel
                const float dy = Y - j * s - 0.5 * s;

                glm::vec3 color(0.0);
                // anti-aliasing
                // trace 4 rays per pixel and average the results
                for (int k = 0; k < samples_per_pixel_x; k++) {
                    for (int l = 0; l < samples_per_pixel_y; l++) {
                        // jitter the ray
                        float x_offset = -0.5*s + (k+0.5+jitter(gen))*subpixel_size_x;
                        float y_offset = -0.5*s + (l+0.5+jitter(gen))*subpixel_size_y;
                        // create the ray
                        Ray ray(origin, glm::normalize(glm::vec3(dx + x_offset, dy + y_offset, Z)));
                        // trace the ray and add the color to the pixel
                        color += trace_ray(ray);
                    }
                }
                color /= samples_per_pixel_x * samples_per_pixel_y;

                // trace the ray and set the color of the pixel
                image.setPixel(i, j, toneMapping(color));
            }
        }
    }

    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
    chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(end - start);

    cout << "It took " << time_span.count() << " seconds to render the image." << endl;
    // cout<<"I could render at "<< (float)CLOCKS_PER_SEC/((float)t) << " frames per second."<<endl;

    // Writing the final results of the rendering
    cout << "Saving render to file '" << filename << "'\n";
    if (!filename.empty()) {
        image.writeImage(filename.c_str());
    } else {
        image.writeImage("./render/result.ppm");
    }

    return 0;
}