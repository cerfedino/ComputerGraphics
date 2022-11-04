#include "renderer.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>
#include <chrono>


using namespace std;

/**
 * Performs several transformations on a scene, given an animation progress percentage.
 * 
 * @param percentage the percentage representing the progress of the animation.
 * @param scene the scene to animate.
 * @return struct Scene 
 */
struct Scene transform(float percentage, struct Scene scene) {
    const glm::vec3 transl1 = glm::vec3(12,0,0);
    const glm::vec3 transl2 = glm::vec3(0,5,0);
    const glm::vec3 rotat1 = glm::vec3(0,180,0);
    const glm::vec3 rotat2 = glm::vec3(0,160,0);
    const glm::vec3 scale2 = glm::vec3(0,2,0);

    const glm::vec3 transl3 = glm::vec3(14,0,14);

    scene.objects.at(2)->setTransformation(genTRMat(glm::vec3(-6,4,23)+transl1*percentage,glm::vec3(0.0)+rotat1*percentage,glm::vec3(7.0f)));
    scene.objects.at(3)->setTransformation(genTRMat(glm::vec3(-6,0.0,16)+transl2*percentage,glm::vec3(0.0)+rotat2*percentage,glm::vec3(3.0f)+scale2*percentage));
        
    scene.lights.at(1)->position = glm::vec3(0.0, 3.0, 12.0) + transl3*percentage;

    return scene;
}

/**
 * Performs several transformations on a sphere, given a pointer to the scene and an animation progress percentage.
 *
 * @param percentage the percentage representing the progress of the animation.
 * @param scene the scene to animate.
 * @return struct Scene
 */
void bounce_ball1(float percentage, struct Scene* scene) {
    // initial ball position
    const glm::vec3 ball1_initial_translation = glm::vec3(-3.0,-1.0-0.5,8.0);
    const glm::vec3 ball1_initial_rotation = glm::vec3(0.0);
    const glm::vec3 ball1_initial_scale = glm::vec3(2.0, 1.5, 2.0);

    const glm::vec3 ball1_translation = glm::vec3(0.0, 3.0*percentage, 0.0);
    const glm::vec3 ball1_rotation = glm::vec3(0.0);
    const glm::vec3 ball1_scale = glm::vec3(1.0f * max(1.0f-percentage-0.6, 0.0), 0.7 * min(3.0*percentage, 1.0), 0.0);

    const glm::mat4 ball1_final_transformation = genTRMat(ball1_initial_translation + ball1_translation,
                                                          ball1_initial_rotation + ball1_rotation,
                                                          ball1_initial_scale + ball1_scale);

    scene->objects.at(4)->setTransformation(ball1_final_transformation);

}

/**
 * Performs several transformations on a sphere, given a pointer to the scene and an animation progress percentage.
 *
 * @param percentage the percentage representing the progress of the animation.
 * @param scene the scene to animate.
 * @return struct Scene
 */
void bounce_ball2(float percentage, struct Scene* scene) {
    // initial ball position
    const glm::vec3 ball2_initial_translation = glm::vec3(1.0,-2.0-0.5,8.0);
    const glm::vec3 ball2_initial_rotation = glm::vec3(0.0);
    const glm::vec3 ball2_initial_scale = glm::vec3(1.0, 0.5, 1.0);

    const glm::vec3 ball2_translation = glm::vec3(0.0, 5.0*percentage, 0.0);
    const glm::vec3 ball2_rotation = glm::vec3(0.0);
    const glm::vec3 ball2_scale = glm::vec3(0.1f * max(1.0f-percentage-0.5, 0.0), 0.5 * min(4.0*percentage, 1.0), 0.0);

    const glm::mat4 ball2_final_transformation = genTRMat(ball2_initial_translation + ball2_translation,
                                                          ball2_initial_rotation + ball2_rotation,
                                                          ball2_initial_scale + ball2_scale);

    scene->objects.at(0)->setTransformation(ball2_final_transformation);
}

/**
 * Pads the start of a String with some amount of characters in order to reach a minimum length.
 * 
 * @param pad the character to use when padding.
 * @param number the minimum lenght of the string.
 * @param str the string to pad.
 * @return string the padded string.
 */
string padStart(char pad, int number, string str) {
    return std::string(number - min(number, int(str.length())), pad) + str;
}

/**
 * Renders the animation
 * 
 * @param argv The first parameter that can be passed is the framerate. Used for defining the amount of frames to render.
 * @return int 
 */
int main(int argc, char const *argv[]) {
    const float seconds = 2.0;
    // The amount of frames to render. Involves, if specified, the framerate.
    const int frames = argc == 2 ? seconds*atoi(argv[1]) : 15;

    // Sets the pad length based on the digits of 'frames'
    const int pad = log10(frames)+1;

    struct Scene scene = sceneDefinition();
    // Total number of rendered images. Used for naming the output frames.
    int number = 0 ;
    // Current index of rendered image
    int i = number;

    chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

    for (; i < frames/2; i++,number++) {
        // Animation progress percentage
        const float quadraticPercent = 1.0 - (i*i/(float)(frames*frames))*3.8;
        cout << "\n\nFrame "<<number+1<<"/"<<frames<<" ["<<number/(float)frames*100.0 << "%]\nAnimating:"<<quadraticPercent*100;

        string filename ("render/animation/render_"+padStart('0',pad,to_string(number))+".ppm");
        cout << "\n'" << filename << "'\n";
        bounce_ball1(quadraticPercent, &scene);
        bounce_ball2(quadraticPercent, &scene);
        scene.objects.at(2)->setTransformation(genTRMat(glm::vec3(-6,4,23),glm::vec3(0.0, -180.0, 0.0)*(number/(float)frames),glm::vec3(7.0f)));
        // scene.objects.at(10)->setTransformation(genTRMat(glm::vec3(3.0*(number/(float)frames), 0.0, 30.0),glm::vec3(-90.0, 0, 0),glm::vec3(1.0f)));
        render(scene, filename);
    }
    for (; i > 0; i--,number++) {
        // Animation progress percentage
        const float quadraticPercent = 1.0 -(i*i/(float)(frames*frames))*3.8;
        cout << "\n\nFrame "<<number+1<<"/"<<frames<<" ["<<number/(float)frames*100.0 << "%]\nAnimating:"<<quadraticPercent*100;

        string filename ("render/animation/render_"+padStart('0',pad,to_string(number))+".ppm");
        cout << "\n'" << filename << "'\n";
        bounce_ball1(quadraticPercent, &scene);
        bounce_ball2(quadraticPercent, &scene);
        scene.objects.at(2)->setTransformation(genTRMat(glm::vec3(-6,4,23),glm::vec3(0.0, -180.0, 0.0)*(number/(float)frames),glm::vec3(7.0f)));
        // scene.objects.at(10)->setTransformation(genTRMat(glm::vec3(3.0*(number/(float)frames), 0.0, 30.0),glm::vec3(-90.0, 0, 0),glm::vec3(1.0f)));
        render(scene, filename);
    }

    chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
    cout << "\n Rendered " << number << " frames in " << chrono::duration_cast<chrono::minutes>(end - start).count() << " minutes.\n";
}
