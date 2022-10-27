#include "renderer.h"

int main(int argc, char const *argv[]) {
    struct Scene scene = sceneDefinition();
//    scene.objects.at(0)->setTransformation(genTRMat(glm::vec3(10.0, -2.0, 8.0),glm::vec3(0.0),glm::vec3(10.0)));
    return render(scene, argc, argv);
}
