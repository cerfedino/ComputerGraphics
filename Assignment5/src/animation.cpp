#include "renderer.h"
#include <iostream>
#include <string>

using namespace std;
int main(int argc, char const *argv[]) {
    struct Scene scene = sceneDefinition();
    int number = 0;
    for (int i = 1; i <= 5; i++,number++) {
        scene.objects.at(1)->setTransformation(genTRMat(glm::vec3(1.0, -2.0, 8.0),glm::vec3(0.0),glm::vec3((float)i)));

        string filename ("render/animation/render_"+to_string(number)+".ppm");
        render(scene, filename);
    }
    for (int i = 4; i > 1; i--,number++) {
        scene.objects.at(1)->setTransformation(genTRMat(glm::vec3(1.0, -2.0, 8.0),glm::vec3(0.0),glm::vec3((float)i)));
            
        string filename ("render/animation/render_"+to_string(number)+".ppm");
        render(scene, filename);
    }
}
