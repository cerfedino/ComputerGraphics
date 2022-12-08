#include "renderer.h"
#include <string>

int main(int argc, char const *argv[]) {
    struct Scene scene = sceneDefinition();
    string filename = argc == 2 ? string(argv[1]) : string("");

    return render(scene, filename);
}