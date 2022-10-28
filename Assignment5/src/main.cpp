#include "renderer.h"
#include <string>

int main(int argc, char const *argv[]) {
    struct Scene scene = sceneDefinition();
    string filename;
    if(argc==2) {
        filename = string(argv[1]);
    } else {
        filename = string("");
    }
   scene.objects.at(0)->setTransformation(genTRMat(glm::vec3(10.0, -2.0, 8.0),glm::vec3(0.0),glm::vec3(10.0)));
    return render(scene, filename);
}
