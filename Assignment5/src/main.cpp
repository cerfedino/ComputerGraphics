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
    return render(scene, filename);
}