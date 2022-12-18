#include "renderer.h"
#include <string>

/**
 Function defining the default scene;
 */
struct Scene sceneDefinition() {
    vector<Light *> lights;
    glm::vec3 ambient_light(0.01f);
    vector<Object *> objects;

    // Materials -------------------------------------------------------------------------------------------------------
    Material blue;
    blue.setAmbient(glm::vec3(0.07f, 0.07f, 0.1f));
    blue.setDiffuse(glm::vec3(0.2f, 0.2f, 1.0f));
    blue.setSpecular(glm::vec3(0.6));
    blue.setRoughness(0.0);
    blue.setReflectivity(0.5f);
    blue.setRefractivity(0.0f);

    // blue.ambient = glm::vec3(0.07f, 0.07f, 0.1f);
    // blue.diffuse = glm::vec3(0.7f, 0.7f, 1.0f);
    // blue.specular = glm::vec3(0.6);
    // blue.shininess = 100.0;
    // blue.reflectivity = 1.0f;

    Material red;
    red.setAmbient(glm::vec3(0.01f, 0.03f, 0.03f));
    red.setDiffuse(glm::vec3(1.0f, 0.3f, 0.3f));
    red.setSpecular(glm::vec3(0.5));
    red.setRoughness(0.5);

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

    Material stone;
    stone.setDiffuse("src/Textures/stone_basecolor.jpg");
    stone.setAmbient("src/Textures/stone_ambientOcclusion.jpg");
    stone.setRoughness("src/Textures/stone_roughness.jpg");
    stone.setNormal("src/Textures/stone_normal.jpg");

    Material organic;
    organic.setDiffuse("src/Textures/organic_basecolor.jpg");
    organic.setAmbient("src/Textures/organic_ambientOcclusion.jpg");
    organic.setRoughness("src/Textures/organic_roughness.jpg");
    organic.setNormal("src/Textures/organic_normal.jpg");
    // organic.setSpecular(glm::vec3(1.0));
    organic.setReflectivity(0.5f);

    Material fur;
    fur.setDiffuse("src/Textures/fur_basecolor.jpg");
    fur.setAmbient("src/Textures/fur_ambientOcclusion.jpg");
    fur.setRoughness("src/Textures/fur_roughness.jpg");
    fur.setNormal("src/Textures/fur_normal.jpg");

    // Procedural textures
    Material checkerboard;
    checkerboard.setDiffuse(checkerboardTexture);
    checkerboard.setDiffuse(glm::vec3(1.0f));
    checkerboard.setSpecular(glm::vec3(1.0));
    checkerboard.setRoughness(0.5f);
    checkerboard.setReflectivity(0.4f);

    // checkerboard.texture = &checkerboardTexture;
    // checkerboard.ambient = glm::vec3(0.0f);
    // checkerboard.specular = glm::vec3(1.0);
    // checkerboard.shininess = 100.0;
    // checkerboard.reflectivity = 0.4f;

    Material rainbow;
    rainbow.setDiffuse(rainbowTexture);
    rainbow.setAmbient(glm::vec3(0.0f));
    rainbow.setSpecular(glm::vec3(0.0));
    rainbow.setRoughness(0.7f);

    // rainbow.texture = &rainbowTexture;
    // rainbow.ambient = glm::vec3(0.0f);
    // rainbow.specular = glm::vec3(1.0);
    // rainbow.shininess = 10.0;

    Material refractive;
    refractive.setSpecular(glm::vec3(1.0));
    refractive.setRoughness(0.5);
    refractive.setReflectivity(1.0f);
    refractive.setRefractivity(1.0f);
    refractive.setRefractionIndex(2.0f);

    // refractive.specular = glm::vec3(1.0);
    // refractive.shininess = 10.0;
    // refractive.reflectivity = 1.0f;
    // refractive.refractivity = 1.0f;
    // refractive.refraction_index = 2.0f;

    // triangle soup
    TriangleSoup *tsoup = new TriangleSoup();
    tsoup->loadObj("src/Models/BreakfastRoom/breakfast_room_fixed_colors.obj");
    tsoup->setTransformation(genTRMat(glm::vec3(0.0), glm::vec3(0.0, -13, 0.0), glm::vec3(1.0f)));
    objects.push_back(tsoup);

    // SPHERES ---------------------------------------------------------------------------------------------------------
    // glm::mat4 sp1_t = genTRMat(glm::vec3(1.0, -2.0, 8.0), glm::vec3(0.0), glm::vec3(1.0f));
    // Sphere *sp1 = new Sphere(blue, sp1_t);

    // glm::mat4 sp2_t = genTRMat(glm::vec3(-1.0, -2.5, 6.0), glm::vec3(0.0), glm::vec3(0.5f));
    // Sphere *sp2 = new Sphere(fur, sp2_t);

    // // glm::mat4 sp3_t = genTRMat(glm::vec3(3.0, -2.0, 6.0),glm::vec3(0.0),glm::vec3(1.0f));
    // // Sphere *sp3 = new Sphere(green,sp3_t);

    // objects.push_back(sp1);
    // objects.push_back(sp2);
    // // objects.push_back(sp3);

    // // textured spheres

    TriangleSoup *smoothSphere = new TriangleSoup();
    smoothSphere->loadObj("src/Models/SmoothSphere/smooth_sphere.obj");
    smoothSphere->setTransformation(genTRMat(glm::vec3(-3,2,9), glm::vec3(0.0), glm::vec3(4.0f)));
    objects.push_back(smoothSphere);

    TriangleSoup *flatSphere = new TriangleSoup();
    flatSphere->loadObj("src/Models/FlatSphere/flat_sphere.obj");
    flatSphere->setTransformation(genTRMat(glm::vec3(3,2,9), glm::vec3(0.0), glm::vec3(4.0f)));
    objects.push_back(flatSphere);

    glm::mat4 sp4_t = genTRMat(glm::vec3(-5, 0, -1.5), glm::vec3(0.0), glm::vec3(1.5f));
    Sphere *sp4 = new Sphere(blue, sp4_t);
    // glm::mat4 sp5_t = genTRMat(glm::vec3(-6, 0.0, 16), glm::vec3(0.0), glm::vec3(3.0f));
    // Sphere *sp5 = new Sphere(fur, sp5_t);

    objects.push_back(sp4);
    // objects.push_back(sp5);
    // //
    // //    glm::mat4 sp6_t = genTRMat(glm::vec3(0.0, 27.0, 0.0),glm::vec3(0.0),glm::vec3(1.0f));
    // //    Plane *sp6 = new Plane(blue,sp6_t);
    // //    objects.push_back(sp6);

    // glm::mat4 sp7_t = genTRMat(glm::vec3(-3.0, -1.0, 8.0), glm::vec3(0.0), glm::vec3(2.0f));
    // Sphere *sp7 = new Sphere(refractive, sp7_t);
    // objects.push_back(sp7);

    // // PLANES ----------------------------------------------------------------------------------------------------------
    // // y
    // glm::mat4 p1_t = genTRMat(glm::vec3(0.0, -3.0, 0.0), glm::vec3(0.0), glm::vec3(1.0f));
    // Plane *p1 = new Plane(stone, p1_t);
    // objects.push_back(p1);

    // glm::mat4 p2_t = genTRMat(glm::vec3(0.0, 27.0, 0.0), glm::vec3(180.0, 0, 0), glm::vec3(1.0f));
    // Plane *p2 = new Plane(dark_blue, p2_t);
    // objects.push_back(p2);

    // // x
    // glm::mat4 p3_t = genTRMat(glm::vec3(-15.0, 0.0, 0.0), glm::vec3(0.0, 0, -90), glm::vec3(1.0f));
    // Plane *p3 = new Plane(blue, p3_t);
    // objects.push_back(p3);

    // glm::mat4 p4_t = genTRMat(glm::vec3(15.0, 0.0, 0.0), glm::vec3(0.0, 0, 90), glm::vec3(1.0f));
    // Plane *p4 = new Plane(red, p4_t);
    // objects.push_back(p4);

    // // z
    // glm::mat4 p5_t = genTRMat(glm::vec3(0.0, 0.0, -0.01), glm::vec3(90.0, 0, 0), glm::vec3(1.0f));
    // Plane *p5 = new Plane(green, p5_t);
    // objects.push_back(p5);

    // glm::mat4 p6_t = genTRMat(glm::vec3(0.0, 0.0, 30.0), glm::vec3(-90.0, 0, 0), glm::vec3(1.0f));
    // Plane *p6 = new Plane(rainbow, p6_t);
    // objects.push_back(p6);

    // // CONES -----------------------------------------------------------------------------------------------------------
    glm::mat4 cone1_t = genTRMat(glm::vec3(4.5, -1.0, -2.0), glm::vec3(0.0), glm::vec3(1.0f, 4.0f, 1.0f));
    Cone *cone1 = new Cone(organic, cone1_t);
    objects.push_back(cone1);

    // glm::mat4 cone2_t = genTRMat(glm::vec3(3.0, -2.0, 9.0), glm::vec3(0.0, 0.0, -110.0), glm::vec3(1.0f, 3.0f, 1.0f));
    // Cone *cone2 = new Cone(checkerboard, cone2_t);
    // objects.push_back(cone2);

    // lights
    lights.push_back(new Light(glm::vec3(0.0, 5, 0), glm::vec3(0.6)));
    lights.push_back(new Light(glm::vec3(10.0, 7, 7), glm::vec3(3.0, 2.0, 1.0)));
    lights.push_back(new Light(glm::vec3(0, 3, 6), glm::vec3(0.5)));
    // lights.push_back(new Light(glm::vec3(0.0, 26.0, 5.0), glm::vec3(4.0)));
    // lights.push_back(new Light(glm::vec3(1.0, 3.0, 12.0), glm::vec3(2.0)));
    // lights.push_back(new Light(glm::vec3(0.0, 5.0, 2.0), glm::vec3(1.0)));
    //    lights.push_back(new Light(glm::vec3(0.0, 20.0, 16.0), glm::vec3(1.0)));

    struct Scene newscene;
    newscene.objects = objects;
    newscene.lights = lights;
    newscene.ambient_light = ambient_light;
    return newscene;
}


int main(int argc, char const *argv[]) {
    struct Scene scene = sceneDefinition();
    string filename = argc == 2 ? string(argv[1]) : string("");

    return render(scene, filename);
}